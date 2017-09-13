//==========================================
//
//	   		  ZapperOS - Path
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#define PATH_MAX_FILE_CHARS	256
#define PATH_MAX_CHARS		4096

namespace zos
{
	class Path
	{
	private:
		char* path;
		uint16_t length;
		uint16_t componentIndex;

	public:
		bool isAbsolute;

	public:
		static bool isValidFilename(const char* strFilename);
		static bool isValidPath(const char* strPath);

	public:
		Path();
		~Path();
		Path(const char* strPath);
		const char* toString();
		char* getComponent(uint16_t index);	//This function returns a new string, should be disposed after usage
		char* getNextComponent();	//This function returns a new string, should be disposed after usage
		char* getFilename();	//This function returns a new string, should be disposed after usage
		Path* getParentPath();	//This function returns a new path, should be disposed after usage
		Path* addComponent(const char* filename);	//This function returns a new path, should be disposed after usage
	};
}