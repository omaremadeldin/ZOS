//==========================================
//
//	    ZapperOS - Standard Arguments
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

//Variable length argument list
typedef unsigned char* va_list;

//Stack item size is same as integer size
#define STACKITEM	int

#define VA_SIZE(TYPE) ((sizeof(TYPE) + sizeof(STACKITEM) - 1) & ~(sizeof(STACKITEM) - 1))
			
#define va_start(AP, LASTARG) (AP = ((va_list)&(LASTARG) + VA_SIZE(LASTARG)))

#define va_arg(AP, TYPE) (AP += VA_SIZE(TYPE), *((TYPE*)(AP - VA_SIZE(TYPE))))

#define va_end(AP)