//==========================================
//
//	  ZapperOS - Video Driver - Mode 0x07
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "../video.h"

#define VMODE7_MEMORY			0xB8000
#define VMODE7_COLS				80
#define VMODE7_ROWS				25
#define VMODE7_DEFAULT_ATTRIB	0x1F

namespace zos
{
	namespace vmodes
	{
		class VMode7 : public Video::VideoMode
		{
		private:
			uint16_t* buffer;
			
		private:
			void init();
		
		public:
			void clearScreen();
			void printCh(char c);
		};
	}
}