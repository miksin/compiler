%{
#include <stdio.h>
#include <stdlib.h>

extern int linenum;             /* declared in lex.l */
extern FILE *yyin;              /* declared by lex */
extern char *yytext;            /* declared by lex */
extern char buf[256];           /* declared in lex.l */
%}

%token SEMICOLON      /* ; */
%token COMMA          /* , */
%token UPPARE LOPARE  /* () */
%token UPBRAC  LOBRAC /* [] */
%token UPBRACE LOBRACE/* {} */
%token ID             /* identifier */
%token WHILE FOR DO   /* keyword */
%token IF ELSE        /* keyword */
%token TRUE FALSE     /* keyword */
%token PRINT          /* keyword */
%token CONST          /* keyword */
%token READ           /* keyword */
%token BOOL BOOLEAN   /* type */
%token VOID           /* type */
%token INT            /* type */
%token FLOAT DOUBLE   /* type */
%token STRING         /* type */
%token CONTINUE BREAK /* keyword */
%token RETURN         /* keyword */
%token ASSIGN         /* = */
%token EXCLAM         /* ! */
%token ANDAND OROR    /* && || */
%token LT GT LE GE    /* < > <= >= */
%token EQUAL NOTEQUAL /* == != */
%token PLUS MINUS     /* + - */
%token MUL DIV        /* * / */
%token MOD            /* % */
%token INTEGER        /* value */
%token FLOAT_NUM      /* value */
%token ONESTRING      /* value */
%token SCIENTIFIC     /* value */

%left OROR
%left ANDAND
%left EXCLAM
%left LT GT EQUAL LE GE NOTEQUAL
%left PLUS MINUS
%left MUL DIV MOD
%left HIGH_P

%%

 ///////////////////// Structure ////////////////////
program :
      decl_and_def_list
    ;
decl_and_def_list :
      declaration_list definition_list
    ;
declaration_list : 
      declaration_list type identifier var_decl SEMICOLON
    | declaration_list type identifier function_decl SEMICOLON
    | declaration_list VOID identifier function_decl SEMICOLON
    | declaration_list CONST type identifier const_decl SEMICOLON
    |
    ;
definition_list :
      definition_list type identifier function_decl UPBRACE statement LOBRACE
    | definition_list VOID identifier function_decl UPBRACE statement LOBRACE
    | type identifier function_decl UPBRACE statement LOBRACE
    | VOID identifier function_decl UPBRACE statement LOBRACE
    ;


 //////////////////// Declaration ///////////////////
var_decl :
      decl_array decl_var_list
    | decl_array
    ;
decl_var_list :
      COMMA identifier decl_array decl_var_list
    | COMMA identifier decl_array
    ;

function_decl :
      UPPARE arguments LOPARE
    ;
arguments :
      type identifier decl_array more_argu
    | type identifier decl_array
    |
    ;
more_argu :
      COMMA type identifier decl_array more_argu
    | COMMA type identifier
    ;

const_decl :
    | COMMA identifier ASSIGN value const_decl
    | ASSIGN value
    ;

decl_array :
      decl_array UPBRAC INTEGER LOBRAC
    |
    ;

 ////////////////// Statements ///////////////////
statement :
      statement type identifier var_decl SEMICOLON
    | statement CONST type identifier const_decl SEMICOLON
    | statement identifier ASSIGN expression SEMICOLON
    | statement PRINT expression SEMICOLON
    | statement IF UPPARE expression LOPARE UPBRACE statement LOBRACE
    | statement IF UPPARE expression LOPARE UPBRACE statement LOBRACE ELSE UPBRACE statement LOBRACE
    | statement FOR UPPARE expression LOPARE UPBRACE statement LOBRACE
    | statement WHILE UPPARE expression LOPARE UPBRACE statement LOBRACE
    | statement SEMICOLON
    | statement UPBRACE statement LOBRACE
    |
    ;

 ////////////// Expression ////////////
expression :
      UPPARE expression LOPARE
    | MINUS expression %prec HIGH_P
    | EXCLAM expression
    | expression PLUS expression
    | expression MINUS expression
    | expression MUL expression
    | expression DIV expression
    | expression MOD expression
    | expression ANDAND expression
    | expression OROR expression
    | expression LT expression
    | expression GT expression
    | expression LE expression
    | expression GE expression
    | expression EQUAL expression
    | expression NOTEQUAL expression
    | value
    | identifier function_call
    ;

 ////////////// Function call //////////////
function_call :
      UPPARE LOPARE
    | UPPARE expression LOPARE
    |
    ;

 ////////////// Micros ////////////////
type : 
      INT
    | float_type
    | STRING
    | bool_type
    ;

float_type : 
      FLOAT
    | DOUBLE
    ;

bool_type : 
      BOOL
    | BOOLEAN
    ;

identifier : 
      ID
    ;

value : 
      int_value
    | FLOAT_NUM
    ;

int_value :
      INTEGER
    | bool_value
    ;

bool_value :
      TRUE
    | FALSE
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

    if( fp == NULL )  {
        fprintf( stdout, "Open  file  error\n" );
        exit(-1);
    }

    yyin = fp;
    yyparse();

    fprintf( stdout, "\n" );
    fprintf( stdout, "|--------------------------------|\n" );
    fprintf( stdout, "|  There is no syntactic error!  |\n" );
    fprintf( stdout, "|--------------------------------|\n" );
    exit(0);
}

