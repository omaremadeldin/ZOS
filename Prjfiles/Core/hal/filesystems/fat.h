//==========================================
//
//		ZapperOS - Filesystem - FAT
//
//==========================================
//This is filesystem driver has been made
//stricty according the specification.
//Reference: MS FAT Specification
//==========================================
//Used Acronyms:
//--------------
//* FAT = File Allocation Table
//* BPB = BIOS Parameter Block
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "../vmgr.h"

#include "../utils/bitfield.h"

#define FAT_COMMON_BPB_OFFSET	0x00
#define FAT_FAT1216_BPB_OFFSET	0x24
#define FAT_FAT32_BPB_OFFSET	0x24

#define FAT_MAX_LONG_FILE_NAME	255
#define FAT_FAT12_EOF			0x0FF8
#define FAT_FAT16_EOF			0xFFF8
#define FAT_FAT32_EOF			0x0FFFFFF8

#define FAT_VAR_FATTYPE			0
#define FAT_VAR_FATSZ			1
#define FAT_VAR_BPB_NUMFATS		2
#define FAT_VAR_BPB_RESVDSECCNT	3
#define FAT_VAR_BPB_SECPERCLUS	4
#define FAT_VAR_BPB_ROOTENTCNT	5
#define FAT_VAR_BPB_ROOTCLUS	6

namespace zos
{
	namespace filesystems
	{
		class FAT : public VMGR::Filesystem
		{
		private:
			struct FATCommonBPB
			{	
				uint8_t		BS_jmpBoot[3];
				uint8_t		BS_OEMName[8];
				uint16_t	BPB_BytsPerSec;
				uint8_t		BPB_SecPerClus;
				uint16_t	BPB_ResvdSecCnt;
				uint8_t		BPB_NumFATs;
				uint16_t	BPB_RootEntCnt;
				uint16_t	BPB_TotSec16;
				uint8_t		BPB_Media;
				uint16_t	BPB_FATSz16;
				uint16_t	BPB_SecPerTrk;
				uint16_t	BPB_NumHeads;
				uint32_t	BPB_HiddSec;
				uint32_t	BPB_TotSec32;
			}__attribute__((packed));

			struct FAT1216BPB
			{	
				uint8_t		BS_DrvNum;
				uint8_t		BS_Reserved1;
				uint8_t		BS_BootSig;
				uint8_t		BS_VolID[4];
				uint8_t		BS_VolLab[11];
				uint8_t		BS_FilSysType[8];
			}__attribute__((packed));

			struct FAT32BPB
			{	
				uint32_t	BPB_FATSz32;
				uint16_t	BPB_ExtFlags;
				uint16_t	BPB_FSVer;
				uint32_t	BPB_RootClus;
				uint16_t	BPB_FSInfo;
				uint16_t	BPB_BkBootSec;
				uint8_t		BPB_Reserved[12];
				uint8_t		BS_DrvNum;
				uint8_t		BS_Reserved1;
				uint8_t		BS_BootSig;
				uint8_t		BS_VolID[4];
				uint8_t		BS_VolLab[11];
				uint8_t		BS_FilSysType[8];
			}__attribute__((packed));

			struct FATFileAttributes : BitField<uint8_t>
			{
				uint8_t 	ATTR_READ_ONLY	:1;
				uint8_t		ATTR_HIDDEN		:1;
				uint8_t		ATTR_SYSTEM		:1;
				uint8_t		ATTR_VOLUME_ID	:1;
				uint8_t		ATTR_DIRECTORY	:1;
				uint8_t		ATTR_ARCHIVE	:1;
				uint8_t						:2;
			}__attribute__((packed));

			struct FATEntry
			{
				uint8_t 			DIR_Name[11];
				FATFileAttributes	DIR_Attr;
				uint8_t				DIR_NTRes;
				uint8_t				DIR_CrtTimeTenth;
				uint16_t			DIR_CrtTime;
				uint16_t			DIR_CrtDate;
				uint16_t			DIR_LstAccDate;
				uint16_t			DIR_FstClusHI;		//Empty in case of FAT12/FAT16
				uint16_t			DIR_WrtTime;
				uint16_t			DIR_WrtDate;
				uint16_t			DIR_FstClusLO;
				uint32_t			DIR_FileSize;
			}__attribute__((packed));

		private:
			uint32_t readFATEntry(VMGR::Volume* volume, uint32_t cluster);
			void writeFATEntry(VMGR::Volume* volume, uint32_t cluster, uint32_t entry);

		public:
			FAT();
			void init(VMGR::Volume* volume);
			bool getFileEntry(VMGR::Volume* volume, const char* filePath, VMGR::File* dstFile);
			//bool read(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead);
		};
	}
}