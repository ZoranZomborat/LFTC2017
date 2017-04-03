#include <stdlib.h>
#include <stdio.h>
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
                err(" exprPrimary missing %s at line %d", atomNames[RPAR], crtTk->line);
            }
        }
        return 1;
    } else if (consume(CT_INT) || consume(CT_REAL) ||
            consume(CT_CHAR) || consume(CT_STRING)){
        return 1;
    } else if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return 1;
            } else
                err(" exprPrimary missing %s at line %d", atomNames[RPAR], crtTk->line);
        } else
            err(" exprPrimary missing expr after %s at line %d", atomNames[LPAR], crtTk->line);
    }

    crtTk = tkStart;
    return 0;
}

/*exprPostfix1: LBRACK expr RBRACK exprPostfix1
          | DOT ID exprPostfix1 | eps*/
int exprPostfix1() {
    Token *tkStart = crtTk;
    if (consume(LBRACK)) {
        if (expr()) {
            if (consume(RBRACK)) {
                if (exprPostfix1()) {
                    return 1;
                }
            } else err(" exprPostfix1 missing %s at line %d", atomNames[RBRACK], crtTk->line);
        } else err(" exprPostfix1 missing expr after %s at line at line %d", atomNames[LBRACK], crtTk->line);
    }
    crtTk = tkStart;
    if (consume(DOT)) {
        if (consume(ID)) {
            if (exprPostfix1()) {
                return 1;
            }
        } else err(" exprPostfix1 missing %s at line %d at line %d", atomNames[ID], crtTk->line, crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprPostfix: exprPrimary exprPostfix1;
int exprPostfix(){
    Token *tkStart = crtTk;
    if(exprPrimary()){
       if(exprPostfix1()){
           return 1;
       }
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

//exprMul1: ( MUL | DIV ) exprCast exprMul1 | eps
int exprMul1(){
    Token *tkStart = crtTk;
    if(consume(MUL) || consume(DIV)){
        if(exprCast()){
            if(exprMul1()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 1;
}

//exprMul: exprCast exprMul1;
int exprMul(){
    Token *tkStart = crtTk;
    if(exprCast())
    {
        if(exprMul1()){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprAdd1: ( ADD | SUB ) exprMul exprAdd1 | eps
int exprAdd1(){
    Token *tkStart = crtTk;
    if(consume(ADD) || consume(SUB)){
        if(exprMul()){
            if(exprAdd1()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 1;
}

//exprAdd: exprMul exprAdd1;
int exprAdd(){
    Token *tkStart = crtTk;
    if(exprMul())
    {
        if(exprAdd1()){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprRel1: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRel1 | eps
int exprRel1(){
    Token *tkStart = crtTk;
    if( consume(LESS) || consume(LESSEQ)
            || consume(GRT) || consume(GRTEQ)){
        if(exprAdd()){
            if(exprRel1()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 1;
}

//exprRel: exprAdd exprRel1;
int exprRel(){
    Token *tkStart = crtTk;
    if(exprAdd())
    {
        if(exprRel1()){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprEq1: ( EQUAL | NOTEQ ) exprRel exprEq1 | eps
int exprEq1(){
    Token *tkStart = crtTk;
    if(consume(EQUAL) || consume(NEQUAL)){
        if(exprRel()){
            if(exprEq1()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 1;
}

//exprEq: exprRel exprEq1;
int exprEq(){
    Token *tkStart = crtTk;
    if(exprRel())
    {
        if(exprEq1()){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprAnd1: AND exprEq exprAnd1 | eps
int exprAnd1(){
    Token *tkStart = crtTk;
    if(consume(AND)){
        if(exprEq()){
            if(exprAnd1()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 1;
}

//exprAnd: exprEq exprAnd1;
int exprAnd(){
    Token *tkStart = crtTk;
    if(exprEq())
    {
        if(exprAnd1()){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprOr1: OR exprAND exprOr1 | eps
int exprOr1(){
    Token *tkStart = crtTk;
    if(consume(OR)){
        if(exprAnd()){
            if(exprOr1()){
                return 1;
            }
        }
    }
    crtTk = tkStart;
    return 1;
}

//exprOr: exprAnd exprOr1;
int exprOr(){
    Token *tkStart = crtTk;
    if(exprAnd())
    {
        if(exprOr1()){
            return 1;
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
        while (declVar() || stm()) {
        };
        if(consume(RACC)){
            return 1;
        }else err(" stmCompound missing %s at line %d", atomNames[RACC], crtTk->line);
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
                        if(consume(ELSE)){
                            if (stm()){
                            } else err(" missing stm after %s at line %d", atomNames[ELSE], crtTk->line);
                        }
                        return 1;
                    }
                }else err(" stm missing %s at line %d", atomNames[RPAR], crtTk->line);
            }
        }
    } else if(consume(WHILE)){
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (stm()) {
                        return 1;
                    }
                } else err(" stm missing %s at line %d", atomNames[RPAR], crtTk->line);
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
                    }else err(" stm missing %s at line %d", atomNames[RPAR], crtTk->line);
                } else err(" stm missing %s at line %d", atomNames[SEMICOL], crtTk->line);
            } else err(" stm missing %s at line %d", atomNames[SEMICOL], crtTk->line);

        }
    } else if (consume(BREAK)){
        if(consume(SEMICOL)){
            return 1;
        } else err(" stm missing %s at line %d", atomNames[SEMICOL], crtTk->line);
    } else if (consume(RETURN)){
        expr();
        if(consume(SEMICOL)){
            return 1;
        } else err(" stm missing %s at line %d", atomNames[SEMICOL], crtTk->line);
    } else {
        int exprFound;
        exprFound = expr();
        if (consume(SEMICOL)) {
            return 1;
        } else if (exprFound) {
            err(" stm missing %s at line %d", atomNames[SEMICOL], crtTk->line);
        }
    }
    crtTk=tkStart;
    return 0;
}

//arrayDecl: LBRACKET expr? RBRACKET ;

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
        } else err(" arrayDecl missing %s at line %d", atomNames[RBRACK], crtTk->line);
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
    if(consume(STRUCT)){
        if(consume(ID)){
            return 1;
        }
    }
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
                } else err(" declVar missing %s at line %d", atomNames[ID], crtTk->line);
            }
            if(consume(SEMICOL))
                return 1;
            else err(" declVar missing %s at line %d", atomNames[SEMICOL], crtTk->line);
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
                while(consume(COMMA)){
                    if(funcArg()){

                    } else err(" declFunc missing %s at line %d", "funcArg", crtTk->line);
                };
            }
            if(consume(RPAR)){
                if(stmCompound()){
                    return 1;
                } else err(" declFunc missing %s after %s at line %d", "stmCompound", atomNames[RPAR], crtTk->line);
            }else err(" declFunc missing %s at line %d", atomNames[RPAR], crtTk->line);
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
                    }  else err(" declStruct missing %s at line %d", atomNames[SEMICOL], crtTk->line);
                }  else err(" declStruct missing %s at line %d", atomNames[RACC], crtTk->line);
            }
        }
    }
    crtTk=tkStart;
    return 0;
}

//unit: ( declStruct | declFunc | declVar )* END ;

//unit: ( declStruct | declFunc | declVar )* END ;
int ruleUnit()
{
    Token *tkStart=crtTk;
    while (!consume(END)) {
        if (declStruct()) {
#ifdef DEBUG
            printf("declStruct\n");
#endif
        } else if (declFunc()) {
#ifdef DEBUG
            printf("declFunc\n");
#endif
        } else if (declVar()) {
#ifdef DEBUG
            printf("declVar\n");
#endif
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
