%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtbl.h"
#include "gen.h"

extern int linenum;             /* declared in lex.l */
extern FILE *yyin;              /* declared by lex */
extern char *yytext;            /* declared by lex */
extern char buf[256];           /* declared in lex.l */
extern int Opt_Statistic;       /* declared in lex.l */
extern int Opt_Symbol;       /* declared in lex.l */
struct SymbolTable *Alice;
struct SymbolTable *Alice_buf;
struct ErrorTable *Error_msg;
struct Entry *entry_buf;
struct Type *type_buf;
struct Arraynode *array_buf;
struct Argu *argu_buf;
struct Entry *now_func;
int return_s;
int inloop;

FILE *outfp;

%}

%union {
    char *text;
    int ival;
    struct Type *type;
    struct Entry *entry;
    struct Argu *argu;
    struct Value *value;
    struct ValueArray *varray;
}

%type <text> identifier
%type <type> argu_type float_type bool_type
%type <value> value bool_value const_value
%type <value> expr var_init
%type <value> a_function initial_expr control_expr
%type <entry> function_decl
%type <argu> func_argu function_call
%type <varray> var_array_init more_expr;
%type <ival> array

%token SEMICOLON      /* ; */
%token COMMA          /* , */
%token UPPARE LOPARE  /* () */
%token UPBRAC  LOBRAC /* [] */
%token UPBRACE LOBRACE/* {} */
%token <text> ID             /* identifier */
%token WHILE FOR DO   /* keyword */
%token IF ELSE        /* keyword */
%token <text> TRUE FALSE     /* keyword */
%token PRINT          /* keyword */
%token CONST          /* keyword */
%token <text> READ           /* keyword */
%token <text> BOOL BOOLEAN   /* type */
%token <text> VOID           /* type */
%token <text> INT            /* type */
%token <text> FLOAT DOUBLE   /* type */
%token <text> STRING         /* type */
%token <text> CONTINUE BREAK /* keyword */
%token <text> RETURN         /* keyword */
%token ASSIGN         /* = */
%token EXCLAM         /* ! */
%token ANDAND OROR    /* && || */
%token LT GT LE GE    /* < > <= >= */
%token EQUAL NOTEQUAL /* == != */
%token PLUS MINUS     /* + - */
%token MUL DIV        /* * / */
%token MOD            /* % */
%token <text> INTEGER        /* value */
%token <text> FLOAT_NUM      /* value */
%token <text> ONESTRING      /* value */
%token <text> SCIENTIFIC     /* value */

%left OROR
%left ANDAND
%left EXCLAM
%left LT GT EQUAL LE GE NOTEQUAL
%left PLUS MINUS
%left MUL DIV MOD
%left HIGH_P

%%

program :
      { GenTitle(); }
      decl_only_list decl_and_def_list
    ;
decl_only_list :
      declaration_list decl_only_list
    | definition_list
    ;

decl_and_def_list :
      decl_and_def_list declaration_list 
    | decl_and_def_list definition_list
    |
    ;
declaration_list : 
      type var_decl SEMICOLON { 
        DelType(type_buf); 
        SymbolTablePush(Alice, Alice_buf);
      }
    | type function_decl SEMICOLON { 
        DelType(type_buf); 
        if(FuncDeclCheck(Alice, $2, Error_msg) == 0)
            SymbolTablePushOne(Alice, $2);
        else
            DelEntry($2);
      }
    | CONST type const_decl SEMICOLON { 
        DelType(type_buf); 
        SymbolTablePush(Alice, Alice_buf);
      }
    ;
definition_list :
      type function_decl UPBRACE { 
        int n;
        DelType(type_buf);
        if((n = FuncDefCheck(Alice, $2, Error_msg)) == 0){ 
            /* Need to push both func and argu */
            SymbolTablePushOne(Alice, $2);
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
            SymbolTablePushArgu(Alice, $2->attr->argu);
            now_func = $2;
            now_func->decl = 1;
        }
        else if(n == 1){  
            /* Need to push only argu */
            struct Entry *founded = SymbolTableFind(Alice, $2->name);
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
            SymbolTablePushArgu(Alice, $2->attr->argu);
            now_func = founded;
            now_func->decl = 1;
        }
        else {
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
            now_func = $2;
        }
      } statement LOBRACE {
        ReturnStatementCheck(Error_msg, now_func, &return_s);
        now_func = NULL;
        if(Opt_Symbol) SymbolTablePrint(Alice);
        SymbolTablePop(Alice);
        SymbolTablePop(Alice_buf);
      }
    ;

var_decl :
      var_decl_member { 
        SymbolTablePushOne(Alice_buf, CopyEntry(entry_buf));
        ClearEntry(entry_buf);
      }
    | var_decl COMMA var_decl_member {
        SymbolTablePushOne(Alice_buf, CopyEntry(entry_buf));
        ClearEntry(entry_buf);
      }
    ;

var_decl_member :
      identifier { 
        AssignEntry(entry_buf, $1, "variable", Alice->nowlevel, CopyType(type_buf), NULL);
      }
    | identifier ASSIGN var_init {
        AssignEntry(entry_buf, $1, "variable", Alice->nowlevel, CopyType(type_buf), NULL);
        InitialValue(entry_buf, $3);
      }
    | identifier decl_array {
        AssignEntry(entry_buf, $1, "variable", Alice->nowlevel, CopyType(type_buf), NULL);
        entry_buf->type->array = CopyArray(array_buf);
        DelArray(&array_buf);
      }
    | identifier decl_array ASSIGN var_array_init {
        AssignEntry(entry_buf, $1, "variable", Alice->nowlevel, CopyType(type_buf), NULL);
        entry_buf->type->array = CopyArray(array_buf);
        DelArray(&array_buf);
        AssignValueArray(entry_buf, $4);
      }
    ;

var_init :
      expr { $$=$1; }
    ;
var_array_init :
      UPBRACE more_expr LOBRACE { $$=$2; }
    | UPBRACE LOBRACE { $$=NULL; }
    ;
more_expr :
      expr {  
        $$ = BuildValueArray();
        ValueArrayPush($$, $1);
      }
    | more_expr COMMA expr {
        ValueArrayPush($1, $3);
      }
    ;

function_decl :
      identifier UPPARE arguments LOPARE {
        AssignEntry(entry_buf, $1, "function", Alice->nowlevel, CopyType(type_buf), BuildAttr(CopyArgu(argu_buf), NULL));
        DelArgu(&argu_buf);
        $$ = CopyEntry(entry_buf);
        ClearEntry(entry_buf);
      }
    | identifier UPPARE LOPARE { 
        AssignEntry(entry_buf, $1, "function", Alice->nowlevel, CopyType(type_buf), BuildAttr(NULL, NULL));
        $$ = CopyEntry(entry_buf);
        ClearEntry(entry_buf);
      }
    ;
arguments :
      arguments COMMA argu_type identifier decl_array {
        $3->array = CopyArray(array_buf);
        DelArray(&array_buf);
        AddArgu(&argu_buf, $4, $3);
      }
    | argu_type identifier decl_array {
        $1->array = CopyArray(array_buf);
        DelArray(&array_buf);
        AddArgu(&argu_buf, $2, $1);
      }
    | arguments COMMA argu_type identifier {
        AddArgu(&argu_buf, $4, $3);
      }
    | argu_type identifier {
        AddArgu(&argu_buf, $2, $1);
      }
    ;

const_decl :
    | const_decl COMMA identifier ASSIGN const_value { 
        AssignEntry(entry_buf, $3, "constant", Alice->nowlevel, CopyType(type_buf), NULL);
        InitialValue(entry_buf, $5);
        SymbolTablePushOne(Alice_buf, CopyEntry(entry_buf));
        ClearEntry(entry_buf);
      }
    | identifier ASSIGN const_value { 
        AssignEntry(entry_buf, $1, "constant", Alice->nowlevel, CopyType(type_buf), NULL);
        InitialValue(entry_buf, $3);
        SymbolTablePushOne(Alice_buf, CopyEntry(entry_buf));
        ClearEntry(entry_buf);
      }
    ;

decl_array :
      decl_array UPBRAC INTEGER LOBRAC { AddDimen(&array_buf, atoi($3)); }
    | UPBRAC INTEGER LOBRAC { AddDimen(&array_buf, atoi($2)); }
    ;

statement :
      statement type var_decl SEMICOLON {
        return_s = 0;
        DelType(type_buf); 
        SymbolTablePush(Alice, Alice_buf);
      }
    | statement CONST type const_decl SEMICOLON {
        return_s = 0;
        DelType(type_buf); 
        SymbolTablePush(Alice, Alice_buf);
      }
    | statement assignment SEMICOLON { return_s = 0; }
    | statement a_function SEMICOLON { return_s = 0; }
    | statement PRINT expr SEMICOLON { 
        return_s = 0; 
        if($3!=NULL && $3->type->array!=NULL){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Variable references in print statement must be scalar type");
            ErrorTablePush(Error_msg, msg);
        }
      }
    | statement READ expr SEMICOLON  { 
        return_s = 0; 
        if($3!=NULL && $3->type->array!=NULL){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "Variable references in read statement must be scalar type");
            ErrorTablePush(Error_msg, msg);
        }
      }
    | statement IF UPPARE expr LOPARE UPBRACE {
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
        } statement LOBRACE {
            BoolexprCheck(Error_msg, $4, "if");
            if(Opt_Symbol) SymbolTablePrint(Alice);
            SymbolTablePop(Alice);
            SymbolTablePop(Alice_buf);
        } else_statement { 
        return_s = 0; 
      }
    | statement WHILE UPPARE expr LOPARE UPBRACE { 
            inloop=1; 
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
        } statement LOBRACE { 
        return_s = 0; 
        inloop = 0;
        BoolexprCheck(Error_msg, $4, "while");
        if(Opt_Symbol) SymbolTablePrint(Alice);
        SymbolTablePop(Alice);
        SymbolTablePop(Alice_buf);
      }
    | statement DO UPBRACE {  
            inloop=1; 
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
        } statement LOBRACE WHILE UPPARE expr LOPARE SEMICOLON { 
        return_s = 0; 
        inloop = 0;
        BoolexprCheck(Error_msg, $9, "while");
        if(Opt_Symbol) SymbolTablePrint(Alice);
        SymbolTablePop(Alice);
        SymbolTablePop(Alice_buf);
      }
    | statement FOR UPPARE initial_expr SEMICOLON control_expr SEMICOLON initial_expr LOPARE UPBRACE { 
            inloop=1; 
            (Alice->nowlevel)++;
            (Alice_buf->nowlevel)++;
        } statement LOBRACE { 
        return_s = 0;
        inloop = 0;
        BoolexprCheck(Error_msg, $6, "for");
        if(Opt_Symbol) SymbolTablePrint(Alice);
        SymbolTablePop(Alice);
        SymbolTablePop(Alice_buf);
      }
    | statement RETURN expr SEMICOLON { 
        return_s = 1; 
        ReturnTypeCheck(Error_msg, now_func, $3); 
      }
    | statement BREAK SEMICOLON { 
        return_s = 0; 
        if(inloop != 1){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "'break' can only appear in loop statements");
            ErrorTablePush(Error_msg, msg);
        }
      }
    | statement CONTINUE SEMICOLON  { 
        return_s = 0;
        if(inloop != 1){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "'continue' can only appear in loop statements");
            ErrorTablePush(Error_msg, msg);
        }
      }
    | statement SEMICOLON { return_s = 0; }
    | statement UPBRACE {
        (Alice->nowlevel)++;
        (Alice_buf->nowlevel)++;
      } statement LOBRACE { 
        if(Opt_Symbol) SymbolTablePrint(Alice);
        SymbolTablePop(Alice);
        SymbolTablePop(Alice_buf);
        return_s = 0;
      }
    |
    ;

else_statement :
      ELSE UPBRACE {
        (Alice->nowlevel)++;
        (Alice_buf->nowlevel)++;
      } statement LOBRACE {
        if(Opt_Symbol) SymbolTablePrint(Alice);
        SymbolTablePop(Alice);
        SymbolTablePop(Alice_buf);
      }
    | { }
    ;

initial_expr :
      expr { $$=$1; }
    | assignment { $$=NULL; }
    |      { $$=NULL; }
    ;

control_expr :
      expr { $$=$1; }
    |      { $$=NULL; }
    ;

expr :
      UPPARE expr LOPARE { $$=$2; }
    | MINUS expr %prec HIGH_P {
        $$=Expr_neg($2, Error_msg);
      }
    | EXCLAM expr { 
        $$=Expr_exclam($2, Error_msg);
      }
    | expr PLUS expr {
        $$=Expr_plus($1, $3, Error_msg);
      }
    | expr MINUS expr {
        $$=Expr_minus($1, $3, Error_msg);
      }
    | expr MUL expr {
        $$=Expr_mul($1, $3, Error_msg);
      }
    | expr DIV expr {
        $$=Expr_div($1, $3, Error_msg);
      }
    | expr MOD expr {
        $$=Expr_mod($1, $3, Error_msg);
      }
    | expr ANDAND expr {
        $$=Expr_and($1, $3, Error_msg);
      }
    | expr OROR expr {
        $$=Expr_or($1, $3, Error_msg);
      }
    | expr LT expr {
        $$=Expr_lt($1, $3, Error_msg);
      }
    | expr GT expr {
        $$=Expr_gt($1, $3, Error_msg);
      }
    | expr LE expr {
        $$=Expr_le($1, $3, Error_msg);
      }
    | expr GE expr {
        $$=Expr_ge($1, $3, Error_msg);
      }
    | expr EQUAL expr {
        $$=Expr_eq($1, $3, Error_msg);
      }
    | expr NOTEQUAL expr {
        $$=Expr_ne($1, $3, Error_msg);
      }
    | value { $$=$1; }
    | identifier { 
        struct Entry *entry = FindID(Alice, Error_msg, $1);
        if(entry!=NULL)
            if(strcmp(entry->kind, "constant") == 0){
                $$ = entry->attr->value;
            }
            else {
                $$ = BuildDefaultValue(CopyType(entry->type));
            }
        else
            $$ = NULL;
      }
    | identifier array {
        struct Entry *entry;
        if((entry = CopyEntry(FindID(Alice, Error_msg, $1))) != NULL)
            ReduceTypeArray(entry->type, $2, Error_msg);
        $$ = (entry!=NULL)? BuildDefaultValue(CopyType(entry->type)) : NULL;
      }
    | a_function { $$=$1; }
    ;

assignment :
      identifier array ASSIGN expr {
        struct Entry *entry = CopyEntry(FindID(Alice, Error_msg, $1));
        if(entry != NULL)
            ReduceTypeArray(entry->type, $2, Error_msg);
        Assignment(entry, $4, Error_msg);
        DelEntry(entry);
      } 
    | identifier ASSIGN expr {
        Assignment(FindID(Alice, Error_msg, $1), $3, Error_msg);
      }
    ;

a_function :
      identifier function_call {
        $$ = CallFunction(Alice, Error_msg, $1, $2);
      }
    ;

function_call :
      UPPARE LOPARE { $$=NULL; }
    | UPPARE func_argu LOPARE { $$=$2; }
    ;

func_argu :
      expr {
        if($1 != NULL){
            $$ = BuildArgu($1->type);
        }
      }
    | func_argu COMMA expr {
        if($3 != NULL){
            $$->next = BuildArgu($3->type);
        }
      }
    ;

array :
      UPBRAC expr LOBRAC {
        $$ = 1;
        if($2!=NULL && ($2->type->array!=NULL || strcmp($2->type->type, "int")!=0)){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "The index of array must be int type");
            ErrorTablePush(Error_msg, msg);
        }
      }
    | array UPBRAC expr LOBRAC {
        $$+=1;
        if($3!=NULL && ($3->type->array!=NULL || strcmp($3->type->type, "int")!=0)){
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg), "The index of array must be int type");
            ErrorTablePush(Error_msg, msg);
        }
      }

type : 
      INT         { type_buf=BuildType($1, NULL); }
    | float_type  { type_buf=$1; }
    | STRING      { type_buf=BuildType($1, NULL); } 
    | bool_type   { type_buf=$1; }
    | VOID        { type_buf=BuildType($1, NULL); }
    ;

argu_type : 
      INT         { $$=BuildType($1, NULL); }
    | float_type  { $$=$1; }
    | STRING      { $$=BuildType($1, NULL); } 
    | bool_type   { $$=$1; }
    ;

float_type : 
      FLOAT        { $$=BuildType($1, NULL); }
    | DOUBLE       { $$=BuildType($1, NULL); }
    ;

bool_type : 
      BOOL       { $$=BuildType($1, NULL); }
    | BOOLEAN    { $$=BuildType($1, NULL); }
    ;

identifier : 
      ID          { $$=strdup($1); }
    ;

value : 
      bool_value { $$=$1; }
    | INTEGER    { $$=BuildValue(BuildType("int", NULL), $1); }
    | FLOAT_NUM  { $$=BuildValue(BuildType("float", NULL), $1); }
    | ONESTRING  { $$=BuildValue(BuildType("string", NULL), $1); }
    | SCIENTIFIC { $$=BuildValue(BuildType("double", NULL), $1); }
    ;

const_value :
      bool_value { $$=$1; }
    | INTEGER    { $$=BuildValue(BuildType("int", NULL), $1); }
    | MINUS INTEGER  { 
        char neg[20];
        snprintf(neg, sizeof(neg), "-%s", $2);
        $$=BuildValue(BuildType("int", NULL), neg);
      }
    | FLOAT_NUM  { $$=BuildValue(BuildType("float", NULL), $1); }
    | MINUS FLOAT_NUM  { 
        char neg[20];
        snprintf(neg, sizeof(neg), "-%s", $2);
        $$=BuildValue(BuildType("float", NULL), neg);
      }
    | ONESTRING  { $$=BuildValue(BuildType("string", NULL), $1); }
    | SCIENTIFIC { $$=BuildValue(BuildType("float", NULL), $1); }
    | MINUS SCIENTIFIC  { 
        char neg[20];
        snprintf(neg, sizeof(neg), "-%s", $2);
        $$=BuildValue(BuildType("double", NULL), neg);
      }

bool_value :
      TRUE  { $$=BuildValue(BuildType("bool", NULL), "true"); }
    | FALSE { $$=BuildValue(BuildType("bool", NULL), "false"); }
    ;

%%
int yyerror( char *msg )
{
    fprintf( stderr, "\n|--------------------------------------------------------------------------\n" );
    fprintf( stderr, "| Error found in Line #%d: %s\n", linenum, buf );
    fprintf( stderr, "|\n" );
    fprintf( stderr, "| Unmatched token: %s\n", yytext );
    fprintf( stderr, "|--------------------------------------------------------------------------\n" );
    exit(-1);
}

int  main( int argc, char **argv )
{
    if( argc != 2 ) {
        fprintf(  stdout,  "Usage:  ./parser  [filename]\n"  );
        exit(0);
    }

    FILE *fp = fopen( argv[1], "r" );
    outfp = fopen("alice.j", "w");

    if( fp == NULL || outfp == NULL)  {
        fprintf( stdout, "Open  file  error\n" );
        exit(-1);
    }
    
    Alice = SymbolTableBuild();
    Alice_buf = SymbolTableBuild();
    Error_msg = ErrorTableBuild();
    entry_buf = (struct Entry*)malloc(sizeof(struct Entry));
    array_buf = NULL;
    DelArgu(&argu_buf);
    now_func = NULL;
    return_s = 0;
    inloop = 0;

    yyin = fp;
    yyparse();

    SymbolTableCheckRemainFunction(Alice, Error_msg);
    if(Opt_Symbol == 1) SymbolTablePrint(Alice);
    ErrorTablePrint(Error_msg);
    if(Error_msg->size == 0){
        fprintf( stdout, "\n" );
        fprintf( stdout, "|-------------------------------------------|\n" );
        fprintf( stdout, "| There is no syntactic and semantic error! |\n" );
        fprintf( stdout, "|-------------------------------------------|\n" );
    }
    if (Opt_Statistic)
        print();
    rm();


    /*
    fprintf( stdout, "\n" );
    fprintf( stdout, "|--------------------------------|\n" );
    fprintf( stdout, "|  There is no syntactic error!  |\n" );
    fprintf( stdout, "|--------------------------------|\n" );
    */

    DelSymbolTable(Alice);
    DelErrorTable(Error_msg);
    exit(0);
}

