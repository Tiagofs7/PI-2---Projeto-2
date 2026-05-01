#include <stdio.h>
#include <stdlib.h>
#include "simulador.h"

int RI;
int A, B;
int ULAout;
int RDM;
int estado = BUSCA; // FSM

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
void ciclo(int memoria_instrucao[], int registradores[], int *PC){
    decode c;
    switch (estado){
        case BUSCA: // BUSCA
            RI = memoria_instrucao[*PC];
            printf("BUSCA: PC = %d, RI = %d\n", *PC, RI);
            (*PC)++;
            estado = DECODE;
            break;
        case DECODE: // DECODE
            c = campos(RI);
            int tipo = tipo_instrucao(c.opcode);

            printf("DECODE:\n");
            printf("opcode: %d\n", c.opcode);

        if (tipo == 0){ // R
            A = registradores[c.rs];
            B = registradores[c.rt];

            printf("Tipo R\n");
            printf("rs: %d rt: %d rd: %d funct: %d\n",
            c.rs, c.rt, c.rd, c.funct);
    }
        else if (tipo == 1){ // I
            A = registradores[c.rs];
            B = registradores[c.rt];

            printf("Tipo I\n");
            printf("rs: %d rt: %d imm: %d\n",
            c.rs, c.rt, c.imm);
    }

        else if (tipo == 2){ // J
            printf("Tipo J\n");
            printf("addr: %d\n", c.addr);
    }

            estado = EXEC;
        break;
        default:
            printf("erro.\n");
            break;
}
}
void step(int memoria_instrucao[], int registradores[], int *PC){
    ciclo(memoria_instrucao, registradores, PC);
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
