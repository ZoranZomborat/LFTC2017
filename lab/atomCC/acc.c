#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../alex/alex.h"

#define BUFFSIZE            50000
static char buff[BUFFSIZE];

extern int asint(Token * tokens);
extern Token* alex(char *buff);

Token *tokens;

int main(int argc, char * argv[]) {
    int nc, fd, sc;

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

    tokens=alex(buff);
    if(tokens==NULL){
        err("Error at alex");
    }
    printf("\nLexical analyzer produced tokens!\n");
    if ((sc = asint(tokens)) == 0) {
        err("Error at alex");
    }
    fprintf(stdout, "\nSyntactic analysis passed!\n");

    return 0;
}
