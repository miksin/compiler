#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "gen.h"
#include "symtbl.h"

extern FILE *outfp; /* declare in parser.y */
extern int linenum; /* declared in lex.l */

void GenTitle(){
    fprintf(outfp, "; alice.j\n");
    fprintf(outfp, ".class public alice\n");
    fprintf(outfp, ".super java/lang/Object\n");
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
