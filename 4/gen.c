#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "gen.h"
#include "symtbl.h"
#define GLOBAL 0

extern FILE *outfp; /* declare in parser.y */
extern int linenum; /* declared in lex.l */
extern int seq;     /* declared in parser.y */

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
    fprintf(outfp, ".super java/lang/Object\n\n");
}

void GenVariableDecl(void* alice){
    struct Entry *entry = (struct Entry*)alice;
    char *id = entry->name;
    char *typename = entry->type->type;
    char typedesc[2];
    typedesc[1] = '\0';

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

        Gen(5, ".field", "public", "static", id, typedesc);
    } 
    else {
        entry->reg = seq++;
    }
}

void GenFunction(void* alice){
    struct Entry *entry = (struct Entry*)alice;
    struct Argu *ptr;
    char *id = entry->name;
    char *typename = entry->type->type;
    char argulist[200];
    char typedesc[2];
    typedesc[1] = '\0';
    
    if(strcmp(id, "main") == 0){
        Gen(4, ".method", "public", "static", "main([Ljava/lang/String;)V");
        Gen(3, ".limit", "stack", "128");
        Gen(3, ".limit", "local", "128");
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

    Gen(4, ".method", "public", "static", argulist);
    Gen(3, ".limit", "stack", "128");
    Gen(3, ".limit", "local", "128");
}
