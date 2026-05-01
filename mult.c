#include <stdio.h>
#include <stdlib.h>
#include "simulador.h"

int RI;
int A, B;
int ULAout;
int RDM;
int estado = 0; // FSM
int PC = 0;

decode campos(int instrucao){
    decode c;
    
    c.opcode = (instrucao >> 12) & 0xF;
    c.rs = (instrucao >> 9) & 0x7;
    c.rt = (instrucao >> 6) & 0x7;
    c.rd = (instrucao >> 3) & 0x7;
    c.funct = instrucao & 0x7;
    c.imm = instrucao & 0x3F;
    c.addr = instrucao & 0xFF;

    if (c.imm >= 32) { // extensao de sinal
        c.imm -= 64; 
    }
    
    return c;
}
void step(int memoria_instrucao[], int registradores[], int *PC){
    ciclo(memoria_instrucao, registradores, PC);
}
void ciclo(int memoria_instrucao[], int registradores[], int *PC){
    struct decode c;
    switch (estado){
        case 0: // BUSCA
            RI = memoria_instrucao[*PC];
            printf("BUSCA: PC = %d, RI = %d\n", *PC, RI);
            (*PC)++;
            estado = 1;
            break;
        case 1: // DECODE
            c = campos(RI);
            A = registradores[c.rs];
            B = registradores[c.rt];
            printf("DECODE:\n");
            printf("opcode: %d\n", c.opcode);
            printf("rs: %d\n", c.rs);
            printf("rt: %d\n", c.rt);
            
            estado = 0;
            break;
        default:
            printf("Estado desconhecido.\n");
            break;
}
}
void run(int memoria_instrucao[], int registradores[], int *PC){
    int ciclos = 0;
    while (*PC <10){
        ciclo(memoria_instrucao, registradores, PC);
        ciclos++;
        printf("Ciclo: %d\n", ciclos);
    }
}
int ULA(int A, int B, int controle, int *flag) {
    int resultado = 0;
    
    switch(controle) {
        case 0:
            resultado = A + B; 
            break;
        case 2: 
            resultado = A - B;
            break;
        case 4: 
            resultado = A & B; 
            break;
        case 5: 
            resultado = A | B; 
            break;
        default: 
            resultado = 0;
    }
    
    *flag = (resultado == 0);
    
    if (resultado > 127 || resultado < -128) {
        printf("Overflow.\n");
    }
    
    return resultado;
}
