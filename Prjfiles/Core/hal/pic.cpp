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

#include "pic.h"

#include "hal.h"

using namespace zos;

void PIC::sendCommandToMaster(uint8_t cmd)
{
	HAL::outportb(PIC_MSTR_CTRL_REG, cmd);
}

void PIC::sendCommandToSlave(uint8_t cmd)
{
	HAL::outportb(PIC_SLAVE_CTRL_REG, cmd);
}

void PIC::sendDataToMaster(uint8_t data)
{
	HAL::outportb(PIC_MSTR_DATA_REG, data);
}

void PIC::sendDataToSlave(uint8_t data)
{
	HAL::outportb(PIC_SLAVE_DATA_REG, data);
}

void PIC::sendEOI(uint8_t irqNo)
{	
	//Check for invalid IRQ No.
	if (irqNo > 16)
		return;

	//EOI command
	OCW2 cmdEOI;
	BYTE_SET(cmdEOI, 0);
	cmdEOI.sendEOI = true;
	
	//If IRQ No. is equal to or greater than 8 then send EOI to slave PIC
	if (irqNo >= 8)
		sendCommandToSlave(BYTE_VAL(cmdEOI));
	
	//Send EOI to master PIC anyway
	sendCommandToMaster(BYTE_VAL(cmdEOI));
}

void PIC::init()
{
	//Setting up the ICW1
	ICW1 bfICW1;
	BYTE_SET(bfICW1, 0);
	bfICW1.sendICW4 = true;
	bfICW1.singlePIC = false;
	bfICW1.interruptType = ICW1::EdgeTriggered;
	bfICW1.Initialize = true;
	
	//Send ICW1 to Master & Slave PICs
	sendCommandToMaster(BYTE_VAL(bfICW1));
	sendCommandToSlave(BYTE_VAL(bfICW1));

	//Send ICW2 to Master PIC
	sendDataToMaster(IRQ0);
	
	//Send ICW2 to Slave PIC
	sendDataToSlave(IRQ8);
	
	//Send ICW3 to Master PIC (bit 2) IR2
	sendDataToMaster(MASTER_ICW3);
	
	//Send ICW3 to Slave PIC (2 binary notion) IR2
	sendDataToSlave(SLAVE_ICW3);
	
	//Send ICW4 to Master & Slave PICs (Enable 80/86 Mode)
	sendDataToMaster(ICW4);
	sendDataToSlave(ICW4);
}