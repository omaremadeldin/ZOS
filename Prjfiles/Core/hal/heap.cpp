//==========================================
//
//		      ZapperOS - Heap
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "heap.h"

#include "exceptions.h"

#include <string.h>

using namespace zos;

uint32_t Heap::pageCount = 0;
Heap::Chunk* Heap::freeList = NULL;
uint32_t Heap::freeListSize = 0;
uint32_t Heap::freeListMaxSize = 0;
uint32_t Heap::freeListPageCount = 0;
Heap::Chunk* Heap::usedList = NULL;
uint32_t Heap::usedListSize = 0;
uint32_t Heap::usedListMaxSize = 0;
uint32_t Heap::usedListPageCount = 0;

void Heap::shiftFreeListRight(int32_t start)
{
	for (int32_t i = (freeListSize - 1); i >= start; i--)
		freeList[i + 1] = freeList[i];
}

void Heap::shiftFreeListLeft(uint32_t start)
{
	for (uint32_t i = start; i < (freeListSize - 1); i++)
		freeList[i] = freeList[i + 1];
}

bool Heap::isChunkInvalid(Chunk* chunk)
{
	if (chunk == NULL)
		return true;

	if (chunk->Length == 0)
		return true;

	return false;
}

void Heap::removeFreeChunk(Heap::Chunk* chunk)
{
	if (freeListSize == 0)
		return;

	uint32_t pos = 0;

	while ((pos < freeListSize) && (freeList[pos].Address != chunk->Address))
		pos++;

	shiftFreeListLeft(pos);

	freeListSize--;
}

void Heap::removeUsedChunk(Heap::Chunk* chunk)
{
	chunk->Address = NULL;
	chunk->Length = 0;

	usedListSize--;
}

Heap::Chunk* Heap::addFreeChunk(VirtualAddress address, uint32_t length)
{
	if (freeListSize == freeListMaxSize)
	{
		if (VMM::alloc(HEAP_FREELIST + (freeListPageCount * VMM_PAGE_SIZE), VMM_PAGE_SIZE) == NULL)
		{
			Exceptions::throwException("Heap Exception", "Heap::addFreeChunk(VirtualAddress, uint32_t)[1]: Cannot expand the heap free list");
			return NULL;
		}

		memset((void*)(HEAP_FREELIST + (freeListPageCount * VMM_PAGE_SIZE)), 0, VMM_PAGE_SIZE);

		freeListPageCount++;
		freeListMaxSize = (freeListPageCount * VMM_PAGE_SIZE) / sizeof(Chunk);
	}

	uint32_t pos = 0;

	if (freeListSize != 0)
	{
		while ((pos < freeListSize) && (freeList[pos].Address < address))
			pos++;

		shiftFreeListRight(pos);
	}

	freeList[pos].Address = address;
	freeList[pos].Length = length;

	freeListSize++;

	return &freeList[pos];
}

Heap::Chunk* Heap::addUsedChunk(VirtualAddress address, uint32_t length)
{
	if (usedListSize == usedListMaxSize)
	{
		if (VMM::alloc(HEAP_USEDLIST + (usedListPageCount * VMM_PAGE_SIZE), VMM_PAGE_SIZE) == NULL)
		{
			Exceptions::throwException("Heap Exception", "Heap::addUsedChunk(VirtualAddress, uint32_t)[1]: Cannot expand the heap used list");
			return NULL;
		}

		memset((void*)(HEAP_USEDLIST + (usedListPageCount * VMM_PAGE_SIZE)), 0, VMM_PAGE_SIZE);

		usedListPageCount++;
		usedListMaxSize = (usedListPageCount * VMM_PAGE_SIZE) / sizeof(Chunk);
	}

	Chunk* chunk = usedList;

	while (!isChunkInvalid(chunk))	//Loop till we find an empty chunk
		chunk++;

	chunk->Address = address;
	chunk->Length = length;

	usedListSize++;

	return chunk;
}

Heap::Chunk* Heap::findFreeChunkContaining(VirtualAddress address)
{
	Chunk* chunk = freeList;

	for (uint32_t i = 0, count = 0; (i < freeListMaxSize) && (count < freeListSize); i++)
	{
		if (isChunkInvalid(&chunk[i]))
			continue;
		else
			count++;

		uint32_t chunkFrom = chunk[i].Address;
		uint32_t chunkTo = chunk[i].Address + chunk[i].Length;

		if (!((chunkTo < address) || (chunkFrom > address)))
			return &chunk[i];
	}

	return NULL;
}

Heap::Chunk* Heap::findUsedChunk(VirtualAddress address)
{
	Chunk* chunk = usedList;

	for (uint32_t i = 0, count = 0; (i < usedListMaxSize) && (count < usedListSize); i++)
	{
		if (isChunkInvalid(&chunk[i]))
			continue;
		else
			count++;

		if (chunk[i].Address == address)
			return &chunk[i];
	}

	return NULL;
}

void Heap::setRegionUsed(VirtualAddress address, uint32_t size)
{
	VirtualAddress from = address;
	VirtualAddress to = address + size;

	Chunk* chunk = findFreeChunkContaining(from);

	if ((chunk == NULL) || (chunk != findFreeChunkContaining(to)))
	{
		Exceptions::throwException("Heap Exception", "Heap::setRegionUsed()[1]: All or part of the region is already used.");
		return;
	}
	
	if ((chunk->Address == address) && (chunk->Length == size))
	{
		removeFreeChunk(chunk);
	}
	else if ((chunk->Address == address) && (chunk->Length > size))
	{
		chunk->Address += size;
		chunk->Length -= size;
	}
	else
	{
		Exceptions::throwException("Heap Exception", "Heap::setRegionUsed()[2]: Unknown region setting.");
	}
}

void Heap::setRegionUnused(VirtualAddress address, uint32_t size)
{
	VirtualAddress from = address;
	VirtualAddress to = address + size;

	Chunk* chunkFrom = findFreeChunkContaining(from);
	Chunk* chunkTo = findFreeChunkContaining(to);

	if ((chunkFrom == NULL) && (chunkTo == NULL))
	{
		addFreeChunk(address, size);
	}
	else if ((chunkFrom != NULL) && (chunkTo == NULL))
	{
		chunkFrom->Length += size;
	}
	else if ((chunkFrom == NULL) && (chunkTo != NULL))
	{
		chunkTo->Address = from;
		chunkTo->Length += size;
	}
	else if ((chunkFrom != NULL) && (chunkTo != NULL) && (chunkFrom != chunkTo))
	{
		chunkFrom->Length += size + chunkTo->Length;
		removeFreeChunk(chunkTo);
	}
}

void* operator new(size_t objectSize)
{
	if (objectSize == 0)
		return NULL;

	Heap::Chunk* chunk = Heap::freeList;

	for (uint32_t i = 0, count = 0; (i < Heap::freeListMaxSize) && (count < Heap::freeListSize); i++)
	{
		if (Heap::isChunkInvalid(&chunk[i]))
			continue;
		else
			count++;

		if (chunk[i].Length >= objectSize)
		{
			VirtualAddress address = chunk[i].Address;

			Heap::setRegionUsed(address, objectSize);
			Heap::addUsedChunk(address, objectSize);

			return (void*)address;
		}
	}

	uint32_t pagesNeeded = ((objectSize + (VMM_PAGE_SIZE - 1)) / VMM_PAGE_SIZE);

	if (VMM::alloc(HEAP_BASEADDRESS + (Heap::pageCount * VMM_PAGE_SIZE), (pagesNeeded * VMM_PAGE_SIZE)) == NULL)
	{
		Exceptions::throwException("Heap Exception", "operator new(size_t)[1]: Cannot expand heap space");
		return NULL;
	}

	Heap::setRegionUnused(HEAP_BASEADDRESS + (Heap::pageCount * VMM_PAGE_SIZE), (pagesNeeded * VMM_PAGE_SIZE));
	Heap::pageCount += pagesNeeded;

	return operator new(objectSize);
}

void operator delete(void* p)
{
	if (p == NULL)
		return;

	VirtualAddress address = (VirtualAddress)p;

	Heap::Chunk* chunk = Heap::findUsedChunk(address);

	if (chunk == NULL)
	{
		Exceptions::throwException("Heap Exception", "operator delete(void*)[1]: Deleting invalid address");
		return;
	}
	
	uint32_t length = chunk->Length;

	Heap::removeUsedChunk(chunk);
	Heap::setRegionUnused(address, length);
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
	if (VMM::alloc(HEAP_BASEADDRESS, VMM_PAGE_SIZE) == NULL)
	{
		Exceptions::throwException("Heap Exception", "Heap::init()[1]: Cannot map the heap base address");
		return;
	}

	pageCount++;		

	if (VMM::alloc(HEAP_FREELIST, VMM_PAGE_SIZE) == NULL)
	{
		Exceptions::throwException("Heap Exception", "Heap::init()[2]: Cannot map the heap free list");
		return;
	}

	freeList = (Chunk*)HEAP_FREELIST;
	freeListPageCount++;
	freeListMaxSize = (freeListPageCount * VMM_PAGE_SIZE) / sizeof(Chunk);
	memset(freeList, 0, freeListMaxSize * sizeof(Chunk));

	setRegionUnused(HEAP_BASEADDRESS, VMM_PAGE_SIZE);

	if (VMM::alloc(HEAP_USEDLIST, VMM_PAGE_SIZE) == NULL)
	{
		Exceptions::throwException("Heap Exception", "Heap::init()[3]: Cannot map the heap used list");
		return;
	}

	usedList = (Chunk*)HEAP_USEDLIST;
	usedListPageCount++;
	usedListMaxSize = (usedListPageCount * VMM_PAGE_SIZE) / sizeof(Chunk);
	memset(usedList, 0, usedListMaxSize * sizeof(Chunk));
}