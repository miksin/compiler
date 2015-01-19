#define LessT  1
#define LessE  2
#define GreatT 3
#define GreatE 4
#define Equal  5
#define NEqual 6
#define CoerReg 127

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
void GenPrintEnd(void *);       // void* -> Value*
void GenRead(void*);            // void* -> Entry*
