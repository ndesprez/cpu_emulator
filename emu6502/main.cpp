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

#include <iostream>
#include <bitset>
#include "processor.h"

using namespace std;

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		Memory *RAM = new Memory();

		Processor *CPU = new Processor(RAM);
		CPU->EndOnBreak = false;
		CPU->SendRST();
		CPU->Step();
		CPU->PC = 0x400;

		if (RAM->ReadFile(argv[1]))
		{
			word previous_pc;
			do
			{
				previous_pc = CPU->PC;
				CPU->Step();
			} while (previous_pc != CPU->PC);

			char *buffer = new char[16 * 3 + 1];

			for (int a = 0; a < 0x100; a += 16)
			{
				cout << RAM->Read(buffer, a + 0x200, 16) << endl;
			}
			cout << uppercase << hex << endl;

			cout << CPU->PC - 30 << ": " << RAM->Read(buffer, CPU->PC - 30, 16) << endl;
			cout << CPU->PC - 14 << ": " << RAM->Read(buffer, CPU->PC - 14, 16) << endl << endl;
			cout << RAM->Read(buffer, 0x1F0, 16) << endl << endl;
			delete[] buffer;

			cout << "A  = " << uppercase << hex << (int)CPU->A << endl;
			cout << "X  = " << (int)CPU->X << endl;
			cout << "Y  = " << (int)CPU->Y << endl;
			cout << "S  = " << (int)CPU->S << endl;
			cout << "PC = " << (int)CPU->PC << endl;
			cout << "     NO-BDIZC" << endl;
			cout << "P  = " << bitset<8>(CPU->P) << endl;
		}
		else
		{
			cout << "Cannot open file \"" << argv[1] << "\"" << endl;
		}
	}
	else
	{
		cout << "Missing argument" << endl;
	}
	return 0;
}