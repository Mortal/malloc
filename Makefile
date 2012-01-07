CFLAGS=-std=gnu99 -g

test: my_malloc.o test.o

my_malloc.o: my_malloc.c my_malloc.h
test.o: test.c my_malloc.h
