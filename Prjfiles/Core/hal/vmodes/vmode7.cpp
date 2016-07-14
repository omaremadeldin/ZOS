//==========================================
//
//	  ZapperOS - Video Driver - Mode 0x07
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "vmode7.h"

#include "../vmm.h"
#include "../crt.h"

using namespace zos;
using namespace zos::vmodes;

void VMode7::init()
{
	CRT::currentCOLS = VMODE7_COLS;
	CRT::currentROWS = VMODE7_ROWS;
	CRT::currentX = 0;
	CRT::currentY = 0;
	
	VMM::mapPage(VMODE7_MEMORY, VIDEO_TEXTBUFFER);
	buffer = (uint16_t*)VIDEO_TEXTBUFFER;
}

void VMode7::clearScreen()
{
	//Fill the screen with white-space to clear it
	const uint16_t ch = (' ' | (VMODE7_DEFAULT_ATTRIB << 8));

	for (uint16_t i=0; i < (VMODE7_COLS * VMODE7_ROWS); i++)
		buffer[i] = ch;
	
	CRT::moveCursor(0,0);
}

void VMode7::printCh(char c)
{
	//If null character then exit
	if (c == '\0')
		return;
	
	//Copy each line up one line
	if (CRT::currentY >= VMODE7_ROWS)
	{
		for (uint8_t i = 0; i < VMODE7_COLS; i++)
			for (uint8_t j = 1; j < VMODE7_ROWS; j++)
				buffer[(i + ((j-1)*VMODE7_COLS))] = buffer[(i + ((j)*VMODE7_COLS))];
		
		for (uint8_t i = 0; i < VMODE7_COLS; i++)
			buffer[(i + ((VMODE7_ROWS-1)*VMODE7_COLS))] = ' ';
		
		CRT::currentY--;
	}
	
	if (c == '\n')
	{
		CRT::currentY++;
		CRT::currentX = 0;
		return;
	}
	else if (c == '\r')
	{
		CRT::currentX = 0; 
		return;
	}
	
	if (!isprint(c))
		return;
	
	uint16_t ch = (c | (VMODE7_DEFAULT_ATTRIB << 8));
	uint32_t position = (CRT::currentX++) + CRT::currentY * VMODE7_COLS;
	
	buffer[position] = ch;
	
	if (CRT::currentX > VMODE7_COLS - 1)
	{
		CRT::currentY++;
		CRT::currentX = 0;
	}
}