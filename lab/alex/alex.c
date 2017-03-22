#include "alex.h"
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


#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");
#define SAFEALLOCSZ(var,Type,size) if((var=(Type*)malloc(sizeof(Type)*size))==NULL)err("not enough memory");
#define BUFFSIZE			50000

static char buff[BUFFSIZE];
static Token *lastToken, *tokens;
static int line = 0;
static char * pCrtCh;

void err(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
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
    if (lastToken) {
        lastToken->next = tk;
    } else {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

char * createString(const char *start, char * curr) {
    char *s;
    int size = (curr - start + 1);
    SAFEALLOCSZ(s, char, size + 1);
    memcpy(s, start, size);
    s[size]='\0';
    return s;
}

int getNextToken() {
    int state = 0, nCh, i;
    char ch;
    const char *pStartCh;
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
                pStartCh = pCrtCh; // memoreaza inceputul charului
                pCrtCh++;
                state = 21;
            }
            else if (ch == '"'){
                pStartCh = pCrtCh; // memoreaza inceputul stringului
                pCrtCh++;
                state = 19;
            }
            else if (ch == ',')
            {
                pCrtCh++;
                addTk(COMMA,line);
                state=28;
            }
            else if (ch == ';')
            {
                pCrtCh++;
                addTk(SEMICOL,line);
                state = 29;
            }
            else if (ch == '(')
            {
                pCrtCh++;
                addTk(LPAR,line);
                state = 30;
            }
            else if (ch == ')')
            {
                pCrtCh++;
                addTk(RPAR,line);
                state = 31;
            }
            else if (ch == '[')
            {
                pCrtCh++;
                addTk(LBRACK,line);
                state = 32;
            }
            else if (ch == ']')
            {
                pCrtCh++;
                addTk(RBRACK,line);
                state = 33;
            }
            else if (ch == '{')
            {
                pCrtCh++;
                addTk(LACC,line);
                state = 34;
            }
            else if (ch == '}')
            {
                pCrtCh++;
                addTk(RACC,line);
                state = 35;
            }
            else if (ch == '+')
            {
                pCrtCh++;
                addTk(ADD,line);
                state = 36;
            }
            else if (ch == '-')
            {
                pCrtCh++;
                addTk(SUB,line);
                state = 37;
            }
            else if (ch == '*')
            {
                pCrtCh++;
                addTk(MUL,line);
                state = 38;
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
                char * s=createString(pStartCh,pCrtCh);
                tk->intnum = strtol(s, NULL, 0);
                return tk->code;
            }
            break;
        case 2:
            if(ch == '8' || ch == '9')
            {
                pCrtCh++;
                state = 7;
            }
            else if (ch >= '0' || ch <= '7')
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
                char * s=createString(pStartCh,pCrtCh);
                tk->intnum = strtol(s, NULL, 0);
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
                char * s=createString(pStartCh,pCrtCh);
                tk->intnum = strtol(s, NULL, 0);
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
                char * s = createString(pStartCh,pCrtCh);
                tk->intnum = strtol(s, NULL, 0);
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
                char * s=createString(pStartCh,pCrtCh);
                addTk(CT_REAL, line);
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
                char * s=createString(pStartCh,pCrtCh);
                addTk(CT_REAL, line);
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
                int strSize = sizeof(atomNames[i]);
                if (nCh == strSize && !memcmp(pStartCh, atomNames[i], strSize)){
                    tk = addTk(i, line);
                    break;
                }
            }
            tk = addTk(ID, line);
            tk->text = createString(pStartCh, pCrtCh);
            return tk->code;
        default:
            err("wrong untreated state : ",state);
            break;
        }
    }
    return 0;
}

int main(int argc, char * argv[]) {
    int nc;
    int fd;
    int tkcode;
    if (argc != 2) {
        fprintf(stderr, "Usage %s filename!\n", argv[0]);
        exit(1);
    }

    if((fd=open(argv[1], O_RDONLY)) < 0)
    {
        printf("err at open\n");
    }

    if ((nc = read(fd, buff, BUFFSIZE)) < 0) {
        err("fread return NULL\n");
    }

    pCrtCh=buff;
    while((tkcode=getNextToken())!=END)
    {
        printf("%s\n",atomNames[tkcode]);
    }

    return 0;
}
