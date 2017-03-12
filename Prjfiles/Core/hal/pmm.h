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

namespace zos
{
	//Global Typedefs
	typedef uint32_t PhysicalAddress;
	
	class PMM
	{
	private:
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
	
	public:
		struct Chunk
		{
			PhysicalAddress	Address;		//Base address
			uint32_t		Length;			//Length in no. of blocks
		}__attribute__((packed));

	private:
		static uint32_t blockCount;
		static Chunk* freeList;
		static uint32_t freeListSize;
		static uint32_t freeListMaxSize;
		static Chunk* usedList;	
		static uint32_t usedListSize;
		static uint32_t usedListMaxSize;

	private:
		static void shiftFreeListRight(int32_t start);
		static void shiftFreeListLeft(uint32_t start);
		static bool isChunkInvalid(Chunk* chunk);
		static void removeFreeChunk(Chunk* chunk);
		static void removeUsedChunk(Chunk* chunk);
		static Chunk* addFreeChunk(PhysicalAddress address, uint32_t length);
		static Chunk* addUsedChunk(PhysicalAddress address, uint32_t length);
		static Chunk* findFreeChunkContaining(PhysicalAddress address);
		static Chunk* findUsedChunk(PhysicalAddress address);
		static void setRegionUsed(PhysicalAddress address, uint32_t size);
		static void setRegionUnused(PhysicalAddress address, uint32_t size);
		static void processBMM(PhysicalAddress address, uint32_t length);
		
	public:
		static uint32_t getFreeMemory();
		static PhysicalAddress alloc(size_t size);
		static void free(PhysicalAddress address);
		static void init();		
	};
}