#include "alex.h"
#include "alex_utils.h"
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static Token *lastToken, *tokens;
static int line = 0;
static char * pCrtCh;
static char * satomList = ",;()[]{}+-*.";
static int tokenLine=0;

void err(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

void tkerr(Token *tk, const char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fprintf(stderr, "at line %d", tk->line);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

Token *addTk(int code, int line) {
    Token *tk;
    SAFEALLOC(tk, Token)
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    tk->infoType=TK_NONE;
    if (lastToken) {
        lastToken->next = tk;
    } else {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

char *toLiteral(char *s)
{
    char *lit;
    int i, j=0, len=strlen(s);
    lit=(char *)malloc(strlen(s));
    for(i=0;i<len;i++)
    {
        if(s[i]!='\\')
        {
            lit[j]=s[i];
            j++;
        } else
        {
            i++;
            switch(s[i]){ //abfnrtv'?\"\\0
            case 'a':
                lit[j]='\a';
                break;
            case 'b':
                lit[j] = '\b';
                break;
            case 'f':
                lit[j] = '\f';
                break;
            case 'n':
                lit[j] = '\n';
                break;
            case 'r':
                lit[j] = '\r';
                break;
            case 't':
                lit[j] = '\t';
                break;
            case 'v':
                lit[j] = '\v';
                break;
            case '\\':
                lit[j] = '\\';
                break;
            case '\'':
                lit[j] = '\'';
                break;
            case '"':
                lit[j] = '\"';
                break;
            case '?':
                lit[j] = '\?';
                break;
            case '0':
                lit[j] = '\0';
                break;
            default :
                err("error at literal conversion");
                break;
            }
            j++;
        }
    }
    lit[j]='\0';
    return lit;
}

char * createString(const char *start, char * curr) {
    char *s;
    int size = (curr - start + 1);
    SAFEALLOCSZ(s, char, size); //allocate also for NULL terminator
    memcpy(s, start, size);
    s[size - 1]='\0';
    s=toLiteral(s);
    return s;
}

void addTokenInfo(Token *tk, char * str, tokenInfoType type)
{
    if(tk==NULL)
        err("Null pointer to token");
    switch(type){
    case TK_STRING:
        tk->info.text = str;
        break;
    case TK_INT:
        tk->info.intnum = strtol(str, NULL, 0);
        break;
    case TK_FLOAT:
        tk->info.floatnum = strtof(str, NULL);
        break;
    default:
        err("Invalid token type");
        break;
    }
    tk->infoType=type;
}

void listToken(Token *tk)
{
    fprintf(stdout,"%s",atomNames[tk->code]);
    if(tk->infoType > 0)
    {
        switch(tk->infoType){
           case TK_STRING:
               fprintf(stdout,":[%s]",tk->info.text);
               break;
           case TK_INT:
               fprintf(stdout,":[%ld]",tk->info.intnum);
               break;
           case TK_FLOAT:
               fprintf(stdout,":[%f]",tk->info.floatnum);
               break;
           default:
               err("Invalid token type");
               break;
        }
    }
    fprintf(stdout,"\n");
}

void listTokenPerLines(Token *tk)
{
    if(tokenLine<tk->line){
        fprintf(stdout,"\n");
        tokenLine=tk->line;
    }
    fprintf(stdout,"%s",atomNames[tk->code]);
    if(tk->infoType > 0)
    {
        switch(tk->infoType){
           case TK_STRING:
               fprintf(stdout,":[%s]",tk->info.text);
               break;
           case TK_INT:
               fprintf(stdout,":[%ld]",tk->info.intnum);
               break;
           case TK_FLOAT:
               fprintf(stdout,":[%f]",tk->info.floatnum);
               break;
           default:
               err("Invalid token type");
               break;
        }
    }

    fprintf(stdout," ");
}

int getNextToken() {
    int state = 0, nCh, i;
    char ch;
    const char *pStartCh;
    char *s, *pos;
    Token *tk;
    while (1) { // bucla infinita
        ch = *pCrtCh;
        switch (state) {
        case 0: // testare tranzitii posibile din starea 0
            if ( ch == '0'){
                pStartCh= pCrtCh; // memoreaza inceputul numarului
                pCrtCh++;
                state=2;
            }
            else if ( ch >= '0' && ch <= '9'){
                pStartCh= pCrtCh; // memoreaza inceputul numarului
                pCrtCh++;
                state=1;
            }
            else if (isalpha(ch) || ch == '_') {
                pStartCh = pCrtCh; // memoreaza inceputul ID-ului
                pCrtCh++; // consuma caracterul
                state = 16;
            }
            else if (ch == '\''){
                pCrtCh++;
                pStartCh = pCrtCh; // memoreaza inceputul charului
                state = 18;
            }
            else if (ch == '"'){
                pCrtCh++;
                pStartCh = pCrtCh; // memoreaza inceputul stringului
                state = 22;
            }
            else if (ch == ' ' || ch == '\r' || ch == '\t') { //SPACE
                pCrtCh++;
            }
            else if (ch == '\n') { // tratat separat pentru a actualiza linia curenta
                line++;
                pCrtCh++;
            }
            else if (ch == '\0') { // sfarsit de sir
                addTk(END, line);
                return END;
            }
            else if ((pos=strchr(satomList,ch))!=NULL)
            {
                pCrtCh++;
                tk = addTk((FIRST_SATOM + (pos - satomList)), line);
                return tk->code;
            }
            else if (ch == '/')
            {
                pCrtCh++;
                state = 39;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                addTk(DOT,line);
                state = 40;
            }
            else if (ch == '&') {
                pCrtCh++;
                state = 41;
            }
            else if (ch == '|') {
                pCrtCh++;
                state = 43;
            }
            else if (ch == '!') {
                pCrtCh++;
                state = 45;
            }
            else if (ch == '=') {
                pCrtCh++;
                state = 47;
            }
            else if (ch == '<') {
                pCrtCh++;
                state = 51;
            }
            else if (ch == '>') {
                pCrtCh++;
                state = 54;
            }
            else
                err("caracter invalid");
            break;
        case 1:
            if(isdigit(ch))
            {
                pCrtCh++;
                state = 1;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 11;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 9;
            }
            else
            {
                tk = addTk(CT_INT, line);
                s = createString(pStartCh, pCrtCh);
                addTokenInfo(tk, s, TK_INT);
                return tk->code;
            }
            break;
        case 2:
            if(ch == '8' || ch == '9')
            {
                pCrtCh++;
                state = 7;
            }
            else if (ch >= '0' && ch <= '7')
            {
                pCrtCh++;
                state = 5;
            }
            else if (ch == 'x'  || ch == 'X')
            {
                pCrtCh++;
                state = 3;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 11;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 9;
            }
            else
            {
                tk = addTk(CT_INT, line);
                s=createString(pStartCh,pCrtCh);
                addTokenInfo(tk, s, TK_INT);
                return tk->code;
            }
            break;
        case 3:
            if(isalnum(ch))
            {
                pCrtCh++;
                state=4;
            }
            else
                err("error at line ", line, " state " ,state);
            break;
        case 4:
            if(isalnum(ch))
            {
                pCrtCh++;
                state=4;
            }
            else
            {
                tk = addTk(CT_INT, line);
                s=createString(pStartCh,pCrtCh);
                addTokenInfo(tk, s, TK_INT);
                return tk->code;
            }
            break;
        case 5:
            if(ch >= '0' && ch <= '7')
            {
                pCrtCh++;
                state=5;
            }
            else if (ch == '8' || ch == '9')
            {
                pCrtCh++;
                state=7;
            }
            else
            {
                tk = addTk(CT_INT, line);
                s = createString(pStartCh,pCrtCh);
                addTokenInfo(tk, s, TK_INT);
                return tk->code;
            }
            break;
        case 7:
            if(isdigit(ch))
            {
                pCrtCh++;
                state=7;
            }
            else if (ch == '8' || ch == '9')
            {
                pCrtCh++;
                state=7;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 11;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 9;
            }
            else
                err("error at line ", line, " state " ,state);
            break;
        case 9:
            if(isdigit(ch))
            {
                pCrtCh++;
                state = 14;
            }
            else
                err("error at line ", line, " state " ,state);
            break;
        case 14:
            if(isdigit(ch))
            {
                pCrtCh++;
                state = 14;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 11;
            }
            else
            {
                tk = addTk(CT_REAL, line);
                s=createString(pStartCh,pCrtCh);
                addTokenInfo(tk, s, TK_FLOAT);
                return tk->code;
            }
            break;
        case 11:
            if(ch == '+' || ch == '-')
            {
                pCrtCh++;
                state = 12;
            }
            else
                state = 12;
            break;
        case 12:
            if(isdigit(ch))
            {
                pCrtCh++;
                state = 13;
            }
            else
                err("error at line ", line, " state " ,state);
            break;
        case 13:
            if(isdigit(ch))
            {
                pCrtCh++;
                state = 13;
            }
            else
            {
                tk = addTk(CT_REAL, line);
                s=createString(pStartCh,pCrtCh);
                addTokenInfo(tk, s, TK_FLOAT);
                return tk->code;
            }
            break;
        case 16:
            if(isalnum(ch) || ch == '_')
            {
                pCrtCh++;
                state = 16;
            }
            else
                state = 17;
            break;
        case 17:
            nCh = pCrtCh - pStartCh; // lungimea cuvantului gasit
            for (i = FIRST_KEYW; i < LAST_KEYW; i++) {
                int strSize = strlen(keyNames[i]);
                if (nCh == strSize && !memcmp(pStartCh, keyNames[i], strSize)){
                    tk = addTk(i, line);
                    return tk->code;
                }
            }
            tk = addTk(ID, line);
            s = createString(pStartCh, pCrtCh);
            addTokenInfo(tk, s, TK_STRING);
            return tk->code;
            break ;
        case 18:
            if(ch == '\\'){
                pCrtCh++;
                state=19;
            }else if(ch != '\''){
                pCrtCh++;
                state=20;
            } else {
                pCrtCh++;
                state=21;
            }
            break;
        case 19:
            if(strchr("abfnrtv'?\"\\0", ch)!=NULL){
                pCrtCh++;
                state=20;
            }else
                err("error at line ", line, " state " ,state);
            break;
        case 20:
            if(ch == '\''){
                tk = addTk(CT_CHAR, line);
                s = createString(pStartCh, pCrtCh);
                addTokenInfo(tk, s, TK_STRING);
                pCrtCh++;
                return tk->code;
            }else
                err("error at line ", line, " state " ,state);
            break;
        case 22:
            if (ch == '\\') {
                pCrtCh++;
                state = 23;
            } else if (ch != '"') {
                pCrtCh++;
                state = 22;
            } else {
                state = 25;
            }
            break;
        case 23:
            if (strchr("abfnrtv'?\"\\0", ch) != NULL) {
                pCrtCh++;
                state = 24;
            } else
                err("error at line ", line, " state ", state);
            break;
        case 24:
            if (ch == '"') {
                state = 25;
            } else
                state=22;
            break;
        case 25:
            tk = addTk(CT_STRING, line);
            s = createString(pStartCh, pCrtCh);
            addTokenInfo(tk, s, TK_STRING);
            pCrtCh++;
            return tk->code;
            break;
        case 39:
            if(ch == '*' ){
                pCrtCh++;
                state = 60;
            } else if( ch == '/'){
                pCrtCh++;
                state = 59;
            } else {
                tk = addTk(DIV, line);
                return tk->code;
            }
            break;
        case 59:
            if(strchr("\n\r\t\0", ch)==NULL){
                pCrtCh++;
                state = 59;
            } else {
                pCrtCh++;
                state = 0;
            }
            break;
        case 60:
            if(ch != '*'){
                pCrtCh++;
                state = 60;
            } else {
                pCrtCh++;
                state = 61;
            }
            break;
        case 61:
            if(ch == '*'){
                pCrtCh++;
                state = 61;
            } else if(ch != '*' && ch != '/'){
                pCrtCh++;
                state = 60;
            } else {
                pCrtCh++;
                state = 0;
            }
            break;
        case 41:
            if(ch == '&'){
                pCrtCh++;
                tk = addTk(AND, line);
                return tk->code;
            } else
                err("error at line ", line, " state " ,state);
            break;
        case 43:
            if(ch == '|'){
                pCrtCh++;
                tk = addTk(OR, line);
                return tk->code;
            } else
                err("error at line ", line, " state " ,state);
            break;
        case 45:
            if(ch == '='){
                pCrtCh++;
                tk = addTk(NEQUAL, line);
                return tk->code;
            } else {
                tk = addTk(NOT, line);
                return tk->code;
            }
            break;
        case 47:
            if(ch == '='){
                pCrtCh++;
                tk = addTk(EQUAL, line);
                return tk->code;
            } else {
                tk = addTk(ASSIGN, line);
                return tk->code;
            }
            break;
        case 51:
            if(ch == '='){
                pCrtCh++;
                tk = addTk(LESSEQ, line);
                return tk->code;
            } else {
                tk = addTk(LESS, line);
                return tk->code;
            }
            break;
        case 54:
            if(ch == '='){
                pCrtCh++;
                tk = addTk(GRTEQ, line);
                return tk->code;
            } else {
                tk = addTk(GRT, line);
                return tk->code;
            }
            break;
        default:
            err("wrong untreated state : ",state);
            break;
        }
    }
    return 0;
}

Token * alex(char *buff){
    Token *tk;
    int tkcode;

    pCrtCh = buff;
    while ((tkcode = getNextToken()) != END) {};

#ifdef DEBUG
    if (tokens != NULL) {
        tk = tokens;
        listTokenPerLines(tk);
        while (tk->next != NULL) {
            tk = tk->next;
            listTokenPerLines(tk);
        }
    }
#endif

    return tokens;
}
