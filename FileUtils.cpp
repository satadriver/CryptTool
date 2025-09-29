

#include "FileUtils.h"

#include <Windows.h>


int FReader(const CHAR* filename, CHAR** lpbuf, unsigned __int64* lpsize) {
	int result = 0;

	HANDLE hf = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hf == INVALID_HANDLE_VALUE)
	{
		result = GetLastError();
		delete* lpbuf;
		return FALSE;
	}

	DWORD highsize = 0;
	*lpsize = GetFileSize(hf, &highsize);
	if (*lpsize == 0) {
		CloseHandle(hf);
		return FALSE;
	}

	result = SetFilePointer(hf, 0, 0, FILE_BEGIN);

	if (lpbuf)
	{
		if (*lpbuf == 0)
		{
			*lpbuf = new CHAR[*lpsize + 1024];
			*(*lpbuf) = 0;
		}
	}
	else {
		CloseHandle(hf);
		return FALSE;
	}

	DWORD readsize = 0;
	result = ReadFile(hf, *lpbuf, *lpsize, &readsize, 0);
	if (result > 0)
	{
		*(CHAR*)((char*)*lpbuf + readsize) = 0;
	}
	else {
		result = GetLastError();
		readsize = 0;
	}
	CloseHandle(hf);
	return readsize;
}




int FWriter(const CHAR* filename, const CHAR* lpbuf, int lpsize, int append) {
	int result = 0;
	HANDLE h = INVALID_HANDLE_VALUE;
	if (append)
	{
		h = CreateFileA(filename, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (h == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		DWORD highsize = 0;
		DWORD filesize = GetFileSize(h, &highsize);

		result = SetFilePointer(h, filesize, (long*)&highsize, FILE_BEGIN);
	}
	else {
		h = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (h == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
	}

	DWORD writesize = 0;
	result = WriteFile(h, lpbuf, lpsize * sizeof(CHAR), &writesize, 0);
	FlushFileBuffers(h);
	CloseHandle(h);
	return result;
}