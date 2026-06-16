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



/**
 * 在内存中的 PE 文件数据里查找指定的 DLL 和函数名
 *
 * @param data 文件读入内存后的内容
 * @param size 文件字节大小
 * @param filename 目标 DLL 名称 (例如 "kernel32.dll")
 * @param funcname 目标函数名称 (例如 "CreateFileA")
 * @return 1 表示找到, 0 表示未找到, -1 表示解析错误
 */
int GetFunction(char* data, int size, char* filename, char* funcname) {
	if (!data || size < sizeof(IMAGE_DOS_HEADER)) {
		return -1;
	}

	// 1. 定位 DOS 头和 NT 头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)data;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return -1; // 不是有效的 PE 文件
	}

	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(data + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
		return -1;
	}

	// 2. 获取导入表的位置
	// 导入表位于数据目录的第 IMAGE_DIRECTORY_ENTRY_IMPORT 项
	IMAGE_DATA_DIRECTORY importDir = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDir.VirtualAddress == 0 || importDir.Size == 0) {
		return 0; // 没有导入表
	}

	// 将 RVA 转换为文件偏移 (FOA)
	// 注意：这里假设 RVA 和 FOA 在头部区域是线性对应的，或者使用简单的减法
	// 严谨的做法应该遍历节表计算，但对于导入表通常在头部，直接减基址通常可行
	// 这里为了演示简单，直接使用指针运算，因为 data 已经是内存映射或读取的基址
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(data + importDir.VirtualAddress);

	// 3. 遍历导入表
	// 导入表以全 0 的 IMAGE_IMPORT_DESCRIPTOR 结构结束
	while (pImportDesc->Name != 0) {
		// 获取 DLL 名称
		// Name 是 RVA，需要转换为 data 中的指针
		char* dllName = (char*)(data + pImportDesc->Name);

		// 比较 DLL 名称 (忽略大小写)
		if (_stricmp(dllName, filename) == 0) {
			// 找到了对应的 DLL，现在开始查找函数

			// 获取导入名称表 (INT) 的位置
			// 通常使用 OriginalFirstThunk，如果为 0 则使用 FirstThunk (但在绑定导入时 FirstThunk 会被修改)
			DWORD thunkOffset = pImportDesc->OriginalFirstThunk;
			if (thunkOffset == 0) {
				thunkOffset = pImportDesc->FirstThunk;
			}

			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(data + thunkOffset);
			PIMAGE_THUNK_DATA pOrigThunk = (PIMAGE_THUNK_DATA)(data + pImportDesc->OriginalFirstThunk);

			// 遍历该 DLL 的导入函数列表
			// 当 AddressOfData 为 0 时结束
			while (pOrigThunk->u1.AddressOfData != 0) {
				// 检查最高位，判断是按序号导入还是按名称导入
				if (!(pOrigThunk->u1.AddressOfData & IMAGE_ORDINAL_FLAG)) {
					// 按名称导入
					// AddressOfData 指向 IMAGE_IMPORT_BY_NAME 结构
					PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME)(data + pOrigThunk->u1.AddressOfData);

					// 比较函数名
					if (strcmp((char*)pImportByName->Name, funcname) == 0) {
						return 1; // 找到了 DLL 和函数
					}
				}
				// 如果是按序号导入 (最高位为1)，则没有函数名，跳过
				pOrigThunk++;
				pThunk++;
			}
			// 如果 DLL 找到了但函数没找到，根据需求返回 0 或继续查找其他 DLL
			// 这里假设只要 DLL 存在但函数不存在就算未找到
			return 0;
		}
		pImportDesc++;
	}

	return 0; // 遍历完所有 DLL 都没找到
}


int SearchFunctionInFile(const char* szFileName,  char* pefn, char * funcname)
{
	int ret = 0;

	//printf("%s CreateFle:%s\r\n", __FUNCTION__, szFileName);

	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//printf("%s CreateFle:%s error:%d\r\n", __FUNCTION__, szFileName, GetLastError());
		return FALSE;
	}

	DWORD fsHigh = 0;
	unsigned int fSize = GetFileSize(hFile, &fsHigh);
	if (fSize <= 0 || fsHigh)
	{
		printf("%s GetFileSize:%s error:%d,high size:%x,low size:%x\r\n",
			__FUNCTION__, szFileName, GetLastError(),fsHigh,fSize);
		CloseHandle(hFile);
		return FALSE;
	}

	char* pFileBuf = new char[fSize + 16];
	if(pFileBuf== 0){
		printf("%s new char error\r\n", __FUNCTION__);
		CloseHandle(hFile);
		return FALSE;
	}
	DWORD dwCnt = 0;
	ret = ReadFile(hFile, pFileBuf, fSize, &dwCnt, 0);
	*(pFileBuf + fSize) = 0;
	CloseHandle(hFile);
	if (ret == 0)
	{
		delete[] pFileBuf;
		return 0;
	}

	ret = IsPeFile(pFileBuf, fSize);
	if (ret) {
		int peSize = GetImageSize(pFileBuf);
		char* peBuf = new char[peSize + 0x1000];
		if (peBuf) {
			ret = MapPeFile(pFileBuf, peBuf, szFileName);
			if (ret) {
				ret = GetFunction(peBuf, peSize, pefn, funcname);
			}
			delete[]peBuf;
			if (ret) {
				printf("File:%s call function:%s!%s\r\n", szFileName, pefn, funcname);
			}
		}
	}

	delete[] pFileBuf;
	return ret;
}

int SearchFunctionInDir(string PreStrPath, int iLayer, char* fn, char * funcname)
{
	int ret = 0;

	static int result = 0;

	string strPath = PreStrPath;
	if (PreStrPath.back() == '\\' || PreStrPath.back() == '/') {
	}
	else {
		strPath += "\\";
	}
	strPath += "*.*";

	WIN32_FIND_DATAA stWfd = { 0 };
	HANDLE hFind = FindFirstFileA(strPath.c_str(), &stWfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("%s FindFirstFileA:%s error:%d\r\n", __FUNCTION__,strPath.c_str(), GetLastError());
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

			string strNextPath = PreStrPath;
			if (strNextPath.back() == '\\' || strNextPath.back() == '/') {
			}
			else {
				strNextPath += "\\";
			}
			strNextPath += stWfd.cFileName;

			ret = SearchFunctionInDir(strNextPath, iLayer + 1, fn, funcname);
		}
		else
		{
			string szFileName = PreStrPath;
			if (PreStrPath.back() == '\\' || PreStrPath.back() == '/') {
			}
			else {
				szFileName += "\\";
			}
			szFileName += stWfd.cFileName;

			int iFilePos = SearchFunctionInFile(szFileName.c_str(), fn, funcname);
			if (iFilePos)
			{
				result++;
			}
		}
	} while (FindNextFileA(hFind, &stWfd));

	FindClose(hFind);

	return result;
}


int SearchInFile(const char* szFileName, const char* pDstContent,int len, const wchar_t* wszcontent)
{
	int ret = 0;

	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//printf("%s CreateFle:%s error:%d\r\n", __FUNCTION__, szFileName, GetLastError());
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
	if(pFileBuf == 0){
		printf("%s new char error\r\n", __FUNCTION__);
		CloseHandle(hFile);
		return FALSE;
	}
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

	string strPath = PreStrPath;
	if (strPath.back() == '\\' || strPath.back() == '/') {
	}
	else {
		strPath += "\\";
	}
	strPath += "*.*";

	WIN32_FIND_DATAA stWfd = { 0 };
	HANDLE hFind = FindFirstFileA(strPath.c_str(), &stWfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("%s FindFirstFileA:%s error:%d\r\n", __FUNCTION__, strPath.c_str(), GetLastError());
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

			string strNextPath = PreStrPath;
			if (strNextPath.back() == '\\' || strNextPath.back() == '/') {

			}
			else {
				strNextPath += "\\";
			}
			strNextPath += stWfd.cFileName;
			
			ret = SearchInDir(strNextPath.c_str(), iLayer + 1, pDstContent, len, wszcontent);
		}
		else
		{
			string szFileName = PreStrPath;
			if (szFileName.back() == '\\' || szFileName.back() == '/') {

			}
			else {
				szFileName += '\\';
			}
			szFileName += stWfd.cFileName;
						
			int iFilePos = SearchInFile(szFileName.c_str(), pDstContent, len, wszcontent);
			if (iFilePos)
			{
				result++;
			}
		}
	} while (FindNextFileA(hFind, &stWfd));

	FindClose(hFind);

	return result;
}


int SearchString(char* option,char* tag,char * filpath) {
	int ret = 0;
	int srclen = 0;
	char szContent[0x1000] = { 0 };

	string str = tag;
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
		if (lstrcmpiA(option, "function") == 0) {
			char* split = strchr((char*)str.c_str(), '!');
			if (split) {
				string fn = str.substr(0, split - str.c_str());
				string funcname = split + 1;
				ret = SearchFunctionInDir(path.c_str(), 1, (char*)fn.c_str(), (char*)funcname.c_str());
			}
		}
		else {
			ret = SearchInDir(path.c_str(), 1, szContent, srclen, wszContent);
		}
		
	}
	else if (ret & FILE_ATTRIBUTE_ARCHIVE) {
		if (lstrcmpiA(option, "function") == 0) {
			char* split = strchr((char*)str.c_str(), '!');
			if (split) {
				string fn = str.substr(0, split - str.c_str());
				string funcname = split + 1;
				ret = SearchFunctionInFile(path.c_str(),(char*)fn.c_str(), (char*)funcname.c_str());
			}
		}
		else {
			ret = SearchInFile(path.c_str(), szContent, srclen, wszContent);
		}	
	}
	return ret;
}
