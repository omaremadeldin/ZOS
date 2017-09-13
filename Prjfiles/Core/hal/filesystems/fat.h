//==========================================
//
//		ZapperOS - Filesystem - FAT
//
//==========================================
//This is filesystem driver has been made
//stricty according the specification.
//Reference: MS FAT Specification
//TODO: Unicode Support
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

#include "../hal.h"
#include "../vmgr.h"

#include "../utils/bitfield.h"
#include "../utils/path.h"

#define FAT_COMMON_BPB_OFFSET		0x00
#define FAT_FAT16_BPB_OFFSET		0x24
#define FAT_FAT32_BPB_OFFSET		0x24

#define FAT_ENTRY_SIZE				0x20

#define FAT_ATTR_LONG_NAME			0x0F
#define FAT_LAST_LONG_ENTRY			0x40
#define FAT_LONG_ENTRY_SIZE			0x0D

#define FAT_MAX_LONG_FILE_NAME		255
#define FAT_MAX_FILE_SIZE			0xFFFFFFFF
#define FAT_MAX_ENTRIES				0xFFFF

#define FAT_FAT16_EOF				0xFFF8
#define FAT_FAT32_EOF				0x0FFFFFF8

#define FAT_FAT16_EOC_MARK			0xFFFF
#define FAT_FAT32_EOC_MARK			0x0FFFFFFF

#define FAT_VAR_FATTYPE				0
#define FAT_VAR_FATSZ				1
#define FAT_VAR_BPB_NUMFATS			2
#define FAT_VAR_BPB_RESVDSECCNT		3
#define FAT_VAR_BPB_SECPERCLUS		4
#define FAT_VAR_BPB_ROOTENTCNT		5
#define FAT_VAR_BPB_ROOTCLUS		6
#define FAT_VAR_CLUSTER_COUNT		7
#define FAT_VAR_OFFLINE_FAT			8
#define FAT_VAR_NEXT_CLUSTER		9
#define FAT_VAR_FREE_COUNT			10
#define FAT_VAR_MIRRORING			11
#define FAT_VAR_ACTIVE_FAT			12

#define FAT_FILE_VAR_ENTRY_CLUSTER	0
#define FAT_FILE_VAR_ENTRY_INDEX	1

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

			struct FAT16BPB
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

			struct FAT32FSInfo
			{
				uint32_t 	FSI_LeadSig;
				uint8_t		FSI_Reserved1[480];
				uint32_t	FSI_StrucSig;
				uint32_t	FSI_Free_Count;
				uint32_t	FSI_Nxt_Free;
				uint8_t		FSI_Reserved2[12];
				uint32_t	FSI_TrailSig;
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
				uint16_t			DIR_FstClusHI;		//Empty in case of FAT16
				uint16_t			DIR_WrtTime;
				uint16_t			DIR_WrtDate;
				uint16_t			DIR_FstClusLO;
				uint32_t			DIR_FileSize;
			}__attribute__((packed));

			struct FATLongEntry
			{
				uint8_t				LDIR_Ord;
				uint16_t			LDIR_Name1[5];
				uint8_t				LDIR_Attr;
				uint8_t				LDIR_Type;
				uint8_t				LDIR_Chksum;
				uint16_t			LDIR_Name2[6];
				uint16_t			LDIR_FstClusLO;
				uint16_t			LDIR_Name3[2];
			}__attribute__((packed));

		private:
			void readFAT(VMGR::Volume* volume, uint8_t* buffer);
			void readCluster(VMGR::Volume* volume, uint32_t cluster, uint8_t* buffer);
			void writeCluster(VMGR::Volume* volume, uint32_t cluster, uint8_t* buffer);
			void clearCluster(VMGR::Volume* volume, uint32_t cluster);
			uint32_t readFATEntry(VMGR::Volume* volume, uint32_t cluster);
			uint32_t readFATEntry(VMGR::Volume* volume, uint8_t* fat, uint32_t cluster);
			void writeFATEntry(VMGR::Volume* volume, uint32_t cluster, uint32_t entry);
			uint8_t processLongEntry(const FATLongEntry* entry, char* dstStr);
			void processShortEntry(const FATEntry* entry, char* dstStr);
			uint8_t shortNameCheckSum(uint8_t* filename);
			uint16_t createDateValue(HAL::DateTime date);
			uint16_t createTimeValue(HAL::DateTime time);
			const char* getFAT32VolumeLabel(VMGR::Volume* volume);
			FATEntry* findFreeEntry(VMGR::Volume* volume, uint8_t* buffer, uint32_t& cluster, uint8_t num = 1);
			uint32_t findFreeCluster(VMGR::Volume* volume);
			void createLongEntry(FATLongEntry* entry, uint8_t order, char* srcStr, uint8_t chksum);
			bool isValid83Char(char c);
			bool generateShortName(char* srcLong, char* dstShort, uint8_t& NTRes, VMGR::File** fileList, uint32_t fileListLength);
			uint32_t list(VMGR::File* directory, VMGR::File** &fileList, bool useShortNames);
		
		public:
			FAT();

		public:
			bool systemID(uint8_t sysID);
			void init(VMGR::Volume* volume);
			uint64_t getFreeSpace(VMGR::Volume* volume);
			void release(VMGR::Volume* volume);
			
		public:
			bool create(VMGR::Volume* volume, Path* filePath, VMGR::File* dstFile, bool isDirectory);
			bool find(VMGR::Volume* volume, Path* filePath, VMGR::File* dstFile);
			uint32_t list(VMGR::File* directory, VMGR::File** &fileList);
			bool remove(VMGR::File* file, bool recursive);
			bool read(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead);
			bool write(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToWrite, uint64_t& bytesWritten);
			bool resize(VMGR::File* file, uint64_t fileSize);
		};
	}
}