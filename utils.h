#pragma once

#include <Windows.h>



int FWriter(const CHAR* filename, const CHAR* lpbuf, int lpsize, int append);

int FReader(const CHAR* filename, CHAR** lpbuf, __int64* lpsize);

int str2hex(char* str, char* dst);

int hex2str(char* hex, int len, char* dst);

int GetNameFromPath(char* path, char* name);