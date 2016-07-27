//==========================================
//
//	  ZapperOS - Standard String Library
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "string.h"

//Copying

void* memcpy(void* destination, const void* source, size_t num)
{
	if (destination == source)
		return destination;
	
	unsigned char* d = (unsigned char*)destination;
	unsigned char* s = (unsigned char*)source;
	
	while (num-- != 0)
		*d++ = *s++;
	
	return d;
}

char* strcpy(char* destination, const char* source)
{
	size_t i = 0;
	
	do
		destination[i] = source[i];
	while (source[i++] != '\0');
	
	return destination;
}

char* strncpy(char* destination, const char* source, size_t num)
{
	size_t i = 0;
	
	do
	{
		if (i == num)
			break;
	
		destination[i] = source[i];
	}
	while (source[i++] != '\0');
	
	while (i < num)
		destination[i++] = '\0';
	
	return destination;
}

//Comparison

int strcmp(const char* str1, const char* str2)
{
	for ( ; *str1 == *str2; str1++, str2++)
		if (*str1 == '\0')
			return 0;
    return ((*(unsigned char *)str1 < *(unsigned char *)str2) ? -1 : +1);
}

//Searching

static char* lastStrTok = NULL;
char* strtok(char* str, const char* delimeters)
{
	if (str == NULL && lastStrTok == NULL)
		return NULL;

	char* result = (str == NULL ? lastStrTok : str);
	if (str != NULL)
		lastStrTok = NULL;

	int srcLength = strlen(result);
	int delLength = strlen(delimeters);

	if (srcLength == 0 || delLength == 0)
		return result;

	for (int i = 0; i <= srcLength; i++)
	{
		for (int j = 0; j < delLength; j++)
		{
			if (result[i] == '\0')
			{
				lastStrTok = NULL;
				return result;
			}

			if (result[i] == delimeters[j])
			{
				result[i] = '\0';

				if (i > srcLength)
					lastStrTok = NULL;
				else
					lastStrTok = &result[i + 1];

				return result;
			}
		}
	}

	return result;
}

//Others

void* memset(void* ptr, int value, size_t num)
{
	if (ptr == NULL)
		return NULL;
	
	unsigned char* p = (unsigned char*)ptr;
	
	while(num-- != 0)
		*p++ = (unsigned char)value;
	
	return ptr;
}

size_t strlen(const char* s)
{
	if (s == NULL)
		return 0;
	
	size_t i = 0;
	
	while (s[i] != '\0')
		i++;
	
	return i;
}

//ZOS Extensions

size_t strtrim(char* s)
{
	if (s == NULL)
		return 0;

	size_t len = strlen(s);

	if (len == 0)
		return 0;

	size_t start = 0;
	size_t end = len - 1;

	while ((isspace(s[start])) && (start != len))
		start++;
	while ((isspace(s[end])) && (end != 0))
		end--;

	if (end < start)
	{
		s[0] = '\0';
		return 0;
	}
	else
	{
		for (size_t i = start; i < end; i++)
			s[i - start] = s[i];

		s[end + 1] = '\0';

		return (end - start + 1);
	}
}

void strtolower(char* s)
{
	if (s == NULL)
		return;

	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++)
		s[i] = tolower(s[i]);
}

void strtoupper(char* s)
{
	if (s == NULL)
		return;
	
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++)
		s[i] = toupper(s[i]);
}

void strreverse(char* s)
{
	if (s == NULL)
		return;
	
	size_t len = strlen(s);
	size_t i = 0;
	size_t j = len - 1;

	for (i = 0; i < len / 2; i++, j--)
	{
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}