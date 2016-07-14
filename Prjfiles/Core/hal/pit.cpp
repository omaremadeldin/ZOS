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

#include "pit.h"

#include "hal.h"
#include "idt.h"
#include "pic.h"

using namespace zos;

assembly_stub void aihPIT(void);
interrupt_handler void ihPIT(void)
{	
	systemTimer++;
	
	PIC::sendEOI(PIT_IRQ_NO);
}

void PIT::sendCommand(uint8_t cmd)
{
	HAL::outportb(PIT_CTRL_REG, cmd);
}

void PIT::sendData(uint8_t data, uint8_t counter)
{
	switch (counter)
	{
		case PIT_COUNTER_0:
			HAL::outportb(PIT_CNT0_REG, data);
			break;
		case PIT_COUNTER_1:
			HAL::outportb(PIT_CNT1_REG, data);
			break;
		case PIT_COUNTER_2:
			HAL::outportb(PIT_CNT2_REG, data);
			break;
	}
}

void PIT::init()
{
	//Reset the system timer
	systemTimer = 0;
	
	//Install IRQ handler
	IDT::installHandler((void*)&aihPIT, PIC::IRQ0 + PIT_IRQ_NO);
	
	//1193181 is the clock rate (required for backward compatability)
	uint16_t divisor = PIT_CLK_RATE / frequency;
 	
	OCW bfOCW;
	BYTE_SET(bfOCW, 0);
	bfOCW.counterType = OCW::BinaryCounter;
	bfOCW.counterMode = OCW::RateGenMode;
	bfOCW.rlMode = OCW::DataMode;
	bfOCW.CounterNo = PIT_COUNTER_0;
 
 	//Send OCW to PIT
	sendCommand(BYTE_VAL(bfOCW));
 
 	//Send the selected frequency to counter 0 data register
 	sendData(LOBYTE(divisor), PIT_COUNTER_0);
	sendData(HIBYTE(divisor), PIT_COUNTER_0);
}