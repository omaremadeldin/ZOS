//==========================================
//
//		ZapperOS - Filesystem - FAT
//
//==========================================
//Used Acronyms:
//--------------
//* FAT = File Allocation Table
//==========================================
//By Omar Emad Eldin
//==========================================

#include "fat.h"

#include "../hal.h"
#include "../ide.h"

#include <string.h>
	
using namespace zos;
using namespace zos::filesystems;

uint32_t FAT::readFATEntry(VMGR::Volume* volume, uint32_t cluster)
{
	uint8_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];

	uint32_t FATOffset, ThisFATSecNum, ThisFATEntOffset, FATEntryVal;

	if (FATType == 12)
		FATOffset = cluster + (cluster / 2);
	else if (FATType == 16)
		FATOffset = cluster * 2;
	else if (FATType == 32)
		FATOffset = cluster * 4;

	ThisFATSecNum = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (FATOffset / IDE_ATA_BYTESPERSECTOR);
	ThisFATEntOffset = (FATOffset % IDE_ATA_BYTESPERSECTOR);

	uint8_t buffer[IDE_ATA_BYTESPERSECTOR];
	((PMGR::Partition*)(volume->Target))->readData(ThisFATSecNum, IDE_ATA_BYTESPERSECTOR, buffer);

	if (FATType == 12)
	{
		FATEntryVal = *((uint16_t*)&buffer[ThisFATEntOffset]);

		if (cluster % 2 == 0)
			FATEntryVal = (FATEntryVal & 0xFFF);
		else
			FATEntryVal = ((FATEntryVal >> 4) & 0xFFF);
	}
	else if (FATType == 16)
		FATEntryVal = *((uint16_t*)&buffer[ThisFATEntOffset]);
	else if (FATType == 32)
		FATEntryVal = (*((uint32_t*)&buffer[ThisFATEntOffset])) & 0x0FFFFFFF;

	return FATEntryVal;
}

void FAT::writeFATEntry(VMGR::Volume* volume, uint32_t cluster, uint32_t entry)
{
	uint8_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];

	uint32_t FATOffset, ThisFATSecNum, ThisFATEntOffset;

	if (FATType == 12)
		FATOffset = cluster + (cluster / 2);
	else if (FATType == 16)
		FATOffset = cluster*2;
	else if (FATType == 32)
		FATOffset = cluster*4;

	ThisFATSecNum = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (FATOffset / IDE_ATA_BYTESPERSECTOR);
	ThisFATEntOffset = (FATOffset % IDE_ATA_BYTESPERSECTOR);
	
	uint8_t buffer[IDE_ATA_BYTESPERSECTOR];
	((PMGR::Partition*)(volume->Target))->readData(ThisFATSecNum, IDE_ATA_BYTESPERSECTOR, buffer);

	if (FATType == 12)
	{
		uint16_t FATEntryVal = readFATEntry(volume, cluster);

		if (cluster % 2 == 0)
			*((uint16_t*)&buffer[ThisFATEntOffset]) = ((entry & 0xFFF) | FATEntryVal);
		else
			*((uint16_t*)&buffer[ThisFATEntOffset]) = (((entry & 0xFFF) << 4) | FATEntryVal);
	}
	if (FATType == 16)
		*((uint16_t*)&buffer[ThisFATEntOffset]) = (entry & 0xFFFF);
	else if (FATType == 32)
		(*((uint32_t*)&buffer[ThisFATEntOffset])) = (entry & 0x0FFFFFFF);
	
	((PMGR::Partition*)(volume->Target))->writeData(ThisFATSecNum, 1, buffer);
}

FAT::FAT()
{
	Name = "FAT";
	systemID = 0x0B;
}

void FAT::init(VMGR::Volume* volume)
{
	if (volume == NULL)
		return;

	uint8_t buffer[IDE_ATA_BYTESPERSECTOR];
	((PMGR::Partition*)volume->Target)->readData(0, 1, buffer);

	FATCommonBPB* commonBPB = ((FATCommonBPB*)((uint32_t)&buffer + FAT_COMMON_BPB_OFFSET));
	FAT1216BPB* fat1216BPB = ((FAT1216BPB*)((uint32_t)&buffer + FAT_FAT1216_BPB_OFFSET));
	FAT32BPB* fat32BPB = ((FAT32BPB*)((uint32_t)&buffer + FAT_FAT32_BPB_OFFSET));

	if (commonBPB->BPB_BytsPerSec != IDE_ATA_BYTESPERSECTOR) //No support for non-512 sector sizes
		return;

	uint16_t RootDirSectors = (((commonBPB->BPB_RootEntCnt * 32) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	uint32_t FATSz, TotSec, DataSec, CountofClusters;

	if (commonBPB->BPB_FATSz16 != 0)
		FATSz = commonBPB->BPB_FATSz16;
	else
		FATSz = fat32BPB->BPB_FATSz32;

	if (commonBPB->BPB_TotSec16 != 0)
		TotSec = commonBPB->BPB_TotSec16;
	else
		TotSec = commonBPB->BPB_TotSec32;

	DataSec = TotSec - (commonBPB->BPB_ResvdSecCnt + (commonBPB->BPB_NumFATs * FATSz) + RootDirSectors);

	CountofClusters = DataSec / commonBPB->BPB_SecPerClus;

	uint8_t FATType;

	if (CountofClusters < 4085)
		FATType = 12;
	else if (CountofClusters < 65525)
		FATType = 16;
	else
		FATType = 32;

	if (FATType == 32)
	{
		strncpy(volume->Name, (const char*)&fat32BPB->BS_VolLab, 11);		
		volume->Name[11] = '\0';
	}
	else
	{
		strncpy(volume->Name, (const char*)&fat1216BPB->BS_VolLab, 11);
		volume->Name[11] = '\0';
	}

	//Set volume filesystem variables to save all needed values for later use
	volume->fsVariables[FAT_VAR_FATTYPE] = FATType;
	volume->fsVariables[FAT_VAR_FATSZ] = (FATType == 32 ? fat32BPB->BPB_FATSz32 : commonBPB->BPB_FATSz16);
	volume->fsVariables[FAT_VAR_BPB_NUMFATS] = commonBPB->BPB_NumFATs;
	volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] = commonBPB->BPB_ResvdSecCnt;
	volume->fsVariables[FAT_VAR_BPB_SECPERCLUS] = commonBPB->BPB_SecPerClus;
	volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] = commonBPB->BPB_RootEntCnt;
	volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] = fat32BPB->BPB_RootClus;

	volume->Online = true;
}

bool FAT::getFileEntry(VMGR::Volume* volume, const char* filePath, VMGR::File* dstFile)
{
	if ((volume == NULL) || (!volume->Online))
		return false;

	if (dstFile == NULL)
		return false;

	if (strlen(filePath) <= 1)
		return false;

	uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];

	uint16_t RootDirSectors = (((volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * sizeof(FATEntry)) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	uint16_t RootDirSector = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (volume->fsVariables[FAT_VAR_BPB_NUMFATS] * FATSz);
	uint16_t FirstDataSector = RootDirSector + RootDirSectors;	//In case of FAT32, RootDirSectors = 0

	uint8_t* buffer = new uint8_t[(IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])];

	if (FATType == 32)
		((PMGR::Partition*)(volume->Target))->readData((FirstDataSector + ((volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])), volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
	else
		((PMGR::Partition*)(volume->Target))->readData(RootDirSector, volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);

	FATEntry* entry = ((FATEntry*)buffer);

	char* path = new char[strlen(filePath) + 1];	//Seperate copy of the path to convert it to lowercase & tokenize with strtok
	strcpy(path, filePath);
	strtolower(path);

	//*//uint32_t currentCluster = (FATType == 32 ? volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] : 0);

	bool rootDirectory = (FATType == 32 ? false : true);
	bool Found = true;

	char* currentFileName = strtok(path, "/");

	while (Found && (currentFileName != NULL))
	{
		Found = false;

		uint16_t clusterCount = 1;
		uint16_t entryCount = 1;

		while ((entryCount * sizeof(FATEntry)) < (clusterCount * IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS]))
		{
			if ((entry->DIR_Name[0] == 0xE5) || (entry->DIR_Name[0] == 0x00)) //Indicates entry is free
			{
				entry++;
				entryCount++;
			}
			else
			{
				if (entry->DIR_Attr.ATTR_READ_ONLY 	&&
					entry->DIR_Attr.ATTR_HIDDEN 	&&
					entry->DIR_Attr.ATTR_SYSTEM		&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					//Long File Name Entry
				}
				else
				{
					char fileName[9];
					strncpy(fileName, ((char*)(entry->DIR_Name)), 8);
					fileName[8] = '\0';
					strtrim(fileName);
					strtolower(fileName);

					char extension[4];
					strncpy(extension, ((char*)(entry->DIR_Name) + 8), 3);
					extension[3] = '\0';
					strtrim(extension);
					strtolower(extension);

					HAL::debug("'%s'.'%s'\n", fileName, extension);

					entry++;
					entryCount++;
				}
			}

			if (!rootDirectory)
			{

			}
		}

		currentFileName = strtok(NULL, "/");
	}

	delete [] buffer;

	return true;
}

// bool FAT::read(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead)
// {
	
// }