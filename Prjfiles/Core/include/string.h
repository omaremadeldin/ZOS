//==========================================
//
//	  ZapperOS - Standard String Library
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stddef.h>

#ifdef __cplusplus
	extern "C"
	{
#endif

//Copying

void* memcpy(void* destination, const void* source, size_t num);
//TODO:void* memmove(void* destination, const void* source, size_t num);
char* strcpy(char* destination, const char* source);
char* strncpy(char* destination, const char* source, size_t num);

//Concatenation

//TODO:char* strcat (char* destination, const char* source);
//TODO:char* strncat (char* destination, const char* source, size_t num);

//Comparison

//TODO:int memcmp(const void* ptr1, const void* ptr2, size_t num);
int strcmp(const char* str1, const char* str2);

//Searching
char* strtok(char* str, const char* delimeters);

//Others
void* memset(void* ptr, int value, size_t num);
size_t strlen(const char* s);

//ZOS Extensions
size_t strtrim(char* s);
void strtolower(char* s);
void strtoupper(char* s);
void strreverse(char* s);

#ifdef __cplusplus
	}
#endif