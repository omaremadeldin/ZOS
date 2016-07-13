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

#include "ctype.h"

int isalpha(int c)
{
	return (((c >= 'A') && (c <= 'Z')) ||
			((c >= 'a') && (c <= 'z')));
}

int isdigit(int c)
{
	return ((c >= '0') && (c <= '9'));
}

int isalnum(int c)
{
	return (isalpha(c)||isdigit(c));
}

int iscntrl(int c)
{
	//ASCII from 0x00 to 0x1F + 0x7F:
	//Control codes
	//Tab
	//White-space control
	//Other control codes
	//DEL intacter
	return (((c >= 0x00) && (c <= 0x1F)) || 0x7F);
}

int isgraph(int c)
{
	//ASCII codes from 0x21 to 0x7E
	//All symbols
	//Numbers
	//Alphabetics
	return ((c >= 0x21) && (c <= 0x7E));
}

int islower(int c)
{
	return ((c >= 'a') && (c <= 'z'));
}

int isupper(int c)
{
	return ((c >= 'A') && (c <= 'Z'));
}

int isprint(int c)
{
	//ASCII codes from 0x20 to 0x7E
	//Space
	//All symbols
	//Numbers
	//Alphabetics
	return ((c >= 0x20) && (c <= 0x7E));
}
	
int ispunct(int c)
{
	return (((c >= '!') && (c <= '/')) ||
			((c >= ':') && (c <= '@')) || 
			((c >= '[') && (c <= '`')) || 
			((c >= '{') && (c <= '~')));
}

int isspace(int c)
{
	//ASCII codes from 0x09 to 0x0D + 0x20
	//Tab
	//White-space control codes
	//Space
	return (((c >= 0x09) && (c <= 0x0D)) || (c == 0x20));
}

int isxdigit(int c)
{
	return (((c >= '0') && (c <= '9')) ||
			((c >= 'A') && (c <= 'F')) ||
			((c >= 'a') && (c <= 'f')));
}

int isascii(int c)
{
	return (c <= 0x7F);
}

int tolower(int c)
{
	if (isupper(c))
		return (c + ('a' - 'A'));
	else
		return c;
}

int toupper(int c)
{
	if (islower(c))
		return (c - ('a' - 'A'));
	else
		return c;
}