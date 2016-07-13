//==========================================
//
//	       ZapperOS - Video Driver
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "video.h"

#include "crt.h"

#include "utils/format.h"

using namespace zos;

Video::VideoMode* Video::currentVideoMode = NULL;

void Video::selectVideoMode(Video::VideoMode* vmode)
{
	if (vmode == NULL)
		return;

	currentVideoMode = vmode;
	currentVideoMode->init();
}

void Video::clearScreen()
{
	currentVideoMode->clearScreen();
}

void Video::printCh(char c)
{
	currentVideoMode->printCh(c);
}

void Video::print(const char* s)
{
	if (s == NULL)
		return;
	
	int i = 0;
	
	while (s[i] != 0)
		printCh(s[i++]);
	
	CRT::updateCursor();
}

void Video::printf(const char* s, ...)
{
	if (s == NULL)
		return;
	
	va_list args;
	va_start(args,s);
	
	char str[256];
	formatWithArgs(s, str, args);
	print(str);

	va_end(args);
}