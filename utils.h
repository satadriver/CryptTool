#pragma once

#include <string>
#include <winreg.h>

#define SYSTEM_VERSION_WIN9X	1
#define SYSTEM_VERSION_WIN2000	2
#define SYSTEM_VERSION_XP		3
#define SYSTEM_VERSION_VISTA	4
#define SYSTEM_VERSION_WIN7		5
#define SYSTEM_VERSION_WIN8		6
#define SYSTEM_VERSION_WIN10	7
#define SYSTEM_VERSION_UNKNOW	8



int str2hex(char* str, char* dst);

int hex2str(char* hex, int len, char* dst);

int GetNameFromPath(char* path, char* name);

int IsSystem64Bit();

int IsCpu64Bit();

int QueryRegistryValue(HKEY hMainKey, char* szSubKey, unsigned long type, char* szKeyName, unsigned char* szKeyValue);

int MapPeFile(char* pFileBuff, char* chBaseAddress,const char * fn);

DWORD GetImageSize(char* pFileBuff);

int IsPeFile(char* data, int size);

#ifdef DUMMYUNIONNAME2
#undef DUMMYUNIONNAME2
#endif
#ifdef DUMMYSTRUCTNAME2
#undef DUMMYSTRUCTNAME2
#endif

std::wstring StringToWString_UTF8(const std::string& str);
std::string WStringToString_UTF8(const std::wstring& wstr);

#pragma pack(1)

/*
typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
	union {
		struct {
			DWORD NameOffset : 31;
			DWORD NameIsString : 1;
		} DUMMYSTRUCTNAME;
		DWORD   Name;
		WORD    Id;
	} DUMMYUNIONNAME;

	union {
		DWORD   OffsetToData;
		struct {
			DWORD   OffsetToDirectory : 31;
			DWORD   DataIsDirectory : 1;
		} DUMMYSTRUCTNAME2;
	} DUMMYUNIONNAME2;
} IMAGE_RESOURCE_DIRECTORY_ENTRY, * PIMAGE_RESOURCE_DIRECTORY_ENTRY;
*/

#pragma pack()