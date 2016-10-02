//==========================================
//
//	   		  ZapperOS - Path
//
//==========================================
//By Omar Emad Eldin
//==========================================

#include "path.h"

#include "../exceptions.h"

#include <ctype.h>
#include <string.h>

using namespace zos;

bool Path::isValidFilename(const char* strFilename)
{
	if (strFilename == NULL)
		return false;

	size_t fnLen = strlen(strFilename);

	if (fnLen == 0)
		return false;

	if (fnLen > PATH_MAX_FILE_CHARS)
		return false;

	for (uint16_t i = 0; i < fnLen; i++)
	{
		char c = strFilename[i];
		//Check for invalid characters
		//Valid chars are A-Z a-z 0-9 . _ - /

		if (!(isalnum(c) || (c == '.') || (c == '_') || (c == '-') || (c == '/')))
			return false;
	}

	return true;
}

bool Path::isValidPath(const char* strPath)
{
	if (strPath == NULL)
		return false;

	size_t pathLen = strlen(strPath);

	if (pathLen == 0)
		return false;

	if (pathLen > PATH_MAX_CHARS)
		return false;

	//Network paths are not supported for now
	if ((pathLen >= 2) && (strPath[1] == '/') && (strPath[0] == '/'))
		return false;

	for (uint16_t i = 0; i < pathLen; i++)
	{
		char c = strPath[i];
		//Check for invalid characters
		//Valid chars are A-Z a-z 0-9 . _ - / (space)

		if (!(isalnum(c) || (c == '.') || (c == '_') || (c == '-') || (c == '/') || (c == ' ')))
			return false;
	}

	return true;
}

Path::Path()
{
	path = NULL;
	length = 0;
	componentIndex = 0;
	isAbsolute = false;
}

Path::~Path()
{
	if (path != NULL)
		delete [] path;

	path = NULL;
	length = 0;
	componentIndex = 0;
	isAbsolute = false;
}

Path::Path(const char* strPath)
{
	if (!isValidPath(strPath))
	{
		Exceptions::throwException("Path Exception", "Invalid path, either exceeded maximum length or invalid format");
		return;
	}

	size_t pathLen = strlen(strPath);

	path = new char[pathLen	+ 1];
	strcpy(path, strPath);
	path[pathLen] = '\0';

	length = pathLen;

	componentIndex = 0;

	isAbsolute = (path[0] == '/');
}

const char* Path::toString()
{
	return path;
}

char* Path::getComponent(uint16_t index)
{
	uint16_t startpos = 0;
	uint16_t endpos = 0;
	uint16_t currentIndex = 0;

	while (currentIndex <= index)
	{
		startpos = endpos;

		while (path[startpos] == '/')
			startpos++;

		endpos = startpos;

		while ((path[endpos] != '/') && (path[endpos] != '\0'))
			endpos++;

		currentIndex++;
	}

	componentIndex = endpos;

	char* result = new char[endpos - startpos + 1];
	strncpy(result, path + startpos, endpos-startpos);
	result[endpos-startpos] = '\0';

	return result;
}

char* Path::getNextComponent()
{
	uint16_t startpos = componentIndex;
	uint16_t endpos = componentIndex;

	startpos = endpos;

	while ((path[startpos] == '/') && (path[startpos] != '\0'))
		startpos++;

	if (path[startpos] == '\0')
		return NULL;

	endpos = startpos;

	while ((path[endpos] != '/') && (path[endpos] != '\0'))
		endpos++;

	componentIndex = endpos;

	char* result = new char[endpos - startpos + 1];
	strncpy(result, path + startpos, endpos-startpos);
	result[endpos-startpos] = '\0';

	return result;
}

Path* Path::getParentPath()
{
	uint16_t newLength = length;
	//  

	while (path[newLength] == '/')
		newLength--;

	while(path[newLength] != '/')
		newLength--;

	newLength++;

	if (newLength == 0)
		return NULL;

	char* strResult = new char[newLength + 1];
	strncpy(strResult, path, newLength);
	strResult[newLength] = '\0';

	Path* result = new Path(strResult);
	delete[] strResult;

	return result;
}