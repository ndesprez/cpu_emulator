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

#include <string.h>

using byte = unsigned char;
using word = unsigned short;

// TODO: add a namespace?

// status register flags
enum Flags : byte {
	fCarry		= 1,
	fZero		= 2,
	fInterrupt	= 4,
	fDecimal	= 8,
	fBreak		= 16,	// always set to 1
	// status bit 5 is always set to 1
	fOverflow	= 64,
	fNegative	= 128
};

// equivalent to addressing modes plus extra for transfer instructions
enum SourceType {
	sAccumulator,	// TAX, TAY
	sIndexX,		// TXA, TXS
	sIndexY,		// TYA
	sStackPointer,	// TSX
	sAbsolute,		// LDA $1234
	sAbsoluteX,		// LDA $1234, X
	sAbsoluteY,		// LDA $1234, Y
	sImmediate,		// LDA #$12
	sImplied,		// BRK, INX, CLC, ASL A, etc.
	sIndirect,		// JMP ($1234)
	sXIndirect,		// LDA ($12), X
	sIndirectY,		// LDA ($12, Y)
	sZeroPage,		// LDA $12
	sZeroPageX,		// LDA $12, X
	sZeroPageY		// LDA $12, Y
};

enum TargetType {
	tNone,			// paired with sImplied, branch and jump instructions
	tAccumulator,	// AND, CMP, ADC, etc.
	tIndexX,		// CPX, TAX, LDX, etc.
	tIndexY,		// STY, TAY, LDY, etc.
	tStackPointer,	// TAS
	tStatus,		// PHP, PLP
	tAddress		// LSR, ROL, INC, etc.
};

class Processor
{
protected:
	const word NonMaskableInterruptVector	= 0xFFFA;
	const word ResetVector					= 0xFFFC;
	const word InterruptVector				= 0xFFFE;

	const byte BreakOpCode = 0x00;

	struct Instruction {
		byte			OpCode;
		const char		*Name;
		SourceType		Source;
		TargetType		Target;
		void			(Processor::*Function)();
	};

	// note: source and target are swapped for store instructions
	const Instruction	LegalInstructionSet[151] = {
		{0x61, "ADC",	sXIndirect,		tAccumulator,	&Processor::AddWithCarry},
		{0x65, "ADC",	sZeroPage,		tAccumulator,	&Processor::AddWithCarry},
		{0x69, "ADC",	sImmediate,		tAccumulator,	&Processor::AddWithCarry},
		{0x6D, "ADC",	sAbsolute,		tAccumulator,	&Processor::AddWithCarry},
		{0x71, "ADC",	sIndirectY,		tAccumulator,	&Processor::AddWithCarry},
		{0x75, "ADC",	sZeroPageX,		tAccumulator,	&Processor::AddWithCarry},
		{0x79, "ADC",	sAbsoluteY,		tAccumulator,	&Processor::AddWithCarry},
		{0x7D, "ADC",	sAbsoluteX,		tAccumulator,	&Processor::AddWithCarry},
		{0x21, "AND",	sXIndirect,		tAccumulator,	&Processor::And},
		{0x25, "AND",	sZeroPage,		tAccumulator,	&Processor::And},
		{0x29, "AND",	sImmediate,		tAccumulator,	&Processor::And},
		{0x2D, "AND",	sAbsolute,		tAccumulator,	&Processor::And},
		{0x31, "AND",	sIndirectY,		tAccumulator,	&Processor::And},
		{0x35, "AND",	sZeroPageX,		tAccumulator,	&Processor::And},
		{0x39, "AND",	sAbsoluteY,		tAccumulator,	&Processor::And},
		{0x3D, "AND",	sAbsoluteX,		tAccumulator,	&Processor::And},
		{0x06, "ASL",	sZeroPage,		tAddress,		&Processor::ShiftLeft},
		{0x0A, "ASL A",	sImplied,		tAccumulator,	&Processor::ShiftLeft},
		{0x0E, "ASL",	sAbsolute,		tAddress,		&Processor::ShiftLeft},
		{0x16, "ASL",	sZeroPageX,		tAddress,		&Processor::ShiftLeft},
		{0x1E, "ASL",	sAbsoluteX,		tAddress,		&Processor::ShiftLeft},
		{0x90, "BCC",	sImmediate,		tNone,			&Processor::BranchIfCarryClear},
		{0xB0, "BCS",	sImmediate,		tNone,			&Processor::BranchIfCarrySet},
		{0xF0, "BEQ",	sImmediate,		tNone,			&Processor::BranchIfEqual},
		{0x24, "BIT",	sZeroPage,		tAddress,		&Processor::BitTest},
		{0x2C, "BIT",	sAbsolute,		tAddress,		&Processor::BitTest},
		{0x30, "BMI",	sImmediate,		tNone,			&Processor::BranchIfMinus},
		{0xD0, "BNE",	sImmediate,		tNone,			&Processor::BranchIfNotEqual},
		{0x10, "BPL",	sImmediate,		tNone,			&Processor::BranchIfPositive},
		{0x00, "BRK",	sImplied,		tNone,			&Processor::Break},
		{0x50, "BVC",	sImmediate,		tNone,			&Processor::BranchIfOverflowClear},
		{0x70, "BVS",	sImmediate,		tNone,			&Processor::BranchIfOverflowSet},
		{0x18, "CLC",	sImplied,		tNone,			&Processor::ClearCarryFlag},
		{0xD8, "CLD",	sImplied,		tNone,			&Processor::ClearDecimalFlag},
		{0x58, "CLI",	sImplied,		tNone,			&Processor::ClearInterruptFlag},
		{0xB8, "CLV",	sImplied,		tNone,			&Processor::ClearOverflowFlag},
		{0xC1, "CMP",	sXIndirect,		tAccumulator,	&Processor::Compare},
		{0xC5, "CMP",	sZeroPage,		tAccumulator,	&Processor::Compare},
		{0xC9, "CMP",	sImmediate,		tAccumulator,	&Processor::Compare},
		{0xCD, "CMP",	sAbsolute,		tAccumulator,	&Processor::Compare},
		{0xD1, "CMP",	sIndirectY,		tAccumulator,	&Processor::Compare},
		{0xD5, "CMP",	sZeroPageX,		tAccumulator,	&Processor::Compare},
		{0xD9, "CMP",	sAbsoluteY,		tAccumulator,	&Processor::Compare},
		{0xDD, "CMP",	sAbsoluteX,		tAccumulator,	&Processor::Compare},
		{0xE0, "CPX",	sImmediate,		tIndexX,		&Processor::Compare},
		{0xE4, "CPX",	sZeroPage,		tIndexX,		&Processor::Compare},
		{0xEC, "CPX",	sAbsolute,		tIndexX,		&Processor::Compare},
		{0xC0, "CPY",	sImmediate,		tIndexY,		&Processor::Compare},
		{0xC4, "CPY",	sZeroPage,		tIndexY,		&Processor::Compare},
		{0xCC, "CPY",	sAbsolute,		tIndexY,		&Processor::Compare},
		{0xC6, "DEC",	sZeroPage,		tAddress,		&Processor::Decrement},
		{0xCE, "DEC",	sAbsolute,		tAddress,		&Processor::Decrement},
		{0xD6, "DEC",	sZeroPageX,		tAddress,		&Processor::Decrement},
		{0xDE, "DEC",	sAbsoluteX,		tAddress,		&Processor::Decrement},
		{0xCA, "DEX",	sImplied,		tIndexX,		&Processor::Decrement},
		{0x88, "DEY",	sImplied,		tIndexY,		&Processor::Decrement},
		{0x41, "EOR",	sXIndirect,		tAccumulator,	&Processor::Xor},
		{0x45, "EOR",	sZeroPage,		tAccumulator,	&Processor::Xor},
		{0x49, "EOR",	sImmediate,		tAccumulator,	&Processor::Xor},
		{0x4D, "EOR",	sAbsolute,		tAccumulator,	&Processor::Xor},
		{0x51, "EOR",	sIndirectY,		tAccumulator,	&Processor::Xor},
		{0x55, "EOR",	sZeroPageX,		tAccumulator,	&Processor::Xor},
		{0x59, "EOR",	sAbsoluteY,		tAccumulator,	&Processor::Xor},
		{0x5D, "EOR",	sAbsoluteX,		tAccumulator,	&Processor::Xor},
		{0xE6, "INC",	sZeroPage,		tAddress,		&Processor::Increment},
		{0xEE, "INC",	sAbsolute,		tAddress,		&Processor::Increment},
		{0xF6, "INC",	sZeroPageX,		tAddress,		&Processor::Increment},
		{0xFE, "INC",	sAbsoluteX,		tAddress,		&Processor::Increment},
		{0xE8, "INX",	sImplied,		tIndexX,		&Processor::Increment},
		{0xC8, "INY",	sImplied,		tIndexY,		&Processor::Increment},
		{0x4C, "JMP",	sAbsolute,		tNone,			&Processor::Jump},
		{0x6C, "JMP",	sIndirect,		tNone,			&Processor::Jump},
		{0x20, "JSR",	sAbsolute,		tNone,			&Processor::Call},
		{0xA1, "LDA",	sXIndirect,		tAccumulator,	&Processor::Load},
		{0xA5, "LDA",	sZeroPage,		tAccumulator,	&Processor::Load},
		{0xA9, "LDA",	sImmediate,		tAccumulator,	&Processor::Load},
		{0xAD, "LDA",	sAbsolute,		tAccumulator,	&Processor::Load},
		{0xB1, "LDA",	sIndirectY,		tAccumulator,	&Processor::Load},
		{0xB5, "LDA",	sZeroPageX,		tAccumulator,	&Processor::Load},
		{0xB9, "LDA",	sAbsoluteY,		tAccumulator,	&Processor::Load},
		{0xBD, "LDA",	sAbsoluteX,		tAccumulator,	&Processor::Load},
		{0xA2, "LDX",	sImmediate,		tIndexX,		&Processor::Load},
		{0xA6, "LDX",	sZeroPage,		tIndexX,		&Processor::Load},
		{0xAE, "LDX",	sAbsolute,		tIndexX,		&Processor::Load},
		{0xB6, "LDX",	sZeroPageY,		tIndexX,		&Processor::Load},
		{0xBE, "LDX",	sAbsoluteY,		tIndexX,		&Processor::Load},
		{0xA0, "LDY",	sImmediate,		tIndexY,		&Processor::Load},
		{0xA4, "LDY",	sZeroPage,		tIndexY,		&Processor::Load},
		{0xAC, "LDY",	sAbsolute,		tIndexY,		&Processor::Load},
		{0xB4, "LDY",	sZeroPageX,		tIndexY,		&Processor::Load},
		{0xBC, "LDY",	sAbsoluteX,		tIndexY,		&Processor::Load},
		{0x46, "LSR",	sZeroPage,		tAddress,		&Processor::ShiftRight},
		{0x4A, "LSR A",	sImplied,		tAccumulator,	&Processor::ShiftRight},
		{0x4E, "LSR",	sAbsolute,		tAddress,		&Processor::ShiftRight},
		{0x56, "LSR",	sZeroPageX,		tAddress,		&Processor::ShiftRight},
		{0x5E, "LSR",	sAbsoluteX,		tAddress,		&Processor::ShiftRight},
		{0xEA, "NOP",	sImplied,		tNone,			&Processor::Nop},
		{0x01, "ORA",	sXIndirect,		tAccumulator,	&Processor::Or},
		{0x05, "ORA",	sZeroPage,		tAccumulator,	&Processor::Or},
		{0x09, "ORA",	sImmediate,		tAccumulator,	&Processor::Or},
		{0x0D, "ORA",	sAbsolute,		tAccumulator,	&Processor::Or},
		{0x11, "ORA",	sIndirectY,		tAccumulator,	&Processor::Or},
		{0x15, "ORA",	sZeroPageX,		tAccumulator,	&Processor::Or},
		{0x19, "ORA",	sAbsoluteY,		tAccumulator,	&Processor::Or},
		{0x1D, "ORA",	sAbsoluteX,		tAccumulator,	&Processor::Or},
		{0x48, "PHA",	sImplied,		tAccumulator,	&Processor::Push},
		{0x08, "PHP",	sImplied,		tStatus,		&Processor::Push},
		{0x68, "PLA",	sImplied,		tAccumulator,	&Processor::Pull},
		{0x28, "PLP",	sImplied,		tStatus,		&Processor::Pull},
		{0x26, "ROL",	sZeroPage,		tAddress,		&Processor::RotateLeft},
		{0x2A, "ROL A",	sImplied,		tAccumulator,	&Processor::RotateLeft},
		{0x2E, "ROL",	sAbsolute,		tAddress,		&Processor::RotateLeft},
		{0x36, "ROL",	sZeroPageX,		tAddress,		&Processor::RotateLeft},
		{0x3E, "ROL",	sAbsoluteX,		tAddress,		&Processor::RotateLeft},
		{0x66, "ROR",	sZeroPage,		tAddress,		&Processor::RotateRight},
		{0x6A, "ROR A",	sImplied,		tAccumulator,	&Processor::RotateRight},
		{0x6E, "ROR",	sAbsolute,		tAddress,		&Processor::RotateRight},
		{0x76, "ROR",	sZeroPageX,		tAddress,		&Processor::RotateRight},
		{0x7E, "ROR",	sAbsoluteX,		tAddress,		&Processor::RotateRight},
		{0x40, "RTI",	sImplied,		tNone,			&Processor::ReturnFromInterrupt},
		{0x60, "RTS",	sImplied,		tNone,			&Processor::Return},
		{0xE1, "SBC",	sXIndirect,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xE5, "SBC",	sZeroPage,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xE9, "SBC",	sImmediate,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xED, "SBC",	sAbsolute,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xF1, "SBC",	sIndirectY,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xF5, "SBC",	sZeroPageX,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xF9, "SBC",	sAbsoluteY,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xFD, "SBC",	sAbsoluteX,		tAccumulator,	&Processor::SubtractWithCarry},
		{0x38, "SEC",	sImplied,		tNone,			&Processor::SetCarryFlag},
		{0xF8, "SED",	sImplied,		tNone,			&Processor::SetDecimalFlag},
		{0x78, "SEI",	sImplied,		tNone,			&Processor::SetInterruptFlag},
		{0x81, "STA",	sXIndirect,		tAccumulator,	&Processor::Store},
		{0x85, "STA",	sZeroPage,		tAccumulator,	&Processor::Store},
		{0x8D, "STA",	sAbsolute,		tAccumulator,	&Processor::Store},
		{0x91, "STA",	sIndirectY,		tAccumulator,	&Processor::Store},
		{0x95, "STA",	sZeroPageX,		tAccumulator,	&Processor::Store},
		{0x99, "STA",	sAbsoluteY,		tAccumulator,	&Processor::Store},
		{0x9D, "STA",	sAbsoluteX,		tAccumulator,	&Processor::Store},
		{0x86, "STX",	sZeroPage,		tIndexX,		&Processor::Store},
		{0x8E, "STX",	sAbsolute,		tIndexX,		&Processor::Store},
		{0x96, "STX",	sZeroPageY,		tIndexX,		&Processor::Store},
		{0x84, "STY",	sZeroPage,		tIndexY,		&Processor::Store},
		{0x8C, "STY",	sAbsolute,		tIndexY,		&Processor::Store},
		{0x94, "STY",	sZeroPageX,		tIndexY,		&Processor::Store},
		{0xAA, "TAX",	sAccumulator,	tIndexX,		&Processor::Load},
		{0xA8, "TAY",	sAccumulator,	tIndexY,		&Processor::Load},
		{0xBA, "TSX",	sStackPointer,	tIndexX,		&Processor::Load},
		{0x8A, "TXA",	sIndexX,		tAccumulator,	&Processor::Load},
		{0x9A, "TXS",	sIndexX,		tStackPointer,	&Processor::Load},
		{0x98, "TYA",	sIndexY,		tAccumulator,	&Processor::Load}
	};

	const Instruction*	InstructionSet[256] = {};
	const Instruction*	LastInstruction;

	byte	*Memory;		// 64kb of RAM (hopefully)
	byte	*Source;		// instruction source
	byte	*Target;		// instruction target
	byte	Data;			// data register
	byte	OpCode;			// instruction register
	word	Address;		// address register

	bool ResetState;				// true if SendRST() was called
	bool InterruptState;			// true if SendIRQ() was called
	bool NonMaskableInterruptState;	// true if SendNMI() was called
	
#pragma region internal functions
	bool SignBit(byte Value);
	word Add(word A, byte B);
	byte Add(byte A, byte B);
	byte ReadData(word Address);
	void WriteData(word Address);
	byte ReadOpCode();
	void ReadDataAtPC();
	word ReadAddress(word Address);
	void WriteAddress(word Address);
	void ReadAddressAtPC();
	void Push(byte Data);
	byte PullByte();
	bool ReadFlag(Flags Flag);
	void WriteFlag(Flags Flag, bool Value);
	void WriteTargetFlags();
#pragma endregion

#pragma region instructions
	void Load();
	void Store();
	void Compare();
	void And();
	void Xor();
	void Or();
	void RotateLeft();
	void RotateRight();
	void ShiftLeft();
	void ShiftRight();
	void Increment();
	void Decrement();
	void AddWithCarry();
	void SubtractWithCarry();
	void Push();
	void PushAddress(word Address);
	void PullAddress(word &Address);
	void Pull();
	void Branch();
	void Jump();
	void Call();
	void Return();
	void Break();
	void Nop();
	void BranchIfMinus();
	void BranchIfPositive();
	void BranchIfEqual();
	void BranchIfNotEqual();
	void BranchIfCarrySet();
	void BranchIfCarryClear();
	void BranchIfOverflowSet();
	void BranchIfOverflowClear();
	void ClearCarryFlag();
	void ClearDecimalFlag();
	void ClearInterruptFlag();
	void ClearOverflowFlag();
	void SetCarryFlag();
	void SetDecimalFlag();
	void SetInterruptFlag();
	void BitTest();
#pragma endregion
	void Reset();
	void Interrupt();
	void NonMaskableInterrupt();
	void ReturnFromInterrupt();

	void ExecuteInstruction();
			
public:
	byte	A;		// accumulator
	byte	X, Y;	// index registers
	word	PC;		// program counter
	byte	S;		// stack pointer
	byte	P;		// status flags
	bool	EndOnBreak;

	Processor(byte *Array);
	bool FlagCarry();
	bool FlagZero();
	bool FlagInterrupt();
	bool FlagDecimal();
	bool FlagOverflow();
	bool FlagNegative();
	void SendRST();
	void SendIRQ();
	void SendNMI();
	void Step();
	void Step(int Count);
	void Run();
	bool IsLastInstruction(const char *Name);
	bool IsLastInstruction(const char *Name, SourceType Source);
	bool IsLastInstruction(const char *Name, SourceType Source, TargetType Target);
};
