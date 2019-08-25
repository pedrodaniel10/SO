/**********************************************************************************************************************
 *                           Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                           *
 *                            Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                            *
 *                                     Duarte Galvão, 83449 / Pedro Lopes, 83540                                      *
 **********************************************************************************************************************
 ****************************************************** i-banco.c *****************************************************
 **********************************************************************************************************************/

#include "commandlinereader.h"
#include "contas.h"
#include "taskprocesses.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXARGS 4
#define BUFFER_SIZE 100

int main (int argc, char** argv) {
	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];
	int numchild = 0;
	comando_t comando;

	inicializarTrincos();
	inicializarSemaforos();
	inicializarVariaveisCondicao();
	inicializarContas();
	inicializarTarefas();

	printf("Bem-vinda/o ao i-banco\n\n");
	signal(SIGUSR1, signalTerminarProcesso);
	while (1) {
		int numargs;
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);

        /* EOF (end of file) do stdin ou comando "sair" */
		if (numargs < 0 ||
			(numargs > 0 && strcmp(args[0], COMANDO_SAIR) == 0)) {

			if (numargs > 1 && strcmp(args[1], "agora") == 0) {
				errno = kill(0, SIGUSR1);

				switch (errno) {
					case EINVAL:
						perror("kill: argumento invalido");
						continue;
					case EPERM:
						perror("kill: processo nao tem permissao para enviar signal");
						continue;
					case ESRCH:
						perror("kill: processo inexistente");
						continue;
				}
			}

			esperarTarefas();
			esperarProcessos(&numchild);

			exit(EXIT_SUCCESS);
		}

		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;

		/* Debitar */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
				continue;
			}

			comando.operacao = DEBITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);
			fazerPedido(comando);
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
			fazerPedido(comando);
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
			fazerPedido(comando);
		}

		/* Ler Saldo */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}

			comando.operacao = LER_SALDO;
			comando.idConta1 = atoi(args[1]);
			fazerPedido(comando);
		}

		/* Simular */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0 && numargs == 2) {
			pid_t pid;

			if (numchild >= NUM_MAX_PROCESSOS) {
				printf("%s: Numero maximo de processos excedido.\n", COMANDO_SIMULAR);
				continue;
			}

			prepararSimulacao();

			pid = fork();
			if (pid < 0) {
				perror("Erro ao criar processo simular.\n");
				exit(EXIT_FAILURE);
			}
			else if (pid == 0) { /* processo filho */
				simular(atoi(args[1]));
				exit(EXIT_SUCCESS);
			}
			else { /* processo pai */
				numchild++;
			}
		}

		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	}
}
