#pragma once
#include <Windows.h>

/*
* Object for Window Resources
* Holds pointer to data as well as size
*/
class EResource
{
public:
	EResource() {}
	EResource(int resourceID) { Load(resourceID); }
	EResource(LPCWSTR resourceName, LPCWSTR resourceType) { Load(resourceName, resourceType); }
	EResource(int resourceID,LPCWSTR resourceType) { Load(resourceID,resourceType); }
	void Load(LPCWSTR resourceName, LPCWSTR resourceType)
	{
		HRSRC resourceHandle = FindResource(NULL, resourceName, resourceType);
		if (resourceHandle)
		{
			DataSize = SizeofResource(NULL, resourceHandle);
			HGLOBAL resourceGHandle = LoadResource(NULL, resourceHandle);
			if (resourceGHandle)
			{
				DataPointer = LockResource(resourceGHandle);
			}
			else
				DataSize = 0;
		}
	}
	void Load(int resourceID,LPCWSTR resourceType)
	{
		HRSRC resourceHandle = FindResource(NULL,MAKEINTRESOURCE(resourceID),resourceType);
		if (resourceHandle)
		{
			DataSize = SizeofResource(NULL, resourceHandle);
			HGLOBAL resourceGHandle = LoadResource(NULL, resourceHandle);
			if (resourceGHandle)
				DataPointer = LockResource(resourceGHandle);
			else
				DataSize = 0;
		}
	}
	void Load(int resourceID)
	{
		HRSRC resourceHandle = FindResource(NULL, MAKEINTRESOURCE(resourceID), MAKEINTRESOURCE(resourceID));
		if (resourceHandle)
		{
			DataSize = SizeofResource(NULL, resourceHandle);
			HGLOBAL resourceGHandle = LoadResource(NULL, resourceHandle);
			if (resourceGHandle)
				DataPointer = LockResource(resourceGHandle);
			else
				DataSize = 0;
		}
	}
	LPCVOID GetDataPointer()
	{
		return DataPointer;
	}
	size_t GetDataSize()
	{
		return DataSize;
	}
	operator bool() { return DataSize; }
private:
	LPCVOID DataPointer = nullptr;
	size_t DataSize = 0;
};


