#include <stdio.h>
#include <stdlib.h>
#include "simulador.h"

int RI;
int A,B;
int ULAout;
int RDM;
int estado = BUSCA;

void escolher_arquivo_mem(char nome_arquivo[]){
    FILE *arquivo;

    printf("\nDigite o nome do arquivo .mem: ");
    scanf("%s", nome_arquivo);
    arquivo = fopen(nome_arquivo, "r");
    printf("Arquivo carregado.\n");
    if (arquivo == NULL) {
        printf("Erro: o arquivo %s nao foi encontrado.\n", nome_arquivo);
    }

    fclose(arquivo);
}

void leitura_arquivo_mem(int memoria[], char nome_arquivo[]) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    
    if(arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }
    
    int i = 0;
    char linha[17];
    
    while (fscanf(arquivo, "%s", linha) != EOF && i < 256) {
        memoria[i] = (int)strtol(linha, NULL, 2);
        i++;
    }
    
    fclose(arquivo);
}

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

void inicializar_registradores(int registradores[]) {
    for (int i = 0; i < 8; i++) {
        registradores[i] = 0;
    }
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

sinaisControle gerarSinais(int opcode, int funct){
    sinaisControle s = {0, 0, 0, 0, 0, 0, 0};
    switch(opcode) {
        case 0:
            s.RegDst = 1;
            s.ULAOp = controle_ULA(opcode, funct);
            break;
        case 4:
            s.RegDst = 0;
            s.ULAOp = 0;
            break;
        case 11:
            s.RegDst = 0;
            s.MemPReg = 1;
            s.ULAOp = 0;
            s.MemRead = 1;
            break;
        case 15:
            s.ULAOp = 0;
            s.MemWrite = 1;
            break;
        case 8:
            s.ULAOp = 2;
            s.Branch = 1;
            break;
        case 2:
            s.Jump = 1;
            break;
        default:
            printf("Instrucao invalida!\n");
    }
    return s;
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

void ciclo(int memoria_instrucao[], int registradores[], int *PC) {
    decode c = campos(RI);

    switch (estado) {
        case BUSCA: 
            RI = memoria_instrucao[*PC]; 
            printf("BUSCA: PC = %d, RI = %d\n", *PC, RI);
            (*PC)++;
            estado = DECODE;
            break;
        case DECODE: 
            printf("DECODE: opcode: %d\n", c.opcode);

            A = registradores[c.rs];
            B = registradores[c.rt];

            if (c.opcode == 0 || c.opcode == 4) { // tipo R e ADDI (vai pra ULA) 
                estado = EXEC;
            } else if (c.opcode == 11 || c.opcode == 15) { // LW e SW (vai pra ULA calcular os endereços pra memoria)
                estado = MEM_ADDR;
            } else if (c.opcode == 8) { // BEQ (vai usar a ULA para comparar)
                estado = BRANCH;
            } else if (c.opcode == 2) { // JUMP
                estado = JUMP;
            }
            break;

        case EXEC: { 
            int flag;
            int op2 = MUX2(B, c.imm, sinais.ULAFonteB);
            int controle = controle_ULA(c.opcode, c.funct); 
            
            // O resultado matemático é gravado direto no REGISTRADOR ULAout
            ULAout = ULA(A, op2, controle, &flag);
            printf("[C3] Executa ULA: ULAout=%d\n", ULAout);
            
            estado = WRITE;
            break;
        }

        case MEM_ADDR: { // LW e SW
            int flag;
            
            ULAout = ULA(A, c.imm, 0, &flag); // o 0 é p forçar a ula a somar
            printf("[C3] Calc Endereco: ULAout=%d\n", ULAout);
            
            estado = MUX2(MEM_WRITE, MEM_READ, (c.opcode == 11));
            break;
        }

        case BRANCH: { // beq
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
            int sinal_RegDst = (c.opcode == 0); // se for 0 a instrucao eh do tipo R.
            int reg_destino = MUX2(c.rt, c.rd, sinal_RegDst); // isso serve pra decidir o registrador que vamos guardar o valor

            int sinal_MemParaReg = 0;
            int dado_escrita = MUX2(ULAout, RDM, sinal_MemParaReg); // isso serve pra decidiro valor que será guardado

            registradores[reg_destino] = dado_escrita; // unindo o o registrador destino e o valor que será guardado
            printf("[C4] Escrita Reg: $%d = %d\n", reg_destino, dado_escrita);

            estado = BUSCA;
            break;
        }

        case MEM_WRITE: { // CICLO 4 - EXCLUSIVO PARA SW
            memoria_instrucao[ULAout] = B; 
            printf("[C4] Escrita Mem: Mem[%d] = %d\n", ULAout, B); // vai escrever na memoria o que tiver no registrador B.

            estado = BUSCA;
            break;
        }

        case MEM_READ: { // CICLO 4 - EXCLUSIVO PARA LW (guarda no RDM)
            RDM = memoria_instrucao[ULAout]; 
            printf("[C4] Leitura Mem: RDM = %d (lido do endereco %d)\n", RDM, ULAout);

            estado = MEM_WRITEBACK;
            break;
        }

        case MEM_WRITEBACK: { // CICLO 5 - ESCRITA DO LW NA MEMORIA
            int sinal_RegDst = 0; 
            int reg_destino = MUX2(c.rt, c.rd, sinal_RegDst); // serve para o mux direcionar para o rt

            int sinal_MemParaReg = 1;
            int dado_escrita = MUX2(ULAout, RDM, sinal_MemParaReg); // serve para o mux direcionar para a saida do RDM

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
