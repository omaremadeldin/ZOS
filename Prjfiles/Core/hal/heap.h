//==========================================
//
//		      ZapperOS - Heap
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#define HEAP_BITMAP				0xCCC00000
#define HEAP_BASEADDRESS		0xCCD00000
#define HEAP_BLOCK_SIZE			1
#define HEAP_BLOCKS_PER_BYTE	8

#define HEAP_NPOS	-1

namespace zos
{
	class Heap
	{
	private:
		static uint32_t pages;
		static uint8_t* buffer;
		static uint32_t maxBlocks;
		static uint32_t size;
		static uint32_t freeBlocks;
		
	private:
		static bool testBlock(uint32_t blockIndex);
		static void setBlockUsed(uint32_t blockIndex);
		static void setBlockUnused(uint32_t blockIndex);
		static void setBlocksUsed(uint32_t blockIndex, size_t num);
		static void setBlocksUnused(uint32_t blockIndex, size_t num);
		static int32_t firstFreeBlock();
		static int32_t firstFreeBlocks(size_t num);
		static void updateSize(bool increase);
		
	private:
		friend void* ::operator new(size_t objectSize);
		friend void ::operator delete(void* p);
		friend void* ::operator new[](size_t arraySize);
		friend void ::operator delete[](void* p);
		
	public:		
		static void init();
	};
}