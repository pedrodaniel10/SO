/**********************************************************************************************************************
 *                           Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                           *
 *                            Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 1.                            *
 *                                     Duarte Galvão, 83449 / Pedro Lopes, 83540                                      *
 **********************************************************************************************************************
 ****************************************************** i-banco.c *****************************************************
 **********************************************************************************************************************/

#include "commandlinereader.h"
#include "contas.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"

#define MAXARGS 3
#define BUFFER_SIZE 100

#define NUM_MAX_PROCESSOS 20

/* Lista dos processos terminados na ordem em que acabaram */
#define P_TERMINADO_NORMALMENTE 0
#define P_TERMINADO_ABRUPTAMENTE 1

struct processoT {
	pid_t pid;
	int estado;
};

/* Funcao auxiliar para o "sair" e o "sair agora" */
void esperarProcessos();


int numchild = 0;

int main (int argc, char** argv) {
	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE];
	inicializarContas();

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

				if (errno == EINVAL) {
					perror("kill: argumento invalido");
					continue;
				}
				else if (errno == EPERM) {
					perror("kill: processo nao tem permissao para enviar signal");
					continue;
				}
				else if (errno == ESRCH) {
					perror("kill: processo inexistente");
					continue;
				}
			}

			esperarProcessos();
			exit(EXIT_SUCCESS);
		}

		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;

		/* Debitar */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
			int idConta, valor;
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
				continue;
			}

			idConta = atoi(args[1]);
			valor = atoi(args[2]);

			if (debitar (idConta, valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
		}

		/* Creditar */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			int idConta, valor;
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			idConta = atoi(args[1]);
			valor = atoi(args[2]);

			if (creditar (idConta, valor) < 0)
				printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);
			else
				printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
		}

		/* Ler Saldo */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			int idConta, saldo;

			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
			idConta = atoi(args[1]);
			saldo = lerSaldo (idConta);
			if (saldo < 0)
				printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
			else
				printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
		}

		/* Simular */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0 && numargs == 2) {
			pid_t pid;

			if (numchild >= NUM_MAX_PROCESSOS) {
				printf("%s: Numero maximo de processos excedido.\n", COMANDO_SIMULAR);
				continue;
			}

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

void esperarProcessos() {
	int estado, nrProcessosTerminados = 0, i;
	pid_t childpid;
	struct processoT processosT[NUM_MAX_PROCESSOS];

	/* esperar que os processos terminem */
	while (numchild > 0){
		childpid = wait(&estado);
		processosT[nrProcessosTerminados].pid = childpid;

		if (WIFSIGNALED(estado))
			processosT[nrProcessosTerminados].estado = P_TERMINADO_ABRUPTAMENTE;
		else
			processosT[nrProcessosTerminados].estado = P_TERMINADO_NORMALMENTE;

		nrProcessosTerminados++;
		numchild--;
	}
	
	/* imprimir estados da terminacao dos processos */
	printf("\ni-banco vai terminar.\n--\n");
	for (i = 0; i < nrProcessosTerminados; i++) {
		printf("FILHO TERMINADO (PID=%d; terminou ", processosT[i].pid);
		if (processosT[i].estado == P_TERMINADO_NORMALMENTE)
			printf("normalmente)\n");
		else
			printf("abruptamente)\n");
	}
	printf("--\ni-banco terminou.\n");		
}
