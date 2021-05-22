#pragma once

using byte = unsigned char;
using word = unsigned short;

class Processor
{
protected:
	const word NonMaskableInterruptVector	= 0xFFFA;
	const word ResetVector					= 0xFFFC;
	const word InterruptVector				= 0xFFFE;

	enum Flags : byte {
		fCarry		=   1,
		fZero		=   2,
		fInterrupt	=   4,
		fDecimal	=   8,
		fBreak		=  16,	// always set to 1
		// status bit 5 is always set to 1
		fOverflow	=  64,
		fNegative	= 128
	};

	// equivalent to addressing modes plus extra for transfer instructions
	enum SourceType {
		sAccumulator,
		sIndexX,
		sIndexY,
		sStackPointer,
		sAbsolute,
		sAbsoluteX,
		sAbsoluteY,
		sImmediate,
		sImplied,
		sIndirect,
		sXIndirect,
		sIndirectY,
		sZeroPage,
		sZeroPageX,
		sZeroPageY
	};

	enum TargetType {
		tNone,
		tAccumulator,
		tIndexX,
		tIndexY,
		tStackPointer,
		tStatus,
		tAddress
	};

	struct Instruction {
		byte			OpCode;
		const char		*Name;
		SourceType		Source;
		TargetType		Target;
		void			(Processor::*Function)();
	};

	const Instruction	LegalInstructionSet[151] = {
		{0x61, "ADC", sXIndirect, tAccumulator, &Processor::AddWithCarry},
		{0x65, "ADC", sZeroPage, tAccumulator, &Processor::AddWithCarry},
		{0x69, "ADC", sImmediate, tAccumulator, &Processor::AddWithCarry},
		{0x6D, "ADC", sAbsolute, tAccumulator, &Processor::AddWithCarry},
		{0x71, "ADC", sIndirectY, tAccumulator, &Processor::AddWithCarry},
		{0x75, "ADC", sZeroPageX, tAccumulator, &Processor::AddWithCarry},
		{0x79, "ADC", sAbsoluteY, tAccumulator, &Processor::AddWithCarry},
		{0x7D, "ADC", sAbsoluteX, tAccumulator, &Processor::AddWithCarry},
		{0x21, "AND", sXIndirect, tAccumulator, &Processor::And},
		{0x25, "AND", sZeroPage, tAccumulator, &Processor::And},
		{0x29, "AND", sImmediate, tAccumulator, &Processor::And},
		{0x2D, "AND", sAbsolute, tAccumulator, &Processor::And},
		{0x31, "AND", sIndirectY, tAccumulator, &Processor::And},
		{0x35, "AND", sZeroPageX, tAccumulator, &Processor::And},
		{0x39, "AND", sAbsoluteY, tAccumulator, &Processor::And},
		{0x3D, "AND", sAbsoluteX, tAccumulator, &Processor::And},
		{0x06, "ASL", sZeroPage, tAddress, &Processor::ShiftLeft},
		{0x0A, "ASL A", sImplied, tAccumulator, &Processor::ShiftLeft},
		{0x0E, "ASL", sAbsolute, tAddress, &Processor::ShiftLeft},
		{0x16, "ASL", sZeroPageX, tAddress, &Processor::ShiftLeft},
		{0x1E, "ASL", sAbsoluteX, tAddress, &Processor::ShiftLeft},
		{0x90, "BCC", sImmediate, tNone, &Processor::BranchIfCarryClear},
		{0xB0, "BCS", sImmediate, tNone, &Processor::BranchIfCarrySet},
		{0xF0, "BEQ", sImmediate, tNone, &Processor::BranchIfEqual},
		{0x24, "BIT", sZeroPage, tNone, &Processor::BitTest},
		{0x2C, "BIT", sAbsolute, tAddress, &Processor::BitTest},
		{0x30, "BMI", sImmediate, tNone, &Processor::BranchIfMinus},
		{0xD0, "BNE", sImmediate, tNone, &Processor::BranchIfNotEqual},
		{0x10, "BPL", sImmediate, tNone, &Processor::BranchIfPositive},
		{0x00, "BRK", sImplied, tNone, &Processor::Break},
		{0x50, "BVC", sImmediate, tNone, &Processor::BranchIfOverflowClear},
		{0x70, "BVS", sImmediate, tNone, &Processor::BranchIfOverflowSet},
		{0x18, "CLC", sImplied, tNone, &Processor::ClearCarryFlag},
		{0xD8, "CLD", sImplied, tNone, &Processor::ClearDecimalFlag},
		{0x58, "CLI", sImplied, tNone, &Processor::ClearInterruptFlag},
		{0xB8, "CLV", sImplied, tNone, &Processor::ClearOverflowFlag},
		{0xC1, "CMP", sXIndirect, tAccumulator, &Processor::Compare},
		{0xC5, "CMP", sZeroPage, tAccumulator, &Processor::Compare},
		{0xC9, "CMP", sImmediate, tAccumulator, &Processor::Compare},
		{0xCD, "CMP", sAbsolute, tAccumulator, &Processor::Compare},
		{0xD1, "CMP", sIndirectY, tAccumulator, &Processor::Compare},
		{0xD5, "CMP", sZeroPageX, tAccumulator, &Processor::Compare},
		{0xD9, "CMP", sAbsoluteY, tAccumulator, &Processor::Compare},
		{0xDD, "CMP", sAbsoluteX, tAccumulator, &Processor::Compare},
		{0xE0, "CPX", sImmediate, tIndexX, &Processor::Compare},
		{0xE4, "CPX", sZeroPage, tIndexX, &Processor::Compare},
		{0xEC, "CPX", sAbsolute, tIndexX, &Processor::Compare},
		{0xC0, "CPY", sImmediate, tIndexY, &Processor::Compare},
		{0xC4, "CPY", sZeroPage, tIndexY, &Processor::Compare},
		{0xCC, "CPY", sAbsolute, tIndexY, &Processor::Compare},
		{0xC6, "DEC", sZeroPage, tNone, &Processor::Decrement},
		{0xCE, "DEC", sAbsolute, tNone, &Processor::Decrement},
		{0xD6, "DEC", sZeroPageX, tNone, &Processor::Decrement},
		{0xDE, "DEC", sAbsoluteX, tNone, &Processor::Decrement},
		{0xCA, "DEX", sImplied, tIndexX, &Processor::Decrement},
		{0x88, "DEY", sImplied, tIndexY, &Processor::Decrement},
		{0x41, "EOR", sXIndirect, tAccumulator, &Processor::Xor},
		{0x45, "EOR", sZeroPage, tAccumulator, &Processor::Xor},
		{0x49, "EOR", sImmediate, tAccumulator, &Processor::Xor},
		{0x4D, "EOR", sAbsolute, tAccumulator, &Processor::Xor},
		{0x51, "EOR", sIndirectY, tAccumulator, &Processor::Xor},
		{0x55, "EOR", sZeroPageX, tAccumulator, &Processor::Xor},
		{0x59, "EOR", sAbsoluteY, tAccumulator, &Processor::Xor},
		{0x5D, "EOR", sAbsoluteX, tAccumulator, &Processor::Xor},
		{0xE6, "INC", sZeroPage, tAddress, &Processor::Increment},
		{0xEE, "INC", sAbsolute, tAddress, &Processor::Increment},
		{0xF6, "INC", sZeroPageX, tAddress, &Processor::Increment},
		{0xFE, "INC", sAbsoluteX, tAddress, &Processor::Increment},
		{0xE8, "INX", sImplied, tIndexX, &Processor::Increment},
		{0xC8, "INY", sImplied, tIndexY, &Processor::Increment},
		{0x4C, "JMP", sAbsolute, tNone, &Processor::Jump},
		{0x6C, "JMP", sIndirect, tNone, &Processor::Jump},
		{0x20, "JSR", sAbsolute, tNone, &Processor::Call},
		{0xA1, "LDA", sXIndirect, tAccumulator,	&Processor::Load},
		{0xA5, "LDA", sZeroPage, tAccumulator, &Processor::Load},
		{0xA9, "LDA", sImmediate, tAccumulator, &Processor::Load},
		{0xAD, "LDA", sAbsolute, tAccumulator, &Processor::Load},
		{0xB1, "LDA", sIndirectY, tAccumulator, &Processor::Load},
		{0xB5, "LDA", sZeroPageX, tAccumulator, &Processor::Load},
		{0xB9, "LDA", sAbsoluteY, tAccumulator, &Processor::Load},
		{0xBD, "LDA", sAbsoluteX, tAccumulator, &Processor::Load},
		{0xA2, "LDX", sImmediate, tIndexX, &Processor::Load},
		{0xA6, "LDX", sZeroPage, tIndexX, &Processor::Load},
		{0xAE, "LDX", sAbsolute, tIndexX, &Processor::Load},
		{0xB6, "LDX", sZeroPageY, tIndexX, &Processor::Load},
		{0xBE, "LDX", sAbsoluteY, tIndexX, &Processor::Load},
		{0xA0, "LDY", sImmediate, tIndexY, &Processor::Load},
		{0xA4, "LDY", sZeroPage, tIndexY, &Processor::Load},
		{0xAC, "LDY", sAbsolute, tIndexY, &Processor::Load},
		{0xB4, "LDY", sZeroPageX, tIndexY, &Processor::Load},
		{0xBC, "LDY", sAbsoluteX, tIndexY, &Processor::Load},
		{0x46, "LSR", sZeroPage, tAddress, &Processor::ShiftRight},
		{0x4A, "LSR A", sImplied, tAccumulator, &Processor::ShiftRight},
		{0x4E, "LSR", sAbsolute, tAddress, &Processor::ShiftRight},
		{0x56, "LSR", sZeroPageX, tAddress, &Processor::ShiftRight},
		{0x5E, "LSR", sAbsoluteX, tAddress, &Processor::ShiftRight},
		{0xEA, "NOP", sImplied, tNone, &Processor::Nop},
		{0x01, "ORA", sXIndirect, tAccumulator, &Processor::Or},
		{0x05, "ORA", sZeroPage, tAccumulator, &Processor::Or},
		{0x09, "ORA", sImmediate, tAccumulator, &Processor::Or},
		{0x0D, "ORA", sAbsolute, tAccumulator, &Processor::Or},
		{0x11, "ORA", sIndirectY, tAccumulator, &Processor::Or},
		{0x15, "ORA", sZeroPageX, tAccumulator, &Processor::Or},
		{0x19, "ORA", sAbsoluteY, tAccumulator, &Processor::Or},
		{0x1D, "ORA", sAbsoluteX, tAccumulator, &Processor::Or},
		{0x48, "PHA", sImplied, tAccumulator, &Processor::Push},
		{0x08, "PHP", sImplied, tStatus, &Processor::Push},
		{0x68, "PLA", sImplied, tAccumulator, &Processor::Pull},
		{0x28, "PLP", sImplied, tStatus, &Processor::Pull},
		{0x26, "ROL", sZeroPage, tAddress, &Processor::RotateLeft},
		{0x2A, "ROL A", sImplied, tAccumulator, &Processor::RotateLeft},
		{0x2E, "ROL", sAbsolute, tAddress, &Processor::RotateLeft},
		{0x36, "ROL", sZeroPageX, tAddress, &Processor::RotateLeft},
		{0x3E, "ROL", sAbsoluteX, tAddress, &Processor::RotateLeft},
		{0x66, "ROR", sZeroPage, tAddress, &Processor::RotateRight},
		{0x6A, "ROR A", sImplied, tAccumulator, &Processor::RotateRight},
		{0x6E, "ROR", sAbsolute, tAddress, &Processor::RotateRight},
		{0x76, "ROR", sZeroPageX, tAddress, &Processor::RotateRight},
		{0x7E, "ROR", sAbsoluteX, tAddress, &Processor::RotateRight},
		{0x40, "RTI", sImplied, tNone, &Processor::ReturnFromInterrupt},
		{0x60, "RTS", sImplied, tNone, &Processor::Return},
		{0xE1, "SBC", sXIndirect, tAccumulator, &Processor::SubtractWithCarry},
		{0xE5, "SBC", sZeroPage, tAccumulator, &Processor::SubtractWithCarry},
		{0xE9, "SBC", sImmediate, tAccumulator, &Processor::SubtractWithCarry},
		{0xED, "SBC", sAbsolute, tAccumulator, &Processor::SubtractWithCarry},
		{0xF1, "SBC", sIndirectY, tAccumulator, &Processor::SubtractWithCarry},
		{0xF5, "SBC", sZeroPageX, tAccumulator, &Processor::SubtractWithCarry},
		{0xF9, "SBC", sAbsoluteY, tAccumulator, &Processor::SubtractWithCarry},
		{0xFD, "SBC", sAbsoluteX, tAccumulator, &Processor::SubtractWithCarry},
		{0x38, "SEC", sImplied, tNone, &Processor::SetCarryFlag},
		{0xF8, "SED", sImplied, tNone, &Processor::SetDecimalFlag},
		{0x78, "SEI", sImplied, tNone, &Processor::SetInterruptFlag},
		{0x81, "STA", sXIndirect, tAccumulator, &Processor::Store},
		{0x85, "STA", sZeroPage, tAccumulator, &Processor::Store},
		{0x8D, "STA", sAbsolute, tAccumulator, &Processor::Store},
		{0x91, "STA", sIndirectY, tAccumulator, &Processor::Store},
		{0x95, "STA", sZeroPageX, tAccumulator, &Processor::Store},
		{0x99, "STA", sAbsoluteY, tAccumulator, &Processor::Store},
		{0x9D, "STA", sAbsoluteX, tAccumulator, &Processor::Store},
		{0x86, "STX", sZeroPage, tIndexX, &Processor::Store},
		{0x8E, "STX", sAbsolute, tIndexX, &Processor::Store},
		{0x96, "STX", sZeroPageY, tIndexX, &Processor::Store},
		{0x84, "STY", sZeroPage, tIndexY, &Processor::Store},
		{0x8C, "STY", sAbsolute, tIndexY, &Processor::Store},
		{0x94, "STY", sZeroPageX, tIndexY, &Processor::Store},
		{0xAA, "TAX", sAccumulator, tIndexX, &Processor::Load},
		{0xA8, "TAY", sAccumulator, tIndexY, &Processor::Load},
		{0xBA, "TSX", sStackPointer, tIndexX, &Processor::Load},
		{0x8A, "TXA", sIndexX, tAccumulator, &Processor::Load},
		{0x9A, "TXS", sIndexX, tStackPointer, &Processor::Load},
		{0x98, "TYA", sIndexY, tAccumulator, &Processor::Load}
	};

	const Instruction*	InstructionSet[256] = {};

	byte	*Memory;		// 64kb of RAM (hopefully)
	byte	*Source;		// instruction source
	byte	*Target;		// instruction target
	byte	Data;			// data register
	byte	OpCode;			// instruction register
	word	Address;		// address register

	bool ResetState;
	bool InterruptState;
	bool NonMaskableInterruptState;
	
#pragma region internal functions
	bool SignBit(byte Value)
	{
		return Value & 0x80;
	}

	word Add(word A, byte B)
	{
		return ((int)A + B) & 0xFFFF;
	}

	byte Add(byte A, byte B)
	{
		return ((word)A + B) & 0xFF;
	}

	byte ReadData(word Address)
	{
		Data = Memory[Address];

		return Data;
	}

	void WriteData(word Address) 
	{
		Memory[Address] = Data;
	}

	byte ReadOpCode()
	{
		OpCode = Memory[PC++];

		return OpCode;
	}

	void ReadDataAtPC()
	{
		ReadData(PC++);
	}

	word ReadAddress(word Address)
	{
		this->Address = Memory[Address] | (Memory[Add(Address, 1)] << 8);

		return this->Address;
	}

	void WriteAddress(word Address)
	{
		Memory[Address] = this->Address & 0xFF;
		Memory[Add(Address, 1)] = this->Address >> 8;
	}

	void ReadAddressAtPC()
	{
		ReadAddress(PC);
		PC += 2;
	}

	bool ReadFlag(Flags Flag)
	{
		return P & Flag;
	}

	void WriteFlag(Flags Flag, bool Value)
	{
		if(Value)
			P |= Flag;
		else
			P &= ~Flag;
	}

	void WriteTargetFlags()
	{
		WriteFlag(fZero, (*Target == 0));
		WriteFlag(fNegative, SignBit(*Target));
	}

#pragma endregion

#pragma region instructions
	void Load()
	{
		*Target = *Source;		
		WriteTargetFlags();
	}

	void Store()
	{
		*Source = *Target;
	}

	void Compare()
	{
		WriteFlag(fCarry, (*Target >= *Source));
		WriteFlag(fZero, (*Target == *Source));
		WriteFlag(fNegative, SignBit(*Target));
	}

	void And()
	{
		*Target &= *Source;
		WriteTargetFlags();
	}

	void Xor()
	{
		*Target ^= *Source;
		WriteTargetFlags();
	}

	void Or()
	{
		*Target |= *Source;
		WriteTargetFlags();
	}

	void RotateLeft()
	{
		byte c = ReadFlag(fCarry);
		WriteFlag(fCarry, SignBit(*Target));
		*Target = ((*Target) << 1) | c;
		WriteTargetFlags();
	}

	void RotateRight()
	{
		byte c = ReadFlag(fCarry);
		WriteFlag(fCarry, (*Target) & 1);
		*Target = ((*Target) >> 1) | (c << 7);
		WriteTargetFlags();
	}

	void ShiftLeft()
	{
		WriteFlag(fCarry, SignBit(*Target));
		*Target <<= 1;
		WriteTargetFlags();
	}

	void ShiftRight()
	{
		WriteFlag(fCarry, (*Target) & 1);
		*Target >>= 1;
		WriteTargetFlags();
	}

	void Increment()
	{
		(*Target)++;
		WriteTargetFlags();
	}

	void Decrement()
	{
		(*Target)--;
		WriteTargetFlags();
	}

	void AddWithCarry()
	{
		// TODO : implement BCD addition
		word result = *Target + *Source + ReadFlag(fCarry);

		// if both operands sign is identical but differs from the result sign (e.g. 100 + 49 = -107)
		WriteFlag(fOverflow, (*Source ^ result) & (*Target ^ result) & 0x80);
		*Target = result & 0xFF;
		WriteFlag(fCarry, result & 0x100);
		WriteTargetFlags();
	}

	void SubtractWithCarry()
	{
		// TODO : implement BCD subtraction
		word result = *Target + ~*Source + ReadFlag(fCarry);

		WriteFlag(fOverflow, (~*Source ^ result) & (*Target ^ result) & 0x80);
		*Target = result & 0xFF;
		WriteFlag(fCarry, result & 0x100);
		WriteTargetFlags();
	}

	void Push()
	{
		Memory[Add((word)0x100, S--)] = *Target;
	}

	void PushAddress(word Address)
	{
		Memory[S--] = Address >> 8;
		Memory[S--] = Address & 0xFF;
	}

	void PullAddress(word &Address)
	{
		Address = Memory[++S] | (Memory[++S] << 8);
	}

	void Pull()
	{
		*Target = Memory[Add((word)0x100, ++S)];
	}

	void Branch()
	{
		PC += (char)*Source;
	}

	void Jump()
	{
		// this avoids PC = &Memory[Address] in absolute and indirect modes
		PC = Address;
	}

	void Call()
	{
		PushAddress(PC);
		Jump();
	}

	void Return()
	{
		PullAddress(PC);
	}

	void Break()
	{
		if (!EndOnBreak)
		{
			PushAddress(PC);
			Memory[S--] = P | fBreak;
			PC = ReadAddress(InterruptVector);
		}
	}

	void Nop()
	{
		// nope nope nope nope...
	}

	void BranchIfMinus()
	{
		if (ReadFlag(fNegative))
			Branch();
	}

	void BranchIfPositive()
	{
		if (!ReadFlag(fNegative))
			Branch();
	}

	void BranchIfEqual() 
	{
		if (ReadFlag(fZero))
			Branch();
	}

	void BranchIfNotEqual() 
	{
		if (!ReadFlag(fZero))
			Branch();
	}

	void BranchIfCarrySet() 
	{
		if (ReadFlag(fCarry))
			Branch();
	}

	void BranchIfCarryClear()
	{
		if (!ReadFlag(fCarry))
			Branch();
	}

	void BranchIfOverflowSet() 
	{
		if (ReadFlag(fOverflow))
			Branch();
	}

	void BranchIfOverflowClear()
	{
		if (!ReadFlag(fOverflow))
			Branch();
	}

	void ClearCarryFlag()
	{
		WriteFlag(fCarry, false);
	}
	
	void ClearDecimalFlag()
	{
		WriteFlag(fDecimal, false);
	}

	void ClearInterruptFlag() 
	{
		WriteFlag(fInterrupt, false);
	}

	void ClearOverflowFlag()
	{
		WriteFlag(fOverflow, false);
	}

	void SetCarryFlag()
	{
		WriteFlag(fCarry, true);
	}

	void SetDecimalFlag()
	{
		WriteFlag(fDecimal, true);
	}

	void SetInterruptFlag()
	{
		WriteFlag(fInterrupt, true);
	}

	void BitTest()
	{
		WriteFlag(fZero, (A & *Target) == 0);
		WriteFlag(fOverflow, *Target & 0x40);
		WriteFlag(fNegative, *Target & 0x80);
	}

	void Reset()
	{
		S = 0xFD;
		P = 0b00110000;
		PC = ReadAddress(ResetVector);
		ResetState = false;
		InterruptState = false;
		NonMaskableInterruptState = false;
	}

	void Interrupt()
	{
		InterruptState = false;
		PushAddress(PC);
		Memory[S--] = P & ~fBreak;
		WriteFlag(fInterrupt, true);
		PC = ReadAddress(InterruptVector);
	}

	void NonMaskableInterrupt()
	{
		NonMaskableInterruptState = false;
		PushAddress(PC);
		Memory[S--] = P;
		WriteFlag(fInterrupt, true);
		PC = ReadAddress(NonMaskableInterruptVector);
	}

	void ReturnFromInterrupt()
	{
		P = Memory[++S];
		Return();
	}

#pragma endregion

	void ExecuteInstruction()
	{
		ReadOpCode();
		// TODO : handle exception when OpCode is undefined (InstructionSet[OpCode] == nullptr)
		const Instruction* in = InstructionSet[OpCode];

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
			
public:
	byte	A;		// accumulator
	byte	X, Y;	// index registers
	word	PC;		// program counter
	byte	S;		// stack pointer
	byte	P;		// status flags
	bool	EndOnBreak;

	Processor(byte *Array)
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

	bool FlagCarry()
	{
		return ReadFlag(fCarry);
	}

	bool FlagZero()
	{
		return ReadFlag(fZero);
	}

	bool FlagInterrupt()
	{
		return ReadFlag(fInterrupt);
	}

	bool FlagDecimal()
	{
		return ReadFlag(fDecimal);
	}

	bool FlagOverflow()
	{
		return ReadFlag(fOverflow);
	}

	bool FlagNegative()
	{
		return ReadFlag(fNegative);
	}

	void SendRST()
	{
		ResetState = true;
	}

	void SendIRQ()
	{
		if(!ReadFlag(fInterrupt))
			InterruptState = true;
	}

	void SendNMI()
	{
		NonMaskableInterruptState = true;
	}

	void Step()
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

	void Step(int Count)
	{
		for (int i = 0; i < Count; i++)
			Step();
	}

	void Run()
	{
		do
		{
			Step();
		} while ((OpCode != 0x00) || !EndOnBreak);
	}
};
