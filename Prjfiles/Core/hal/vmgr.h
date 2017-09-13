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
#define VMGR_MAX_FS_VARIABLES	13

#define VMGR_MAX_F_VARIABLES	2

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

		public:
			virtual bool systemID(uint8_t sysID) = 0;
			virtual void init(Volume* volume) = 0;
			virtual uint64_t getFreeSpace(Volume* volume) = 0;
			virtual void release(Volume* volume) = 0;

		public:
			virtual bool create(Volume* volume, Path* filePath, File* dstFile, bool isDirectory = false) = 0;
			virtual bool find(Volume* volume, Path* filePath, File* dstFile) = 0;
			virtual uint32_t list(File* directory, File** &fileList) = 0;
			virtual bool remove(File* file, bool recursive) = 0;
			virtual bool read(File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead) = 0;
			virtual bool write(File* file, uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToWrite, uint64_t& bytesWritten) = 0;
			virtual bool resize(File* file, uint64_t fileSize) = 0;			
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

		public:
			uint64_t getFreeSpace();
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
			uint32_t fVariables[VMGR_MAX_F_VARIABLES];

		public:
			File();
			~File();
			bool open(Path* path, IOModes mode = ReadOnly);
			File(Path* path, IOModes mode = ReadOnly);
			static bool create(Path* filePath, File* dstFile, bool isDirectory = false);
			static uint32_t list(VMGR::File* directory, File** &fileList);
			static bool remove(VMGR::File* file, bool recursive);
			bool read(uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToRead, uint64_t& bytesRead);
			bool write(uint8_t* buffer, uint64_t bufferSize, uint64_t offset, uint64_t bytesToWrite, uint64_t& bytesWritten);
			bool readAll(uint8_t* buffer, uint64_t bufferSize, uint64_t& bytesRead);
			bool write(uint8_t* buffer, uint64_t bufferSize, uint64_t bytesToWrite, uint64_t& bytesWritten);
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