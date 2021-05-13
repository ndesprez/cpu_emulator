#include "pch.h"
#include "CppUnitTest.h"
#include "..\emu6502\processor.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace emu6502test
{
	TEST_CLASS(Load)
	{
	public:
		Processor *MOS6502;
		byte	  *Memory;


		TEST_METHOD_INITIALIZE(createCPU)
		{
			// method initialization code
			Memory = new byte[0x10000];
			Memory[0xFFFC] = 0x00;
			Memory[0xFFFD] = 0x10;

			MOS6502 = new Processor(Memory);
			MOS6502->SendRST();
			MOS6502->Step();
		}

		TEST_METHOD_CLEANUP(deleteCPU)
		{
			delete MOS6502;
			delete[] Memory;
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
	};
}
