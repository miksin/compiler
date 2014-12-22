struct SymbolTable {
    int nowlevel;
    int size;
    int capacity;
    struct Entry** entryVector;
};

struct ErrorTable {
    int size;
    int capacity;
    struct Error **errorVector;
};

struct Error {
    int linenum;
    char msg[1024];
};

struct Entry {
    char name[33];
    char kind[12];
    int level;
    int decl;
    struct Type *type;
    struct Attribute *attr;
};

struct Type {
    char type[10];
    struct Arraynode* array;
};

struct Arraynode {
    int dimension;
    struct Arraynode* next;
};

struct Attribute {
    struct Argu* argu;
    struct Value* value;
    struct ValueArray *varray;
};

struct Argu {
    char name[33];
    struct Type* type;
    struct Argu* next;
};

struct Value {
    struct Type *type;
    char strV[1024];
    double dval;
    int ival;
};

struct ValueArray {
    int size;
    int capacity;
    struct Value **vVector;
};

struct SymbolTable* SymbolTableBuild();
void SymbolTablePrint(struct SymbolTable*);
void SymbolTablePrintLevel(int);
void SymbolTablePrintType(struct Type*, int);
void SymbolTablePrintAttr(struct Attribute*, const char*, struct Type*);
void SymbolTablePush(struct SymbolTable*, struct SymbolTable*);
void SymbolTablePushOne(struct SymbolTable*, struct Entry*);
void SymbolTablePushArgu(struct SymbolTable*, struct Argu*);
void SymbolTablePop(struct SymbolTable*);
struct Entry* SymbolTableFind(struct SymbolTable*, const char*);

struct ErrorTable* ErrorTableBuild();
void ErrorTablePrint(struct ErrorTable*);
void ErrorTablePush(struct ErrorTable* ,const char*);
int ValDeclCheck(struct SymbolTable*, struct Entry*, struct ErrorTable*);
int ConstDeclCheck(struct SymbolTable*, struct Entry*, struct ErrorTable*);
int FuncDeclCheck(struct SymbolTable*, struct Entry*, struct ErrorTable*);
int FuncDefCheck(struct SymbolTable*, struct Entry*, struct ErrorTable*);

struct Entry* BuildEntry(const char*, const char*, int, struct Type*, struct Attribute*);
void AssignEntry(struct Entry*, const char*, const char*, int, struct Type*, struct Attribute*);
struct Entry* CopyEntry(const struct Entry*);
struct Entry* ClearEntry(struct Entry*);

struct Type* BuildType(const char*, struct Arraynode*);
struct Type* CopyType(const struct Type*);
struct Arraynode* CopyArray(struct Arraynode*);
void AddDimen(struct Arraynode**, int);

struct Attribute* BuildAttr(struct Argu*, struct Value*);
struct Attribute* CopyAttr(const struct Attribute*);

struct Argu* BuildArgu(struct Type*);
struct Argu* CopyArgu(struct Argu*);
void AddArgu(struct Argu**, const char*, struct Type*);

struct Value* BuildValue(struct Type*, const char*);
struct Value* CopyValue(struct Value*);
void AssignValue(struct Entry*, struct Value*);
struct ValueArray* BuildValueArray();
struct ValueArray* CopyValueArray(struct ValueArray*);
void ValueArrayPush(struct ValueArray*, struct Value*);
void AssignValueArray(struct Entry*, struct ValueArray*);

int AssignValueCheck(struct Entry*, struct Value*);

int CmpType(struct Type*, struct Type*);
int CmpArgu(struct Argu*, struct Argu*);

void DelSymbolTable(struct SymbolTable*);
void DelErrorTable(struct ErrorTable*);
void DelEntry(struct Entry*);
void DelType(struct Type*);
void DelAttr(struct Attribute*);
void DelValue(struct Value*);
void DelValueArray(struct ValueArray*);
void DelArray(struct Arraynode**);
void DelArgu(struct Argu**);

struct Value* Expr_neg(struct Value*, struct ErrorTable*);
struct Value* Expr_exclam(struct Value*, struct ErrorTable*);
struct Value* Expr_plus(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_minus(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_mul(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_div(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_mod(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_and(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_or(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_lt(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_gt(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_le(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_ge(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_eq(struct Value*, struct Value*, struct ErrorTable*);
struct Value* Expr_ne(struct Value*, struct Value*, struct ErrorTable*);
