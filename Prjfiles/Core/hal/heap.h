//==========================================
//
//		      ZapperOS - Heap
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "vmm.h"

#define HEAP_FREELIST			0xCC000000
#define HEAP_USEDLIST			0xCD000000
#define HEAP_BASEADDRESS		0xCE000000

namespace zos
{
	class Heap
	{
	private:
		struct Chunk
		{
			VirtualAddress	Address;		//Base address
			uint32_t		Length;			//Length in no. of blocks
		}__attribute__((packed));

	private:
		static uint32_t pageCount;			//Pages for the heap itself
		static Chunk* freeList;
		static uint32_t freeListSize;
		static uint32_t freeListMaxSize;
		static uint32_t freeListPageCount;
		static Chunk* usedList;	
		static uint32_t usedListSize;
		static uint32_t usedListMaxSize;
		static uint32_t usedListPageCount;
		static Chunk* largestFreeChunk;
		
	private:		
		static void shiftFreeListRight(int32_t start);
		static void shiftFreeListLeft(uint32_t start);
		static bool isChunkInvalid(Chunk* chunk);
		static void removeFreeChunk(Chunk* chunk);
		static void removeUsedChunk(Chunk* chunk);
		static Chunk* addFreeChunk(VirtualAddress address, uint32_t length);
		static Chunk* addUsedChunk(VirtualAddress address, uint32_t length);
		static Chunk* findFreeChunkContaining(VirtualAddress address);
		static Chunk* findUsedChunk(VirtualAddress address);
		static void setRegionUsed(VirtualAddress address, uint32_t size);
		static void setRegionUnused(VirtualAddress address, uint32_t size);
		
	private:
		friend void* ::operator new(size_t objectSize);
		friend void ::operator delete(void* p);
		friend void* ::operator new[](size_t arraySize);
		friend void ::operator delete[](void* p);
		
	public:		
		static void init();
	};
}