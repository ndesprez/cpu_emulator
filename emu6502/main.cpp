/*
CPU emulator (https://github.com/ndesprez/cpu_emulator)
Copyright(C) 2021 Nicolas Desprez

This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

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