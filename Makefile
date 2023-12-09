CC = gcc
CFLAGS = -Wall -Wextra

main: main.c menu.o network.o process.o loglib.o
	$(CC) $(CFLAGS) -o main main.c menu.o network.o process.o loglib.o -lcurses

loglib.o: loglib.c
	$(CC) $(CFLAGS) -c loglib.c
	
process.o : process.c
	$(CC) $(CFLAGS) -c process.c

menu.o: menu.c
	$(CC) $(CFLAGS) -c menu.c

network.o: network.c
	$(CC) $(CFLAGS) -c network.c


clean:
	rm -rf *.out *.o