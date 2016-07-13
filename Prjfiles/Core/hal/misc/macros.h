//==========================================
//
//		      ZapperOS - Macros
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#define assembly_stub 		extern "C"
#define interrupt_handler 	extern "C"

#define MAKE_DWORD(x,y)		((((uint32_t)y) << 16) | ((uint32_t)x))

#define HIWORD(x) 			((x >> 16) & 0xFFFF)
#define LOWORD(x) 			(x & 0xFFFF)

#define HIBYTE(x) 			((x >> 8) & 0xFF)
#define LOBYTE(x) 			(x & 0xFF)

#define BYTE_VAL(x) 		(*((uint8_t*)&x))
#define BYTE_SET(x,y)		(*((uint8_t*)&x) = y)

#define DWORD_VAL(x) 		(*((uint32_t*)&x))
#define DWORD_SET(x,y)		(*((uint32_t*)&x) = y)