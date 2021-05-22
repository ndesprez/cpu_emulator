#include "pch.h"
#include "CppUnitTest.h"
#include "..\emu6502\processor.h"

#include <assert.h>
#include <ctype.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace emu6502test
{
	const wchar_t MessageNegativeFalse[] = L"Negative flag is false.";
	const wchar_t MessageNegativeTrue[] = L"Negative flag is true.";
	const wchar_t MessageZeroFalse[] = L"Zero flag is false.";
	const wchar_t MessageZeroTrue[] = L"Zero flag is true.";
	const wchar_t MessageCarryFalse[] = L"Carry flag is false.";
	const wchar_t MessageCarryTrue[] = L"Carry flag is true.";

#define ASSERT_NEGATIVE_TRUE	Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse)
#define ASSERT_NEGATIVE_FALSE	Assert::IsFalse(CPU->FlagNegative(), MessageNegativeTrue)
#define ASSERT_ZERO_TRUE		Assert::IsTrue(CPU->FlagZero(), MessageZeroFalse)
#define ASSERT_ZERO_FALSE		Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue)
#define ASSERT_CARRY_TRUE		Assert::IsTrue(CPU->FlagCarry(), MessageCarryFalse)
#define ASSERT_CARRY_FALSE		Assert::IsFalse(CPU->FlagCarry(), MessageCarryTrue)

	Processor	*CPU;
	byte		*RAM;
	word		WriteCounter;

	void _method_initialize()
	{
		RAM = new byte[0x10000];
		RAM[0xFFFC] = 0x00;
		RAM[0xFFFD] = 0x10;

		CPU = new Processor(RAM);
		CPU->EndOnBreak = true;
		CPU->SendRST();
		CPU->Step();
		WriteCounter = CPU->PC;
	}

	void _method_cleanup()
	{
		delete CPU;
		delete[] RAM;
	}

	void _write(char const *Data, bool AddBreak = true)
	{
		byte value;
		word i = 0;
		bool high = true;

		while (char c = Data[i])
		{
			if (c == ' ')
			{
				assert(high == true);
				i++;
				continue;
			}

			assert(isxdigit(c));

			if (high)
				value = 0;

			if (c >= '0' && c <= '9')
				value |= c - 0x30;
			else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f')
				value |= (c | 0x20) - 0x57;

			if (high)
				value <<= 4;
			else
				RAM[WriteCounter++] = value;

			high = !high;
			i++;
		}
		assert(high == true);

		if (AddBreak)
		{
			RAM[WriteCounter] = 0x00;
		}
	}

	void _write(word Address, char const *Data, bool AddBreak = false)
	{
		WriteCounter = Address;
		_write(Data, AddBreak);
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
			_write("A9 D5");
			CPU->Run();
			Assert::AreEqual(0xD5, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_ABS)
		{
			_write("AD 00 20");
			_write(0x2000, "7A");
			CPU->Run();
			Assert::AreEqual(0x7A, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_ABSX)
		{
			_write("A2 20 BD 00 20");
			_write(0x2020, "DB");
			CPU->Run();
			Assert::AreEqual(0xDB, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_ABSY)
		{
			_write("A0 30 B9 00 20");
			_write(0x2030, "DC");
			CPU->Run();
			Assert::AreEqual(0xDC, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_ZPG)
		{
			_write("A5 40");
			_write(0x0040, "DD");
			CPU->Run();
			Assert::AreEqual(0xDD, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_ZPGX)
		{
			_write("A2 10 B5 40");
			_write(0x0050, "DE");
			CPU->Run();
			Assert::AreEqual(0xDE, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_XIND)
		{
			_write("A2 20 A1 20");
			_write(0x0040, "00 30");
			_write(0x3000, "DF");
			CPU->Run();
			Assert::AreEqual(0xDF, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDA_INDY)
		{
			_write("A0 30 B1 20");
			_write(0x0020, "00 40");
			_write(0x4030, "E0");
			CPU->Run();
			Assert::AreEqual(0xE0, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDX_IMM)
		{
			_write("A2 C7");
			CPU->Run();
			Assert::AreEqual(0xC7, (int)CPU->X);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDX_ABS)
		{
			_write("AE 00 20");
			_write(0x2000, "C8");
			CPU->Run();
			Assert::AreEqual(0xC8, (int)CPU->X);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDX_ABSY)
		{
			_write("A0 30 BE 00 20");
			_write(0x2030, "C9");
			CPU->Run();
			Assert::AreEqual(0xC9, (int)CPU->X);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDX_ZPG)
		{
			_write("A6 40");
			_write(0x0040, "DE");
			CPU->Run();
			Assert::AreEqual(0xDE, (int)CPU->X);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDX_ZPGY)
		{
			_write("A0 91 B6 80");
			_write(0x0011, "DF");
			CPU->Run();
			Assert::AreEqual(0xDF, (int)CPU->X);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDY_IMM)
		{
			_write("A0 91");
			CPU->Run();
			Assert::AreEqual(0x91, (int)CPU->Y);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDY_ABS)
		{
			_write("AC 00 20");
			_write(0x2000, "92");
			CPU->Run();
			Assert::AreEqual(0x92, (int)CPU->Y);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}
		
		TEST_METHOD(LDY_ABSX)
		{
			_write("A2 30 BC 00 20");
			_write(0x2030, "CA");
			CPU->Run();
			Assert::AreEqual(0xCA, (int)CPU->Y);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDY_ZPG)
		{
			_write("A4 50");
			_write(0x0050, "DF");
			CPU->Run();
			Assert::AreEqual(0xDF, (int)CPU->Y);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LDY_ZPGX)
		{
			_write("A2 20 B4 80");
			_write(0x00A0, "E0");
			CPU->Run();
			Assert::AreEqual(0xE0, (int)CPU->Y);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 D5 8D 00 20");
			CPU->Run();
			Assert::AreEqual(0xD5, (int)RAM[0x2000]);
		}

		TEST_METHOD(STA_ABSX)
		{
			_write("A9 D6 A2 C7 9D 00 20");
			CPU->Run();
			Assert::AreEqual(0xD6, (int)RAM[0x20C7]);
		}

		TEST_METHOD(STA_ABSY)
		{
			_write("A9 D7 A0 B7 99 00 20");
			CPU->Run();
			Assert::AreEqual(0xD7, (int)RAM[0x20B7]);
		}

		TEST_METHOD(STA_ZPG)
		{
			_write("A9 D8 85 20");
			CPU->Run();
			Assert::AreEqual(0xD8, (int)RAM[0x0020]);
		}

		TEST_METHOD(STA_ZPGX)
		{
			_write("A9 D9 A2 10 95 20");
			CPU->Run();
			Assert::AreEqual(0xD9, (int)RAM[0x0030]);
		}

		TEST_METHOD(STA_XIND)
		{
			_write("A9 DA A2 10 81 20");
			_write(0x0030, "16 20");
			CPU->Run();
			Assert::AreEqual(0xDA, (int)RAM[0x2016]);
		}

		TEST_METHOD(STA_INDY)
		{
			_write("A9 DB A0 20 91 60");
			_write(0x0060, "11 30");
			CPU->Run();
			Assert::AreEqual(0xDB, (int)RAM[0x3031]);
		}

		TEST_METHOD(STX_ABS)
		{
			_write("A2 75 8E 00 21");
			CPU->Run();
			Assert::AreEqual(0x75, (int)RAM[0x2100]);
		}

		TEST_METHOD(STX_ZPG)
		{
			_write("A2 77 86 38");
			CPU->Run();
			Assert::AreEqual(0x77, (int)RAM[0x0038]);
		}

		TEST_METHOD(STX_ZPGY)
		{
			_write("A2 76 A0 10 96 40");
			CPU->Run();
			Assert::AreEqual(0x76, (int)RAM[0x0050]);
		}

		TEST_METHOD(STY_ABS)
		{
			_write("A0 11 8C 10 40");
			CPU->Run();
			Assert::AreEqual(0x11, (int)RAM[0x4010]);
		}

		TEST_METHOD(STY_ZPG)
		{
			_write("A0 12 84 45");
			CPU->Run();
			Assert::AreEqual(0x12, (int)RAM[0x0045]);
		}

		TEST_METHOD(STY_ZPGX)
		{
			_write("A0 13 A2 20 94 60");
			CPU->Run();
			Assert::AreEqual(0x13, (int)RAM[0x0080]);
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
			_write("A9 3A A2 FF AA");
			CPU->Run();
			Assert::AreEqual(0x3A, (int)CPU->X);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(TAY)
		{
			_write("A9 3B A0 FF A8");
			CPU->Run();
			Assert::AreEqual(0x3B, (int)CPU->Y);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(TSX)
		{
			_write("A2 FF BA");
			CPU->Run();
			Assert::AreEqual((int)CPU->S, (int)CPU->X);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(TXA)
		{
			_write("A9 FF A2 D1 8A");
			CPU->Run();
			Assert::AreEqual(0xD1, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(TXS)
		{
			_write("A2 D3 9A");
			CPU->Run();
			Assert::AreEqual(0xD3, (int)CPU->S);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(TYA)
		{
			_write("A9 FF A0 D2 98");
			CPU->Run();
			Assert::AreEqual(0xD2, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 5A 48");
			CPU->Run();
			Assert::AreEqual(RAM[(word)CPU->S + 0x100 + 1], CPU->A);
		}

		TEST_METHOD(PLA)
		{
			_write("A9 6A 48 A9 FF 68");
			CPU->Run();
			Assert::AreEqual(0x6A, (int)CPU->A);
		}

		TEST_METHOD(PHP)
		{
			_write("08");
			CPU->Run();
			Assert::AreEqual(RAM[(word)CPU->S + 0x100 + 1], CPU->P);
		}

		TEST_METHOD(PLP)
		{
			_write("A9 31 48 28");
			CPU->Run();
			Assert::AreEqual(0x31, (int)CPU->P);
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
			_write("38");
			CPU->Run();
			Assert::IsTrue(CPU->FlagCarry());
		}

		TEST_METHOD(SED)
		{
			_write("F8");
			CPU->Run();
			Assert::IsTrue(CPU->FlagDecimal());
		}

		TEST_METHOD(SEI)
		{
			_write("78");
			CPU->Run();
			Assert::IsTrue(CPU->FlagInterrupt());
		}

		TEST_METHOD(CLC)
		{
			_write("38 18");
			CPU->Run();
			Assert::IsFalse(CPU->FlagCarry());
		}

		TEST_METHOD(CLD)
		{
			_write("F8 D8");
			CPU->Run();
			Assert::IsFalse(CPU->FlagDecimal());
		}

		TEST_METHOD(CLI)
		{
			_write("78 58");
			CPU->Run();
			Assert::IsFalse(CPU->FlagInterrupt());
		}

		TEST_METHOD(CLV)
		{
			_write("A9 40 48 28 B8");
			CPU->Run();
			Assert::IsFalse(CPU->FlagOverflow());
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
			_write("A9 55 29 0F");
			CPU->Run();
			Assert::AreEqual(0x05, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(AND_ABS)
		{
			_write("A9 66 2D 00 20");
			_write(0x2000, "F0");
			CPU->Run();
			Assert::AreEqual(0x60, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(AND_ABSX)
		{
			_write("A9 77 A2 10 3D 00 20");
			_write(0x2010, "0F");
			CPU->Run();
			Assert::AreEqual(0x07, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(AND_ABSY)
		{
			_write("A9 88 A0 20 39 00 20");
			_write(0x2020, "F0");
			CPU->Run();
			Assert::AreEqual(0x80, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(AND_ZPG)
		{
			_write("A9 99 25 48");
			_write(0x0048, "0F");
			CPU->Run();
			Assert::AreEqual(0x09, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(AND_ZPGX)
		{
			_write("A9 AA A2 10 35 58");
			_write(0x0068, "55");
			CPU->Run();
			Assert::AreEqual(0x00, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_TRUE;
		}

		TEST_METHOD(AND_XIND)
		{
			_write("A9 BB A2 30 21 70");
			_write(0x00A0, "10 20");
			_write(0x2010, "0F");
			CPU->Run();
			Assert::AreEqual(0x0B, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(AND_INDY)
		{
			_write("A9 CC A0 18 31 35");
			_write(0x0035, "10 20");
			_write(0x2028, "F0");
			CPU->Run();
			Assert::AreEqual(0xC0, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 05 09 A0");
			CPU->Run();
			Assert::AreEqual(0xA5, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_ABS)
		{
			_write("A9 06 0D 00 20");
			_write(0x2000, "B0");
			CPU->Run();
			Assert::AreEqual(0xB6, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_ABSX)
		{
			_write("A9 07 A2 20 1D 00 30");
			_write(0x3020, "C0");
			CPU->Run();
			Assert::AreEqual(0xC7, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_ABSY)
		{
			_write("A9 08 A0 30 19 00 40");
			_write(0x4030, "D0");
			CPU->Run();
			Assert::AreEqual(0xD8, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_ZPG)
		{
			_write("A9 09 05 78");
			_write(0x0078, "E0");
			CPU->Run();
			Assert::AreEqual(0xE9, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_ZPGX)
		{
			_write("A9 0A A2 10 15 38");
			_write(0x0048, "F0");
			CPU->Run();
			Assert::AreEqual(0xFA, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_XIND)
		{
			_write("A9 0B A2 38 01 40");
			_write(0x0078, "10 50");
			_write(0x5010, "10");
			CPU->Run();
			Assert::AreEqual(0x1B, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ORA_INDY)
		{
			_write("A9 0C A0 20 11 60");
			_write(0x0060, "00 40");
			_write(0x4020, "20");
			CPU->Run();
			Assert::AreEqual(0x2C, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 FF 49 AA");
			CPU->Run();
			Assert::AreEqual(0x55, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_ABS)
		{
			_write("A9 F0 4D 00 20");
			_write(0x2000, "38");
			CPU->Run();
			Assert::AreEqual(0xC8, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_ABSX)
		{
			_write("A9 09 A2 10 5D 00 30");
			_write(0x3010, "C8");
			CPU->Run();
			Assert::AreEqual(0xC1, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_ABSY)
		{
			_write("A9 90 A2 10 5D 00 30");
			_write(0x3010, "8C");
			CPU->Run();
			Assert::AreEqual(0x1C, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_ZPG)
		{
			_write("A9 5A 45 88");
			_write(0x0088, "FF");
			CPU->Run();
			Assert::AreEqual(0xA5, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_ZPGX)
		{
			_write("A9 A5 A2 10 55 98");
			_write(0x00A8, "F0");
			CPU->Run();
			Assert::AreEqual(0x55, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_XIND)
		{
			_write("A9 FF A2 50 41 10");
			_write(0x0060, "00 71");
			_write(0x7100, "55");
			CPU->Run();
			Assert::AreEqual(0xAA, (int)CPU->A);
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(EOR_INDY)
		{
			_write("A9 FF A0 20 51 20");
			_write(0x0020, "00 26");
			_write(0x2620, "AA");
			CPU->Run();
			Assert::AreEqual(0x55, (int)CPU->A);
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 87 18 0A");
			CPU->Run();
			Assert::AreEqual(0x0E, (int)CPU->A);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ASL_ABS)
		{
			_write("18 0E 00 20");
			_write(0x2000, "55");
			CPU->Run();
			Assert::AreEqual(0xAA, (int)RAM[0x2000]);
			ASSERT_CARRY_FALSE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ASL_ABSX)
		{
			_write("A2 30 18 1E 50 30");
			_write(0x3080, "80");
			CPU->Run();
			Assert::AreEqual(0x00, (int)RAM[0x3080]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_TRUE;
		}

		TEST_METHOD(ASL_ZPG)
		{
			_write("18 06 20");
			_write(0x0020, "55");
			CPU->Run();
			Assert::AreEqual(0xAA, (int)RAM[0x0020]);
			ASSERT_CARRY_FALSE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ASL_ZPGX)
		{
			_write("A2 30 18 16 30");
			_write(0x0060, "C0");
			CPU->Run();
			Assert::AreEqual(0x80, (int)RAM[0x0060]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LSR_A)
		{
			_write("A9 87 4A");
			CPU->Run();
			Assert::AreEqual(0x43, (int)CPU->A);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LSR_ABS)
		{
			_write("4E 00 20");
			_write(0x2000, "55");
			CPU->Run();
			Assert::AreEqual(0x2A, (int)RAM[0x2000]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LSR_ABSX)
		{
			_write("A2 30 5E 50 30");
			_write(0x3080, "01");
			CPU->Run();
			Assert::AreEqual(0x00, (int)RAM[0x3080]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_TRUE;
		}

		TEST_METHOD(LSR_ZPG)
		{
			_write("46 20");
			_write(0x0020, "FF");
			CPU->Run();
			Assert::AreEqual(0x7F, (int)RAM[0x0020]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(LSR_ZPGX)
		{
			_write("A2 30 56 30");
			_write(0x0060, "C0");
			CPU->Run();
			Assert::AreEqual(0x60, (int)RAM[0x0060]);
			ASSERT_CARRY_FALSE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 87 18 2A");
			CPU->Run();
			Assert::AreEqual(0x0E, (int)CPU->A);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROL_ABS)
		{
			_write("38 2E 00 20");
			_write(0x2000, "55");
			CPU->Run();
			Assert::AreEqual(0xAB, (int)RAM[0x2000]);
			ASSERT_CARRY_FALSE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROL_ABSX)
		{
			_write("A2 30 18 3E 50 30");
			_write(0x3080, "80");
			CPU->Run();
			Assert::AreEqual(0x00, (int)RAM[0x3080]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_TRUE;
		}

		TEST_METHOD(ROL_ZPG)
		{
			_write("18 26 20");
			_write(0x0020, "55");
			CPU->Run();
			Assert::AreEqual(0xAA, (int)RAM[0x0020]);
			ASSERT_CARRY_FALSE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROL_ZPGX)
		{
			_write("A2 30 18 36 30");
			_write(0x0060, "C4");
			CPU->Run();
			Assert::AreEqual(0x88, (int)RAM[0x0060]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROR_A)
		{
			_write("A9 87 18 6A");
			CPU->Run();
			Assert::AreEqual(0x43, (int)CPU->A);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROR_ABS)
		{
			_write("18 6E 00 20");
			_write(0x2000, "55");
			CPU->Run();
			Assert::AreEqual(0x2A, (int)RAM[0x2000]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROR_ABSX)
		{
			_write("A2 30 38 7E 50 30");
			_write(0x3080, "01");
			CPU->Run();
			Assert::AreEqual(0x80, (int)RAM[0x3080]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_TRUE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROR_ZPG)
		{
			_write("18 66 20");
			_write(0x0020, "FF");
			CPU->Run();
			Assert::AreEqual(0x7F, (int)RAM[0x0020]);
			ASSERT_CARRY_TRUE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
		}

		TEST_METHOD(ROR_ZPGX)
		{
			_write("A2 30 18 76 30");
			_write(0x0060, "C0");
			CPU->Run();
			Assert::AreEqual(0x60, (int)RAM[0x0060]);
			ASSERT_CARRY_FALSE;
			ASSERT_NEGATIVE_FALSE;
			ASSERT_ZERO_FALSE;
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
			_write("A9 60 C9 61");
			CPU->Run();
			ASSERT_CARRY_FALSE;
			ASSERT_ZERO_FALSE;
			ASSERT_NEGATIVE_TRUE;
		}

		TEST_METHOD(CMP_ABS)
		{
			_write("A9 10 CD 00 30");
			_write(0x3000, "F0");
			CPU->Run();
			ASSERT_CARRY_FALSE;
			ASSERT_ZERO_FALSE;
			ASSERT_NEGATIVE_FALSE;
		}

		TEST_METHOD(CMP_ABSX)
		{
			_write("A9 F0 A2 10 DD 00 30");
			_write(0x3010, "F0");
			CPU->Run();
			ASSERT_CARRY_TRUE;
			ASSERT_ZERO_TRUE;
			ASSERT_NEGATIVE_FALSE;
		}
		TEST_METHOD(CMP_ABSY)
		{
			_write("A9 AA A0 20 D9 00 40");
			_write(0x4020, "55");
			CPU->Run();
			ASSERT_CARRY_TRUE;
			ASSERT_ZERO_FALSE;
			ASSERT_NEGATIVE_FALSE;
		}
	};
}
