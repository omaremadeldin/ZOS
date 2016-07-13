//==========================================
//
//	       ZapperOS - Video Driver
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#define VIDEO_TEXTBUFFER		0xCB0B8000
#define VIDEO_GRAPHICSBUFFER	0xCB0A0000

namespace zos
{
	class Video
	{
	public:
		class VideoMode
		{
			friend class Video;

		private:
			virtual void init() = 0;
		
		public:
			virtual void clearScreen() = 0;
			virtual void printCh(char c) = 0;
		};

	private:
		static VideoMode* currentVideoMode;
	
	public:
		static void selectVideoMode(VideoMode* vmode);
		static void clearScreen();
		static void printCh(char c);
		static void print(const char* s);
		static void printf(const char* s, ...);
	};
}