CC=gcc
CFLAGS=-std=gnu99 -Wall -Werror -Wextra -pthread -pedantic

all: proj2

proj2.o: proj2.c
	$(CC) $(CFLAGS) -c $< -o proj2.o

proj2: proj2.o
	$(CC) $(CFLAGS) $< -o proj2

clean:
	rm *.o

win-clean:
	del *.o proj2.exe
