/**********************************************************************************************************************
 *                           Instituto Superior Técnico - LEIC-A/2006 - Sistemas Operativos                           *
 *                            Ano Letivo 2016/2017 - 1o. Semestre - Projeto - Exercício 2.                            *
 *                                     Duarte Galvão, 83449 / Pedro Lopes, 83540                                      *
 **********************************************************************************************************************
 ****************************************************** contas.h ******************************************************
 **********************************************************************************************************************/

#ifndef CONTAS_H
#define CONTAS_H

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1

#define NORMALMENTE 0
#define ABRUPTAMENTE 1

#define ATRASO 1

#define FALSE 0
#define TRUE 1

void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int transferir(int idConta1, int idConta2, int valor);
int lerSaldo(int idConta);
int definirSaldo(int idConta, int valor);
void simular(int numAnos);
void signalTerminarProcesso(int s);


#endif
