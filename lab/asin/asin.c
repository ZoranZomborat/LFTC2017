#include <stdlib.h>
#include "asin.h"

Token *consumedTk;

int consume(Token** pctk,int code)
{
    Token *ctk=*pctk;
    if(ctk->code==code)
    {
        consumedTk=ctk;
        ctk=ctk->next;
        *pctk=ctk;
        return 1;
    }
    return 0;
}

/*exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
           | CT_INT | CT_REAL | CT_CHAR
           | CT_STRING | LPAR expr RPAR ;*/
int exprPrimary(Token *ctk){
    Token *tkStart = ctk;

    if(consume(&ctk, ID)){
        if (consume(&ctk, LPAR)) {
            if ( expr(ctk)){
                while (consume(&ctk, COMMA) && expr(ctk)) {
                };
            }
            if (consume(&ctk, RPAR)) {
                return 1;
            } else{
                err("err at exprPrimary missing ", atomNames[RPAR]);
            }
        }
        return 1;
    } else if (consume(&ctk, CT_INT) || consume(&ctk, CT_REAL) ||
            consume(&ctk, CT_CHAR) || consume(&ctk, CT_STRING)){
        return 1;
    } else if (consume(&ctk, LPAR) && expr(ctk) && consume(&ctk, RPAR)){
        return 1;
    }

    ctk = tkStart;
    return 0;
}


/*exprPostfix: exprPostfix LBRACKET expr RBRACKET
           | exprPostfix DOT ID | exprPrimary ;*/
int exprPostfix(Token *ctk){
    Token *tkStart = ctk;
    if(exprPrimary(ctk)){
        return 1;
    } else if(exprPostfix(ctk)){
        if(consume(&ctk, DOT)){
            if(consume(&ctk ,ID)){
                return 1;
            }
        }else if(consume(&ctk, LBRACK)){
            if(expr(ctk)){
                if(consume(&ctk, RBRACK)){
                    return 1;
                }
            }
        }
        else
            err("err at exprPostfix");
    }
    ctk = tkStart;
    return 0;
}

//typeName: typeBase arrayDecl?
int typeName(Token *ctk)
{
    Token *tkStart = ctk;
    if (typeBase(ctk)) {
        arrayDecl (ctk);
        return 1;
    }
    ctk = tkStart;
    return 0;
}

//exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(Token *ctk)
{
    Token *tkStart = ctk;
    if(consume(&ctk, LPAR)){
        if(typeName(ctk)){
            if(consume(&ctk, RPAR)){
                if(exprCast(ctk)){
                    return 1;
                }
            }
        }
    } else if(exprUnary(ctk)){
        return 1;
    }
    ctk = tkStart;
    return 0;
}

//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
int exprRel(Token * ctk)
{
    Token *tkStart = ctk;
    if(exprAdd(ctk)){
        return 1;
    } else if (exprRel(ctk)){
        if( consume(&ctk, LESS) || consume(&ctk, LESSEQ)
                || consume(&ctk, GRT) || consume(&ctk, GRTEQ)){
            if(exprAdd(ctk)){
                return 1;
            }
        }
    }
    ctk = tkStart;
    return 0;
}

//exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
int exprAdd(Token *ctk)
{
    Token *tkStart = ctk;
    if (exprMul(ctk)){
        return 1;
    } else if (exprAdd(ctk))
    {
        if(consume(&ctk,ADD) || consume(&ctk,SUB)){
            if (exprMul(ctk)){
                return 1;
            }
        }
    }
    ctk = tkStart;
    return 0;
}

//exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
int exprMul(Token *ctk){
    Token *tkStart = ctk;
    if (exprCast(ctk)){
        return 1;
    } else if (exprMul(ctk))
    {
        if(consume(&ctk,MUL) || consume(&ctk,DIV)){
            if (exprCast(ctk)){
                return 1;
            }
        }
    }
    ctk = tkStart;
    return 0;
}


//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel ;
int exprEq(Token *ctk){
    Token *tkStart = ctk;
    if(exprRel(ctk))
    {
        return 1;
    }
    else if (exprEq(ctk)) {
        if(consume(&ctk,EQUAL) || consume(&ctk,NEQUAL))
        {
            if(exprRel(ctk)){
                return 1;
            }
        }
    }
    ctk = tkStart;
    return 0;
}

//exprAnd: exprAnd AND exprEq | exprEq ;
int exprAnd(Token *ctk){
    Token *tkStart = ctk;
    if(exprEq(ctk))
    {
        return 1;
    }
    else if(exprAnd(ctk)){
        if(consume(&ctk,AND)){
            if(exprEq(ctk)){
                return 1;
            }
        }
    }
    ctk= tkStart;
    return 0;
}

//exprOr: exprOr OR exprAnd | exprAnd ;
int exprOr(Token *ctk){
    Token *tkStart = ctk;
    if(exprAnd(ctk))
    {
        return 1;
    }
    else if(exprOr(ctk)){
        if(consume(&ctk,OR)){
            if(exprAnd(ctk)){
                return 1;
            }
        }
    }
    ctk= tkStart;
    return 0;
}

//( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(Token *ctk)
{
    Token *tkStart = ctk;
    if(consume(&ctk,SUB)||consume(&ctk,NOT))
    {
        if(exprUnary(ctk))
        {
            return 1;
        }
    } else if(exprPostfix(ctk))
    {
        return 1;
    }
    ctk= tkStart;
    return 0;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign(Token *ctk)
{
    Token *tkStart = ctk;
    if(exprUnary(ctk))
    {
        if(consume(&ctk, ASSIGN))
        {
            if(exprAssign(ctk))
            {
                return 1;
            }
        }
    } else if(exprOr(ctk))
    {
        return 1;
    }
    ctk= tkStart;
    return 0;
}

//expr: exprAssign ;
int expr(Token *ctk)
{
    Token *tkStart = ctk;
    if(exprAssign(ctk)){
        return 1;
    }
    ctk= tkStart;
    return 0;
}

//stmCompound: LACC ( declVar | stm )* RACC ;
int stmCompound(Token *ctk)
{
    Token *tkStart = ctk;
    if (consume(&ctk, LACC)) {
        while (declVar(ctk) || stm(ctk)) {};
        if(consume(&ctk, RACC)){
            return 1;
        }
    }
    ctk = tkStart;
    return 0;
}


/*stm: stmCompound
           | IF LPAR expr RPAR stm ( ELSE stm )?
           | WHILE LPAR expr RPAR stm
           | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
           | BREAK SEMICOLON
           | RETURN expr? SEMICOLON
           | expr? SEMICOLON ;*/
int stm(Token *ctk)
{
    Token *tkStart = ctk;
    if (stmCompound(ctk)) {
        return 1;
    } else if (consume(&ctk, IF)) {
        if (consume(&ctk, LPAR)) {
            if (expr(ctk)) {
                if (consume(&ctk, RPAR)) {
                    if (stm(ctk)) {
                        consume(&ctk, ELSE);
                        stm(ctk);
                        return 1;
                    }
                }
            }
        }
    } else if(consume(&ctk, WHILE)){
        if (consume(&ctk, LPAR)) {
            if (expr(ctk)) {
                if (consume(&ctk, RPAR)) {
                    if (stm(ctk)) {
                        return 1;
                    }
                }
            }
        }
    } else if(consume(&ctk, FOR)){
        if (consume(&ctk, LPAR)){
            expr(ctk);
            if(consume(&ctk, SEMICOL)){
                expr(ctk);
                if (consume(&ctk, SEMICOL)) {
                    expr(ctk);
                    if (consume(&ctk, RPAR)) {
                        if(stm(ctk))
                        {
                            return 1;
                        }
                    }
                }
            }

        }
    } else if (consume(&ctk, BREAK)){
        if(consume(&ctk, SEMICOL)){
            return 1;
        }
    } else if (consume(&ctk, RETURN)){
        expr(ctk);
        if(consume(&ctk, SEMICOL)){
            return 1;
        }
    } else {
        expr(ctk);
        if(consume(&ctk, SEMICOL)){
            return 1;
        }
    }
    ctk=tkStart;
    return 0;
}

//arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl(Token *ctk)
{
    Token *tkStart=ctk;
    if(consume(&ctk,LBRACK))
    {
        expr(ctk);
        if(consume(&ctk,RBRACK))
        {
            return 1;
        }
    }
    ctk=tkStart;
    return 0;
}

//typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase(Token *ctk)
{
    Token *tkStart=ctk;
    if(consume(&ctk,INT))
        return 1;
    ctk=tkStart;
    if(consume(&ctk,DOUBLE))
        return 1;
    ctk=tkStart;
    if(consume(&ctk,CHAR))
        return 1;
    ctk=tkStart;
    if(consume(&ctk,STRUCT))
        return 1;
    ctk=tkStart;
    return 0;
}

//declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(Token *ctk)
{
    Token *tkStart=ctk;
    if(typeBase(ctk))
    {
        if(consume(&ctk,ID))
        {
            arrayDecl(ctk);
            while(consume(&ctk,COMMA))
            {
                if(consume(&ctk,ID))
                {
                    arrayDecl(ctk);
                }
            }
            if(consume(&ctk,SEMICOL))
                return 1;
        }
    }
    ctk=tkStart;
    return 0;
}

//funcArg: typeBase ID arrayDecl?
int funcArg(Token *ctk)
{
    Token *tkStart=ctk;
    if(typeBase(ctk))
    {
        if(consume(&ctk,ID))
        {
            arrayDecl(ctk);
            return 1;
        }
    }
    ctk=tkStart;
    return 0;
}

//declFunc: ( typeBase MUL? | VOID ) ID LPAR ( funcArg ( COMMA funcArg )* )? RPAR stmCompound ;
int declFunc(Token *ctk)
{
    Token *tkStart=ctk;
    int valid=0;
    if(typeBase(ctk)){
        consume(&ctk,MUL);
        valid = 1;
    } else if(consume(&ctk,VOID)){
        valid = 1;
    }

    if(valid && consume(&ctk,ID)){
        if(consume(&ctk,LPAR)){
            if(funcArg(ctk))
            {
                while(consume(&ctk,COMMA) && funcArg(ctk)){};
            }
            if(consume(&ctk,RPAR)){
                if(stmCompound(ctk)){
                    return 1;
                }
            }
        }
    }
    ctk=tkStart;
    return 0;
}

//declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct(Token *ctk)
{
    Token *tkStart=ctk;
    if(consume(&ctk,STRUCT)){
        if(consume(&ctk,ID)){
            if(consume(&ctk,LACC)){
                while(declVar(ctk)){};
                if(consume(&ctk,RACC)){
                    if(consume(&ctk,SEMICOL)){
                       return 1;
                    }
                }
            }
        }
    }
    ctk=tkStart;
    return 0;
}

//unit: ( declStruct | declFunc | declVar )* END ;
int ruleUnit(Token *ctk)
{
    Token *tkStart=ctk;
    while (!consume(&ctk, END)) {
        if (declStruct(ctk) || declFunc(ctk) || declVar(ctk)) {

        }
    }
    if ((consumedTk != NULL) && (consumedTk->code==END))
        return 1;
    ctk = tkStart;
    return 0;
}

int asint(Token * tokens)
{
    int sc;
    if(tokens!=NULL)
        sc = ruleUnit(tokens);
    return sc;
}
