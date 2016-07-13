//==========================================
//
//	     	   ZapperOS - GDT
//		(General Descriptor Table)
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "hal.h"

#define GDT_ENTRIES	6

namespace zos
{
	assembly_stub void gdtFlush();
	
	class GDT
	{
	public:
		struct Entry
		{
			enum RWEBit
			{
				DataReadOnly 	= 0,
				DataReadWrite	= 1,
				CodeExecuteOnly	= 0,
				CodeExecuteRead	= 1
			};

			enum DescriptorType
			{
				SystemDescriptor		= 0,
				CodeOrDataDescriptor 	= 1
			};

			enum DCBit
			{
				DataGrowsUp			= 0,
				DataGrowsDown		= 1,
				CodeConformingOff	= 0,
				CodeConformingOn	= 1
			};

			enum SegmentSize
			{
				Segment16Bit	= 0,
				Segment32Bit	= 1
			};

			uint16_t		limitLO				:16;	//Segment limit lower 16-Bits
			uint16_t		baseLO				:16;	//Base address lower 16-Bits
			uint8_t			baseMID				:8;		//Base address middle 8-Bits
			uint8_t			Access				:1;		//Access Bit (Used with virtual memory) 
			RWEBit			RWEFlag				:1;		//Read Write Execute Flag
			DCBit			DirectionConforming	:1;		//If Data then direction of growth, If Code then whether its conforming or not
			bool			Executable			:1;		//Executable ?
			DescriptorType	descriptorType		:1;		//Normally its a 'CodeOrDataDescriptor'
			uint8_t			PRVLGLevel			:2;		//Privilege level (Ring No.) (0...3)
			bool			Present				:1;		//Present in memory ?
			uint8_t			limitHI				:4;		//Segment limit higher 4-Bits
			uint8_t			Reserved			:2;		//Reserved (Always 0)
			SegmentSize		segmentSize 		:1;		//16-Bit/32-Bit Segment
			bool			Granularity4K		:1;		//1B/4KB Granularity
			uint8_t			baseHI				:8;		//Base address higher 8-Bits
		}__attribute__((packed));
		
		struct Register
		{
			uint16_t Limit;
			uint32_t Base;
		}__attribute__((packed));
	
	private:
		static Entry GDTDescriptors[GDT_ENTRIES];
		static Register GDTR;
		
	public:
		static void installDescriptor(Entry descriptor, uint8_t index);			
		static void init();
	};
}