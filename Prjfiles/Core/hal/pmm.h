//==========================================
//
//	  ZapperOS - Physical Memory Manager
//
//==========================================
//Used Acronyms:
//--------------
//* PMM = Physical Memory Manager
//* BMM = BIOS Memory Map
//==========================================
//TODO:
//--------------
//* Implement Exceptions
//		-(Deallocating an unallocated address for example)
//* Copy memory map to kernel memory
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#define PMM_BLOCK_SIZE			4096
#define PMM_BLOCKS_PER_BYTE		8

#define PMM_NPOS				-1

namespace zos
{
	//Global Typedefs
	typedef uint32_t PhysicalAddress;
	
	class PMM
	{
	public:
		struct BMMEntry
		{
			enum Type : uint32_t
			{
				Available = 1,
				Reserved = 2,
				ACPIReclaim = 3,
				ACPINVS = 4
			};
			
			uint64_t 		baseAddress;	//Base address of address range
			uint64_t 		Length;			//Length of address range in bytes
			Type 			entryType;		//Entry Type
			uint32_t 		acpi_null;		//Reserved for future use
		}__attribute__((packed));
		
	private:
		static uint8_t* buffer;
		static uint32_t maxBlocks;
		static uint32_t size;
		static uint32_t freeBlocks;
	
	public:
		static uint32_t getFreeBlocks();
		
	private:
		static bool testBlock(uint32_t blockIndex);
		static void setBlockUsed(uint32_t blockIndex);
		static void setBlockUnused(uint32_t blockIndex);
		static void setBlocksUsed(uint32_t blockIndex, size_t num);
		static void setBlocksUnused(uint32_t blockIndex, size_t num);
		static void setRegionUsed(PhysicalAddress address, size_t size);
		static void setRegionUnused(PhysicalAddress address, size_t size);
		static int32_t firstFreeBlock();
		static int32_t firstFreeBlocks(size_t num);
		static void processBMM(PhysicalAddress address, uint32_t length);
		
	public:
		static PhysicalAddress alloc();
		static void free(PhysicalAddress address);
		static void init();		
	};
}