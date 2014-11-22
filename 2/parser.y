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

program : decl_and_def_list
        ;
decl_and_def_list : declaration_list
                  //  | decl_and_def_list definition_list
                    ;
declaration_list : declaration_list funct_decl
                 | declaration_list var_decl
                 | declaration_list const_decl
                 |
                 ;

funct_decl : decl UPPARE arguments LOPARE SEMICOLON
           | VOID identifier UPPARE arguments LOPARE SEMICOLON
           ;
var_decl : decl id_list SEMICOLON
         ;
const_decl : CONST INT identifier ASSIGN INTEGER const_int_list SEMICOLON
           | CONST float_type identifier ASSIGN FLOAT_NUM const_float_list SEMICOLON
           | CONST STRING identifier ASSIGN ONESTRING const_string_list SEMICOLON
           | CONST bool_type identifier ASSIGN  const_bool_list SEMICOLON
           ;

decl : type identifier array
     ;

arguments : non_empty_arguments
          |
          ;

non_empty_arguments : non_empty_arguments COMMA decl
                    | decl
                    ;

id_list : id_list COMMA identifier
        |
        ;

const_int_list : const_int_list COMMA identifier ASSIGN INTEGER
               |
               ;

const_float_list : const_float_list COMMA identifier ASSIGN FLOAT_NUM
                 |
                 ;

const_string_list : const_string_list COMMA identifier ASSIGN ONESTRING
                  |
                  ;

const_bool_list : const_bool_list COMMA identifier ASSIGN bool_value
                |
                ;

array : is_array
      |
      ;

is_array : is_array index
         | index
         ;

index : UPBRAC expression LOBRAC
      ;

expression : value
           ;

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

bool_value : TRUE
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

