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
    printf("Name                             Kind       Level       Type               Attribute\n");
    for(i=0; i<Alice->size; i++){
        printf("%s %s %d %s\n", Alice->entryVector[i]->name, Alice->entryVector[i]->kind, Alice->entryVector[i]->level, Alice->entryVector[i]->type->type);
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

    Alice->entryVector[Alice->size++] = Bob; }

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

struct Entry* CopyEntry(const struct Entry* rhs){
    struct Entry *alice = (struct Entry*)malloc(sizeof(struct Entry));
    strcpy(alice->name, rhs->name);
    strcpy(alice->kind, rhs->kind);
    alice->level = rhs->level;
    alice->type = CopyType(rhs->type);
    alice->attr = CopyAttr(rhs->attr);
    return alice;   
}

struct Type* BuildType(const char* type, struct Arraynode* array){
    struct Type *alice = (struct Type*)malloc(sizeof(struct Type));
    strcpy(alice->type, type);
    alice->array = array;
    return alice;
}

struct Type* CopyType(const struct Type* rhs){
    struct Type *alice = (struct Type*)malloc(sizeof(struct Type));
    strcpy(alice->type, rhs->type);
    struct Arraynode *ptr;
    for(ptr=rhs->array; ptr!=NULL; ptr=ptr->next){
        TypeAddDimen(alice, ptr->dimension);
    }    
    return alice;
}

void TypeAddDimen(struct Type* alice, int dimen){
    struct Arraynode *bob = (struct Arraynode*)malloc(sizeof(struct Arraynode));
    bob->dimension = dimen;
    bob->next = NULL;
    if(alice->array == NULL){
        alice->array = bob;
    }
    else {
        struct Arraynode *ptr=alice->array, *ptr_p=NULL;
        while(ptr!=NULL){
            ptr_p = ptr;
            ptr = ptr->next;
        }
        ptr_p->next = bob;
    }
}

struct Attribute* BuildAttr(struct Argu* argu, int intV, double floV, const char* strV){
    struct Attribute *alice = (struct Attribute*)malloc(sizeof(struct Attribute));
    alice->argu = argu;
    alice->intV = intV;
    alice->floV = floV;
    strcpy(alice->strV,strV);
    return alice;
}

struct Attribute* CopyAttr(const struct Attribute* rhs){
    struct Attribute *alice = (struct Attribute*)malloc(sizeof(struct Attribute));
    alice->argu = CopyArgu(rhs->argu);
    alice->intV = rhs->intV;
    alice->floV = rhs->floV;
    strcpy(alice->strV, rhs->strV);
    return alice;
}

struct Argu* BuildArgu(const char* type, struct Arraynode* array){
    struct Argu *alice = (struct Argu*)malloc(sizeof(struct Argu));
    strcpy(alice->type, type);
    alice->array = array;
    alice->next = NULL;
    return alice;
}

struct Argu* CopyArgu(const struct Argu* rhs){
    struct Argu *alice = (struct Argu*)malloc(sizeof(struct Argu));
    strcpy(alice->type, rhs->type);
    struct Arraynode *ptr;
    for(ptr=rhs->array; ptr!=NULL; ptr=ptr->next){
        ArguAddDimen(alice, ptr->dimension);
    }    
    alice->next = rhs->next;
    return alice;
}

void ArguAddDimen(struct Argu* alice, int dimen){
    struct Arraynode *bob = (struct Arraynode*)malloc(sizeof(struct Arraynode));
    bob->dimension = dimen;
    bob->next = NULL;
    if(alice->array == NULL){
        alice->array = bob;
    }
    else {
        struct Arraynode *ptr=alice->array, *ptr_p=NULL;
        while(ptr!=NULL){
            ptr_p = ptr;
            ptr = ptr->next;
        }
        ptr_p->next = bob;
    }
}

void DelEntry(struct Entry* alice){
    DelType(alice->type);
    DelAttr(alice->attr);
    free(alice);
}

void DelType(struct Type* alice){
    struct Arraynode *ptr=alice->array, *rmptr=NULL;
    while(ptr!=NULL){
        rmptr = ptr;
        ptr = ptr->next;
        free(rmptr);
    }
    free(alice);
}

void DelAttr(struct Attribute* alice){
    DelArgu(alice->argu);
    free(alice);
}

void DelArgu(struct Argu* alice){
    struct Arraynode *ptr=alice->array, *rmptr=NULL;
    while(ptr!=NULL){
        rmptr = ptr;
        ptr = ptr->next;
        free(rmptr);
    }
    free(alice);
}
