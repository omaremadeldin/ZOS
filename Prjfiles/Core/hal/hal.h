//==========================================
//
//		      ZapperOS - HAL
//		(Hardware Abstraction Layer)
//==========================================
//Used Acronyms:
//--------------
//* IDT = Interrupt Descriptor Table
//* PIC = Programmable Interrupt Controller
//* PIT = Programmabel Interval Timer
//* PMM = Physical Memory Manager
//* VMM = Virtual Memory Manager
//* PCI = Peripheral Component Interconnect
//* IDE = Integrated Drive Electronics
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "misc/multiboot.h"
#include "misc/macros.h"

//Global Definitions
#define KERNEL_PBASE			0x100000
#define KERNEL_VBASE			0xC0000000

#define RING_USERLAND			0x03

#define RING0_CODE_SELECTOR 	0x08
#define RING0_DATA_SELECTOR		0x10
#define RING3_CODE_SELECTOR 	0x1B
#define RING3_DATA_SELECTOR		0x23

#define DEFAULT_VIDEO_MODE		0x07

//TODO: Make sure all classes use heap after its initialization
//TODO: Re-Implement Bitfields (HAL utils)

namespace zos
{
	//Global Variables
	extern "C" uint32_t KERNEL_SIZE;
	
	extern "C" uint32_t STACK_START;
	extern "C" uint32_t STACK_END;
	
	extern "C" multiboot_info* BOOT_INFO;
	
	//External Functions
	extern "C" void ctorsInit(void);
	
	class HAL
	{
	public:
		static uint32_t memorySize;
	
	//I/O Functions	
	public:
		static uint8_t inportb(uint16_t port);
		static uint16_t inportw(uint16_t port);
		static uint32_t inportl(uint16_t port);
		static void insportw(uint16_t port, uint32_t iterations, void* dest);
		
		static void outportb(uint16_t port, uint8_t value);
		static void outportw(uint16_t port, uint16_t value);
		static void outportl(uint16_t port, uint32_t value);
		static void outsportw(uint16_t port, uint32_t iterations, void* src);
	
	//Debugging Functions	
	private:
		static void debugChar(uint8_t val, bool parallel = false);		
		static void debugStr(const char* s);
	public:
		static void debug(const char* s, ...);
		
	//Miscellaneous Functions
	public:
		static void sleep(uint32_t ms);
		static void inline enableInterrupts();
		static void inline disableInterrupts();
		static void halt();
	
	public:
		//Initializes HAL and all its child devices
		static void init();
	};
}