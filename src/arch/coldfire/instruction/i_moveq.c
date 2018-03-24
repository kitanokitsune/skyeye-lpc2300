/**********************************/
/*                                */
/*  Copyright 2000, David Grant   */
/*                                */
/*  see LICENSE for more details  */
/*                                */
/**********************************/

#include "coldfire.h"

/* Move instruction */

/* Format 
   
 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 0 | 1 | 1 | 1 |  Register | 0 |            Data               |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

*/

int MOVEQTime=1;


INSTRUCTION_4ARGS(MOVEQ,
	unsigned Code2,4,
	unsigned Register,3,
	unsigned Code1,1,
	signed   Data,8);

static void execute(void)
{
	struct _Address Destination;
	unsigned int SValue;
	MOVEQ_Instr Instr;
	Memory_RetrWordFromPC(&Instr.Code);

	SValue=(unsigned int)Instr.Bits.Data;
	
	if(!EA_GetFromPC(&Destination, 32, 0, Instr.Bits.Register)) return;

	EA_PutValue(&Destination, SValue);

	/* 	X - not affected
		N - set if result is -ve, cleared otherwise
		Z - set if result is zero, cleared otherwise
		V - always cleared
		C - always cleared */
		/* Set the status register */
	memory_core.sr &= 0xFFF0;
	SRBits->N = ((int)SValue < 0);
	SRBits->Z = (SValue == 0);


	cycle(MOVEQTime);

	return;
}

static int disassemble(char *Instruction, char *Arg1, char *Arg2)
{
	MOVEQ_Instr Instr;
	Memory_RetrWordFromPC(&Instr.Code);

	sprintf(Instruction, "MOVEQ");

	sprintf(Arg1, "#0x%02X", ((char)Instr.Bits.Data) & 0x000000FF);
	Addressing_Print(32, 0, Instr.Bits.Register, Arg2);
	return 0;
}

int moveq_5206_register(void)
{
	instruction_register(0x7000, 0xF100, &execute, &disassemble);
	return 1;
}
