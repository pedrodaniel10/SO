/**********************************************************************************************************************
 *                           Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                           *
 *                            Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                            *
 *                                     Duarte Galvão, 83449 / Pedro Lopes, 83540                                      *
 **********************************************************************************************************************
 ************************************************** taskprocesses.h ***************************************************
 **********************************************************************************************************************/
#ifndef TASKPROCESSES_H
#define TASKPROCESSES_H

#include "contas.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

#define NUM_MAX_PROCESSOS 20
#define NUM_TRABALHADORAS 3
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"

typedef struct {
	int operacao;
	int idConta;
	int valor;
} comando_t;

enum {DEBITAR, CREDITAR, LER_SALDO, SAIR};

extern pthread_mutex_t trincosContas[NUM_CONTAS];

/* Lista dos processos terminados na ordem em que acabaram */
#define P_TERMINADO_NORMALMENTE 0
#define P_TERMINADO_ABRUPTAMENTE 1



void inicializarTarefas();
void inicializarTrincos();
void inicializarSemaforos();

void criarSemaforo(sem_t* sem, unsigned int value);

void abrirTrinco(pthread_mutex_t* mutex);
void fecharTrinco(pthread_mutex_t* mutex);

void esperarSemaforo(sem_t* sem);
void assinalarSemaforo(sem_t* sem);

void fazerPedido(comando_t);
comando_t retirarPedido();

void* efetuarComando();

void esperarTarefas();
void esperarProcessos(int* numchild);

#endif
