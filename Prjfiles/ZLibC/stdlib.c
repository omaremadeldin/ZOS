//==========================================
//
//	    ZapperOS - C++ Standard Library
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "stdlib.h"

//String conversion

int atoi(const char* str)
{
	int result = 0;
	unsigned int digit;
	int negative;

	while (isspace(str[0]))
		str++;

	if (str[0] == '-')
	{
		negative = 1;
		str++;
	}
	else
	{
		negative = 0;
		if (str[0] == '+')
			str++;
	}

	while (str[0] != '\0')
	{
		digit = str[0] - '0';

		if (digit > 9)
			break;

		result = (result * 10) + digit;

		str++;
	}

	if (negative)
		return -result;
	else
		return result;
}