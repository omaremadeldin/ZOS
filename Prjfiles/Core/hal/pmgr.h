//==========================================
//
//		ZapperOS - Partition Manager
//
//==========================================
//Used Acronyms:
//--------------
//* LBA = Linear Block Addressing
//==========================================
//TODO:
//--------------
//* Initalize Disk (MBR)
//* Support GUID
//* New Partition
//* Delete Partition
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "ide.h"

#include "utils/linkedlist.h"

#define PMGR_MBR_PARTITION_1 	0x1BE
#define PMGR_MBR_PARTITION_2 	0x1CE
#define PMGR_MBR_PARTITION_3 	0x1DE
#define PMGR_MBR_PARTITION_4 	0x1EE

namespace zos
{
	class PMGR
	{
	public:
		class Partition
		{
			private:
				IDE::Controller::Channel::Device* Disk;
				
			public:
				bool Bootable;
				uint8_t systemID;
				uint64_t startingLBA;
				uint64_t totalSectors;
				
			public:
				Partition(IDE::Controller::Channel::Device* disk, bool bootable, uint8_t systemID, uint64_t startingLBA, uint64_t totalSectors);
				void readData(uint64_t LBA, uint16_t noOfSectors, uint8_t* buffer);
				void writeData(uint64_t LBA, uint16_t noOfSectors, uint8_t* buffer);
		};
		
	private:
		struct PTEntry
		{	
			uint8_t						:7;
			bool		Bootable		:1;
			uint8_t		startingHead	:8;
			uint8_t		startingSector	:6;
			uint16_t	startingCylinder:10;
			uint8_t		systemID		:8;
			uint8_t		endingHead		:8;
			uint8_t		endingSector	:6;
			uint16_t	endingCylinder	:10;
			uint32_t	startingLBA		:32;
			uint32_t	totalSectors	:32;
		}__attribute__((packed));
		
	public:
		static void scan();
	};
	
	extern "C" LinkedList<PMGR::Partition*> PARTITIONS;
}