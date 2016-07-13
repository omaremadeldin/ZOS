//==========================================
//
//   ZapperOS - Standard Integer Typedefs
//
//==========================================
//http://www.cplusplus.com/reference/cstdint/
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

//Different sizes of integers signed & unsigned
typedef signed char 		int8_t;
typedef unsigned char 		uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned            uint32_t;
typedef long long           int64_t;
typedef unsigned long long 	uint64_t;

//Signed Integers max & min numbers
//8-bits
#define INT8_MAX         127
#define INT8_MIN          -128
//16-bits
#define INT16_MAX        32767
#define INT16_MIN         -32768
//32-bits
#define INT32_MAX        2147483647
#define INT32_MIN        (-INT32_MAX-1)
//64-bits
#define INT64_MAX        9223372036854775807LL
#define INT64_MIN        (-INT64_MAX-1)
//Unsigned Integers max numbers
#define UINT8_MAX         255
#define UINT16_MAX        65535
#define UINT32_MAX        4294967295U
#define UINT64_MAX        18446744073709551615ULL

//Macros for defining Integer constants
//Signed
#define INT8_C(v)    (v)
#define INT16_C(v)   (v)
#define INT32_C(v)   (v)
#define INT64_C(v)   (v ## LL)
//Unsigned
#define UINT8_C(v)   (v ## U)
#define UINT16_C(v)  (v ## U)
#define UINT32_C(v)  (v ## U)
#define UINT64_C(v)  (v ## ULL)