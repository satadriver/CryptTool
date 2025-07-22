


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



int FReader(const CHAR* filename, CHAR** lpbuf, __int64* lpsize) {
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


int str2hex_old(unsigned char* str,unsigned char *dst) {
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



int hex2str(char * hex,int len,char * dst)
{
	int dstlen = 0;
	for (int i = 0; i < len; i++)
	{
		unsigned char c = hex[i];
		unsigned char high = (c >> 4) & 0x0f;
		unsigned char low = (c) & 0x0f;
		if(high >9){
			high = high + 55;
		}
		else {
			high = high + 48;
		}
		if (low > 9) {
			low = low + 55;
		}
		else {
			low = low + 48;
		}
		dst[dstlen++] = high;
		dst[dstlen++] = low;
	}

	return dstlen;
}


int str2hex(char * str,char * dst) {
	int dstlen = 0;
	int len = lstrlenA(str);
	for (int i = 0; i < len; i++) {
		if (str[i] == '\\' && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
			i += 2;
			unsigned char low = str[i];
			unsigned char high = str[i+1];

			if (low >= 'a' && low <= 'f') {
				low = low - 0x20;
				low = low - 55;
			}
			else if (low >= 'A' && low <= 'F') {
				low = low - 55;
			}
			else if (low >= '0' && low <= '9') {
				low = low - 48;
			}
			else {
				break;
			}

			if (high >= 'a' && high <= 'f') {
				high = high - 0x20;
				high = high - 55;
			}
			else if (high >= 'A' && high <= 'F') {
				high = high - 55;
			}
			else if (high >= '0' && high <= '9') {
				high = high - 48;
			}
			else {
				break;
			}

			unsigned char c = (high << 4) | (low & 0x0f);
			dst[dstlen] = c;
			dstlen++;
		}
		else {
			dst[dstlen] = str[i];
			dstlen++;
		}
	}
	return dstlen;
}


int GetNameFromPath(char* path,char * name) {
	int len = lstrlenA(path);
	for (int i = len; i >= 0; i--) {
		if (path[i] == '\\') {
			lstrcpyA(name, path + i + 1);
			return lstrlenA(path + i + 1);
		}
	}
	return 0;
}