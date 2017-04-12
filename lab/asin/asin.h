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

typedef union {
    long
    int i;              // int, char
    double d;           // double
    const char *str;    // char[]
} CtVal;

typedef struct {
    Type type;          // type of the result
    int isLVal;         // if it is a LVal
    int isCtVal;        // if it is a constant value (int, real, char, char[])
    CtVal ctVal;        // the constat value
} RetVal;

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
int expr(RetVal *rv);
int exprAssign(RetVal *rv);
int exprOr1(RetVal *rv);
int exprAnd1(RetVal *rv);
int exprEq1(RetVal *rv);
int exprRel1(RetVal *rv);
int exprAdd1(RetVal *rv);
int exprMul1(RetVal *rv);
int exprOr(RetVal *rv);
int exprAnd(RetVal *rv);
int exprEq(RetVal *rv);
int exprRel(RetVal *rv);
int exprAdd(RetVal *rv);
int exprMul(RetVal *rv);
int exprCast(RetVal *rv);
int exprUnary(RetVal *rv);
int exprPostfix(RetVal *rv);
int exprPrimary(RetVal *rv);
