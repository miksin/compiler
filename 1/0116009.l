%{
#define LIST                { strncat(buf, yytext, sizeof(buf) - strlen(buf) - 1); }
#define strCat(t)           { strncat(str_buf, t, sizeof(str_buf) - strlen(str_buf) - 1); }
#define token(t)            { LIST; if (Opt_Token) printf("<\"%s\">\n", t); }
#define tokenString(t, s)   { LIST; if (Opt_Token) printf("<%s:%s>\n", t, s); }
#define MAX_LINE_LENGTH 257
#define MAX_ID_LENGTH 257
int Opt_Source = 1;
int Opt_Token = 1;
int Opt_Statistic = 1;
int Opt_Flag = 0;
int Opt_Flag_change = 0;
int linenum = 1;
char buf[MAX_LINE_LENGTH];
char str_buf[MAX_LINE_LENGTH];
char e_ch[3];
struct entry{
     char*  id;
     int    freq;
     struct entry* next;
};

struct entry* Table_head = NULL;

void insert(char*);
void print();
void print_all(struct entry*);
void rm();
%}

digit       [0-9]
nonzero     [1-9]
letter      [a-zA-Z]
delimiter   [,;\(\)\[\]\{\}]
arithmetic  [\+\-\*/%]
relational  [<>!=]=?
logical     (&&)|(\|\|)
keyword     while|do|if|else|true|false|for|int|print|const|read|boolean|bool|void|float|double|string|continue|break|return
space       [ \t]
doubleq     [\"]
space_n     [ \t\n]

%Start Comment String Pragma_f
%%
<INITIAL>\/\/.*\n {   //one-line comment
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf);
    ++linenum;
    buf[0] = '\0';
}

<Pragma_f>\/\/.*\n {   //one-line comment
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf);
    ++linenum;
    buf[0] = '\0';
}

<INITIAL>\/\* {   //multi-line comment start
    LIST;
    BEGIN Comment;
}

<Pragma_f>\/\* {   //multi-line comment start
    LIST;
    BEGIN Comment;
}

<Comment>\*\/ { //multi-line comment end
    LIST;
    BEGIN 0;
}
 
<Comment>.  {   //multi-line comment content
    LIST;
}

<INITIAL>{doubleq}  {   //string start
    LIST;
    BEGIN String;
}

<String>{doubleq} { //string end
    tokenString("string", str_buf);
    str_buf[0] = '\0';
    BEGIN 0;
}

<String>\\t {     //string exception "\t"
    LIST;
    e_ch[0] = '\t';
    e_ch[1] = '\0';
    strCat(e_ch);
}

<String>\\n {     //string exception "\n"
    LIST;
    e_ch[0] = '\n';
    e_ch[1] = '\0';
    strCat(e_ch);
}

<String>\\[\\"] {     //string exception "\\", "\""
    LIST;
    e_ch[0] = yytext[1];
    e_ch[1] = '\0';
    strCat(e_ch);
}

<String>\n { // Invalid string
    fprintf(stderr, "Error at line %d: %s\n", linenum, yytext);
    exit(1);
}

<String>.  {
    LIST;
    strCat(yytext);
}

<INITIAL>{delimiter} {   //delimiter
    tokenString("delim", yytext); 
}

<INITIAL>{arithmetic}|{relational}|{logical} {   //operator
    token(yytext);
}

<INITIAL>{keyword} {     //keywords
    tokenString("KW", yytext);
}

<INITIAL>({nonzero}{digit}*)|0 { //integers
    tokenString("integer", yytext);
}

<INITIAL>{digit}+(\.{digit}+)?[Ee][\+\-]?{digit}+    { //scientific
    tokenString("scientific", yytext);
}

<INITIAL>{digit}+\.{digit}+ { //floating-point
    tokenString("float", yytext);
}

<INITIAL>{letter}({letter}|{digit})* {  //identifier
    insert(yytext);
    tokenString("id", yytext);
}

<INITIAL>^#pragma{space}*(source|token|statistic){space}*(on|off) {
    LIST;
    if(strstr(yytext, "on") != NULL){
        if(strstr(yytext, "source") != NULL){
            Opt_Source = 1;
        }
        else if(strstr(yytext, "token") != NULL){
            Opt_Token = 1;
        }
        else if(strstr(yytext, "statistic") != NULL){
            Opt_Statistic = 1;
        }
    }
    else if(strstr(yytext, "off") != NULL){
        if(strstr(yytext, "source") != NULL){
            Opt_Source = 0;
        }
        else if(strstr(yytext, "token") != NULL){
            Opt_Token = 0;
        }
        else if(strstr(yytext, "statistic") != NULL){
            Opt_Statistic = 0;
        }
    }
    BEGIN Pragma_f;
}

{space} {  //spaces
    LIST;
}

<Pragma_f>\n {
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf);
    ++linenum;
    buf[0] = '\0';
    BEGIN 0;
}

\n {
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf);
    ++linenum;
    buf[0] = '\0';
}

. { // Unrecognized character
    fprintf(stderr, "Error at line %d: %s\n", linenum, yytext);
    exit(1);
}

%%

int main( int argc, char **argv )
{   
    if (argc >= 2)
    {
        yyin = fopen( argv[1], "r" );
        if ( NULL == yyin ) {
            fprintf( stderr, "Cannot open: %s\n", argv[1] );
            exit(-1);
        }
    }
    else
    {
        yyin = stdin;
    }

    yylex();

    if (Opt_Statistic)
    {
        // Print frequency table
        print();
    }
    
    rm();
    exit(0);
}

void insert(char* str){
    struct entry* p = Table_head;
    int flag = 0;
    
    while(p != NULL){
        if(strcmp(str,p->id) == 0){
            ++(p->freq);
            flag = 1;
            break;
        }
        p = p->next;
    }

    if(flag == 0){
        struct entry* new_entry = (struct entry*)malloc(sizeof(struct entry));
        char* new_str = (char*)malloc(258*sizeof(char));
        strcpy(new_str,str);
        new_entry->id = new_str;
        new_entry->freq = 1;
        new_entry->next = Table_head;
        Table_head = new_entry;
    }

}
/*
void print(){
    printf("frequencies of identifiers:\n");
    struct entry* p = Table_head;
    while(p != NULL){
        printf("%-20s%d\n",p->id,p->freq);
        p = p->next;
    }
}
*/
void print(){
    printf("frequencies of identifiers:\n");
    struct entry* p = Table_head;
    print_all(p);
}

void print_all(struct entry* p){
    if(p != NULL){
        print_all(p->next);
        printf("%s\t%d\n",p->id,p->freq);
    }
}

void rm(){
    struct entry* p = Table_head;
    while(p != NULL){
        free(p->id);
        free(p);
        p = p->next;
    }
}
