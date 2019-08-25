/**************************************************************************************************
 *                 Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                 *
 *                  Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                  *
 *                           Duarte Galvão, 83449 / Pedro Lopes, 83540                            *
 **************************************************************************************************
 *************************************** i-banco-terminal.c ***************************************
 **************************************************************************************************/


#include "global.h"
#include "commandlinereader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define MAXARGS 4
#define BUFFER_SIZE 100
#define COMANDO_SAIR_TERMINAL "sair-terminal"

char *pipename_ibanco;
int i_banco_pipe;
int pipe_resposta;
comando_t comando;


void signalPipe(int s){
	printf("Comando nao efetuado. Perdeu-se ligacao com i-banco.\ni-banco-terminal vai terminar.\n");
	close(i_banco_pipe);
	exit(1);
}

/* Na forma como o i-banco esta e' impossivel ter o pipe de resposta quebrado
 * pelo que o read ficara sempre bloqueado e nunca retornara 0.
 * A unica forma de ficar quebrado e' este processo acabar abruptamente ou terminar,
 * neste caso se o i-banco receber o comando ele ira processar o comando e quando
 * tentar devolver resposta (como o pipe esta quebrado) recebe SIGPIPE que e prontamente
 * ignorado. */
int main(int argc, char **argv){
	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];
	char pipename[50];
	time_t t1,t2;

	signal(SIGPIPE, signalPipe);

	/* Ligacao ao pipe do i-banco. */
	if (argc == 2) {
		pipename_ibanco = argv[1];
		i_banco_pipe = open(argv[1],O_WRONLY);
	}
	else {
		printf("Sintaxe inválida, tente de novo.\n");
		exit(EXIT_FAILURE);
	}

	/* Criar pipe de resposta. */
	sprintf(pipename,"/tmp/i-banco-pipe-%d",getpid());
	unlink(pipename);

	if (mkfifo(pipename,0777) == -1) {
		perror("Erro ");
		exit(EXIT_FAILURE);
	}

	/* Pedir Ligacao i-banco. */
	comando.operacao = PEDIR_LIGACAO;
	comando.pid = getpid();
	write(i_banco_pipe, (void*) &comando, sizeof(comando_t));

	/* Abrir pipe de resposta. */
	pipe_resposta = open(pipename,O_RDONLY);

	/* Recebe o modelo do comando_t a utilizar para correta comunicacao. */
	if(read(pipe_resposta,(void*) &comando, sizeof(comando_t))==0){
        printf("O pipe de resposta esta partido, impossivel obter resposta.\ni-banco-terminal vai terminar.\n");
        exit(EXIT_FAILURE);
    }

	printf("Bem-vinda/o ao i-banco\n\n");
	while (1) {
		int numargs;
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */
		if (numargs < 0 ||
			(numargs > 0 && strcmp(args[0], COMANDO_SAIR) == 0)) {
				if (numargs > 1 && strcmp(args[1], "agora") == 0)
					comando.operacao=SAIR_AGORA;
				else
					comando.operacao=SAIR;

				write(i_banco_pipe,(void*) &comando, sizeof(comando_t));
		}

		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;

		/* Sair Terminal */
		else if (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0) {
			exit(EXIT_SUCCESS);
		}

		/* Debitar */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);

				continue;
			}

			comando.operacao = DEBITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);

			write(i_banco_pipe,(void*) &comando, sizeof(comando_t));
			t1 = time(NULL);
			if(read(pipe_resposta,(void*) &comando, sizeof(comando_t))==0){
                printf("O pipe de resposta esta partido, impossivel obter resposta.\ni-banco-terminal vai terminar.\n");
                exit(EXIT_FAILURE);
            }
			t2 = time(NULL);

			if (comando.result == CMD_NOT_OK)
				printf("%s(%d, %d): Erro\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
			else if (comando.result == CMD_OK)
				printf("%s(%d, %d): OK\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
			else
				printf("Erro ao efetuar comando\n");

			printf("Tempo de execucao: %.3f segundos\n\n", difftime(t2,t1));
		}

		/* Creditar */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			comando.operacao = CREDITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);

			write(i_banco_pipe,(void*) &comando, sizeof(comando_t));
			t1 = time(NULL);
			if(read(pipe_resposta,(void*) &comando, sizeof(comando_t))==0){
                printf("O pipe de resposta esta partido, impossivel obter resposta.\ni-banco-terminal vai terminar.\n");
                exit(EXIT_FAILURE);
            }
			t2 = time(NULL);

			if (comando.result == CMD_NOT_OK)
				printf("%s(%d, %d): Erro\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
			else if (comando.result == CMD_OK)
				printf("%s(%d, %d): OK\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
			else
				printf("Erro ao efetuar comando\n");

			printf("Tempo de execucao: %.3f segundos\n\n", difftime(t2,t1));
		}

		/* Transferir */
		else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {
			if (numargs < 4) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			comando.operacao = TRANSFERIR;
			comando.idConta1 = atoi(args[1]);
			comando.idConta2 = atoi(args[2]);
			comando.valor = atoi(args[3]);

			write(i_banco_pipe,(void*) &comando, sizeof(comando_t));
			t1 = time(NULL);
			if(read(pipe_resposta,(void*) &comando, sizeof(comando_t))==0){
                printf("O pipe de resposta esta partido, impossivel obter resposta.\ni-banco-terminal vai terminar.\n");
                exit(EXIT_FAILURE);
            }
			t2 = time(NULL);

			if (comando.result == CMD_NOT_OK)
				printf("Erro ao transferir %d da conta %d para a conta %d.\n", comando.valor, comando.idConta1, comando.idConta2);
			else if (comando.result == CMD_OK)
				printf("%s(%d, %d, %d): OK\n", COMANDO_TRANSFERIR, comando.idConta1, comando.idConta2, comando.valor);
			else
				printf("Erro ao efetuar comando\n");

			printf("Tempo de execucao: %.3f segundos\n\n", difftime(t2,t1));
		}

		/* Ler Saldo */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}

			comando.operacao = LER_SALDO;
			comando.idConta1 = atoi(args[1]);

			write(i_banco_pipe,(void*) &comando, sizeof(comando_t));
			t1 = time(NULL);
			if(read(pipe_resposta,(void*) &comando, sizeof(comando_t))==0){
                printf("O pipe de resposta esta partido, impossivel obter resposta.\ni-banco-terminal vai terminar.\n");
                exit(EXIT_FAILURE);
            }
			t2 = time(NULL);

			if (comando.result == CMD_NOT_OK)
				printf("%s(%d): Erro.\n", COMANDO_LER_SALDO, comando.idConta1);
			else if (comando.result == CMD_OK)
				printf("%s(%d): O saldo da conta é %d.\n", COMANDO_LER_SALDO, comando.idConta1, comando.valor);
			else
				printf("Erro ao efetuar comando\n");

			printf("Tempo de execucao: %.3f segundos\n\n", difftime(t2,t1));
		}

		/* Simular */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0 && numargs == 2) {
			comando.operacao = SIMULAR;
			comando.valor = atoi(args[1]);

			write(i_banco_pipe,(void*) &comando, sizeof(comando_t));
		}

		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	}
	
	close(i_banco_pipe);
	close(pipe_resposta);
	return 0;
}
