/**********************************************************************************************************************
 *                           Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                           *
 *                            Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                            *
 *                                     Duarte Galvão, 83449 / Pedro Lopes, 83540                                      *
 **********************************************************************************************************************
 ****************************************************** contas.c ******************************************************
 **********************************************************************************************************************/

#include "contas.h"
#include "taskprocesses.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define atrasar() sleep(ATRASO)

int contasSaldos[NUM_CONTAS];
int signalReceived = FALSE;


int contaExiste(int idConta) {
  return (idConta > 0 && idConta <= NUM_CONTAS);
}

void inicializarContas() {
  int i;
  for (i=0; i<NUM_CONTAS; i++)
    contasSaldos[i] = 0;
}

int debitar(int idConta, int valor) {
	atrasar();
	if (!contaExiste(idConta))
		return -1;
	fecharTrinco(&trincosContas[idConta-1]);
	if (contasSaldos[idConta - 1] < valor){
		abrirTrinco(&trincosContas[idConta-1]);
		return -1;
	}
	contasSaldos[idConta - 1] -= valor;
	abrirTrinco(&trincosContas[idConta-1]);
	return 0;
}

int creditar(int idConta, int valor) {
	atrasar();
	if (!contaExiste(idConta))
		return -1;
	fecharTrinco(&trincosContas[idConta-1]);
	contasSaldos[idConta - 1] += valor;
	abrirTrinco(&trincosContas[idConta-1]);
	return 0;
}

int lerSaldo(int idConta) {
	int saldo;
	atrasar();
	if (!contaExiste(idConta))
		return -1;
	fecharTrinco(&trincosContas[idConta-1]);
	saldo = contasSaldos[idConta - 1];
	abrirTrinco(&trincosContas[idConta-1]);
	return saldo;
}

int definirSaldo(int idConta, int valor){
	if (!contaExiste(idConta) || valor < 0)
		return -1;
	else
		contasSaldos[idConta - 1] = valor;
	return 0;
}

void simular(int numAnos) {
	int ano, id, saldo;

	for (ano = 0; ano <= numAnos; ano++) {
		printf("SIMULACAO: Ano %d\n",ano);
		printf("=================\n");
		for (id = 1; id <= NUM_CONTAS; id++){
			saldo = lerSaldo(id);
			printf("Conta %d, Saldo %d\n", id, saldo);

			/* calcular valor para o ano seguinte */
			saldo = (int) (saldo * (1 + TAXAJURO) - CUSTOMANUTENCAO);
			if (saldo < 0)
				saldo = 0;
			definirSaldo(id,saldo);
		}
		printf("\n");

		if (signalReceived) {
			printf("Simulacao terminada por signal.\n");
			break;
		}
	}
}

void signalTerminarProcesso(int s){
	signalReceived = TRUE;
	signal(SIGUSR1, signalTerminarProcesso);
}
