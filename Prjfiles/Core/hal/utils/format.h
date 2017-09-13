//==========================================
//
//	   ZapperOS - Format String Utility
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

const char* itoa(int32_t i, uint8_t padding = 0);

const char* xtoa(uint32_t i, uint8_t padding = 0);

void formatWithArgs(const char* s, char* dst, va_list args);

void format(const char* s, char* dst, ...);