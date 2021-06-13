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

#include <cassert>
#include <fstream>
#include <cctype>
#include <cstring>
#include "memory.h"

using namespace std;

Memory::Memory(void)
{
	Array = new byte[0x10000];
	WriteCounter = 0;
}

Memory::~Memory(void)
{
	delete[] Array;
}

byte Memory::NibbleToByte(const char Nibble)
{
	if (Nibble >= '0' && Nibble <= '9')
		return Nibble - 0x30;
	else if ((Nibble | 0x20) >= 'a' && (Nibble | 0x20) <= 'f')
		return (Nibble | 0x20) - 0x57;
	else
		return 0;
}

byte Memory::HexToByte(const char *Hex)
{
	return NibbleToByte(*(Hex + 1)) + (NibbleToByte(*Hex) << 4);
}

word Memory::HexToWord(const char *Hex)
{
	return HexToByte(Hex + 2) + (HexToByte(Hex) << 8);
}

byte Memory::operator [] (word Index) const
{
	return Array[Index];
}

byte& Memory::operator[] (word Index)
{
	return Array[Index];
}

char *Memory::Read(char *Buffer, word Address, word Size)
{
	for (int i = 0; i < Size; i++)
	{
		word a = i + Address;

		Buffer[i * 3] = (Array[a] >> 4) + ((Array[a] >> 4) > 9 ? 55 : 48);
		Buffer[i * 3 + 1] = (Array[a] & 0x0F) + ((Array[a] & 0x0F) > 9 ? 55 : 48);
		Buffer[i * 3 + 2] = ' ';
	}

	Buffer[Size * 3] = 0;

	return Buffer;
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
		}
		else
		{
			assert(isxdigit(c));

			if (high)
				value = 0;

			value |= NibbleToByte(c);

			if (high)
				value <<= 4;
			else
				Array[WriteCounter++] = value;

			high = !high;
		}
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

// TODO: return a more explicit error (exception?)
bool Memory::ReadFile(const char *filename)
{
	ifstream	file;
	bool		hex_format = false;
	bool		success = false;
	
	if (_stricmp(strrchr(filename, '.'), ".hex") == 0)
	{
		file = ifstream(filename);
		hex_format = true;
	}
	else
		file = ifstream(filename, ios::binary);

	if (file.is_open())
	{
		file.seekg(0);

		for (int i = 0; i < 0x10000; i++)
		{
			Array[i] = 0xFF;
		}

		if (hex_format)
		{
			//   1 =  1 : semicolon
			// + 2 =  3 : byte count
			// + 4 =  7 : address
			// + 2 =  9 : record type
			// + x      : length * 2, up to 510
			// + 2 = 11 : checksum
			// + 1 = 12 : terminator
			char	row[522];
			int		line = 1;
			bool	end_of_file = false;

			do
			{
				file.getline(row, 522);

				if (file.fail())
				{
					success = false; // ERROR: row delimiter not found (line probably too long)
					break;
				}

				if (row[0] != ':')
				{
					success = false; // ERROR: line should start with a semicolon
					break;
				}

				byte byte_count = HexToByte(row + 1);
				word address = HexToWord(row + 3);
				byte record_type = HexToByte(row + 7);
				int	 checksum = 0;

				for (int i = 0; i < byte_count + 5; i++)
					checksum += HexToByte(row + i * 2 + 1);

				if (checksum & 0xFF)
				{
					success = false; // ERROR: checksum error
					break;
				}

				switch (record_type)
				{
				case 00:	// data
					for (int i = 0; i < byte_count; i++)
					{
						Array[address + i] = HexToByte(row + i * 2 + 9);
					}
					break;
				case 01:	// end of file
					end_of_file = true;
					success = true;
					break;
				default:
					break;
				}

				line++;
			} while (!end_of_file && !file.eof());
		}
		else 
		{
			file.read((char *)Array, 0x10000);
			success = true;
		}

		file.close();
	}

	return success;
}