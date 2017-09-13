//==========================================
//
//		ZapperOS - Filesystem - FAT
//
//==========================================
//Used Acronyms:
//--------------
//* FAT = File Allocation Table
//* LFN = Long File Name
//==========================================
//By Omar Emad Eldin
//==========================================

#include "fat.h"

#include "../ide.h"

#include "../utils/format.h"

#include "../misc/macros.h"

#include <string.h>
	
using namespace zos;
using namespace zos::filesystems;

void FAT::readFAT(VMGR::Volume* volume, uint8_t* buffer)
{
	const uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];
	const uint8_t activeFAT = volume->fsVariables[FAT_VAR_ACTIVE_FAT];

	((PMGR::Partition*)(volume->Target))->readData(volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (activeFAT * FATSz), FATSz, buffer);
}

void FAT::readCluster(VMGR::Volume* volume, uint32_t cluster, uint8_t* buffer)
{
	const uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];
	const uint16_t RootDirSectors = (((volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * sizeof(FATEntry)) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	const uint16_t RootDirSector = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (volume->fsVariables[FAT_VAR_BPB_NUMFATS] * FATSz);
	const uint16_t FirstDataSector = RootDirSector + RootDirSectors;	//In case of FAT32, RootDirSectors = 0

	if (cluster == 0)	//Root directory
		((PMGR::Partition*)(volume->Target))->readData(RootDirSector, RootDirSectors, buffer);
	else
		((PMGR::Partition*)(volume->Target))->readData((FirstDataSector + ((cluster - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])), volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
}

void FAT::writeCluster(VMGR::Volume* volume, uint32_t cluster, uint8_t* buffer)
{
	const uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];
	const uint16_t RootDirSectors = (((volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * sizeof(FATEntry)) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	const uint16_t RootDirSector = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (volume->fsVariables[FAT_VAR_BPB_NUMFATS] * FATSz);
	const uint16_t FirstDataSector = RootDirSector + RootDirSectors;	//In case of FAT32, RootDirSectors = 0

	if (cluster == 0)	//Root directory
		((PMGR::Partition*)(volume->Target))->writeData(RootDirSector, RootDirSectors, buffer);
	else
		((PMGR::Partition*)(volume->Target))->writeData((FirstDataSector + ((cluster - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])), volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
}

void FAT::clearCluster(VMGR::Volume* volume, uint32_t cluster)
{
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS];

	uint8_t* buffer = new uint8_t[clusterSize];
	memset(buffer, 0, clusterSize);

	writeCluster(volume, cluster, buffer);

	delete [] buffer;
}

uint32_t FAT::readFATEntry(VMGR::Volume* volume, uint32_t cluster)
{
	const uint8_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];
	const uint8_t activeFAT = volume->fsVariables[FAT_VAR_ACTIVE_FAT];

	uint32_t FATOffset, ThisFATSecNum, ThisFATEntOffset, FATEntryVal;

	if (FATType == 16)
		FATOffset = cluster * 2;
	else if (FATType == 32)
		FATOffset = cluster * 4;

	ThisFATSecNum = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (activeFAT * FATSz) + (FATOffset / IDE_ATA_BYTESPERSECTOR);
	ThisFATEntOffset = (FATOffset % IDE_ATA_BYTESPERSECTOR);

	uint8_t* buffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];	
	((PMGR::Partition*)(volume->Target))->readData(ThisFATSecNum, 1, buffer);

	if (FATType == 16)
		FATEntryVal = *((uint16_t*)&buffer[ThisFATEntOffset]);
	else if (FATType == 32)
		FATEntryVal = (*((uint32_t*)&buffer[ThisFATEntOffset])) & 0x0FFFFFFF;

	delete [] buffer;

	return FATEntryVal;
}

uint32_t FAT::readFATEntry(VMGR::Volume* volume, uint8_t* fat, uint32_t cluster)
{
	uint8_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];

	uint32_t FATOffset, FATEntryVal;

	if (FATType == 16)
		FATOffset = cluster * 2;
	else if (FATType == 32)
		FATOffset = cluster * 4;

	if (FATType == 16)
		FATEntryVal = *((uint16_t*)&fat[FATOffset]);
	else if (FATType == 32)
		FATEntryVal = (*((uint32_t*)&fat[FATOffset])) & 0x0FFFFFFF;

	return FATEntryVal;
}

void FAT::writeFATEntry(VMGR::Volume* volume, uint32_t cluster, uint32_t entry)
{
	const uint8_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];
	const uint8_t FATNum = volume->fsVariables[FAT_VAR_BPB_NUMFATS];

	uint32_t FATOffset, ThisFATSecNum, ThisFATEntOffset;

	if (FATType == 16)
		FATOffset = cluster * 2;
	else if (FATType == 32)
		FATOffset = cluster * 4;

	uint8_t* buffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];

	for (uint8_t i = 0; i < FATNum; i++)
	{
		if (!volume->fsVariables[FAT_VAR_MIRRORING])
			i = volume->fsVariables[FAT_VAR_ACTIVE_FAT];

		ThisFATSecNum = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (i * FATSz) + (FATOffset / IDE_ATA_BYTESPERSECTOR);
		ThisFATEntOffset = (FATOffset % IDE_ATA_BYTESPERSECTOR);

		((PMGR::Partition*)(volume->Target))->readData(ThisFATSecNum, 1, buffer);

		if (FATType == 16)
			*((uint16_t*)&buffer[ThisFATEntOffset]) = (entry & 0xFFFF);
		else if (FATType == 32)
			(*((uint32_t*)&buffer[ThisFATEntOffset])) = (entry & 0x0FFFFFFF);
		
		((PMGR::Partition*)(volume->Target))->writeData(ThisFATSecNum, 1, buffer);

		if (!volume->fsVariables[FAT_VAR_MIRRORING])
			break;
	}

	readFAT(volume, (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT]);	//Update offline FAT version
	delete [] buffer;	
}

uint8_t FAT::processLongEntry(const FATLongEntry* entry, char* dstStr)
{
	uint8_t offset = ((entry->LDIR_Ord & (~FAT_LAST_LONG_ENTRY)) - 1) * FAT_LONG_ENTRY_SIZE;
	uint8_t currentIndex = 0;
	
	bool isLastEntry = ((entry->LDIR_Ord & FAT_LAST_LONG_ENTRY) == FAT_LAST_LONG_ENTRY);

	for (uint8_t i = 0; i < 5; i++)
	{
		if ((entry->LDIR_Name1[i] == 0x0000) || (entry->LDIR_Name1[i] == 0xFFFF))
		{
			if (isLastEntry)
				dstStr[offset + currentIndex] = '\0';

			return entry->LDIR_Chksum;
		}

		dstStr[offset + currentIndex] = (char)entry->LDIR_Name1[i];
		currentIndex++;
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		if ((entry->LDIR_Name2[i] == 0x0000) || (entry->LDIR_Name2[i] == 0xFFFF))
		{
			if (isLastEntry)
				dstStr[offset + currentIndex] = '\0';

			return entry->LDIR_Chksum;
		}

		dstStr[offset + currentIndex] = (char)entry->LDIR_Name2[i];
		currentIndex++;
	}

	for (uint8_t i = 0; i < 2; i++)
	{
		if ((entry->LDIR_Name3[i] == 0x0000) || (entry->LDIR_Name3[i] == 0xFFFF))
		{
			if (isLastEntry)
				dstStr[offset + currentIndex] = '\0';

			return entry->LDIR_Chksum;
		}

		dstStr[offset + currentIndex] = (char)entry->LDIR_Name3[i];
		currentIndex++;
	}

	return entry->LDIR_Chksum;
}

void FAT::processShortEntry(const FATEntry* entry, char* dstStr)
{
	char fileNameWithoutExt[9];
	strncpy(fileNameWithoutExt, (char*)entry->DIR_Name, 8);
	fileNameWithoutExt[8] = '\0';
	strtrim(fileNameWithoutExt);	
	uint8_t lengthWithoutExt = strlen(fileNameWithoutExt);

	char extension[4];
	strncpy(extension, ((char*)(entry->DIR_Name) + 8), 3);
	extension[3] = '\0';
	strtrim(extension);		
	uint8_t extensionLength = strlen(extension);

	if (GET_BIT(entry->DIR_NTRes, 3))
		strtolower(fileNameWithoutExt);

	strcpy(dstStr, fileNameWithoutExt);
	
	if (extensionLength > 0)
	{
		dstStr[lengthWithoutExt] = '.';

		if (GET_BIT(entry->DIR_NTRes, 4))
			strtolower(extension);

		strcpy(dstStr + lengthWithoutExt + 1, extension);
		dstStr[lengthWithoutExt + extensionLength + 1] = '\0';
	}
	else
	{
		dstStr[lengthWithoutExt] = '\0';
	}
}

uint8_t FAT::shortNameCheckSum(uint8_t* filename)
{
	uint8_t sum = 0;
	
	for (uint8_t len = 11; len != 0; len--)
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *filename++;
	
	return (sum);
}

uint16_t FAT::createDateValue(HAL::DateTime date)
{
	uint16_t days = (date.Day & 0x001F);
	uint16_t months = ((date.Month << 5) & 0x01E0);
	uint16_t years = (((date.Year - 1980) << 9) & 0xFE00);

	return (years | months | days);
}

uint16_t FAT::createTimeValue(HAL::DateTime time)
{
	uint16_t seconds = ((time.Seconds / 2) & 0x001F);
	uint16_t minutes = ((time.Minutes << 5) & 0x07E0);
	uint16_t hours = (((time.Hours) << 11) & 0xF800);

	return (hours | minutes | seconds);
}

const char* FAT::getFAT32VolumeLabel(VMGR::Volume* volume)
{
	if (volume == NULL)
		return NULL;

	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];

	if (FATType != 32)
		return NULL;

	const uint32_t EOF = FAT_FAT32_EOF;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	const uint16_t entriesPerCluster = clusterSize / FAT_ENTRY_SIZE;
	uint8_t* offlineFAT = (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT];
	
	uint8_t* buffer = new uint8_t[clusterSize];

	uint32_t cluster = volume->fsVariables[FAT_VAR_BPB_ROOTCLUS];
	while ((cluster < EOF) && (cluster > 1))
	{
		readCluster(volume, cluster, buffer);

		static char longFileName[255];
		static uint8_t lfnChecksum = 0;
		static char shortFileName[13];

		for (uint16_t i = 0; i < entriesPerCluster; i++)
		{
			FATEntry* entry = &((FATEntry*)buffer)[i];

			if ((entry->DIR_Name[0] != 0xE5) && (entry->DIR_Name[0] != 0x00))
			{
				if (entry->DIR_Attr.ATTR_READ_ONLY	&&
					entry->DIR_Attr.ATTR_HIDDEN 	&&
					entry->DIR_Attr.ATTR_SYSTEM		&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					//Long File Name Entry					
					lfnChecksum = processLongEntry((FATLongEntry*)entry, longFileName);
				}
				else
				{
					char* fileName;

					processShortEntry(entry, shortFileName);					

					if (lfnChecksum == shortNameCheckSum(entry->DIR_Name))
						fileName = longFileName;
					else
						fileName = shortFileName;

					if (!entry->DIR_Attr.ATTR_DIRECTORY	&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
					{
						delete [] buffer;
						return fileName;
					}

					longFileName[0] = '\0';
				}
			}
		}

		cluster = readFATEntry(volume, offlineFAT, cluster);
	}

	delete [] buffer;
	return NULL;
}

FAT::FATEntry* FAT::findFreeEntry(VMGR::Volume* volume, uint8_t* buffer, uint32_t& cluster, uint8_t num)
{
	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	const uint16_t entriesPerCluster = clusterSize / FAT_ENTRY_SIZE;
	const uint16_t rootEntries = volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT];
	uint8_t* offlineFAT = (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT];

	bool rootDirectory = false;

	if (cluster == 0)
		rootDirectory = true;

	uint32_t totalEntries = 0;

	while ((cluster < EOF) && ((cluster > 1) || rootDirectory))
	{
		readCluster(volume, cluster, buffer);	

		FATEntry* foundEntry = NULL;
		uint8_t foundCount = 0;

		for (uint16_t i = 0; i < (rootDirectory ? rootEntries : entriesPerCluster); i++)
		{
			FATEntry* entry = &((FATEntry*)buffer)[i];

			if ((entry->DIR_Name[0] == 0xE5) || (entry->DIR_Name[0] == 0x00)) //Indicates entry is free
			{
				if ((foundEntry != NULL) && (foundEntry == (entry - foundCount)))
				{
					foundCount++;
				}
				else
				{
					foundEntry = entry;
					foundCount = 1;
				}
			}
			else
			{
				totalEntries++;
			}

			if (totalEntries > FAT_MAX_ENTRIES)
				return NULL;

			if (foundCount == num)
				return foundEntry;
		}

		if (rootDirectory)
			break;

		cluster = readFATEntry(volume, offlineFAT, cluster);
	}

	return NULL;
}

uint32_t FAT::findFreeCluster(VMGR::Volume* volume)
{
	if (volume == NULL)
		return NULL;

	const uint32_t nextFreeCluster = volume->fsVariables[FAT_VAR_NEXT_CLUSTER];
	const uint32_t clusterCount = volume->fsVariables[FAT_VAR_CLUSTER_COUNT];
	uint8_t* offlineFAT = (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT];

	for (uint32_t i = nextFreeCluster; i <= (clusterCount + 1); i++)
	{
		if (readFATEntry(volume, offlineFAT, i) == 0)
		{
			volume->fsVariables[FAT_VAR_NEXT_CLUSTER] = i;
			volume->fsVariables[FAT_VAR_FREE_COUNT]--;
			return i;
		}
	}

	for (uint32_t i = 2; i < nextFreeCluster; i++)
	{
		if (readFATEntry(volume, offlineFAT, i) == 0)
		{
			volume->fsVariables[FAT_VAR_NEXT_CLUSTER] = i;
			volume->fsVariables[FAT_VAR_FREE_COUNT]--;
			return i;
		}
	}

	return NULL;
}

void FAT::createLongEntry(FATLongEntry* entry, uint8_t order, char* srcStr, uint8_t chksum)
{
	uint32_t len = strlen(srcStr);
	uint8_t offset = (order - 1) * FAT_LONG_ENTRY_SIZE;
	uint8_t remainingChars = len - offset;
	uint8_t currentIndex = 0;
	bool isLastEntry = (((remainingChars + 1) <= FAT_LONG_ENTRY_SIZE) ? true : false);	//plus one for the null terminating character

	entry->LDIR_Ord = order | (isLastEntry ? FAT_LAST_LONG_ENTRY : 0);
	entry->LDIR_Attr = FAT_ATTR_LONG_NAME;
	entry->LDIR_Type = 0;
	entry->LDIR_Chksum = chksum;
	entry->LDIR_FstClusLO = 0;

	for (uint8_t i = 0; i < 5; i++)
	{
		if (currentIndex < remainingChars)
		{
			entry->LDIR_Name1[i] = srcStr[offset + currentIndex];
			currentIndex++;
		}
		else if (currentIndex == remainingChars)
		{
			entry->LDIR_Name1[i] = 0x0000;
			currentIndex++;
		}
		else
		{
			entry->LDIR_Name1[i] = 0xFFFF;
			currentIndex++;
		}
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		if (currentIndex < remainingChars)
		{
			entry->LDIR_Name2[i] = srcStr[offset + currentIndex];
			currentIndex++;
		}
		else if (currentIndex== remainingChars)
		{
			entry->LDIR_Name2[i] = 0x0000;
			currentIndex++;
		}
		else
		{
			entry->LDIR_Name2[i] = 0xFFFF;
			currentIndex++;
		}
	}

	for (uint8_t i = 0; i < 2; i++)
	{
		if (currentIndex < remainingChars)
		{
			entry->LDIR_Name3[i] = srcStr[offset + currentIndex];
			currentIndex++;
		}
		else if (currentIndex == remainingChars)
		{
			entry->LDIR_Name3[i] = 0x0000;
			currentIndex++;
		}
		else
		{
			entry->LDIR_Name3[i] = 0xFFFF;
			currentIndex++;
		}
	}
}

bool FAT::isValid83Char(char c)
{
	if ((c == '+') ||
		(c == ',') ||
		(c == ';') ||
		(c == '=') ||
		(c == '[') ||
		(c == ']'))
		return false;

	return true;
}

bool FAT::generateShortName(char* srcLong, char* dstShort, uint8_t& NTRes, VMGR::File** fileList, uint32_t fileListLength)
{
	bool lossyConversion = false;
	bool lfnFits83 = false;

	uint32_t len = strlen(srcLong);
	char* src = new char[len + 1];
	strcpy(src, srcLong);

	strtoupper(src);
	lossyConversion = false;

	//replacing illegal chars with underscores & removing spaces
	for (uint32_t i = 0; i < len; i++)
	{
		if (!isValid83Char(src[i]))
		{
			lossyConversion = true;
			src[i] = '_';
		}

		if (src[i] == ' ')
			src[i] = '\0';
	}

	//removing leading periods
	for (uint32_t i = 0; i < len; i++)
	{
		if (src[i] == '.')
			src[i] = '\0';
		else
			break;
	}

	//8.3 filename is trailing space padded
	memset(dstShort, ' ', 11);

	//Basename	
	uint8_t baseChars = 0;
	uint8_t baseCopied = 0;

	if (islower(srcLong[0]))
		NTRes = SET_BIT(NTRes, 3);

	for (uint32_t i = 0; i < len; i++)
	{
		if (src[i] == '\0')
			continue;

		if (src[i] == '.')
			break;

		if (baseCopied < 8)
			dstShort[baseCopied++] = src[i];

		baseChars++;
	}

	if (baseChars == baseCopied)
		lfnFits83 = true;

	//Extension
	uint8_t extChars = 0;
	uint8_t extCopied = 0;

	for (int32_t i = len; i >= 0; i--)
	{
		if (src[i] == '\0')
			continue;

		if (src[i] == '.')
		{
			if (islower(srcLong[i + 1]))
				NTRes = SET_BIT(NTRes, 4);

			for (uint32_t j = i + 1; j < len; j++)
			{
				if (extCopied < 3)
					dstShort[8 + extCopied++] = src[j];

				extChars++;
			}

			if (extChars != extCopied)
				lfnFits83 = false;

			break;
		}
	}

	bool collides = false;

	for (uint32_t i = 0; i < fileListLength; i++)
	{
		collides = true;
		
		for (uint8_t j = 0; j < 11; j++)
		{			
			if (fileList[i]->Name[j] != dstShort[j])
			{
				collides = false;
				break;
			}
		}

		if (collides)
			break;
	}

	bool needLFN = false;

	if (!lfnFits83 || lossyConversion || collides)
	{
		needLFN = true;
		uint8_t tailNo = 1;

		if (baseCopied <= 6)
		{
			dstShort[baseCopied] = '~';
			dstShort[baseCopied + 1] = itoa(tailNo++)[0];
		}
		else
		{
			dstShort[6] = '~';
			dstShort[7] = itoa(tailNo++)[0];
		}	

		do
		{

			for (uint32_t i = 0; i < fileListLength; i++)
			{
				collides = true;

				for (uint8_t j = 0; j < 11; j++)
				{
					if (fileList[i]->Name[j] != dstShort[j])
					{
						collides = false;
						break;
					}
				}

				if (collides)
					break;
			}

			if (collides)
			{
				uint32_t rnd = HAL::random(0xFF);

				if (baseCopied <= 6)
				{
					if (tailNo > 9)
					{
						const char* rndHex = xtoa(rnd);

						if (baseCopied >= 2)
							dstShort[baseCopied - 2] = rndHex[0];

						if (baseCopied >= 1)
							dstShort[baseCopied - 1] = rndHex[1];

						tailNo = 1;
					}
					else
					{
						dstShort[baseCopied] = '~';
						dstShort[baseCopied + 1] = itoa(tailNo++)[0];
					}
				}
				else
				{
					if (tailNo > 9)
					{
						const char* rndHex = xtoa(rnd);
						dstShort[4] = rndHex[0];
						dstShort[5] = rndHex[1];

						tailNo = 1;
					}
					else
					{
						dstShort[6] = '~';
						dstShort[7] = itoa(tailNo++)[0];
					}
				}
			}
		}
		while (collides);
	}

	delete [] src;
	return needLFN;
}

FAT::FAT()
{
	Name = "FAT";
}

bool FAT::systemID(uint8_t sysID)
{
	if ((sysID == 0x04) ||		//DOS 3.0+ 16-bit FAT (up to 32M)
		(sysID == 0x05) ||		//DOS 3.3+ Extended Partition
		(sysID == 0x06) ||		//DOS 3.31+ 16-bit FAT (over 32M)
		(sysID == 0x0B))		//WIN95 OSR2 32-bit FAT
		return true;

	return false;
}

void FAT::init(VMGR::Volume* volume)
{
	if (volume == NULL)
		return;

	uint8_t* buffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];
	((PMGR::Partition*)volume->Target)->readData(0, 1, buffer);

	FATCommonBPB* commonBPB = ((FATCommonBPB*)((uint32_t)buffer + FAT_COMMON_BPB_OFFSET));
	FAT16BPB* fat16BPB = ((FAT16BPB*)((uint32_t)buffer + FAT_FAT16_BPB_OFFSET));
	FAT32BPB* fat32BPB = ((FAT32BPB*)((uint32_t)buffer + FAT_FAT32_BPB_OFFSET));

	if (commonBPB->BPB_BytsPerSec != IDE_ATA_BYTESPERSECTOR)
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

	if (FATType == 12)	//FAT12 is not and won't be supported
	{
		delete [] buffer;
		return;
	}

	//Set volume filesystem variables to save all needed values for later use
	volume->fsVariables[FAT_VAR_FATTYPE] = FATType;
	volume->fsVariables[FAT_VAR_FATSZ] = FATSz;
	volume->fsVariables[FAT_VAR_BPB_NUMFATS] = commonBPB->BPB_NumFATs;
	volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] = commonBPB->BPB_ResvdSecCnt;
	volume->fsVariables[FAT_VAR_BPB_SECPERCLUS] = commonBPB->BPB_SecPerClus;
	volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] = commonBPB->BPB_RootEntCnt;
	volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] = fat32BPB->BPB_RootClus;
	volume->fsVariables[FAT_VAR_CLUSTER_COUNT] = CountofClusters;

	uint8_t* offlineFAT = new uint8_t[FATSz * IDE_ATA_BYTESPERSECTOR];
	readFAT(volume, offlineFAT);

	volume->fsVariables[FAT_VAR_NEXT_CLUSTER] = 0x02;
	volume->fsVariables[FAT_VAR_FREE_COUNT] = 0;

	volume->fsVariables[FAT_VAR_OFFLINE_FAT] = (uint32_t)offlineFAT;
	volume->fsVariables[FAT_VAR_MIRRORING] = true;
	volume->fsVariables[FAT_VAR_ACTIVE_FAT] = 0;

	if (FATType == 32)
	{
		volume->fsVariables[FAT_VAR_MIRRORING] = ((fat32BPB->BPB_ExtFlags & 0x80) == 0);
		volume->fsVariables[FAT_VAR_ACTIVE_FAT] = (fat32BPB->BPB_ExtFlags & 0x0F);

		uint8_t* fsiBuffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];
		((PMGR::Partition*)volume->Target)->readData(fat32BPB->BPB_FSInfo, 1, fsiBuffer);

		FAT32FSInfo* fsInfo = (FAT32FSInfo*)fsiBuffer;

		if ((fsInfo->FSI_LeadSig == 0x41615252) && 
			(fsInfo->FSI_StrucSig == 0x61417272) && 
			(fsInfo->FSI_TrailSig == 0xAA550000))
		{
			if (fsInfo->FSI_Nxt_Free != 0xFFFFFFFF)
				volume->fsVariables[FAT_VAR_NEXT_CLUSTER] = fsInfo->FSI_Nxt_Free;				

			if (fsInfo->FSI_Free_Count != 0xFFFFFFFF)
				volume->fsVariables[FAT_VAR_FREE_COUNT] = fsInfo->FSI_Free_Count;				
		}

		delete [] fsiBuffer;

		const char* volumeName = getFAT32VolumeLabel(volume);

		if (volumeName != NULL)
			strcpy(volume->Name, volumeName);
		else
		{
			strncpy(volume->Name, (const char*)&fat32BPB->BS_VolLab, 11);
			volume->Name[11] = '\0';
			strtrim(volume->Name);
		}		
	}
	else
	{
		strncpy(volume->Name, (const char*)&fat16BPB->BS_VolLab, 11);
		volume->Name[11] = '\0';
		strtrim(volume->Name);
	}

	delete [] buffer;

	volume->Online = true;
}

uint64_t FAT::getFreeSpace(VMGR::Volume* volume)
{
	if ((volume == NULL) || (!volume->Online))
		return 0;

	const uint32_t clusterCount = volume->fsVariables[FAT_VAR_CLUSTER_COUNT];
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	uint8_t* offlineFAT = (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT];
	
	if (volume->fsVariables[FAT_VAR_FREE_COUNT] != 0)
		return (volume->fsVariables[FAT_VAR_FREE_COUNT] * clusterSize);

	uint32_t freeClusters = 0;

	for (uint32_t i = 2; i <= (clusterCount + 1); i++)
		if (readFATEntry(volume, offlineFAT, i) == 0)
			freeClusters++;

	volume->fsVariables[FAT_VAR_FREE_COUNT] = freeClusters;

	return (freeClusters * clusterSize);
}

void FAT::release(VMGR::Volume* volume)
{
	if ((volume == NULL) || (!volume->Online))
		return;

	delete [] (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT];
	volume->fsVariables[FAT_VAR_OFFLINE_FAT] = NULL;

	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];

	if (FATType == 32)
	{
		uint8_t* fsiBuffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];
		((PMGR::Partition*)volume->Target)->readData(0, 1, fsiBuffer);
		FAT32BPB* fat32BPB = ((FAT32BPB*)((uint32_t)fsiBuffer + FAT_FAT32_BPB_OFFSET));

		uint16_t fsiSector = fat32BPB->BPB_FSInfo;

		((PMGR::Partition*)volume->Target)->readData(fsiSector, 1, fsiBuffer);

		FAT32FSInfo* fsInfo = (FAT32FSInfo*)fsiBuffer;

		if ((fsInfo->FSI_LeadSig == 0x41615252) && 
			(fsInfo->FSI_StrucSig == 0x61417272) && 
			(fsInfo->FSI_TrailSig == 0xAA550000))
		{
			fsInfo->FSI_Nxt_Free = volume->fsVariables[FAT_VAR_NEXT_CLUSTER];
			fsInfo->FSI_Free_Count = volume->fsVariables[FAT_VAR_FREE_COUNT];		
		}

		((PMGR::Partition*)volume->Target)->writeData(fsiSector, 1, fsiBuffer);

		delete [] fsiBuffer;
	}

	volume->Online = false;
}

bool FAT::create(VMGR::Volume* volume, Path* filePath, VMGR::File* dstFile, bool isDirectory)
{	
	if ((volume == NULL) || (!volume->Online))
		return false;

	if (filePath == NULL)
		return false;

	if (find(volume, filePath, NULL))	//another file with the same name already exists
		return false;

	Path* parentDir = filePath->getParentPath();
	VMGR::File* directory = new VMGR::File();

	if (!find(volume, parentDir, directory))
	{
		delete directory;
		delete parentDir;
		return false;
	}
	else if (!directory->isDirectory)
	{
		delete directory;
		return false;
	}

	char* filename = filePath->getFilename();

	if (filename == NULL)
	{
		delete directory;
		return false;
	}

	uint16_t len = strlen(filename) + 1;
	uint8_t longEntryCount = (len + (FAT_LONG_ENTRY_SIZE - 1)) / FAT_LONG_ENTRY_SIZE;

	if ((len == 0) || (len > FAT_MAX_LONG_FILE_NAME))
	{
		delete filename;
		delete directory;
		return false;
	}

	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint32_t EOC = (FATType == 32 ? FAT_FAT32_EOC_MARK : FAT_FAT16_EOC_MARK);
	const uint16_t rootDirSize = volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	uint8_t* offlineFAT = (uint8_t*)volume->fsVariables[FAT_VAR_OFFLINE_FAT];

	uint8_t* buffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];

	char* shortFileName = new char[12];
	shortFileName[11] = '\0';

	HAL::DateTime now = HAL::getCurrentDateTime();

	VMGR::File** fileList = NULL;
	uint32_t fileListLength = list(directory, fileList, true);

	uint8_t NTRes = 0;
	bool needLFN = generateShortName(filename, shortFileName, NTRes, fileList, fileListLength);

	for (uint32_t i = 0; i < fileListLength; i++)
		delete fileList[i];

	delete [] fileList;

	uint32_t cluster = directory->firstCluster;
	FATEntry* entry = NULL;

	if (needLFN)
		entry = findFreeEntry(volume, buffer, cluster, longEntryCount + 1);
	else
		entry = findFreeEntry(volume, buffer, cluster);

	if ((entry == NULL) || (cluster >= EOF))
	{
		uint32_t newCluster = findFreeCluster(volume);

		if ((cluster == 0) || (newCluster == NULL))
		{
			//ERROR: Not enough disk space
			delete [] shortFileName;
			delete [] buffer;
			delete filename;
			delete directory;
			return false;
		}

		cluster = directory->firstCluster;
		uint32_t lastUsedCluster = 0;

		while (cluster < EOF)
		{
			lastUsedCluster = cluster;
			cluster = readFATEntry(volume, offlineFAT, cluster);
		}

		writeFATEntry(volume, lastUsedCluster, newCluster);
		writeFATEntry(volume, newCluster, EOC);

		cluster = newCluster;
		clearCluster(volume, cluster);

		if (needLFN)
			entry = findFreeEntry(volume, buffer, cluster, longEntryCount + 1);
		else
			entry = findFreeEntry(volume, buffer, cluster);
	}

	if (needLFN)
	{
		uint8_t chksum = shortNameCheckSum((uint8_t*)shortFileName);

		for (uint8_t i = longEntryCount; i > 0; i--, entry++)
			createLongEntry((FATLongEntry*)entry, i, filename, chksum);
	}

	strncpy((char*)entry->DIR_Name, shortFileName, 11);
	entry->DIR_Attr.clear();
	entry->DIR_NTRes = NTRes;
	entry->DIR_CrtTimeTenth = 0;
	entry->DIR_CrtTime = createTimeValue(now);
	entry->DIR_CrtDate = createDateValue(now);
	entry->DIR_LstAccDate = createDateValue(now);
	entry->DIR_WrtTime = createTimeValue(now);
	entry->DIR_WrtDate = createDateValue(now);
	entry->DIR_FileSize = 0;

	delete [] shortFileName;

	if (isDirectory)
	{
		uint32_t newCluster = findFreeCluster(volume);

		if (newCluster == NULL)
		{
			delete [] buffer;
			delete filename;
			delete directory;
			return false;
		}

		writeFATEntry(volume, newCluster, EOC);
		entry->DIR_Attr.ATTR_DIRECTORY = true;
		entry->DIR_FstClusHI = HIWORD(newCluster);
		entry->DIR_FstClusLO = LOWORD(newCluster);

		uint8_t* newBuffer = new uint8_t[clusterSize];
		memset(newBuffer, 0, clusterSize);

		FATEntry* dirEntry = (FATEntry*)newBuffer;
		memset(dirEntry->DIR_Name, ' ', 11);
		dirEntry->DIR_Name[0] = '.';
		dirEntry->DIR_Attr.clear();
		dirEntry->DIR_Attr.ATTR_DIRECTORY = true;
		dirEntry->DIR_NTRes = 0;
		dirEntry->DIR_CrtTimeTenth = 0;
		dirEntry->DIR_CrtTime = createTimeValue(now);
		dirEntry->DIR_CrtDate = createDateValue(now);
		dirEntry->DIR_LstAccDate = createDateValue(now);
		dirEntry->DIR_WrtTime = createTimeValue(now);
		dirEntry->DIR_WrtDate = createDateValue(now);
		dirEntry->DIR_FileSize = 0;
		dirEntry->DIR_FstClusHI = HIWORD(newCluster);
		dirEntry->DIR_FstClusLO = LOWORD(newCluster);

		dirEntry++;

		memset(dirEntry->DIR_Name, ' ', 11);
		dirEntry->DIR_Name[0] = '.';
		dirEntry->DIR_Name[1] = '.';
		dirEntry->DIR_Attr.clear();
		dirEntry->DIR_Attr.ATTR_DIRECTORY = true;
		dirEntry->DIR_NTRes = 0;
		dirEntry->DIR_CrtTimeTenth = 0;
		dirEntry->DIR_CrtTime = createTimeValue(now);
		dirEntry->DIR_CrtDate = createDateValue(now);
		dirEntry->DIR_LstAccDate = createDateValue(now);
		dirEntry->DIR_WrtTime = createTimeValue(now);
		dirEntry->DIR_WrtDate = createDateValue(now);
		dirEntry->DIR_FileSize = 0;
		dirEntry->DIR_FstClusHI = HIWORD(directory->firstCluster);
		dirEntry->DIR_FstClusLO = LOWORD(directory->firstCluster);

		writeCluster(volume, newCluster, newBuffer);

		delete [] newBuffer;
	}
	else
	{
		entry->DIR_FstClusHI = 0;
		entry->DIR_FstClusLO = 0;
	}

	writeCluster(volume, cluster, buffer);

	if (dstFile != NULL)
	{
		dstFile->Name = new char[strlen(filename) + 1];
		strcpy(dstFile->Name, filename);
		dstFile->Name[strlen(filename)] = '\0';

		//dstFile->filePath = filePath;
		dstFile->parentVolume = volume;
		dstFile->isDirectory = isDirectory;
		dstFile->firstCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
		dstFile->fileSize = 0;
		dstFile->isOpen = true;
		dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = (uint32_t)cluster;
		dstFile->fVariables[FAT_FILE_VAR_ENTRY_INDEX] = (uint32_t)(entry -  (FATEntry*)buffer);
	}

	delete [] buffer;
	delete filename;
	delete directory;

	return true;
}

bool FAT::find(VMGR::Volume* volume, Path* filePath, VMGR::File* dstFile)
{
	if ((volume == NULL) || (!volume->Online))
		return false;

	if ((filePath == NULL) || (!filePath->isAbsolute))
		return false;

	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint16_t rootDirSize = volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	const uint16_t entriesPerCluster = clusterSize / FAT_ENTRY_SIZE;
	const uint16_t rootEntries = volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT];
	uint8_t* offlineFAT = (uint8_t*)(volume->fsVariables[FAT_VAR_OFFLINE_FAT]);

	uint16_t currentIndex = 1;
	char* currentFileName = filePath->getComponent(currentIndex++);

	if (strlen(currentFileName) == 0)	//Path points to root directory
	{
		if (dstFile != NULL)
		{
			dstFile->Name = new char[2];
			strcpy(dstFile->Name, "/");

			dstFile->filePath = filePath;
			dstFile->parentVolume = volume;
			dstFile->isDirectory = true;

			if (FATType == 32)
				dstFile->firstCluster = volume->fsVariables[FAT_VAR_BPB_ROOTCLUS];
			else
				dstFile->firstCluster = 0;
			
			dstFile->fileSize = 0;
			dstFile->isOpen = true;
			dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = 0;
			dstFile->fVariables[FAT_FILE_VAR_ENTRY_INDEX] = 0;
		}

		delete [] currentFileName;
		return true;
	}

	uint8_t* buffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];
	uint32_t currentCluster = (FATType == 32 ? volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] : 0);

	readCluster(volume, currentCluster, buffer);	//Read root directory
	bool rootDirectory = (FATType == 32 ? false : true);

	bool quit = false;

	while (!quit && (currentFileName != NULL))
	{
		static char longFileName[255];
		static uint8_t lfnChecksum = 0;
		static char shortFileName[13];		

		for (uint16_t i = 0; i < (rootDirectory ? rootEntries : entriesPerCluster); i++)
		{
			FATEntry* entry = &((FATEntry*)buffer)[i];

			if ((entry->DIR_Name[0] != 0xE5) && (entry->DIR_Name[0] != 0x00))
			{
				if (entry->DIR_Attr.ATTR_READ_ONLY	&&
					entry->DIR_Attr.ATTR_HIDDEN 	&&
					entry->DIR_Attr.ATTR_SYSTEM		&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					//Long File Name Entry
					lfnChecksum = processLongEntry((FATLongEntry*)entry, longFileName);
				}
				else
				{
					char* fileName;

					processShortEntry(entry, shortFileName);

					if (lfnChecksum == shortNameCheckSum(entry->DIR_Name))
						fileName = longFileName;
					else
						fileName = shortFileName;

					if ((strcmp(currentFileName, fileName) == 0) || (strcmp(currentFileName, shortFileName) == 0))
					{
						if (!entry->DIR_Attr.ATTR_DIRECTORY	&&
							!entry->DIR_Attr.ATTR_VOLUME_ID)
						{
							//This is a file entry	
							char* nextName = NULL;

							if ((nextName = filePath->getNextComponent()) == NULL)
							{
								//File FOUND
								if (dstFile != NULL)
								{
									dstFile->Name = new char[strlen(fileName) + 1];
									strcpy(dstFile->Name, fileName);
									dstFile->Name[strlen(fileName)] = '\0';

									dstFile->filePath = filePath;
									dstFile->parentVolume = volume;
									dstFile->isDirectory = false;
									dstFile->firstCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
									dstFile->fileSize = entry->DIR_FileSize;
									dstFile->isOpen = true;

									if (rootDirectory)
										dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = 0;
									else
										dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = currentCluster;

									dstFile->fVariables[FAT_FILE_VAR_ENTRY_INDEX] = i;									
								}

								delete [] nextName;
								delete [] currentFileName;
								delete [] buffer;				
								return true;
							}
							else
							{
								//ERROR: Invalid path (This isn't the last component in the path)
								delete [] nextName;
								delete [] currentFileName;
								delete [] buffer;
								return false;
							}
						}
						else if (	entry->DIR_Attr.ATTR_DIRECTORY	&&
									!entry->DIR_Attr.ATTR_VOLUME_ID)
						{
							//This is a directory entry
							char* nextName = NULL;

							if ((nextName = filePath->getNextComponent()) == NULL)
							{
								//directory FOUND
								if (dstFile != NULL)
								{
									dstFile->Name = new char[strlen(fileName) + 1];
									strcpy(dstFile->Name, fileName);
									dstFile->Name[strlen(fileName)] = '\0';

									dstFile->filePath = filePath;
									dstFile->parentVolume = volume;
									dstFile->isDirectory = true;
									dstFile->firstCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
									dstFile->fileSize = entry->DIR_FileSize;
									dstFile->isOpen = true;

									if (rootDirectory)
										dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = 0;
									else
										dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = currentCluster;

									dstFile->fVariables[FAT_FILE_VAR_ENTRY_INDEX] = i;
								}

								delete [] nextName;
								delete [] currentFileName;
								delete [] buffer;	
								return true;
							}
							else
							{
								currentCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
								readCluster(volume, currentCluster, buffer);
								
								rootDirectory = false;
								
								delete [] nextName;
								delete [] currentFileName;
								currentFileName = filePath->getComponent(currentIndex++);
								break;
							}
						}
						else
						{
							//This is an invalid entry
							//ERROR: Invalid entry
							delete [] currentFileName;
							delete [] buffer;
							return false;
						}
					}

					longFileName[0] = '\0';
				}				
			}

			if (i == entriesPerCluster - 1)
			{
				if (rootDirectory)
				{
					quit = true;
				}
				else
				{
					currentCluster = readFATEntry(volume, offlineFAT, currentCluster);

					if (currentCluster < EOF)
						readCluster(volume, currentCluster, buffer);
					else
						quit = true;
				}
			}
		}
	}

	delete [] currentFileName;
	delete [] buffer;

	return false;
}

uint32_t FAT::list(VMGR::File* directory, VMGR::File** &fileList, bool useShortNames)
{
	if ((directory == NULL) || (!directory->parentVolume->Online) || (!directory->isOpen) || (!directory->isDirectory))
		return 0;

	const uint16_t FATType = directory->parentVolume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint16_t rootDirSize = directory->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * directory->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	const uint16_t entriesPerCluster = clusterSize / FAT_ENTRY_SIZE;
	const uint16_t rootEntries = directory->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT];
	uint8_t* offlineFAT = (uint8_t*)directory->parentVolume->fsVariables[FAT_VAR_OFFLINE_FAT];

	VMGR::File** output = new VMGR::File*[entriesPerCluster];
	uint16_t outputSize = entriesPerCluster;
	uint16_t outputCount = 0;

	uint8_t* buffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];
	uint32_t cluster = directory->firstCluster;
	
	bool rootDirectory = false;

	if (cluster == 0)	//Cluster is zero if it is root directory before FAT32
		rootDirectory = true;

	while ((cluster < EOF) && ((cluster > 1) || rootDirectory))
	{		
		readCluster(directory->parentVolume, cluster, buffer);

		static char longFileName[255];
		static uint8_t lfnChecksum = 0;
		static char shortFileName[13];

		for (uint16_t i = 0; i < (rootDirectory ? rootEntries : entriesPerCluster); i++)
		{
			FATEntry* entry = &((FATEntry*)buffer)[i];
			
			if ((entry->DIR_Name[0] != 0xE5) && (entry->DIR_Name[0] != 0x00))
			{
				if (entry->DIR_Attr.ATTR_READ_ONLY	&&
					entry->DIR_Attr.ATTR_HIDDEN 	&&
					entry->DIR_Attr.ATTR_SYSTEM		&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					//Long File Name Entry
					if (!useShortNames)
						lfnChecksum = processLongEntry((FATLongEntry*)entry, longFileName);
				}
				else
				{
					char* fileName;

					processShortEntry(entry, shortFileName);					

					if (!useShortNames && (lfnChecksum == shortNameCheckSum(entry->DIR_Name)))
						fileName = longFileName;
					else
						fileName = shortFileName;

					if (!entry->DIR_Attr.ATTR_VOLUME_ID)
					{

						uint32_t len = strlen(fileName);
						VMGR::File* dstFile = new VMGR::File();

						if (useShortNames)
						{
							dstFile->Name = new char[12];
							strncpy(dstFile->Name, (char*)entry->DIR_Name, 11);
							dstFile->Name[11] = '\0';
							dstFile->filePath = NULL;
						}
						else
						{
							dstFile->Name = new char[len + 1];
							strcpy(dstFile->Name, fileName);
							dstFile->Name[len] = '\0';
							dstFile->filePath = directory->filePath->addComponent(dstFile->Name);
						}

						dstFile->parentVolume = directory->parentVolume;
						dstFile->isDirectory = entry->DIR_Attr.ATTR_DIRECTORY;
						dstFile->firstCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
						dstFile->fileSize = entry->DIR_FileSize;
						dstFile->isOpen = true;
						
						if (rootDirectory)
							dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = 0;
						else
							dstFile->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER] = cluster;

						dstFile->fVariables[FAT_FILE_VAR_ENTRY_INDEX] = i;

						output[outputCount++] = dstFile;

						if (outputCount == outputSize)
						{
							VMGR::File** newOutput = new VMGR::File*[outputSize + entriesPerCluster];
							
							memcpy(newOutput, output, (outputCount * sizeof(VMGR::File*)));
							delete [] output;
							
							output = newOutput;
							outputSize += entriesPerCluster;
						}
					}

					longFileName[0] = '\0';
				}
			}
		}

		if (rootDirectory)
			break;

		cluster = readFATEntry(directory->parentVolume, offlineFAT, cluster);		
	}

	delete [] buffer;

	if (outputCount != outputSize)
	{
		VMGR::File** newOutput = new VMGR::File*[outputCount];
		
		memcpy(newOutput, output, sizeof(VMGR::File*) * outputCount);
		delete [] output;
		
		output = newOutput;
		outputSize = outputCount;
	}

	fileList = output;
	return outputCount;
}

uint32_t FAT::list(VMGR::File* directory, VMGR::File** &fileList)
{
	return list(directory, fileList, false);
}

bool FAT::remove(VMGR::File* file, bool recursive)
{
	if ((file == NULL) || (!file->parentVolume->Online) || (!file->isOpen))
		return false;

	const uint8_t FATType = file->parentVolume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint16_t rootDirSize = file->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	const uint16_t entriesPerCluster = clusterSize / FAT_ENTRY_SIZE;
	const uint16_t rootEntries = file->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT];
	uint8_t* offlineFAT = (uint8_t*)file->parentVolume->fsVariables[FAT_VAR_OFFLINE_FAT];

	if (file->isDirectory)
	{
		VMGR::File** fileList = NULL;
		uint32_t fileListLength = list(file, fileList);

		if ((fileListLength < 2) ||
			((strcmp(fileList[0]->Name, ".") != 0) && 
			(strcmp(fileList[1]->Name, "..") != 0)))
		{
			//This is the root directory, you can't delete this
			for (uint32_t i = 0; i < fileListLength; i++)
					delete fileList[i];
				delete [] fileList;

			return false;
		}

		if (!recursive && fileListLength > 2)
		{
			for (uint32_t i = 0; i < fileListLength; i++)
					delete fileList[i];
				delete [] fileList;

			return false;
		}
		else
		{
			for (uint32_t i = 2; i < fileListLength; i++)
			{
				if (!remove(fileList[i], recursive))
				{
					for (uint32_t i = 0; i < fileListLength; i++)
						delete fileList[i];
					delete [] fileList;

					return false;
				}
			}
		}

		for (uint32_t i = 0; i < fileListLength; i++)
			delete fileList[i];
		delete [] fileList;
	}

	uint32_t cluster = file->firstCluster;

	while ((cluster < EOF) && (cluster > 1))
	{
		uint32_t clusterToRemove = cluster;
		cluster = readFATEntry(file->parentVolume, offlineFAT, cluster);
		writeFATEntry(file->parentVolume, clusterToRemove, 0);
	}

	Path* parentPath = file->filePath->getParentPath();
	VMGR::File* parentDir = new VMGR::File;

	if (!find(file->parentVolume, parentPath, parentDir))
	{
		delete parentDir;
		delete parentPath;
		return false;
	}

	uint32_t parentCluster = parentDir->firstCluster;
	uint32_t entryCluster = file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER];
	uint32_t entryIndex = file->fVariables[FAT_FILE_VAR_ENTRY_INDEX];

	uint8_t* buffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];

	readCluster(file->parentVolume, entryCluster, buffer);

	FATEntry* entryToRemove = &((FATEntry*)buffer)[entryIndex];
	uint8_t chksum = shortNameCheckSum(entryToRemove->DIR_Name);	//Used to check for LFN entries	
	entryToRemove->DIR_Name[0] = 0xE5;

	writeCluster(file->parentVolume, entryCluster, buffer);

	bool rootDirectory = false;

	if (parentCluster == 0)	//Cluster is zero if it is root directory before FAT32
		rootDirectory = true;

	bool quit = false;

	while ((!quit) && ((parentCluster < EOF) && ((parentCluster > 1) || rootDirectory)))
	{
		readCluster(file->parentVolume, parentCluster, buffer);

		bool lfnsRemoved = false;

		for (uint16_t i = 0; i < (rootDirectory ? rootEntries : entriesPerCluster); i++)
		{
			if ((parentCluster == entryCluster) && (i == entryIndex))
			{
				quit = true;
				break;
			}

			FATEntry* entry = &((FATEntry*)buffer)[i];

			if ((entry->DIR_Name[0] != 0xE5) && (entry->DIR_Name[0] != 0x00))
			{
				if (entry->DIR_Attr.ATTR_READ_ONLY	&&
						entry->DIR_Attr.ATTR_HIDDEN 	&&
						entry->DIR_Attr.ATTR_SYSTEM		&&
						entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					//Long File Name Entry
					FATLongEntry* lfnToRemove = (FATLongEntry*)entry;

					if (lfnToRemove->LDIR_Chksum == chksum)
					{
						((FATEntry*)lfnToRemove)->DIR_Name[0] = 0xE5;
						lfnsRemoved = true;
					}
				}
			}
		}

		if (lfnsRemoved)
			writeCluster(file->parentVolume, parentCluster, buffer);

		if (rootDirectory)
			break;

		parentCluster = readFATEntry(file->parentVolume, offlineFAT, parentCluster);
	}

	delete [] buffer;
	delete parentDir;

	return true;
}


bool FAT::read(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead)
{	
	if ((file == NULL) || (buffer == NULL) || (bufferSize == 0))
		return false;

	if ((!file->isOpen) || (file->isDirectory) || (file->fileSize == 0))
		return false;

	if (((offset + bytesToRead) > file->fileSize) || (bytesToRead > bufferSize))
		return false;

	//If bytesToRead is 0 then read from offset till the end of the file
	if (bytesToRead == 0)
		bytesToRead = file->fileSize - offset;

	const uint8_t FATType = file->parentVolume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint16_t rootDirSize = file->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	uint8_t* offlineFAT = (uint8_t*)file->parentVolume->fsVariables[FAT_VAR_OFFLINE_FAT];

	bytesRead = 0;

	uint32_t currentCluster = file->firstCluster;
	uint16_t clusterCount = 1;

	uint8_t* tempBuffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];

	while ((currentCluster < EOF) && (currentCluster > 1))
	{
		uint32_t posFrom = (clusterCount - 1) * clusterSize;
		uint32_t posTo = (clusterCount * clusterSize);

		if (posTo < offset)
			continue;

		if (posFrom < offset)
			posFrom	= offset;

		if (posTo > (offset + bytesToRead))
			posTo = offset + bytesToRead;

		readCluster(file->parentVolume, currentCluster, tempBuffer);

		memcpy((buffer + bytesRead), (tempBuffer + posFrom), (posTo - posFrom));
		bytesRead += (posTo - posFrom);

		currentCluster = readFATEntry(file->parentVolume, offlineFAT, currentCluster);
		clusterCount++;
	}

	HAL::DateTime now = HAL::getCurrentDateTime();
	readCluster(file->parentVolume, file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER], tempBuffer);

	FATEntry* entry = (FATEntry*)tempBuffer;
	entry[file->fVariables[FAT_FILE_VAR_ENTRY_INDEX]].DIR_LstAccDate = createDateValue(now);
	writeCluster(file->parentVolume, file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER], tempBuffer);

	delete [] tempBuffer;

	if (bytesRead > 0)
		return true;
	else
		return false;
}

bool FAT::write(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToWrite, uint64_t& bytesWritten)
{	
	if ((file == NULL) || (buffer == NULL) || (bufferSize == 0))
		return false;

	if ((!file->isOpen) || (file->isDirectory))
		return false;

	if ((bytesToWrite == 0) || (bytesToWrite > bufferSize) || (offset > file->fileSize))
		return false;

	const uint16_t rootDirSize = file->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	uint8_t* offlineFAT = (uint8_t*)file->parentVolume->fsVariables[FAT_VAR_OFFLINE_FAT];

	bytesWritten = 0;
	uint64_t newfileSize = (file->fileSize > (offset + bytesToWrite) ? file->fileSize : (offset + bytesToWrite));

	if (!resize(file, newfileSize))
		return false;

	uint32_t currentCluster = file->firstCluster;
	uint16_t clusterCount = 1;

	uint8_t* tempBuffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];

	while (bytesWritten != bytesToWrite)
	{
		uint32_t posFrom = (clusterCount - 1) * clusterSize;
		uint32_t posTo = (clusterCount * clusterSize);

		if (posTo < offset)
			continue;

		if (posFrom < offset)
			posFrom	= offset;

		if (posTo > (offset + bytesToWrite))
			posTo = offset + bytesToWrite;

		if ((posTo - posFrom) != clusterSize)
			readCluster(file->parentVolume, currentCluster, tempBuffer);
		
		memcpy((tempBuffer + (posFrom % clusterSize)), (buffer + bytesWritten), (posTo - posFrom));
		writeCluster(file->parentVolume, currentCluster, tempBuffer);
		bytesWritten += (posTo - posFrom);

		currentCluster = readFATEntry(file->parentVolume, offlineFAT, currentCluster);
		clusterCount++;
	}

	HAL::DateTime now = HAL::getCurrentDateTime();
	readCluster(file->parentVolume, file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER], tempBuffer);

	FATEntry* entry = &((FATEntry*)tempBuffer)[file->fVariables[FAT_FILE_VAR_ENTRY_INDEX]];

	entry->DIR_LstAccDate = createDateValue(now);
	entry->DIR_FstClusHI = HIWORD(file->firstCluster);
	entry->DIR_WrtTime = createTimeValue(now);
	entry->DIR_WrtDate = createDateValue(now);
	entry->DIR_FstClusLO = LOWORD(file->firstCluster);
	entry->DIR_FileSize = file->fileSize;
	writeCluster(file->parentVolume, file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER], tempBuffer);

	delete [] tempBuffer;

	if (bytesWritten == bytesToWrite)
		return true;
	else
		return false;
}

bool FAT::resize(VMGR::File* file, uint64_t fileSize)
{
	if ((file == NULL) || (!file->isOpen) || (file->isDirectory))
		return false;

	if (file->fileSize == fileSize)
		return true;

	if (fileSize > FAT_MAX_FILE_SIZE)
		return false;

	const uint8_t FATType = file->parentVolume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : FAT_FAT16_EOF);
	const uint32_t EOC = (FATType == 32 ? FAT_FAT32_EOC_MARK : FAT_FAT16_EOC_MARK);
	const uint16_t rootDirSize = file->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * FAT_ENTRY_SIZE;
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS];
	uint8_t* offlineFAT = (uint8_t*)file->parentVolume->fsVariables[FAT_VAR_OFFLINE_FAT];

	uint32_t currentClusterCount = ((file->fileSize + (clusterSize - 1)) / clusterSize);
	uint32_t targetClusterCount = ((fileSize + (clusterSize - 1)) / clusterSize);

	if (targetClusterCount > currentClusterCount)
	{
		uint32_t cluster = file->firstCluster;
		uint32_t prevCluster = cluster;

		while ((cluster < EOF) && (cluster > 1))
		{
			prevCluster = cluster;
			cluster = readFATEntry(file->parentVolume, offlineFAT, cluster);
		}

		cluster = prevCluster;	//This is the last cluster in the chain

		for (uint32_t i = 0; i < (targetClusterCount - currentClusterCount); i++)
		{
			uint32_t newCluster = findFreeCluster(file->parentVolume);

			if (newCluster == NULL)
				return false;

			if ((cluster < EOF) && (cluster > 1))
				writeFATEntry(file->parentVolume, cluster, newCluster);
			else
				file->firstCluster = newCluster;

			writeFATEntry(file->parentVolume, newCluster, EOC);

			cluster = newCluster;
		}
	}
	else if (targetClusterCount < currentClusterCount)
	{
		uint32_t cluster = file->firstCluster;
		uint32_t clusterCount = 1;
		bool removeCluster = false;

		while ((cluster < EOF) && (cluster > 1))
		{
			uint32_t posTo = clusterCount * clusterSize;
			uint32_t nextCluster = readFATEntry(file->parentVolume, offlineFAT, cluster);

			if (removeCluster)
			{
				writeFATEntry(file->parentVolume, cluster, 0);
			}
			else if (posTo >= fileSize)
			{
				removeCluster = true;	//Start removing from next cluster;
				writeFATEntry(file->parentVolume, cluster, EOC);
			}
			
			cluster = nextCluster;
			clusterCount++;
		}

		if (fileSize == 0)
		{
			writeFATEntry(file->parentVolume, file->firstCluster, 0);
			file->firstCluster = 0;
		}
	}	

	file->fileSize = fileSize;

	uint8_t* tempBuffer = new uint8_t[(clusterSize >= rootDirSize) ? clusterSize : rootDirSize];

	HAL::DateTime now = HAL::getCurrentDateTime();
	readCluster(file->parentVolume, file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER], tempBuffer);

	FATEntry* entry = &((FATEntry*)tempBuffer)[file->fVariables[FAT_FILE_VAR_ENTRY_INDEX]];

	entry->DIR_LstAccDate = createDateValue(now);
	entry->DIR_FstClusHI = HIWORD(file->firstCluster);
	entry->DIR_WrtTime = createTimeValue(now);
	entry->DIR_WrtDate = createDateValue(now);
	entry->DIR_FstClusLO = LOWORD(file->firstCluster);
	entry->DIR_FileSize = file->fileSize;
	writeCluster(file->parentVolume, file->fVariables[FAT_FILE_VAR_ENTRY_CLUSTER], tempBuffer);

	delete [] tempBuffer;
	return true;
}