//==========================================
//
//	     	   ZapperOS - IDT
//		(Interrupt Descriptor Table)
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#define MAX_INTERRUPTS	256

namespace zos
{
	class IDT
	{
	private:
		struct Entry
		{
			enum GateType
			{
				Descriptor16Bit = 0b0110,
				Descriptor32Bit = 0b1110
			};
			
			uint16_t 		offsetLO;			//Lower 16-bits of offset
			uint16_t 		Selector;			//Code Selector
			uint8_t  		 				:8;	//Always Zero (reserved)
			GateType 		gateType		:4;	//Gate Type
			uint8_t	 		storageSegment	:1;	//Storage Segment
			uint8_t	 		PRVLGLevel		:2;	//Privilege Level (Ring No.) (0...3)
			bool	 		Present			:1;	//Present ?
			uint16_t 		offsetHI;			//Higher 16-bits of offset
		}__attribute__((packed));
		
		struct Register
		{
			uint16_t IDTSize;
			uint32_t IDTOffset;
		} __attribute__((packed));
		
	private:
		static Entry IDTEntries[MAX_INTERRUPTS];
		static Register IDTR;
		
	public:
		static void installHandler(void* handler, int index, uint8_t PRVLG = 0);
		static void init();
	};
}