#include <assert.h>
#include <ctype.h>

#include "processor.h"

Processor *CPU;
byte	  *RAM;

word		WriteCounter;

void _write(char const *Data, bool AddBreak = true)
{
	byte value;
	word i = 0;
	bool high = true;

	while (char c = Data[i])
	{
		if (c == ' ')
		{
			assert(high == true);
			i++;
			continue;
		}

		assert(isxdigit(c));

		if (high)
			value = 0;

		if (c >= '0' && c <= '9')
			value |= c - 0x30;
		else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f')
			value |= (c | 0x20) - 0x57;

		if (high)
			value <<= 4;
		else
			RAM[WriteCounter++] = value;

		high = !high;
		i++;
	}
	assert(high == true);

	if (AddBreak)
	{
		RAM[WriteCounter] = 0x00;
	}
}

void _write(word Address, char const *Data, bool AddBreak = false)
{
	WriteCounter = Address;
	_write(Data, AddBreak);
}

int main(void)
{
	RAM = new byte[0x10000];

	// set PC vector to 0x1000
	RAM[0xFFFC] = 0x00;
	RAM[0xFFFD] = 0x10;

	CPU = new Processor(RAM);
	CPU->EndOnBreak = true;
	CPU->SendRST();
	CPU->Step();
	WriteCounter = CPU->PC;
	_write("A2 30 18 36 30");
	_write(0x0060, "C4");
	CPU->Run();

	delete CPU;
	delete[] RAM;

	return 0;
}