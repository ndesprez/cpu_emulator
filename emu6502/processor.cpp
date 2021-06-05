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
#include <cstdio>
#include <cassert>
#include <cstring>
#include "processor.h"

#pragma warning(disable : 4996) // for strcpy() in Disassemble()

Processor::Processor(Memory *RAM) : RAM(*RAM)
{
	Source = nullptr;
	Target = nullptr;
	Data = 0;
	Address = 0;
	OpCode = 0;
	LastInstruction = nullptr;

	A = 0;
	X = 0;
	Y = 0;
	S = 0;
	P = 0;
	PC = 0;

	Clock = 0;

	EndOnBreak = false;

	ResetState = false;
	InterruptState = false;
	NonMaskableInterruptState = false;

	// leaves room for undocumented/illegal instructions
	for (int i = 0; i < 151; i++)
	{
		InstructionSet[LegalInstructionSet[i].OpCode] = &LegalInstructionSet[i];
	}
}

bool Processor::FlagCarry()
{
	return ReadFlag(fCarry);
}

bool Processor::FlagZero()
{
	return ReadFlag(fZero);
}

bool Processor::FlagInterrupt()
{
	return ReadFlag(fInterrupt);
}

bool Processor::FlagDecimal()
{
	return ReadFlag(fDecimal);
}

bool Processor::FlagOverflow()
{
	return ReadFlag(fOverflow);
}

bool Processor::FlagNegative()
{
	return ReadFlag(fNegative);
}

void Processor::SendRST()
{
	ResetState = true;
}

void Processor::SendIRQ()
{
	if (!ReadFlag(fInterrupt))
		InterruptState = true;
}

void Processor::SendNMI()
{
	NonMaskableInterruptState = true;
}

void Processor::Step()
{
	if (ResetState)
	{
		Reset();
		return;
	}

	const Instruction *ins = ReadInstruction();

	static char code[20];

	Disassemble(code, ins);
	//cout << code << endl;
	DecodeInstruction(ins);
	ExecuteInstruction(ins);

	if (NonMaskableInterruptState)
		NonMaskableInterrupt();
	else if (InterruptState)
		Interrupt();
}

void Processor::Step(int Count)
{
	for (int i = 0; i < Count; i++)
		Step();
}

void Processor::Run()
{
	// TODO: maybe we should skip do/while if EndOnBreak == false to avoid infinite loop
	do
	{
		Step();
	} while ((OpCode != BreakOpCode) || !EndOnBreak);
}

bool Processor::IsLastInstruction(const char *Name)
{
	return (strcmp(LastInstruction->Name, Name) == 0);
}

bool Processor::IsLastInstruction(const char *Name, Sources Source)
{
	return ((strcmp(LastInstruction->Name, Name) == 0) && (LastInstruction->Source == Source));
}

bool Processor::IsLastInstruction(const char *Name, Sources Source, Targets Target)
{
	return ((strcmp(LastInstruction->Name, Name) == 0) && (LastInstruction->Source == Source) && (LastInstruction->Target == Target));
}

#pragma region internal functions
bool Processor::SignBit(byte Value)
{
	return Value & 0x80;
}

word Processor::Add(word A, byte B)
{
	return ((int)A + B) & 0xFFFF;
}

byte Processor::Add(byte A, byte B)
{
	return ((word)A + B) & 0xFF;
}

byte Processor::ReadByte(word Address)
{
	Tick();
	return RAM[Address];
}

byte Processor::ReadOpCode()
{
	OpCode = ReadByte(PC++);
	return OpCode;
}

word Processor::ReadWord(word Address)
{
	Tick(2);
	return RAM[Address] | (RAM[Add(Address, 1)] << 8);
}

void Processor::Push(byte Data)
{
	RAM[Add((word)0x100, S--)] = Data;
	Tick();
}

byte Processor::PullByte()
{
	Tick();
	return RAM[Add((word)0x100, ++S)];
}

bool Processor::ReadFlag(Flags Flag)
{
	return P & Flag;
}

void Processor::WriteFlag(Flags Flag, bool Value)
{
	if (Value)
		P |= Flag;
	else
		P &= ~Flag;
}

void Processor::WriteTargetFlags()
{
	WriteFlag(fZero, (*Target == 0));
	WriteFlag(fNegative, SignBit(*Target));
}

void Processor::Tick(byte Cycles)
{
	// TODO: insert throttling here
	Clock += Cycles;
}

#pragma endregion

#pragma region instructions
void Processor::Load()
{
	*Target = *Source;

	// here we assume that if the target is S, we're executing TXS
	// TODO: think about encoding which flags are affected in the Instruction struct
	if(Target != &S)
		WriteTargetFlags();
}

void Processor::Store()
{
	*Source = *Target;
}

void Processor::Compare()
{
	WriteFlag(fCarry, (*Target >= *Source));
	WriteFlag(fZero, (*Target == *Source));
	WriteFlag(fNegative, SignBit(*Target - *Source));
}

void Processor::And()
{
	*Target &= *Source;
	WriteTargetFlags();
}

void Processor::Xor()
{
	*Target ^= *Source;
	WriteTargetFlags();
}

void Processor::Or()
{
	*Target |= *Source;
	WriteTargetFlags();
}

void Processor::RotateLeft()
{
	byte c = ReadFlag(fCarry);
	WriteFlag(fCarry, SignBit(*Target));
	*Target = ((*Target) << 1) | c;
	Tick(2); // one cycle for modify, one cycle for write
	WriteTargetFlags();
}

void Processor::RotateRight()
{
	byte c = ReadFlag(fCarry);
	WriteFlag(fCarry, (*Target) & 1);
	*Target = ((*Target) >> 1) | (c << 7);
	Tick(2); // one cycle for modify, one cycle for write
	WriteTargetFlags();
}

void Processor::ShiftLeft()
{
	WriteFlag(fCarry, SignBit(*Target));
	*Target <<= 1;
	Tick(2); // one cycle for modify, one cycle for write
	WriteTargetFlags();
}

void Processor::ShiftRight()
{
	WriteFlag(fCarry, (*Target) & 1);
	*Target >>= 1;
	Tick(2); // one cycle for modify, one cycle for write
	WriteTargetFlags();
}

void Processor::Increment()
{
	(*Target)++;
	Tick(2); // one cycle for modify, one cycle for write
	WriteTargetFlags();
}

void Processor::Decrement()
{
	(*Target)--;
	Tick(2); // one cycle for modify, one cycle for write
	WriteTargetFlags();
}

void Processor::AddWithCarry()
{
	word result;
	if (FlagDecimal())
	{
		byte lo_nibble = (*Source & 0x0F) + (*Target & 0x0F) + ReadFlag(fCarry);
		word hi_nibble = (*Source & 0xF0) + (*Target & 0xF0);

		if (lo_nibble >= 0x0A)
		{
			lo_nibble -= 0x0A;
			hi_nibble += 0x10;
		}

		if (hi_nibble >= 0xA0)
			hi_nibble += 0x60;

		result = lo_nibble + hi_nibble;
		result = result;
	}
	else
	{
		result = *Target + *Source + ReadFlag(fCarry);
	}
	WriteFlag(fCarry, result & 0x100);
	// if both operands sign is identical but differs from the result sign (e.g. 100 + 49 = -107)
	WriteFlag(fOverflow, (*Source ^ result) & (*Target ^ result) & 0x80);
	*Target = result & 0xFF;

	if (!FlagDecimal())
		WriteTargetFlags();
}

void Processor::SubtractWithCarry()
{
	word result;

	if (FlagDecimal())
	{
		byte lo_nibble = (*Target & 0x0F) + ((0x99 - *Source) & 0x0F) + ReadFlag(fCarry);
		word hi_nibble = (*Target & 0xF0) + ((0x99 - *Source) & 0xF0);

		if (lo_nibble >= 0x0A)
		{
			lo_nibble -= 0x0A;
			hi_nibble += 0x10;
		}

		if (hi_nibble >= 0xA0)
			hi_nibble += 0x60;

		result = lo_nibble + hi_nibble;
		WriteFlag(fCarry, result & 0x100);
	}
	else
	{
		result = *Target - *Source - 1 + ReadFlag(fCarry);
		WriteFlag(fCarry, (result & 0x100) == 0);
	}
	// if both operands sign is identical but differs from the result sign (e.g. -100 - 49 = 107)
	WriteFlag(fOverflow, ~(*Source ^ result) & (*Target ^ result) & 0x80);
	*Target = result & 0xFF;
	
	if(!FlagDecimal())
		WriteTargetFlags();
}

void Processor::Push()
{
	Push(*Target);
}

void Processor::PushAddress(word Address)
{
	Push(Address >> 8);
	Push(Address & 0xFF);
}

void Processor::PullAddress(word &Address)
{
	Address = PullByte() | (PullByte() << 8);
}

void Processor::Pull()
{
	*Target = PullByte();
	Tick(); // discarded data
	if (Target == &P)
	{
		WriteFlag(fBreak, true);
		WriteFlag(fReserved, true);
	}
	else
	{
		WriteTargetFlags();
	}
}

void Processor::Branch()
{
	if ((PC & 0xFF00) != ((PC + (char)*Source) & 0xFF00))
		Tick();
	Tick();
	PC += (char)*Source;
}

void Processor::Jump()
{
	// this avoids PC = &RAM[Address] in absolute and indirect modes
	PC = Address;
}

void Processor::Call()
{
	PushAddress(PC - 1);
	Tick(); // discarded data
	Jump();
}

void Processor::Return()
{
	PullAddress(PC);
	Tick(2); // discarded data
	PC++;
}

void Processor::Break()
{
	if (!EndOnBreak)
	{
		PushAddress(PC + 1);
		Push(P | fBreak | fReserved);
		WriteFlag(fInterrupt, true);
		PC = ReadWord(InterruptVector);
	}
}

void Processor::Nop()
{
	// nope nope nope nope...
}

void Processor::BranchIfMinus()
{
	if (ReadFlag(fNegative))
		Branch();
}

void Processor::BranchIfPositive()
{
	if (!ReadFlag(fNegative))
		Branch();
}

void Processor::BranchIfEqual()
{
	if (ReadFlag(fZero))
		Branch();
}

void Processor::BranchIfNotEqual()
{
	if (!ReadFlag(fZero))
		Branch();
}

void Processor::BranchIfCarrySet()
{
	if (ReadFlag(fCarry))
		Branch();
}

void Processor::BranchIfCarryClear()
{
	if (!ReadFlag(fCarry))
		Branch();
}

void Processor::BranchIfOverflowSet()
{
	if (ReadFlag(fOverflow))
		Branch();
}

void Processor::BranchIfOverflowClear()
{
	if (!ReadFlag(fOverflow))
		Branch();
}

void Processor::ClearCarryFlag()
{
	WriteFlag(fCarry, false);
}

void Processor::ClearDecimalFlag()
{
	WriteFlag(fDecimal, false);
}

void Processor::ClearInterruptFlag()
{
	WriteFlag(fInterrupt, false);
}

void Processor::ClearOverflowFlag()
{
	WriteFlag(fOverflow, false);
}

void Processor::SetCarryFlag()
{
	WriteFlag(fCarry, true);
}

void Processor::SetDecimalFlag()
{
	WriteFlag(fDecimal, true);
}

void Processor::SetInterruptFlag()
{
	WriteFlag(fInterrupt, true);
}

void Processor::BitTest()
{
	WriteFlag(fZero, (A & *Target) == 0);
	WriteFlag(fOverflow, *Target & 0x40);
	WriteFlag(fNegative, *Target & 0x80);
}

void Processor::Reset()
{
	S = 0xFF;
	P = 0b00110100;
	PC = ReadWord(ResetVector);
	Clock = 0;
	ResetState = false;
	InterruptState = false;
	NonMaskableInterruptState = false;
}

void Processor::Interrupt()
{
	InterruptState = false;
	PushAddress(PC);
	Push(P & ~fBreak);
	WriteFlag(fInterrupt, true);
	PC = ReadWord(InterruptVector);
}

void Processor::NonMaskableInterrupt()
{
	NonMaskableInterruptState = false;
	PushAddress(PC);
	Push(P);
	WriteFlag(fInterrupt, true);
	PC = ReadWord(NonMaskableInterruptVector);
}

void Processor::ReturnFromInterrupt()
{
	P = PullByte();
	Tick(); // discarded data
	PullAddress(PC);
}

#pragma endregion

const Processor::Instruction *Processor::ReadInstruction()
{
	ReadOpCode();

	// TODO: handle exception when OpCode is undefined (InstructionSet[OpCode] == nullptr)
	const Instruction *ins = InstructionSet[OpCode];

	if ((OpCode != BreakOpCode) || (!EndOnBreak))
		LastInstruction = ins;

	assert(ins != nullptr);

	if (InstructionLength[ins->Source] == 2)
		Data = ReadByte(PC++);
	else if (InstructionLength[ins->Source] == 3)
	{
		Address = ReadWord(PC);
		PC += 2;
	}

	return ins;
}

void Processor::DecodeInstruction(const Instruction *Ins)
{
	switch (Ins->Source)
	{
	case sImplied:
		Source = nullptr;
		Tick(); // discarded opcode
		break;
	case sAccumulator:
		Source = &A;
		break;
	case sIndexX:
		Source = &X;
		break;
	case sIndexY:
		Source = &Y;
		break;
	case sStackPointer:
		Source = &S;
		break;
	case sAbsolute:
		Source = &RAM[Address];
		// if we write data in some way
		if (Ins->Target != tNone)
			Tick();
		break;
	case sAbsoluteX:
		Source = &RAM[Add(Address, X)];
		if (!Ins->InternalExecution || (Add((word)(Address & 0xFF), X) >= 0x100))
			Tick();
		Tick();
		break;
	case sAbsoluteY:
		Source = &RAM[Add(Address, Y)];
		if (!Ins->InternalExecution || (Add((word)(Address & 0xFF), Y) >= 0x100))
			Tick();
		Tick();
		break;
	case sImmediate:
		Source = &Data;
		break;
	case sIndirect:
		// JMP ($xxFF) bug (luckily the only instruction to use indirect mode)
		if ((Address & 0xFF) == 0xFF)
		{
			Address = RAM[Address] | (RAM[Address & 0xFF00] << 8);
			Tick(2);
		}
		else
			Address = ReadWord(Address);

		Source = &RAM[Address];
		break;
	case sXIndirect:
		Source = &RAM[ReadWord(Add(Data, X))];
		Tick(2);
		break;
	case sIndirectY:
		Address = ReadWord(Data);
		Source = &RAM[Add(Address, Y)];
		if (!Ins->InternalExecution || (Add((word)(Address & 0xFF), Y) >= 0x100))
			Tick();
		Tick();
		break;
	case sZeroPage:
		Source = &RAM[Data];
		Tick();
		break;
	case sZeroPageX:
		Source = &RAM[Add(Data, X)];
		Tick(2);
		break;
	case sZeroPageY:
		Source = &RAM[Add(Data, Y)];
		Tick(2);
		break;
	default:
		// unknown addressing mode ?
		break;
	}

	// TODO: this can be set in the LegalInstructionSet array initialisation
	switch (Ins->Target)
	{
	case tNone:
		Target = nullptr;
		break;
	case tAccumulator:
		Target = &A;
		break;
	case tIndexX:
		Target = &X;
		break;
	case tIndexY:
		Target = &Y;
		break;
	case tStackPointer:
		Target = &S;
		break;
	case tStatus:
		Target = &P;
		break;
	case tAddress:
		Target = Source;
		break;
	default:
		// unknown target ?
		break;
	}
}

void Processor::ExecuteInstruction(const Instruction *Ins)
{
	(this->*Ins->Function)();
}

void Processor::Disassemble(char Output[20], const Instruction * Ins)
{
	int f;
	word value;
	const char format[7][12] = {"%s #$%X" , "%s $%X", "%s $%X, X", "%s $%X, Y", "%s ($%X)","%s ($%X), X", "%s ($%X, Y)"};

	switch (Ins->Source)
	{
	case sImplied:
		strcpy(Output, Ins->Name);
		return;
	case sImmediate:
		if(Ins->Target == tNone)	// relative, i.e. branch
			f = 1;
		else
			f = 0;
		break;
	case sAbsolute:
	case sZeroPage:
		f = 1;
		break;
	case sAbsoluteX:
	case sZeroPageX:
		f = 2;
		break;
	case sAbsoluteY:
	case sZeroPageY:
		f = 3;
		break;
	case sIndirect:
		f = 4;
		break;
	case sXIndirect:
		f = 5;
		break;
	case sIndirectY:
		f = 6;
		break;
	}

	if (InstructionLength[Ins->Source] == 2)
		value = Data;
	else if (InstructionLength[Ins->Source] == 3)
		value = Address;

	snprintf(Output, 20, format[f], Ins->Name, value);
}
