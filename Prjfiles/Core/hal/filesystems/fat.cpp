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

//TODO: Check for maximum cluster count & entry count

#include "fat.h"

#include "../hal.h"
#include "../ide.h"

#include "../misc/macros.h"

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

	uint8_t* buffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];	
	((PMGR::Partition*)(volume->Target))->readData(ThisFATSecNum, 1, buffer);

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

	delete [] buffer;

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
	
	uint8_t* buffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];
	((PMGR::Partition*)(volume->Target))->readData(ThisFATSecNum, 1, buffer);

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

	delete [] buffer;
}

const char* FAT::getFAT32VolumeLabel(VMGR::Volume* volume)
{
	if (volume == NULL)
		return NULL;

	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];

	if (FATType != 32)
		return NULL;

	const uint16_t RootDirSectors = (((volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * sizeof(FATEntry)) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	const uint16_t RootDirSector = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (volume->fsVariables[FAT_VAR_BPB_NUMFATS] * FATSz);
	const uint16_t FirstDataSector = RootDirSector + RootDirSectors;	//In case of FAT32, RootDirSectors = 0

	uint8_t* buffer = new uint8_t[(IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])];
	((PMGR::Partition*)(volume->Target))->readData((FirstDataSector + ((volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])), volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);

	FATEntry* entry = ((FATEntry*)buffer);

	uint32_t currentCluster = volume->fsVariables[FAT_VAR_BPB_ROOTCLUS];

	static char longFileName[255];
	static uint8_t lfnIndex = 0;
	longFileName[0] = '\0';

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
			if (entry->DIR_Attr.ATTR_READ_ONLY	&&
				entry->DIR_Attr.ATTR_HIDDEN 	&&
				entry->DIR_Attr.ATTR_SYSTEM		&&
				entry->DIR_Attr.ATTR_VOLUME_ID)
			{
				//Long File Name Entry
				uint8_t* longFileEntry = ((uint8_t*)entry);

				//If this is the first entry in long file name chain then clear the string
				if ((longFileEntry[0] & 0x40) == 0x40)
					lfnIndex = 0;
				
				//Enumerate characters backwards
				
				//2 Characters
				for (int8_t i = 30; i >= 28; i-= 2)
					if ((longFileEntry[i] != 0xFF) && (longFileEntry[i] != 0x00))
						longFileName[lfnIndex++] = (char)longFileEntry[i];

				//6 Characters
				for (int8_t i = 24; i >= 14; i-= 2)
					if ((longFileEntry[i] != 0xFF) && (longFileEntry[i] != 0x00))
						longFileName[lfnIndex++] = (char)longFileEntry[i];

				//5 Characters
				for (int8_t i = 9; i >= 1; i-= 2)
					if ((longFileEntry[i] != 0xFF) && (longFileEntry[i] != 0x00))
						longFileName[lfnIndex++] = (char)longFileEntry[i];

				longFileName[lfnIndex] = '\0';
				
				entry++;
				entryCount++;
			}
			else
			{
				char* fileName;

				if (longFileName[0] != '\0')
				{
					strreverse(longFileName);
					fileName = longFileName;
				}
				else
				{
					static char shortFileName[13];

					char fileNameWithoutExt[9];
					strncpy(fileNameWithoutExt, ((char*)(entry->DIR_Name)), 8);
					fileNameWithoutExt[8] = '\0';
					strtrim(fileNameWithoutExt);	
					uint8_t lengthWithoutExt = strlen(fileNameWithoutExt);

					char extension[4];
					strncpy(extension, ((char*)(entry->DIR_Name) + 8), 3);
					extension[3] = '\0';
					strtrim(extension);		
					uint8_t extensionLength = strlen(extension);

					strcpy(shortFileName, fileNameWithoutExt);
					if (extensionLength > 0)
					{
						shortFileName[lengthWithoutExt] = '.';
						strcpy(shortFileName + lengthWithoutExt + 1, extension);
						shortFileName[lengthWithoutExt + extensionLength + 1] = '\0';
					}
					else
					{
						shortFileName[lengthWithoutExt] = '\0';
					}

					
					fileName = shortFileName;
				}
					
				if (!entry->DIR_Attr.ATTR_DIRECTORY	&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					delete [] buffer;
					return fileName;
				}					

				entry++;
				entryCount++;
			}
		}

		if (entryCount * sizeof(FATEntry) == (clusterCount * IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS]))
		{					
			currentCluster = readFATEntry(volume, currentCluster);

			if (currentCluster >= FAT_FAT32_EOF)
			{
				break;
			}
			else
			{
				uint32_t firstSectorOfCluster = (FirstDataSector + ((currentCluster - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS]));
				((PMGR::Partition*)(volume->Target))->readData(firstSectorOfCluster, volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
				entry = ((FATEntry*)buffer);
				clusterCount++;
			}
		}
	}

	delete [] buffer;

	return NULL;
}

FAT::FAT()
{
	Name = "FAT";
	systemID = 0x0B;
}

void FAT::init(VMGR::Volume* volume)
{
	//TODO: if FAT32 get Volume name from root directory
	if (volume == NULL)
		return;

	uint8_t* buffer = new uint8_t[IDE_ATA_BYTESPERSECTOR];
	((PMGR::Partition*)volume->Target)->readData(0, 1, buffer);

	FATCommonBPB* commonBPB = ((FATCommonBPB*)((uint32_t)buffer + FAT_COMMON_BPB_OFFSET));
	FAT1216BPB* fat1216BPB = ((FAT1216BPB*)((uint32_t)buffer + FAT_FAT1216_BPB_OFFSET));
	FAT32BPB* fat32BPB = ((FAT32BPB*)((uint32_t)buffer + FAT_FAT32_BPB_OFFSET));

	if (commonBPB->BPB_BytsPerSec != IDE_ATA_BYTESPERSECTOR) //No support for non-512 sector sizes
	{
		return;
	}

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

	//Set volume filesystem variables to save all needed values for later use
	volume->fsVariables[FAT_VAR_FATTYPE] = FATType;
	volume->fsVariables[FAT_VAR_FATSZ] = (FATType == 32 ? fat32BPB->BPB_FATSz32 : commonBPB->BPB_FATSz16);
	volume->fsVariables[FAT_VAR_BPB_NUMFATS] = commonBPB->BPB_NumFATs;
	volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] = commonBPB->BPB_ResvdSecCnt;
	volume->fsVariables[FAT_VAR_BPB_SECPERCLUS] = commonBPB->BPB_SecPerClus;
	volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] = commonBPB->BPB_RootEntCnt;
	volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] = fat32BPB->BPB_RootClus;

	if (FATType == 32)
	{
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
		strncpy(volume->Name, (const char*)&fat1216BPB->BS_VolLab, 11);
		volume->Name[11] = '\0';
		strtrim(volume->Name);
	}

	volume->Online = true;
}

bool FAT::create(VMGR::Volume* volume, Path* filePath, VMGR::File* dstFile)
{
	if ((volume == NULL) || (!volume->Online))
		return false;

	if (filePath == NULL)
		return false;

	if (dstFile == NULL)
		return false;

	

	return false;
}

bool FAT::find(VMGR::Volume* volume, Path* filePath, VMGR::File* dstFile)
{
	if ((volume == NULL) || (!volume->Online))
		return false;

	if ((filePath == NULL) || (!filePath->isAbsolute))
		return false;

	if (dstFile == NULL)
		return false;

	const uint16_t FATType = volume->fsVariables[FAT_VAR_FATTYPE];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : (FATType == 16 ? FAT_FAT16_EOF : FAT_FAT12_EOF));
	const uint16_t FATSz = volume->fsVariables[FAT_VAR_FATSZ];

	const uint16_t RootDirSectors = (((volume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * sizeof(FATEntry)) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	const uint16_t RootDirSector = volume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (volume->fsVariables[FAT_VAR_BPB_NUMFATS] * FATSz);
	const uint16_t FirstDataSector = RootDirSector + RootDirSectors;	//In case of FAT32, RootDirSectors = 0

	uint8_t* buffer = new uint8_t[(IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])];

	if (FATType == 32)
		((PMGR::Partition*)(volume->Target))->readData((FirstDataSector + ((volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS])), volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
	else
		((PMGR::Partition*)(volume->Target))->readData(RootDirSector, volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);

	FATEntry* entry = ((FATEntry*)buffer);

	uint32_t currentCluster = (FATType == 32 ? volume->fsVariables[FAT_VAR_BPB_ROOTCLUS] : 0);

	bool rootDirectory = (FATType == 32 ? false : true);
	bool Found = true;

	char* currentFileName = filePath->getComponent(1);

	while (Found && (currentFileName != NULL))
	{
		Found = false;

		static char longFileName[255];
		static uint8_t lfnIndex = 0;
		longFileName[0] = '\0';

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
				if (entry->DIR_Attr.ATTR_READ_ONLY	&&
					entry->DIR_Attr.ATTR_HIDDEN 	&&
					entry->DIR_Attr.ATTR_SYSTEM		&&
					entry->DIR_Attr.ATTR_VOLUME_ID)
				{
					//Long File Name Entry
					uint8_t* longFileEntry = ((uint8_t*)entry);

					//If this is the first entry in long file name chain then clear the string
					if ((longFileEntry[0] & 0x40) == 0x40)
						lfnIndex = 0;
					
					//Enumerate characters backwards
					
					//2 Characters
					for (int8_t i = 30; i >= 28; i-= 2)
						if ((longFileEntry[i] != 0xFF) && (longFileEntry[i] != 0x00))
							longFileName[lfnIndex++] = (char)longFileEntry[i];

					//6 Characters
					for (int8_t i = 24; i >= 14; i-= 2)
						if ((longFileEntry[i] != 0xFF) && (longFileEntry[i] != 0x00))
							longFileName[lfnIndex++] = (char)longFileEntry[i];

					//5 Characters
					for (int8_t i = 9; i >= 1; i-= 2)
						if ((longFileEntry[i] != 0xFF) && (longFileEntry[i] != 0x00))
							longFileName[lfnIndex++] = (char)longFileEntry[i];

					longFileName[lfnIndex] = '\0';
					
					entry++;
					entryCount++;
				}
				else
				{
					char* fileName;

					if (longFileName[0] != '\0')
					{
						strreverse(longFileName);
						fileName = longFileName;
					}
					else
					{
						static char shortFileName[13];

						char fileNameWithoutExt[9];
						strncpy(fileNameWithoutExt, ((char*)(entry->DIR_Name)), 8);
						fileNameWithoutExt[8] = '\0';
						strtrim(fileNameWithoutExt);	
						uint8_t lengthWithoutExt = strlen(fileNameWithoutExt);

						char extension[4];
						strncpy(extension, ((char*)(entry->DIR_Name) + 8), 3);
						extension[3] = '\0';
						strtrim(extension);		
						uint8_t extensionLength = strlen(extension);

						strcpy(shortFileName, fileNameWithoutExt);
						if (extensionLength > 0)
						{
							shortFileName[lengthWithoutExt] = '.';
							strcpy(shortFileName + lengthWithoutExt + 1, extension);
							shortFileName[lengthWithoutExt + extensionLength + 1] = '\0';
						}
						else
						{
							shortFileName[lengthWithoutExt] = '\0';
						}

						
						fileName = shortFileName;
					}

					if (strcmp(currentFileName, fileName) == 0)
						Found = true;

					if (Found)
					{
						if (!entry->DIR_Attr.ATTR_DIRECTORY	&&
							!entry->DIR_Attr.ATTR_VOLUME_ID)
						{
							//This is a file entry					
							if (strtok(NULL, "/") == NULL)
							{
								//File FOUND
								dstFile->Name = new char[strlen(fileName) + 1];
								strcpy(dstFile->Name, fileName);
								dstFile->Name[strlen(fileName)] = '\0';

								dstFile->firstCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
								dstFile->fileSize = entry->DIR_FileSize;
								dstFile->ioMode = VMGR::File::IOModes::ReadOnly;

								delete [] buffer;				
								return true;
							}
							else
							{
								//ERROR: Invalid path (This isn't the last component in the path)
								delete [] buffer;
								return false;
							}
						}
						else if (	entry->DIR_Attr.ATTR_DIRECTORY	&&
									!entry->DIR_Attr.ATTR_VOLUME_ID)
						{
							//This is a directory entry
							currentCluster = MAKE_DWORD(entry->DIR_FstClusLO, entry->DIR_FstClusHI);
							uint32_t firstSectorOfCluster = (FirstDataSector + ((currentCluster - 2)* volume->fsVariables[FAT_VAR_BPB_SECPERCLUS]));
							((PMGR::Partition*)(volume->Target))->readData(firstSectorOfCluster, volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
							entry = ((FATEntry*)buffer);
							
							rootDirectory = false;
							break;
						}
						else
						{
							//This is an invalid entry
							//ERROR: Invalid entry
							delete [] buffer;
							return false;
						}
					}

					entry++;
					entryCount++;
				}
			}

			if (!rootDirectory)
			{
				if (entryCount * sizeof(FATEntry) == (clusterCount * IDE_ATA_BYTESPERSECTOR * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS]))
				{					
					currentCluster = readFATEntry(volume, currentCluster);

					if (currentCluster >= EOF)
					{
						Found = false;
						break;
					}
					else
					{
						uint32_t firstSectorOfCluster = (FirstDataSector + ((currentCluster - 2) * volume->fsVariables[FAT_VAR_BPB_SECPERCLUS]));
						((PMGR::Partition*)(volume->Target))->readData(firstSectorOfCluster, volume->fsVariables[FAT_VAR_BPB_SECPERCLUS], buffer);
						entry = ((FATEntry*)buffer);
						clusterCount++;
					}
				}
			}
		}

		currentFileName = filePath->getNextComponent();
	}

	delete [] buffer;

	return false;
}

bool FAT::read(VMGR::File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead)
{	
	if ((file == NULL) || (buffer == NULL) || (bufferSize == 0))
		return false;

	if (((offset + bytesToRead) > file->fileSize) || (bytesToRead > bufferSize))
		return false;

	if (!file->isOpen)
		return false;

	//If bytesToRead is 0 then read from offset till the end of the file
	if (bytesToRead == 0)
		bytesToRead = file->fileSize - offset;

	const uint8_t FATType = file->parentVolume->fsVariables[FAT_VAR_FATTYPE];
	const uint16_t FATSz = file->parentVolume->fsVariables[FAT_VAR_FATSZ];
	const uint32_t EOF = (FATType == 32 ? FAT_FAT32_EOF : (FATType == 16 ? FAT_FAT16_EOF : FAT_FAT12_EOF));
	const uint16_t clusterSize = IDE_ATA_BYTESPERSECTOR * file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS];

	const uint16_t RootDirSectors = (((file->parentVolume->fsVariables[FAT_VAR_BPB_ROOTENTCNT] * sizeof(FATEntry)) + (IDE_ATA_BYTESPERSECTOR - 1)) / IDE_ATA_BYTESPERSECTOR);
	const uint16_t RootDirSector = file->parentVolume->fsVariables[FAT_VAR_BPB_RESVDSECCNT] + (file->parentVolume->fsVariables[FAT_VAR_BPB_NUMFATS] * FATSz);
	const uint16_t FirstDataSector = RootDirSector + RootDirSectors;	//In case of FAT32, RootDirSectors = 0

	bytesRead = 0;

	uint32_t currentCluster = file->firstCluster;
	uint16_t clusterCount = 1;

	while (((uint64_t)(clusterCount * clusterSize) < offset) && (currentCluster < EOF))
	{
		currentCluster = readFATEntry(file->parentVolume, currentCluster);
		clusterCount++;
	}

	uint8_t* tempBuffer = new uint8_t[clusterSize];

	while (((uint64_t)(clusterCount * clusterSize) >= (offset + bytesToRead)) && (currentCluster < EOF))
	{
		uint32_t firstSectorOfCluster = (FirstDataSector + ((currentCluster - 2) * file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS]));
		((PMGR::Partition*)(file->parentVolume->Target))->readData(firstSectorOfCluster, file->parentVolume->fsVariables[FAT_VAR_BPB_SECPERCLUS], tempBuffer);

		uint32_t posFrom = (clusterCount - 1) * clusterSize;
		uint32_t posTo = clusterCount * clusterSize;

		if (posFrom < offset)
			posFrom = offset;

		if (posTo > (offset + bytesToRead))
			posTo = offset + bytesToRead;

		memcpy((buffer + bytesRead), (tempBuffer + posFrom), (posTo - posFrom));
		bytesRead += (posTo - posFrom);

		currentCluster = readFATEntry(file->parentVolume, currentCluster);
		clusterCount++;

		posFrom += clusterSize;
		posTo += clusterSize;
	}

	delete [] tempBuffer;

	return false;
}