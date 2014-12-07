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
      type identifier var_decl SEMICOLON
    | type identifier function_decl SEMICOLON
    | VOID identifier function_decl SEMICOLON
    | CONST type identifier const_decl SEMICOLON
    ;
definition_list :
      type identifier function_decl UPBRACE statement LOBRACE
    | VOID identifier function_decl UPBRACE statement LOBRACE
    ;

 //////////////////// Decl and Def ///////////////////
var_decl :
      decl_array decl_assign decl_var_list
    | decl_array decl_assign
    ;
decl_assign :
      ASSIGN var_init
    |
    ;
var_init :
      expr
    | UPBRACE expr more_expr LOBRACE
    ;
more_expr :
     
    | more_expr COMMA expr
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
    | statement assignment SEMICOLON
    | statement a_function SEMICOLON
    | statement PRINT expr SEMICOLON
    | statement READ identifier id_append SEMICOLON
    | statement IF UPPARE expr LOPARE UPBRACE statement LOBRACE
    | statement IF UPPARE expr LOPARE UPBRACE statement LOBRACE ELSE UPBRACE statement LOBRACE
    | statement FOR UPPARE initial_expr SEMICOLON initial_expr SEMICOLON initial_expr LOPARE UPBRACE statement LOBRACE
    | statement WHILE UPPARE expr LOPARE UPBRACE statement LOBRACE
    | statement DO UPBRACE statement LOBRACE WHILE UPPARE expr LOPARE SEMICOLON
    | statement RETURN expr SEMICOLON
    | statement BREAK SEMICOLON
    | statement CONTINUE SEMICOLON
    | statement SEMICOLON
    | statement UPBRACE statement LOBRACE
    |
    ;

 ////////////// expression ////////////
init_expr :
      init_expr COMMA assignment
    | init_expr COMMA expr
    | assignment
    | expr
    ;

initial_expr :
      init_expr
    |
    ;

expr :
      UPPARE expr LOPARE
    | MINUS expr %prec HIGH_P
    | EXCLAM expr
    | expr PLUS expr
    | expr MINUS expr
    | expr MUL expr
    | expr DIV expr
    | expr MOD expr
    | expr ANDAND expr
    | expr OROR expr
    | expr LT expr
    | expr GT expr
    | expr LE expr
    | expr GE expr
    | expr EQUAL expr
    | expr NOTEQUAL expr
    | value
    | identifier id_append
    ;

assignment :
      identifier array ASSIGN var_init
    | identifier ASSIGN var_init
    ;

 ////////////// What ID //////////////
id_append :
      function_call
    | array
    |
    ;

a_function :
      identifier function_call
    ;

function_call :
      UPPARE LOPARE
    | UPPARE expr more_funct_argu LOPARE
    ;

more_funct_argu :
    
    | more_funct_argu COMMA expr
    ;

array :
      UPBRAC expr LOBRAC
    | array UPBRAC expr LOBRAC

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
    | ONESTRING
    | SCIENTIFIC
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

