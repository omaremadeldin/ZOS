//==========================================
//
//	   ZapperOS - Format String Utility
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

char* itoa(int32_t i);

char* xtoa(uint32_t i);

void formatWithArgs(const char* s, char* dst, va_list args);

void format(const char* s, char* dst, ...);