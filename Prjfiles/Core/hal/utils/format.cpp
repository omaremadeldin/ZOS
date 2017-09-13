//==========================================
//
//	   ZapperOS - Format String Utility
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "format.h"

#include <string.h>

const char* itoa(int32_t i, uint8_t padding)
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

		int8_t len = strlen(p);

		for (uint8_t i = 0; i < (padding - len); i++)
			*--p = '0';
		
		return p;
	}
	else {			/* i < 0 */
		do
		{
			*--p = '0' - (i % 10);
			i /= 10;
		}
		while (i != 0);

		int8_t len = strlen(p);

		for (uint8_t i = 0; i < (padding - len); i++)
			*--p = '0';

		*--p = '-';
	}
	
	return p;
}

const char* xtoa(uint32_t i, uint8_t padding)
{
  	//Room for 19 digits and '\0'
  	static char buf[19 + 1];
	//Points to terminating '\0'
  	char *p = buf + 20;	
	
	do {
	  *--p = (((i % 16) > 9) ? ('A' + (i % 16) - 10) : ('0' + (i % 16)));
	  i /= 16;
	} while (i != 0);

	int8_t len = strlen(p);

	for (uint8_t i = 0; i < (padding - len); i++)
		*--p = '0';

	return p;
}

void formatWithArgs(const char* s, char* dst, va_list args)
{
	if (s == NULL)
		return;
	
	int j = 0;
	
	int length = strlen(s);
	
	for (int i=0; i < length; i++)
	{
		if (s[i] == '%')
		{
			int k = 1;
			if (isdigit(s[i + k]))
				k++;

			int padding = 0;
			if (k > 1)
				padding = s[i + k - 1] - '0';

			switch (s[i+k])
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
					const char* str = itoa(val, padding);
					
					int ln = strlen(str);
					for (int k=0; k<ln; k++)
						dst[j++] = str[k];
					
					break;
				}
				case 'x':
				{
					uint32_t val = va_arg(args, uint32_t);
					const char* str = xtoa(val, padding);
					
					int ln = strlen(str);
					for (int k=0; k<ln; k++)
						dst[j++] = str[k];
					
					break;
				}
				case 'g':
				{
					uint32_t val = va_arg(args, int32_t);

					uint8_t count = 0;
					while ((val >= 1024) && (count <= 4))
					{
						val /= 1024;
						count++;
					}

					const char* str = itoa(val);
					
					int ln = strlen(str);
					for (int k=0; k<ln; k++)
						dst[j++] = str[k];

					switch (count)
					{
						case 0:
						{
							dst[j++] = 'B';
							break;
						}
						case 1:
						{
							dst[j++] = 'K';
							dst[j++] = 'B';
							break;
						}
						case 2:
						{
							dst[j++] = 'M';
							dst[j++] = 'B';
							break;
						}
						case 3:
						{
							dst[j++] = 'G';
							dst[j++] = 'B';
							break;
						}
						case 4:
						{
							dst[j++] = 'T';
							dst[j++] = 'B';
							break;
						}
					}
					
					break;
				}
				case 'b':
				{
					bool b = va_arg(args, bool);
					const char strT[] = "true";
					const char strF[] = "false";
					
					const char* str = (b) ? strT : strF;
					
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
			
			i += k;
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