//==========================================
//
//		      ZapperOS - Heap
//
//==========================================
//By Omar Emad Eldin
//==========================================

//TODO:Check loops so they can't be larger that max block
//TODO:Review usage of Heap::size

#include "heap.h"

#include "vmm.h"

#include <string.h>

using namespace zos;

uint32_t Heap::pages = 0;
uint8_t* Heap::buffer = NULL;
uint32_t Heap::maxBlocks = 0;
uint32_t Heap::size = 0;
uint32_t Heap::freeBlocks = 0;

bool Heap::testBlock(uint32_t blockIndex)
{
	return ((buffer[(blockIndex / HEAP_BLOCKS_PER_BYTE)] & (1 << (blockIndex % HEAP_BLOCKS_PER_BYTE))) != 0);
}

void Heap::setBlockUsed(uint32_t blockIndex)
{
	buffer[(blockIndex / HEAP_BLOCKS_PER_BYTE)] |= (1 << (blockIndex % HEAP_BLOCKS_PER_BYTE));
	freeBlocks--;
}

void Heap::setBlockUnused(uint32_t blockIndex)
{
	buffer[(blockIndex / HEAP_BLOCKS_PER_BYTE)] &= ~(1 << (blockIndex % HEAP_BLOCKS_PER_BYTE));
	freeBlocks++;
}

void Heap::setBlocksUsed(uint32_t blockIndex, size_t num)
{
	for (uint32_t i = blockIndex; i < (blockIndex + num); i++)
		setBlockUsed(i);
}

void Heap::setBlocksUnused(uint32_t blockIndex, size_t num)
{
	for (uint32_t i = blockIndex; i < (blockIndex + num); i++)
		setBlockUnused(i);
}

int32_t Heap::firstFreeBlock()
{
	for (uint32_t i = 0; i < maxBlocks; i++)
		if (!testBlock(i))
			return i;

	return HEAP_NPOS;
}

int32_t Heap::firstFreeBlocks(size_t num)
{
	uint32_t currentIndex = 0;
	uint32_t currentLength = 0;

	for (uint32_t i = 1; i < maxBlocks; i++)
	{
		if (currentLength == 0)
			currentIndex = i;
	
		if (!testBlock(i))
			currentLength++;
		else
			currentLength = 0;
	
		if (currentLength == num)
			return currentIndex;
	}

	return HEAP_NPOS;
}

void Heap::updateSize(bool increase)
{
	if (increase)
	{
		if (VMM::alloc(HEAP_BASEADDRESS + (pages++ * VMM_PAGE_SIZE)) == NULL)
		{
			return;
			//TODO:Exceptions
			//throwSoftwareFault("HEAP: Cannot increase the heap base address");
		}
		
		maxBlocks += (VMM_PAGE_SIZE / HEAP_BLOCK_SIZE);
		freeBlocks += (VMM_PAGE_SIZE / HEAP_BLOCK_SIZE);
		
		if (pages % HEAP_BLOCKS_PER_BYTE == 0)
		{
			if (VMM::alloc(HEAP_BITMAP + ((pages / HEAP_BLOCKS_PER_BYTE) * VMM_PAGE_SIZE)) == NULL)
			{
				return;
				//TODO:Exceptions
				//throwSoftwareFault("HEAP: Cannot increase the heap bitmap");
			}
			
			memset((void*)(HEAP_BITMAP + ((pages / HEAP_BLOCKS_PER_BYTE) * VMM_PAGE_SIZE)), 0, VMM_PAGE_SIZE / HEAP_BLOCKS_PER_BYTE);
		}
	}
	else
	{
		if (pages == 1)
			return;
		
		VMM::free(HEAP_BASEADDRESS + (pages-- * VMM_PAGE_SIZE));
		maxBlocks = (maxBlocks - (VMM_PAGE_SIZE / HEAP_BLOCK_SIZE));
		
		if (pages % HEAP_BLOCKS_PER_BYTE == 0)
			VMM::free(HEAP_BITMAP + ((pages / HEAP_BLOCKS_PER_BYTE) * VMM_PAGE_SIZE));
	}
}

void* operator new(size_t objectSize)
{	
	if (objectSize > 0xFFFFFFFF)
		return NULL;
	
	if (((Heap::maxBlocks - Heap::freeBlocks) * HEAP_BLOCK_SIZE) + objectSize >= (Heap::pages * VMM_PAGE_SIZE))
		Heap::updateSize(true);
	
	int32_t n = Heap::firstFreeBlocks(objectSize + 4);
	
	if (n == HEAP_NPOS)
		return NULL;
	
	uint32_t* heapBuffer = ((uint32_t*)(HEAP_BASEADDRESS + (n * HEAP_BLOCK_SIZE)));
	heapBuffer[0] = objectSize;
	
	Heap::setBlocksUsed(n, objectSize + 4);
	
	heapBuffer += 4*HEAP_BLOCK_SIZE / (sizeof(uint32_t));
	
	return heapBuffer;
}

void operator delete(void* p)
{
	if (p == NULL)
		return;

	uint32_t* heapBuffer = (uint32_t*)p;
	
	size_t objectSize = heapBuffer[-1];
	
	VirtualAddress address = ((VirtualAddress)p - HEAP_BASEADDRESS - (4 * HEAP_BLOCK_SIZE));
	
	int32_t n = address / HEAP_BLOCK_SIZE;
	
	Heap::setBlocksUnused(n, objectSize + 4);
	
	if (((Heap::maxBlocks - Heap::freeBlocks) * HEAP_BLOCK_SIZE) - objectSize <= (Heap::pages * VMM_PAGE_SIZE))
		Heap::updateSize(false);
}

void* operator new[](size_t arrSize)
{
	return operator new(arrSize);
}

void operator delete[](void* p)
{
	operator delete(p);
}

void Heap::init()
{
	pages = 0;
	
	if (VMM::alloc(HEAP_BITMAP) == NULL)
	{
		return;
		//TODO:Exceptions
		//throwSoftwareFault("HEAP: Cannot map the heap bitmap");
	}
	
	if (VMM::alloc(HEAP_BASEADDRESS + (pages++ * VMM_PAGE_SIZE)) == NULL)
	{
		return;
		//TODO:Exceptions
		//throwSoftwareFault("HEAP: Cannot map the heap base address");
	}
	
	uint32_t maxBlks = (pages * VMM_PAGE_SIZE) / HEAP_BLOCK_SIZE;
	
	buffer = (uint8_t*)HEAP_BITMAP;
	maxBlocks = maxBlks;
	size = (maxBlks / HEAP_BLOCKS_PER_BYTE + ((maxBlks % HEAP_BLOCKS_PER_BYTE != 0) ? 1 : 0));
	freeBlocks = maxBlks;
	
	memset((void*)HEAP_BITMAP, 0, size);
}