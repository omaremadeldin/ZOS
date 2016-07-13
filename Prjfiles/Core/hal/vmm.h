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

#pragma once

#include <stdlib.h>

#include "pmm.h"

#include "utils/bitfield.h"

#define VMM_PAGE_SIZE	4096

#define VMM_PAGE_DIRECTORY	0xFFFFF000
#define VMM_PAGE_TABLE		0xFFFFE000

#define VMM_ENTRIES_IN_PAGEDIRECTORY	1024
#define VMM_ENTRIES_IN_PAGETABLE		1024

namespace zos
{
	//Global Typedefs
	typedef uint32_t VirtualAddress;
	
	class VMM
	{
	public:
		struct PDEntry;
		struct PTEntry;
		
		typedef PDEntry* PageDirectory;
		typedef PTEntry* PageTable;
	public:
		//Page Directory Entry 
		struct PDEntry : BitField<uint32_t>
		{
			enum RWFlag
			{
				ReadOnly = 0,
				ReadWrite = 1
			};

			enum USFlag
			{
				SupervisorMode = 0,
				UserMode = 1
			};

			enum WTFlag
			{
				WriteBackCaching = 0,
				WriteThroughCaching = 1
			};
			
			bool  		Present 	: 1;		//Present ?
			RWFlag  	rwFlag		: 1;		//Read/Write Flag
			USFlag  	usFlag		: 1;		//User/Supervisor Flag
			WTFlag 		wtFlag		: 1;		//Write-Through Flag
			bool  	 	Cache		: 1;		//Cache Page Tables ?
			bool	  	accessFlag	: 1;		//Access Flag (Set by the processor)
			uint8_t  				: 1;		//Reserved
			uint8_t  	pageSize	: 1;		//Page Size ? (4KB/4MB)
			bool	 	Global		: 1;		//Global ?
			uint8_t		Available	: 3;		//Available for use by the OS
			PhysicalAddress	Frame	: 20;		//Physical Address
			
			PageTable inline getFrame() const;
			
			void inline setFrame(PageTable entry);			
			void inline setFrame(PhysicalAddress entry);
		}__attribute__((packed));
		
		//Page Table Entry 
		struct PTEntry : BitField<uint32_t>
		{
			enum RWFlag
			{
				ReadOnly = 0,
				ReadWrite = 1
			};

			enum USFlag
			{
				SupervisorMode = 0,
				UserMode = 1
			};
			
			bool			Present 	: 1;		//Present ?
			RWFlag 			rwFlag		: 1;		//Read/Write Flag
			USFlag 			usFlag		: 1;		//User/Supervisor Flag
			uint8_t 					: 2;		//Reserved
			bool  			accessFlag	: 1;		//Access Flag (Set by the processor)
			bool  			dirtyFlag	: 1;		//Dirty Flag (Set by the processor)
			uint8_t 					: 2;		//Reserved
			uint8_t  		Available	: 3;		//Available for use by the OS
			PhysicalAddress Frame		: 20;		//Physical Address
			
			PhysicalAddress inline getFrame() const;
			
			void inline setFrame(PhysicalAddress entry);
		}__attribute__((packed));
	
	private:
		static PageDirectory pageDirectory;
		static PageTable pageTable;
		
		__attribute__((aligned (4096)))
		static PTEntry swapPageTable[VMM_ENTRIES_IN_PAGETABLE];
		
	private:
		static uint32_t inline getPageTableIndex(VirtualAddress vAddress);
		static uint32_t inline getDirectoryIndex(VirtualAddress vAddress);
		static void setPDBR(PageDirectory pageDirectory);
		static PageDirectory getPDBR();
		static void flushTLBEntry(VirtualAddress vAddress);
		static void lookupPageTable(PageTable pageTable);	
		
	public:
		static void mapPage(PhysicalAddress pAddress, VirtualAddress vAddress);
		static void mapPages(PhysicalAddress pAddress, VirtualAddress vAddress, size_t num);
		static VirtualAddress alloc(VirtualAddress vAddress);
		static void free(VirtualAddress vAddress);
		static void init();
	};
}