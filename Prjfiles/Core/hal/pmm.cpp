//==========================================
//
//	  ZapperOS - Physical Memory Manager
//
//==========================================
//Used Acronyms:
//--------------
//* PMM = Physical Memory Manager
//==========================================
//TODO:
//--------------
//* Implement Exceptions
//		-(Deallocating an unallocated address for example)
//* Copy memory map to kernel memory
//==========================================
//By Omar Emad Eldin
//==========================================

#include "pmm.h"

#include "hal.h"

#include <string.h>

using namespace zos;

uint32_t PMM::blockCount = 0;
PMM::Chunk* PMM::freeList = NULL;
uint32_t PMM::freeListSize = 0;
uint32_t PMM::freeListMaxSize = 0;
PMM::Chunk* PMM::usedList = NULL;
uint32_t PMM::usedListSize = 0;
uint32_t PMM::usedListMaxSize = 0;

void PMM::shiftFreeListRight(int32_t start)
{
	for (int32_t i = (freeListSize - 1); i >= start; i--)
		freeList[i + 1] = freeList[i];
}

void PMM::shiftFreeListLeft(uint32_t start)
{
	for (uint32_t i = start; i < (freeListSize - 1); i++)
		freeList[i] = freeList[i + 1];
}

bool PMM::isChunkInvalid(PMM::Chunk* chunk)
{
	if (chunk == NULL)
		return true;

	if (chunk->Length == 0)
		return true;

	return false;
}

void PMM::removeFreeChunk(PMM::Chunk* chunk)
{
	if (freeListSize == 0)
		return;

	uint32_t pos = 0;

	while ((pos < freeListSize) && (freeList[pos].Address != chunk->Address))
		pos++;

	shiftFreeListLeft(pos);

	freeListSize--;
}

void PMM::removeUsedChunk(PMM::Chunk* chunk)
{
	chunk->Address = NULL;
	chunk->Length = 0;

	usedListSize--;
}

PMM::Chunk* PMM::addFreeChunk(PhysicalAddress address, uint32_t length)
{
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

PMM::Chunk* PMM::addUsedChunk(PhysicalAddress address, uint32_t length)
{
	Chunk* chunk = usedList;

	while (!isChunkInvalid(chunk))	//Loop till we find an empty chunk
		chunk++;
	
	chunk->Address = address;
	chunk->Length = length;

	usedListSize++;

	return chunk;
}

PMM::Chunk* PMM::findFreeChunkContaining(PhysicalAddress address)
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

PMM::Chunk* PMM::findUsedChunk(PhysicalAddress address)
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

void PMM::setRegionUsed(PhysicalAddress address, uint32_t size)
{
	PhysicalAddress from = address;
	PhysicalAddress to = address + size;

	Chunk* chunk = findFreeChunkContaining(from);

	if ((chunk == NULL) || (chunk != findFreeChunkContaining(to)))
		return;

	uint32_t chunkFrom = chunk->Address;
	uint32_t chunkTo = chunk->Address + chunk->Length;

	if ((chunk->Address == address) && (chunk->Length == size))
	{
		removeFreeChunk(chunk);
	}
	else if ((chunkFrom < from) && (chunkTo > to))
	{
		removeFreeChunk(chunk);
		addFreeChunk(chunkFrom, (from - chunkFrom));
		addFreeChunk(to, (chunkTo - to));
	}
	else if ((chunkFrom >= from) && (chunkTo > to))
	{
		chunk->Address = to;
		chunk->Length = (chunkTo - to);
	}
	else if ((chunkFrom < from) && (chunkTo <= to))
	{
		chunk->Length = (from - chunkFrom);
	}
}

void PMM::setRegionUnused(PhysicalAddress address, uint32_t size)
{
	PhysicalAddress from = address;
	PhysicalAddress to = address + size;

	Chunk* chunkFrom = findFreeChunkContaining(from);
	Chunk* chunkTo = findFreeChunkContaining(to);

	if ((chunkFrom == NULL) && (chunkTo == NULL))
	{
		addFreeChunk(address, size);
	}
	else if ((chunkFrom != NULL) && (chunkTo == NULL))
	{
		chunkFrom->Length = to - (chunkFrom->Address + chunkFrom->Length);
	}
	else if ((chunkFrom == NULL) && (chunkTo != NULL))
	{
		chunkTo->Address = from;
		chunkTo->Length = (chunkTo->Address + chunkTo->Length) - from;
	}
	else if ((chunkFrom != NULL) && (chunkTo != NULL) && (chunkFrom != chunkTo))
	{
		chunkFrom->Length = (to - (chunkFrom->Address + chunkFrom->Length)) + ((chunkTo->Address + chunkTo->Length) - from);
		removeFreeChunk(chunkTo);
	}
}

void PMM::processBMM(PhysicalAddress address, uint32_t length)
{
	for (uint32_t i = 0; i < length; i++)
	{
		BMMEntry BMME = *((BMMEntry*)(address + i*sizeof(BMMEntry)));

		if (BMME.entryType == BMMEntry::Available)
			setRegionUnused(BMME.baseAddress, BMME.Length);
	}
}

uint32_t PMM::getFreeMemory()
{
	uint32_t result = 0;
	Chunk* chunk = freeList;

	for (uint32_t i = 0, count = 0; (i < freeListMaxSize) && (count < freeListSize); i++)
	{
		if (isChunkInvalid(&chunk[i]))
			continue;
		else
			count++;

		result += chunk[i].Length;
	}

	return result;
}

PhysicalAddress PMM::alloc(size_t size)
{
	if (size == 0)
		return NULL;

	uint32_t blocks = size / PMM_BLOCK_SIZE + ((size % PMM_BLOCK_SIZE != 0) ? 1 : 0);
	uint32_t realSize = blocks * PMM_BLOCK_SIZE;

	Chunk* chunk = freeList;

	for (uint32_t i = 0, count = 0; (i < freeListMaxSize) && (count < freeListSize); i++)
	{
		if (isChunkInvalid(&chunk[i]))
			continue;
		else
			count++;

		if (chunk[i].Length >= realSize)
		{
			PhysicalAddress address = chunk[i].Address;

			setRegionUsed(address, realSize);
			addUsedChunk(address, realSize);

			return address;
		}
	}	

	return NULL;
}

void PMM::free(PhysicalAddress address)
{
	if (address == NULL)
		return;

	Chunk* chunk = findUsedChunk(address);

	if (chunk == NULL)
		return;
	
	uint32_t length = chunk->Length;

	removeUsedChunk(chunk);
	setRegionUnused(address, length);
}

void PMM::init()
{
	blockCount = (HAL::memorySize / PMM_BLOCK_SIZE + ((HAL::memorySize % PMM_BLOCK_SIZE != 0) ? 1 : 0));

	freeList = ((Chunk*)(KERNEL_VBASE + KERNEL_SIZE));		//The free chunk list will be located just after the kernel
	freeListMaxSize = blockCount / 2;

	KERNEL_SIZE += (freeListMaxSize * sizeof(Chunk));

	usedList = ((Chunk*)(KERNEL_VBASE + KERNEL_SIZE));		//The used chunk list will be located after the free list
	usedListMaxSize = blockCount;

	KERNEL_SIZE += (usedListMaxSize * sizeof(Chunk));

	memset(freeList, 0, (freeListMaxSize * sizeof(Chunk))); 	//Default all free chunk list items to invalid
	memset(usedList, 0, (usedListMaxSize * sizeof(Chunk))); 	//Default all used chunk list items to invalid

	processBMM(BOOT_INFO->mmap_addr, BOOT_INFO->mmap_length);	//Process the BIOS Memory Map and set regions to used/unused accordingly

	setRegionUsed(NULL, PMM_BLOCK_SIZE);	//First block is always used so as not to return a NULL address
	setRegionUsed(KERNEL_PBASE, KERNEL_SIZE);	//Flag the space used by the kernel as used
}