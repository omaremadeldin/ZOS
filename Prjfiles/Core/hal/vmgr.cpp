//==========================================
//
//		ZapperOS - Volume Manager
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "vmgr.h"

#include "filesystems/fat.h"

#include <string.h>

using namespace zos;

LinkedList<VMGR::Filesystem*> zos::FILESYSTEMS;
LinkedList<VMGR::Volume*> zos::VOLUMES;

VMGR::Volume::Volume()
{
	this->Name[0] = '\0';
	this->Online = false;
}

VMGR::Volume::Volume(uint8_t id, Filesystem* fs, bool physical, void* target)
{
	this->ID = id;
	this->Name[0] = '\0';
	this->fileSystem = fs;
	this->Online = false;
	this->Physical = physical;
	this->Target = target;
}

VMGR::File::File()
{
	Name = NULL;
	filePath = NULL;
	parentVolume = NULL;
	firstCluster = 0;
	fileSize = 0;
	isOpen = false;
	ioMode = IOModes::ReadOnly;
}

bool VMGR::File::open(Path* path, IOModes mode)
{
	if ((path == NULL) || (!path->isAbsolute))
		return false;

	if (mode == IOModes::ReadWrite)
		return false;

	isOpen = false;

	char* strVolumeID = path->getComponent(0);
	uint32_t volumeID = atoi(strVolumeID);
	delete [] strVolumeID;

	LinkedList<Volume*>::Node* i = VOLUMES.head;
	while (i != NULL)
	{
		if (i->value->ID == volumeID)
		{
			if (i->value->Online)
			{
				parentVolume = i->value;
				break;
			}
		}

		i = i->nextNode;
	}

	if ((parentVolume == NULL) || 
		(!parentVolume->fileSystem->find(parentVolume, path, this)))
		return false;

	filePath = path;
	isOpen = true;

	return true;
}

VMGR::File::File(Path* path, IOModes mode)
{
	Name = NULL;
	filePath = NULL;
	parentVolume = NULL;
	firstCluster = 0;
	fileSize = 0;
	isOpen = false;
	ioMode = IOModes::ReadOnly;

	open(path, mode);
}

VMGR::File::~File()
{
	if (Name != NULL)
		delete [] Name;

	if (filePath != NULL)
		delete filePath;

	Name = NULL;
	filePath = NULL;
	parentVolume = NULL;
	firstCluster = 0;
	fileSize = 0;
	isOpen = false;
	ioMode = IOModes::ReadOnly;
}

bool VMGR::File::read(uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead)
{
	return parentVolume->fileSystem->read(this, buffer, bufferSize, offset, bytesToRead, bytesRead);
}

bool VMGR::File::readAll(uint8_t* buffer, uint64_t bufferSize, uint64_t& bytesRead)
{
	return parentVolume->fileSystem->read(this, buffer, bufferSize, 0, 0, bytesRead);
}

uint8_t VMGR::generateVolumeID()
{
	static uint8_t volID = 0;
	return ++volID;
}

void VMGR::init()
{
	FILESYSTEMS.add(new filesystems::FAT());
}

bool VMGR::mount(PMGR::Partition* partition)
{
	if (partition == NULL)
		return false;

	Filesystem* fs = NULL;

	LinkedList<Filesystem*>::Node* i = FILESYSTEMS.head;
	while (i != NULL)
	{
		if (i->value->systemID == partition->systemID)
		{
			fs = i->value;
			break;
		}

		i = i->nextNode;
	}

	if (fs == NULL)
		return false;		//Unsupported filesystem

	Volume* newVol = new Volume(generateVolumeID(), fs, true, partition);

	fs->init(newVol);
	VOLUMES.add(newVol);

	return true;
}

bool VMGR::unmount(uint8_t volumeID)
{
	LinkedList<Volume*>::Node* i = VOLUMES.head;
	while (i != NULL)
	{
		if (i->value->ID == volumeID)
		{
			delete i->value;
			VOLUMES.remove(i);
			break;
		}

		i = i->nextNode;
	}

	return false;
}

void VMGR::mountAll()
{
	LinkedList<PMGR::Partition*>::Node* i = PARTITIONS.head;
	while (i != NULL)
	{
		mount(i->value);
		i = i->nextNode;
	}
}

void VMGR::unmountAll()
{
	LinkedList<Volume*>::Node* i = VOLUMES.head;
	while (i != NULL)
	{
		unmount(i->value->ID);
		i = i->nextNode;
	}
}