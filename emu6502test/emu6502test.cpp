#include "pch.h"
#include "CppUnitTest.h"
#include "..\emu6502\processor.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace emu6502test
{
	const wchar_t MessageNegativeFalse[] = L"Flag Negative is false.";
	const wchar_t MessageNegativeTrue[] = L"Flag Negative is true.";
	const wchar_t MessageZeroTrue[] = L"Flag Zero is true.";

	Processor	*MOS6502;
	byte		*Memory;
	word		WriteCounter;

	void _method_initialize()
	{
		Memory = new byte[0x10000];
		Memory[0xFFFC] = 0x00;
		Memory[0xFFFD] = 0x10;

		MOS6502 = new Processor(Memory);
		MOS6502->SendRST();
		MOS6502->Step();
		WriteCounter = MOS6502->PC;
	}

	void _method_cleanup()
	{
		delete MOS6502;
		delete[] Memory;
	}

	void _write(char const *Data)
	{
		byte value;
		word i = 0;
		bool high = true;

		while (char c = Data[i])
		{
			if (c == ' ')
			{
				i++;
				continue;
			}

			if (high)
				value = 0;

			if (c >= '0' && c <= '9')
				value |= c - 0x30;
			else if ((c | 0x20) >= 'a' && (c | 0x20) <= 'f')
				value |= (c | 0x20) - 0x57;

			if (high)
				value <<= 4;
			else
				Memory[WriteCounter++] = value;

			high = !high;
			i++;
		}
	}

	void _write(word Address, char const *Data)
	{
		WriteCounter = Address;
		_write(Data);
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
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xD5);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ABS)
		{
			_write("AD 00 20");
			_write(0x2000, "7A");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0x7A);
			Assert::IsFalse(MOS6502->FlagNegative(), MessageNegativeTrue);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ABSX)
		{
			_write("A2 20 BD 00 20");
			_write(0x2020, "DB");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->A, 0xDB);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ABSY)
		{
			_write("A0 30 B9 00 20");
			_write(0x2030, "DC");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->A, 0xDC);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ZPG)
		{
			_write("A5 40");
			_write(0x0040, "DD");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDD);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_ZPGX)
		{
			_write("A2 10 B5 40");
			_write(0x0050, "DE");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->A, 0xDE);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_XIND)
		{
			_write("A2 20 A1 20");
			_write(0x0040, "00 30");
			_write(0x3000, "DF");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->A, 0xDF);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDA_INDY)
		{
			_write("A0 30 B1 20");
			_write(0x0020, "00 40");
			_write(0x4030, "E0");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->A, 0xE0);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_IMM)
		{
			_write("A2 C7");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xC7);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ABS)
		{
			_write("AE 00 20");
			_write(0x2000, "C8");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xC8);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ABSY)
		{
			_write("A0 30 BE 00 20");
			_write(0x2030, "C9");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->X, 0xC9);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ZPG)
		{
			_write("A6 40");
			_write(0x0040, "DE");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xDE);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDX_ZPGY)
		{
			_write("A0 91 B6 80");
			_write(0x0011, "DF");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->X, 0xDF);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_IMM)
		{
			_write("A0 91");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0x91);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_ABS)
		{
			_write("AC 00 20");
			_write(0x2000, "92");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0x92);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}
		
		TEST_METHOD(LDY_ABSX)
		{
			_write("A2 30 BC 00 20");
			_write(0x2030, "CA");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->Y, 0xCA);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_ZPG)
		{
			_write("A4 50");
			_write(0x0050, "DF");
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0xDF);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(LDY_ZPGX)
		{
			_write("A2 20 B4 80");
			_write(0x00A0, "E0");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->Y, 0xE0);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
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
			MOS6502->Step(2);
			Assert::AreEqual((int)Memory[0x2000], 0xD5);
		}

		TEST_METHOD(STA_ABSX)
		{
			_write("A9 D6 A2 C7 9D 00 20");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x20C7], 0xD6);
		}

		TEST_METHOD(STA_ABSY)
		{
			_write("A9 D7 A0 B7 99 00 20");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x20B7], 0xD7);
		}

		TEST_METHOD(STA_ZPG)
		{
			_write("A9 D8 85 20");
			MOS6502->Step(2);
			Assert::AreEqual((int)Memory[0x0020], 0xD8);
		}

		TEST_METHOD(STA_ZPGX)
		{
			_write("A9 D9 A2 10 95 20");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x0030], 0xD9);
		}

		TEST_METHOD(STA_XIND)
		{
			_write("A9 DA A2 10 81 20");
			_write(0x0030, "16 20");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x2016], 0xDA);
		}

		TEST_METHOD(STA_INDY)
		{
			_write("A9 DB A0 20 91 60");
			_write(0x0060, "11 30");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x3031], 0xDB);
		}

		TEST_METHOD(STX_ABS)
		{
			_write("A2 75 8E 00 21");
			MOS6502->Step(2);
			Assert::AreEqual((int)Memory[0x2100], 0x75);
		}

		TEST_METHOD(STX_ZPG)
		{
			_write("A2 77 86 38");
			MOS6502->Step(2);
			Assert::AreEqual((int)Memory[0x0038], 0x77);
		}

		TEST_METHOD(STX_ZPGY)
		{
			_write("A2 76 A0 10 96 40");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x0050], 0x76);
		}

		TEST_METHOD(STY_ABS)
		{
			_write("A0 11 8C 10 40");
			MOS6502->Step(2);
			Assert::AreEqual((int)Memory[0x4010], 0x11);
		}

		TEST_METHOD(STY_ZPG)
		{
			_write("A0 12 84 45");
			MOS6502->Step(2);
			Assert::AreEqual((int)Memory[0x0045], 0x12);
		}

		TEST_METHOD(STY_ZPGX)
		{
			_write("A0 13 A2 20 94 60");
			MOS6502->Step(3);
			Assert::AreEqual((int)Memory[0x0080], 0x13);
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
			MOS6502->Step(3);
			Assert::AreEqual((int)MOS6502->X, 0x3A);
			Assert::IsFalse(MOS6502->FlagNegative(), MessageNegativeTrue);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TAY)
		{
			_write("A9 3B A0 FF A8");
			MOS6502->Step(3);
			Assert::AreEqual((int)MOS6502->Y, 0x3B);
			Assert::IsFalse(MOS6502->FlagNegative(), MessageNegativeTrue);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TSX)
		{
			_write("A2 FF BA");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->X, (int)MOS6502->S);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TXA)
		{
			_write("A9 FF A2 D1 8A");
			MOS6502->Step(3);
			Assert::AreEqual((int)MOS6502->A, 0xD1);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TXS)
		{
			_write("A2 D3 9A");
			MOS6502->Step(2);
			Assert::AreEqual((int)MOS6502->S, 0xD3);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}

		TEST_METHOD(TYA)
		{
			_write("A9 FF A0 D2 98");
			MOS6502->Step(3);
			Assert::AreEqual((int)MOS6502->A, 0xD2);
			Assert::IsTrue(MOS6502->FlagNegative(), MessageNegativeFalse);
			Assert::IsFalse(MOS6502->FlagZero(), MessageZeroTrue);
		}
	};
}
