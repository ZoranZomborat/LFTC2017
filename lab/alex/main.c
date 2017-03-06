#include "alex.h"
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");
#define BUFFSIZE			50000

static FILE * fd;

static char buff[BUFFSIZE];
static Token *lastToken, *tokens;
static int line=0;
static char * pCrtCh;

bool isalpha(char ch)
{
	if ((ch > 'A'&& ch < 'Z') || (ch >'a'&& ch < 'z'))
		return true;
	return false;
}

bool isnum(char ch)
{
	if (ch > '0'&& ch < '9')
		return true;
	return false;
}

bool isalnum(char ch)
{
	if (isalpha(ch) || isnum(ch))
		return true;
	return false;
}

void err(const char *fmt,...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

Token *addTk(int code,int line)
{
	Token *tk;
	SAFEALLOC(tk, Token)
	tk->code = code;
	tk->line = line;
	tk->next = NULL;
	if (lastToken){
		lastToken->next = tk;
	}
	else{
		tokens = tk;
	}
	lastToken = tk;
	return tk;
}

char * createString(const char *start, char * curr)
{
	return NULL;
}

int getNextToken()
{
	int state = 0, nCh;
	char ch;
	const char *pStartCh;
	Token *tk;
	while (1){ // bucla infinita
		ch = *pCrtCh;
		switch (state){
		case 0: // testare tranzitii posibile din starea 0
			if (isalpha(ch) || ch == '_'){
				pStartCh = pCrtCh; // memoreaza inceputul ID-ului
				pCrtCh++; // consuma caracterul
				state = 1; // trece la noua stare
			}
			else if (ch == '='){
				pCrtCh++;
				state = 3;
			}
			else if (ch == ' ' || ch == '\r' || ch == '\t'){
				pCrtCh++; // consuma caracterul si ramane in starea 0
			}
			else if (ch == '\n'){ // tratat separat pentru a actualiza linia curenta
				line++;
				pCrtCh++;
			}
			else if (ch == 0){ // sfarsit de sir
				addTk(END,line);
				return END;
			}
			else err("caracter invalid");
			break;
		case 1:
			if (isalnum(ch) || ch == '_')pCrtCh++;
			else state = 2;
			break;
		case 2:
			nCh = pCrtCh - pStartCh; // lungimea cuvantului gasit
			// teste cuvinte cheie
			if (nCh == 5 && !memcmp(pStartCh, "break", 5))tk = addTk(BREAK,line);
			else if (nCh == 4 && !memcmp(pStartCh, "char", 4))tk = addTk(CHAR,line);
			// … toate cuvintele cheie …
			else{ // daca nu este un cuvant cheie, atunci e un ID
				tk = addTk(ID,line);
				tk->text = createString(pStartCh, pCrtCh);
			}
			return tk->code;
		case 3:
			if (ch == '='){
				pCrtCh++;
				state = 4;
			}
			else state = 5;
			break;
		case 4:
			addTk(EQUAL,line);
			return EQUAL;
		case 5:
			addTk(ASSIGN,line);
			return ASSIGN;
		}
	}
}



void main(int argc, char * argv[])
{
	int nc;
	if (argc != 2)
	{
		fprintf(stderr, "Usage %s filename!\n", argv[0]);
		exit(1);
	}

	fopen(argv[1], "r");

	if ((nc = fread(buff, sizeof(char), BUFFSIZE, fd)) < 0)
	{
		err("fread return NULL\n");
	}


	readInput();
}