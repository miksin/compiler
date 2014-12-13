#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtbl.h"

void SymbolTableBuild(struct SymbolTable* Alice){
    Alice = (struct SymbolTable*)malloc(sizeof(struct SymbolTable));
    Alice->nowlevel = 0;
    Alice->size = 0;
    Alice->capacity = 8;
    Alice->entryVector = (struct Entry**)malloc(sizeof(struct Entry*) * 8);
}

void SymbolTablePrint(struct SymbolTable* Alice){
    int i;
    printf("Name\tKind\tLevel\tType\tAttribute\n");
    for(i=0; i<Alice->size; i++){
        printf("%s\n");
    }
}

void SymbolTablePush(struct SymbolTable* Alice, struct Entry* Bob){
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

struct Entry* BuildEntry(const char* name, const char* kind, int level, struct Type* type, struct Attribute* attr){
    struct Entry *alice = (struct Entry*)malloc(sizeof(struct Entry));
    strcpy(alice->name, name);
    strcpy(alice->kind, kind);
    alice->level = level;
    alice->type = type;
    alice->attr = attr;
    return alice;
}

struct Entry* CopyEntry(const struct Entry* entry){
    struct Entry *alice = (struct Entry*)malloc(sizeof(struct Entry));
    strcpy(alice->name, entry->name);
    strcpy(alice->kind, entry->kind);
    alice->level = entry->level;
    //alice->type = CopyType(entry->type);
    //alice->attr = CopyAttr(entry->attr);
    return alice;   
}

