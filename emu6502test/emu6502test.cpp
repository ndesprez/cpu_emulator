#include "pch.h"
#include "CppUnitTest.h"
#include "..\emu6502\processor.h"

#include <assert.h>
#include <ctype.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace emu6502test
{
	const wchar_t MessageNegativeFalse[] = L"Flag Negative is false.";
	const wchar_t MessageNegativeTrue[] = L"Flag Negative is true.";
	const wchar_t MessageZeroTrue[] = L"Flag Zero is true.";

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

	TEST_CLASS(LoadInstructions)
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
			Assert::AreEqual((int)CPU->A, 0xD5);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ABS)
		{
			_write("AD 00 20");
			_write(0x2000, "7A");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0x7A);
			Assert::IsFalse(CPU->FlagNegative(), MessageNegativeTrue);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ABSX)
		{
			_write("A2 20 BD 00 20");
			_write(0x2020, "DB");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xDB);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ABSY)
		{
			_write("A0 30 B9 00 20");
			_write(0x2030, "DC");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xDC);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ZPG)
		{
			_write("A5 40");
			_write(0x0040, "DD");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xDD);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ZPGX)
		{
			_write("A2 10 B5 40");
			_write(0x0050, "DE");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xDE);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_XIND)
		{
			_write("A2 20 A1 20");
			_write(0x0040, "00 30");
			_write(0x3000, "DF");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xDF);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_INDY)
		{
			_write("A0 30 B1 20");
			_write(0x0020, "00 40");
			_write(0x4030, "E0");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xE0);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_IMM)
		{
			_write("A2 C7");
			CPU->Run();
			Assert::AreEqual((int)CPU->X, 0xC7);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ABS)
		{
			_write("AE 00 20");
			_write(0x2000, "C8");
			CPU->Run();
			Assert::AreEqual((int)CPU->X, 0xC8);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ABSY)
		{
			_write("A0 30 BE 00 20");
			_write(0x2030, "C9");
			CPU->Run();
			Assert::AreEqual((int)CPU->X, 0xC9);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ZPG)
		{
			_write("A6 40");
			_write(0x0040, "DE");
			CPU->Run();
			Assert::AreEqual((int)CPU->X, 0xDE);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ZPGY)
		{
			_write("A0 91 B6 80");
			_write(0x0011, "DF");
			CPU->Run();
			Assert::AreEqual((int)CPU->X, 0xDF);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_IMM)
		{
			_write("A0 91");
			CPU->Run();
			Assert::AreEqual((int)CPU->Y, 0x91);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_ABS)
		{
			_write("AC 00 20");
			_write(0x2000, "92");
			CPU->Run();
			Assert::AreEqual((int)CPU->Y, 0x92);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}
		
		TEST_METHOD(LDY_ABSX)
		{
			_write("A2 30 BC 00 20");
			_write(0x2030, "CA");
			CPU->Run();
			Assert::AreEqual((int)CPU->Y, 0xCA);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_ZPG)
		{
			_write("A4 50");
			_write(0x0050, "DF");
			CPU->Run();
			Assert::AreEqual((int)CPU->Y, 0xDF);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_ZPGX)
		{
			_write("A2 20 B4 80");
			_write(0x00A0, "E0");
			CPU->Run();
			Assert::AreEqual((int)CPU->Y, 0xE0);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}
	};

	TEST_CLASS(StoreInstructions)
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
			Assert::AreEqual((int)RAM[0x2000], 0xD5);
		}

		TEST_METHOD(STA_ABSX)
		{
			_write("A9 D6 A2 C7 9D 00 20");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x20C7], 0xD6);
		}

		TEST_METHOD(STA_ABSY)
		{
			_write("A9 D7 A0 B7 99 00 20");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x20B7], 0xD7);
		}

		TEST_METHOD(STA_ZPG)
		{
			_write("A9 D8 85 20");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x0020], 0xD8);
		}

		TEST_METHOD(STA_ZPGX)
		{
			_write("A9 D9 A2 10 95 20");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x0030], 0xD9);
		}

		TEST_METHOD(STA_XIND)
		{
			_write("A9 DA A2 10 81 20");
			_write(0x0030, "16 20");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x2016], 0xDA);
		}

		TEST_METHOD(STA_INDY)
		{
			_write("A9 DB A0 20 91 60");
			_write(0x0060, "11 30");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x3031], 0xDB);
		}

		TEST_METHOD(STX_ABS)
		{
			_write("A2 75 8E 00 21");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x2100], 0x75);
		}

		TEST_METHOD(STX_ZPG)
		{
			_write("A2 77 86 38");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x0038], 0x77);
		}

		TEST_METHOD(STX_ZPGY)
		{
			_write("A2 76 A0 10 96 40");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x0050], 0x76);
		}

		TEST_METHOD(STY_ABS)
		{
			_write("A0 11 8C 10 40");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x4010], 0x11);
		}

		TEST_METHOD(STY_ZPG)
		{
			_write("A0 12 84 45");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x0045], 0x12);
		}

		TEST_METHOD(STY_ZPGX)
		{
			_write("A0 13 A2 20 94 60");
			CPU->Run();
			Assert::AreEqual((int)RAM[0x0080], 0x13);
		}
	};

	TEST_CLASS(TransferInstructions)
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
			Assert::AreEqual((int)CPU->X, 0x3A);
			Assert::IsFalse(CPU->FlagNegative(), MessageNegativeTrue);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TAY)
		{
			_write("A9 3B A0 FF A8");
			CPU->Run();
			Assert::AreEqual((int)CPU->Y, 0x3B);
			Assert::IsFalse(CPU->FlagNegative(), MessageNegativeTrue);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TSX)
		{
			_write("A2 FF BA");
			CPU->Run();
			Assert::AreEqual((int)CPU->X, (int)CPU->S);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TXA)
		{
			_write("A9 FF A2 D1 8A");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xD1);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TXS)
		{
			_write("A2 D3 9A");
			CPU->Run();
			Assert::AreEqual((int)CPU->S, 0xD3);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TYA)
		{
			_write("A9 FF A0 D2 98");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0xD2);
			Assert::IsTrue(CPU->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(CPU->FlagZero(), MessageZeroTrue);
		}
	};

	TEST_CLASS(StackInstructions)
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
			Assert::AreEqual(CPU->A, RAM[(word)CPU->S + 0x100 + 1]);
		}

		TEST_METHOD(PLA)
		{
			_write("A9 6A 48 A9 FF 68");
			CPU->Run();
			Assert::AreEqual((int)CPU->A, 0x6A);
		}

		TEST_METHOD(PHP)
		{
			_write("08");
			CPU->Run();
			Assert::AreEqual(CPU->P, RAM[(word)CPU->S + 0x100 + 1]);
		}

		TEST_METHOD(PLP)
		{
			_write("A9 31 48 28");
			CPU->Run();
			Assert::AreEqual((int)CPU->P, 0x31);
		}
	};

	TEST_CLASS(FlagsInstructions)
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
}
