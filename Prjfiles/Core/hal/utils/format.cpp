//==========================================
//
//	   ZapperOS - Format String Utility
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "format.h"

#include <string.h>

char* itoa(int32_t i)
{
	//Room for 19 digits, - and '\0'
	static char buf[19 + 2];
	//Points to terminating '\0'
	char *p = buf + 19 + 1;
	
	if (i >= 0)
	{
		do
		{
			*--p = '0' + (i % 10);
			i /= 10;
		}
		while (i != 0);
		
		return p;
	}
	else {			/* i < 0 */
		do
		{
			*--p = '0' - (i % 10);
			i /= 10;
		}
		while (i != 0);
		*--p = '-';
	}
	
	return p;
}

char* xtoa(uint32_t i)
{
  	//Room for 19 digits and '\0'
  	static char buf[19 + 1];
	//Points to terminating '\0'
  	char *p = buf + 20;	
	
	do {
	  *--p = (((i % 16) > 9) ? ('A' + (i % 16) - 10) : ('0' + (i % 16)));
	  i /= 16;
	} while (i != 0);
	return p;
}

void formatWithArgs(const char* s, char* dst, va_list args)
{
	if (s == NULL)
		return;
	
	int j = 0;
	
	int length = strlen(s);
	
	for (int i=0; i<length; i++)
	{
		if (s[i] == '%')
		{
			switch (s[i+1])
			{
				case '%':
				{
					dst[j++] = '%';
					break;
				}
				case 'c':
				{
					char c = va_arg(args, char);
					dst[j++] = c;
					break;
				}
				case 's':
				{
					char* val = va_arg(args, char*);
					
					int ln = strlen(val);
					for (int k=0; k<ln; k++)
						dst[j++] = val[k];
					
					break;
				}
				case 'd':
				case 'i':
				{
					uint32_t val = va_arg(args, int32_t);
					char* str = itoa(val);
					
					int ln = strlen(str);
					for (int k=0; k<ln; k++)
						dst[j++] = str[k];
					
					break;
				}
				case 'x':
				{
					uint32_t val = va_arg(args, uint32_t);
					char* str = xtoa(val);
					
					int ln = strlen(str);
					for (int k=0; k<ln; k++)
						dst[j++] = str[k];
					
					break;
				}
				case 'b':
				{
					bool b = va_arg(args, bool);
					char strT[] = "true";
					char strF[] = "false";
					
					char* str = (b) ? strT : strF;
					
					int ln = strlen(str);
					for (int k=0; k<ln; k++)
						dst[j++] = str[k];
					
					break;
				}
				default:
				{
					break;
				}
			}
			
			i++;
		}
		else
		{
			dst[j++] = s[i];
		}
	}
	
	dst[j] = '\0';
}

void format(const char* s, char* dst, ...)
{
	if (s == NULL)
		return;
	
	va_list args;
	va_start(args,dst);
	
	formatWithArgs(s, dst, args);
	
	va_end(args);
}