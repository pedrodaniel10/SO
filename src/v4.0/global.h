/**************************************************************************************************
 *                 Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                 *
 *                  Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                  *
 *                           Duarte Galvão, 83449 / Pedro Lopes, 83540                            *
 **************************************************************************************************
 ******************************************** global.h ********************************************
 **************************************************************************************************/


#ifndef GLOBAL_H
#define GLOBAL_H

#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_TRANSFERIR "transferir"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "sair agora"

/* operacao */
enum {DEBITAR, CREDITAR, TRANSFERIR, LER_SALDO, SIMULAR, SAIR, SAIR_AGORA, PEDIR_LIGACAO};
/* result */
enum {CMD_ERROR, CMD_OK, CMD_NOT_OK};

typedef struct {
	int pid;
	int ficheiro_i_banco;
	int operacao;
	int idConta1;
	int idConta2;
	int valor;
	int result;
} comando_t;
#endif
