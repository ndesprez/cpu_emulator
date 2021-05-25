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
	void Write(char const *Data, bool AddBreak = true);
	void Write(word Address, char const * Data, bool AddBreak = false);
};