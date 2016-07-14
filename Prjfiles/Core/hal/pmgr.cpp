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
//* Handle null Disk
//==========================================
//By Omar Emad Eldin
//==========================================

#include "pmgr.h"

using namespace zos;

LinkedList<PMGR::Partition*> zos::PARTITIONS;

PMGR::Partition::Partition(IDE::Controller::Channel::Device* disk, bool bootable, uint8_t systemID, uint64_t startingLBA, uint64_t totalSectors)
{
	this->Disk = disk;
	this->Bootable = bootable;
	this->systemID = systemID;
	this->startingLBA = startingLBA;
	this->totalSectors = totalSectors;
}

void PMGR::Partition::readData(uint64_t LBA, uint16_t noOfSectors, uint8_t* buffer)
{
	if ((LBA + noOfSectors) > totalSectors)
		noOfSectors = ((LBA + noOfSectors) - totalSectors);

	Disk->readData((startingLBA + LBA), noOfSectors, buffer, false);
}

void PMGR::Partition::writeData(uint64_t LBA, uint16_t noOfSectors, uint8_t* buffer)
{
	if ((LBA + noOfSectors) > totalSectors)
			noOfSectors = ((LBA + noOfSectors) - totalSectors);

	Disk->writeData((startingLBA + LBA), noOfSectors, buffer, false);
}

void PMGR::scan()
{
	LinkedList<IDE::Controller::Channel::Device*>::Node* n = IDE_DEVICES.head;
	while (n != NULL)
	{
		IDE::Controller::Channel::Device* disk = n->value;
		n = n->nextNode;
		
		//This is not a disk
		if (disk->deviceType != IDE::Controller::Channel::Device::ATA)
			continue;
		
		static uint8_t buffer[IDE_ATA_BYTESPERSECTOR];
		
		//Read first sector of disk
		disk->readData(0, 1, &buffer);
		
		//MBR signature 0xAA55
		if (!(buffer[510] == 0x55) || !(buffer[511] == 0xAA))
			continue;
		
		PTEntry entry1 = *((PTEntry*)&buffer[PMGR_MBR_PARTITION_1]);
		PTEntry entry2 = *((PTEntry*)&buffer[PMGR_MBR_PARTITION_2]);
		PTEntry entry3 = *((PTEntry*)&buffer[PMGR_MBR_PARTITION_3]);
		PTEntry entry4 = *((PTEntry*)&buffer[PMGR_MBR_PARTITION_4]);
		
		if (entry1.systemID != 0x00)
			PARTITIONS.add(new Partition(disk, entry1.Bootable, entry1.systemID, entry1.startingLBA, entry1.totalSectors));
		
		if (entry2.systemID != 0x00)
			PARTITIONS.add(new Partition(disk, entry2.Bootable, entry2.systemID, entry2.startingLBA, entry2.totalSectors));
		
		if (entry3.systemID != 0x00)
			PARTITIONS.add(new Partition(disk, entry3.Bootable, entry3.systemID, entry3.startingLBA, entry3.totalSectors));
		
		if (entry4.systemID != 0x00)
			PARTITIONS.add(new Partition(disk, entry4.Bootable, entry4.systemID, entry4.startingLBA, entry4.totalSectors));
	}
}