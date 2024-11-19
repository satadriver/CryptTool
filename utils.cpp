


#include <windows.h>
#include <Shlobj.h>
#include <stdio.h>
#include <io.h>
#include <UserEnv.h>
#include <tlhelp32.h>
#include <WtsApi32.h>
#include <Shlwapi.h>
#include <Psapi.h>


#pragma comment(lib,"wtsapi32.lib")

#pragma comment(lib,"Userenv.lib")


#pragma warning(disable: 4996)



int FReader(const CHAR* filename, CHAR** lpbuf, int* lpsize) {
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




int FWriter(const CHAR* filename,const CHAR* lpbuf, int lpsize, int append) {
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




const char* gNumStr = "0123456789ABCDEF";


int str2hex(unsigned char* str,unsigned char *dst) {
	int dstlen = 0;
	int len = lstrlenA((char*)str);
	for (int i = 0; i < len; i+=2) {
		char t = str[i];
		if (t >= 'a' && t <= 'f') {
			t = t - 0x20;
		}

		unsigned high = 0;
		for (int j = 0; j < 16; j++) {
			if (t == gNumStr[j]) {
				high = j;
			}
		}

		t = str[i+1];
		if (t >= 'a' && t <= 'f') {
			t = t - 0x20;
		}
		unsigned low = 0;
		for (int j = 0; j < 16; j++) {
			if (t == gNumStr[j]) {
				low = j;
			}
		}

		unsigned char c = (high << 4) | (low & 0x0f);
		dst[dstlen++] = c;
	}

	return dstlen;
}



void hex2str(char * hex,int len)
{
	for (int i = 0; i < len; i++)
		printf("%02X", hex[i]);

	printf("\r\n");

}