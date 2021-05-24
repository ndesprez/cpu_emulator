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

#include "processor.h"

Processor::Processor(byte *Array)
{
	Memory = Array;
	Source = nullptr;
	Target = nullptr;
	Data = 0;
	Address = 0;
	OpCode = 0;

	A = 0;
	X = 0;
	Y = 0;
	S = 0;
	P = 0;
	PC = 0;

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

	ExecuteInstruction();

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
	do
	{
		Step();
	} while ((OpCode != BreakOpCode) || !EndOnBreak);
}

bool Processor::IsLastInstruction(const char *Name)
{
	return (strcmp(LastInstruction->Name, Name) == 0);
}

bool Processor::IsLastInstruction(const char *Name, SourceType Source)
{
	return ((strcmp(LastInstruction->Name, Name) == 0) && (LastInstruction->Source == Source));
}

bool Processor::IsLastInstruction(const char *Name, SourceType Source, TargetType Target)
{
	return ((strcmp(LastInstruction->Name, Name) == 0) && (LastInstruction->Source == Source) && (LastInstruction->Target == Target));
}

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

byte Processor::ReadData(word Address)
{
	Data = Memory[Address];

	return Data;
}

void Processor::WriteData(word Address)
{
	Memory[Address] = Data;
}

byte Processor::ReadOpCode()
{
	OpCode = Memory[PC++];

	return OpCode;
}

void Processor::ReadDataAtPC()
{
	ReadData(PC++);
}

word Processor::ReadAddress(word Address)
{
	this->Address = Memory[Address] | (Memory[Add(Address, 1)] << 8);

	return this->Address;
}

void Processor::WriteAddress(word Address)
{
	Memory[Address] = this->Address & 0xFF;
	Memory[Add(Address, 1)] = this->Address >> 8;
}

void Processor::ReadAddressAtPC()
{
	ReadAddress(PC);
	PC += 2;
}

void Processor::Push(byte Data)
{
	Memory[Add((word)0x100, S--)] = Data;
}

byte Processor::PullByte()
{
	return Memory[Add((word)0x100, ++S)];
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

// AddWithCarry(byte) avoids the duplication of code between ADC and SBC
void Processor::AddWithCarry(byte Value, byte Carry)
{
	word result;
	if (FlagDecimal())
	{
		byte lo_nibble = (Value & 0x0F) + (*Target & 0x0F) + Carry;
		byte hi_nibble = (Value & 0xF0) + (*Target & 0xF0);

		if (lo_nibble >= 0x0A)
			lo_nibble += 0x06;

		result = lo_nibble + hi_nibble;

		if (result >= 0xA0)
			result += 0x60;
	}
	else
	{
		result = *Target + Value + ReadFlag(fCarry);
	}
	// if both operands sign is identical but differs from the result sign (e.g. 100 + 49 = -107)
	WriteFlag(fOverflow, (Value ^ result) & (*Target ^ result) & 0x80);
	*Target = result & 0xFF;
	WriteFlag(fCarry, result & 0x100);
	WriteTargetFlags();
}

#pragma endregion

#pragma region instructions
void Processor::Load()
{
	*Target = *Source;
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
	// TODO: code a better byte subtraction if necessary
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
	WriteTargetFlags();
}

void Processor::RotateRight()
{
	byte c = ReadFlag(fCarry);
	WriteFlag(fCarry, (*Target) & 1);
	*Target = ((*Target) >> 1) | (c << 7);
	WriteTargetFlags();
}

void Processor::ShiftLeft()
{
	WriteFlag(fCarry, SignBit(*Target));
	*Target <<= 1;
	WriteTargetFlags();
}

void Processor::ShiftRight()
{
	WriteFlag(fCarry, (*Target) & 1);
	*Target >>= 1;
	WriteTargetFlags();
}

void Processor::Increment()
{
	(*Target)++;
	WriteTargetFlags();
}

void Processor::Decrement()
{
	(*Target)--;
	WriteTargetFlags();
}

void Processor::AddWithCarry()
{
	AddWithCarry(*Source, ReadFlag(fCarry));
}

void Processor::SubtractWithCarry()
{
	AddWithCarry(~*Source, -1 + ReadFlag(fCarry));
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
}

void Processor::Branch()
{
	PC += (char)*Source;
}

void Processor::Jump()
{
	// this avoids PC = &Memory[Address] in absolute and indirect modes
	PC = Address;
}

void Processor::Call()
{
	PushAddress(PC - 1);
	Jump();
}

void Processor::Return()
{
	PullAddress(PC);
	PC++;
}

void Processor::Break()
{
	if (!EndOnBreak)
	{
		PushAddress(PC);
		Push(P | fBreak);
		PC = ReadAddress(InterruptVector);
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
	S = 0xFD;
	P = 0b00110000;
	PC = ReadAddress(ResetVector);
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
	PC = ReadAddress(InterruptVector);
}

void Processor::NonMaskableInterrupt()
{
	NonMaskableInterruptState = false;
	PushAddress(PC);
	Push(P);
	WriteFlag(fInterrupt, true);
	PC = ReadAddress(NonMaskableInterruptVector);
}

void Processor::ReturnFromInterrupt()
{
	P = PullByte();
	Return();
}

#pragma endregion

void Processor::ExecuteInstruction()
{
	ReadOpCode();

	// TODO: handle exception when OpCode is undefined (InstructionSet[OpCode] == nullptr)
	const Instruction *in = InstructionSet[OpCode];

	if ((OpCode != BreakOpCode) || (!EndOnBreak))
		LastInstruction = in;

	switch (in->Source)
	{
	case sImplied:
		Source = nullptr;
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
		ReadAddressAtPC();
		Source = &Memory[Address];
		break;
	case sAbsoluteX:
		ReadAddressAtPC();
		Source = &Memory[Add(Address, X)];
		break;
	case sAbsoluteY:
		ReadAddressAtPC();
		Source = &Memory[Add(Address, Y)];
		break;
	case sImmediate:
		ReadDataAtPC();
		Source = &Data;
		break;
	case sIndirect:
		ReadAddressAtPC();

		// JMP ($xxFF) bug (luckily the only instruction to use indirect mode)
		if ((Address & 0xFF) == 0xFF)
			Address = Memory[Address] | (Memory[Address & 0xFF00] << 8);
		else
			ReadAddress(Address);

		Source = &Memory[Address];
		break;
	case sXIndirect:
		ReadDataAtPC();
		ReadAddress(Add(Data, X));
		Source = &Memory[Address];
		break;
	case sIndirectY:
		ReadDataAtPC();
		ReadAddress(Data);
		Source = &Memory[Add(Address, Y)];
		break;
	case sZeroPage:
		ReadDataAtPC();
		Source = &Memory[Data];
		break;
	case sZeroPageX:
		ReadDataAtPC();
		Source = &Memory[Add(Data, X)];
		break;
	case sZeroPageY:
		ReadDataAtPC();
		Source = &Memory[Add(Data, Y)];
		break;
	default:
		// unknown addressing mode ?
		break;
	}

	// TODO: this can be set in the LegalInstructionSet array initialisation
	switch (in->Target)
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

	(this->*in->Function)();
}