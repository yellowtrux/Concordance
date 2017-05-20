CC=gcc
CFLAGS=-I.

concordance: concordance.o LineNumberArray.o
	$(CC) -o concordance concordance.o LineNumberArray.o -I.