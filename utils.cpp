





#include <Windows.h>
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