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

uint8_t* PMM::buffer = NULL;
uint32_t PMM::maxBlocks = 0;
uint32_t PMM::size = 0;
uint32_t PMM::freeBlocks = 0;

uint32_t PMM::getFreeBlocks()
{
	return freeBlocks;
}

bool PMM::testBlock(uint32_t blockIndex)
{
	return ((buffer[(blockIndex / PMM_BLOCKS_PER_BYTE)] & (1 << (blockIndex % PMM_BLOCKS_PER_BYTE))) != 0);
}

void PMM::setBlockUsed(uint32_t blockIndex)
{
	buffer[(blockIndex / PMM_BLOCKS_PER_BYTE)] |= (1 << (blockIndex % PMM_BLOCKS_PER_BYTE));
	freeBlocks--;
}

void PMM::setBlockUnused(uint32_t blockIndex)
{
	buffer[(blockIndex / PMM_BLOCKS_PER_BYTE)] &= ~(1 << (blockIndex % PMM_BLOCKS_PER_BYTE));
	freeBlocks++;
}

void PMM::setBlocksUsed(uint32_t blockIndex, size_t num)
{
	for (uint32_t i = blockIndex; i < (blockIndex + num); i++)
		setBlockUsed(i);
}

void PMM::setBlocksUnused(uint32_t blockIndex, size_t num)
{
	for (uint32_t i = blockIndex; i < (blockIndex + num); i++)
		setBlockUnused(i);
}

void PMM::setRegionUsed(PhysicalAddress address, size_t size)
{
	uint32_t blockIndex = address / PMM_BLOCK_SIZE;
	uint32_t blocksCount = size / PMM_BLOCK_SIZE + ((size % PMM_BLOCK_SIZE != 0)? 1 : 0);

	setBlocksUsed(blockIndex, blocksCount);
}

void PMM::setRegionUnused(PhysicalAddress address, size_t size)
{
	uint32_t blockIndex = address / PMM_BLOCK_SIZE;
	uint32_t blocksCount = size / PMM_BLOCK_SIZE + ((size % PMM_BLOCK_SIZE != 0)? 1 : 0);

	//Prevents Setting or Unsetting Block 0, as alloc can't return 0
	if (blockIndex == 0)
	{
		blockIndex++;
		blocksCount--;
	}

	setBlocksUnused(blockIndex, blocksCount);
}

int32_t PMM::firstFreeBlock()
{
	for (uint32_t i = 0; i < maxBlocks; i++)
		if (!testBlock(i))
			return i;

	return PMM_NPOS;
}

int32_t PMM::firstFreeBlocks(size_t num)
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

	return PMM_NPOS;
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

PhysicalAddress PMM::alloc()
{
	//No free blocks -> out of memory
	if (freeBlocks == 0)
		return NULL;

	int32_t frame = firstFreeBlock();

	//Can't find free block -> out of memory
	if (frame == PMM_NPOS)
		return NULL;

	setBlockUsed(frame);

	PhysicalAddress address = frame * PMM_BLOCK_SIZE;

	return address;
}

void PMM::free(PhysicalAddress address)
{
	int32_t frame = address / PMM_BLOCK_SIZE;

	setBlockUnused(frame);
}

void PMM::init()
{
	const uint32_t bitmapLocation = KERNEL_VBASE + KERNEL_SIZE;
	
	//By default all blocks are used unless stated
	uint32_t maxBlks = HAL::memorySize / PMM_BLOCK_SIZE + ((HAL::memorySize % PMM_BLOCK_SIZE != 0) ? 1 : 0);
	
	buffer = (uint8_t*)bitmapLocation;
	maxBlocks = maxBlks;
	size = (maxBlks / PMM_BLOCKS_PER_BYTE + ((maxBlks % PMM_BLOCKS_PER_BYTE != 0) ? 1 : 0));
	freeBlocks = 0;
	
	//Default the memory map with 1s representing used blocks
	memset((void*)bitmapLocation, 0xFF, size);
	
	//Flag the space used by the kernel as used
	setRegionUsed(KERNEL_PBASE, KERNEL_SIZE);
	
	//Flag the space used by the memory map as used
	setRegionUsed((bitmapLocation - KERNEL_VBASE + KERNEL_PBASE), size);
	
	//Process the BIOS Memory Map and set regions to used/unused accordingly
	processBMM(BOOT_INFO->mmap_addr, BOOT_INFO->mmap_length);
}