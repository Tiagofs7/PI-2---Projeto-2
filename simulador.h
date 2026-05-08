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
typedef struct{
    int RegDst; // decide se o destino é rd (1) ou rt (0)
    int ULAOp; 
    int MemRead;  
    int MemWrite; 
    int MemPReg; //verifica se dado escrito vem da memoria (1) ou da ULA (0)
    int RegWrite;
    int Branch;
    int Jump;
} sinaisControle;

decode campos(int instrucao);


#endif
