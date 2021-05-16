#include "processor.h"

Processor *MOS6502;
byte	  *Memory;

int main(void)
{
	Memory = new byte[0x10000];

	// set PC vector to 0x1000
	Memory[0xFFFC] = 0x00;
	Memory[0xFFFD] = 0x10;

	Memory[0x1000] = 0xA2;
	Memory[0x1001] = 0x20;
	Memory[0x1002] = 0xA1;
	Memory[0x1003] = 0x20;
	Memory[0x0040] = 0x00;
	Memory[0x0041] = 0x30;
	Memory[0x3000] = 0xDF;

	MOS6502 = new Processor(Memory);
	MOS6502->SendRST();
	MOS6502->Step(3);

	delete MOS6502;
	delete[] Memory;

	return 0;
}