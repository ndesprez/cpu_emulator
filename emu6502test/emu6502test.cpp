#include "pch.h"
#include "CppUnitTest.h"
#include "..\emu6502\processor.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace emu6502test
{
	Processor *MOS6502;
	byte	  *Memory;

	static void _method_initialize()
	{
		Memory = new byte[0x10000];
		Memory[0xFFFC] = 0x00;
		Memory[0xFFFD] = 0x10;

		MOS6502 = new Processor(Memory);
		MOS6502->SendRST();
		MOS6502->Step();
	}

	static void _method_cleanup()
	{
		delete MOS6502;
		delete[] Memory;
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
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xD5;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xD5);
		}

		TEST_METHOD(LDA_ABS)
		{
			Memory[0x1000] = 0xAD;
			Memory[0x1001] = 0x00;
			Memory[0x1002] = 0x20;
			Memory[0x2000] = 0xDA;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDA);
		}

		TEST_METHOD(LDA_ABSX)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x20;
			Memory[0x1002] = 0xBD;
			Memory[0x1003] = 0x00;
			Memory[0x1004] = 0x20;
			Memory[0x2020] = 0xDB;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDB);
		}

		TEST_METHOD(LDA_ABSY)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x30;
			Memory[0x1002] = 0xB9;
			Memory[0x1003] = 0x00;
			Memory[0x1004] = 0x20;
			Memory[0x2030] = 0xDC;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDC);
		}

		TEST_METHOD(LDA_ZPG)
		{
			Memory[0x1000] = 0xA5;
			Memory[0x1001] = 0x40;
			Memory[0x0040] = 0xDD;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDD);
		}

		TEST_METHOD(LDA_ZPGX)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x10;
			Memory[0x1002] = 0xB5;
			Memory[0x1003] = 0x40;
			Memory[0x0050] = 0xDE;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDE);
		}

		TEST_METHOD(LDA_XIND)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x20;
			Memory[0x1002] = 0xA1;
			Memory[0x1003] = 0x20;
			Memory[0x0040] = 0x00;
			Memory[0x0041] = 0x30;
			Memory[0x3000] = 0xDF;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xDF);
		}

		TEST_METHOD(LDA_INDY)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x30;
			Memory[0x1002] = 0xB1;
			Memory[0x1003] = 0x20;
			Memory[0x0020] = 0x00;
			Memory[0x0021] = 0x40;
			Memory[0x4030] = 0xE0;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xE0);
		}

		TEST_METHOD(LDX_IMM)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0xC7;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xC7);
		}

		TEST_METHOD(LDX_ABS)
		{
			Memory[0x1000] = 0xAE;
			Memory[0x1001] = 0x00;
			Memory[0x1002] = 0x20;
			Memory[0x2000] = 0xC8;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xC8);
		}

		TEST_METHOD(LDX_ABSY)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x30;
			Memory[0x1002] = 0xBE;
			Memory[0x1003] = 0x00;
			Memory[0x1004] = 0x20;
			Memory[0x2030] = 0xC9;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xC9);
		}

		TEST_METHOD(LDX_ZPG)
		{
			Memory[0x1000] = 0xA6;
			Memory[0x1001] = 0x40;
			Memory[0x0040] = 0xDE;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xDE);
		}

		TEST_METHOD(LDX_ZPGY)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x91;
			Memory[0x1002] = 0xB6;
			Memory[0x1003] = 0x80;
			Memory[0x0011] = 0xDF;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0xDF);
		}

		TEST_METHOD(LDY_IMM)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x91;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0x91);
		}

		TEST_METHOD(LDY_ABS)
		{
			Memory[0x1000] = 0xAC;
			Memory[0x1001] = 0x00;
			Memory[0x1002] = 0x20;
			Memory[0x2000] = 0x92;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0x92);
		}
		
		TEST_METHOD(LDY_ABSX)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x30;
			Memory[0x1002] = 0xBC;
			Memory[0x1003] = 0x00;
			Memory[0x1004] = 0x20;
			Memory[0x2030] = 0xCA;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0xCA);
		}

		TEST_METHOD(LDY_ZPG)
		{
			Memory[0x1000] = 0xA4;
			Memory[0x1001] = 0x50;
			Memory[0x0050] = 0xDF;
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0xDF);
		}

		TEST_METHOD(LDY_ZPGX)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x20;
			Memory[0x1002] = 0xB4;
			Memory[0x1003] = 0x80;
			Memory[0x00A0] = 0xE0;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0xE0);
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
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xD5;
			Memory[0x1002] = 0x8D;
			Memory[0x1003] = 0x00;
			Memory[0x1004] = 0x20;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x2000], 0xD5);
		}

		TEST_METHOD(STA_ABSX)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xD6;
			Memory[0x1002] = 0xA2;
			Memory[0x1003] = 0xC7;
			Memory[0x1004] = 0x9D;
			Memory[0x1005] = 0x00;
			Memory[0x1006] = 0x20;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x20C7], 0xD6);
		}

		TEST_METHOD(STA_ABSY)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xD7;
			Memory[0x1002] = 0xA0;
			Memory[0x1003] = 0xB7;
			Memory[0x1004] = 0x99;
			Memory[0x1005] = 0x00;
			Memory[0x1006] = 0x20;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x20B7], 0xD7);
		}

		TEST_METHOD(STA_ZPG)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xD8;
			Memory[0x1002] = 0x85;
			Memory[0x1003] = 0x20;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x0020], 0xD8);
		}

		TEST_METHOD(STA_ZPGX)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xD9;
			Memory[0x1002] = 0xA2;
			Memory[0x1003] = 0x10;
			Memory[0x1004] = 0x95;
			Memory[0x1005] = 0x20;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x0030], 0xD9);
		}

		TEST_METHOD(STA_XIND)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xDA;
			Memory[0x1002] = 0xA2;
			Memory[0x1003] = 0x10;
			Memory[0x1004] = 0x81;
			Memory[0x1005] = 0x20;
			Memory[0x0030] = 0x16;
			Memory[0x0031] = 0x20;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x2016], 0xDA);
		}

		TEST_METHOD(STA_INDY)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xDB;
			Memory[0x1002] = 0xA0;
			Memory[0x1003] = 0x20;
			Memory[0x1004] = 0x91;
			Memory[0x1005] = 0x60;
			Memory[0x0060] = 0x11;
			Memory[0x0061] = 0x30;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x3031], 0xDB);
		}

		TEST_METHOD(STX_ABS)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x75;
			Memory[0x1002] = 0x8E;
			Memory[0x1003] = 0x00;
			Memory[0x1004] = 0x21;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x2100], 0x75);
		}

		TEST_METHOD(STX_ZPG)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x77;
			Memory[0x1002] = 0x86;
			Memory[0x1003] = 0x38;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x0038], 0x77);
		}

		TEST_METHOD(STX_ZPGY)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0x76;
			Memory[0x1002] = 0xA0;
			Memory[0x1003] = 0x10;
			Memory[0x1004] = 0x96;
			Memory[0x1005] = 0x40;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x0050], 0x76);
		}

		TEST_METHOD(STY_ABS)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x11;
			Memory[0x1002] = 0x8C;
			Memory[0x1003] = 0x10;
			Memory[0x1004] = 0x40;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x4010], 0x11);
		}

		TEST_METHOD(STY_ZPG)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x12;
			Memory[0x1002] = 0x84;
			Memory[0x1003] = 0x45;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)Memory[0x0045], 0x12);
		}

		TEST_METHOD(STY_ZPGX)
		{
			Memory[0x1000] = 0xA0;
			Memory[0x1001] = 0x13;
			Memory[0x1002] = 0xA2;
			Memory[0x1003] = 0x20;
			Memory[0x1004] = 0x94;
			Memory[0x1005] = 0x60;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
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
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0x3A;
			Memory[0x1002] = 0xA2;
			Memory[0x1003] = 0xFF;
			Memory[0x1004] = 0xAA;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, 0x3A);
		}

		TEST_METHOD(TAY)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0x3B;
			Memory[0x1002] = 0xA0;
			Memory[0x1003] = 0xFF;
			Memory[0x1004] = 0xA8;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->Y, 0x3B);
		}

		TEST_METHOD(TSX)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0xFF;
			Memory[0x1002] = 0xBA;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->X, (int)MOS6502->S);
		}

		TEST_METHOD(TXA)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xFF;
			Memory[0x1002] = 0xA2;
			Memory[0x1003] = 0xD1;
			Memory[0x1004] = 0x8A;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xD1);
		}

		TEST_METHOD(TXS)
		{
			Memory[0x1000] = 0xA2;
			Memory[0x1001] = 0xD3;
			Memory[0x1002] = 0x9A;
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->S, 0xD3);
		}

		TEST_METHOD(TYA)
		{
			Memory[0x1000] = 0xA9;
			Memory[0x1001] = 0xFF;
			Memory[0x1002] = 0xA0;
			Memory[0x1003] = 0xD2;
			Memory[0x1004] = 0x98;
			MOS6502->Step();
			MOS6502->Step();
			MOS6502->Step();
			Assert::AreEqual((int)MOS6502->A, 0xD2);
		}
	};
}
