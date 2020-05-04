/*
    Remove all unnecessary lines (including this one) in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name: [Full name]
    ID  : [Your Student ID]
*/

#include "../lc3bsim3.h"

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) + (x[J3] << 3) + (x[J2] << 2) + (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

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
    printf("run n            -  execute program for n cycles      \n");
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

    eval_micro_sequencer();   
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();

    CURRENT_LATCHES = NEXT_LATCHES;

    CYCLE_COUNT++;
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
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%04x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%04x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%04x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%04x\n", BUS);
    printf("MDR          : 0x%04x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%04x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%04x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%04x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%04x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%04x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%04x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%04x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%04x\n", CURRENT_LATCHES.MAR);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n",
                    i, ucode_filename);
            exit(-1);
        }

        /* Put in bits one at a time. */
        index = 0;

        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
                        ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
                        ucode_filename, i, j);
                exit(-1);
            }

            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
            index++;
        }

        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
                    ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
                argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

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

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/



/* 
 * This function emulates memory and the WE logic. 
 * Keep track of which cycle of MEMEN we are dealing with.  
 * If fourth, we need to latch Ready bit at the end of 
 * cycle to prepare microsequencer for the fifth cycle.  
 */
void cycle_memory() 
/*{{{*/
{ 
    static int MemCycleCnt = 0;

    int lR_W       = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
    int lDATA_SIZE = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    int lMAR       = CURRENT_LATCHES.MAR;
    int lMIO_EN    = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);

    int lMAR_0     = partVal(lMAR, 0, 0);
    int WE0        = lR_W & (((!lMAR_0) & (!lDATA_SIZE)) | lDATA_SIZE);
    int WE1        = lR_W & ((lMAR_0 & (!lDATA_SIZE)) | lDATA_SIZE);
    int MEM_EN     = lMIO_EN;

    MemOut = 0;
    NEXT_LATCHES.READY = 0;
    printf("MemCycleCnt = %d\n", MemCycleCnt);
    printf("MEM_EN = %d, R_W = %d, WE0 = %d, WE1 = %d\n", MEM_EN, lR_W, WE0, WE1);

    if (MEM_EN)
    {
        if (MemCycleCnt >= 0)
        {
            if (MemCycleCnt < 3)
            {
                MemCycleCnt++;
                return;
            }
            else if (MemCycleCnt == 3)
            {
                MemCycleCnt++;
                NEXT_LATCHES.READY = 1;
                return;
            }
            else
            {
                MemCycleCnt = 0;
            }
        }
        if (lR_W)	/* write */
        {
            if (WE0)
            {
                MEMORY[(CURRENT_LATCHES.MAR>>1)][0] = partVal(CURRENT_LATCHES.MDR, 7, 0);
            }
            if (WE1)
            {
                MEMORY[(CURRENT_LATCHES.MAR>>1)][1] = partVal(CURRENT_LATCHES.MDR, 15, 8);
            }
        }
        else	/* read */
        {
            MemOut/*NEXT_LATCHES.MDR*/ = Low16bits((MEMORY[(CURRENT_LATCHES.MAR>>1)][1] << 8) + (MEMORY[(CURRENT_LATCHES.MAR>>1)][0]));
        }
        MemCycleCnt++;
    }
    else
    {
        MemCycleCnt = 0;
    }
}
/*}}}*/


int blockADDR2MUX(int lADDR2MUX, int lZero, int lOffset6, int lPCoffset9, int lPCoffset11)
{
    switch (lADDR2MUX & 0x3)
    {
        case 0:
            return lZero;
        case 1:
            return blockSEXT(lOffset6, 6);
        case 2:
            return blockSEXT(lPCoffset9, 9);
        case 3:
            return blockSEXT(lPCoffset11, 11);
        default:
            printf("Warning: unknown ADDR2MUX = %d\n", lADDR2MUX);
    }
    return lZero;
}

int blockLSHF1(int lLSHF1, int value)
{
    if (lLSHF1)
    {
        return Low16bits(value << 1);
    }
    else
    {
        return Low16bits(value);
    }
}

int blockADDR1MUX(int lADDR1MUX, int lPC, int lBaseR)
{
    switch (lADDR1MUX)
    {
        case 0:
            return lPC;
        case 1:
            return lBaseR;
        default:
            printf("Warning: unknown ADDR1MUX = %d\n", lADDR1MUX);
    }
    return lPC;
}

int blockSR1MUX(int lSR1MUX, int lIR)
{
    switch (lSR1MUX)
    {
        case 0:
            return partVal(lIR, 11, 9);
        case 1:
            return partVal(lIR, 8, 6);
        default:
            printf("Warning: unknown SR1MUX = %d\n", lSR1MUX);
    }
    return partVal(lIR, 11, 9);
}

int blockSHF(int lIR_5_0, int lSR1)
{
    int lIR_5_4 = partVal(lIR_5_0, 5, 4);
    int lAmount4 = partVal(lIR_5_0, 3, 0);
    /*printf("lIR_5_4 = %d, lSR1 = %04X, lAmount4 = %d\n", lIR_5_4, lSR1, lAmount4);*/
    switch (lIR_5_4)
    {
        case 0:
            return Low16bits(lSR1 << lAmount4);
        case 1:
            return Low16bits(lSR1 >> lAmount4);
        case 3:
            return Low16bits(blockSEXT(lSR1 >> lAmount4, 16 -lAmount4));
        case 4:
            break;;
        default:
            /* printf("Warning: unknown IR[5:4] = %d\n", lIR_5_4); */
            ;
    }
    return Low16bits(lSR1 << lAmount4);
}


/* 
 * Datapath routine emulating operations before driving the bus.
 * Evaluate the input of tristate drivers 
 *       Gate_MARMUX,
 *		 Gate_PC,
 *		 Gate_ALU,
 *		 Gate_SHF,
 *		 Gate_MDR.
 */    
void eval_bus_drivers()
{
    inputOfGateMARMUX      = 0;
    inputOfGatePC          = 0;
    inputOfGateALU         = 0;
    inputOfGateSHF         = 0;
    inputOfGateMDR         = 0;
    inputOfMARMUXFromAdder = 0;  /*input of both MARMUX & PCMUX*/

    inputOfMARMUXFromAdder = Low16bits
        (
         blockLSHF1
         (
          GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION), 
          blockADDR2MUX
          (
           GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION), 
           0, 
           partVal(CURRENT_LATCHES.IR, 5, 0), 
           partVal(CURRENT_LATCHES.IR, 8, 0), 
           partVal(CURRENT_LATCHES.IR, 10, 0)
          )
         ) + 
         blockADDR1MUX
         (
          GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION), 
          CURRENT_LATCHES.PC, 
          CURRENT_LATCHES.REGS[blockSR1MUX(GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION), CURRENT_LATCHES.IR)]
         )
        );

    /* input of GateSHF */
    inputOfGateSHF = blockSHF
        (
         partVal(CURRENT_LATCHES.IR, 5, 0),
         CURRENT_LATCHES.REGS[blockSR1MUX(GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION), CURRENT_LATCHES.IR)]
        );

    /*
     *  Lab3-2 assignment as follows
     *
     */
    /* input of GateMARMUX */

    /* input of GatePC */

    /* input of GateALU */

    /* input of GateMDR */
}


/* 
 * Datapath routine for driving the bus from one of the 5 possible 
 * tristate drivers. 
 */       
void drive_bus()
{
    /*
     *  Lab3-2 assignment
     *  
     *  replace the following line:
     */
    BUS = 0;
}

