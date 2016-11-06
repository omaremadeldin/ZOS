//==========================================
//
//		      ZapperOS - HAL
//		(Hardware Abstraction Layer)
//==========================================
//Used Acronyms:
//--------------
//* IDT = Interrupt Descriptor Table
//* PIC = Programmable Interrupt Controller
//* PIT = Programmable Interval Timer
//* PMM = Physical Memory Manager
//* VMM = Virtual Memory Manager
//* PCI = Peripheral Component Interconnect
//* IDE = Integrated Drive Electronics
//==========================================
//By Omar Emad Eldin
//==========================================

#include "hal.h"

#include "gdt.h"
#include "idt.h"
#include "exceptions.h"
#include "pic.h"
#include "pit.h"
#include "rtc.h"
#include "kybrd.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "video.h"
#include "pci.h"
#include "ide.h"
#include "pmgr.h"
#include "vmgr.h"

#include "vmodes/vmode7.h"

#include "utils/format.h"
#include "utils/path.h"

#include <string.h>

using namespace zos;

uint32_t HAL::memorySize = 0;

uint8_t HAL::inportb(uint16_t port)
{
	uint8_t result;
	__asm__("in %0, %1":"=a"(result):"Nd"(port));
	return result;
}

uint16_t HAL::inportw(uint16_t port)
{
	uint16_t result;
	__asm__("in %0, %1":"=a"(result):"Nd"(port));
	return result;
}

uint32_t HAL::inportl(uint16_t port)
{
	uint32_t result;
	__asm__("in %0, %1":"=a"(result):"Nd"(port));
	return result;
}

void HAL::insportw(uint16_t port, uint32_t iterations, void* dest)
{
	__asm__("mov ecx,%0"::"g"(iterations):"ecx");
	__asm__("mov dx,%0"::"gw"(port):"edx");
	__asm__("mov edi,%0"::"g"((uint32_t)dest):"edi");
	__asm__("cld");
	__asm__("rep insw");
}

void HAL::outportb(uint16_t port, uint8_t value)
{
	__asm__("out %1,%0": :"a"(value),"Nd"(port));
}

void HAL::outportw(uint16_t port, uint16_t value)
{
	__asm__("out %1,%0": :"a"(value),"Nd"(port));
}

void HAL::outportl(uint16_t port, uint32_t value)
{
	__asm__("out %1,%0": :"a"(value),"Nd"(port));
}

void HAL::outsportw(uint16_t port, uint32_t iterations, void* src)
{
	__asm__("mov ecx,%0"::"g"(iterations):"ecx");
	__asm__("mov dx,%0"::"gw"(port):"edx");
	__asm__("mov esi,%0"::"g"((uint32_t)src):"esi");
	__asm__("cld");
	__asm__("rep outsw");
}

void HAL::debugChar(uint8_t val, bool parallel)
{
	if (parallel)
	{
		//Parallel Port
		outportb(0x37a, 0x04|0x08);
		outportb(0x378, val);
		outportb(0x37a, 0x01);
	}
	else
	{
		//Port 0xE9 Hack
		outportb(0xE9, val);
	}
}

void HAL::debugStr(const char* s)
{
	if (s == NULL)
		return;

	int len = strlen(s);
	for (int i=0; i<len; i++)
		debugChar(s[i]);
}

void HAL::debug(const char* s, ...)
{
	va_list args;
	va_start(args,s);

	char str[256];

	formatWithArgs(s, str, args);

	debugStr(str);

	va_end(args);
}

void HAL::sleep(uint32_t ms)
{
	uint32_t current = systemTimer;

	while (systemTimer < (current + ms));
}

void inline HAL::enableInterrupts()
{
	__asm__("sti");
}

void inline HAL::disableInterrupts()
{
	__asm__("cli");
}

void HAL::halt()
{
	disableInterrupts();
	__asm__("hlt");
}

void HAL::init()
{
	//Global Descriptor Table
	GDT::init();
	debug("GDT setup complete.\n");
	
	//Interrupt Descriptor Table
	IDT::init();
	debug("IDT setup complete.\n");
	
	//Install exception handlers
	Exceptions::init();
	debug("Exception handlers installed.\n");
	
	//Programmable Interrupt Controller	
	PIC::init();
	debug("PIC initialized.\n");
	
	//Programmable Interval Timer
	PIT::init();
	debug("PIT initialized.\n");
	
	//Real Time Clock
	RTC::fetchTime();
	debug("Date: %i-%i-%i, Time: %i:%i:%i.\n", RTC::Year, RTC::Month, RTC::Day, RTC::Hours, RTC::Minutes, RTC::Seconds);

	enableInterrupts();
	debug("Interrupts enabled.\n");
	
	//Keyboard
	KYBRD::init();
	debug("Keyboard initialized.\n");
	
	//Get memory size passed from bootloader
	memorySize = MAKE_DWORD(BOOT_INFO->mem_lower, BOOT_INFO->mem_upper) * 1024;
	
	//Physical Memory Manager
	PMM::init();
	debug("Physical Memory Manager initialized.\n");
	
	//Virtual Memory Manager
	VMM::init();
	debug("Virtual Memory Manager initialized.\n");
	
	//Heap
	Heap::init();
	debug("Heap initialized.\n");
	
	//Constructors Initializing
	debug("Calling Global Constructors...\n");
	ctorsInit();
	
	debug("Selecting & Initializing Video Mode...\n");
	Video::selectVideoMode(new vmodes::VMode7);
	Video::clearScreen();
	
	debug("%i MBs of memory detected, %i MBs free to use.\n", \
		memorySize/1024/1024, (PMM::getFreeBlocks() * PMM_BLOCK_SIZE)/1024/1024);
	debug("Kernel size is %i KBs.\n", KERNEL_SIZE/1024);
	
	//Scanning PCI Bus for devices
	debug("Scanning PCI Bus for devices... ");
	PCI::scan();
	debug("%i device(s) detected.\n", PCI_DEVICES.length);
	
	LinkedList<PCI::Device*>::Node* n = PCI_DEVICES.head;
	while (n != NULL)
	{
		debug("--Bus:0x%x, Device:0x%x, Function:0x%x, Class:%s, Subclass:%s\n", n->value->bus, n->value->device, n->value->function, n->value->deviceClass->name, n->value->deviceSubclass->name);
		n = n->nextNode;
	}
	
	debug("Detecting IDE devices... ");
	IDE::scan();
	debug("%i device(s) detected.\n", IDE_DEVICES.length);
	
	LinkedList<IDE::Controller::Channel::Device*>::Node* k = IDE_DEVICES.head;
	while (k != NULL)
	{
		if (k->value->deviceType == IDE::Controller::Channel::Device::ATA)
		{
			uint64_t diskSize = (k->value->maxLBA / (1024 * 1024 / IDE_ATA_BYTESPERSECTOR));
			debug("--Type:ATA, Model:'%s', Size:%iMBs\n", k->value->Model, diskSize);
		}
		else
		{
			debug("--Type:ATAPI, Model:'%s'\n", k->value->Model);
		}
		
		k = k->nextNode;
	}
	
	PMGR::scan();
	debug("%i partition(s) detected.\n", PARTITIONS.length);
	
	LinkedList<PMGR::Partition*>::Node* j = PARTITIONS.head;
	while (j != NULL)
	{
		debug("--Bootable:%s, System ID:0x%x\n", (j->value->Bootable ? "Yes":"No"), j->value->systemID);
		j = j->nextNode;
	}

	VMGR::init();
	debug("%i filesystem(s) detected.\n", FILESYSTEMS.length);

	LinkedList<VMGR::Filesystem*>::Node* l = FILESYSTEMS.head;
	while (l != NULL)
	{
		debug("--Name:%s, System ID:0x%x\n", l->value->Name, l->value->systemID);
		l = l->nextNode;
	}
	
	VMGR::mountAll();
	debug("%i volume(s) mounted.\n", VOLUMES.length);
	LinkedList<VMGR::Volume*>::Node* m = VOLUMES.head;
	while (m != NULL)
	{
		debug("--Name:'%s', Online:%s, ID:%i\n", m->value->Name, (m->value->Online ? "Yes":"No"), m->value->ID);
		m = m->nextNode;
	}

	debug("Initializations Finished.\n");
}