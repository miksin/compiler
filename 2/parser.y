%{
#include <stdio.h>
#include <stdlib.h>

extern int linenum;             /* declared in lex.l */
extern FILE *yyin;              /* declared by lex */
extern char *yytext;            /* declared by lex */
extern char buf[256];           /* declared in lex.l */
%}

%token SEMICOLON      /* ; */
%token UPPARE         /* ( */
%token LOPARE         /* ) */
%token UPBRAC         /* [ */
%token LOBRAC         /* ] */
%token COMMA          /* , */
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
%token ASSIGN         /* sign */
%token INTEGER        /* value */
%token FLOAT_NUM      /* value */
%token ONESTRING      /* value */
%token SCIENTIFIC     /* value */

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


 ////////////// Micros ////////////////
type : INT
     | float_type
     | STRING
     | bool_type
     ;

float_type : FLOAT
           | DOUBLE
           ;

bool_type : BOOL
          | BOOLEAN
          ;

identifier : ID
           ;

value : INTEGER
      | FLOAT_NUM
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

