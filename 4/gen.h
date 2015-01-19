#define LessT  1
#define LessE  2
#define GreatT 3
#define GreatE 4
#define Equal  5
#define NEqual 6
#define CoerReg 127
#define Cond 0
#define Loop 1

struct Label {
    int num;
    int type;
};

void StackPush(int num, int type);
int StackTop(int type);
int StackPop();

void Gen(int n, ...);
void GenTitle();

void GenVariableDecl(void*);    // void* -> Entry*
void GenFunction(void*);        // void* -> Entry*
void GenLoadVal(void*);         // void* -> Value*
void GenLoadValbyID(void*);     // void* -> Entry*

void GenArithExpr(void* e1, char op, void* e2);
void GenLogiExpr(void* e1, char op, void* e2);
void GenNegExpr(void* e1, char op);
void GenRelExpr(void* e1, int op, void* e2);

void GenAssignment(void* alice, void* bob);// void* -> Entry*  void* -> Value*
void GenPrintStart();
void GenPrintEnd(void *);           // void* -> Value*
void GenRead(void*);                // void* -> Entry*
void GenReturn(void*, void *);      // void* -> Value*  void* -> Entry*
void GenFunctionCall(void*);        // void* -> Entry*
void GenArguCoercion(void*, void*); // void* -> Argu*

void GenIfStart();
void GenIfElse();
void GenIfExit();

void GenForBegin();
void GenForInc();
void GenForStart();
void GenForExit();

void GenWhileBegin();
void GenWhileStart();
void GenWhileExit();

void GenDoWhileStart();
void GenDoWhileBegin();
void GenDoWhileExit();

void GenBreak();
void GenContinue();
