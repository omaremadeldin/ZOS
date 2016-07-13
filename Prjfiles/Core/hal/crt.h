//==========================================
//
//   ZapperOS - CRT Microcontroller Driver
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "video.h"

#define CRT_INDEX_REG	0x3D4
#define CRT_DATA_REG	0x3D5

#define CRT_CMD_CUR_HI	0xE		//Cursor position high-byte
#define CRT_CMD_CUR_LO	0xF		//Cursor position low-byte 

namespace zos
{
	class CRT
	{		
	public:
		static uint16_t currentCOLS;
		static uint16_t currentROWS;
		static uint16_t currentX;
		static uint16_t currentY;
		
	public:
		static void moveCursor(uint8_t x, uint8_t y);
		static void updateCursor();
	};
}