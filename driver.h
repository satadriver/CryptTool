#pragma once

#include <windows.h>

BOOL LoadNTDriver(WCHAR* lpszDriverName, WCHAR* lpszDriverPath, int servicetype, int boottype, WCHAR* groupname);

int LoadDriver(char* driverpath);
int RemoveDriver(char* drivername);
void TestDriver(char* drvname);