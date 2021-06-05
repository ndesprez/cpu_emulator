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
#include "memory.h"

using namespace std;

// TODO: add a namespace?

// status register flags
enum Flags : byte {
	fCarry		= 1,
	fZero		= 2,
	fInterrupt	= 4,
	fDecimal	= 8,
	fBreak		= 16,	// always set to 1
	fReserved	= 32,	// always set to 1
	fOverflow	= 64,
	fNegative	= 128
};

// equivalent to addressing modes plus extra for transfer instructions
enum Sources {
	sAccumulator = 0,	// TAX, TAY
	sIndexX,			// TXA, TXS
	sIndexY,			// TYA
	sStackPointer,		// TSX
	sAbsolute,			// LDA $1234
	sAbsoluteX,			// LDA $1234, X
	sAbsoluteY,			// LDA $1234, Y
	sImmediate,			// LDA #$12
	sImplied,			// BRK, INX, CLC, ASL A, etc.
	sIndirect,			// JMP ($1234)
	sXIndirect,			// LDA ($12), X
	sIndirectY,			// LDA ($12, Y)
	sZeroPage,			// LDA $12
	sZeroPageX,			// LDA $12, X
	sZeroPageY			// LDA $12, Y
};

// instruction length in bytes, indexed by SourceType
// TODO: could it be encoded in the lower two bits of SourceType?
const byte InstructionLength[15] = {1, 1, 1, 1, 3, 3, 3, 2, 1, 3, 2, 2, 2, 2, 2};

enum Targets {
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
		byte		OpCode;						// machine language opcode
		const char	*Name;						// assembly instruction name
		bool		InternalExecution;			// if true, instruction can skip a cycle in some addressing modes
		Sources		Source;						// addressing mode
		Targets		Target;						// the type of data to be changed
		void		(Processor::*Function)();	// the Processor function to execute when the instruction is decoded
	};

	// note: source and target are swapped for store instructions
	const Instruction	LegalInstructionSet[151] = {
		{0x61, "ADC",	true,	sXIndirect,		tAccumulator,	&Processor::AddWithCarry},
		{0x65, "ADC",	true,	sZeroPage,		tAccumulator,	&Processor::AddWithCarry},
		{0x69, "ADC",	true,	sImmediate,		tAccumulator,	&Processor::AddWithCarry},
		{0x6D, "ADC",	true,	sAbsolute,		tAccumulator,	&Processor::AddWithCarry},
		{0x71, "ADC",	true,	sIndirectY,		tAccumulator,	&Processor::AddWithCarry},
		{0x75, "ADC",	true,	sZeroPageX,		tAccumulator,	&Processor::AddWithCarry},
		{0x79, "ADC",	true,	sAbsoluteY,		tAccumulator,	&Processor::AddWithCarry},
		{0x7D, "ADC",	true,	sAbsoluteX,		tAccumulator,	&Processor::AddWithCarry},
		{0x21, "AND",	true,	sXIndirect,		tAccumulator,	&Processor::And},
		{0x25, "AND",	true,	sZeroPage,		tAccumulator,	&Processor::And},
		{0x29, "AND",	true,	sImmediate,		tAccumulator,	&Processor::And},
		{0x2D, "AND",	true,	sAbsolute,		tAccumulator,	&Processor::And},
		{0x31, "AND",	true,	sIndirectY,		tAccumulator,	&Processor::And},
		{0x35, "AND",	true,	sZeroPageX,		tAccumulator,	&Processor::And},
		{0x39, "AND",	true,	sAbsoluteY,		tAccumulator,	&Processor::And},
		{0x3D, "AND",	true,	sAbsoluteX,		tAccumulator,	&Processor::And},
		{0x06, "ASL",	false,	sZeroPage,		tAddress,		&Processor::ShiftLeft},
		{0x0A, "ASL A",	false,	sImplied,		tAccumulator,	&Processor::ShiftLeft},
		{0x0E, "ASL",	false,	sAbsolute,		tAddress,		&Processor::ShiftLeft},
		{0x16, "ASL",	false,	sZeroPageX,		tAddress,		&Processor::ShiftLeft},
		{0x1E, "ASL",	false,	sAbsoluteX,		tAddress,		&Processor::ShiftLeft},
		{0x90, "BCC",	false,	sImmediate,		tNone,			&Processor::BranchIfCarryClear},
		{0xB0, "BCS",	false,	sImmediate,		tNone,			&Processor::BranchIfCarrySet},
		{0xF0, "BEQ",	false,	sImmediate,		tNone,			&Processor::BranchIfEqual},
		{0x24, "BIT",	true,	sZeroPage,		tAddress,		&Processor::BitTest},
		{0x2C, "BIT",	true,	sAbsolute,		tAddress,		&Processor::BitTest},
		{0x30, "BMI",	false,	sImmediate,		tNone,			&Processor::BranchIfMinus},
		{0xD0, "BNE",	false,	sImmediate,		tNone,			&Processor::BranchIfNotEqual},
		{0x10, "BPL",	false,	sImmediate,		tNone,			&Processor::BranchIfPositive},
		{0x00, "BRK",	false,	sImplied,		tNone,			&Processor::Break},
		{0x50, "BVC",	false,	sImmediate,		tNone,			&Processor::BranchIfOverflowClear},
		{0x70, "BVS",	false,	sImmediate,		tNone,			&Processor::BranchIfOverflowSet},
		{0x18, "CLC",	false,	sImplied,		tNone,			&Processor::ClearCarryFlag},
		{0xD8, "CLD",	false,	sImplied,		tNone,			&Processor::ClearDecimalFlag},
		{0x58, "CLI",	false,	sImplied,		tNone,			&Processor::ClearInterruptFlag},
		{0xB8, "CLV",	false,	sImplied,		tNone,			&Processor::ClearOverflowFlag},
		{0xC1, "CMP",	true,	sXIndirect,		tAccumulator,	&Processor::Compare},
		{0xC5, "CMP",	true,	sZeroPage,		tAccumulator,	&Processor::Compare},
		{0xC9, "CMP",	true,	sImmediate,		tAccumulator,	&Processor::Compare},
		{0xCD, "CMP",	true,	sAbsolute,		tAccumulator,	&Processor::Compare},
		{0xD1, "CMP",	true,	sIndirectY,		tAccumulator,	&Processor::Compare},
		{0xD5, "CMP",	true,	sZeroPageX,		tAccumulator,	&Processor::Compare},
		{0xD9, "CMP",	true,	sAbsoluteY,		tAccumulator,	&Processor::Compare},
		{0xDD, "CMP",	true,	sAbsoluteX,		tAccumulator,	&Processor::Compare},
		{0xE0, "CPX",	true,	sImmediate,		tIndexX,		&Processor::Compare},
		{0xE4, "CPX",	true,	sZeroPage,		tIndexX,		&Processor::Compare},
		{0xEC, "CPX",	true,	sAbsolute,		tIndexX,		&Processor::Compare},
		{0xC0, "CPY",	true,	sImmediate,		tIndexY,		&Processor::Compare},
		{0xC4, "CPY",	true,	sZeroPage,		tIndexY,		&Processor::Compare},
		{0xCC, "CPY",	true,	sAbsolute,		tIndexY,		&Processor::Compare},
		{0xC6, "DEC",	false,	sZeroPage,		tAddress,		&Processor::Decrement},
		{0xCE, "DEC",	false,	sAbsolute,		tAddress,		&Processor::Decrement},
		{0xD6, "DEC",	false,	sZeroPageX,		tAddress,		&Processor::Decrement},
		{0xDE, "DEC",	false,	sAbsoluteX,		tAddress,		&Processor::Decrement},
		{0xCA, "DEX",	false,	sImplied,		tIndexX,		&Processor::Decrement},
		{0x88, "DEY",	false,	sImplied,		tIndexY,		&Processor::Decrement},
		{0x41, "EOR",	true,	sXIndirect,		tAccumulator,	&Processor::Xor},
		{0x45, "EOR",	true,	sZeroPage,		tAccumulator,	&Processor::Xor},
		{0x49, "EOR",	true,	sImmediate,		tAccumulator,	&Processor::Xor},
		{0x4D, "EOR",	true,	sAbsolute,		tAccumulator,	&Processor::Xor},
		{0x51, "EOR",	true,	sIndirectY,		tAccumulator,	&Processor::Xor},
		{0x55, "EOR",	true,	sZeroPageX,		tAccumulator,	&Processor::Xor},
		{0x59, "EOR",	true,	sAbsoluteY,		tAccumulator,	&Processor::Xor},
		{0x5D, "EOR",	true,	sAbsoluteX,		tAccumulator,	&Processor::Xor},
		{0xE6, "INC",	false,	sZeroPage,		tAddress,		&Processor::Increment},
		{0xEE, "INC",	false,	sAbsolute,		tAddress,		&Processor::Increment},
		{0xF6, "INC",	false,	sZeroPageX,		tAddress,		&Processor::Increment},
		{0xFE, "INC",	false,	sAbsoluteX,		tAddress,		&Processor::Increment},
		{0xE8, "INX",	false,	sImplied,		tIndexX,		&Processor::Increment},
		{0xC8, "INY",	false,	sImplied,		tIndexY,		&Processor::Increment},
		{0x4C, "JMP",	false,	sAbsolute,		tNone,			&Processor::Jump},
		{0x6C, "JMP",	false,	sIndirect,		tNone,			&Processor::Jump},
		{0x20, "JSR",	false,	sAbsolute,		tNone,			&Processor::Call},
		{0xA1, "LDA",	true,	sXIndirect,		tAccumulator,	&Processor::Load},
		{0xA5, "LDA",	true,	sZeroPage,		tAccumulator,	&Processor::Load},
		{0xA9, "LDA",	true,	sImmediate,		tAccumulator,	&Processor::Load},
		{0xAD, "LDA",	true,	sAbsolute,		tAccumulator,	&Processor::Load},
		{0xB1, "LDA",	true,	sIndirectY,		tAccumulator,	&Processor::Load},
		{0xB5, "LDA",	true,	sZeroPageX,		tAccumulator,	&Processor::Load},
		{0xB9, "LDA",	true,	sAbsoluteY,		tAccumulator,	&Processor::Load},
		{0xBD, "LDA",	true,	sAbsoluteX,		tAccumulator,	&Processor::Load},
		{0xA2, "LDX",	true,	sImmediate,		tIndexX,		&Processor::Load},
		{0xA6, "LDX",	true,	sZeroPage,		tIndexX,		&Processor::Load},
		{0xAE, "LDX",	true,	sAbsolute,		tIndexX,		&Processor::Load},
		{0xB6, "LDX",	true,	sZeroPageY,		tIndexX,		&Processor::Load},
		{0xBE, "LDX",	true,	sAbsoluteY,		tIndexX,		&Processor::Load},
		{0xA0, "LDY",	true,	sImmediate,		tIndexY,		&Processor::Load},
		{0xA4, "LDY",	true,	sZeroPage,		tIndexY,		&Processor::Load},
		{0xAC, "LDY",	true,	sAbsolute,		tIndexY,		&Processor::Load},
		{0xB4, "LDY",	true,	sZeroPageX,		tIndexY,		&Processor::Load},
		{0xBC, "LDY",	true,	sAbsoluteX,		tIndexY,		&Processor::Load},
		{0x46, "LSR",	false,	sZeroPage,		tAddress,		&Processor::ShiftRight},
		{0x4A, "LSR A",	false,	sImplied,		tAccumulator,	&Processor::ShiftRight},
		{0x4E, "LSR",	false,	sAbsolute,		tAddress,		&Processor::ShiftRight},
		{0x56, "LSR",	false,	sZeroPageX,		tAddress,		&Processor::ShiftRight},
		{0x5E, "LSR",	false,	sAbsoluteX,		tAddress,		&Processor::ShiftRight},
		{0xEA, "NOP",	false,	sImplied,		tNone,			&Processor::Nop},
		{0x01, "ORA",	true,	sXIndirect,		tAccumulator,	&Processor::Or},
		{0x05, "ORA",	true,	sZeroPage,		tAccumulator,	&Processor::Or},
		{0x09, "ORA",	true,	sImmediate,		tAccumulator,	&Processor::Or},
		{0x0D, "ORA",	true,	sAbsolute,		tAccumulator,	&Processor::Or},
		{0x11, "ORA",	true,	sIndirectY,		tAccumulator,	&Processor::Or},
		{0x15, "ORA",	true,	sZeroPageX,		tAccumulator,	&Processor::Or},
		{0x19, "ORA",	true,	sAbsoluteY,		tAccumulator,	&Processor::Or},
		{0x1D, "ORA",	true,	sAbsoluteX,		tAccumulator,	&Processor::Or},
		{0x48, "PHA",	false,	sImplied,		tAccumulator,	&Processor::Push},
		{0x08, "PHP",	false,	sImplied,		tStatus,		&Processor::Push},
		{0x68, "PLA",	false,	sImplied,		tAccumulator,	&Processor::Pull},
		{0x28, "PLP",	false,	sImplied,		tStatus,		&Processor::Pull},
		{0x26, "ROL",	false,	sZeroPage,		tAddress,		&Processor::RotateLeft},
		{0x2A, "ROL A",	false,	sImplied,		tAccumulator,	&Processor::RotateLeft},
		{0x2E, "ROL",	false,	sAbsolute,		tAddress,		&Processor::RotateLeft},
		{0x36, "ROL",	false,	sZeroPageX,		tAddress,		&Processor::RotateLeft},
		{0x3E, "ROL",	false,	sAbsoluteX,		tAddress,		&Processor::RotateLeft},
		{0x66, "ROR",	false,	sZeroPage,		tAddress,		&Processor::RotateRight},
		{0x6A, "ROR A",	false,	sImplied,		tAccumulator,	&Processor::RotateRight},
		{0x6E, "ROR",	false,	sAbsolute,		tAddress,		&Processor::RotateRight},
		{0x76, "ROR",	false,	sZeroPageX,		tAddress,		&Processor::RotateRight},
		{0x7E, "ROR",	false,	sAbsoluteX,		tAddress,		&Processor::RotateRight},
		{0x40, "RTI",	false,	sImplied,		tNone,			&Processor::ReturnFromInterrupt},
		{0x60, "RTS",	false,	sImplied,		tNone,			&Processor::Return},
		{0xE1, "SBC",	true,	sXIndirect,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xE5, "SBC",	true,	sZeroPage,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xE9, "SBC",	true,	sImmediate,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xED, "SBC",	true,	sAbsolute,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xF1, "SBC",	true,	sIndirectY,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xF5, "SBC",	true,	sZeroPageX,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xF9, "SBC",	true,	sAbsoluteY,		tAccumulator,	&Processor::SubtractWithCarry},
		{0xFD, "SBC",	true,	sAbsoluteX,		tAccumulator,	&Processor::SubtractWithCarry},
		{0x38, "SEC",	false,	sImplied,		tNone,			&Processor::SetCarryFlag},
		{0xF8, "SED",	false,	sImplied,		tNone,			&Processor::SetDecimalFlag},
		{0x78, "SEI",	false,	sImplied,		tNone,			&Processor::SetInterruptFlag},
		{0x81, "STA",	false,	sXIndirect,		tAccumulator,	&Processor::Store},
		{0x85, "STA",	false,	sZeroPage,		tAccumulator,	&Processor::Store},
		{0x8D, "STA",	false,	sAbsolute,		tAccumulator,	&Processor::Store},
		{0x91, "STA",	false,	sIndirectY,		tAccumulator,	&Processor::Store},
		{0x95, "STA",	false,	sZeroPageX,		tAccumulator,	&Processor::Store},
		{0x99, "STA",	false,	sAbsoluteY,		tAccumulator,	&Processor::Store},
		{0x9D, "STA",	false,	sAbsoluteX,		tAccumulator,	&Processor::Store},
		{0x86, "STX",	false,	sZeroPage,		tIndexX,		&Processor::Store},
		{0x8E, "STX",	false,	sAbsolute,		tIndexX,		&Processor::Store},
		{0x96, "STX",	false,	sZeroPageY,		tIndexX,		&Processor::Store},
		{0x84, "STY",	false,	sZeroPage,		tIndexY,		&Processor::Store},
		{0x8C, "STY",	false,	sAbsolute,		tIndexY,		&Processor::Store},
		{0x94, "STY",	false,	sZeroPageX,		tIndexY,		&Processor::Store},
		{0xAA, "TAX",	false,	sAccumulator,	tIndexX,		&Processor::Load},
		{0xA8, "TAY",	false,	sAccumulator,	tIndexY,		&Processor::Load},
		{0xBA, "TSX",	false,	sStackPointer,	tIndexX,		&Processor::Load},
		{0x8A, "TXA",	false,	sIndexX,		tAccumulator,	&Processor::Load},
		{0x9A, "TXS",	false,	sIndexX,		tStackPointer,	&Processor::Load},
		{0x98, "TYA",	false,	sIndexY,		tAccumulator,	&Processor::Load}
	};

	const Instruction	*InstructionSet[256] = {};
	const Instruction	*LastInstruction;

	Memory	&RAM;			// 64kb of RAM (hopefully)
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
	byte ReadByte(word Address);
	byte ReadOpCode();
	word ReadWord(word Address);
	void Push(byte Data);
	byte PullByte();
	bool ReadFlag(Flags Flag);
	void WriteFlag(Flags Flag, bool Value);
	void WriteTargetFlags();
	void Tick(byte Cycles = 1);
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

	const Instruction *ReadInstruction();
	void DecodeInstruction(const Instruction * Ins);
	void ExecuteInstruction(const Instruction * Ins);
	void Disassemble(char Output[20], const Instruction * Ins);
			
public:
	int		Clock;	// internal clock
	byte	A;		// accumulator
	byte	X, Y;	// index registers
	word	PC;		// program counter
	byte	S;		// stack pointer
	byte	P;		// status flags
	bool	EndOnBreak;	// if true, Run() will stop on BRK

	Processor(Memory *RAM);
	bool FlagCarry();
	bool FlagZero();
	bool FlagInterrupt();
	bool FlagDecimal();
	bool FlagOverflow();
	bool FlagNegative();
	void SendRST();
	void SendIRQ();
	void SendNMI();
	void Step();			// execute instruction at PC, deal with IRQ and RST if necessary
	void Step(int Count);	// execute Count instructions
	void Run();				// execute instructions until BRK is met (if EndOnBreak == true) or forever
	// used in the tests to verify that the last opcode matches the instruction being tested
	bool IsLastInstruction(const char *Name);
	bool IsLastInstruction(const char *Name, Sources Source);
	bool IsLastInstruction(const char *Name, Sources Source, Targets Target);
};
