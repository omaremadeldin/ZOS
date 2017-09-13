//==========================================
//
//		      ZapperOS - HAL
//		(Hardware Abstraction Layer)
//==========================================
//Used Acronyms:
//--------------
//* GDT  = Global Descriptor Table
//* IDT  = Interrupt Descriptor Table
//* PIC  = Programmable Interrupt Controller
//* PIT  = Programmable Interval Timer
//* RTC  = Real Time Clock
//* PMM  = Physical Memory Manager
//* VMM  = Virtual Memory Manager
//* PCI  = Peripheral Component Interconnect
//* IDE  = Integrated Drive Electronics
//* PMGR = Partition Manager
//* VMGR = Volume Manager
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
		static uint32_t systemTimer;
		
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
		static uint32_t getUsedKernelHeap();
		static uint32_t getFreeKernelHeap();
		
	public:
		struct DateTime
		{
			uint16_t	Year;
			uint8_t		Month;
			uint8_t		Day;

			uint8_t		Hours;
			uint8_t		Minutes;
			uint8_t		Seconds;
		}__attribute__((packed));

	//Miscellaneous Functions
	public:
		static uint32_t random(uint32_t max);
		static void sleep(uint32_t ms);
		static DateTime getCurrentDateTime(); 
		static void inline enableInterrupts();
		static void inline disableInterrupts();
		static void halt();

	public:
		//Initializes HAL and all its child devices
		static void init();
	};
}