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
#include "memory.h"

Memory::Memory(void)
{
	Array = new byte[0x10000];
	WriteCounter = 0;
}

Memory::~Memory(void)
{
	delete[] Array;
}

byte Memory::operator [] (word Index) const
{
	return Array[Index];
}

byte& Memory::operator[](word Index)
{
	return Array[Index];
}

void Memory::Write(char const *Data, bool AddBreak)
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
			Array[WriteCounter++] = value;

		high = !high;
		i++;
	}
	assert(high == true);

	if (AddBreak)
	{
		Array[WriteCounter] = 0x00;
	}
}

void Memory::Write(word Address, char const *Data, bool AddBreak)
{
	WriteCounter = Address;
	Write(Data, AddBreak);
}