CFLAGS=-g  -Wall -pedantic

all: i-banco i-banco-terminal

i-banco: i-banco.o taskprocesses.o contas.o
	gcc -pthread -o i-banco i-banco.o  taskprocesses.o contas.o

i-banco-terminal: i-banco-terminal.o commandlinereader.o
	gcc -pthread -o i-banco-terminal i-banco-terminal.o commandlinereader.o

i-banco.o: i-banco.c taskprocesses.h global.h contas.h
	gcc $(CFLAGS) -c i-banco.c

i-banco-terminal.o: global.h commandlinereader.h
	gcc $(CFLAGS) -c i-banco-terminal.c

taskprocesses.o: taskprocesses.c taskprocesses.h global.h
	gcc $(CFLAGS) -c taskprocesses.c

contas.o: contas.c contas.h global.h
	gcc $(CFLAGS) -c contas.c

commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc $(CFLAGS) -c commandlinereader.c

clean:
	rm *.o i-banco i-banco-terminal *.txt i-banco-pipe i-banco-pipe-*
