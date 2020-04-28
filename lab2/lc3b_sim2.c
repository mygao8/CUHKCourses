/*
    Remove all unnecessary lines (including this one) in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name: GAO, Mingyuan
    ID  : 1155107738
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   CEG3420 Lab2                                              */
/*   The Chinese University of Hong Kong                       */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
int RUN_BIT;	/* run bit */

#define LC_3b_REGS 8

/* Data Structure for Latch */
typedef struct System_Latches_Struct{
    int PC,		        /* program counter */
        N,		        /* n condition bit */
        Z,		        /* z condition bit */
        P;		        /* p condition bit */
    int REGS[LC_3b_REGS]; /* register file.  */
} System_Latches;

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3b ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
    process_instruction();
    // printf("end process_instr\n");
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
    // printf("end cycle\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000)
        {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
        // printf("i=%d, CUR.PC=0x%x\n\n", i, CURRENT_LATCHES.PC);
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%04x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%04x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%04x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%04x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;

        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;

        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);

        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;

        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                    program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
                argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

/* retrun the word content start from startAddr */
int memWord (int startAddr)
{
    /*
     * Lab2-1 assignment.
     * */
    startAddr >>= 1;
    // printf("memWord startAddr: 0x%x\n", startAddr);
    int low = MEMORY[startAddr][0];
    // printf("memWord MEMORY[startAddr][0]: 0x%x\n", MEMORY[startAddr][0]);
    int high = MEMORY[startAddr][1];
    // printf("memWord MEMORY[startAddr][1]: 0x%x\n", MEMORY[startAddr][1]);
    int content = (high << 8) + low;
    // printf("memWord content: 0x%x\n", content);
    return content;
    return 0;
}

/* return the corresponding value of intst[hBit:lBit] */
int partVal (int instr, int hBit, int lBit)
{
    /*
     * Lab2-1 assignment.
     */
    int tmp = Low16bits(instr << (15 - hBit));
    return (tmp >> (15 - hBit + lBit));
    return 0;
}

/* sign extend the imm to 16 bits
 * 'width' is the bit width before imm is extended*/
int SEXT (int imm, int width)
{
    /*
     * Lab2-2 assignment.
     */
    imm = partVal(imm, width - 1, 0);
    int leftBit = imm >> (width - 1);
    if (leftBit == 1){
        int tmp = 1 << width;
        return Low16bits(tmp + imm);
    }
    return Low16bits(imm);
    return 0;
}

/* set condition code */
void setCC(int num)
{
    /*
     * Lab2-2 assignment.
     */
    if (num == 0){
        NEXT_LATCHES.Z = 1; 
        NEXT_LATCHES.N = 0; 
        NEXT_LATCHES.P = 0;
    }
    else if ((num>>15) == 0){
        NEXT_LATCHES.Z = 0; 
        NEXT_LATCHES.N = 0; 
        NEXT_LATCHES.P = 1;
    }
    else if ((num>>15) == 1){
        NEXT_LATCHES.Z = 0; 
        NEXT_LATCHES.N = 1; 
        NEXT_LATCHES.P = 0;  
    }
}

/*  
 *  Process one instruction at a time  
 *  -Fetch one instruction
 *  -Decode 
 *  -Execute
 *  -Update NEXT_LATCHES
 */     
void process_instruction()
{
    int curInstr = memWord(CURRENT_LATCHES.PC);
    int opCode = (curInstr >> 12);
    printf("process_instruction()| curInstr = 0x%04x\n", curInstr);

    /*
     * Lab2-1 assignment: update NEXT_LATCHES
     * (Think about how to update new PC!)
     * */
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
    // printf("NEXT.PC=0x%x\n", NEXT_LATCHES.PC);
    /*
     * Define more variables if needed
     * */
    int DR, SR1, SR2, imm5, trapvect8, PCoffset9, offset6, BaseR, boffset6, SR;
    int value, targetMem, tmp;


    switch(opCode) /* opCode */
    {
        case 1: /* add, and */
        case 5:
            DR = partVal(curInstr, 11, 9);
            SR1 = partVal(curInstr, 8, 6);
            if (partVal(curInstr, 5, 5)) /* imm5 */
            {
                imm5 = partVal(curInstr, 4, 0);
                value = SEXT(imm5, 5);
            }
            else
            {
                SR2 = partVal(curInstr, 2, 0);
                value = CURRENT_LATCHES.REGS[SR2];
            }
            if (opCode == 1)
            {
                NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] + value);
            }
            else if (opCode == 5)
            {
                NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.REGS[SR1] & value);
            }
            setCC(NEXT_LATCHES.REGS[DR]);
            break;

        case 0: /* br */
            /*
            * Lab2-2 assignment: 
            */
            PCoffset9 = partVal(curInstr, 8, 0);
            if (CURRENT_LATCHES.Z){
                NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2 + Low16bits(SEXT(PCoffset9, 9) << 1));
            }
            break;

        case 2: /* ldb */
            /*
            * Lab2-2 assignment: 
            * (Note the difference of the instruction based on your SID!)
            * */
            DR = partVal(curInstr, 8, 6);
            boffset6 = partVal(curInstr, 5, 0);
            BaseR = partVal(curInstr, 11, 9);
            targetMem = CURRENT_LATCHES.REGS[BaseR] + Low16bits(SEXT(boffset6, 6));
            if (targetMem%2==0){
                tmp = memWord(targetMem) & 0x00ff;
            }
            else{
                tmp = memWord(targetMem) & 0xff00;
            }
            // printf("tmp:0x%x\n", tmp);
            // printf("CurInstr:0x%x, DR:0x%x, boffset6:0x%x, BaseR:0x%x, target:0x%x\n", curInstr, DR, boffset6, BaseR, targetMem);
            NEXT_LATCHES.REGS[DR] = Low16bits(SEXT(tmp, 8));
            // printf("Byte:0x%x\n\n", partVal(NEXT_LATCHES.REGS[DR], 7, 0));
            setCC(NEXT_LATCHES.REGS[DR]);
            break;

        case 3: /* STB */
            /*
            * Lab2-2 assignment: 
            * (Note the difference of the instruction based on your SID!)
            * */
            SR = partVal(curInstr, 11, 9);
            boffset6 = partVal(curInstr, 5, 0);
            BaseR = partVal(curInstr, 8, 6);
            targetMem = CURRENT_LATCHES.REGS[BaseR] + Low16bits(SEXT(boffset6, 6));
            // printf("CurInstr:0x%x, SR:0x%x, boffset6:0x%x, BaseR:0x%x, target:0x%x\n", curInstr, SR, boffset6, BaseR, targetMem);
            MEMORY[targetMem / 2][targetMem % 2] = partVal(NEXT_LATCHES.REGS[SR], 7, 0);
            // printf("Byte:0x%x\n\n", partVal(NEXT_LATCHES.REGS[SR], 7, 0));
            break;

        case 4: /* jsr, jsrr */
            break;

        case 6: /* ldw */
            /* Lab2-2 assignment: */
            DR = partVal(curInstr, 11, 9);
            BaseR = partVal(curInstr, 8, 6);
            offset6 = partVal(curInstr, 5, 0);
            NEXT_LATCHES.REGS[DR] = memWord(Low16bits(CURRENT_LATCHES.REGS[BaseR] + Low16bits(SEXT(offset6, 6) << 1)));
            setCC(NEXT_LATCHES.REGS[DR]);
            break;

        case 7: /* STW */
            break;

        case 9: /* NOT XOR */
            break;

        case 12: /* JMP RET */
            break;

        case 13: /* LSHF RSHFL RSHFA */
            break;

        case 14: /* lea */
            DR = partVal(curInstr, 11, 9);
            PCoffset9 = partVal(curInstr, 8, 0);
            NEXT_LATCHES.REGS[DR] = Low16bits(CURRENT_LATCHES.PC + 2 + Low16bits(SEXT(PCoffset9, 9) << 1));
            setCC(NEXT_LATCHES.REGS[DR]);
            break;

        case 15: /* TRAP */
            trapvect8 = partVal(curInstr, 7, 0);
            NEXT_LATCHES.REGS[7] = Low16bits(CURRENT_LATCHES.PC + 2);
            NEXT_LATCHES.PC = memWord(trapvect8<<1);
            break;

        default:
            printf("Unknown opCode captured !\n");
            /*exit (1);*/
    }
}

