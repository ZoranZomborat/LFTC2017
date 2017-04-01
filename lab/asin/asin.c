#include <stdlib.h>
#include "asin.h"

static Token *consumedTk;
static Token *crtTk;

int consume(atomType code)
{
    if(crtTk->code==code)
    {
        consumedTk=crtTk;
        crtTk=crtTk->next;
        return 1;
    }
    return 0;
}

/*exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
           | CT_INT | CT_REAL | CT_CHAR
           | CT_STRING | LPAR expr RPAR ;*/
int exprPrimary(){
    Token *tkStart = crtTk;

    if(consume(ID)){
        if (consume(LPAR)) {
            if ( expr()){
                while (consume(COMMA) && expr()) {
                };
            }
            if (consume(RPAR)) {
                return 1;
            } else{
                err("err at exprPrimary missing ", atomNames[RPAR]);
            }
        }
        return 1;
    } else if (consume(CT_INT) || consume(CT_REAL) ||
            consume(CT_CHAR) || consume(CT_STRING)){
        return 1;
    } else if (consume(LPAR) && expr() && consume(RPAR)){
        return 1;
    }

    crtTk = tkStart;
    return 0;
}


/*exprPostfix: exprPostfix LBRACKET expr RBRACKET
           | exprPostfix DOT ID | exprPrimary ;*/
int exprPostfix(){
    Token *tkStart = crtTk;
    if(exprPrimary()){
        return 1;
    } else if(exprPostfix()){
        if(consume(DOT)){
            if(consume(ID)){
                return 1;
            }
        }else if(consume(LBRACK)){
            if(expr()){
                if(consume(RBRACK)){
                    return 1;
                }
            }
        }
        else
            err("err at exprPostfix");
    }
    crtTk = tkStart;
    return 0;
}

//typeName: typeBase arrayDecl?
int typeName()
{
    Token *tkStart = crtTk;
    if (typeBase()) {
        arrayDecl ();
        return 1;
    }
    crtTk = tkStart;
    return 0;
}

//exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast()
{
    Token *tkStart = crtTk;
    if(consume(LPAR)){
        if(typeName()){
            if(consume(RPAR)){
                if(exprCast()){
                    return 1;
                }
            }
        }
    } else if(exprUnary()){
        return 1;
    }
    crtTk = tkStart;
    return 0;
}

//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd ;
int exprRel()
{
    Token *tkStart = crtTk;
    if(exprAdd()){
        return 1;
    } else if (exprRel()){
        if( consume(LESS) || consume(LESSEQ)
                || consume(GRT) || consume(GRTEQ)){
            if(exprAdd()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 0;
}

//exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ;
int exprAdd()
{
    Token *tkStart = crtTk;
    if (exprMul()){
        return 1;
    } else if (exprAdd())
    {
        if(consume(ADD) || consume(SUB)){
            if (exprMul()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 0;
}

//exprMul: exprMul ( MUL | DIV ) exprCast | exprCast ;
int exprMul(){
    Token *tkStart = crtTk;
    if (exprCast()){
        return 1;
    } else if (exprMul())
    {
        if(consume(MUL) || consume(DIV)){
            if (exprCast()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 0;
}


//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel ;
int exprEq(){
    Token *tkStart = crtTk;
    if(exprRel())
    {
        return 1;
    }
    else if (exprEq()) {
        if(consume(EQUAL) || consume(NEQUAL))
        {
            if(exprRel()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 0;
}

//exprAnd: exprAnd AND exprEq | exprEq ;
int exprAnd(){
    Token *tkStart = crtTk;
    if(exprEq())
    {
        return 1;
    }
    else if(exprAnd()){
        if(consume(AND)){
            if(exprEq()){
                return 1;
            }
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprOr: exprOr OR exprAnd | exprAnd ;
int exprOr(){
    Token *tkStart = crtTk;
    if(exprAnd())
    {
        return 1;
    }
    else if(exprOr()){
        if(consume(OR)){
            if(exprAnd()){
                return 1;
            }
        }
    }
    crtTk= tkStart;
    return 0;
}

//( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary()
{
    Token *tkStart = crtTk;
    if(consume(SUB)||consume(NOT))
    {
        if(exprUnary())
        {
            return 1;
        }
    } else if(exprPostfix())
    {
        return 1;
    }
    crtTk= tkStart;
    return 0;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign()
{
    Token *tkStart = crtTk;
    if(exprUnary())
    {
        if(consume(ASSIGN))
        {
            if(exprAssign())
            {
                return 1;
            }
        }
    }
    crtTk= tkStart;
    if(exprOr())
    {
        return 1;
    }
    crtTk= tkStart;
    return 0;
}

//expr: exprAssign ;
int expr()
{
    Token *tkStart = crtTk;
    if(exprAssign()){
        return 1;
    }
    crtTk= tkStart;
    return 0;
}

//stmCompound: LACC ( declVar | stm )* RACC ;
int stmCompound()
{
    Token *tkStart = crtTk;
    if (consume(LACC)) {
        while (declVar() || stm()) {};
        if(consume(RACC)){
            return 1;
        }
    }
    crtTk = tkStart;
    return 0;
}


/*stm: stmCompound
           | IF LPAR expr RPAR stm ( ELSE stm )?
           | WHILE LPAR expr RPAR stm
           | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
           | BREAK SEMICOLON
           | RETURN expr? SEMICOLON
           | expr? SEMICOLON ;*/
int stm()
{
    Token *tkStart = crtTk;
    if (stmCompound()) {
        return 1;
    } else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        consume(ELSE);
                        stm();
                        return 1;
                    }
                }
            }
        }
    } else if(consume(WHILE)){
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        return 1;
                    }
                }
            }
        }
    } else if(consume(FOR)){
        if (consume(LPAR)){
            expr();
            if(consume(SEMICOL)){
                expr();
                if (consume(SEMICOL)) {
                    expr();
                    if (consume(RPAR)) {
                        if(stm())
                        {
                            return 1;
                        }
                    }
                }
            }

        }
    } else if (consume(BREAK)){
        if(consume(SEMICOL)){
            return 1;
        }
    } else if (consume(RETURN)){
        expr();
        if(consume(SEMICOL)){
            return 1;
        }
    } else {
        expr();
        if(consume(SEMICOL)){
            return 1;
        }
    }
    crtTk=tkStart;
    return 0;
}

//arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl()
{
    Token *tkStart=crtTk;
    if(consume(LBRACK))
    {
        expr();
        if(consume(RBRACK))
        {
            return 1;
        }
    }
    crtTk=tkStart;
    return 0;
}

//typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase()
{
    Token *tkStart=crtTk;
    if(consume(INT))
        return 1;
    crtTk=tkStart;
    if(consume(DOUBLE))
        return 1;
    crtTk=tkStart;
    if(consume(CHAR))
        return 1;
    crtTk=tkStart;
    if(consume(STRUCT))
        return 1;
    crtTk=tkStart;
    return 0;
}

//declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar()
{
    Token *tkStart=crtTk;
    if(typeBase())
    {
        if(consume(ID))
        {
            arrayDecl();
            while(consume(COMMA))
            {
                if(consume(ID))
                {
                    arrayDecl();
                }
            }
            if(consume(SEMICOL))
                return 1;
        }
    }
    crtTk=tkStart;
    return 0;
}

//funcArg: typeBase ID arrayDecl?
int funcArg()
{
    Token *tkStart=crtTk;
    if(typeBase())
    {
        if(consume(ID))
        {
            arrayDecl();
            return 1;
        }
    }
    crtTk=tkStart;
    return 0;
}

//declFunc: ( typeBase MUL? | VOID ) ID LPAR ( funcArg ( COMMA funcArg )* )? RPAR stmCompound ;
int declFunc()
{
    Token *tkStart=crtTk;
    int valid=0;
    if(typeBase()){
        consume(MUL);
        valid = 1;
    } else if(consume(VOID)){
        valid = 1;
    }

    if(valid && consume(ID)){
        if(consume(LPAR)){
            if(funcArg())
            {
                while(consume(COMMA) && funcArg()){};
            }
            if(consume(RPAR)){
                if(stmCompound()){
                    return 1;
                }
            }
        }
    }
    crtTk=tkStart;
    return 0;
}

//declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct()
{
    Token *tkStart=crtTk;
    if(consume(STRUCT)){
        if(consume(ID)){
            if(consume(LACC)){
                while(declVar()){};
                if(consume(RACC)){
                    if(consume(SEMICOL)){
                       return 1;
                    }
                }
            }
        }
    }
    crtTk=tkStart;
    return 0;
}

//unit: ( declStruct | declFunc | declVar )* END ;
int ruleUnit()
{
    Token *tkStart=crtTk;
    while (!consume(END)) {
        if (declStruct() || declFunc() || declVar()) {

        }
    }
    if ((consumedTk != NULL) && (consumedTk->code==END))
        return 1;
    crtTk = tkStart;
    return 0;
}

int asint(Token * tokens)
{
    int sc;
    if (tokens != NULL) {
        crtTk = tokens;
        sc = ruleUnit(tokens);
    }
    return sc;
}
