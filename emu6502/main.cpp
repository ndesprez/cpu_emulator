#include "processor.h"

Processor *CPU;
byte	  *RAM;

int main(void)
{
	RAM = new byte[0x10000];

	// set PC vector to 0x1000
	RAM[0xFFFC] = 0x00;
	RAM[0xFFFD] = 0x10;
/*
	RAM[0x1000] = 0xA2;
	RAM[0x1001] = 0x20;
	RAM[0x1002] = 0xA1;
	RAM[0x1003] = 0x20;
	RAM[0x0040] = 0x00;
	RAM[0x0041] = 0x30;
	RAM[0x3000] = 0xDF;
*/

	RAM[0x1000] = 0x38;

	CPU = new Processor(RAM);
	CPU->SendRST();
	CPU->Step(2);

	bool f = CPU->FlagCarry();

	delete CPU;
	delete[] RAM;

	return 0;
}