mpextract: mpextract.o
	gcc -o mpextract mpextract.o

mpextract.o: mpextract.c
	gcc -c mpextract.c

