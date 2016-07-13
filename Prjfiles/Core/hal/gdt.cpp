//==========================================
//
//	     	   ZapperOS - GDT
//		(General Descriptor Table)
//==========================================
//By Omar Emad Eldin
//==========================================

#include "gdt.h"

#include <string.h>

using namespace zos;

GDT::Entry GDT::GDTDescriptors[GDT_ENTRIES];
GDT::Register GDT::GDTR;

void GDT::installDescriptor(Entry descriptor, uint8_t index)
{
	if (index >= GDT_ENTRIES)
		return;
	
	GDTDescriptors[index] = descriptor;
}

void GDT::init()
{
	//Zero out our GDT first
	memset(&GDTDescriptors[0], 0, (sizeof(struct Entry) * GDT_ENTRIES));
	
	Entry Ring0CodeDescriptor;
	Ring0CodeDescriptor.limitLO = 0xFFFF;
	Ring0CodeDescriptor.limitHI = 0xF;
	Ring0CodeDescriptor.baseLO = 0x0000;
	Ring0CodeDescriptor.baseMID = 0x00;
	Ring0CodeDescriptor.baseHI = 0x00;
	Ring0CodeDescriptor.Access = false;
	Ring0CodeDescriptor.Present = true;
	Ring0CodeDescriptor.RWEFlag = Entry::CodeExecuteRead;
	Ring0CodeDescriptor.DirectionConforming = Entry::CodeConformingOff;
	Ring0CodeDescriptor.Executable = true;
	Ring0CodeDescriptor.descriptorType = Entry::CodeOrDataDescriptor;
	Ring0CodeDescriptor.PRVLGLevel = 0;
	Ring0CodeDescriptor.segmentSize = Entry::Segment32Bit;
	Ring0CodeDescriptor.Granularity4K = true;
	Ring0CodeDescriptor.Reserved = 0;
	installDescriptor(Ring0CodeDescriptor, 1);

	Entry Ring0DataDescriptor;
	Ring0DataDescriptor.limitLO = 0xFFFF;
	Ring0DataDescriptor.limitHI = 0xF;
	Ring0DataDescriptor.baseLO = 0x0000;
	Ring0DataDescriptor.baseMID = 0x00;
	Ring0DataDescriptor.baseHI = 0x00;
	Ring0DataDescriptor.Access = false;
	Ring0DataDescriptor.Present = true;
	Ring0DataDescriptor.RWEFlag = Entry::DataReadWrite;
	Ring0DataDescriptor.DirectionConforming = Entry::DataGrowsUp;
	Ring0DataDescriptor.Executable = false;
	Ring0DataDescriptor.descriptorType = Entry::CodeOrDataDescriptor;
	Ring0DataDescriptor.PRVLGLevel = 0;
	Ring0DataDescriptor.segmentSize = Entry::Segment32Bit;
	Ring0DataDescriptor.Granularity4K = true;
	Ring0DataDescriptor.Reserved = 0;
	installDescriptor(Ring0DataDescriptor, 2);

	Entry Ring3CodeDescriptor;
	Ring3CodeDescriptor.limitLO = 0xFFFF;
	Ring3CodeDescriptor.limitHI = 0xF;
	Ring3CodeDescriptor.baseLO = 0x0000;
	Ring3CodeDescriptor.baseMID = 0x00;
	Ring3CodeDescriptor.baseHI = 0x00;
	Ring3CodeDescriptor.Access = false;
	Ring3CodeDescriptor.Present = true;
	Ring3CodeDescriptor.RWEFlag = Entry::CodeExecuteRead;
	Ring3CodeDescriptor.DirectionConforming = Entry::CodeConformingOff;
	Ring3CodeDescriptor.Executable = true;
	Ring3CodeDescriptor.descriptorType = Entry::CodeOrDataDescriptor;
	Ring3CodeDescriptor.PRVLGLevel = 3;
	Ring3CodeDescriptor.segmentSize = Entry::Segment32Bit;
	Ring3CodeDescriptor.Granularity4K = true;
	Ring3CodeDescriptor.Reserved = 0;
	installDescriptor(Ring3CodeDescriptor, 3);

	Entry Ring3DataDescriptor;
	Ring3DataDescriptor.limitLO = 0xFFFF;
	Ring3DataDescriptor.limitHI = 0xF;
	Ring3DataDescriptor.baseLO = 0x0000;
	Ring3DataDescriptor.baseMID = 0x00;
	Ring3DataDescriptor.baseHI = 0x00;
	Ring3DataDescriptor.Access = false;
	Ring3DataDescriptor.Present = true;
	Ring3DataDescriptor.RWEFlag = Entry::DataReadWrite;
	Ring3DataDescriptor.DirectionConforming = Entry::DataGrowsUp;
	Ring3DataDescriptor.Executable = false;
	Ring3DataDescriptor.descriptorType = Entry::CodeOrDataDescriptor;
	Ring3DataDescriptor.PRVLGLevel = 3;
	Ring3DataDescriptor.segmentSize = Entry::Segment32Bit;
	Ring3DataDescriptor.Granularity4K = true;
	Ring3DataDescriptor.Reserved = 0;
	installDescriptor(Ring3DataDescriptor, 4);

	GDTR.Limit = (sizeof(struct Entry) * GDT_ENTRIES - 1);
	GDTR.Base = ((uint32_t)&GDTDescriptors[0]);

	__asm__("lgdt %0" : "=m" (GDTR));

	gdtFlush();
}