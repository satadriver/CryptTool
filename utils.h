#pragma once

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

