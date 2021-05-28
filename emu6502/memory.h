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

#pragma once

#include "types.h"

class Memory
{
protected:
	byte	*Array;

public:
	word	WriteCounter;

	Memory(void);
	~Memory(void);
	byte operator [] (word Index) const;
	byte& operator [] (word Index);
	char * Read(char *Buffer, word Address, word Size);
	void Write(char const *Data, bool AddBreak = true);
	void Write(word Address, char const * Data, bool AddBreak = false);
	bool ReadFile(const char *);
};