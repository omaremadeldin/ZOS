//==========================================
//
//		ZapperOS - Volume Manager
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "pmgr.h"

#include "utils/linkedlist.h"
#include "utils/path.h"

#define VMGR_MAX_VOLUME_NAME	32
#define VMGR_MAX_FS_VARIABLES	8

namespace zos
{
	class VMGR
	{
	public:
		class Volume;
		class File;

		class Filesystem
		{
		public:
			const char* Name;
			uint8_t systemID;

		public:
			virtual void init(Volume* volume) = 0;
			virtual bool create(Volume* volume, Path* filePath, File* dstFile) = 0;
			virtual bool find(Volume* volume, Path* filePath, File* dstFile) = 0;
			virtual bool read(File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead) = 0;
			//virtual bool write(File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToWrite, uint64_t& bytesWritten) = 0;
		};

		class Volume
		{
		public:
			uint8_t ID;
			char Name[VMGR_MAX_VOLUME_NAME];
			bool Online;
			bool Physical;
			void* Target;	//Must be of type PMGR::Partition if Physical==true
			Filesystem* fileSystem;
			uint32_t fsVariables[VMGR_MAX_FS_VARIABLES];	//File system variables

		public:
			Volume();
			Volume(uint8_t id, Filesystem* fs, bool physical, void* target);
		};

		class File
		{
		public:
			enum IOModes
			{
				ReadOnly,
				ReadWrite,
				ReadAppend
			};

		public:
			char* Name;
			Path* filePath;
			Volume* parentVolume;
			uint64_t firstCluster;
			uint64_t fileSize;
			bool isOpen;
			bool isDirectory;
			IOModes ioMode;

		public:
			File();
			~File();
			bool open(Path* path, IOModes mode = ReadOnly);
			File(Path* path, IOModes mode);
			bool read(uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead);
			bool readAll(uint8_t* buffer, uint64_t bufferSize, uint64_t& bytesRead);
		};

	private:
		static uint8_t generateVolumeID();

	public:
		static void init();
		static bool mount(PMGR::Partition* partition);
		static bool unmount(uint8_t volumeID);
		static void mountAll();
		static void unmountAll();
	};

	extern "C" LinkedList<VMGR::Filesystem*> FILESYSTEMS;
	extern "C" LinkedList<VMGR::Volume*> VOLUMES;
}