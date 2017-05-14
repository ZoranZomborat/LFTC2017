#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "asin.h"

static Token *consumedTk;
static Token *crtTk;
static Symbols *symbolsTab;
static Symbol *crtFunc;
static Symbol *crtStruct;
static int crtDepth=0;

void initSymbols(Symbols * symbols) {
    symbols->begin = NULL;
    symbols->fill = NULL;
    symbols->end = NULL;
}

Symbol *addSymbol(Symbols *symbols,const char * name, int cls) {
    Symbol *s;
    if (symbols->fill == symbols->end) { // create more room
        int count = symbols->end-symbols->begin;
        int n = count * 2; // double the room
        if (n == 0)
            n = 1; // needed for the initial case

        symbols->begin = (Symbol **) realloc(symbols->begin,
                n * sizeof(Symbol *));
        if (symbols->begin == NULL)
            err("not enough memory");
        symbols->fill = symbols->begin + count;
        symbols->end = symbols->begin + n;
    }
    SAFEALLOC(s, Symbol);
    *symbols->fill++ = s;
    s->name = name;
    s->cls = cls;
    s->depth = crtDepth;
    return s;
}

Symbol *findSymbol(Symbols *symbols, const char *name) {
    Symbol *s = NULL;
    int n = symbols->fill - symbols->begin;
    int i;
    for (i = 0; i < n; i++) {
        if (strlen(symbols->begin[i]->name) == strlen(name)) {
            if (!strncmp(symbols->begin[i]->name, name, strlen(name)))
                s = symbols->begin[i];
        }
    }
    return s;
}

void addVar(Token *tkName, Type t) {
    Symbol *s;
    if (crtStruct) {
        if (findSymbol(&crtStruct->members, tkName->info.text))
            err("symbol redefinition: %s at line %d", tkName->info.text, crtTk->line);
        s = addSymbol(&crtStruct->members, tkName->info.text, CLS_VAR);
    } else if (crtFunc) {
        s = findSymbol(symbolsTab, tkName->info.text);
        if (s && s->depth == crtDepth)
            err("symbol redefinition: %s at line %d", tkName->info.text, crtTk->line);
        s = addSymbol(symbolsTab, tkName->info.text, CLS_VAR);
        s->mem = MEM_LOCAL;
    } else {
        if (findSymbol(symbolsTab, tkName->info.text))
            err("symbol redefinition: %s at line %d", tkName->info.text, crtTk->line);
        s = addSymbol(symbolsTab, tkName->info.text, CLS_VAR);
        s->mem = MEM_GLOBAL;
    }
    s->type = t;
}

int deleteSymbolsAfter(Symbols *st, Symbol *symbol) {
    int i, j, n;
    n = st->fill - st->begin;
    for (i = 0; i < n; i++) {
        if (st->begin[i] == symbol) {
            for (j = n - 1; j > i; j--) {
                free(st->begin[j]);
            }
            st->fill = st->begin + i + 1;
            return 1;
        }
    }
    return 0;
}

char* getDepthFormat(Symbol *s) {
    char *str;
    int i, n = s->depth;
    str = (char *) malloc(n);
    for (i = 0; i < n; i++) {
        str[i] = '\t';
    }
    return str;
}

int listSymbols(Symbols *st) {
    int i, n;
    if (st == NULL)
        return 0;
    n = st->fill - st->begin;
    for (i = 0; i < n; i++) {
        printf("%s-%s\n", getDepthFormat(st->begin[i]), st->begin[i]->name);
        //if(st->begin[i]->cls==CLS_FUNC)
            //listSymbols(&(st->begin[i]->args));
        //if(st->begin[i]->cls==CLS_STRUCT)
            //listSymbols(&(st->begin[i]->members));
    }
    return 1;
}

Type createType(int typeBase, int nElements) {
    Type t;
    t.typeBase = typeBase;
    t.nElements = nElements;
    return t;
}

void cast(Type * dst, Type * src) {
    if (src->nElements > -1) {
        if (dst->nElements > -1) {
            if (src->typeBase != dst->typeBase)
                err("an array cannot be converted to an array of another type, at line %d", crtTk->line);
        } else {
            err("an array cannot be converted to a non-array, at line %d", crtTk->line);
        }
    } else {
        if (dst->nElements > -1) {
            err("a non-array cannot be converted to an array, at line %d", crtTk->line);
        }
    }
    switch (src->typeBase) {
    case TB_CHAR:
    case TB_INT:
    case TB_DOUBLE:
        switch (dst->typeBase) {
        case TB_CHAR:
        case TB_INT:
        case TB_DOUBLE:
            return;
        }
        break;
    case TB_STRUCT:
        if (dst->typeBase == TB_STRUCT) {
            if (src->s != dst->s)
                err("a structure cannot be converted to another one, at line %d", crtTk->line);
            return;
        }
        break;
    default:
        err("unknown source type!");
        break;
    }
}

Type getArithType(Type *s1, Type *s2) {
    Type t;
    t.typeBase = ((s1->typeBase < s2->typeBase)?s1->typeBase:s2->typeBase);
    t.s = s1->s;
    t.nElements = s1->nElements;
    return t;
}

Symbol *addExtFunc(const char * name, Type type) {
    Symbol *s = addSymbol(symbolsTab, name, CLS_EXTFUNC);
    s->type = type;

    initSymbols(&s->args);
    return s;
}

Symbol *addFuncArg(Symbol *func, const char *name, Type type) {
    Symbol *a = addSymbol(&func->args, name, CLS_VAR);
    a->type = type;
    return a;
}

void manageExtFunctions() {
    Symbol *s;
    //void put_s(char s[])    Afiseaza sirul de caractere dat
    s = addExtFunc("put_s", createType(TB_VOID, -1));
    addFuncArg(s, "s", createType(TB_CHAR, 0));
    //void get_s(char s[])    Cere de la tastatura un sir de caractere si il depune in s
    s = addExtFunc("get_s", createType(TB_VOID, -1));
    addFuncArg(s, "s", createType(TB_CHAR, 0));
    //void put_i(int i)       Afiseaza intregul „i”
    s = addExtFunc("put_i", createType(TB_VOID, -1));
    addFuncArg(s, "i", createType(TB_INT, -1));
    //int get_i()             Cere de la tastatura un intreg
    s = addExtFunc("get_i", createType(TB_INT, -1));
    //void put_d(double d)    Afiseaza nr real „d”
    s = addExtFunc("put_d", createType(TB_VOID, -1));
    addFuncArg(s, "d", createType(TB_DOUBLE, -1));
    //double get_d()          Cere de la tastatura un real
    s = addExtFunc("get_d", createType(TB_DOUBLE, -1));
    //void put_c(char c)      Afiseaza nr real „c”
    s = addExtFunc("put_c", createType(TB_VOID, -1));
    addFuncArg(s, "c", createType(TB_CHAR, -1));
    //char get_c()            Cere de la tastatura un real
    s = addExtFunc("get_c", createType(TB_CHAR, -1));
    //double seconds()        Returneaza un numar de secunde
    s = addExtFunc("seconds", createType(TB_DOUBLE, -1));
}

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
int exprPrimary(RetVal *rv){
    Token *tkStart = crtTk;
    Token *tkName;
    Symbol *s;
    Symbol **crtDefArg;
    RetVal arg;
    if(consume(ID)){
        tkName = consumedTk;
        {
            s = findSymbol(symbolsTab, tkName->info.text);
            if (!s)
                tkerr(crtTk, "undefined symbol %s", tkName->info.text);
            rv->type = s->type;
            rv->isCtVal = 0;
            rv->isLVal = 1;
        }
        if (consume(LPAR)) {
            {
                crtDefArg = s->args.begin;
                if (s->cls != CLS_FUNC && s->cls != CLS_EXTFUNC)
                    tkerr(crtTk, "call of the non-function %s", tkName->info.text);
            }
            if ( expr(&arg)){
                {
                    if (crtDefArg == s->args.end)
                        tkerr(crtTk, "too many arguments in call");
                    cast(&(*crtDefArg)->type, &(arg.type));
                    crtDefArg++;
                }
                while (consume(COMMA) && expr(&arg)) {
                    {
                        if (crtDefArg == s->args.end)
                            tkerr(crtTk, "too many arguments in call");
                        cast(&(*crtDefArg)->type, &(arg.type));
                        crtDefArg++;
                    }
                };
            }
            if (consume(RPAR)) {
                {
                    if (crtDefArg != s->args.end)
                        tkerr(crtTk, "too few arguments in call");
                    rv->type = s->type;
                    rv->isCtVal = rv->isLVal = 0;
                }
                return 1;
            } else{
                err(" exprPrimary missing %s at line %d", atomNames[RPAR], crtTk->line);
            }
        }
        return 1;
    } else if (consume(CT_INT)){
        Token *tki=consumedTk;
        {
            rv->type = createType(TB_INT, -1);
            rv->ctVal.i = tki->info.intnum;
            rv->isCtVal = 1;
            rv->isLVal = 0;
        }
        return 1;
    } else if (consume(CT_REAL)) {
        Token *tkr=consumedTk;
        {
            rv->type = createType(TB_DOUBLE, -1);
            rv->ctVal.d = tkr->info.floatnum;
            rv->isCtVal = 1;
            rv->isLVal = 0;
        }
        return 1;
    } else if (consume(CT_CHAR)){
        Token *tkc=consumedTk;
        {
            rv->type = createType(TB_CHAR, -1);
            rv->ctVal.str = tkc->info.text;
            rv->isCtVal = 1;
            rv->isLVal = 0;
        }
        return 1;
    } else if (consume(CT_STRING)){
        Token *tks=consumedTk;
        {
            rv->type = createType(TB_CHAR, 0);
            rv->ctVal.str = tks->info.text;
            rv->isCtVal = 1;
            rv->isLVal = 0;
        }
        return 1;
    } else if (consume(LPAR)) {
        if (expr(rv)) {
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
int exprPostfix1(RetVal *rv) {
    Token *tkStart = crtTk;
    RetVal rve;
    if (consume(LBRACK)) {
        if (expr(&rve)) {
            {
                if (rv->type.nElements < 0)
                    tkerr(crtTk, "only an array can be indexed");
                Type typeInt = createType(TB_INT, -1);
                cast(&typeInt, &rve.type);
                //rv->type = rv.type;
                rv->type.nElements = -1;
                rv->isLVal = 1;
                rv->isCtVal = 0;
            }
            if (consume(RBRACK)) {
                if (exprPostfix1(rv)) {
                    return 1;
                }
            } else err(" exprPostfix1 missing %s at line %d", atomNames[RBRACK], crtTk->line);
        } else err(" exprPostfix1 missing expr after %s at line at line %d", atomNames[LBRACK], crtTk->line);
    }
    crtTk = tkStart;
    if (consume(DOT)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            {
                Symbol *sStruct = rv->type.s;
                Symbol *sMember = findSymbol(&sStruct->members, tkName->info.text);
                if (!sMember)
                    tkerr(crtTk, "struct %s does not have a member %s ",
                            sStruct->name, tkName->info.text);
                rv->type = sMember->type;
                rv->isLVal = 1;
                rv->isCtVal = 0;
            }
            if (exprPostfix1(rv)) {
                return 1;
            }
        } else err(" exprPostfix1 missing %s at line %d at line %d", atomNames[ID], crtTk->line, crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprPostfix: exprPrimary exprPostfix1;
int exprPostfix(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprPrimary(rv)){
       if(exprPostfix1(rv)){
           return 1;
       }
    }
    crtTk = tkStart;
    return 0;
}

//typeName: typeBase arrayDecl?
int typeName(Type *type) {
    SAFEALLOC(type,Type);
    Token *tkStart = crtTk;
    if (typeBase(type)) {
        if (!arrayDecl(type)) {
            type->nElements = -1;
        }
        return 1;
    }
    crtTk = tkStart;
    return 0;
}

//exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(RetVal *rv)
{
    Token *tkStart = crtTk;
    RetVal rve;
    Type t;
    if(consume(LPAR)){
        if(typeName(&t)){
            if (consume(RPAR)) {
                if (exprCast(&rve)) {
                    {
                        cast(&t, &rve.type);
                        rv->type = t;
                        rv->isCtVal = rv->isLVal = 0;
                    }
                    return 1;
                }
            } else err(" exprCast missing %s at line %d", atomNames[RPAR], crtTk->line);
        } else err(" exprCast missing %s at line %d", "typeName", crtTk->line);
    } else if(exprUnary(rv)){
        return 1;
    }
    crtTk = tkStart;
    return 0;
}

//exprMul1: ( MUL | DIV ) exprCast exprMul1 | eps
int exprMul1(RetVal *rv){
    Token *tkStart = crtTk;
    RetVal rve;
    Token *tkOp;
    if(consume(MUL) || consume(DIV)){
        tkOp = consumedTk;
        if(exprCast(&rve)){
            {
                if (rv->type.nElements > -1 || rve.type.nElements > -1)
                    tkerr(crtTk, "an array cannot be multiplied or divided");
                if (rv->type.typeBase == TB_STRUCT
                        || rve.type.typeBase == TB_STRUCT)
                    tkerr(crtTk, "a structure cannot be multiplied or divided");
                rv->type = getArithType(&rv->type, &rve.type);
                rv->isCtVal = rv->isLVal = 0;
            }
            if(exprMul1(rv)){
                return 1;
            }
        } else err(" exprMul missing expr after %s at line %d", atomNames[tkStart->code], crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprMul: exprCast exprMul1;
int exprMul(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprCast(rv))
    {
        if(exprMul1(rv)){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprAdd1: ( ADD | SUB ) exprMul exprAdd1 | eps
int exprAdd1(RetVal *rv){
    Token *tkStart = crtTk;
    RetVal rve;
    Token *tkOp;
    if(consume(ADD) || consume(SUB)){
        tkOp = consumedTk;
        if(exprMul(&rve)){
            {
                if (rv->type.nElements > -1 || rve.type.nElements > -1)
                    tkerr(crtTk, "an array cannot be added or subtracted");
                if (rv->type.typeBase == TB_STRUCT
                        || rve.type.typeBase == TB_STRUCT)
                    tkerr(crtTk, "a structure cannot be added or subtracted");
                rv->type = getArithType(&rv->type, &rve.type);
                rv->isCtVal = rv->isLVal = 0;
            }
            if(exprAdd1(rv)){
                return 1;
            }
        } else
            err(" exprAdd missing expr after %s at line %d", atomNames[tkStart->code], crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprAdd: exprMul exprAdd1;
int exprAdd(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprMul(rv))
    {
        if(exprAdd1(rv)){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprRel1: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRel1 | eps
int exprRel1(RetVal *rv){
    Token *tkStart = crtTk;
    RetVal rve;
    Token *tkOp;
    if( consume(LESS) || consume(LESSEQ)
            || consume(GRT) || consume(GRTEQ)){
        tkOp = consumedTk;
        if(exprAdd(&rve)){
            {
                if (rv->type.nElements > -1 || rve.type.nElements > -1)
                    tkerr(crtTk, "an array cannot be compared");
                if (rv->type.typeBase == TB_STRUCT
                        || rve.type.typeBase == TB_STRUCT)
                    tkerr(crtTk, "a structure cannot be compared");
                rv->type = createType(TB_INT, -1);
                rv->isCtVal = rv->isLVal = 0;
            }
            if(exprRel1(rv)){
                return 1;
            }
        } else err(" exprRel missing expr after %s at line %d", atomNames[tkStart->code], crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprRel: exprAdd exprRel1;
int exprRel(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprAdd(rv))
    {
        if(exprRel1(rv)){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprEq1: ( EQUAL | NOTEQ ) exprRel exprEq1 | eps
int exprEq1(RetVal *rv){
    Token *tkStart = crtTk;
    RetVal rve;
    Token *tkOp;
    if(consume(EQUAL) || consume(NEQUAL)){
        tkOp = consumedTk;
        if(exprRel(&rve)){
            {
                if (rv->type.typeBase == TB_STRUCT
                        || rve.type.typeBase == TB_STRUCT)
                    tkerr(crtTk, "a structure cannot be compared");
                rv->type = createType(TB_INT, -1);
                rv->isCtVal = rv->isLVal = 0;
            }
            if(exprEq1(rv)){
                return 1;
            }
        } else err(" exprEq missing expr after %s at line %d", atomNames[tkStart->code], crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprEq: exprRel exprEq1;
int exprEq(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprRel(rv))
    {
        if(exprEq1(rv)){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprAnd1: AND exprEq exprAnd1 | eps
int exprAnd1(RetVal *rv){
    Token *tkStart = crtTk;
    RetVal rve;
    if(consume(AND)){
        if(exprEq(&rve)){
            {
                if (rv->type.typeBase == TB_STRUCT
                        || rve.type.typeBase == TB_STRUCT)
                    tkerr(crtTk, "a structure cannot be logically tested");
                rv->type = createType(TB_INT, -1);
                rv->isCtVal = rv->isLVal = 0;
            }
            if(exprAnd1(rv)){
                return 1;
            }
        } else err(" exprAnd missing expr after %s at line %d", atomNames[AND], crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprAnd: exprEq exprAnd1;
int exprAnd(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprEq(rv))
    {
        if(exprAnd1(rv)){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//exprOr1: OR exprAND exprOr1 | eps
int exprOr1(RetVal *rv){
    Token *tkStart = crtTk;
    RetVal rve;
    if(consume(OR)){
        if(exprAnd(&rve)){
            {
                if (rv->type.typeBase == TB_STRUCT
                        || rve.type.typeBase == TB_STRUCT)
                    err("a structure cannot be logically tested");
                rv->type = createType(TB_INT, -1);
                rv->isCtVal = rv->isLVal = 0;
            }
            if(exprOr1(rv)){
                return 1;
            }
        } else err(" exprOr missing expr after %s at line %d", atomNames[OR], crtTk->line);
    }
    crtTk = tkStart;
    return 1;
}

//exprOr: exprAnd exprOr1;
int exprOr(RetVal *rv){
    Token *tkStart = crtTk;
    if(exprAnd(rv))
    {
        if(exprOr1(rv)){
            return 1;
        }
    }
    crtTk= tkStart;
    return 0;
}

//( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(RetVal *rv)
{
    Token *tkStart = crtTk;
    Token *tkOp;
    if(consume(SUB)||consume(NOT))
    {
        tkOp = consumedTk;
        if(exprUnary(rv))
        {
            {
                if (tkOp->code == SUB) {
                    if (rv->type.nElements >= 0)
                        tkerr(crtTk, "unary '-' cannot be applied to an array");
                    if (rv->type.typeBase == TB_STRUCT)
                        tkerr(crtTk, "unary '-' cannot be applied to a struct");
                } else {  // NOT
                    if (rv->type.typeBase == TB_STRUCT)
                        tkerr(crtTk, "'!' cannot be applied to a struct");
                    rv->type = createType(TB_INT, -1);
                }
                rv->isCtVal = rv->isLVal = 0;
            }
            return 1;
        }
    } else if(exprPostfix(rv))
    {
        return 1;
    }
    crtTk= tkStart;
    return 0;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign(RetVal *rv)
{
    Token *tkStart = crtTk;
    RetVal rve;
    if(exprUnary(rv))
    {
        if(consume(ASSIGN))
        {
            if(exprAssign(&rve))
            {
                {
                    if (!rv->isLVal)
                        err("cannot assign to a non-lval, at line %d", crtTk->line);
                    if (rv->type.nElements > -1 || rve.type.nElements > -1)
                        err("the arrays cannot be assigned, at line %d", crtTk->line);
                    cast(&rv->type, &rve.type);
                    rv->isCtVal = rv->isLVal = 0;
                }
                return 1;
            }
        }
    }
    crtTk= tkStart;
    if(exprOr(rv))
    {
        return 1;
    }
    crtTk= tkStart;
    return 0;
}

//expr: exprAssign ;
int expr(RetVal *rv)
{
    Token *tkStart = crtTk;
    if(exprAssign(rv)){
        return 1;
    }
    crtTk= tkStart;
    return 0;
}

//stmCompound: LACC ( declVar | stm )* RACC ;
int stmCompound()
{
    Token *tkStart = crtTk;
    Symbol *start=symbolsTab->fill[-1];
    if (consume(LACC)) {
        {
            crtDepth++;
        }
        while (declVar() || stm()) {
        };
        if(consume(RACC)){
            {
                crtDepth--;
                deleteSymbolsAfter(symbolsTab, start);
            }
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
    RetVal rv;
    RetVal rv1, rv2, rv3;

    if (stmCompound()) {
        return 1;
    } else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr(&rv)) {
                {
                    if (rv.type.typeBase == TB_STRUCT)
                        tkerr(crtTk, "a structure cannot be logically tested");
                }
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
            if (expr(&rv)) {
                {
                    if (rv.type.typeBase == TB_STRUCT)
                        tkerr(crtTk, "a structure cannot be logically tested");
                }
                if (consume(RPAR)) {
                    if (stm()) {
                        return 1;
                    }
                } else err(" stm missing %s at line %d", atomNames[RPAR], crtTk->line);
            }
        }
    } else if(consume(FOR)){
        if (consume(LPAR)){
            expr(&rv1);
            if(consume(SEMICOL)){
                expr(&rv2);
                {
                    if (rv2.type.typeBase == TB_STRUCT)
                        tkerr(crtTk, "a structure cannot be logically tested");
                }
                if (consume(SEMICOL)) {
                    expr(&rv3);
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
        expr(&rv);
        {
            if(crtFunc->type.typeBase==TB_VOID)
                tkerr(crtTk,"a void function cannot return a value");
            cast(&crtFunc->type,&rv.type);
        }
        if(consume(SEMICOL)){
            return 1;
        } else err(" stm missing %s at line %d", atomNames[SEMICOL], crtTk->line);
    } else {
        int exprFound;
        exprFound = expr(&rv);
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
int arrayDecl(Type *type)
{
    Token *tkStart=crtTk;
    if(consume(LBRACK))
    {
        RetVal rv;
        if(expr(&rv)){
            // if an expression, get its value
            if (!rv.isCtVal)
                err("the array size is not a constant, at line %d", crtTk->line);
            if (rv.type.typeBase != TB_INT)
                err("the array size is not an integer, at line %d", crtTk->line);
            type->nElements = rv.ctVal.i;
        } else {
            type->nElements = 0;       // for now do not compute the real size
        }
        if(consume(RBRACK))
        {
            return 1;
        } else err(" arrayDecl missing %s at line %d", atomNames[RBRACK], crtTk->line);
    }
    crtTk=tkStart;
    return 0;
}

//typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase(Type *type)
{
    Token *tkStart=crtTk;
    if(consume(INT)){
        type->typeBase=TB_INT;
        return 1;
    }
    crtTk=tkStart;
    if(consume(DOUBLE)){
        type->typeBase=TB_DOUBLE;
        return 1;
    }
    crtTk=tkStart;
    if(consume(CHAR)){
        type->typeBase=TB_CHAR;
        return 1;
    }
    crtTk=tkStart;
    if(consume(STRUCT)){
        if(consume(ID)){
            {
                Token *tkName=consumedTk;
                Symbol *s = findSymbol(symbolsTab, tkName->info.text);
                if (s == NULL)
                    err("undefined symbol: %s at line %d", tkName->info.text, crtTk->line);
                if (s->cls != CLS_STRUCT)
                    err("%s is not a struct at line %d", tkName->info.text, crtTk->line);
                type->typeBase = TB_STRUCT;
                type->s = s;
            }
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
    Token* tkName;
    Type varType;
    if(typeBase(&varType))
    {
        if(consume(ID))
        {
            tkName = consumedTk;
            if(!arrayDecl(&varType)){
                varType.nElements=-1;
            }
            {
                addVar(tkName,varType);
            }
            while(consume(COMMA))
            {
                if(consume(ID))
                {
                    tkName = consumedTk;
                    if(!arrayDecl(&varType)){
                        varType.nElements=-1;
                    }
                    {
                        addVar(tkName,varType);
                    }
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
    Token* tkName;
    Type varType;
    if(typeBase(&varType))
    {
        if(consume(ID))
        {
            tkName = consumedTk;
            if(!arrayDecl(&varType)){
                varType.nElements=-1;
            }

            {
                Symbol *s = addSymbol(symbolsTab, tkName->info.text, CLS_VAR);
                s->mem = MEM_ARG;
                s->type = varType;
                s = addSymbol(&crtFunc->args, tkName->info.text, CLS_VAR);
                s->mem = MEM_ARG;
                s->type = varType;
            }

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
    Token* tkName;
    Type varType;
    int valid=0;
    if(typeBase(&varType)){
        if(consume(MUL)){
            varType.nElements=0;
        }
        else{
            varType.nElements=-1;
        }
        valid = 1;
    } else if(consume(VOID)){
        {
            varType.typeBase=TB_VOID;
        }
        valid = 1;
    }

    if(valid && consume(ID)){
        tkName = consumedTk;
        if(consume(LPAR)){
            {
                if (findSymbol(symbolsTab, tkName->info.text))
                    err("symbol redefinition: %s at line %d", tkName->info.text, crtTk->line);
                crtFunc = addSymbol(symbolsTab, tkName->info.text, CLS_FUNC);
                initSymbols(&crtFunc->args);
                crtFunc->type = varType;
                crtDepth++;
            }
            if(funcArg())
            {
                while(consume(COMMA)){
                    if(funcArg()){

                    } else err(" declFunc missing %s at line %d", "funcArg", crtTk->line);
                };
            }
            if(consume(RPAR)){
                {
                    crtDepth--;
                }
                if(stmCompound()){
                    {
                        deleteSymbolsAfter(symbolsTab, crtFunc);
                        crtFunc = NULL;
                    }
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
    Token *tkName;
    if(consume(STRUCT)){
        if(consume(ID)){
            tkName=consumedTk;
            if (consume(LACC)){
                {
                    if (findSymbol(symbolsTab, tkName->info.text)!=NULL)
                        err("symbol redefinition: %s at line %d", tkName->info.text, tkName->line);
                    crtStruct = addSymbol(symbolsTab, tkName->info.text, CLS_STRUCT);
                    initSymbols(&crtStruct->members);
                }
                while(declVar()){};
                if (consume(RACC)) {
                    if (consume(SEMICOL)) {
                        {
                            crtStruct = NULL;
                        }
                        return 1;
                    } else err(" declStruct missing %s at line %d", atomNames[SEMICOL], crtTk->line);
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
        } else{
            err(" ruleUnit no declaration found before %s at line %d", atomNames[END], crtTk->line);
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
    SAFEALLOC(symbolsTab,Symbols);
    initSymbols(symbolsTab);
    manageExtFunctions();
    if (tokens != NULL) {
        crtTk = tokens;
        sc = ruleUnit();
    }
#ifdef DEBUG
    printf("\nSymbol table:\n");
    listSymbols(symbolsTab);
#endif
    return sc;
}
