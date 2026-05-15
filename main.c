#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulador.h"

int main() {
    int memoria[256]     = {0};
    int registradores[8] = {0};
    int PC               = 0;
    int num_instrucoes   = 0;
    int opcao;

    printf("=======================================\n");
    printf("     SIMULADOR MIPS MULTICICLO\n");
    printf("=======================================\n");

    inicializar_registradores(registradores);

    do {
        printf("\n-------- Menu do Simulador Mini MIPS --------\n");
        printf("1 - Carregar memoria (.mem)\n");
        printf("2 - Imprimir memorias\n");
        printf("3 - Imprimir registradores\n");
        printf("4 - Executar programa (Run)\n");
        printf("5 - Executar um ciclo (Step)\n");
        printf("6 - Estatisticas\n");
        printf("---------------------------------------------\n");
        printf("0 - Sair\n");
        printf("Escolha: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1: {
                char nome[100];
                printf("Nome do arquivo .mem: ");
                scanf("%s", nome);
                memset(memoria, 0, sizeof(memoria));
                PC = 0;
                estado = BUSCA;
                resetar_estatisticas();
                inicializar_registradores(registradores);
                num_instrucoes = leitura_arquivo_mem(memoria, nome);
                break;
            }
            case 2:
                if (num_instrucoes == 0) { printf("Carregue um arquivo .mem primeiro.\n"); break; }
                imprimir_memorias(memoria, num_instrucoes);
                break;
            case 3:
                imprimir_registradores(registradores, PC);
                break;
            case 4: {
                if (num_instrucoes == 0) { printf("Carregue um arquivo .mem primeiro.\n"); break; }
                printf("\n--- Iniciando Execucao ---\n");
                while (estado != BUSCA || PC < num_instrucoes) {
                    if (estado == BUSCA) registrar_instrucao(memoria, PC);
                    ciclo(memoria, registradores, &PC);
                    incrementar_ciclos();
                }
                printf("--- Execucao concluida ---\n");
                break;
            }
            case 5: {
                if (num_instrucoes == 0) { printf("Carregue um arquivo .mem primeiro.\n"); break; }
                if (estado == BUSCA) registrar_instrucao(memoria, PC);
                ciclo(memoria, registradores, &PC);
                incrementar_ciclos();
                printf("Step executado. PC=%d | Estado=%d\n", PC, estado);
                break;
            }
            case 6:
                imprimir_estatisticas();
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida. Tente novamente.\n");
        }
    } while (opcao != 0);

    return 0;
}
