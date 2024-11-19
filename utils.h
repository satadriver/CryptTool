#pragma once

#include <Windows.h>


int FWriter(const CHAR* filename, const CHAR* lpbuf, int lpsize, int append);

int FReader(const CHAR* filename, CHAR** lpbuf, int* lpsize);

int str2hex(unsigned char* str, unsigned char* dst);