.DEFAULT_GOAL := all

CFLAGS = -g -Wall -DDEBUG

ALEX_DIR = ./alex
ASIN_DIR = ./asin
ACC_DIR = ./atomCC

alex.o: $(ALEX_DIR)/alex.c
	gcc $(CFLAGS) $(ALEX_DIR)/alex.c -c

asin.o: $(ASIN_DIR)/asin.c
	gcc $(CFLAGS) $(ASIN_DIR)/asin.c -c 
	
acc.o: $(ACC_DIR)/acc.c
	gcc $(CFLAGS) $(ACC_DIR)/acc.c -c

all: alex.o asin.o acc.o
	gcc -o acc $^

clean:
	rm alex.o asin.o
