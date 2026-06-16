

#include <Windows.h>

#include <stdio.h>


#include "utils.h"

#include <lmwksta.h>
#include <lmerr.h>

#pragma comment(lib,"Netapi32.lib")


#pragma comment(lib,"wtsapi32.lib")

#pragma comment(lib,"Userenv.lib")


#pragma warning(disable: 4996)








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
	for (int i = 0; i < len; i+=2) {
		if (str[i] == '\\' && (str[i + 1] == 'x' || str[i + 1] == 'X')) {
			i += 2;
		}
		else {
			break;
		}

		unsigned char low = str[i + 1];
		unsigned char high = str[i];

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


int GetWindowsVersion()
{
	WKSTA_INFO_100* wkstaInfo = NULL;
	NET_API_STATUS netStatus = NetWkstaGetInfo(NULL, 100, (LPBYTE*)&wkstaInfo);
	if (netStatus == NERR_Success)
	{
		DWORD dwMajVer = wkstaInfo->wki100_ver_major;
		DWORD dwMinVer = wkstaInfo->wki100_ver_minor;
		DWORD dwVersion = (DWORD)MAKELONG(dwMinVer, dwMajVer);
		//netStatus = NetApiBufferFree(wkstaInfo);

		int iSystemVersion = 0;
		if (dwVersion < 0x50000)
		{
			iSystemVersion = SYSTEM_VERSION_WIN9X;
		}
		else if (dwVersion == 0x50000)
		{

			iSystemVersion = SYSTEM_VERSION_WIN2000;
		}
		else if (dwVersion > 0x50000 && dwVersion < 0x60000)
		{

			iSystemVersion = SYSTEM_VERSION_XP;
		}
		else if (dwVersion == 0x60000)
		{

			iSystemVersion = SYSTEM_VERSION_VISTA;
		}
		else if (dwVersion == 0x60001)
		{

			iSystemVersion = SYSTEM_VERSION_WIN7;
		}
		else if (dwVersion >= 0x60002 && dwVersion <= 0x60003)
		{

			iSystemVersion = SYSTEM_VERSION_WIN8;
		}
		else if (dwVersion >= 0x60003 || dwVersion == 0x100000)
		{

			iSystemVersion = SYSTEM_VERSION_WIN10;
		}
		else
		{
			iSystemVersion = SYSTEM_VERSION_UNKNOW;
		}
		return iSystemVersion;
	}

	return FALSE;
}

BOOL IsCpu64Bit()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		return 64;
	else
		return 32;
}


int IsSystem64Bit()
{
	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	BOOL bIsWow64 = FALSE;
	//IsWow64Process is not available on all supported versions of Windows
	char szIsWow64Process[] = { 'I','s','W','o','w','6','4','P','r','o','c','e','s','s',0 };
	HMODULE lpDllKernel32 = LoadLibraryA("kernel32.dll");
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(lpDllKernel32, szIsWow64Process);
	if (NULL != fnIsWow64Process)
	{
		int iRet = fnIsWow64Process(GetCurrentProcess(), &bIsWow64);
		if (iRet && bIsWow64)
		{
			return 64;
		}
	}

	return 32;
}


int QueryRegistryValue(HKEY hMainKey, char* szSubKey,unsigned long type, char* szKeyName, unsigned char* szKeyValue)
{
	unsigned long iType = KEY_READ;
	DWORD dwDisPos = REG_OPENED_EXISTING_KEY;
	HKEY hKey = 0;
	int iRes = 0;


	int iCpuBits = IsCpu64Bit();

	int ver = GetWindowsVersion();
	PVOID dwWow64Value=0;
	if (ver >= 4)
	{
		if (iCpuBits == 64 && hMainKey == HKEY_LOCAL_MACHINE)
		{
			HMODULE lpDllKernel32 = LoadLibraryA("kernel32.dll");
			typedef void(__stdcall* ptrWow64DisableWow64FsRedirection)(PVOID);
			ptrWow64DisableWow64FsRedirection lpWow64DisableWow64FsRedirection =
				(ptrWow64DisableWow64FsRedirection)GetProcAddress(lpDllKernel32, "Wow64DisableWow64FsRedirection");
			if (lpWow64DisableWow64FsRedirection )
			{
				lpWow64DisableWow64FsRedirection(&dwWow64Value);
				iType |= KEY_WOW64_64KEY;
			}
		}
	}

	//KEY_WEITE will cause error like winlogon
	//winlogon :Registry symbolic links should only be used for for application compatibility when absolutely necessary.
	iRes = RegCreateKeyExA(hMainKey, szSubKey, 0, REG_NONE, REG_OPTION_NON_VOLATILE, iType, 0, &hKey, &dwDisPos);

	if (ver >= 4)
	{
		if (iCpuBits == 64 && hMainKey == HKEY_LOCAL_MACHINE)
		{
			HMODULE lpDllKernel32 = LoadLibraryA("kernel32.dll");
			typedef void(__stdcall* ptrWow64RevertWow64FsRedirection)(PVOID);
			ptrWow64RevertWow64FsRedirection lpWow64RevertWow64FsRedirection =
				(ptrWow64RevertWow64FsRedirection)GetProcAddress(lpDllKernel32, "Wow64RevertWow64FsRedirection");
			if (lpWow64RevertWow64FsRedirection )
			{
				lpWow64RevertWow64FsRedirection(&dwWow64Value);
			}
		}
	}

	if (iRes != ERROR_SUCCESS)
	{
		return FALSE;
	}

	//if value is 234 ,it means out buffer is limit
	//2 is not value
	unsigned long iQueryLen = MAX_PATH;
	iRes = RegQueryValueExA(hKey, szKeyName, 0, &type, szKeyValue, &iQueryLen);
	if (iRes == ERROR_SUCCESS)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int IsPeFile(char * data,int size) {
	if (!data || size < sizeof(IMAGE_DOS_HEADER)) {
		return 0;
	}

	// 1. 定位 DOS 头和 NT 头
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)data;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return 0; // 不是有效的 PE 文件
	}

	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(data + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
		return 0;
	}
	return 1;
}

DWORD GetImageSize(char* pFileBuff)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileBuff;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pFileBuff + pDos->e_lfanew);

	int is64bit = 0;
	int magic = pNt->OptionalHeader.Magic;
	if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
		is64bit = 1;
	}

	if(is64bit) {
		PIMAGE_NT_HEADERS64 pNt64 = (PIMAGE_NT_HEADERS64)pNt;
		return pNt64->OptionalHeader.SizeOfImage;
	}

	DWORD dwSizeOfImage = pNt->OptionalHeader.SizeOfImage;

	return dwSizeOfImage;
}


DWORD GetPeEntry(char* pe) {
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pe;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pe + pDos->e_lfanew);
	DWORD entry = pNt->OptionalHeader.AddressOfEntryPoint;

	return entry;
}


DWORD GetPeType(DWORD chBaseAddress) {
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)chBaseAddress;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(chBaseAddress + dos->e_lfanew);

	return nt->FileHeader.Characteristics;
}

DWORD GetImageBase(char* pFileBuff)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileBuff;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pFileBuff + pDos->e_lfanew);
	DWORD imagebase = pNt->OptionalHeader.ImageBase;

	return imagebase;
}



int MapPeFile(char* pFileBuff, char* chBaseAddress,const char *fn)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileBuff;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pFileBuff + pDos->e_lfanew);

	int is64bit = 0;
	int magic = pNt->OptionalHeader.Magic;
	if(magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC){
		is64bit = 1;
	}

	DWORD dwSizeOfHeaders = pNt->OptionalHeader.SizeOfHeaders;
	memcpy(chBaseAddress, pFileBuff, dwSizeOfHeaders);

	//PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);
	PIMAGE_SECTION_HEADER pSection = 0;
	if (is64bit) {
		pSection = (PIMAGE_SECTION_HEADER)((char*)pNt + sizeof(IMAGE_NT_HEADERS64));
	}
	else {
		pSection = (PIMAGE_SECTION_HEADER)((char*)pNt + sizeof(IMAGE_NT_HEADERS32));
	}

	DWORD totalSize = 0;

	DWORD vs = GetImageSize(pFileBuff);

	int nNumerOfSections = pNt->FileHeader.NumberOfSections;
	for (int i = 0; i < nNumerOfSections; i++)
	{
		if ((0 == pSection[i].VirtualAddress) || (0 == pSection[i].SizeOfRawData))
		{
			continue;
		}

		totalSize += pSection[i].SizeOfRawData;
		if (pSection[i].SizeOfRawData > vs) {
			printf("file:%s Image size:%x SizeOfRawData:%x section:%s error\r\n",fn, vs, pSection[i].SizeOfRawData,
				pSection[i].Name);
			return 0;
		}

		char* chDestMem = (char*)((DWORD)chBaseAddress + pSection[i].VirtualAddress);
		char* chSrcMem = (char*)((DWORD)pFileBuff + pSection[i].PointerToRawData);
		DWORD dwSizeOfRawData = pSection[i].SizeOfRawData;
		memcpy(chDestMem, chSrcMem, dwSizeOfRawData);
	}

	return TRUE;
}








int ResourceParser(DWORD module, DWORD resbase, PIMAGE_RESOURCE_DIRECTORY resdir, int level, DWORD id, DWORD type, DWORD* offset, DWORD* size)
{
	int ret = 0;
	int find = 0;
	char buf[1024];

	if (resdir->NumberOfIdEntries == 0 && resdir->NumberOfNamedEntries == 0)
	{
		return 0;
	}

	int cnt = resdir->NumberOfIdEntries + resdir->NumberOfNamedEntries;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY res_dir_entry =
		(PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)resdir + sizeof(IMAGE_RESOURCE_DIRECTORY));
	for (int i = 0; i < cnt; i++)
	{

		if (res_dir_entry->DataIsDirectory)
		{
			if (res_dir_entry->NameIsString)
			{
				PIMAGE_RESOURCE_DIR_STRING_U str =
					(PIMAGE_RESOURCE_DIR_STRING_U)(resbase + res_dir_entry->NameOffset);
				ret = WideCharToMultiByte(CP_ACP,0,(WCHAR*)str->NameString, str->Length, buf,sizeof(buf),0,0);
				//__printf("get resource level:%u name:%s\r\n", (char*)level, buf);
			}
			else {
				//__printf("get resource level:%u type:%u:\r\n", (char*)level, name);
			}

			//name高位是0的话，第一层是类型，如上图rt定义
			if (level == 1)
			{
				if (type != res_dir_entry->Name)
				{
					res_dir_entry++;
					continue;
				}
			}

			//第二层是id号码，第三层是语言标识
			if (level == 2)
			{
				if (res_dir_entry->Name != id)
				{
					res_dir_entry++;
					continue;
				}
			}

			int nextlevel = level + 1;

			PIMAGE_RESOURCE_DIRECTORY nextdir =
				(PIMAGE_RESOURCE_DIRECTORY)(resbase + res_dir_entry->OffsetToDirectory);
			ret = ResourceParser(module, resbase, nextdir, nextlevel, id, type, offset, size);
		}
		else {
			if (res_dir_entry->NameIsString)
			{
				PIMAGE_RESOURCE_DIR_STRING_U str =
					(PIMAGE_RESOURCE_DIR_STRING_U)(resbase + res_dir_entry->NameOffset);

				ret = WideCharToMultiByte(CP_ACP, 0, (WCHAR*)str->NameString, str->Length, buf, sizeof(buf), 0, 0);
				//__printf("get item name:%s,size:%u,address:%x\r\n", buf,size,offset);
			}
			else {
				//__printf("get item type:%u,size:%u,address:%x\r\n",(char*) name,size,offset);
			}

			PIMAGE_RESOURCE_DATA_ENTRY res_data_entry =
				(PIMAGE_RESOURCE_DATA_ENTRY)(resbase + res_dir_entry->OffsetToData);
			char* resoffset = (char*)res_data_entry->OffsetToData + module;
			DWORD ressize = res_data_entry->Size;

			*offset = res_data_entry->OffsetToData + module;
			*size = res_data_entry->Size;
		}
		res_dir_entry++;
	}

	return 0;
}



int GetResFromID(DWORD module, int id, DWORD type, DWORD* offset, DWORD* size)
{
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(module + dos->e_lfanew);
	PIMAGE_FILE_HEADER fh = &nt->FileHeader;

	PIMAGE_RESOURCE_DIRECTORY res = (PIMAGE_RESOURCE_DIRECTORY)(nt->OptionalHeader.DataDirectory[2].VirtualAddress + module);

	*offset = 0;
	*size = 0;
	ResourceParser(module, (DWORD)res, res, 1, id, type, offset, size);

	return 0;
}


int GetResFromName(DWORD module, const char* name, DWORD type, DWORD* offset, DWORD* size)
{
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(module + dos->e_lfanew);
	PIMAGE_FILE_HEADER fh = &nt->FileHeader;

	PIMAGE_RESOURCE_DIRECTORY res =
		(PIMAGE_RESOURCE_DIRECTORY)(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress + module);

	*offset = 0;
	*size = 0;
	ResourceParser(module, (DWORD)res, res, 1, (DWORD)name, type, offset, size);

	return 0;
}


#include <string>
#include <locale>
#include <codecvt> // C++17 中已弃用

// string (UTF-8) 转 wstring (UTF-16)
std::wstring StringToWString_UTF8(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

// wstring (UTF-16) 转 string (UTF-8)
std::string WStringToString_UTF8(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(wstr);
}