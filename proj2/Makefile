CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pthread -pedantic #add -Werror

all: proj2

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c $< -o proj2.o

proj2: proj2.o
	$(CC) $(CFLAGS) $< -o proj2

clean:
	rm *.o

win-clean:
	del *.o proj2.exe
