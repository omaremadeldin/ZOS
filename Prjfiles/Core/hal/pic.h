//==========================================
//
//	 ZapperOS - 8259A PIC Microcontroller
//   (Programmable Interrupt Controller)
//==========================================
//Used Acronyms:
//--------------
//* PIC = Programmable Interrupt Controller
//* IRQ = Interrupt Request
//* IMR = Interrupt Mask Register
//* ICW = Initialization Command Word
//* EOI = End Of Interrupt
//==========================================
//Initalizing:
//ICW1	->	Set Basic Settings For PIC
//ICW2	->	Set Base IRQ Interrupt No.
//ICW3	->	Set IR2 as the connection between PICs
//ICW4	->	Enable 80/86 Mode
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "utils/bitfield.h"

#define	PIC_MSTR_CTRL_REG	0x20
#define	PIC_MSTR_STS_REG	0x20
#define PIC_MSTR_DATA_REG	0x21
#define PIC_MSTR_IMR_REG	0x21
 
#define	PIC_SLAVE_CTRL_REG	0xA0
#define	PIC_SLAVE_STS_REG	0xA0
#define PIC_SLAVE_DATA_REG	0xA1
#define	PIC_SLAVE_IMR_REG	0xA1

namespace zos
{
	class PIC
	{
	private:
		//PIC ICW1 
		struct ICW1 : BitField<uint8_t>
		{
			enum InterruptType
			{
				EdgeTriggered = 0,
				LevelTriggered = 1
			};
			
			bool 					sendICW4		:1;		//Send ICW4 ?
			bool 					singlePIC		:1;		//Signle PIC ?
			uint8_t									:1;		//Reserved
			InterruptType			interruptType	:1;		//Interrupt Type ? (Level/Edge) Triggered
			bool 					Initialize		:1;		//Initialzie ?
			uint8_t 								:3;		//Reserved
		}__attribute__((packed));
		
	public:
		//PIC ICW2
		static const uint8_t IRQ0 = 0x20;			//IRQ0 Corresponding Interrupt No.
		static const uint8_t IRQ8 = IRQ0 + 8;		//IRQ8 Corresponding Interrupt No.
	
	private:
		//PIC ICW3
		static const uint8_t MASTER_ICW3 = 0b00000100;		//IR2 (Connects Master PIC to Slave PIC)
		static const uint8_t SLAVE_ICW3 = 0x02;			//IR2 (Connects Master PIC to Slave PIC)
		
		//PIC ICW4
		static const uint8_t ICW4 = 0x01;		//Enables 80x86 Modes
		
		//PIC OCW2 
		struct OCW2 : BitField<uint8_t>
		{
			bool 	Level1		:1;		//Level 1 Interrupt ?
			bool 	Level2		:1;		//Level 2 Interrupt ?
			bool 	Level3		:1;		//Level 3 Interrupt ?
			uint8_t 			:2;		//Reserved
			bool 	sendEOI		:1;		//Send EOI Signal ?
			bool 	Select		:1;		//Select ?
			bool 	Rotate		:1;		//Rotate ?
		}__attribute__((packed));
		
	private:
		static void sendCommandToMaster(uint8_t cmd);
		static void sendCommandToSlave(uint8_t cmd);			
		static void sendDataToMaster(uint8_t data);
		static void sendDataToSlave(uint8_t data);

	public:
		static void sendEOI(uint8_t irqNo);
		static void init();
	};
}