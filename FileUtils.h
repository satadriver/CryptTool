#pragma once

#include <Windows.h>

int FWriter(const CHAR* filename, const CHAR* lpbuf, int lpsize, int append);

int FReader(const CHAR* filename, CHAR** lpbuf, __int64* lpsize);