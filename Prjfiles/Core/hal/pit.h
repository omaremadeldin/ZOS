//==========================================
//
//	 ZapperOS - 8253 PIT Microcontroller
//     (Programmable Interval Timer)
//==========================================
//Used Acronyms:
//--------------
//* PIT = Programmable Interval Timer
//* IRQ = Interrupt Request
//* OCW = Operation Command Word
//* LSB = Least Significant Bit
//* MSB = Most Significant Bit
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "utils/bitfield.h"

#define PIT_IRQ_NO		0x00		//IRQ0

#define PIT_CNT0_REG	0x40		//Reads and Writes to counter 0
#define PIT_CNT1_REG	0x41		//Reads and Writes to counter 1
#define PIT_CNT2_REG	0x42		//Reads and Writes to counter 2
#define PIT_CTRL_REG	0x43		//Writes to the PIT control register

#define PIT_CLK_RATE	1193181

#define PIT_COUNTER_0	0x00
#define PIT_COUNTER_1	0x01
#define PIT_COUNTER_2	0x02

namespace zos
{
	static uint32_t systemTimer;
	
	class PIT
	{
	private:
		struct OCW : BitField<uint8_t>
		{
			enum CounterType
			{
				BinaryCounter = 0,		//Binary Notion
				BCDCounter = 1			//Binary Coded Decimal
			};

			enum CounterMode
			{
				TerminalMode = 0,
				OneShotMode = 1,
				RateGenMode = 2,
				SquareWaveMode = 3,
				SoftwareMode = 4,
				HardwareMode = 5
			};

			enum RLMode
			{
				LatchMode = 0,
				LSBOnlyMode = 1,
				MSBOnlyMode = 2,
				DataMode = 3
			};
			
			CounterType 	counterType	:1;		//Counter Type
			CounterMode 	counterMode	:3;		//Counting Mode
			RLMode			rlMode		:2;		//Read Load Mode
			uint8_t			CounterNo	:2;		//Counter No. 0...3
		}__attribute__((packed));
	
		//Frequency (1000) = 1 count per millisecond
	private:
		static const uint32_t frequency = 1000;
		
	private:
		static void sendCommand(uint8_t cmd);
		static void sendData(uint8_t data, uint8_t counter);
	
	public:
		static void init();
	};
}