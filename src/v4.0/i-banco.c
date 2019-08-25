/**************************************************************************************************
 *                 Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                 *
 *                  Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                  *
 *                           Duarte Galvão, 83449 / Pedro Lopes, 83540                            *
 **************************************************************************************************
 ******************************************** i-banco.c *******************************************
 **************************************************************************************************/


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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Se o i-banco-pipe nao tiver mais writers, o i-banco fara close do pipe
 * e criara outro com o mesmo nome e bloquear-se-a na funcao open ate que
 * um terminal se ligue ao pipe.
 * Se um pipe de resposta tiver quebrado, o SIGPIPE recebido e ignorado.
 * Ou seja, efetua comando mas nao envia resposta que o comando foi efetuado
 * e estado da operacao (se correu bem ou nao) e possivel verificar no log.txt
 * pois apenas os comandos bem sucedidos sao escritos no log.txt */
int main (int argc, char** argv) {
	int numchild = 0, nrTerminais = 0;
	comando_t comando;
	pid_t pid;
	char pipename[] = "/tmp/i-banco-pipe";
	int i_banco_pipe;

	inicializarTrincos();
	inicializarSemaforos();
	inicializarVariaveisCondicao();
	inicializarContas();
	inicializarTarefas();

	signal(SIGUSR1, signalTerminarProcesso);
	signal(SIGPIPE, SIG_IGN);

	unlink(pipename);
	if (mkfifo(pipename, 0777) == -1) {
		perror("Erro ");
		exit(1);
	}

	i_banco_pipe = open(pipename, O_RDONLY);

	while(TRUE) {
		if (read(i_banco_pipe, (void*) &comando, sizeof(comando_t)) == 0) {
			close(i_banco_pipe);
			unlink(pipename);
			if (mkfifo(pipename, 0777) == -1)
				exit(1);
			i_banco_pipe= open(pipename, O_RDONLY);
			continue;
		}
		if (comando.operacao == PEDIR_LIGACAO){
			char pipenameResposta[50];
			sprintf(pipenameResposta,"/tmp/i-banco-pipe-%d",comando.pid);
			comando.ficheiro_i_banco = open(pipenameResposta, O_WRONLY);
			nrTerminais++;
			write(comando.ficheiro_i_banco, (void*) &comando, sizeof(comando_t));
		}
		else if (comando.operacao == SIMULAR) {
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
				char filename[50];
				int file;
				sprintf(filename,"./i-banco-sim-%d.txt",getpid());
				unlink(filename);
				close(STDOUT_FILENO);
				file = open(filename, O_CREAT | O_WRONLY, 0777);
				file = dup(file);
				close(ibanco_log);
                ibanco_log=-1;
				simular(comando.valor);
				close(file);
				exit(EXIT_SUCCESS);
			}
			else { /* processo pai */
				numchild++;
			}
		}
		else if (comando.operacao == SAIR) {
			int i;
			esperarTarefas();
			esperarProcessos(&numchild);

			if (nrTerminais != 0)
				for(i = 3; i < (nrTerminais+3); i++)
					close(i);

			exit(EXIT_SUCCESS);
		}
		else if (comando.operacao == SAIR_AGORA) {
			int i;
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
			esperarTarefas();
			esperarProcessos(&numchild);

			if (nrTerminais != 0)
				for (i = 3; i < (nrTerminais+3); i++)
					close(i);

			exit(EXIT_SUCCESS);
		}
		else {
			fazerPedido(comando);
		}

	}
	exit(EXIT_FAILURE);
}
