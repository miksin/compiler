#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtbl.h"
#include "gen.h"

extern int linenum;             /* declared in lex.l */
extern struct ErrorTable *Error_msg; /* declared in parser.y */
extern int seq;     /* declared in parser.y */

struct SymbolTable* SymbolTableBuild(){
    struct SymbolTable *Alice = (struct SymbolTable*)malloc(sizeof(struct SymbolTable));
    Alice->nowlevel = 0;
    Alice->size = 0;
    Alice->capacity = 8;
    Alice->entryVector = (struct Entry**)malloc(sizeof(struct Entry*) * 8);
    return Alice;
}

void SymbolTablePrint(struct SymbolTable* Alice){
    int i;
    printf("===================================================================================================\n");
    printf("Name                             Kind       Level       Type               Attribute               \n");
    printf("---------------------------------------------------------------------------------------------------\n");
    for(i=0; i<Alice->size; i++){
        if(Alice->entryVector[i]->level == Alice->nowlevel){
            printf("%-33s%-11s", Alice->entryVector[i]->name, Alice->entryVector[i]->kind);
            SymbolTablePrintLevel(Alice->entryVector[i]->level);
            SymbolTablePrintType(Alice->entryVector[i]->type, 0);
            SymbolTablePrintAttr(Alice->entryVector[i]->attr, Alice->entryVector[i]->kind, Alice->entryVector[i]->type);
            printf("\n");
        }
    }
    printf("===================================================================================================\n");
}

void SymbolTablePrintLevel(int level){
    char buf[20];
    int i, size;
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d", level);
    if(level == 0)
        strcat(buf, "(global)");
    else
        strcat(buf, "(local)");
    for(i=0, size=12-strlen(buf); i<size; i++){
        strcat(buf, " ");
    }  
    printf("%-12s", buf);
}

void SymbolTablePrintType(struct Type *alice, int argu){
    if(alice == NULL)
        return;
    char buf[1024], dbuf[12];
    struct Arraynode *ptr;
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%s", alice->type);

    for(ptr=alice->array; ptr!=NULL; ptr=ptr->next){
        memset(dbuf, 0, sizeof(dbuf));
        snprintf(dbuf, sizeof(dbuf), "[%d]", ptr->dimension);
        strcat(buf, dbuf);
    }
    
    if(argu == 0)
        printf("%-19s", buf);
    else if(argu == 1)
        printf("%s", buf);
}

void SymbolTablePrintAttr(struct Attribute* alice, const char* kind, struct Type *type){
    if(alice == NULL)
        return;
    if(strcmp(kind, "constant") == 0){
        if(strcmp(type->type, "int") == 0){
            printf("%d", alice->value->ival);
        }
        else if(strcmp(type->type, "float") == 0 || strcmp(type->type, "double") == 0){
            printf("%lf", alice->value->dval);
        }
        else if(strcmp(type->type, "bool") == 0 || strcmp(type->type, "boolean") == 0){
            if(alice->value->ival == 0)
                printf("false");
            else
                printf("true");
        }
        else if(strcmp(type->type, "string") == 0){
            printf("%s", alice->value->strV);
        }
    }
    else if(strcmp(kind, "function") == 0){
        int count = 1;
        struct Argu *ptr;
        for(ptr=alice->argu; ptr!=NULL; ptr=ptr->next){
            if(count != 1)
                printf(",");
            SymbolTablePrintType(ptr->type, 1);
            count++;
        }
    }
}

void SymbolTablePush(struct SymbolTable* Alice, struct SymbolTable *Bob){
    if(Alice == NULL || Bob == NULL)
        return;
    int i, size=Bob->size;
    for(i=0; i<size; i++){
        if((strcmp(Bob->entryVector[i]->kind, "variable")==0 || strcmp(Bob->entryVector[i]->kind, "constant")==0) && ValDeclCheck(Alice, Bob->entryVector[i], Error_msg) != 1){
            SymbolTablePushOne(Alice, Bob->entryVector[i]);
            GenVariableDecl(Bob->entryVector[i]);
            GenAssignment(Bob->entryVector[i]);
        }
        else{
            DelEntry(Bob->entryVector[i]);
        }
    }
    Bob->size = 0;
}

void SymbolTablePushOne(struct SymbolTable* Alice, struct Entry* Bob){
    if(Alice == NULL || Bob == NULL)
        return;
    if(Alice->size == Alice->capacity){
        Alice->capacity *= 2;
        struct Entry **RMentryVector = Alice->entryVector;
        Alice->entryVector = (struct Entry**)malloc(sizeof(struct Entry*) * Alice->capacity);
        int i;
        for(i=0; i<Alice->size; i++){
            (Alice->entryVector)[i] = RMentryVector[i];
        }
        free(RMentryVector);
    }
    
    Alice->entryVector[Alice->size++] = Bob;
}

void SymbolTablePushArgu(struct SymbolTable* Alice, struct Argu* argu){
    if(Alice == NULL)
        return;
    struct Argu *ptr;
    for(ptr=argu; ptr!=NULL; ptr=ptr->next){
        struct Entry *argu_entry = BuildEntry(ptr->name, "parameter", Alice->nowlevel, CopyType(ptr->type), NULL);
        SymbolTablePushOne(Alice, argu_entry);
        argu_entry->reg = seq++;
    }
}

void SymbolTablePop(struct SymbolTable* Alice){
    int i, times=Alice->size;
    for(i=0; i<times; i++){
        if(Alice->entryVector[i]->level == Alice->nowlevel){
            DelEntry(Alice->entryVector[i]);
            Alice->entryVector[i] = NULL;
            Alice->size--;
        }
    }
    Alice->nowlevel--;
}

void SymbolTableCheckRemainFunction(struct SymbolTable* tbl, struct ErrorTable* errtbl){
    if(tbl == NULL || errtbl == NULL)
        return;

    int i, size = tbl->size;
    char msg[1024];
    for(i=0; i<size; i++){
        if(strcmp(tbl->entryVector[i]->kind, "function")==0 && tbl->entryVector[i]->decl==0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Function '%s' has no definition", tbl->entryVector[i]->name);
            ErrorTablePush(errtbl, msg);
        }
    }
}

struct Entry* SymbolTableFind(struct SymbolTable* Alice, const char* key){
    struct Entry* ptr = NULL;
    int i;
    for(i=Alice->size-1; i>=0; i--){
        if(strcmp(Alice->entryVector[i]->name, key) == 0){
            ptr = Alice->entryVector[i];
            break;
        }
    }
    return ptr;
}

struct Entry* FindID(struct SymbolTable* tbl, struct ErrorTable* errtbl, const char* key){
    struct Entry *founded = SymbolTableFind(tbl, key);
    if(founded == NULL){
        char msg[1024];
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Identifier '%s' undeclared", key);
        ErrorTablePush(errtbl, msg);
    }
    return founded;
}

struct Value* CallFunction(struct SymbolTable* tbl, struct ErrorTable* errtbl, const char* key, struct Argu* argu){
    struct Entry *founded = FindID(tbl, errtbl, key);
    if(founded == NULL)
        return NULL;
    if(strcmp(founded->kind, "function") != 0){
        char msg[1024];
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Identifier '%s' is not a function", key);
        ErrorTablePush(errtbl, msg);
        return NULL;
    }
   
    struct Argu *fargu = (founded->attr == NULL)? NULL : founded->attr->argu;
    if(CmpArguCall(fargu, argu) != 0){
        char msg[1024];
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Invalid type coercion in argument");
        ErrorTablePush(errtbl, msg);
    }

    return BuildDefaultValue(CopyType(founded->type));
}

struct ErrorTable* ErrorTableBuild(){
    struct ErrorTable *Alice = (struct ErrorTable*)malloc(sizeof(struct ErrorTable));
    Alice->size = 0;
    Alice->capacity = 8;
    Alice->errorVector = (struct Error**)malloc(sizeof(struct Error*) * 8);
    return Alice;
}

void ErrorTablePrint(struct ErrorTable* alice){
    if(alice == NULL)
        return;
    printf("\n");
    int i, size = alice->size;
    for(i=0; i<size; i++){
        printf("##########Error at Line #%-4d: %s. ##########\n", alice->errorVector[i]->linenum, alice->errorVector[i]->msg);
    }
}

void ErrorTablePush(struct ErrorTable* Alice, const char* msg){
    if(Alice->size == Alice->capacity){
        Alice->capacity *= 2;
        struct Error **RMerrorVector = Alice->errorVector;
        Alice->errorVector = (struct Error**)malloc(sizeof(struct Error*) * Alice->capacity);
        int i;
        for(i=0; i<Alice->size; i++){
            (Alice->errorVector)[i] = RMerrorVector[i];
        }
        free(RMerrorVector);
    }
    
    struct Error *Bob = (struct Error*)malloc(sizeof(struct Error));
    memset(Bob, 0, sizeof(struct Error));
    Bob->linenum = linenum;
    snprintf(Bob->msg, sizeof(Bob->msg), "%s", msg);
    Alice->errorVector[Alice->size++] = Bob;
}

int ValDeclCheck(struct SymbolTable* tbl, struct Entry* entry, struct ErrorTable* errtbl){
    if(tbl==NULL || entry==NULL || errtbl==NULL)
        return -1;

    char msg[1024];
    int reval = 0;
    struct Entry *founded;
    if((founded = SymbolTableFind(tbl, entry->name)) != NULL){
        if(founded->level == entry->level){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Identifier '%s' has been declared in this scope", entry->name);
            ErrorTablePush(errtbl, msg);
            reval = 1;
        }
    }
    
    if(strcmp(entry->type->type, "void") == 0){
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Value type should not be 'void'");
        ErrorTablePush(errtbl, msg);
        reval = 1;
    }
    
    if(entry->type->array==NULL && entry->attr!=NULL && entry->attr->value!=NULL){
        if(strcmp(entry->type->type, "double") == 0){
            if(strcmp(entry->attr->value->type->type, "double") == 0)
                ;
            else if(strcmp(entry->attr->value->type->type, "float") == 0)
                strcpy(entry->attr->value->type->type, "double");
            else if(strcmp(entry->attr->value->type->type, "int") == 0){
                strcpy(entry->attr->value->type->type, "double");
                entry->attr->value->dval = (double)entry->attr->value->dval;
            }
            else {
                memset(msg, 0, sizeof(msg));
                snprintf(msg, sizeof(msg), "Invalid initial value type coercion: %s -> %s", entry->attr->value->type->type, entry->type->type);
                ErrorTablePush(errtbl, msg);
                DelAttr(entry->attr);
                entry->attr = NULL;
                reval = 1;
            }
        }
        else if(strcmp(entry->type->type, "float") == 0){
            if(strcmp(entry->attr->value->type->type, "float") == 0)
                ;
            else if(strcmp(entry->attr->value->type->type, "int") == 0){
                strcpy(entry->attr->value->type->type, "float");
                entry->attr->value->dval = (double)entry->attr->value->dval;
            }
            else {
                memset(msg, 0, sizeof(msg));
                snprintf(msg, sizeof(msg), "Invalid initial value type coercion: %s -> %s", entry->attr->value->type->type, entry->type->type);
                ErrorTablePush(errtbl, msg);
                DelAttr(entry->attr);
                entry->attr = NULL;
                reval = 1;
            }
        }
        else {
            if(strcmp(entry->attr->value->type->type, entry->type->type) != 0){
                memset(msg, 0, sizeof(msg));
                snprintf(msg, sizeof(msg), "Invalid initial value type coercion: %s -> %s", entry->attr->value->type->type, entry->type->type);
                ErrorTablePush(errtbl, msg);
                DelAttr(entry->attr);
                entry->attr = NULL;
                reval = 1;
            }
        }
    }
    else if(entry->type->array!=NULL && entry->attr!=NULL && entry->attr->varray==NULL){ 
        int array_size, i;
        struct Arraynode *ptr;
        for(ptr=entry->type->array, array_size=1; ptr!=NULL; ptr=ptr->next){
            array_size*=(ptr->dimension);
        }
        if(array_size <= 0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "The index must be greater than zero in array declarations");
            ErrorTablePush(errtbl, msg);
            return 1;
        }
    }
    else if(entry->type->array!=NULL && entry->attr!=NULL && entry->attr->varray!=NULL){ /* if is array */
        int array_size, i;
        struct Arraynode *ptr;
        for(ptr=entry->type->array, array_size=1; ptr!=NULL; ptr=ptr->next){
            array_size*=(ptr->dimension);
        }
        if(array_size <= 0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "The index must be greater than zero in array declarations");
            ErrorTablePush(errtbl, msg);
            return 1;
        }
        if(entry->attr->varray->size > array_size){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Too many initializers");
            ErrorTablePush(errtbl, msg);
            reval = 1;
            for(i=array_size; i<entry->attr->varray->size; i++){
                DelValue(entry->attr->varray->vVector[i]);
            }
            entry->attr->varray->size = array_size;
        }
        for(i=entry->attr->varray->size; i<array_size; i++){
            if(strcmp(entry->type->type, "double")==0 || strcmp(entry->type->type, "float")==0)
                ValueArrayPush(entry->attr->varray, BuildValue(CopyType(entry->type), "0.0"));
            else if(strcmp(entry->type->type, "int")==0)
                ValueArrayPush(entry->attr->varray, BuildValue(CopyType(entry->type), "0"));
            else if(strcmp(entry->type->type, "bool")==0 || strcmp(entry->type->type, "boolean")==0)
                ValueArrayPush(entry->attr->varray, BuildValue(CopyType(entry->type), "false"));
            else
                ValueArrayPush(entry->attr->varray, BuildValue(CopyType(entry->type), ""));
        }
        
        for(i=0; i<array_size; i++){ /* Deal with each index */
            if(strcmp(entry->type->type, "double") == 0){
                if(entry->attr->varray->vVector[i] == NULL)
                    entry->attr->varray->vVector[i] = BuildValue(BuildType("double", NULL), "0.0");
                else if(strcmp(entry->attr->varray->vVector[i]->type->type, "double") == 0)
                    ;
                else if(strcmp(entry->attr->varray->vVector[i]->type->type, "float") == 0)
                    strcpy(entry->attr->varray->vVector[i]->type->type, "double");
                else if(strcmp(entry->attr->varray->vVector[i]->type->type, "int") == 0){
                    strcpy(entry->attr->varray->vVector[i]->type->type, "double");
                    entry->attr->varray->vVector[i]->dval = (double)entry->attr->varray->vVector[i]->ival;
                }
                else {
                    memset(msg, 0, sizeof(msg));
                    snprintf(msg, sizeof(msg), "Invalid initial value type coercion: %s -> %s", entry->attr->varray->vVector[i]->type->type, entry->type->type);
                    ErrorTablePush(errtbl, msg);
                    DelValue(entry->attr->varray->vVector[i]);
                    if(strcmp(entry->type->type, "double")==0 || strcmp(entry->type->type, "float")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0.0");
                    else if(strcmp(entry->type->type, "int")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0");
                    else if(strcmp(entry->type->type, "bool")==0 || strcmp(entry->type->type, "boolean")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "false");
                    else
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "");
                    reval = 1;
                }
            }
            else if(strcmp(entry->type->type, "float") == 0){
                if(entry->attr->varray->vVector[i] == NULL)
                    entry->attr->varray->vVector[i] = BuildValue(BuildType("float", NULL), "0.0");
                else if(strcmp(entry->attr->varray->vVector[i]->type->type, "float") == 0)
                    ;
                else if(strcmp(entry->attr->varray->vVector[i]->type->type, "int") == 0){
                    strcpy(entry->attr->varray->vVector[i]->type->type, "float");
                    entry->attr->varray->vVector[i]->dval = (double)entry->attr->varray->vVector[i]->ival;
                }
                else {
                    memset(msg, 0, sizeof(msg));
                    snprintf(msg, sizeof(msg), "Invalid initial value type coercion: %s -> %s", entry->attr->varray->vVector[i]->type->type, entry->type->type);
                    ErrorTablePush(errtbl, msg);
                    DelValue(entry->attr->varray->vVector[i]);
                    if(strcmp(entry->type->type, "double")==0 || strcmp(entry->type->type, "float")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0.0");
                    else if(strcmp(entry->type->type, "int")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0");
                    else if(strcmp(entry->type->type, "bool")==0 || strcmp(entry->type->type, "boolean")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "false");
                    else
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "");
                    reval = 1;
                }
            }
            else {
                if(entry->attr->varray->vVector[i] == NULL){
                    if(strcmp(entry->type->type, "double") == 0 || strcmp(entry->type->type, "float") == 0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0.0");
                    else if(strcmp(entry->type->type, "int") == 0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0");
                    else if(strcmp(entry->type->type, "bool") == 0 || strcmp(entry->type->type, "boolean") == 0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "false");
                    else if(strcmp(entry->type->type, "string") == 0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "");
                }
                else if(strcmp(entry->attr->varray->vVector[i]->type->type, entry->type->type) != 0){
                    memset(msg, 0, sizeof(msg));
                    snprintf(msg, sizeof(msg), "Invalid initial value type coercion: %s -> %s", entry->attr->varray->vVector[i]->type->type, entry->type->type);
                    ErrorTablePush(errtbl, msg);
                    DelValue(entry->attr->varray->vVector[i]);
                    if(strcmp(entry->type->type, "double")==0 || strcmp(entry->type->type, "float")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0.0");
                    else if(strcmp(entry->type->type, "int")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "0");
                    else if(strcmp(entry->type->type, "bool")==0 || strcmp(entry->type->type, "boolean")==0)
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "false");
                    else
                        entry->attr->varray->vVector[i] = BuildValue(CopyType(entry->type), "");
                    reval = 1;
                }
            }
        } 
    }

    return reval;
}

int ConstDeclCheck(struct SymbolTable* tbl, struct Entry* entry, struct ErrorTable* errtbl){
    if(tbl==NULL || entry==NULL || errtbl==NULL)
        return -1;

    char msg[1024];
    int reval;
    struct Entry *founded;
    if((founded = SymbolTableFind(tbl, entry->name)) != NULL){
        if(founded->level == entry->level){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Identifier '%s' has been declared in this scope", entry->name);
            ErrorTablePush(errtbl, msg);
            reval = 1;
        }
    }
    
    if(strcmp(entry->type->type, "void") == 0){
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Value type should not be 'void'");
        ErrorTablePush(errtbl, msg);
        reval = 1;
    }

    if(entry->type->array==NULL && entry->attr!=NULL && entry->attr->value!=NULL){
        if(strcmp(entry->type->type, "double") == 0){
            if(strcmp(entry->attr->value->type->type, "double") == 0)
                ;
            else if(strcmp(entry->attr->value->type->type, "float") == 0)
                strcpy(entry->attr->value->type->type, "double");
            else if(strcmp(entry->attr->value->type->type, "int") == 0){
                strcpy(entry->attr->value->type->type, "double");
                entry->attr->value->dval = (double)entry->attr->value->dval;
            }
            else {
                memset(msg, 0, sizeof(msg));
                snprintf(msg, sizeof(msg), "Invalid initial value type: %s", entry->attr->value->type->type);
                ErrorTablePush(errtbl, msg);
                DelAttr(entry->attr);
                entry->attr = NULL;
                reval = 1;
            }
        }
        else if(strcmp(entry->type->type, "float") == 0){
            if(strcmp(entry->attr->value->type->type, "float") == 0)
                ;
            else if(strcmp(entry->attr->value->type->type, "int") == 0){
                strcpy(entry->attr->value->type->type, "float");
                entry->attr->value->dval = (double)entry->attr->value->dval;
            }
            else {
                memset(msg, 0, sizeof(msg));
                snprintf(msg, sizeof(msg), "Invalid initial value type: %s", entry->attr->value->type->type);
                ErrorTablePush(errtbl, msg);
                DelAttr(entry->attr);
                entry->attr = NULL;
                reval = 1;
            }
        }
        else {
            if(strcmp(entry->attr->value->type->type, entry->type->type) != 0){
                memset(msg, 0, sizeof(msg));
                snprintf(msg, sizeof(msg), "Invalid initial value type: %s", entry->attr->value->type->type);
                ErrorTablePush(errtbl, msg);
                DelAttr(entry->attr);
                entry->attr = NULL;
                reval = 1;
            }
        }
    }

    return reval;
}

int FuncDeclCheck(struct SymbolTable* tbl, struct Entry* entry, struct ErrorTable* errtbl){
    if(tbl==NULL || entry==NULL || errtbl==NULL)
        return -1;

    char msg[1024];
    if(SymbolTableFind(tbl, entry->name) != NULL){
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Identifier '%s' has been declared", entry->name);
        ErrorTablePush(errtbl, msg);
        return 1;
    }
}
int FuncDefCheck(struct SymbolTable* tbl, struct Entry* entry, struct ErrorTable* errtbl){ 
    if(tbl==NULL || entry==NULL || errtbl==NULL)
        return -1;

    char msg[1024];
    struct Entry *founded;
    int reval = 0;
    if((founded = SymbolTableFind(tbl, entry->name)) != NULL){
        reval = 1;  /* 1: No need to push this function into symboltable */
        if(CmpType(entry->type, founded->type) != 0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Function type different from declaration");
            ErrorTablePush(errtbl, msg);
                /* 
            if(strcmp(founded->kind, "function") == 0){
                DelType(founded->type);
                founded->type = CopyType(entry->type);
            }
                */
            reval = 2;
        }
        if(CmpArgu(entry->attr->argu, founded->attr->argu) != 0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Function argument mismatch");
            ErrorTablePush(errtbl, msg);
                /*
            if(strcmp(founded->kind, "function") == 0){
                DelArgu(&(founded->attr->argu));
                founded->attr->argu = CopyArgu(entry->attr->argu);
            }
                */
            reval = 2;
        }
        if(founded->decl != 0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Redefinition of '%s'", founded->name);
            ErrorTablePush(errtbl, msg);
            reval = 2;
        }
    }
    return reval;
}
   
void ReturnStatementCheck(struct ErrorTable* errtbl, struct Entry* entry, int* return_s){
    if(errtbl==NULL || entry==NULL)
        return;
    if(*return_s != 1 && strcmp(entry->type->type, "void")!=0){
        char msg[1024];
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "The last statement must be a return statement");
        ErrorTablePush(errtbl, msg);
    }
    *return_s = 0;
}

void ReturnTypeCheck(struct ErrorTable* errtbl, struct Entry* entry, struct Value* value){
    if(errtbl==NULL || entry==NULL || value==NULL)
        return;
    
    char msg[1024];
    if(value->type->array != NULL){
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Return type must be scalar type");
        ErrorTablePush(errtbl, msg);
        return;
    }

    if(strcmp(entry->type->type, "double")==0){
        if(strcmp(value->type->type, "double")!=0 && strcmp(value->type->type, "float")!=0 && strcmp(value->type->type, "int")!=0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Return type coercion error: %s -> %s", value->type->type, entry->type->type);
            ErrorTablePush(errtbl, msg);
            return;
        }
    }
    else if(strcmp(entry->type->type, "float")==0){
        if(strcmp(value->type->type, "float")!=0 && strcmp(value->type->type, "int")!=0){
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Return type coercion error: %s -> %s", value->type->type, entry->type->type);
            ErrorTablePush(errtbl, msg);
            return;
        }
    }
    else if(strcmp(entry->type->type, value->type->type)!=0){
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg), "Return type coercion error: %s -> %s", value->type->type, entry->type->type);
        ErrorTablePush(errtbl, msg);
        return;
    }
}

void BoolexprCheck(struct ErrorTable* errtbl, struct Value* expr, const char* use){
    if(errtbl==NULL || expr==NULL)
        return;

    if(expr->type->array!=NULL || strcmp(expr->type->type, "bool")!=0){
        char msg[1024];
        memset(msg, 0, sizeof(msg));
        if(strcmp(use, "for")==0)
            snprintf(msg, sizeof(msg), "Control expression in %s statement must be boolean type", use);
        else
            snprintf(msg, sizeof(msg), "Condition expression in %s statement must be boolean type", use);
        ErrorTablePush(errtbl, msg);
    }
}

struct Entry* BuildEntry(const char* name, const char* kind, int level, struct Type* type, struct Attribute* attr){
    struct Entry *alice = (struct Entry*)malloc(sizeof(struct Entry));
    snprintf(alice->name, sizeof(alice->name), "%s", name);
    snprintf(alice->kind, sizeof(alice->kind), "%s", kind);
    alice->level = level;
    alice->decl = 0;
    alice->type = type;
    alice->attr = (strcmp(kind, "function")!=0 && attr==NULL)? BuildAttr(NULL, BuildDefaultValue(CopyType(type))) : attr;
    return alice;
}

void AssignEntry(struct Entry* alice, const char* name, const char* kind, int level, struct Type* type, struct Attribute* attr){
    if(alice == NULL)
        return;
    if(name != NULL){
        memset(alice->name, 0, sizeof(alice->name));
        snprintf(alice->name, sizeof(alice->name), "%s", name);
    }
    if(kind != NULL)
        strcpy(alice->kind, kind);
    if(level >= 0)
        alice->level = level;
    if(type != NULL){
        DelType(alice->type);
        alice->type = type;
    }
    if(attr != NULL){
        DelAttr(alice->attr);
        alice->attr = attr;
    }
    else if(type != NULL && strcmp(kind, "function")!=0){
        alice->attr = BuildAttr(NULL, BuildDefaultValue(CopyType(type)));
    }
}

struct Entry* CopyEntry(const struct Entry* rhs){
    if(rhs == NULL)
        return NULL;
    struct Entry *alice = (struct Entry*)malloc(sizeof(struct Entry));
    strcpy(alice->name, rhs->name);
    strcpy(alice->kind, rhs->kind);
    alice->level = rhs->level;
    alice->type = CopyType(rhs->type);
    alice->attr = CopyAttr(rhs->attr);
    return alice;   
}

struct Entry* ClearEntry(struct Entry* alice){
    strcpy(alice->name, "");
    strcpy(alice->kind, "");
    alice->level = 0;
    DelAttr(alice->attr);
    alice->attr = NULL;
}

struct Type* BuildType(const char* type, struct Arraynode* array){
    struct Type *alice = (struct Type*)malloc(sizeof(struct Type));
    strcpy(alice->type, type);
    alice->array = array;
    return alice;
}

struct Type* CopyType(const struct Type* rhs){
    if(rhs == NULL)
        return NULL;
    struct Type *alice = (struct Type*)malloc(sizeof(struct Type));
    strcpy(alice->type, rhs->type);
    alice->array = NULL;
    struct Arraynode *ptr;
    for(ptr=rhs->array; ptr!=NULL; ptr=ptr->next){
        AddDimen(&(alice->array), ptr->dimension);
    }    
    return alice;
}

struct Type* ReduceTypeArray(struct Type* alice, int times, struct ErrorTable* errtbl){
    if(alice == NULL)
        return NULL;
    int i;
    for(i=0; i<times; i++){
        if(alice->array == NULL){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Invalid array access");
            ErrorTablePush(errtbl, msg);
            return NULL;
        }
        struct Arraynode *RMarraynode = alice->array;
        alice->array = alice->array->next;
        free(RMarraynode);
    }

    return alice;
}

struct Arraynode* CopyArray(struct Arraynode* alice){
    struct Arraynode *bob = NULL, *ptr;
    for(ptr=alice; ptr!=NULL; ptr=ptr->next){
        AddDimen(&bob, ptr->dimension);
    }    
    return bob;
}

void AddDimen(struct Arraynode** alice, int dimen){
    struct Arraynode *bob = (struct Arraynode*)malloc(sizeof(struct Arraynode));
    bob->dimension = dimen;
    bob->next = NULL;
    if(*alice == NULL){
        *alice = bob;
    }
    else {
        struct Arraynode *ptr=*alice, *ptr_p=NULL;
        while(ptr!=NULL){
            ptr_p = ptr;
            ptr = ptr->next;
        }
        ptr_p->next = bob;
    }
}

struct Attribute* BuildAttr(struct Argu* argu, struct Value *val){
    struct Attribute *alice = (struct Attribute*)malloc(sizeof(struct Attribute));
    alice->argu = argu;
    alice->value = val;
    alice->varray = NULL;
    return alice;
}

struct Attribute* CopyAttr(const struct Attribute* rhs){
    if(rhs == NULL)
        return NULL;

    struct Attribute *alice = (struct Attribute*)malloc(sizeof(struct Attribute));
    alice->argu = CopyArgu(rhs->argu);
    alice->value = CopyValue(rhs->value);
    alice->varray = CopyValueArray(rhs->varray);
    return alice;
}

struct Argu* BuildArgu(struct Type* type){
    if(type == NULL)
        return NULL;
    struct Argu *alice = (struct Argu*)malloc(sizeof(struct Argu));
    alice->type = type;
    alice->next = NULL;
    return alice;
}

struct Argu* CopyArgu(struct Argu* rhs){
    if(rhs == NULL)
        return NULL;
    struct Argu *alice = (struct Argu*)malloc(sizeof(struct Argu)), *ptr;
    memset(alice, 0, sizeof(struct Argu));
    strcpy(alice->name, rhs->name);
    alice->type = CopyType(rhs->type);
    alice->next = NULL;

    for(ptr=rhs->next; ptr!=NULL; ptr=ptr->next){
        AddArgu(&alice, ptr->name, ptr->type);
    }    
    return alice;
}

void AddArgu(struct Argu** alice, const char* name, struct Type* type){
    struct Argu *bob = (struct Argu*)malloc(sizeof(struct Argu));
    memset(bob, 0, sizeof(struct Argu));
    snprintf(bob->name, sizeof(bob->name), "%s", name);
    bob->type = type;
    bob->next = NULL;
    if(*alice == NULL){
        *alice = bob;
    }
    else {
        struct Argu *ptr=*alice, *ptr_p=NULL;
        while(ptr!=NULL){
            ptr_p = ptr;
            ptr = ptr->next;
        }
        ptr_p->next = bob;
    }
}

struct Value* BuildValue(struct Type* type_, const char* val){
    struct Value *alice = (struct Value*)malloc(sizeof(struct Value));
    alice->type = type_;
    if(strcmp(alice->type->type, "float") == 0 || strcmp(alice->type->type, "double") == 0){
        alice->dval = atof(val);
    }
    else if(strcmp(alice->type->type, "int") == 0){
        alice->ival = atoi(val);
    }
    else if(strcmp(alice->type->type, "bool") == 0 || strcmp(alice->type->type, "boolean") == 0){
        if(strcmp(val, "true") == 0)
            alice->ival = 1;
        else if(strcmp(val, "false") == 0)
            alice->ival = 0;
    }
    else if(strcmp(alice->type->type, "string") == 0){
        memset(alice->strV, 0, sizeof(alice->strV));
        snprintf(alice->strV, sizeof(alice->strV), "%s", val);
    }
    return alice;
}

struct Value* BuildDefaultValue(struct Type* type){
    if(type == NULL)
        return NULL;

    struct Value *returnvalue = NULL;
    if(strcmp(type->type, "float") == 0 || strcmp(type->type, "double") == 0)
        returnvalue = BuildValue(type, "0.0");
    else if(strcmp(type->type, "int") == 0)
        returnvalue = BuildValue(type, "0");
    else if(strcmp(type->type, "bool") == 0 || strcmp(type->type, "boolean") == 0)
        returnvalue = BuildValue(type, "false");
    else if(strcmp(type->type, "string") == 0)
        returnvalue = BuildValue(type, "");

    return returnvalue; 
}

struct Value* CopyValue(struct Value* rhs){
    if(rhs == NULL)
        return NULL;
    struct Value *alice = (struct Value*)malloc(sizeof(struct Value));
    alice->type = CopyType(rhs->type);
    strcpy(alice->strV, rhs->strV);
    alice->dval = rhs->dval;
    alice->ival = rhs->ival;

    return alice;
}

void InitialValue(struct Entry* alice, struct Value* val){
    if(alice==NULL || val==NULL)
        return;

    DelAttr(alice->attr);
    alice->attr= BuildAttr(NULL, val);
}

void Assignment(struct Entry* entry, struct Value* value, struct ErrorTable* errtbl){
    if(entry==NULL || value==NULL)
        return;
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(strcmp(entry->kind, "function") == 0){
        snprintf(msg, sizeof(msg), "Cannot assign to a function: '%s'", entry->name);
        ErrorTablePush(errtbl, msg);
        return;
    }
    if(strcmp(entry->kind, "constant") == 0){
        snprintf(msg, sizeof(msg), "Cannot assign to a constant: '%s'", entry->name);
        ErrorTablePush(errtbl, msg);
        return;
    }
    if(entry->type->array!=NULL || value->type->array!=NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer assignment");
        ErrorTablePush(errtbl, msg);
        return;
    }
    if(strcmp(entry->type->type, "double")==0){
        if(strcmp(value->type->type, "double")!=0 && strcmp(value->type->type, "float")!=0 && strcmp(value->type->type, "int")!=0){
            snprintf(msg, sizeof(msg), "Invalid type coercion: %s -> %s", value->type->type, entry->type->type);
            ErrorTablePush(errtbl, msg);
            return;
        }
    }
    else if(strcmp(entry->type->type, "float")==0){
        if(strcmp(value->type->type, "float")!=0 && strcmp(value->type->type, "int")!=0){
            snprintf(msg, sizeof(msg), "Invalid type coercion: %s -> %s", value->type->type, entry->type->type);
            ErrorTablePush(errtbl, msg);
            return;
        }
    }
    else if(strcmp(entry->type->type, value->type->type)!=0){
        snprintf(msg, sizeof(msg), "Invalid type coercion: %s -> %s", value->type->type, entry->type->type);
        ErrorTablePush(errtbl, msg);
        return;
    }
}

struct ValueArray* BuildValueArray(){
    struct ValueArray *Alice = (struct ValueArray*)malloc(sizeof(struct ValueArray));
    Alice->size = 0;
    Alice->capacity = 8;
    Alice->vVector = (struct Value**)malloc(sizeof(struct Value*) * 8);
    return Alice;
}

struct ValueArray* CopyValueArray(struct ValueArray* rhs){
    if(rhs == NULL)
        return NULL;
    struct ValueArray *Alice = (struct ValueArray*)malloc(sizeof(struct ValueArray));
    Alice->size = rhs->size;
    Alice->capacity = rhs->capacity;
    Alice->vVector = (struct Value**)malloc(sizeof(struct Value*) * Alice->capacity);
    int i, size = rhs->size;
    for(i=0; i<size; i++){
        Alice->vVector[i] = CopyValue(rhs->vVector[i]);
    }
    return Alice;
}

void ValueArrayPush(struct ValueArray* Alice, struct Value* Bob){
    if(Alice == NULL)
        return;
    if(Alice->size == Alice->capacity){
        Alice->capacity *= 2;
        struct Value **RMvVector = Alice->vVector;
        Alice->vVector = (struct Value**)malloc(sizeof(struct Value*) * Alice->capacity);
        int i;
        for(i=0; i<Alice->size; i++){
            (Alice->vVector)[i] = RMvVector[i];
        }
        free(RMvVector);
    }
    
    Alice->vVector[Alice->size++] = Bob;
}

void AssignValueArray(struct Entry* entry, struct ValueArray* varray){
    if(entry == NULL || varray == NULL)
        return;
    if(entry->attr == NULL)
        entry->attr = BuildAttr(NULL, NULL);
    DelValue(entry->attr->value);
    entry->attr->value = NULL;
    DelValueArray(entry->attr->varray);
    entry->attr->varray = varray;
}

int CmpType(struct Type* t1, struct Type* t2){
    if(t1 == NULL || t2 == NULL)
        return -1;
    if(strcmp(t1->type, t2->type) != 0)
        return 1;

    struct Arraynode *ptr1=t1->array, *ptr2=t2->array;
    while(1){
        if(ptr1 == NULL && ptr2 == NULL)
            return 0;
        if(ptr1 == NULL && ptr2 != NULL)
            return 1;
        if(ptr1 != NULL && ptr2 == NULL)
            return 1;
        if(ptr1->dimension != ptr2->dimension)
            return 1;
        ptr1=ptr1->next;
        ptr2=ptr2->next;
    }
}

int CmpTypeCall(struct Type* t1, struct Type* t2){
    if(t1 == NULL || t2 == NULL)
        return -1;
    if(strcmp(t1->type, "double") == 0 && strcmp(t2->type, "double") != 0 && 
       strcmp(t2->type, "float") != 0 && strcmp(t2->type, "int") != 0)
        return 1;
    if(strcmp(t1->type, "float") == 0 && strcmp(t2->type, "float") != 0 && strcmp(t2->type, "int") != 0)
        return 1;
    if(strcmp(t1->type, "double")!=0 && strcmp(t1->type, "float")!=0 && strcmp(t1->type, t2->type) != 0)
        return 1;

    struct Arraynode *ptr1=t1->array, *ptr2=t2->array;
    while(1){
        if(ptr1 == NULL && ptr2 == NULL)
            return 0;
        if(ptr1 == NULL && ptr2 != NULL)
            return 1;
        if(ptr1 != NULL && ptr2 == NULL)
            return 1;
        if(ptr1->dimension != ptr2->dimension)
            return 1;
        ptr1=ptr1->next;
        ptr2=ptr2->next;
    }
}

int CmpArgu(struct Argu* a1, struct Argu* a2){
    struct Argu *ptr1=a1, *ptr2=a2;
    while(1){
        if(ptr1 == NULL && ptr2 == NULL)
            return 0;
        if(ptr1 == NULL && ptr2 != NULL)
            return 1;
        if(ptr1 != NULL && ptr2 == NULL)
            return 1;
        if(CmpType(ptr1->type, ptr2->type) != 0)
            return 1;

        ptr1=ptr1->next;
        ptr2=ptr2->next;
    }
}

int CmpArguCall(struct Argu* a1, struct Argu* a2){
    struct Argu *ptr1=a1, *ptr2=a2;
    while(1){
        if(ptr1 == NULL && ptr2 == NULL){
            return 0;
        }
        if(ptr1 == NULL && ptr2 != NULL){
            return 1;
        }    
        if(ptr1 != NULL && ptr2 == NULL){
            return 1;
        }
        if(CmpTypeCall(ptr1->type, ptr2->type) != 0){
            return 1;
        }
        
        ptr1=ptr1->next;
        ptr2=ptr2->next;
    }
}

void DelSymbolTable(struct SymbolTable* Alice){
    if(Alice == NULL)
        return;
    int i, size = Alice->size;
    for(i=0; i<size; i++){
        DelEntry(Alice->entryVector[i]);
    }
    free(Alice->entryVector);
}
void DelErrorTable(struct ErrorTable* Alice){
    if(Alice == NULL)
        return;
    int i, size = Alice->size;
    for(i=0; i<size; i++){
        free(Alice->errorVector[i]);
    }
    free(Alice->errorVector);
}

void DelEntry(struct Entry* alice){
    if(alice!=NULL){
        DelType(alice->type);
        DelAttr(alice->attr);
        free(alice);
    }
}

void DelType(struct Type* alice){
    if(alice!=NULL){
        struct Arraynode *ptr=alice->array, *rmptr=NULL;
        while(ptr!=NULL){
            rmptr = ptr;
            ptr = ptr->next;
            free(rmptr);
        }
        free(alice);
    }
}

void DelAttr(struct Attribute* alice){
    if(alice!=NULL){
        DelArgu(&(alice->argu));
        DelValue(alice->value);
        DelValueArray(alice->varray);
        free(alice);
    }
}

void DelValue(struct Value* alice){
    if(alice!=NULL){
        DelType(alice->type);
        free(alice);
    }
}

void DelValueArray(struct ValueArray* Alice){
    if(Alice == NULL)
        return;
    int i, size = Alice->size;
    for(i=0; i<size; i++){
        DelValue(Alice->vVector[i]);
    }
    free(Alice->vVector);
}

void DelArray(struct Arraynode** alice){
    struct Arraynode *ptr=*alice, *rmptr=NULL;
    while(ptr!=NULL){
        rmptr = ptr;
        ptr = ptr->next;
        free(rmptr);
    }
    *alice = NULL;
}

void DelArgu(struct Argu** alice){
    struct Argu *ptr=*alice, *rmptr=NULL;
    while(ptr!=NULL){
        rmptr = ptr;
        ptr = ptr->next;
        free(rmptr);
    }
    *alice = NULL;
}

struct Value* Expr_neg(struct Value* value, struct ErrorTable* errtbl){
    if(value == NULL)
        return NULL;

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(value->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(value);
        return NULL;
    }
        
    if(strcmp(value->type->type, "string")==0 || strcmp(value->type->type, "bool")==0 || strcmp(value->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '-'");
        ErrorTablePush(errtbl, msg);
    }
    else if(strcmp(value->type->type, "float")==0 || strcmp(value->type->type, "double")==0){
        value->dval *= -1;
    }
    else if(strcmp(value->type->type, "int")==0){
        value->ival *= -1;
    }
    return value;
}

struct Value* Expr_exclam(struct Value* value, struct ErrorTable* errtbl){
    if(value == NULL)
        return NULL;

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(value->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(value);
        return NULL;
    }
        
    if(strcmp(value->type->type, "bool")!=0 && strcmp(value->type->type, "boolean")!=0){
        snprintf(msg, sizeof(msg), "Invalid operation with '!'");
        ErrorTablePush(errtbl, msg);
    }
    value->ival = (value->ival==0)? 1: 0;

    return value;
}

struct Value* Expr_plus(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '+'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
    if(strcmp(v1->type->type, "double")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->dval += (double)v2->ival;
        }
        else {
            v1->dval += v2->dval;
        }
    }
    else if(strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->dval += (double)v2->ival;
        }
        else {
            v1->dval += v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->ival += v2->ival;
        }
        else {
            v1->dval = (double)v1->ival + v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    //DelValue(v2);

    return v1;
}

struct Value* Expr_minus(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '+'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
    if(strcmp(v1->type->type, "double")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->dval -= (double)v2->ival;
        }
        else {
            v1->dval -= v2->dval;
        }
    }
    else if(strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->dval -= (double)v2->ival;
        }
        else {
            v1->dval -= v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->ival -= v2->ival;
        }
        else {
            v1->dval = (double)v1->ival - v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    //DelValue(v2);

    return v1;
}
struct Value* Expr_mul(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '*'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
    if(strcmp(v1->type->type, "double")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->dval *= (double)v2->ival;
        }
        else {
            v1->dval *= v2->dval;
        }
    }
    else if(strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->dval *= (double)v2->ival;
        }
        else {
            v1->dval *= v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            v1->ival *= v2->ival;
        }
        else {
            v1->dval = (double)v1->ival * v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    //DelValue(v2);

    return v1;
}

struct Value* Expr_div(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){  /* Need to avoid zero? */
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '/'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "double")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v2->ival != 0)
                v1->dval /= (double)v2->ival;
        }
        else {
            if(v2->dval != 0.0)
                v1->dval /= v2->dval;
        }
    }
    else if(strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v2->ival != 0)
                v1->dval /= (double)v2->ival;
        }
        else {
            if(v2->dval != 0.0)
                v1->dval /= v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v2->ival != 0)
                v1->ival /= v2->ival;
        }
        else {
            if(v2->dval != 0.0)
                v1->dval = (double)v1->ival / v2->dval;
            strcpy(v1->type->type, v2->type->type);
        }
    }
    //DelValue(v2);

    return v1;
}

struct Value* Expr_mod(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "int")!=0 || strcmp(v1->type->type, "int")!=0){
        snprintf(msg, sizeof(msg), "Invalid operation with '%%'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
    
    if(v2->ival != 0)
        v1->ival %= v2->ival;
    //DelValue(v2);

    return v1;
}

struct Value* Expr_and(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if((strcmp(v1->type->type, "bool")!=0 && strcmp(v1->type->type, "boolean")!=0)){
        snprintf(msg, sizeof(msg), "Invalid operation with '&&'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
    
    v1->ival = v1->ival & v2->ival;
    //DelValue(v2);

    return v1;
}

struct Value* Expr_or(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if((strcmp(v1->type->type, "bool")!=0 && strcmp(v1->type->type, "boolean")!=0)){
        snprintf(msg, sizeof(msg), "Invalid operation with '||'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
    
    v1->ival = v1->ival | v2->ival;
    //DelValue(v2);

    return v1;
}

struct Value* Expr_lt(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '<'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    if(strcmp(v1->type->type, "double")==0 || strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->dval < v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->dval < v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->ival < v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->ival < v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    //DelValue(v2);
    strcpy(v1->type->type, "bool");

    return v1;
}

struct Value* Expr_gt(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '>'");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    if(strcmp(v1->type->type, "double")==0 || strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->dval > v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->dval > v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->ival > v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->ival > v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    //DelValue(v2);
    strcpy(v1->type->type, "bool");

    return v1;
}
struct Value* Expr_le(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '<='");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    if(strcmp(v1->type->type, "double")==0 || strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->dval <= v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->dval <= v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->ival <= v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->ival <= v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    //DelValue(v2);
    strcpy(v1->type->type, "bool");

    return v1;
}
struct Value* Expr_ge(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '>='");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    if(strcmp(v1->type->type, "double")==0 || strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->dval >= v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->dval >= v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->ival >= v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->ival >= v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    //DelValue(v2);
    strcpy(v1->type->type, "bool");

    return v1;
}
struct Value* Expr_eq(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '=='");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    if(strcmp(v1->type->type, "double")==0 || strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->dval == v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->dval == v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->ival == v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->ival == v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    //DelValue(v2);
    strcpy(v1->type->type, "bool");

    return v1;
}
struct Value* Expr_ne(struct Value* v1, struct Value* v2, struct ErrorTable* errtbl){
    if(v1 == NULL || v2 == NULL){
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    char msg[1024];
    memset(msg, 0, sizeof(msg));
    if(v1->type->array != NULL || v2->type->array != NULL){
        snprintf(msg, sizeof(msg), "Invalid pointer operation");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }
        
    if(strcmp(v1->type->type, "string")==0 || strcmp(v1->type->type, "bool")==0 || strcmp(v1->type->type, "boolean")==0 ||
       strcmp(v2->type->type, "string")==0 || strcmp(v2->type->type, "bool")==0 || strcmp(v2->type->type, "boolean")==0){
        snprintf(msg, sizeof(msg), "Invalid operation with '!='");
        ErrorTablePush(errtbl, msg);
        DelValue(v1);
        DelValue(v2);
        return NULL;
    }

    if(strcmp(v1->type->type, "double")==0 || strcmp(v1->type->type, "float")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->dval != v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->dval != v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    else if(strcmp(v1->type->type, "int")==0){
        if(strcmp(v2->type->type, "int") == 0){
            if(v1->ival != v2->ival)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
        else {
            if(v1->ival != v2->dval)
                v1->ival = 1;
            else 
                v1->ival = 0;
        }
    }
    //DelValue(v2);
    strcpy(v1->type->type, "bool");

    return v1;
}
