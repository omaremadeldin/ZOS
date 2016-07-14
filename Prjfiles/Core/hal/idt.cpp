//==========================================
//
//	     	   ZapperOS - IDT
//		(Interrupt Descriptor Table)
//==========================================
//By Omar Emad Eldin
//==========================================

#include "idt.h"

#include "hal.h"
#include "video.h"

#include <string.h>

using namespace zos;

assembly_stub void aihDefault(void);
interrupt_handler void ihDefault(void)
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**");
	for (;;);
}

IDT::Entry IDT::IDTEntries[MAX_INTERRUPTS];
IDT::Register IDT::IDTR;

void IDT::installHandler(void* handler, int index, uint8_t PRVLG)
{
	if (handler == NULL)
		return;

	memset((void*)&IDTEntries[index], 0, sizeof(struct Entry));
	
	IDTEntries[index].offsetLO = LOWORD((uint32_t)handler);
	IDTEntries[index].Selector = RING0_CODE_SELECTOR;
	IDTEntries[index].gateType = Entry::Descriptor32Bit;
	IDTEntries[index].storageSegment = 0;
	IDTEntries[index].PRVLGLevel = PRVLG;
	IDTEntries[index].Present = true;
	IDTEntries[index].offsetHI = HIWORD((uint32_t)handler);
}

void IDT::init()
{	
	for (int i=0; i<MAX_INTERRUPTS; i++)
		installHandler((void*)&aihDefault, i);
	
	IDTR.IDTSize = sizeof(IDTEntries);
	IDTR.IDTOffset = (uint32_t)&IDTEntries;
	
	__asm__("mov eax,%0"::"g"(&IDTR):"eax");
	__asm__("lidt [eax]":::"eax");
}