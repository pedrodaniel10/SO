/**********************************************************************************************************************
 *                           Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                           *
 *                            Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                            *
 *                                     Duarte Galvão, 83449 / Pedro Lopes, 83540                                      *
 **********************************************************************************************************************
 ************************************************** taskprocesses.c ***************************************************
 **********************************************************************************************************************/
#include "taskprocesses.h"

comando_t cmd_buffer[CMD_BUFFER_DIM];
int buff_write_idx = 0, buff_read_idx = 0;
int nrPedidos=0;
pthread_mutex_t trincoColocar, trincoRetirar, trincoNrPedidos, trincosContas[NUM_CONTAS];
sem_t semRetirar, semColocar;

pthread_t tarefas[NUM_TRABALHADORAS];

pthread_cond_t todosPedidosEfetuados;

void inicializarTarefas() {
	int i;
	for (i = 0; i < NUM_TRABALHADORAS; i++) {
		pthread_create(&tarefas[i], 0, efetuarComando, NULL);

		switch (errno) {
			case EAGAIN:
				perror("pthread_create: Nao ha recursos suficientes para criar tarefa.\n");
				break;
			case EINVAL:
				perror("pthread_create: Definicoes invalidas em attr.\n");
				break;
			case EPERM:
				perror("pthread_create: Sem permissao.\n");
				break;
		}
	}
}

void inicializarTrincos() {
	int i;

	/* Trincos dos Semaforos */
	pthread_mutex_init(&trincoColocar, NULL);
	pthread_mutex_init(&trincoRetirar, NULL);

	/* Trincos das Contas */
	for (i = 0; i < NUM_CONTAS; i++)
		pthread_mutex_init(&trincosContas[i], NULL);
	/* Trinco para usar a variavel nrPedidos*/
	pthread_mutex_init(&trincoNrPedidos, NULL);
}

void inicializarSemaforos() {
	criarSemaforo(&semRetirar, 0);
	criarSemaforo(&semColocar, CMD_BUFFER_DIM);
}

void inicializarVariaveisCondicao(){
	if (pthread_cond_init(&todosPedidosEfetuados,NULL)) {
		printf("Erro ao iniciar variavel de condicao.\n");
		exit(1);
	}
}

void criarSemaforo(sem_t* sem, unsigned int value) {
	sem_init(sem, 0, value);

	switch (errno) {
		case EINVAL:
			perror("sem_init: Valor excede SEM_VALUE_MAX.\n");
			break;
		case ENOSYS:
			perror("sem_init: Error ENOSYS.\n");
			break;
	}
}

void abrirTrinco(pthread_mutex_t* mutex) {
	pthread_mutex_unlock(mutex);

	if (errno == EINVAL)
		perror("pthread_mutex_unlock: O mutex nao foi inicializado corretamente.\n");
	else if (errno == EPERM)
		perror("pthread_mutex_unlock: A tarefa que chamou nao tem permissao.\n");
}

void fecharTrinco(pthread_mutex_t* mutex) {
	pthread_mutex_lock(mutex);

	if (errno == EINVAL)
		perror("pthread_mutex_lock: O mutex nao foi inicializado corretamente.\n");
	else if (errno == EDEADLK)
		perror("pthread_mutex_lock: O mutex ja se encontrava trancado.\n");
}



void esperarSemaforo(sem_t* sem) {
	sem_wait(sem);

	if (errno == EINTR)
		perror("sem_wait: Chamada interrompida por signal.\n");
	else if (errno == EINVAL)
		perror("sem_wait: Semaforo invalido.\n");
}

void assinalarSemaforo(sem_t* sem) {
	sem_post(sem);

	if (errno == EINVAL)
		perror("sem_post: Semaforo invalido.\n");
	else if (errno == EOVERFLOW)
		perror("sem_post: O valor maximo do semaforo seria excedido.\n");
}



void fazerPedido(comando_t comando) {
	esperarSemaforo(&semColocar);
	fecharTrinco(&trincoColocar);

	cmd_buffer[buff_write_idx] = comando;
	buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;

	abrirTrinco(&trincoColocar);

	fecharTrinco(&trincoNrPedidos);
	nrPedidos++;
	abrirTrinco(&trincoNrPedidos);

	assinalarSemaforo(&semRetirar);
}

comando_t retirarPedido() {
	comando_t comando;

	esperarSemaforo(&semRetirar);
	fecharTrinco(&trincoRetirar);

	comando = cmd_buffer[buff_read_idx];
	buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;

	abrirTrinco(&trincoRetirar);
	assinalarSemaforo(&semColocar);

	return comando;
}



void* efetuarComando(void *idTarefa) {
	while(TRUE) {
		comando_t comando = retirarPedido();
		switch (comando.operacao){
			case DEBITAR:
				if (debitar(comando.idConta1, comando.valor) < 0)
					printf("%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
				else
					printf("%s(%d, %d): OK\n\n", COMANDO_DEBITAR, comando.idConta1, comando.valor);
				break;

			case CREDITAR:
				if (creditar(comando.idConta1, comando.valor) < 0)
					printf("%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
				else
					printf("%s(%d, %d): OK\n\n", COMANDO_CREDITAR, comando.idConta1, comando.valor);
				break;

			case TRANSFERIR:
				if (transferir(comando.idConta1, comando.idConta2, comando.valor) < 0)
					printf("Erro ao transferir %d da conta %d para a conta %d.\n\n", comando.valor, comando.idConta1, comando.idConta2);
				else
					printf("%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, comando.idConta1, comando.idConta2, comando.valor);
				break;

			case LER_SALDO:
				if ((comando.valor=lerSaldo(comando.idConta1)) < 0)
					printf("%s(%d): Erro.\n\n", COMANDO_LER_SALDO, comando.idConta1);
				else
					printf("%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, comando.idConta1, comando.valor);
				break;

			case SAIR:
				pthread_exit(NULL);
				break;

			default:
				perror("Erro ao efetuar comando.\n");
				exit(1);
		}

		fecharTrinco(&trincoNrPedidos);
		if (--nrPedidos == 0)
			pthread_cond_signal(&todosPedidosEfetuados);
		abrirTrinco(&trincoNrPedidos);
	}
	return NULL;
}



void prepararSimulacao() {
	fecharTrinco(&trincoNrPedidos);
	while (nrPedidos != 0)
		pthread_cond_wait(&todosPedidosEfetuados, &trincoNrPedidos);
	abrirTrinco(&trincoNrPedidos);
}



void esperarTarefas() {
	int i;
	comando_t comando;
	comando.operacao = SAIR;

	for (i = 0; i < NUM_TRABALHADORAS; i++)
		fazerPedido(comando);
	for (i = 0; i < NUM_TRABALHADORAS; i++) {
		pthread_join(tarefas[i], NULL);

		switch (errno) {
			case EDEADLK:
				perror("pthread_join: Ocorreu um deadlock.\n");
				break;
			case EINVAL:
				perror("pthread_join: Nao e joinable / Outra tarefa ja esta a espera para juntar com esta.\n");
				break;
			case ESRCH:
				perror("pthread_join: Nao foi encontrada nenhuma tarefa com o ID dado.\n");
				break;
		}
	}
}

void esperarProcessos(int* numchild) {
	int estado, nrProcessosTerminados = 0, i;
	pid_t childpid;
	struct processoT {
		pid_t pid;
		int estado;
	};
	struct processoT processosT[NUM_MAX_PROCESSOS];

	/* esperar que os processos terminem */
	while (*numchild > 0){
		childpid = wait(&estado);
		processosT[nrProcessosTerminados].pid = childpid;

		if (WIFSIGNALED(estado))
			processosT[nrProcessosTerminados].estado = P_TERMINADO_ABRUPTAMENTE;
		else
			processosT[nrProcessosTerminados].estado = P_TERMINADO_NORMALMENTE;

		nrProcessosTerminados++;
		(*numchild)--;
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
