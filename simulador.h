#ifndef SIMULADOR_H
#define SIMULADOR_H

//estados fsm
#define BUSCA 0
#define DECODE 1
#define EXEC 2
#define WRITE 3
#define MEM_ADDR 4
#define MEM_READ 5
#define MEM_WRITEBACK 6
#define MEM_WRITE 7
#define BRANCH 8
#define JUMP 9
typedef struct decode{
    int opcode;
    int rs;
    int rt;
    int rd;
    int funct;
    int imm;
    int addr;
} decode;
typedef struct sinaisControle {
    int PCEsc, IouD, EscMem, IREsc;
    int LerRegs, LerMem, MemParaReg, EscReg, RegDst;
    int ULAFonteA, ULAFonteB, ULAControle, PCFonte, Branch;
} sinaisControle;

decode campos(int instrucao);
sinaisControle gerarSinais(int estado, int opcode, int funct);
int proximo_estado(int estado, int opcode);


#endif
