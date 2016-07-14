//==========================================
//
//   ZapperOS - CRT Microcontroller Driver
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "crt.h"

#include "hal.h"

using namespace zos;

uint16_t CRT::currentCOLS = 0;
uint16_t CRT::currentROWS = 0;
uint16_t CRT::currentX = 0;
uint16_t CRT::currentY = 0;

void CRT::moveCursor(uint8_t x, uint8_t y)
{
	currentX = x;
	currentY = y;
	
	uint16_t position = x + (y*currentCOLS);
	
	//Send cursor position low-byte command
	HAL::outportb(CRT_INDEX_REG, CRT_CMD_CUR_LO);
	//Send cursor position low-byte
	HAL::outportb(CRT_DATA_REG, (position & 0xFF));
	
	//Send cursor position high-byte command
	HAL::outportb(CRT_INDEX_REG, CRT_CMD_CUR_HI);
	//Send cursor position high-byte
	HAL::outportb(CRT_DATA_REG, ((position >> 0x08) & 0xFF));
}

void CRT::updateCursor()
{
	moveCursor(currentX,currentY);
}