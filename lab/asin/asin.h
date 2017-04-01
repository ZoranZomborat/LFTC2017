#include "../alex/alex.h"

int consume(Token** ctk,int code);

int ruleUnit(Token *tk);
int declStruct(Token *tk);
int declVar(Token *tk);
int typeBase(Token *tk);
int arrayDecl(Token *tk);
int typeName(Token *tk);
int declFunc(Token *tk);
int funcArg(Token *tk);
int stm(Token *tk);
int stmCompound(Token *tk);
int expr(Token *tk);
int exprAssign(Token *tk);
int exprOr(Token *tk);
int exprAnd(Token *tk);
int exprEq(Token *tk);
int exprRel(Token *tk);
int exprAdd(Token *tk);
int exprMul(Token *tk);
int exprCast(Token *tk);
int exprUnary(Token *tk);
int exprPostfix(Token *tk);
int exprPrimary(Token *tk);
