#ifndef LC3BSIM3_h
#define LC3BSIM3_h

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   CEG3420 Lab3                                              */
/*   The Chinese University of Hong Kong                       */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x);
int GetCOND(int *x);
int GetJ(int *x);
int GetLD_MAR(int *x);
int GetLD_MDR(int *x);
int GetLD_IR(int *x);
int GetLD_BEN(int *x);
int GetLD_REG(int *x);
int GetLD_CC(int *x);
int GetLD_PC(int *x);
int GetGATE_PC(int *x);
int GetGATE_MDR(int *x);
int GetGATE_ALU(int *x);
int GetGATE_MARMUX(int *x);
int GetGATE_SHF(int *x);
int GetPCMUX(int *x);
int GetDRMUX(int *x);
int GetSR1MUX(int *x);
int GetADDR1MUX(int *x);
int GetADDR2MUX(int *x);
int GetMARMUX(int *x);
int GetALUK(int *x);
int GetMIO_EN(int *x);
int GetR_W(int *x);
int GetDATA_SIZE(int *x);
int GetLSHF1(int *x);

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte.
   WE0 is used for the least significant byte of a word.
   WE1 is used for the most significant byte of a word.
   */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	    /* value of the bus */

typedef struct System_Latches_Struct{
    int PC,		          /* program counter */
        N,		          /* n condition bit */
        Z,		          /* z condition bit */
        P;		          /* p condition bit */
    int REGS[LC_3b_REGS]; /* register file.  */

    int MDR,	/* memory data register */
        MAR,	/* memory address register */
        IR,		/* instruction register */
        BEN;    /* ben register */

    int READY;	/* ready bit */
    /* The ready bit is also latched as you dont want the memory system to assert it 
       at a bad point in the cycle*/

    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

    int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */
System_Latches CURRENT_LATCHES, NEXT_LATCHES;

int CYCLE_COUNT;    /* A cycle counter */
int MemOut;         /* save the direct output of MEMORY each cycle */


/* five input tristate drivers */
int inputOfGateMARMUX;
int inputOfGatePC;
int inputOfGateALU;
int inputOfGateSHF;
int inputOfGateMDR;

/* input of both MARMUX and PCMUX  */
int inputOfMARMUXFromAdder;

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
 * you may or may not need these helper functions 
 */
int partVal (int instr, int hBit, int lBit);  /* return the corresponding value of intst[hBit:lBit] */
int blockZEXT_LSHF1(int value);
int blockLSHF1(int lLSHF1, int value);
int blockSEXT(int imm, int width);
int blockADDR1MUX(int lADDR1MUX, int lPC, int lBaseR);
int blockADDR2MUX(int lADDR2MUX, int lZero, int lOffset6, int lPCoffset9, int lPCoffset11);
int blockSR1MUX(int lSR1MUX, int lIR);
int blockSR2MUX(int lIR_5, int lSR2, int lImm);
int blockMARMUX(int lMARMUX, int lOutputOfLSHF1_ZEXT, int lOutputOfAddrAdder);
int blockALU(int lALUK, int lSR1, int lOP2);
int blockSHF(int lIR_5_0, int lSR1);
int blockMDRLogic1(int lMAR_0, int lDATA_SIZE, int lMDR);
int blockMDRLogic2(int lMAR_0, int lDATA_SIZE, int lBUS);
int blockPCMUX(int lPCMUX, int lPCAdd2, int lBUS, int lAdder);
int blockDRMUX(int lDRMUX, int lIR_11_9, int lR7);

#endif
