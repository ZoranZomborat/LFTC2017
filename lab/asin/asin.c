
#include "../alex/alex.h"
static Token *tokens;

Token *consumedTk;

int consume(Token* ctk,int code)
{
    if(ctk->code==code)
    {
        consumedTk=ctk;
        ctk=ctk->next;
        return 1;
    }
    return 0;
}
//arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl(Token *ctk)
{
    Token *tkStart=ctk;
    if(consume(ctk,LBRACK))
    {
        expr();
        if(consume(ctk,RBRACK))
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
    if(!consume(ctk,STRUCT)) return 0;
    if(!consume(ctk,ID)) return 0;
    if(!consume(ctk,LACC)) return 0;
    while(declVar(ctk)){};
    if(!consume(ctk,RACC)) return 0;
    if(!consume(ctk,SEMICOL)) return 0;
    return 1;
}

//declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar(Token *ctk)
{
    Token *tkStart=ctk;
    if(typeBase(ctk))
    {
        if(consume(ctk,ID))
        {
            arrayDecl();
            while(consume(ctk,COMMA))
            {
                if(consume(ctk,ID))
                {
                    arrayDecl();
                }
            }
            if(consume(ctk,SEMICOL))
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
        if(consume(ctk,ID))
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
    int valid = 0;
    if(typeBase(ctk)){
        consume(ctk,MUL);
        valid=1;
    }else if(comsume(ctk,VOID)){
        valid=1;
    }
    else
        err("err at declFunc missing ",atomNames[ID]);
    if(!consume(ctk,ID))     err("err at declFunc missing ",atomNames[ID]);
    if(!consume(ctk,LPAR))   err("err at declFunc missing ",atomNames[LPAR]);
    funcArg();
    while(consume(ctk,COMMA) && funcArg()){};
    if(!consume(ctk,RPAR))   err("err at declFunc missing ",atomNames[RPAR]);
    if(!stmCompound(ctk))    err("err at declFunc missing stmCompound");
    return 1;
}


//declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct(Token *ctk)
{
    if(!consume(ctk,STRUCT)) return 0;
    if(!consume(ctk,ID)) err("err at declStruct missing ",atomNames[ID]);
    if(!consume(ctk,LACC)) err("err at declStruct missing ",atomNames[LACC]);
    while(declVar(ctk)){};
    if(!consume(ctk,RACC)) err("err at declStruct missing ",atomNames[RACC]);
    if(!consume(ctk,SEMICOL)) err("err at declStruct missing ",atomNames[RACC]);
    return 1;
}

//unit: ( declStruct | declFunc | declVar )* END ;
int ruleUnit(Token *ctk)
{
    Token *tkStart=ctk;
    while (consume(ctk, END)) {
        if (declStruct(ctk) || declFunc(ctk) || declVar(ctk)) {

        }
    }
    if (consumedTk->code==END)
        return 1;
    return 0;
}

void asin(Token * tokens)
{

}
