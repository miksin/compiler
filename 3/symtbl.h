struct SymbolTable {
    int nowlevel;
    int size;
    int capacity;
    struct Entry** entryVector;
};

struct Entry {
    char name[33];
    char kind[12];
    int level;
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
    int intV;
    double floV;
    char* strV;
};

struct Argu {
    char type[10];
    struct Arraynode* array;
    struct Argu* next;
};

void SymbolTableBuild(struct SymbolTable*);
void SymbolTablePrint(struct SymbolTable*);
void SymbolTablePush(struct SymbolTable*, struct Entry*);
struct Entry* SymbolTableFind(struct SymbolTable*, const char*);

struct Entry* BuildEntry(const char*, const char*, int, struct Type*, struct Attribute*);
struct Entry* CopyEntry(const struct Entry*);
struct Type* BuildType(const char*, const struct Arraynode*);
struct Type* CopyType(const struct Type*);
struct Arraynode* AddDimen(struct Arraynode*, int);
struct Attribute* BuildAttr(const struct Argu*, int, double, const char*);
struct Attribute* CopyAttr(const struct Attribute*);
