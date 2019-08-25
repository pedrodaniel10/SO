CFLAGS=-g  -Wall -pedantic

all: i-banco

i-banco: i-banco.o taskprocesses.o contas.o commandlinereader.o
	gcc -pthread -o i-banco i-banco.o  taskprocesses.o contas.o commandlinereader.o

i-banco.o: i-banco.c
	gcc $(CFLAGS) -c i-banco.c

taskprocesses.o: taskprocesses.c taskprocesses.h
	gcc $(CFLAGS) -c taskprocesses.c

contas.o: contas.c contas.h
	gcc $(CFLAGS) -c contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc $(CFLAGS) -c commandlinereader.c

clean:
	rm *.o i-banco
