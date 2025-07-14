#pragma once

#include <Windows.h>

void hex2str(char* hex, int len);

int FWriter(const CHAR* filename, const CHAR* lpbuf, int lpsize, int append);

int FReader(const CHAR* filename, CHAR** lpbuf, __int64* lpsize);

int str2hex(unsigned char* str, unsigned char* dst);

int GetNameFromPath(char* path, char* name);