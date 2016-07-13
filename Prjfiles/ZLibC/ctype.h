//==========================================
//
//		  ZapperOS - Character Type
//
//==========================================
//http://www.cplusplus.com/reference/cctype/
//http://www.asciitable.com/
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#define _CTYPE_CASE		0b00100000		//Uppercase/Lowercase control bit

#ifdef __cplusplus
	extern "C"
	{
#endif

int isalpha(int c);

int isdigit(int c);

int isalnum(int c);

int iscntrl(int c);

int isgraph(int c);

int islower(int c);

int isupper(int c);

int isprint(int c);
	
int ispunct(int c);

int isspace(int c);

int isxdigit(int c);

int isascii(int c);

int tolower(int c);

int toupper(int c);

#ifdef __cplusplus
	}
#endif