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
along with this progRAM->If not, see < http://www.gnu.org/licenses/>.
*/

#include "pch.h"
#include "CppUnitTest.h"
#include "processor.h"
#include "memory.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace emu6502test
{
	// TODO: need to think harder about the pattern we're seeing here (maybe create a class?)
	const wchar_t MessageCarryFalse[] = L"Carry flag is false.";
	const wchar_t MessageCarryTrue[] = L"Carry flag is true.";
	
	const wchar_t MessageZeroFalse[] = L"Zero flag is false.";
	const wchar_t MessageZeroTrue[] = L"Zero flag is true.";
	
	const wchar_t MessageInterruptFalse[] = L"Interrupt flag is false.";
	const wchar_t MessageInterruptTrue[] = L"Interrupt flag is true.";
	
	const wchar_t MessageDecimalFalse[] = L"Decimal flag is false.";
	const wchar_t MessageDecimalTrue[] = L"Decimal flag is true.";
	
	const wchar_t MessageOverflowFalse[] = L"Overflow flag is false.";
	const wchar_t MessageOverflowTrue[] = L"Overflow flag is true.";
	
	const wchar_t MessageNegativeFalse[] = L"Negative flag is false.";
	const wchar_t MessageNegativeTrue[] = L"Negative flag is true.";
	
	const wchar_t MessageMismatch[] = L"Instruction mismatch";

	Processor	*CPU;
	Memory		*RAM;
	word		WriteCounter;
	byte		InitialStatus;		// initial value of CPU->P
	byte		UnchangedFlagsMask; // when a bit is set, corresponding flag shouldn't change after the instruction is executed

	void AssertCarry(bool Value)
	{
		UnchangedFlagsMask &= ~fCarry;
		if(Value)
			Assert::IsTrue(CPU->FlagCarry(), MessageCarryFalse);
		else
			Assert::IsFalse(CPU->FlagCarry(), MessageCarryTrue);
	}	
	
	void AssertZero(bool Value)
	{
		UnchangedFlagsMask &= ~fZero;
		if (Value)
			Assert::IsTrue(CPU->FlagZero(), MessageZeroFalse);
		else
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
	}

	void AssertInterrupt(bool Value)
	{
		UnchangedFlagsMask &= ~fInterrupt;
		if (Value)
			Assert::IsTrue(CPU->FlagInterrupt(), MessageInterruptFalse);
		else
			Assert::IsFalse(CPU->FlagInterrupt(), MessageInterruptTrue);
	}

	void AssertDecimal(bool Value)
	{
		UnchangedFlagsMask &= ~fDecimal;
		if (Value)
			Assert::IsTrue(CPU->FlagDecimal(), MessageDecimalFalse);
		else
			Assert::IsFalse(CPU->FlagDecimal(), MessageDecimalTrue);
	}

	void AssertOverflow(bool Value)
	{
		UnchangedFlagsMask &= ~fOverflow;
		if (Value)
			Assert::IsTrue(CPU->FlagOverflow(), MessageOverflowFalse);
		else
			Assert::IsFalse(CPU->FlagOverflow(), MessageOverflowTrue);
	}

	void AssertNegative(bool Value)
	{ 
		UnchangedFlagsMask &= ~fNegative;
		if(Value)
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse); 
		else
			Assert::IsFalse(CPU->FlagNegative(), MessageNegativeTrue);
	}

	void AssertFlagsUnchanged()
	{
		Assert::IsFalse((InitialStatus ^ CPU->P) & UnchangedFlagsMask, L"A status flag has changed unexpectedly.");
	}

	// AssertLastInstruction: making sure the last instruction we ran is the one we're testing
	void AssertLastInstruction(const char *Name)
	{
		Assert::IsTrue(CPU->IsLastInstruction(Name), MessageMismatch);
	}

	void AssertLastInstruction(const char *Name, SourceType Source) 
	{
		Assert::IsTrue(CPU->IsLastInstruction(Name, Source), MessageMismatch);
	}

	void AssertLastInstruction(const char *Name, SourceType Source, TargetType Target)
	{
		Assert::IsTrue(CPU->IsLastInstruction(Name, Source, Target), MessageMismatch);
	}

	void _method_initialize()
	{
		RAM = new Memory();
		(*RAM)[0xFFFC] = 0x00;
		(*RAM)[0xFFFD] = 0x10;

		CPU = new Processor(RAM);
		CPU->EndOnBreak = true;
		CPU->SendRST();
		CPU->Step();
		RAM->WriteCounter = CPU->PC;
		UnchangedFlagsMask = 0xFF;
		InitialStatus = CPU->P;

		// always start the test with PHP so we can pull (i.e. reset) the 
		// status register just before we execute the instruction we're testing
		RAM->Write("08");
	}

	void _method_cleanup()
	{
		delete CPU;
		delete RAM;
	}

	TEST_CLASS(Load)
	{
	public:
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(LDA_IMM)
		{
			RAM->Write("A9 D5");
			CPU->Run();
			AssertLastInstruction("LDA", sImmediate);
			Assert::AreEqual(0xD5, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_ABS)
		{
			RAM->Write("AD 00 20");
			RAM->Write(0x2000, "7A");
			CPU->Run();
			AssertLastInstruction("LDA", sAbsolute);
			Assert::AreEqual(0x7A, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_ABSX)
		{
			RAM->Write("A2 20 28 BD 00 20");
			RAM->Write(0x2020, "DB");
			CPU->Run();
			AssertLastInstruction("LDA", sAbsoluteX);
			Assert::AreEqual(0xDB, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_ABSY)
		{
			RAM->Write("A0 30 28 B9 00 20");
			RAM->Write(0x2030, "DC");
			CPU->Run();
			AssertLastInstruction("LDA", sAbsoluteY);
			Assert::AreEqual(0xDC, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_ZPG)
		{
			RAM->Write("A5 40");
			RAM->Write(0x0040, "DD");
			CPU->Run();
			AssertLastInstruction("LDA", sZeroPage);
			Assert::AreEqual(0xDD, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_ZPGX)
		{
			RAM->Write("A2 10 28 B5 40");
			RAM->Write(0x0050, "DE");
			CPU->Run();
			AssertLastInstruction("LDA", sZeroPageX);
			Assert::AreEqual(0xDE, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_XIND)
		{
			RAM->Write("A2 20 28 A1 20");
			RAM->Write(0x0040, "00 30");
			RAM->Write(0x3000, "DF");
			CPU->Run();
			AssertLastInstruction("LDA", sXIndirect);
			Assert::AreEqual(0xDF, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDA_INDY)
		{
			RAM->Write("A0 30 28 B1 20");
			RAM->Write(0x0020, "00 40");
			RAM->Write(0x4030, "E0");
			CPU->Run();
			AssertLastInstruction("LDA", sIndirectY);
			Assert::AreEqual(0xE0, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDX_IMM)
		{
			RAM->Write("A2 C7");
			CPU->Run();
			AssertLastInstruction("LDX", sImmediate);
			Assert::AreEqual(0xC7, (int)CPU->X);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDX_ABS)
		{
			RAM->Write("AE 00 20");
			RAM->Write(0x2000, "C8");
			CPU->Run();
			AssertLastInstruction("LDX", sAbsolute);
			Assert::AreEqual(0xC8, (int)CPU->X);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDX_ABSY)
		{
			RAM->Write("A0 30 28 BE 00 20");
			RAM->Write(0x2030, "C9");
			CPU->Run();
			AssertLastInstruction("LDX", sAbsoluteY);
			Assert::AreEqual(0xC9, (int)CPU->X);
			AssertNegative(true);
			AssertZero(false);
		}

		TEST_METHOD(LDX_ZPG)
		{
			RAM->Write("A6 40");
			RAM->Write(0x0040, "DE");
			CPU->Run();
			AssertLastInstruction("LDX", sZeroPage);
			Assert::AreEqual(0xDE, (int)CPU->X);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDX_ZPGY)
		{
			RAM->Write("A0 91 28 B6 80");
			RAM->Write(0x0011, "DF");
			CPU->Run();
			AssertLastInstruction("LDX", sZeroPageY);
			Assert::AreEqual(0xDF, (int)CPU->X);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDY_IMM)
		{
			RAM->Write("A0 91");
			CPU->Run();
			AssertLastInstruction("LDY", sImmediate);
			Assert::AreEqual(0x91, (int)CPU->Y);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDY_ABS)
		{
			RAM->Write("AC 00 20");
			RAM->Write(0x2000, "92");
			CPU->Run();
			AssertLastInstruction("LDY", sAbsolute);
			Assert::AreEqual(0x92, (int)CPU->Y);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
		
		TEST_METHOD(LDY_ABSX)
		{
			RAM->Write("A2 30 28 BC 00 20");
			RAM->Write(0x2030, "CA");
			CPU->Run();
			AssertLastInstruction("LDY", sAbsoluteX);
			Assert::AreEqual(0xCA, (int)CPU->Y);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDY_ZPG)
		{
			RAM->Write("A4 50");
			RAM->Write(0x0050, "DF");
			CPU->Run();
			AssertLastInstruction("LDY", sZeroPage);
			Assert::AreEqual(0xDF, (int)CPU->Y);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LDY_ZPGX)
		{
			RAM->Write("A2 20 28 B4 80");
			RAM->Write(0x00A0, "E0");
			CPU->Run();
			AssertLastInstruction("LDY", sZeroPageX);
			Assert::AreEqual(0xE0, (int)CPU->Y);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Store)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(STA_ABS)
		{
			RAM->Write("A9 D5 28 8D 00 20");
			CPU->Run();
			AssertLastInstruction("STA", sAbsolute);
			Assert::AreEqual(0xD5, (int)(*RAM)[0x2000]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STA_ABSX)
		{
			RAM->Write("A9 D6 A2 C7 28 9D 00 20");
			CPU->Run();
			AssertLastInstruction("STA", sAbsoluteX);
			Assert::AreEqual(0xD6, (int)(*RAM)[0x20C7]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STA_ABSY)
		{
			RAM->Write("A9 D7 A0 B7 28 99 00 20");
			CPU->Run();
			AssertLastInstruction("STA", sAbsoluteY);
			Assert::AreEqual(0xD7, (int)(*RAM)[0x20B7]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STA_ZPG)
		{
			RAM->Write("A9 D8 28 85 20");
			CPU->Run();
			AssertLastInstruction("STA", sZeroPage);
			Assert::AreEqual(0xD8, (int)(*RAM)[0x0020]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STA_ZPGX)
		{
			RAM->Write("A9 D9 A2 10 28 95 20");
			CPU->Run();
			AssertLastInstruction("STA", sZeroPageX);
			Assert::AreEqual(0xD9, (int)(*RAM)[0x0030]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STA_XIND)
		{
			RAM->Write("A9 DA A2 10 28 81 20");
			RAM->Write(0x0030, "16 20");
			CPU->Run();
			AssertLastInstruction("STA", sXIndirect);
			Assert::AreEqual(0xDA, (int)(*RAM)[0x2016]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STA_INDY)
		{
			RAM->Write("A9 DB A0 20 28 91 60");
			RAM->Write(0x0060, "11 30");
			CPU->Run();
			AssertLastInstruction("STA", sIndirectY);
			Assert::AreEqual(0xDB, (int)(*RAM)[0x3031]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STX_ABS)
		{
			RAM->Write("A2 75 28 8E 00 21");
			CPU->Run();
			AssertLastInstruction("STX", sAbsolute);
			Assert::AreEqual(0x75, (int)(*RAM)[0x2100]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STX_ZPG)
		{
			RAM->Write("A2 77 28 86 38");
			CPU->Run();
			AssertLastInstruction("STX", sZeroPage);
			Assert::AreEqual(0x77, (int)(*RAM)[0x0038]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STX_ZPGY)
		{
			RAM->Write("A2 76 A0 10 28 96 40");
			CPU->Run();
			AssertLastInstruction("STX", sZeroPageY);
			Assert::AreEqual(0x76, (int)(*RAM)[0x0050]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STY_ABS)
		{
			RAM->Write("A0 11 28 8C 10 40");
			CPU->Run();
			AssertLastInstruction("STY", sAbsolute);
			Assert::AreEqual(0x11, (int)(*RAM)[0x4010]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STY_ZPG)
		{
			RAM->Write("A0 12 28 84 45");
			CPU->Run();
			AssertLastInstruction("STY", sZeroPage);
			Assert::AreEqual(0x12, (int)(*RAM)[0x0045]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(STY_ZPGX)
		{
			RAM->Write("A0 13 A2 20 28 94 60");
			CPU->Run();
			AssertLastInstruction("STY", sZeroPageX);
			Assert::AreEqual(0x13, (int)(*RAM)[0x0080]);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Transfer)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(TAX)
		{
			RAM->Write("A9 3A A2 FF 28 AA");
			CPU->Run();
			AssertLastInstruction("TAX");
			Assert::AreEqual(0x3A, (int)CPU->X);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(TAY)
		{
			RAM->Write("A9 3B A0 FF 28 A8");
			CPU->Run();
			AssertLastInstruction("TAY");
			Assert::AreEqual(0x3B, (int)CPU->Y);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(TSX)
		{
			RAM->Write("A2 FF 28 BA");
			CPU->Run();
			AssertLastInstruction("TSX");
			Assert::AreEqual((int)CPU->S, (int)CPU->X);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(TXA)
		{
			RAM->Write("A9 FF A2 D1 28 8A");
			CPU->Run();
			AssertLastInstruction("TXA");
			Assert::AreEqual(0xD1, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(TXS)
		{
			RAM->Write("A2 D3 28 9A");
			CPU->Run();
			AssertLastInstruction("TXS");
			Assert::AreEqual(0xD3, (int)CPU->S);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(TYA)
		{
			RAM->Write("A9 FF A0 D2 28 98");
			CPU->Run();
			AssertLastInstruction("TYA");
			Assert::AreEqual(0xD2, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Stack)
	{
	public:
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(PHA)
		{
			RAM->Write("A9 5A 48");
			CPU->Run();
			AssertLastInstruction("PHA");
			Assert::AreEqual((*RAM)[(word)CPU->S + 0x100 + 1], CPU->A);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(PLA)
		{
			// we can't pull P here because we need to pull A first
			RAM->Write("A9 6A 48 A9 01 68");
			CPU->Run();
			AssertLastInstruction("PLA");
			Assert::AreEqual(0x6A, (int)CPU->A);
			// TODO: test negative and zero flags
			AssertFlagsUnchanged();
		}

		TEST_METHOD(PHP)
		{
			RAM->Write("08");
			CPU->Run();
			AssertLastInstruction("PHP");
			Assert::AreEqual((*RAM)[(word)CPU->S + 0x100 + 1], CPU->P);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(PLP)
		{
			RAM->Write("A9 31 48 28"); // LDA #$31 PHA PLP (carry is set)
			CPU->Run();
			AssertLastInstruction("PLP");
			Assert::AreEqual(0x31, (int)CPU->P);
			AssertCarry(true); // PLP doesn't affect the carry but our code does
			AssertInterrupt(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Flags)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(SEC)
		{
			RAM->Write("38");
			CPU->Run();
			AssertLastInstruction("SEC");
			AssertCarry(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SED)
		{
			RAM->Write("F8");
			CPU->Run();
			AssertLastInstruction("SED");
			AssertDecimal(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SEI)
		{
			RAM->Write("78");
			CPU->Run();
			AssertLastInstruction("SEI");
			AssertInterrupt(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CLC)
		{
			RAM->Write("38 18");
			CPU->Run();
			AssertLastInstruction("CLC");
			AssertCarry(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CLD)
		{
			RAM->Write("F8 D8");
			CPU->Run();
			AssertLastInstruction("CLD");
			AssertDecimal(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CLI)
		{
			RAM->Write("78 58");
			CPU->Run();
			AssertLastInstruction("CLI");
			AssertInterrupt(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CLV)
		{
			RAM->Write("A9 7F 69 7F B8"); // forces an overflow then clears it
			CPU->Run();
			AssertLastInstruction("CLV");
			AssertOverflow(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(And)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(AND_IMM)
		{
			RAM->Write("A9 55 28 29 0F");
			CPU->Run();
			AssertLastInstruction("AND", sImmediate);
			Assert::AreEqual(0x05, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_ABS)
		{
			RAM->Write("A9 66 28 2D 00 20");
			RAM->Write(0x2000, "F0");
			CPU->Run();
			AssertLastInstruction("AND", sAbsolute);
			Assert::AreEqual(0x60, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_ABSX)
		{
			RAM->Write("A9 77 A2 10 28 3D 00 20");
			RAM->Write(0x2010, "0F");
			CPU->Run();
			AssertLastInstruction("AND", sAbsoluteX);
			Assert::AreEqual(0x07, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_ABSY)
		{
			RAM->Write("A9 88 A0 20 28 39 00 20");
			RAM->Write(0x2020, "F0");
			CPU->Run();
			AssertLastInstruction("AND", sAbsoluteY);
			Assert::AreEqual(0x80, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_ZPG)
		{
			RAM->Write("A9 99 28 25 48");
			RAM->Write(0x0048, "0F");
			CPU->Run();
			AssertLastInstruction("AND", sZeroPage);
			Assert::AreEqual(0x09, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_ZPGX)
		{
			RAM->Write("A9 AA A2 10 28 35 58");
			RAM->Write(0x0068, "55");
			CPU->Run();
			AssertLastInstruction("AND", sZeroPageX);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertNegative(false);
			AssertZero(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_XIND)
		{
			RAM->Write("A9 BB A2 30 28 21 70");
			RAM->Write(0x00A0, "10 20");
			RAM->Write(0x2010, "0F");
			CPU->Run();
			AssertLastInstruction("AND", sXIndirect);
			Assert::AreEqual(0x0B, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(AND_INDY)
		{
			RAM->Write("A9 CC A0 18 28 31 35");
			RAM->Write(0x0035, "10 20");
			RAM->Write(0x2028, "F0");
			CPU->Run();
			AssertLastInstruction("AND", sIndirectY);
			Assert::AreEqual(0xC0, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Or)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(ORA_IMM)
		{
			RAM->Write("A9 05 28 09 A0");
			CPU->Run();
			AssertLastInstruction("ORA", sImmediate);
			Assert::AreEqual(0xA5, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_ABS)
		{
			RAM->Write("A9 06 28 0D 00 20");
			RAM->Write(0x2000, "B0");
			CPU->Run();
			AssertLastInstruction("ORA", sAbsolute);
			Assert::AreEqual(0xB6, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_ABSX)
		{
			RAM->Write("A9 07 A2 20 28 1D 00 30");
			RAM->Write(0x3020, "C0");
			CPU->Run();
			AssertLastInstruction("ORA", sAbsoluteX);
			Assert::AreEqual(0xC7, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_ABSY)
		{
			RAM->Write("A9 08 A0 30 28 19 00 40");
			RAM->Write(0x4030, "D0");
			CPU->Run();
			AssertLastInstruction("ORA", sAbsoluteY);
			Assert::AreEqual(0xD8, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_ZPG)
		{
			RAM->Write("A9 09 28 05 78");
			RAM->Write(0x0078, "E0");
			CPU->Run();
			AssertLastInstruction("ORA", sZeroPage);
			Assert::AreEqual(0xE9, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_ZPGX)
		{
			RAM->Write("A9 0A A2 10 28 15 38");
			RAM->Write(0x0048, "F0");
			CPU->Run();
			AssertLastInstruction("ORA", sZeroPageX);
			Assert::AreEqual(0xFA, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_XIND)
		{
			RAM->Write("A9 0B A2 38 28 01 40");
			RAM->Write(0x0078, "10 50");
			RAM->Write(0x5010, "10");
			CPU->Run();
			AssertLastInstruction("ORA", sXIndirect);
			Assert::AreEqual(0x1B, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ORA_INDY)
		{
			RAM->Write("A9 0C A0 20 28 11 60");
			RAM->Write(0x0060, "00 40");
			RAM->Write(0x4020, "20");
			CPU->Run();
			AssertLastInstruction("ORA", sIndirectY);
			Assert::AreEqual(0x2C, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Xor)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(EOR_IMM)
		{
			RAM->Write("A9 FF 28 49 AA");
			CPU->Run();
			AssertLastInstruction("EOR", sImmediate);
			Assert::AreEqual(0x55, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_ABS)
		{
			RAM->Write("A9 F0 28 4D 00 20");
			RAM->Write(0x2000, "38");
			CPU->Run();
			AssertLastInstruction("EOR", sAbsolute);
			Assert::AreEqual(0xC8, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_ABSX)
		{
			RAM->Write("A9 09 A2 10 28 5D 00 30");
			RAM->Write(0x3010, "C8");
			CPU->Run();
			AssertLastInstruction("EOR", sAbsoluteX);
			Assert::AreEqual(0xC1, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_ABSY)
		{
			RAM->Write("A9 90 A0 20 28 59 00 30");
			RAM->Write(0x3020, "8C");
			CPU->Run();
			AssertLastInstruction("EOR", sAbsoluteY);
			Assert::AreEqual(0x1C, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_ZPG)
		{
			RAM->Write("A9 5A 28 45 88");
			RAM->Write(0x0088, "FF");
			CPU->Run();
			AssertLastInstruction("EOR", sZeroPage);
			Assert::AreEqual(0xA5, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_ZPGX)
		{
			RAM->Write("A9 A5 A2 10 28 55 98");
			RAM->Write(0x00A8, "F0");
			CPU->Run();
			AssertLastInstruction("EOR", sZeroPageX);
			Assert::AreEqual(0x55, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_XIND)
		{
			RAM->Write("A9 FF A2 50 28 41 10");
			RAM->Write(0x0060, "00 71");
			RAM->Write(0x7100, "55");
			CPU->Run();
			AssertLastInstruction("EOR", sXIndirect);
			Assert::AreEqual(0xAA, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(EOR_INDY)
		{
			RAM->Write("A9 FF A0 20 28 51 20");
			RAM->Write(0x0020, "00 26");
			RAM->Write(0x2620, "AA");
			CPU->Run();
			AssertLastInstruction("EOR", sIndirectY);
			Assert::AreEqual(0x55, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Shift)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(ASL_A)
		{
			RAM->Write("A9 87 28 18 0A");
			CPU->Run();
			AssertLastInstruction("ASL A");
			Assert::AreEqual(0x0E, (int)CPU->A);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ASL_ABS)
		{
			RAM->Write("28 0E 00 20");
			RAM->Write(0x2000, "55");
			CPU->Run();
			AssertLastInstruction("ASL", sAbsolute);
			Assert::AreEqual(0xAA, (int)(*RAM)[0x2000]);
			AssertCarry(false);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ASL_ABSX)
		{
			RAM->Write("A2 30 28 1E 50 30");
			RAM->Write(0x3080, "80");
			CPU->Run();
			AssertLastInstruction("ASL", sAbsoluteX);
			Assert::AreEqual(0x00, (int)(*RAM)[0x3080]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ASL_ZPG)
		{
			RAM->Write("28 06 20");
			RAM->Write(0x0020, "55");
			CPU->Run();
			AssertLastInstruction("ASL", sZeroPage);
			Assert::AreEqual(0xAA, (int)(*RAM)[0x0020]);
			AssertCarry(false);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ASL_ZPGX)
		{
			RAM->Write("A2 30 28 16 30");
			RAM->Write(0x0060, "C0");
			CPU->Run();
			AssertLastInstruction("ASL", sZeroPageX);
			Assert::AreEqual(0x80, (int)(*RAM)[0x0060]);
			AssertCarry(true);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LSR_A)
		{
			RAM->Write("A9 87 28 4A");
			CPU->Run();
			AssertLastInstruction("LSR A");
			Assert::AreEqual(0x43, (int)CPU->A);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LSR_ABS)
		{
			RAM->Write("4E 00 20");
			RAM->Write(0x2000, "55");
			CPU->Run();
			AssertLastInstruction("LSR", sAbsolute);
			Assert::AreEqual(0x2A, (int)(*RAM)[0x2000]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LSR_ABSX)
		{
			RAM->Write("A2 30 28 5E 50 30");
			RAM->Write(0x3080, "01");
			CPU->Run();
			AssertLastInstruction("LSR", sAbsoluteX);
			Assert::AreEqual(0x00, (int)(*RAM)[0x3080]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LSR_ZPG)
		{
			RAM->Write("46 20");
			RAM->Write(0x0020, "FF");
			CPU->Run();
			AssertLastInstruction("LSR", sZeroPage);
			Assert::AreEqual(0x7F, (int)(*RAM)[0x0020]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(LSR_ZPGX)
		{
			RAM->Write("A2 30 28 56 30");
			RAM->Write(0x0060, "C0");
			CPU->Run();
			AssertLastInstruction("LSR", sZeroPageX);
			Assert::AreEqual(0x60, (int)(*RAM)[0x0060]);
			AssertCarry(false);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Rotate)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(ROL_A)
		{
			RAM->Write("A9 87 28 2A");
			CPU->Run();
			AssertLastInstruction("ROL A");
			Assert::AreEqual(0x0E, (int)CPU->A);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROL_ABS)
		{
			RAM->Write("28 38 2E 00 20");
			RAM->Write(0x2000, "55");
			CPU->Run();
			AssertLastInstruction("ROL", sAbsolute);
			Assert::AreEqual(0xAB, (int)(*RAM)[0x2000]);
			AssertCarry(false);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROL_ABSX)
		{
			RAM->Write("A2 30 28 3E 50 30");
			RAM->Write(0x3080, "80");
			CPU->Run();
			AssertLastInstruction("ROL", sAbsoluteX);
			Assert::AreEqual(0x00, (int)(*RAM)[0x3080]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROL_ZPG)
		{
			RAM->Write("26 20");
			RAM->Write(0x0020, "55");
			CPU->Run();
			AssertLastInstruction("ROL", sZeroPage);
			Assert::AreEqual(0xAA, (int)(*RAM)[0x0020]);
			AssertCarry(false);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROL_ZPGX)
		{
			RAM->Write("A2 30 28 36 30");
			RAM->Write(0x0060, "C4");
			CPU->Run();
			AssertLastInstruction("ROL", sZeroPageX);
			Assert::AreEqual(0x88, (int)(*RAM)[0x0060]);
			AssertCarry(true);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROR_A)
		{
			RAM->Write("A9 87 28 6A");
			CPU->Run();
			AssertLastInstruction("ROR A");
			Assert::AreEqual(0x43, (int)CPU->A);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROR_ABS)
		{
			RAM->Write("6E 00 20");
			RAM->Write(0x2000, "55");
			CPU->Run();
			AssertLastInstruction("ROR", sAbsolute);
			Assert::AreEqual(0x2A, (int)(*RAM)[0x2000]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROR_ABSX)
		{
			RAM->Write("A2 30 28 38 7E 50 30");
			RAM->Write(0x3080, "01");
			CPU->Run();
			AssertLastInstruction("ROR", sAbsoluteX);
			Assert::AreEqual(0x80, (int)(*RAM)[0x3080]);
			AssertCarry(true);
			AssertNegative(true);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROR_ZPG)
		{
			RAM->Write("66 20");
			RAM->Write(0x0020, "FF");
			CPU->Run();
			AssertLastInstruction("ROR", sZeroPage);
			Assert::AreEqual(0x7F, (int)(*RAM)[0x0020]);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ROR_ZPGX)
		{
			RAM->Write("A2 30 28 76 30");
			RAM->Write(0x0060, "C0");
			CPU->Run();
			AssertLastInstruction("ROR", sZeroPageX);
			Assert::AreEqual(0x60, (int)(*RAM)[0x0060]);
			AssertCarry(false);
			AssertNegative(false);
			AssertZero(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Compare)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(CMP_IMM)
		{
			RAM->Write("A9 60 28 C9 61");
			CPU->Run();
			AssertLastInstruction("CMP", sImmediate);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_ABS)
		{
			RAM->Write("A9 10 28 CD 00 30");
			RAM->Write(0x3000, "F0");
			CPU->Run();
			AssertLastInstruction("CMP", sAbsolute);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_ABSX)
		{
			RAM->Write("A9 F0 A2 10 28 DD 00 30");
			RAM->Write(0x3010, "F0");
			CPU->Run();
			AssertLastInstruction("CMP", sAbsoluteX);
			AssertCarry(true);
			AssertZero(true);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_ABSY)
		{
			RAM->Write("A9 AA A0 20 28 D9 00 40");
			RAM->Write(0x4020, "55");
			CPU->Run();
			AssertLastInstruction("CMP", sAbsoluteY);
			AssertCarry(true);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_ZPG)
		{
			RAM->Write("A9 10 28 C5 30");
			RAM->Write(0x0030, "F0");
			CPU->Run();
			AssertLastInstruction("CMP", sZeroPage);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_ZPGX)
		{
			RAM->Write("A9 F0 A2 10 28 D5 30");
			RAM->Write(0x0040, "F0");
			CPU->Run();
			AssertLastInstruction("CMP", sZeroPageX);
			AssertCarry(true);
			AssertZero(true);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_XIND)
		{
			RAM->Write("A9 40 A2 50 28 C1 40");
			RAM->Write(0x0090, "00 30");
			RAM->Write(0x3000, "80");
			CPU->Run();
			AssertLastInstruction("CMP", sXIndirect);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CMP_INDY)
		{
			RAM->Write("A9 80 A0 20 28 D1 30");
			RAM->Write(0x0030, "00 30");
			RAM->Write(0x3020, "40");
			CPU->Run();
			AssertLastInstruction("CMP", sIndirectY);
			AssertCarry(true);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CPX_IMM)
		{
			RAM->Write("A2 60 28 E0 61");
			CPU->Run();
			AssertLastInstruction("CPX", sImmediate);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CPX_ABS)
		{
			RAM->Write("A2 10 28 EC 00 30");
			RAM->Write(0x3000, "F0");
			CPU->Run();
			AssertLastInstruction("CPX", sAbsolute);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CPX_ZPG)
		{
			RAM->Write("A2 10 28 E4 30");
			RAM->Write(0x0030, "F0");
			CPU->Run();
			AssertLastInstruction("CPX", sZeroPage);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CPY_IMM)
		{
			RAM->Write("A0 61 28 C0 60");
			CPU->Run();
			AssertLastInstruction("CPY", sImmediate);
			AssertCarry(true);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CPY_ABS)
		{
			RAM->Write("A0 10 28 CC 00 30");
			RAM->Write(0x3000, "F0");
			CPU->Run();
			AssertLastInstruction("CPY", sAbsolute);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(CPY_ZPG)
		{
			RAM->Write("A0 10 28 C4 30");
			RAM->Write(0x0030, "F6");
			CPU->Run();
			AssertLastInstruction("CPY", sZeroPage);
			AssertCarry(false);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Increment)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(INC_ABS)
		{
			RAM->Write("EE 00 20");
			RAM->Write(0x2000, "30");
			CPU->Run();
			AssertLastInstruction("INC", sAbsolute);
			Assert::AreEqual(0x31, (int)(*RAM)[0x2000]);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(INC_ABSX)
		{
			RAM->Write("A2 50 28 FE 00 30");
			RAM->Write(0x3050, "FF");
			CPU->Run();
			AssertLastInstruction("INC", sAbsoluteX);
			Assert::AreEqual(0x00, (int)(*RAM)[0x3050]);
			AssertZero(true);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(INC_ZPG)
		{
			RAM->Write("E6 20");
			RAM->Write(0x0020, "7F");
			CPU->Run();
			AssertLastInstruction("INC", sZeroPage);
			Assert::AreEqual(0x80, (int)(*RAM)[0x0020]);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(INC_ZPGX)
		{
			RAM->Write("A2 40 28 F6 40");
			RAM->Write(0x0080, "FE");
			CPU->Run();
			AssertLastInstruction("INC", sZeroPageX);
			Assert::AreEqual(0xFF, (int)(*RAM)[0x0080]);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(INX)
		{
			RAM->Write("A2 0F 28 E8");
			CPU->Run();
			AssertLastInstruction("INX");
			Assert::AreEqual(0x10, (int)CPU->X);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(INY)
		{
			RAM->Write("A0 FF 28 C8");
			CPU->Run();
			AssertLastInstruction("INY");
			Assert::AreEqual(0x00, (int)CPU->Y);
			AssertZero(true);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Decrement)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(DEC_ABS)
		{
			RAM->Write("CE 00 20");
			RAM->Write(0x2000, "30");
			CPU->Run();
			AssertLastInstruction("DEC", sAbsolute);
			Assert::AreEqual(0x2F, (int)(*RAM)[0x2000]);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(DEC_ABSX)
		{
			RAM->Write("A2 50 28 DE 00 30");
			RAM->Write(0x3050, "00");
			CPU->Run();
			AssertLastInstruction("DEC", sAbsoluteX);
			Assert::AreEqual(0xFF, (int)(*RAM)[0x3050]);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(DEC_ZPG)
		{
			RAM->Write("C6 20");
			RAM->Write(0x0020, "80");
			CPU->Run();
			AssertLastInstruction("DEC", sZeroPage);
			Assert::AreEqual(0x7F, (int)(*RAM)[0x0020]);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(DEC_ZPGX)
		{
			RAM->Write("A2 40 28 D6 40");
			RAM->Write(0x0080, "FF");
			CPU->Run();
			AssertLastInstruction("DEC", sZeroPageX);
			Assert::AreEqual(0xFE, (int)(*RAM)[0x0080]);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(DEX)
		{
			RAM->Write("A2 0F 28 CA");
			CPU->Run();
			AssertLastInstruction("DEX");
			Assert::AreEqual(0x0E, (int)CPU->X);
			AssertZero(false);
			AssertNegative(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(DEY)
		{
			RAM->Write("A0 00 28 88");
			CPU->Run();
			AssertLastInstruction("DEY");
			Assert::AreEqual(0xFF, (int)CPU->Y);
			AssertZero(false);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Add)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(ADC_IMM)
		{
			RAM->Write("A9 01 28 69 01");
			CPU->Run();
			AssertLastInstruction("ADC", sImmediate);
			Assert::AreEqual(0x02, (int)CPU->A);
			AssertCarry(false);
			AssertNegative(false);
			AssertZero(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_ABS)
		{
			RAM->Write("A9 FF 28 38 6D 00 20");
			RAM->Write(0x2000, "FF");
			CPU->Run();
			AssertLastInstruction("ADC", sAbsolute);
			Assert::AreEqual(0xFF, (int)CPU->A);
			AssertCarry(true);
			AssertNegative(true);
			AssertZero(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_ABSX)
		{
			RAM->Write("A9 80 A2 10 28 7D 00 20");
			RAM->Write(0x2010, "80");
			CPU->Run();
			AssertLastInstruction("ADC", sAbsoluteX);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertNegative(false);
			AssertZero(true);
			AssertCarry(true);
			AssertOverflow(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_ABSY)
		{
			RAM->Write("A9 0A A0 20 28 79 00 20");
			RAM->Write(0x2020, "F6");
			CPU->Run();
			AssertLastInstruction("ADC", sAbsoluteY);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertNegative(false);
			AssertZero(true);
			AssertCarry(true);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_ZPG)
		{
			RAM->Write("A9 20 28 65 48");
			RAM->Write(0x0048, "40");
			CPU->Run();
			AssertLastInstruction("ADC", sZeroPage);
			Assert::AreEqual(0x60, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_ZPGX)
		{
			RAM->Write("A9 F6 A2 10 28 75 58");
			RAM->Write(0x0068, "F6");
			CPU->Run();
			AssertLastInstruction("ADC", sZeroPageX);
			Assert::AreEqual(0xEC, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertCarry(true);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_XIND)
		{
			RAM->Write("A9 0F A2 30 28 38 61 70");
			RAM->Write(0x00A0, "10 20");
			RAM->Write(0x2010, "0F");
			CPU->Run();
			AssertLastInstruction("ADC", sXIndirect);
			Assert::AreEqual(0x1F, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_INDY)
		{
			RAM->Write("A9 7F A0 18 28 38 71 35");
			RAM->Write(0x0035, "10 20");
			RAM->Write(0x2028, "7F");
			CPU->Run();
			AssertLastInstruction("ADC", sIndirectY);
			Assert::AreEqual(0xFF, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(ADC_IMM_BCD)
		{
			RAM->Write("F8 A9 50 69 50");
			CPU->Run();
			AssertLastInstruction("ADC", sImmediate);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertCarry(true);
			AssertDecimal(true);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Subtract)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(SBC_IMM)
		{
			RAM->Write("A9 01 28 18 E9 01");
			CPU->Run();
			AssertLastInstruction("SBC", sImmediate);
			Assert::AreEqual(0xFF, (int)CPU->A);
			AssertCarry(false);
			AssertNegative(true);
			AssertZero(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_ABS)
		{
			RAM->Write("A9 FF 28 38 ED 00 20");
			RAM->Write(0x2000, "FF");
			CPU->Run();
			AssertLastInstruction("SBC", sAbsolute);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertCarry(true);
			AssertNegative(false);
			AssertZero(true);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_ABSX)
		{
			RAM->Write("A9 80 A2 10 28 FD 00 20");
			RAM->Write(0x2010, "7F");
			CPU->Run();
			AssertLastInstruction("SBC", sAbsoluteX);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertNegative(false);
			AssertZero(true);
			AssertCarry(true);
			AssertOverflow(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_ABSY)
		{
			RAM->Write("A9 0A A0 20 38 F9 00 20");
			RAM->Write(0x2020, "F6");
			CPU->Run();
			AssertLastInstruction("SBC", sAbsoluteY);
			Assert::AreEqual(0x14, (int)CPU->A);
			AssertNegative(false);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_ZPG)
		{
			RAM->Write("A9 20 28 E5 48");
			RAM->Write(0x0048, "40");
			CPU->Run();
			AssertLastInstruction("SBC", sZeroPage);
			Assert::AreEqual(0xDF, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_ZPGX)
		{
			RAM->Write("A9 F6 A2 10 38 F5 58");
			RAM->Write(0x0068, "F6");
			CPU->Run();
			AssertLastInstruction("SBC", sZeroPageX);
			Assert::AreEqual(0x00, (int)CPU->A);
			AssertNegative(false);
			AssertZero(true);
			AssertCarry(true);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_XIND)
		{
			RAM->Write("A9 0F A2 30 28 38 E1 70");
			RAM->Write(0x00A0, "10 20");
			RAM->Write(0x2010, "1F");
			CPU->Run();
			AssertLastInstruction("SBC", sXIndirect);
			Assert::AreEqual(0xF0, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_INDY)
		{
			RAM->Write("A9 7F A0 18 28 38 F1 35");
			RAM->Write(0x0035, "10 20");
			RAM->Write(0x2028, "80");
			CPU->Run();
			AssertLastInstruction("SBC", sIndirectY);
			Assert::AreEqual(0xFF, (int)CPU->A);
			AssertNegative(true);
			AssertZero(false);
			AssertCarry(false);
			AssertOverflow(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(SBC_IMM_BCD)
		{
			RAM->Write("F8 A9 41 38 E9 36");
			CPU->Run();
			AssertLastInstruction("SBC", sImmediate);
			Assert::AreEqual(0x05, (int)CPU->A);
			AssertCarry(true);
			AssertDecimal(true);
			AssertOverflow(false);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Branch)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(BCC)
		{
			RAM->Write("18 90 0F");
			RAM->Write(0x1013, "00");
			CPU->Run();
			AssertLastInstruction("BCC");
			Assert::AreEqual(0x1013, (int)CPU->PC - 1); // -1 to account for BRK
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BCS)
		{
			RAM->Write("38 B0 F0");
			RAM->Write(0x0FF4, "00");
			CPU->Run();
			AssertLastInstruction("BCS");
			Assert::AreEqual(0x0FF4, (int)CPU->PC - 1);
			AssertCarry(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BEQ)
		{
			RAM->Write("A9 00 F0 7F");
			RAM->Write(0x1084, "00");
			CPU->Run();
			AssertLastInstruction("BEQ");
			Assert::AreEqual(0x1084, (int)CPU->PC - 1);
			AssertZero(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BNE)
		{
			RAM->Write("A9 01 D0 80");
			RAM->Write(0x0F85, "00");
			CPU->Run();
			AssertLastInstruction("BNE");
			Assert::AreEqual(0x0F85, (int)CPU->PC - 1);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BMI)
		{
			RAM->Write("A9 FF 30 7F");
			RAM->Write(0x1084, "00");
			CPU->Run();
			AssertLastInstruction("BMI");
			Assert::AreEqual(0x1084, (int)CPU->PC - 1);
			AssertNegative(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BPL)
		{
			RAM->Write("A9 01 10 80");
			RAM->Write(0x0F85, "00");
			CPU->Run();
			AssertLastInstruction("BPL");
			Assert::AreEqual(0x0F85, (int)CPU->PC - 1);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BVS)
		{
			RAM->Write("A9 7F 69 7F 70 7F");
			RAM->Write(0x1086, "00");
			CPU->Run();
			AssertLastInstruction("BVS");
			Assert::AreEqual(0x1086, (int)CPU->PC - 1);
			AssertNegative(true);
			AssertOverflow(true);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BVC)
		{
			RAM->Write("50 80");
			RAM->Write(0x0F83, "00");
			CPU->Run();
			AssertLastInstruction("BVC");
			Assert::AreEqual(0x0F83, (int)CPU->PC - 1);
			AssertFlagsUnchanged();
		}
	};

	TEST_CLASS(Bit)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(BIT_ABS)
		{
			RAM->Write("A9 05 2C 00 20");
			RAM->Write(0x2000, "FA");
			CPU->Run();
			AssertLastInstruction("BIT", sAbsolute);
			AssertZero(true);
			AssertOverflow(true);
			AssertNegative(true);
		}

		TEST_METHOD(BIT_ZPG)
		{
			RAM->Write("A9 F8 24 20");
			RAM->Write(0x0020, "0F");
			CPU->Run();
			AssertLastInstruction("BIT", sZeroPage);
			AssertZero(false);
			AssertOverflow(false);
			AssertNegative(false);
		}
	};

	TEST_CLASS(Other)
	{
		TEST_METHOD_INITIALIZE(createCPU)
		{
			_method_initialize();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			_method_cleanup();
		}

		TEST_METHOD(NOP)
		{
			RAM->Write("EA");
			CPU->Run();
			AssertLastInstruction("NOP");
			AssertFlagsUnchanged();
		}

		TEST_METHOD(JMP_ABS)
		{
			RAM->Write("4C 00 40");
			RAM->Write(0x4000, "00");
			CPU->Run();
			AssertLastInstruction("JMP", sAbsolute);
			Assert::AreEqual(0x4000, CPU->PC - 1);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(JMP_IND)
		{
			RAM->Write("6C 00 40");
			RAM->Write(0x4000, "00 50");
			RAM->Write(0x5000, "00");
			CPU->Run();
			AssertLastInstruction("JMP", sIndirect);
			Assert::AreEqual(0x5000, CPU->PC - 1);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(JMP_IND_BUG)
		{
			// instead of reading 0x50 at 0x4000, JMP (0x3FFF) reads 0x60 at 0x3FF00
			RAM->Write("6C FF 3F");
			RAM->Write(0x3FFF, "00 50");
			RAM->Write(0x3F00, "60");
			RAM->Write(0x6000, "00");
			CPU->Run();
			AssertLastInstruction("JMP", sIndirect);
			Assert::AreEqual(0x6000, CPU->PC - 1);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(JSR)
		{
			RAM->Write("20 00 30");
			RAM->Write(0x3000, "00");
			CPU->Run();
			AssertLastInstruction("JSR");
			Assert::AreEqual(0x3000, CPU->PC - 1);
			// checks PC on the stack
			Assert::AreEqual(0x03, (int)(*RAM)[(word)CPU->S + 0x100 + 1]);
			Assert::AreEqual(0x10, (int)(*RAM)[(word)CPU->S + 0x100 + 2]);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(RTS)
		{
			RAM->Write("20 00 30");
			RAM->Write(0x3000, "60");
			CPU->Run();
			AssertLastInstruction("RTS");
			Assert::AreEqual(0x1004, CPU->PC - 1);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(BRK)
		{
			RAM->Write("00");
			RAM->Write(0xFFFE, "00 80");
			CPU->EndOnBreak = false;
			CPU->Step(2);
			AssertLastInstruction("BRK");
			Assert::AreEqual(0x8000, (int)CPU->PC);
			AssertFlagsUnchanged();
		}

		TEST_METHOD(RTI)
		{
			RAM->Write("00 EA");
			RAM->Write(0xFFFE, "00 80");
			RAM->Write(0x8000, "EA 40");
			CPU->EndOnBreak = false;
			CPU->Step(4);
			AssertLastInstruction("RTI");
			Assert::AreEqual(0x1003, (int)CPU->PC);
			AssertFlagsUnchanged();
		}
	};
}
