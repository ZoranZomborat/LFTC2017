#include "../alex/alex.h"

int consume(atomType code);

typedef struct _Symbol Symbol;
typedef struct _Symbols Symbols;
typedef struct _Type Type;

typedef enum {
    TB_INT = 0,
    TB_DOUBLE,
    TB_CHAR,
    TB_STRUCT,
    TB_VOID
} TBtype;

typedef enum {
    CLS_VAR,
    CLS_FUNC,
    CLS_EXTFUNC,
    CLS_STRUCT
} CLStype;

typedef enum {
    MEM_GLOBAL,
    MEM_ARG,
    MEM_LOCAL
} MEMtype;

typedef struct _Type {
    TBtype typeBase;
    Symbol *s; // struct definition for TBtype
    int nElements;// >0 array of given size, 0=array without size, <0 non array
} Type;

typedef struct _Symbols{
    Symbol **begin; // the beginning of the symbols, or NULL
    Symbol **fill;   // the position after the last symbol
    Symbol **end;// the position after the allocated space
}Symbols;

typedef struct _Symbol
{
    const char *name; // a reference to the name stored in a token
    CLStype cls;
    MEMtype mem;
    Type type;
    int depth; // 0-global, 1-in function, 2...N-nested blocks in function
    union
    {
        Symbols args; // used only for functions
        Symbols members;// used only for structs
    };
}Symbol;

//Syntactic rules functions
int ruleUnit();
int declStruct();
int declVar();
int typeBase(Type *type);
int arrayDecl(Type *type);
int typeName();
int declFunc();
int funcArg();
int stm();
int stmCompound();
int expr();
int exprAssign();
int exprOr1();
int exprAnd1();
int exprEq1();
int exprRel1();
int exprAdd1();
int exprMul1();
int exprOr();
int exprAnd();
int exprEq();
int exprRel();
int exprAdd();
int exprMul();
int exprCast();
int exprUnary();
int exprPostfix();
int exprPrimary();
