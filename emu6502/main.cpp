#include "processor.h"

Processor *MOS6502;
byte	  *Memory;

int main(void)
{
	Memory = new byte[0x10000];

	// set PC vector to 0x1000
	Memory[0xFFFC] = 0x00;
	Memory[0xFFFD] = 0x10;

	Memory[0x1000] = 0xA9; // LDA #
	Memory[0x1001] = 69;

	Memory[0x1002] = 0xAA; // TAX

	Memory[0x1003] = 0xCA; // DEX

	MOS6502 = new Processor(Memory);
	MOS6502->Reset();
	MOS6502->Step();
	MOS6502->Step();
	MOS6502->Step();

	delete MOS6502;
	delete Memory;

	return 0;
}