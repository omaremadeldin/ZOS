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
	Name[0] = '\0';
	Online = false;
}

VMGR::Volume::Volume(uint8_t id, Filesystem* fs, bool physical, void* target)
{
	ID = id;
	Name[0] = '\0';
	fileSystem = fs;
	Online = false;
	Physical = physical;
	Target = target;
}

uint64_t VMGR::Volume::getFreeSpace()
{
	if (fileSystem == NULL)
		return 0;

	return fileSystem->getFreeSpace(this);
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

	ioMode = mode;
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
	ioMode = mode;

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

bool VMGR::File::create(Path* filePath, File* dstFile, bool isDirectory)
{
	if ((filePath == NULL) || (!filePath->isAbsolute))
		return false;

	char* strVolumeID = filePath->getComponent(0);
	uint32_t volumeID = atoi(strVolumeID);
	delete [] strVolumeID;

	Volume* volume = NULL;

	LinkedList<Volume*>::Node* i = VOLUMES.head;
	while (i != NULL)
	{
		if (i->value->ID == volumeID)
		{
			if (i->value->Online)
			{
				volume = i->value;
				break;
			}
		}

		i = i->nextNode;
	}

	if (volume == NULL)
		return false;

	return volume->fileSystem->create(volume, filePath, dstFile, isDirectory);
}

uint32_t VMGR::File::list(VMGR::File* directory, File** &fileList)
{
	if ((directory == NULL) || (!directory->isDirectory))
		return false;

	char* strVolumeID = directory->filePath->getComponent(0);
	uint32_t volumeID = atoi(strVolumeID);
	delete [] strVolumeID;

	Volume* volume = NULL;

	LinkedList<Volume*>::Node* i = VOLUMES.head;
	while (i != NULL)
	{
		if (i->value->ID == volumeID)
		{
			if (i->value->Online)
			{
				volume = i->value;
				break;
			}
		}

		i = i->nextNode;
	}

	if (volume == NULL)
		return false;

	return volume->fileSystem->list(directory, fileList);
}

bool VMGR::File::remove(VMGR::File* file, bool recursive)
{
	if (file == NULL)
		return false;

	char* strVolumeID = file->filePath->getComponent(0);
	uint32_t volumeID = atoi(strVolumeID);
	delete [] strVolumeID;

	Volume* volume = NULL;

	LinkedList<Volume*>::Node* i = VOLUMES.head;
	while (i != NULL)
	{
		if (i->value->ID == volumeID)
		{
			if (i->value->Online)
			{
				volume = i->value;
				break;
			}
		}

		i = i->nextNode;
	}

	if (volume == NULL)
		return false;

	return volume->fileSystem->remove(file, recursive);
}

bool VMGR::File::read(uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead)
{
	if ((parentVolume == NULL) || (!parentVolume->Online))
		return false;

	return parentVolume->fileSystem->read(this, buffer, bufferSize, offset, bytesToRead, bytesRead);
}

bool VMGR::File::write(uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToWrite, uint64_t& bytesWritten)
{
	if ((parentVolume == NULL) || (!parentVolume->Online))
		return false;

	if (this->ioMode == IOModes::ReadOnly)
		return false;

	return parentVolume->fileSystem->write(this, buffer, bufferSize, offset, bytesToWrite, bytesWritten);
}

bool VMGR::File::readAll(uint8_t* buffer, uint64_t bufferSize, uint64_t& bytesRead)
{
	if ((parentVolume == NULL) || (!parentVolume->Online))
		return false;

	return parentVolume->fileSystem->read(this, buffer, bufferSize, 0, 0, bytesRead);
}

bool VMGR::File::write(uint8_t* buffer, uint64_t bufferSize, uint64_t bytesToWrite, uint64_t& bytesWritten)
{
	if ((parentVolume == NULL) || (!parentVolume->Online))
		return false;

	if (ioMode == IOModes::ReadOnly)
		return false;

	if (ioMode == IOModes::ReadWrite)
		return parentVolume->fileSystem->write(this, buffer, bufferSize, 0, bytesToWrite, bytesWritten) && parentVolume->fileSystem->resize(this, bytesToWrite);
	else if (ioMode == IOModes::ReadAppend)
		return parentVolume->fileSystem->write(this, buffer, bufferSize, fileSize, bytesToWrite, bytesWritten);

	return false;
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
		if (i->value->systemID(partition->systemID))
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
			i->value->fileSystem->release(i->value);
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