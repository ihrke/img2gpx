CC=gcc
COPTS=-pedantic -ansi -g
LD_ARGS=-lpng

img2gpx: main.o parser.o wplist.o linalg.o
	$(CC) -o img2gpx $(LD_ARGS) main.o parser.o wplist.o linalg.o

main.o: main.c parser.h wplist.h
	$(CC) $(COPTS) -c -o main.o main.c

parser.o: parser.c parser.h wplist.h
	$(CC) $(COPTS) -c -o parser.o parser.c
wplist.o: wplist.c wplist.h
	$(CC) $(COPTS) -c -o wplist.o wplist.c
linalg.o: linalg.c linalg.h
	$(CC) $(COPTS) -c -o linalg.o linalg.c

tags:
	cscope -b *.h *.c
	ctags -R *.h *.c

test: matrixtest.c linalg.o
	gcc -c -g matrixtest.c
	gcc -g -o test matrixtest.o linalg.o
