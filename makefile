all:
	gcc -Wall -D_DEFAULT_SOURCE -o lsc lsc.c util/*.c

clean:
	rm -f ./lsc