//==========================================
//
//	  ZapperOS - Virtual Memory Manager
//
//==========================================
//Used Acronyms:
//--------------
//* VMM = Virtual Memory Manager
//* PDBR = Page Directory Base Register
//==========================================
//By Omar Emad Eldin
//==========================================

#include "vmm.h"

#include "hal.h"

#include <string.h>

using namespace zos;

VMM::PageTable inline VMM::PDEntry::getFrame() const
{
	return (PageTable)(Frame << 0xC);
}

void inline VMM::PDEntry::setFrame(PageTable entry)
{
	Frame = (PhysicalAddress)entry >> 0xC;
}

void inline VMM::PDEntry::setFrame(PhysicalAddress entry)
{
	Frame = entry >> 0xC;
}

PhysicalAddress inline VMM::PTEntry::getFrame() const
{
	return (Frame << 0xC);
}

void inline VMM::PTEntry::setFrame(PhysicalAddress entry)
{
	Frame = entry >> 0xC;
}

VMM::PageDirectory VMM::pageDirectory = (PageDirectory)VMM_PAGE_DIRECTORY;
VMM::PageTable VMM::pageTable = (PageTable)VMM_PAGE_TABLE;
VMM::PTEntry VMM::swapPageTable[VMM_ENTRIES_IN_PAGETABLE];

uint32_t inline VMM::getPageTableIndex(VirtualAddress vAddress)
{
	return ((vAddress >> 12) & 0x3FF);
}

uint32_t inline VMM::getDirectoryIndex(VirtualAddress vAddress)
{
	return ((vAddress >> 22) & 0x3FF);
}

void VMM::setPDBR(PageDirectory pageDirectory)
{
	pageDirectory = (PageDirectory)((uint32_t)pageDirectory & 0xFFFFF000);
	__asm__("mov eax, %0"::"g"(pageDirectory):"eax");
	__asm__("mov cr3, eax":::"eax");
}

VMM::PageDirectory VMM::getPDBR()
{
	PDEntry* PDBR = NULL;
	__asm__("mov %0, cr3":"=r"(PDBR));
	PDBR = (PDEntry*)((uint32_t)PDBR & 0xFFFFF000);
	return PDBR;
}

void VMM::flushTLBEntry(VirtualAddress vAddress)
{
	uint32_t volatile address = vAddress;
	__asm__("mov eax, %0"::"g"(address):"eax");
	__asm__("invlpg [eax]":::"eax");
}

void VMM::lookupPageTable(PageTable pageTable)
{
	const uint32_t pageIndex = getPageTableIndex(VMM_PAGE_TABLE);
	
	swapPageTable[pageIndex].rwFlag = PTEntry::ReadWrite;
	swapPageTable[pageIndex].usFlag = PTEntry::UserMode;
	swapPageTable[pageIndex].setFrame((PhysicalAddress)pageTable);
	swapPageTable[pageIndex].Present = true;
	flushTLBEntry(VMM_PAGE_TABLE);
}

void VMM::mapPage(PhysicalAddress pAddress, VirtualAddress vAddress)
{
	uint32_t PDindex = getDirectoryIndex(vAddress);
	uint32_t PTindex = getPageTableIndex(vAddress);
	
	if (!pageDirectory[PDindex].Present)
	{
		PhysicalAddress p = PMM::alloc(VMM_PAGE_SIZE);
		
		//TODO:Exceptions
		//if (p == NULL)
		//	return throwSoftwareFault("VMM: Not enough memory for mapping new page.");
		
		if (p == NULL)
			return;
		
		PageTable newPT = (PageTable)p;
		lookupPageTable(newPT);
		
		memset((void*)VMM_PAGE_TABLE, 0, VMM_PAGE_SIZE);
		
		pageDirectory[PDindex].rwFlag = PDEntry::ReadWrite;
		pageDirectory[PDindex].usFlag = PDEntry::UserMode;
		pageDirectory[PDindex].setFrame(newPT);
		pageDirectory[PDindex].Present = true;
	}
	
	PageTable curPT = pageDirectory[PDindex].getFrame();
	lookupPageTable(curPT);
	
	if (pageTable[PTindex].Present)
	{
		//If page already present deallocate the old block and set the new one
		PMM::free(pageTable[PTindex].getFrame());
		pageTable[PTindex].rwFlag = PTEntry::ReadWrite;
		pageTable[PTindex].usFlag = PTEntry::UserMode;
		pageTable[PTindex].setFrame(pAddress);
	}
	else
	{
		//If page not present then set the new block only
		pageTable[PTindex].rwFlag = PTEntry::ReadWrite;
		pageTable[PTindex].usFlag = PTEntry::UserMode;
		pageTable[PTindex].setFrame(pAddress);
		pageTable[PTindex].Present = true;
	}
	
	flushTLBEntry(vAddress);
}

void VMM::mapPages(PhysicalAddress pAddress, VirtualAddress vAddress, uint32_t num)
{
	for (uint32_t i = 0; i < num; i++)
		mapPage(pAddress + (i * VMM_PAGE_SIZE), vAddress + (i * VMM_PAGE_SIZE));
}

VirtualAddress VMM::alloc(VirtualAddress vAddress, size_t size)
{
	uint32_t pages = (size + (VMM_PAGE_SIZE - 1)) / VMM_PAGE_SIZE;
	PhysicalAddress p = PMM::alloc(pages * VMM_PAGE_SIZE);
	
	if (p == NULL)
		return NULL;
	
	mapPages(p, vAddress, pages);
	
	return vAddress;
}

void VMM::free(VirtualAddress vAddress)
{
	uint32_t PDindex = getDirectoryIndex(vAddress);
	uint32_t PTindex = getPageTableIndex(vAddress);
	
	if (!pageDirectory[PDindex].Present)
		return;
	
	PageTable curPT = pageDirectory[PDindex].getFrame();
	lookupPageTable(curPT);
	
	if (!pageTable[PTindex].Present)
		return;

	PMM::Chunk* chunk = PMM::findUsedChunk(pageTable[PTindex].getFrame());
	
	if (chunk == NULL)
		return;

	uint32_t pageCount = (chunk->Length + (VMM_PAGE_SIZE - 1)) / VMM_PAGE_SIZE; 

	PMM::free(pageTable[PTindex].getFrame());

	for (uint32_t i = 0; i < pageCount; i++)
	{
		PDindex = getDirectoryIndex(vAddress + (i * VMM_PAGE_SIZE));
		PTindex = getPageTableIndex(vAddress + (i * VMM_PAGE_SIZE));

		if (!pageDirectory[PDindex].Present)
			continue;

		PageTable curPT = pageDirectory[PDindex].getFrame();
		lookupPageTable(curPT);
	
		if (!pageTable[PTindex].Present)
			continue;

		pageTable[PTindex].Present = false;

		flushTLBEntry(vAddress + (i * VMM_PAGE_SIZE));
	}
}

void VMM::init()
{
	//Clear the swap page table
	memset(swapPageTable, 0, sizeof(swapPageTable));
	
	//Set the address 0xFFFFF000 to point to the page directory
	uint32_t pageIndex = getPageTableIndex(VMM_PAGE_DIRECTORY);
	swapPageTable[pageIndex].rwFlag = PTEntry::ReadWrite;
	swapPageTable[pageIndex].usFlag = PTEntry::UserMode;
	swapPageTable[pageIndex].setFrame((PhysicalAddress)getPDBR());
	swapPageTable[pageIndex].Present = true;
	
	//Attach the swap page table to the page directory
	uint32_t swapIndex = getDirectoryIndex(VMM_PAGE_TABLE);
	pageDirectory[swapIndex].rwFlag = PDEntry::ReadWrite;
	pageDirectory[swapIndex].usFlag = PDEntry::UserMode;
	pageDirectory[swapIndex].setFrame((PhysicalAddress)&swapPageTable - KERNEL_VBASE + KERNEL_PBASE);
	pageDirectory[swapIndex].Present = true;
	
	flushTLBEntry(VMM_PAGE_DIRECTORY);
	flushTLBEntry(VMM_PAGE_TABLE);
	
	//TODO:relocate stack
	//Discard the first 4MBs identity pages except 1 page for the stack
	//pageDirectory[0].Present = false;
	PageTable PT0 = pageDirectory[0].getFrame();
	lookupPageTable(PT0);
	
	for (uint32_t i = 0; i < VMM_ENTRIES_IN_PAGETABLE; i++)
		if (!(i*VMM_PAGE_SIZE == 0x0000 || i*VMM_PAGE_SIZE == 0x8F000))
			pageTable[i].Present = false;
}