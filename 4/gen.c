#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "gen.h"
#include "symtbl.h"
#define GLOBAL 0

extern FILE *outfp;   /* declare in parser.y */
extern int linenum;   /* declared in lex.l */
extern int seq;       /* declared in parser.y */
extern int exprLabel; /* declared in parser.y */
extern struct SymbolTable *Alice;   /* declared in parser.y */

static int stacksize = 0;
static int stackcapacity = 0;
static struct Label *stack = NULL;

void StackPush(int num, int type){
    if(stackcapacity == 0){
        stackcapacity = 2048;
        stack = (struct Label*)malloc(stackcapacity * sizeof(struct Label));
    }

    if(stacksize == stackcapacity){
        stackcapacity *= 2;
        struct Label *old = stack;
        stack = (struct Label*)malloc(stackcapacity * sizeof(struct Label));
        memcpy(stack, old, stacksize * sizeof(struct Label));
    }

    stack[stacksize].num = num;
    stack[stacksize].type = type;
    stacksize++;
}

int StackTop(int type){
    int i;
    for(i=stacksize-1; i>=0; i--){
        if(stack[i].type == type){
            return stack[i].num;
        }
    }
    return -1;
}

int StackPop(){
    return (stacksize==0)? -1 : stack[--stacksize].num;
}

void Gen(int n, ...){
    va_list ap;
    int i;

    va_start(ap, n);
    for(i=0; i<n; i++){
        fprintf(outfp, "%s ", va_arg(ap, char*));
    }
    va_end(ap);
    fprintf(outfp, "\n");
}

void GenTitle(){
    fprintf(outfp, "; alice.j\n");
    fprintf(outfp, ".class public alice\n");
    fprintf(outfp, ".super java/lang/Object\n");
    fprintf(outfp, ".field public static _sc Ljava/util/Scanner;\n\n");
}

void GenVariableDecl(void* alice){
    if(alice == NULL)   return;
    struct Entry *entry = (struct Entry*)alice;
    char *id = entry->name;
    char *typename = entry->type->type;
    char typedesc[2];
    typedesc[1] = '\0';

    if(strcmp(entry->kind, "constant") == 0)
        return;
    if(entry->level == GLOBAL){
        if(strcmp(typename, "int") == 0){
            typedesc[0] = 'I';
        }
        else if(strcmp(typename, "float") == 0){
            typedesc[0] = 'F';
        }
        else if(strcmp(typename, "double") == 0){
            typedesc[0] = 'D';
        }
        else if(strcmp(typename, "bool") == 0){
            typedesc[0] = 'Z';
        }

        fprintf(outfp, ".field public static %s %s\n", id, typedesc);
    } 
    else {
        entry->reg = seq++;
        if(strcmp(typename, "double") == 0)
            seq++;
    }
}

void GenFunction(void* alice){
    if(alice == NULL)   return;
    struct Entry *entry = (struct Entry*)alice;
    struct Argu *ptr;
    char *id = entry->name;
    char *typename = entry->type->type;
    char argulist[200];
    char typedesc[2];
    typedesc[1] = '\0';
    
    if(strcmp(id, "main") == 0){
        fprintf(outfp, ".method public static main([Ljava/lang/String;)V\n");
        fprintf(outfp, ".limit stack 128\n");
        fprintf(outfp, ".limit locals 128\n");
        fprintf(outfp, "new java/util/Scanner\n");
        fprintf(outfp, "dup\n");
        fprintf(outfp, "getstatic java/lang/System/in Ljava/io/InputStream;\n");
        fprintf(outfp, "invokespecial java/util/Scanner/<init>(Ljava/io/InputStream;)V\n");
        fprintf(outfp, "putstatic alice/_sc Ljava/util/Scanner;\n");
        return;
    }
    
    if(strcmp(typename, "int") == 0){
        typedesc[0] = 'I';
    }
    else if(strcmp(typename, "float") == 0){
        typedesc[0] = 'F';
    }
    else if(strcmp(typename, "double") == 0){
        typedesc[0] = 'D';
    }
    else if(strcmp(typename, "bool") == 0){
        typedesc[0] = 'Z';
    }
    else if(strcmp(typename, "void") == 0){
        typedesc[0] = 'V';
    }
    
    memset(argulist, 0, sizeof(argulist));
    snprintf(argulist, sizeof(argulist), "%s(", id);
    for(ptr=entry->attr->argu; ptr!=NULL; ptr=ptr->next){
        char *argutypename = ptr->type->type;
        if(strcmp(argutypename, "int") == 0){
            strcat(argulist, "I");
        }
        else if(strcmp(argutypename, "float") == 0){
            strcat(argulist, "F");
        }
        else if(strcmp(argutypename, "double") == 0){
            strcat(argulist, "D");
        }
        else if(strcmp(argutypename, "bool") == 0){
            strcat(argulist, "Z");
        }
    }
    strcat(argulist, ")");
    strcat(argulist, typedesc);

    fprintf(outfp, ".method public static %s\n", argulist);
    fprintf(outfp, ".limit stack 128\n");
    fprintf(outfp, ".limit locals 128\n");
}

void GenLoadVal(void* alice){
    if(alice == NULL)   return;
    struct Value *val = (struct Value*)alice;
    char *typename = val->type->type;
    char exprbuf[200];

    if(strcmp(typename, "int") == 0){
        snprintf(exprbuf, sizeof(exprbuf), "ldc %d", val->ival);
    }
    else if(strcmp(typename, "float") == 0){
        snprintf(exprbuf, sizeof(exprbuf), "ldc %lf", val->dval);
    }
    else if(strcmp(typename, "double") == 0){
        snprintf(exprbuf, sizeof(exprbuf), "ldc %lf", val->dval);
    }
    else if(strcmp(typename, "bool") == 0){ 
        snprintf(exprbuf, sizeof(exprbuf), "iconst_%d", val->ival);
    }
    else if(strcmp(typename, "string") == 0){
        snprintf(exprbuf, sizeof(exprbuf), "ldc \"%s\"", val->strV);
    }

    fprintf(outfp, "%s\n", exprbuf);
}

void GenLoadValbyID(void* alice){
    if(alice == NULL)   return;
    struct Entry *entry = (struct Entry*)alice;
    struct Value *val = entry->attr->value;
    int level = entry->level;
    char *id = entry->name;
    char *typename = entry->type->type;
    char *kind = entry->kind;
    char exprbuf[200];
    char typedesc[2];
    typedesc[1] = '\0';
    memset(exprbuf, 0, sizeof(exprbuf));

    if(strcmp(kind, "constant") == 0){
        if(strcmp(typename, "int") == 0){
            snprintf(exprbuf, sizeof(exprbuf), "ldc %d", val->ival);
        }
        else if(strcmp(typename, "float") == 0){
            snprintf(exprbuf, sizeof(exprbuf), "ldc %lf", val->dval);
        }
        else if(strcmp(typename, "double") == 0){
            snprintf(exprbuf, sizeof(exprbuf), "ldc %lf", val->dval);
        }
        else if(strcmp(typename, "bool") == 0){ 
            snprintf(exprbuf, sizeof(exprbuf), "iconst_%d", val->ival);
        }
        else if(strcmp(typename, "string") == 0){
            snprintf(exprbuf, sizeof(exprbuf), "ldc \"%s\"", val->strV);
        }
    }
    else{
        if(level == 0){
            if(strcmp(typename, "int") == 0){
                typedesc[0] = 'I';
            }
            else if(strcmp(typename, "float") == 0){
                typedesc[0] = 'F';
            }
            else if(strcmp(typename, "double") == 0){
                typedesc[0] = 'D';
            }
            else if(strcmp(typename, "bool") == 0){
                typedesc[0] = 'Z';
            }
            snprintf(exprbuf, sizeof(exprbuf), "getstatic alice/%s %s", id, typedesc);
        }
        else{
            if(strcmp(typename, "int") == 0){
                snprintf(exprbuf, sizeof(exprbuf), "iload %d", entry->reg);
            }
            else if(strcmp(typename, "float") == 0){
                snprintf(exprbuf, sizeof(exprbuf), "fload %d", entry->reg);
            }
            else if(strcmp(typename, "double") == 0){
                snprintf(exprbuf, sizeof(exprbuf), "dload %d", entry->reg);
            }
            else if(strcmp(typename, "bool") == 0){ 
                snprintf(exprbuf, sizeof(exprbuf), "iload %d", entry->reg);
            }
        }
    }

    fprintf(outfp, "%s\n", exprbuf);
}

void GenArithExpr(void* e1, char op, void* e2){
    if(e1==NULL || e2==NULL) return;
    struct Value *v1=(struct Value*)e1, *v2=(struct Value*)e2;
    char *t1 = v1->type->type, *t2 = v2->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char exprbuf[10];
    char optype[2];
    optype[1] = '\0';
    memset(exprbuf, 0, sizeof(exprbuf));

    if(strcmp(t1, d)==0 || strcmp(t2, d)==0){
        optype[0] = 'd';
        if(strcmp(t1, f) == 0){
            fprintf(outfp, "dstore %d\n", CoerReg);
            fprintf(outfp, "f2d\n");
            fprintf(outfp, "dload %d\n", CoerReg);
        }
        else if(strcmp(t1, i) == 0){
            fprintf(outfp, "dstore %d\n", CoerReg);
            fprintf(outfp, "i2d\n");
            fprintf(outfp, "dload %d\n", CoerReg);
        }
        else if(strcmp(t2, f) == 0){
            fprintf(outfp, "f2d\n");
        }
        else if(strcmp(t2, i) == 0){
            fprintf(outfp, "i2d\n");
        }
    }
    else if(strcmp(t1, f)==0 || strcmp(t2, f)==0){
        optype[0] = 'f';
        if(strcmp(t1, i) == 0){
            fprintf(outfp, "fstore %d\n", CoerReg);
            fprintf(outfp, "i2f\n");
            fprintf(outfp, "fload %d\n", CoerReg);
        }
        else if(strcmp(t2, i) == 0){
            fprintf(outfp, "i2f\n");
        }
    }
    else {
        optype[0] = 'i';
    }
  
    switch(op) {
        case '+':
            snprintf(exprbuf, sizeof(exprbuf), "%sadd", optype);
            break;
        case '-':
            snprintf(exprbuf, sizeof(exprbuf), "%ssub", optype);
            break;
        case '*':
            snprintf(exprbuf, sizeof(exprbuf), "%smul", optype);
            break;
        case '/':
            snprintf(exprbuf, sizeof(exprbuf), "%sdiv", optype);
            break;
        case '%':
            snprintf(exprbuf, sizeof(exprbuf), "%srem", optype);
            break;
    }

    fprintf(outfp, "%s\n", exprbuf);
} 

void GenLogiExpr(void* e1, char op, void* e2){
    if(e1==NULL || e2==NULL) return;
    char exprbuf[10];
    memset(exprbuf, 0, sizeof(exprbuf));
  
    switch(op) {
        case '&':
            snprintf(exprbuf, sizeof(exprbuf), "iand");
            break;
        case '|':
            snprintf(exprbuf, sizeof(exprbuf), "ior");
            break;
    }

    fprintf(outfp, "%s\n", exprbuf);
}

void GenNegExpr(void* e1, char op){
    if(e1==NULL) return;
    struct Value *v1=(struct Value*)e1;
    char *t1 = v1->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char exprbuf[10];
    char optype[2];
    optype[1] = '\0';
    memset(exprbuf, 0, sizeof(exprbuf));

    if(strcmp(t1, d)==0){
        optype[0] = 'd';
    }
    else if(strcmp(t1, f)==0){
        optype[0] = 'f';
    }
    else {
        optype[0] = 'i';
    }
  
    switch(op) {
        case '-':
            snprintf(exprbuf, sizeof(exprbuf), "%sneg", optype);
            break;
        case '!':
            fprintf(outfp, "iconst_1\n");
            snprintf(exprbuf, sizeof(exprbuf), "ixor");
            break;
    }

    fprintf(outfp, "%s\n", exprbuf);
}

void GenRelExpr(void* e1, int op, void* e2){
    if(e1==NULL || e2==NULL) return;
    struct Value *v1=(struct Value*)e1, *v2=(struct Value*)e2;
    char *t1 = v1->type->type, *t2 = v2->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char exprbuf[10];
    char optype[2];
    optype[1] = '\0';
    memset(exprbuf, 0, sizeof(exprbuf));
    int L1, L2;

    if(strcmp(t1, d)==0 || strcmp(t2, d)==0){
        optype[0] = 'd';
        if(strcmp(t1, f) == 0){
            fprintf(outfp, "dstore %d\n", CoerReg);
            fprintf(outfp, "f2d\n");
            fprintf(outfp, "dload %d\n", CoerReg);
        }
        else if(strcmp(t1, i) == 0){
            fprintf(outfp, "dstore %d\n", CoerReg);
            fprintf(outfp, "i2d\n");
            fprintf(outfp, "dload %d\n", CoerReg);
        }
        else if(strcmp(t2, f) == 0){
            fprintf(outfp, "f2d\n");
        }
        else if(strcmp(t2, i) == 0){
            fprintf(outfp, "i2d\n");
        }
    }
    else if(strcmp(t1, f)==0 || strcmp(t2, f)==0){
        optype[0] = 'f';
        if(strcmp(t1, i) == 0){
            fprintf(outfp, "fstore %d\n", CoerReg);
            fprintf(outfp, "i2f\n");
            fprintf(outfp, "fload %d\n", CoerReg);
        }
        else if(strcmp(t2, i) == 0){
            fprintf(outfp, "i2f\n");
        }
    }
    else {
        optype[0] = 'i';
    }

    if(optype[0] != 'i'){
        fprintf(outfp, "%scmpl\n", optype);
    }
    else {
        fprintf(outfp, "%ssub\n", optype);
    }
    
    L1 = exprLabel++;
    L2 = exprLabel++;
    switch(op) {
        case LessT :
            fprintf(outfp, "iflt expr_%d\n", L1);
            break;
        case LessE :
            fprintf(outfp, "ifle expr_%d\n", L1);
            break;
        case GreatT :
            fprintf(outfp, "ifgt expr_%d\n", L1);
            break;
        case GreatE :
            fprintf(outfp, "ifge expr_%d\n", L1);
            break;
        case Equal :
            fprintf(outfp, "ifeq expr_%d\n", L1);
            break;
        case NEqual :
            fprintf(outfp, "ifne expr_%d\n", L1);
            break;
    }

    fprintf(outfp, "iconst_0\n");
    fprintf(outfp, "goto expr_%d\n", L2);
    fprintf(outfp, "expr_%d:\n", L1);
    fprintf(outfp, "iconst_1\n");
    fprintf(outfp, "expr_%d:\n", L2);
}

void GenAssignment(void* alice, void* bob){
    if(alice == NULL || bob == NULL)   return;
    struct Entry *entry = (struct Entry*)alice;
    struct Value *value = (struct Value*)bob;
    char *typename = entry->type->type;
    char *t2 = value->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char b[7] = "bool";
    char optype[2];
    optype[1] = '\0';

    if(entry->level == 0){
        if(strcmp(typename, d) == 0){
            optype[0] = 'D';
            if(strcmp(t2, f) == 0){
                fprintf(outfp, "f2d\n");
            }
            else if(strcmp(t2, i) == 0){
                fprintf(outfp, "i2d\n");
            }
        }
        else if(strcmp(typename, f) == 0){
            optype[0] = 'F';
            if(strcmp(t2, i) == 0){
                fprintf(outfp, "i2d\n");
            }
        }
        else if(strcmp(typename, i) == 0){
            optype[0] = 'I';
        }
        else if(strcmp(typename, b) == 0){
            optype[0] = 'Z';
        }
        fprintf(outfp, "putstatic alice/%s %s\n", entry->name, optype);
    }
    else {
        if(strcmp(typename, d) == 0){
            optype[0] = 'd';
            if(strcmp(t2, f) == 0){
                fprintf(outfp, "f2d\n");
            }
            else if(strcmp(t2, i) == 0){
                fprintf(outfp, "i2d\n");
            }
        }
        else if(strcmp(typename, f) == 0){
            optype[0] = 'f';
            if(strcmp(t2, i) == 0){
                fprintf(outfp, "i2d\n");
            }
        }
        else if(strcmp(typename, i) == 0){
            optype[0] = 'i';
        }
        else if(strcmp(typename, b) == 0){
            optype[0] = 'i';
        }
        fprintf(outfp, "%sstore %d\n", optype, entry->reg);
    }
}

void GenPrintStart(){
    fprintf(outfp, "getstatic java/lang/System/out Ljava/io/PrintStream;\n");
}

void GenPrintEnd(void *alice){
    if(alice == NULL)   return;
    struct Value *val = (struct Value*)alice;
    char *typename = val->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char b[7] = "bool";
    char s[7] = "string";
    char optype[50];
    optype[1] = '\0';

    if(strcmp(typename, d) == 0){
        optype[0] = 'D';
    }
    else if(strcmp(typename, f) == 0){
        optype[0] = 'F';
    }
    else if(strcmp(typename, i) == 0){
        optype[0] = 'I';
    }
    else if(strcmp(typename, b) == 0){
        optype[0] = 'Z';
    }
    else if(strcmp(typename, s) == 0){
        snprintf(optype, sizeof(optype), "Ljava/lang/String;");
    }
    fprintf(outfp, "invokevirtual java/io/PrintStream/print(%s)V\n", optype);
}

void GenRead(void* alice){
    if(alice == NULL)   return;
    struct Entry *entry = (struct Entry*)alice;
    char *typename = entry->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char b[7] = "bool";
    char optype[50];
    memset(optype, 0, sizeof(optype));
    char opt[2];
    opt[1] = '\0';

    if(strcmp(typename, d) == 0){
        snprintf(optype, sizeof(optype), "Double()D");
        opt[0] = 'd';
    }
    else if(strcmp(typename, f) == 0){
        snprintf(optype, sizeof(optype), "Float()F");
        opt[0] = 'f';
    }
    else if(strcmp(typename, i) == 0){
        snprintf(optype, sizeof(optype), "Int()I");
        opt[0] = 'i';
    }
    else if(strcmp(typename, b) == 0){
        snprintf(optype, sizeof(optype), "Boolean()Z");
        opt[0] = 'i';
    }
    fprintf(outfp, "getstatic alice/_sc Ljava/util/Scanner;\n");
    fprintf(outfp, "invokevirtual java/util/Scanner/next%s\n", optype);

    if(entry->level == 0)
        fprintf(outfp, "putstatic alice/%s %s\n", entry->name, opt);
    else
        fprintf(outfp, "%sstore %d\n", opt, entry->reg);
}

void GenReturn(void* alice, void* bob){
    if(alice == NULL || bob==NULL)   return;
    struct Value *value = (struct Value*)alice;
    struct Entry *entry = (struct Entry*)bob;
    char *typename = value->type->type;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char b[7] = "bool";
    char opt[2];
    opt[1] = '\0';

    if(strcmp(entry->name, "main") == 0){
        opt[0] = '\0';
    }
    else if(strcmp(typename, d) == 0){
        opt[0] = 'd';
    }
    else if(strcmp(typename, f) == 0){
        opt[0] = 'f';
    }
    else if(strcmp(typename, i) == 0){
        opt[0] = 'i';
    }
    else if(strcmp(typename, b) == 0){
        opt[0] = 'i';
    }

    fprintf(outfp, "%sreturn\n", opt);
}

void GenFunctionCall(void* alice){
    if(alice == NULL)   return;
    struct Entry *entry = (struct Entry*)alice;
    struct Argu *argu; 
    char *typename;
    char d[7] = "double";
    char f[7] = "float";
    char i[7] = "int";
    char b[7] = "bool";
    char v[7] = "void";
    char exprbuf[200];
    memset(exprbuf, 0, sizeof(exprbuf));
    char opt[2];
    opt[1] = '\0';

    snprintf(exprbuf, sizeof(exprbuf), "invokestatic alice/%s(", entry->name);
    for(argu=entry->attr->argu; argu!=NULL; argu=argu->next){
        typename = argu->type->type;

        if(strcmp(typename, d) == 0){
            opt[0] = 'D';
        }
        else if(strcmp(typename, f) == 0){
            opt[0] = 'F';
        }
        else if(strcmp(typename, i) == 0){
            opt[0] = 'I';
        }
        else if(strcmp(typename, b) == 0){
            opt[0] = 'Z';
        }

        strcat(exprbuf, opt);
    }
    strcat(exprbuf, ")");
    
    typename = entry->type->type;
    if(strcmp(typename, d) == 0){
        opt[0] = 'D';
    }
    else if(strcmp(typename, f) == 0){
        opt[0] = 'F';
    }
    else if(strcmp(typename, i) == 0){
        opt[0] = 'I';
    }
    else if(strcmp(typename, b) == 0){
        opt[0] = 'Z';
    }
    else if(strcmp(typename, v) == 0){
        opt[0] = 'V';
    }
    strcat(exprbuf, opt);

    fprintf(outfp, "%s\n", exprbuf);
}

void GenIfStart(){
    int GetLabel = exprLabel++;
    StackPush(GetLabel, Cond);
    fprintf(outfp, "ifeq IFelse_%d\n", GetLabel);
}

void GenIfElse(){
    int GetLabel = StackTop(Cond);
    fprintf(outfp, "goto IFexit_%d\n", GetLabel);
    fprintf(outfp, "IFelse_%d:\n", GetLabel);
}

void GenIfExit(){
    int GetLabel = StackPop();
    fprintf(outfp, "IFexit_%d:\n", GetLabel);
}

void GenForBegin(){
    int GetLabel = exprLabel++;
    StackPush(GetLabel, Loop);
    fprintf(outfp, "Loopbegin_%d:\n", GetLabel);
}

void GenForInc(){
    int GetLabel = StackTop(Loop);
    fprintf(outfp, "ifne Loopstart_%d\n", GetLabel);
    fprintf(outfp, "goto Loopexit_%d\n", GetLabel);
    fprintf(outfp, "Loopinc_%d:\n", GetLabel);
}

void GenForStart(){
    int GetLabel = StackTop(Loop);
    fprintf(outfp, "goto Loopbegin_%d\n", GetLabel);
    fprintf(outfp, "Loopstart_%d:\n", GetLabel);
}

void GenForExit(){
    int GetLabel = StackPop();
    fprintf(outfp, "goto Loopinc_%d\n", GetLabel);
    fprintf(outfp, "Loopexit_%d:\n", GetLabel);
}

void GenWhileBegin(){
    int GetLabel = exprLabel++;
    StackPush(GetLabel, Loop);
    fprintf(outfp, "Loopbegin_%d:\n", GetLabel);
    fprintf(outfp, "Loopinc_%d:\n", GetLabel);
}

void GenWhileStart(){
    int GetLabel = StackTop(Loop);
    fprintf(outfp, "ifne Loopstart_%d\n", GetLabel);
    fprintf(outfp, "goto Loopexit_%d\n", GetLabel);
    fprintf(outfp, "Loopstart_%d:\n", GetLabel);
}

void GenWhileExit(){
    int GetLabel = StackPop();
    fprintf(outfp, "goto Loopbegin_%d\n", GetLabel);
    fprintf(outfp, "Loopexit_%d:\n", GetLabel);
}

void GenDoWhileStart(){
    int GetLabel = exprLabel++;
    StackPush(GetLabel, Loop);
    fprintf(outfp, "Loopstart_%d:\n", GetLabel);
}

void GenDoWhileBegin(){
    int GetLabel = StackTop(Loop);
    fprintf(outfp, "Loopinc_%d:\n", GetLabel);
}

void GenDoWhileExit(){
    int GetLabel = StackPop();
    fprintf(outfp, "ifne Loopstart_%d\n", GetLabel);
    fprintf(outfp, "goto Loopexit_%d\n", GetLabel);
    fprintf(outfp, "Loopexit_%d:\n", GetLabel);
}

void GenBreak(){
    int GetLabel = StackTop(Loop);
    fprintf(outfp, "goto Loopexit_%d\n", GetLabel);
}

void GenContinue(){
    int GetLabel = StackTop(Loop);
    fprintf(outfp, "goto Loopinc_%d\n", GetLabel);
}
