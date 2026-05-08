#include <stdio.h>
#include <stdlib.h>
#include "simulador.h"

int RI;
int A,B;
int ULAout;
int RDM;
int estado = BUSCA;

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

void guardarIR(int instrucao){
    RI = instrucao;
    
}
int retornarIR(){
    return RI;
}

// MUX de 2 entradas (para sinais de controle de 1 bit)
int MUX2(int entrada0, int entrada1, int controle) {
    return (controle == 0) ? entrada0 : entrada1;
}

// MUX de 3/4 entradas (para sinais de controle de 2 bits)
int MUX4(int entrada0, int entrada1, int entrada2, int entrada3, int controle) {
    switch(controle) {
        case 0: return entrada0;
        case 1: return entrada1;
        case 2: return entrada2;
        case 3: return entrada3;
        default: return entrada0;
    }
}
void ciclo(int memoria_instrucao[], int registradores[], int *PC) {
    static decode c;

    switch (estado) {
        case BUSCA: 
            guardarIR(memoria_instrucao[*PC]);
            printf("BUSCA: PC = %d, RI = %d\n", *PC, RI);
            (*PC)++;
            estado = DECODE;
            break;
        case DECODE: 
            c = campos(retornarIR());
            printf("DECODE: opcode: %d\n", c.opcode);

            A = registradores[c.rs];
            B = registradores[c.rt];

            if (c.opcode == 0 || c.opcode == 4) {
                estado = EXEC;
            } else if (c.opcode == 11 || c.opcode == 15) {
                estado = MEM_ADDR;
            } else if (c.opcode == 8) {
                estado = BRANCH;
            } else if (c.opcode == 2) {
                estado = JUMP;
            }
            break;
        case EXEC: { 
            int flag;
            int op2 = (c.opcode == 4) ? c.imm : B; 
            
            int controle = controle_ULA(c.opcode, c.funct); 
            
            carregarULAout(ULA(A, op2, controle, &flag));
            printf("[C3] Executa ULA: ULAout=%d\n", lerULAout());
            
            estado = WRITE;
            break;
        }
        case MEM_ADDR: {
            int flag;
            carregarULAout(ULA(A, c.imm, 0, &flag)); 
            printf("[C3] Calc Endereco: ULAout=%d\n", lerULAout());
            
            estado = (c.opcode == 11) ? MEM_READ : MEM_WRITE; 
            break;
        }
        case BRANCH: {
            int flag;
            ULA(A, B, 2, &flag); 
            
            if (flag) *PC += c.imm;
            printf("[C3] BEQ: PC=%d\n", *PC);
            
            estado = BUSCA;
            break;
        }
        case JUMP:
            *PC = c.addr;
            printf("[C3] JUMP: PC=%d\n", *PC);
            
            estado = BUSCA;
            break;
        case WRITE: { 
            int sinal_RegDst = (c.opcode == 0) ? 1 : 0;
            int reg_destino = MUX2(c.rt, c.rd, sinal_RegDst);

            int sinal_MemParaReg = 0;
            int dado_escrita = MUX2(lerULAout(), RDM, sinal_MemParaReg);

            registradores[reg_destino] = dado_escrita;
            printf("[C4] Escrita Reg: $%d = %d\n", reg_destino, dado_escrita);

            estado = BUSCA;
            break;
        }
        case MEM_WRITE: {
            int endereco = lerULAout();
            memoria_instrucao[endereco] = B; 
            printf("[C4] Escrita Mem: Mem[%d] = %d\n", endereco, B);

            estado = BUSCA;
            break;
        }
        case MEM_READ: {
            int endereco = lerULAout();
            RDM = memoria_instrucao[endereco]; 
            printf("[C4] Leitura Mem: RDM = %d (lido do endereco %d)\n", RDM, endereco);

            estado = MEM_WRITEBACK;
            break;
        }
        case MEM_WRITEBACK: {
            int sinal_RegDst = 0;
            int reg_destino = MUX2(c.rt, c.rd, sinal_RegDst);

            int sinal_MemParaReg = 1;
            int dado_escrita = MUX2(lerULAout(), RDM, sinal_MemParaReg);

            registradores[reg_destino] = dado_escrita;
            printf("[C5] Writeback (LW): $%d = %d\n", reg_destino, dado_escrita);

            estado = BUSCA;
            break;
        }
        default:
            printf("erro.\n");
            estado = BUSCA;
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
int controle_ULA(int opcode, int funct) {
    switch(opcode) {
    case 0: // tipo R
        switch(funct) {
            case 0: return 0; // ADD
            case 2: return 2; // SUB
            case 4: return 4; // AND
            case 5: return 5; // OR
            default: return -1;
            }
    case 11: // LW
    case 15: // SW
    case 4:  // ADDI
        return 0;
    case 8: // BEQ
        return 2; // sub para fazer a comparação
    default:
        return -1;
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
void carregarULAout(int resultado){
    ULAout = resultado;
}
int lerULAout(){
    return ULAout;
}
