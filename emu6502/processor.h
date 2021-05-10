#pragma once

using byte = unsigned char;
using word = unsigned short;

class Processor
{
protected:
	typedef enum enum_Flags : byte {
		Carry = 1,
		Zero = 2,
		Interrupt = 4,
		Decimal = 8,
		BreakCommand = 16,
		// status bit 5 is always set to 1
		Overflow = 64,
		Negative = 128
	} Flags;

	// equivalent to addressing modes plus extra for transfer instructions
	enum class SourceType {
		Accumulator = 0,
		IndexX,
		IndexY,
		StackPointer,
		Absolute,
		AbsoluteX,
		AbsoluteY,
		Immediate,
		Implied,
		Indirect,
		XIndirect,
		IndirectY,
		ZeroPage,
		ZeroPageX,
		ZeroPageY
	};

	enum class TargetType {
		None = 0,
		Accumulator,
		IndexX,
		IndexY,
		StackPointer,
		Status,
		Address
	};

	typedef struct struct_Instruction {
		byte			OpCode;
		const char		*Name;
		SourceType		Source;
		TargetType		Target;
		void			(Processor::*Function)();
	} Instruction;

	Instruction LegalInstructionSet[151] = {
		{0x61, "ADC", SourceType::XIndirect, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x65, "ADC", SourceType::ZeroPage, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x69, "ADC", SourceType::Immediate, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x6D, "ADC", SourceType::Absolute, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x71, "ADC", SourceType::IndirectY, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x75, "ADC", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x79, "ADC", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x7D, "ADC", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::AddWithCarry},
		{0x21, "AND", SourceType::XIndirect, TargetType::Accumulator, &Processor::And},
		{0x25, "AND", SourceType::ZeroPage, TargetType::Accumulator, &Processor::And},
		{0x29, "AND", SourceType::Immediate, TargetType::Accumulator, &Processor::And},
		{0x2D, "AND", SourceType::Absolute, TargetType::Accumulator, &Processor::And},
		{0x31, "AND", SourceType::IndirectY, TargetType::Accumulator, &Processor::And},
		{0x34, "AND", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::And},
		{0x38, "AND", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::And},
		{0x3D, "AND", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::And},
		{0x06, "ASL", SourceType::ZeroPage, TargetType::Address, &Processor::ShiftLeft},
		{0x0A, "ASL", SourceType::Implied, TargetType::Accumulator, &Processor::ShiftLeft},
		{0x0E, "ASL", SourceType::Absolute, TargetType::Address, &Processor::ShiftLeft},
		{0x16, "ASL", SourceType::ZeroPageX, TargetType::Address, &Processor::ShiftLeft},
		{0x1E, "ASL", SourceType::AbsoluteX, TargetType::Address, &Processor::ShiftLeft},
		{0x90, "BCC", SourceType::Immediate, TargetType::None, &Processor::BranchIfCarryClear},
		{0xB0, "BCS", SourceType::Immediate, TargetType::None, &Processor::BranchIfCarrySet},
		{0xF0, "BEQ", SourceType::Immediate, TargetType::None, &Processor::BranchIfEqual},
		{0x24, "BIT", SourceType::ZeroPage, TargetType::None, &Processor::BitTest},
		{0x2C, "BIT", SourceType::Absolute, TargetType::Address, &Processor::BitTest},
		{0x30, "BMI", SourceType::Immediate, TargetType::None, &Processor::BranchIfMinus},
		{0xD0, "BNE", SourceType::Immediate, TargetType::None, &Processor::BranchIfNotEqual},
		{0x10, "BPL", SourceType::Immediate, TargetType::None, &Processor::BranchIfPositive},
		{0x00, "BRK", SourceType::Implied, TargetType::None, &Processor::Break},
		{0x50, "BVC", SourceType::Immediate, TargetType::None, &Processor::BranchIfOverflowClear},
		{0x70, "BVS", SourceType::Immediate, TargetType::None, &Processor::BranchIfOverflowSet},
		{0x18, "CLC", SourceType::Implied, TargetType::None, &Processor::ClearCarryFlag},
		{0xD8, "CLD", SourceType::Implied, TargetType::None, &Processor::ClearDecimalFlag},
		{0x58, "CLI", SourceType::Implied, TargetType::None, &Processor::ClearInterruptFlag},
		{0xB8, "CLV", SourceType::Implied, TargetType::None, &Processor::ClearOverflowFlag},
		{0xC1, "CMP", SourceType::XIndirect, TargetType::Accumulator, &Processor::Compare},
		{0xC5, "CMP", SourceType::ZeroPage, TargetType::Accumulator, &Processor::Compare},
		{0xC9, "CMP", SourceType::Immediate, TargetType::Accumulator, &Processor::Compare},
		{0xCD, "CMP", SourceType::Absolute, TargetType::Accumulator, &Processor::Compare},
		{0xD1, "CMP", SourceType::IndirectY, TargetType::Accumulator, &Processor::Compare},
		{0xD5, "CMP", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::Compare},
		{0xD9, "CMP", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::Compare},
		{0xDD, "CMP", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::Compare},
		{0xE0, "CPX", SourceType::Immediate, TargetType::IndexX, &Processor::Compare},
		{0xE4, "CPX", SourceType::ZeroPage, TargetType::IndexX, &Processor::Compare},
		{0xEC, "CPX", SourceType::Absolute, TargetType::IndexX, &Processor::Compare},
		{0xC0, "CPY", SourceType::Immediate, TargetType::IndexY, &Processor::Compare},
		{0xC4, "CPY", SourceType::ZeroPage, TargetType::IndexY, &Processor::Compare},
		{0xCC, "CPY", SourceType::Absolute, TargetType::IndexY, &Processor::Compare},
		{0xC6, "DEC", SourceType::ZeroPage, TargetType::None, &Processor::Decrement},
		{0xCE, "DEC", SourceType::Absolute, TargetType::None, &Processor::Decrement},
		{0xD6, "DEC", SourceType::ZeroPageX, TargetType::None, &Processor::Decrement},
		{0xDE, "DEC", SourceType::AbsoluteX, TargetType::None, &Processor::Decrement},
		{0xCA, "DEX", SourceType::Implied, TargetType::IndexX, &Processor::Decrement},
		{0x88, "DEY", SourceType::Implied, TargetType::IndexY, &Processor::Decrement},
		{0x41, "EOR", SourceType::XIndirect, TargetType::Accumulator, &Processor::Xor},
		{0x45, "EOR", SourceType::ZeroPage, TargetType::Accumulator, &Processor::Xor},
		{0x49, "EOR", SourceType::Immediate, TargetType::Accumulator, &Processor::Xor},
		{0x4D, "EOR", SourceType::Absolute, TargetType::Accumulator, &Processor::Xor},
		{0x51, "EOR", SourceType::IndirectY, TargetType::Accumulator, &Processor::Xor},
		{0x55, "EOR", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::Xor},
		{0x59, "EOR", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::Xor},
		{0x5D, "EOR", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::Xor},
		{0xE6, "INC", SourceType::ZeroPage, TargetType::Address, &Processor::Increment},
		{0xEE, "INC", SourceType::Absolute, TargetType::Address, &Processor::Increment},
		{0xF6, "INC", SourceType::ZeroPageX, TargetType::Address, &Processor::Increment},
		{0xFE, "INC", SourceType::AbsoluteX, TargetType::Address, &Processor::Increment},
		{0xE8, "INX", SourceType::Implied, TargetType::IndexX, &Processor::Increment},
		{0xC8, "INY", SourceType::Implied, TargetType::IndexY, &Processor::Increment},
		{0x4C, "JMP", SourceType::Absolute, TargetType::None, &Processor::Jump},
		{0x6C, "JMP", SourceType::Indirect, TargetType::None, &Processor::Jump},
		{0x20, "JSR", SourceType::Absolute, TargetType::None, &Processor::Call},
		{0xA1, "LDA", SourceType::XIndirect, TargetType::Accumulator,	&Processor::Load},
		{0xA5, "LDA", SourceType::ZeroPage, TargetType::Accumulator, &Processor::Load},
		{0xA9, "LDA", SourceType::Immediate, TargetType::Accumulator, &Processor::Load},
		{0xAD, "LDA", SourceType::Absolute, TargetType::Accumulator, &Processor::Load},
		{0xB1, "LDA", SourceType::IndirectY, TargetType::Accumulator, &Processor::Load},
		{0xB5, "LDA", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::Load},
		{0xB9, "LDA", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::Load},
		{0xBD, "LDA", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::Load},
		{0xA2, "LDX", SourceType::Immediate, TargetType::IndexX, &Processor::Load},
		{0xA6, "LDX", SourceType::ZeroPage, TargetType::IndexX, &Processor::Load},
		{0xAE, "LDX", SourceType::Absolute, TargetType::IndexX, &Processor::Load},
		{0xB6, "LDX", SourceType::ZeroPageY, TargetType::IndexX, &Processor::Load},
		{0xBE, "LDX", SourceType::AbsoluteY, TargetType::IndexX, &Processor::Load},
		{0xA0, "LDY", SourceType::Immediate, TargetType::IndexY, &Processor::Load},
		{0xA4, "LDY", SourceType::ZeroPage, TargetType::IndexY, &Processor::Load},
		{0xAC, "LDY", SourceType::Absolute, TargetType::IndexY, &Processor::Load},
		{0xB4, "LDY", SourceType::ZeroPageX, TargetType::IndexY, &Processor::Load},
		{0xBC, "LDY", SourceType::AbsoluteX, TargetType::IndexY, &Processor::Load},
		{0x46, "LSR", SourceType::ZeroPage, TargetType::Address, &Processor::ShiftRight},
		{0x4A, "LSR", SourceType::Implied, TargetType::Accumulator, &Processor::ShiftRight},
		{0x4E, "LSR", SourceType::Absolute, TargetType::Address, &Processor::ShiftRight},
		{0x56, "LSR", SourceType::ZeroPageX, TargetType::Address, &Processor::ShiftRight},
		{0x5E, "LSR", SourceType::AbsoluteX, TargetType::Address, &Processor::ShiftRight},
		{0xEA, "NOP", SourceType::Implied, TargetType::None, &Processor::Nop},
		{0x01, "ORA", SourceType::XIndirect, TargetType::Accumulator, &Processor::Or},
		{0x05, "ORA", SourceType::ZeroPage, TargetType::Accumulator, &Processor::Or},
		{0x09, "ORA", SourceType::Immediate, TargetType::Accumulator, &Processor::Or},
		{0x0D, "ORA", SourceType::Absolute, TargetType::Accumulator, &Processor::Or},
		{0x11, "ORA", SourceType::IndirectY, TargetType::Accumulator, &Processor::Or},
		{0x15, "ORA", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::Or},
		{0x19, "ORA", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::Or},
		{0x1D, "ORA", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::Or},
		{0x48, "PHA", SourceType::Implied, TargetType::Accumulator, &Processor::Push},
		{0x08, "PHP", SourceType::Implied, TargetType::Status, &Processor::Push},
		{0x68, "PLA", SourceType::Implied, TargetType::Accumulator, &Processor::Pull},
		{0x28, "PLP", SourceType::Implied, TargetType::Status, &Processor::Pull},
		{0x26, "ROL", SourceType::ZeroPage, TargetType::Address, &Processor::RotateLeft},
		{0x2A, "ROL", SourceType::Implied, TargetType::Accumulator, &Processor::RotateLeft},
		{0x2E, "ROL", SourceType::Absolute, TargetType::Address, &Processor::RotateLeft},
		{0x35, "ROL", SourceType::ZeroPageX, TargetType::None, &Processor::RotateLeft},
		{0x3E, "ROL", SourceType::AbsoluteX, TargetType::Address, &Processor::RotateLeft},
		{0x66, "ROR", SourceType::ZeroPage, TargetType::Address, &Processor::RotateRight},
		{0x6A, "ROR", SourceType::Implied, TargetType::Accumulator, &Processor::RotateRight},
		{0x6E, "ROR", SourceType::Absolute, TargetType::Address, &Processor::RotateRight},
		{0x76, "ROR", SourceType::ZeroPageX, TargetType::Address, &Processor::RotateRight},
		{0x7E, "ROR", SourceType::AbsoluteX, TargetType::Address, &Processor::RotateRight},
		{0x40, "RTI", SourceType::Implied, TargetType::None, &Processor::ReturnFromInterrupt},
		{0x60, "RTS", SourceType::Implied, TargetType::None, &Processor::Return},
		{0xE1, "SBC", SourceType::XIndirect, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xE5, "SBC", SourceType::ZeroPage, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xE9, "SBC", SourceType::Immediate, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xED, "SBC", SourceType::Absolute, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xF1, "SBC", SourceType::IndirectY, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xF5, "SBC", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xF9, "SBC", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0xFD, "SBC", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::SubtractWithCarry},
		{0x37, "SEC", SourceType::Implied, TargetType::None, &Processor::SetCarryFlag},
		{0xF8, "SED", SourceType::Implied, TargetType::None, &Processor::SetDecimalFlag},
		{0x78, "SEI", SourceType::Implied, TargetType::None, &Processor::SetInterruptFlag},
		{0x81, "STA", SourceType::XIndirect, TargetType::Accumulator, &Processor::Store},
		{0x85, "STA", SourceType::ZeroPage, TargetType::Accumulator, &Processor::Store},
		{0x8D, "STA", SourceType::Absolute, TargetType::Accumulator, &Processor::Store},
		{0x91, "STA", SourceType::IndirectY, TargetType::Accumulator, &Processor::Store},
		{0x95, "STA", SourceType::ZeroPageX, TargetType::Accumulator, &Processor::Store},
		{0x99, "STA", SourceType::AbsoluteY, TargetType::Accumulator, &Processor::Store},
		{0x9D, "STA", SourceType::AbsoluteX, TargetType::Accumulator, &Processor::Store},
		{0x86, "STX", SourceType::ZeroPage, TargetType::IndexX, &Processor::Store},
		{0x8E, "STX", SourceType::Absolute, TargetType::IndexX, &Processor::Store},
		{0x96, "STX", SourceType::ZeroPageY, TargetType::IndexX, &Processor::Store},
		{0x84, "STY", SourceType::ZeroPage, TargetType::IndexY, &Processor::Store},
		{0x8C, "STY", SourceType::Absolute, TargetType::IndexY, &Processor::Store},
		{0x94, "STY", SourceType::ZeroPageX, TargetType::IndexY, &Processor::Store},
		{0xAA, "TAX", SourceType::Accumulator, TargetType::IndexX, &Processor::Load},
		{0xA8, "TAY", SourceType::Accumulator, TargetType::IndexY, &Processor::Load},
		{0xBA, "TSX", SourceType::StackPointer, TargetType::IndexX, &Processor::Load},
		{0x8A, "TXA", SourceType::IndexX, TargetType::Accumulator, &Processor::Load},
		{0x9A, "TXS", SourceType::IndexX, TargetType::StackPointer, &Processor::Load},
		{0x98, "TYA", SourceType::IndexY, TargetType::Accumulator, &Processor::Load}
	};

	Instruction	InstructionSet[256];

	byte	*Memory;		// 64kb of RAM (hopefully)
	byte	*Source;		// instruction source data
	byte	*Target;		// instruction target
	byte	Data;			// data register
	byte	OpCode;			// instruction register
	word	Address;		// address register
	
#pragma region low level code
	bool SignBit(byte Value)
	{
		return Value & 0x80;
	}

	word AddWordByte(word A, byte B)
	{
		return ((int)A + B) & 0xFFFF;
	}

	byte AddBytes(byte A, byte B)
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
		this->Address = Memory[Address] | (Memory[AddWordByte(Address, 1)] << 8);

		return this->Address;
	}

	void WriteAddress(word Address)
	{
		Memory[Address] = this->Address & 0xFF;
		Memory[AddWordByte(Address, 1)] = this->Address >> 8;
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
		WriteFlag(Zero, (*Target == 0));
		WriteFlag(Negative, SignBit(*Target));
	}

#pragma endregion

#pragma region simple instructions
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
		WriteFlag(Carry, (*Target >= *Source));
		WriteFlag(Zero, (*Target == *Source));
		WriteFlag(Negative, SignBit(*Target));
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
		byte c = ReadFlag(Carry);
		WriteFlag(Carry, SignBit(*Target));
		*Target = ((*Target) << 1) | c;
		WriteTargetFlags();
	}

	void RotateRight()
	{
		byte c = ReadFlag(Carry);
		WriteFlag(Carry, (*Target) & 1);
		*Target = ((*Target) >> 1) | (c << 7);
		WriteTargetFlags();
	}

	void ShiftLeft()
	{
		WriteFlag(Carry, SignBit(*Target));
		*Target <<= 1;
		WriteTargetFlags();
	}

	void ShiftRight()
	{
		WriteFlag(Carry, (*Target) & 1);
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
		// TODO : check how the added carry affect the overflow logic
		word result = *Target + *Source + ReadFlag(Carry);

		// if both operands sign is identical but differs from the result sign (e.g. 100 + 49 = -107)
		WriteFlag(Overflow, ~(SignBit(*Target) ^ SignBit(*Source)) && (SignBit(*Target) ^ SignBit((byte)result)));
		*Target = result & 0xFF;
		WriteFlag(Carry, result & 0x100);
		WriteTargetFlags();
	}

	void SubtractWithCarry()
	{
		// TODO : implement SubtractWithCarry
	}

	void Push()
	{
		Memory[AddWordByte(0x100, S)] = *Target;
		S--;
	}

	void Pull()
	{
		*Target = Memory[AddWordByte(0x100, S)];
		S++;
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
		// TODO : implement Call
	}

	void Return()
	{
		// TODO : implement Return
	}

	void Break()
	{
		// TODO : implement Break
	}

	void ReturnFromInterrupt()
	{
		// TODO : implement ReturnFromInterrupt
	}

	void Nop()
	{
		// nope nope nope nope...
	}


#pragma endregion

#pragma region complex instructions
	void BranchIfPositive()
	{
		if (!ReadFlag(Negative))
		{
			Branch();
		}
	}
	void BranchIfMinus() {}
	void BranchIfEqual() {}
	void BranchIfNotEqual() {}
	void BranchIfCarryClear() {}
	void BranchIfCarrySet() {}
	void BranchIfOverflowClear() {}
	void BranchIfOverflowSet() {}

	void ClearCarryFlag()
	{
		WriteFlag(Carry, false);
	}
	
	void ClearDecimalFlag()
	{
		WriteFlag(Decimal, false);
	}

	void ClearInterruptFlag() 
	{
		WriteFlag(Interrupt, false);
	}
	void ClearOverflowFlag()
	{
		WriteFlag(Overflow, false);
	}

	void BitTest() {}

	void SetCarryFlag()
	{
		WriteFlag(Carry, true);
	}
	void SetDecimalFlag()
	{
		WriteFlag(Decimal, true);
	}
	void SetInterruptFlag()
	{
		WriteFlag(Interrupt, true);
	}

#pragma endregion

	void ExecuteInstruction()
	{
		ReadOpCode();
		Instruction in = InstructionSet[OpCode];

		switch (in.Source)
		{
		case SourceType::Accumulator:
			Source = (byte *)&A;
			break;
		case SourceType::IndexX:
			Source = (byte *)&X;
			break;
		case SourceType::IndexY:
			Source = (byte *)&Y;
			break;
		case SourceType::StackPointer:
			Source = (byte *)&S;
			break;
		case SourceType::Absolute:
			ReadAddressAtPC();
			Source = &Memory[Address];
			break;
		case SourceType::AbsoluteX:
			ReadAddressAtPC();
			Source = &Memory[AddWordByte(Address, X)];
			break;
		case SourceType::AbsoluteY:
			ReadAddressAtPC();
			Source = &Memory[AddWordByte(Address, Y)];
			break;
		case SourceType::Immediate:
			ReadDataAtPC();
			Source = &Data;
			break;
		case SourceType::Implied:
			Source = nullptr;
			break;
		case SourceType::Indirect:
			ReadAddressAtPC();
			ReadAddress(Address);
			Source = &Memory[Address];
			break;
		case SourceType::XIndirect:
			ReadDataAtPC();
			ReadAddress(Memory[AddBytes(Data, X)]);
			Source = &Memory[Address];
			break;
		case SourceType::IndirectY:
			ReadDataAtPC();
			ReadAddress(Data);
			Source = &Memory[AddBytes(Data, Y)];
			break;
		case SourceType::ZeroPage:
			ReadDataAtPC();
			Source = &Memory[Data];
			break;
		case SourceType::ZeroPageX:
			ReadDataAtPC();
			Source = &Memory[AddBytes(Data, X)];
			break;
		case SourceType::ZeroPageY:
			ReadDataAtPC();
			Source = &Memory[AddBytes(Data, Y)];
			break;
		default:
			// unknown addressing mode ?
			break;
		}

		switch (in.Target)
		{
		case TargetType::None:
			Target = nullptr;
			break;
		case TargetType::Accumulator:
			Target = (byte *)&A;
			break;
		case TargetType::IndexX:
			Target = &X;
			break;
		case TargetType::IndexY:
			Target = &Y;
			break;
		case TargetType::StackPointer:
			Target = &S;
			break;
		case TargetType::Status:
			Target = &P;
			break;
		case TargetType::Address:
			Target = &Memory[Address];
			break;
		default:
			// unknown target ?
			break;
		}

		(this->*in.Function)();
	}
			
public:
	byte	A;		// accumulator
	byte	X, Y;	// index registers
	word	PC;		// program counter
	byte	S;		// stack pointer
	byte	P;		// status flags

	Processor(byte *Array)
	{
		Memory = Array;
		for (int i = 0; i < 151; i++)
		{
			InstructionSet[LegalInstructionSet[i].OpCode] = LegalInstructionSet[i];
		}
	}

	void Reset()
	{
		S = 0xFD;
		P = 0b00100000;
		PC = ReadAddress(0xFFFC);
	}

	void Step()
	{
		ExecuteInstruction();
	}
};
