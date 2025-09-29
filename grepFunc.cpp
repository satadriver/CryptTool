#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "utils.h"

using namespace std;

//melody.shop.ele.me/login
//https://melody.shop.ele.me/login


void upcase(char* str, int size, char* dststr) {
	if(str == 0 || dststr == 0 || size <= 0) {
		return;
	}

	for (int i = 0; i < size; i++) {
		if (str[i] >= 'A' && str[i] <= 'Z') {
			dststr[i] = str[i] + 0x20;
		}
		else {
			dststr[i] = str[i];
		}
	}
}


void lowcase(char* str, int size, char* dststr) {
	if (str == 0 || dststr == 0 || size <= 0) {
		return;
	}

	for (int i = 0; i < size; i++) {
		if (str[i] >= 'a' && str[i] <= 'z') {
			dststr[i] = str[i] - 0x20;
		}
		else {
			dststr[i] = str[i];
		}
	}
}


int SearchInFile(const char* szFileName, const char* pDstContent,int len, const wchar_t* wszcontent)
{
	int ret = 0;

	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("%s CreateFle:%s error:%d\r\n", __FUNCTION__, szFileName, GetLastError());
		return FALSE;
	}

	unsigned int asclen = len;

	unsigned int fSize = GetFileSize(hFile, 0);
	if (fSize < asclen)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	char* pFileBuf = new char[fSize + 16];

	DWORD dwCnt = 0;
	ret = ReadFile(hFile, pFileBuf, fSize, &dwCnt, 0);
	*(pFileBuf + fSize) = 0;
	CloseHandle(hFile);
	if (ret == 0)
	{
		delete[] pFileBuf;
		return 0;
	}

	for (unsigned int i = 0; i <= fSize - asclen; i++)
	{
		upcase(pFileBuf + i, asclen, pFileBuf + i);
		if (memcmp(pDstContent, pFileBuf + i, asclen) == 0)
		{
			printf("Find string:\"%s\" at file:\"%s\" Position:%u\r\n", pDstContent, szFileName, i);
		}
	}

	unsigned int unilen = asclen * sizeof(wchar_t);
	if (fSize < unilen)
	{
		delete[] pFileBuf;
		return FALSE;
	}
	for (unsigned int i = 0; i <= fSize - unilen; i++)
	{
		upcase(pFileBuf + i, unilen, pFileBuf + i);
		if (memcmp((char*)wszcontent, pFileBuf + i, unilen) == 0)
		{
			printf("Find string:\"%s\" at file:\"%s\" Position:%u\r\n",pDstContent, szFileName, i);
		}
	}

	delete[] pFileBuf;
	return FALSE;
}

int SearchInDir(const char* PreStrPath, int iLayer, const char* pDstContent,int len, const wchar_t* wszcontent)
{
	int ret = 0;

	static int result = 0;

	int prePathLen = lstrlenA(PreStrPath);

	char strPath[0x1000] = { 0 };
	memcpy(strPath, PreStrPath, prePathLen);
	if (PreStrPath[prePathLen-1] == '\\' || PreStrPath[prePathLen-1] == '/') {
		memcpy(strPath + prePathLen, "*.*", lstrlenA("*.*"));
	}
	else {
		memcpy(strPath + prePathLen, "\\*.*", lstrlenA("\\*.*"));
	}

	WIN32_FIND_DATAA stWfd = { 0 };
	HANDLE hFind = FindFirstFileA(strPath, &stWfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFileA:%s error:%d\r\n", strPath, GetLastError());
		return 0;
	}

	char szLastDir[] = { '.','.',0 };
	char szCurDir[] = { '.',0 };
	do
	{
		if (stWfd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			if (lstrcmpiA(stWfd.cFileName, szLastDir) == 0 || lstrcmpiA(stWfd.cFileName, szCurDir) == 0)
			{
				continue;
			}

			char strNextPath[0x1000] = { 0 };

			memcpy(strNextPath, PreStrPath, prePathLen);
			if (PreStrPath[prePathLen-1] == '\\' || PreStrPath[prePathLen-1] == '/') {
				memcpy(strNextPath + prePathLen , stWfd.cFileName, lstrlenA(stWfd.cFileName));
			}
			else {
				*(strNextPath + prePathLen) = '\\';
				memcpy(strNextPath + prePathLen + 1, stWfd.cFileName, lstrlenA(stWfd.cFileName));
			}
			
			ret = SearchInDir(strNextPath, iLayer + 1, pDstContent,len, wszcontent);
			if (ret)
			{

			}
			else {

			}
		}
		else
		{
			char szFileName[0x1000] = { 0 };
			memcpy(szFileName, PreStrPath, prePathLen);
			if (PreStrPath[prePathLen-1] == '\\' || PreStrPath[prePathLen-1] == '/') {
				memcpy(szFileName + prePathLen , stWfd.cFileName, lstrlenA(stWfd.cFileName));
			}
			else {
				*(szFileName + prePathLen) = '\\';
				memcpy(szFileName + prePathLen + 1, stWfd.cFileName, lstrlenA(stWfd.cFileName));
			}
						
			int iFilePos = SearchInFile(szFileName, pDstContent,len, wszcontent);
			if (iFilePos)
			{
				result++;
			}
		}
	} while (FindNextFileA(hFind, &stWfd));

	FindClose(hFind);

	return result;
}


int SearchString(char* option,char* data,char * filpath) {
	int ret = 0;
	int srclen = 0;
	char szContent[0x1000] = { 0 };

	string str = data;
	string path = filpath;

	char* lpstr = (char*)str.c_str();
	char c = lpstr[0];
	if (str.c_str()[0] == '\"') {
		str = str.substr(1);
	}
	if (str.c_str()[str.length()-1] == '\"') {
		str = str.substr(0,str.length() -1);
	}

	if (path.c_str()[0] == '\"') {
		path = path.substr(1);
	}
	if (path.c_str()[path.length() - 1] == '\"') {
		path = path.substr(0, path.length() - 1);
	}

	wchar_t wszContent[0x1000] = { 0 };
	if (option[0] == 'b') {
		srclen = str2hex((char*)str.c_str(), szContent);
		szContent[srclen] = 0;
		memcpy(wszContent, szContent, srclen);
		wszContent[srclen] = 0;
	}
	else if (option[0] == 's') {
		srclen = lstrlenA((char*)str.c_str());
		upcase((char*)str.c_str(), srclen, szContent);
		szContent[srclen] = 0;

		int wlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wszContent, sizeof(wszContent) / sizeof(wchar_t));
		wszContent[wlen] = 0;

		srclen++;
	}

	ret = GetFileAttributesA(path.c_str());
	if (ret & FILE_ATTRIBUTE_DIRECTORY) {
		ret = SearchInDir(path.c_str(), 1, szContent,srclen, wszContent);
	}
	else if (ret & FILE_ATTRIBUTE_ARCHIVE) {
		ret = SearchInFile(path.c_str(), szContent,srclen , wszContent);
	}
	return ret;
}


int test(int argc, char** argv)
{
	int ret = 0;

	wchar_t wszcontent[1024] = { 0 };

	char* searchpath = 0;
	char* searchstr = 0;
	if (argc < 2)
	{
		//printf("parameter wrong!\r\n");
		//printf("usage 1 (search in defined directory):c:\\test\\ stringvalue\r\n");
		//printf("usage 2 (search in current directory):stringvalue\r\n");
		//getchar();
		//return FALSE;

		printf("input the path the content in which you want to search:");
		char inputpath[1024];
		char inputstr[1024];
		ret = scanf("%s", inputpath);

		printf("input the content you want to search:");
		ret = scanf("%s", inputstr);


		MultiByteToWideChar(CP_ACP, 0, inputstr, -1, wszcontent, sizeof(wszcontent));

		ret = SearchInDir(inputpath, 1, inputstr, lstrlenA(inputstr)+1, wszcontent);
	}
	else {
		if (argc == 2)
		{
			MultiByteToWideChar(CP_ACP, 0, argv[1], -1, wszcontent, sizeof(wszcontent) / sizeof(wchar_t));

			char szcurdir[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, szcurdir);

			printf("start searching string:\"%s\" in path:\"%s\",please to wait patiently...\r\n\r\n", argv[1], szcurdir);

			char szcontent[1024];
			upcase(argv[1], lstrlenA(argv[1]), szcontent);
			ret = SearchInDir(szcurdir, 1, szcontent, lstrlenA(szcontent)+1, wszcontent);

		}
		else if (argc >= 3)
		{
			string path = argv[1];
			if (path.back() == '/' || path.back() == '\\')
			{
				path = path.substr(0, path.length() - 1);
			}
			MultiByteToWideChar(CP_ACP, 0, argv[2], -1, wszcontent, sizeof(wszcontent)/sizeof(wchar_t));

			printf("start searching string:\"%s\" in path:\"%s\",please wait patiently...\r\n\r\n", argv[2], path.c_str());

			ret = GetFileAttributesA(path.c_str());

			char szcontent[1024];
			upcase(argv[2], lstrlenA(argv[2]), szcontent);

			if (ret & FILE_ATTRIBUTE_DIRECTORY)
			{
				ret = SearchInDir(path.c_str(), 1, szcontent, lstrlenA(szcontent)+1, wszcontent);
			}
			else {
				ret = SearchInFile(path.c_str(), szcontent, lstrlenA(szcontent)+1,wszcontent);
			}
		}
	}

	if (ret == 0)
	{
		printf("Search string in path failed!\r\n");
		ret = getchar();
		return FALSE;
	}
	else
	{
		printf("Search string successfully, found string position(offset from beginning):%u\r\n", ret);
		ret = getchar();
		return TRUE;
	}
	return TRUE;
}