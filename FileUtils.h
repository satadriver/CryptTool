#pragma once

#include <Windows.h>
#include <string.h>
#include <string> 
#include <corecrt_wstring.h>

using namespace std;

int FWriter(const CHAR* filename, const CHAR* lpbuf, int lpsize, int append);

int FReader(const CHAR* filename, CHAR** lpbuf, unsigned __int64* lpsize);

int FindProcessLockingFile(const wchar_t* filePath, wchar_t* processName, DWORD maxLen);
int FindProcessLockingFile(const std::wstring& filePath, std::wstring& lockingProcessPath);