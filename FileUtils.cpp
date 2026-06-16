
#include <Windows.h>
#include "FileUtils.h"



#include <restartmanager.h>
#include <stdio.h>

using namespace std;

#pragma comment(lib, "Rstrtmgr.lib")

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






#include <windows.h>
#include <RestartManager.h>
#include <string>
#include <iostream>

#pragma comment(lib, "Rstrtmgr.lib")

// 辅助函数：通过 PID 获取进程的全路径名称
bool GetProcessPathByPID(DWORD pid, std::wstring& processPath) {
    if (pid == 0) return false;

    // PROCESS_QUERY_LIMITED_INFORMATION 权限较低，更容易获取成功
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return false; // 权限不足，无法打开进程
    }

    WCHAR buffer[MAX_PATH] = { 0 };
    DWORD bufSize = MAX_PATH;

    // 获取完整路径 (Windows Vista+ 支持)
    BOOL bSuccess = QueryFullProcessImageNameW(hProcess, 0, buffer, &bufSize);

    CloseHandle(hProcess);

    if (bSuccess) {
        processPath = buffer;
        return true;
    }
    return false;
}

// 主函数：查找占用指定文件的进程路径
int FindProcessLockingFile(const std::wstring& filePath, std::wstring& lockingProcessPath) {
    if (filePath.empty()) return false;

    // 1. 确保传入的是绝对路径 (RM API 必须使用绝对路径)
    WCHAR absPath[MAX_PATH] = { 0 };
    if (GetFullPathNameW(filePath.c_str(), MAX_PATH, absPath, NULL) == 0) {
        return false; // 转换绝对路径失败
    }

    DWORD sessionHandle = 0;
    WCHAR sessionKey[CCH_RM_SESSION_KEY + 1] = { 0 };

    // 2. 创建 Restart Manager 会话
    DWORD dwError = RmStartSession(&sessionHandle, 0, sessionKey);
    if (dwError != ERROR_SUCCESS) return false;

    // 3. 注册要查询的文件
    PCWSTR files[] = { absPath };
    dwError = RmRegisterResources(sessionHandle, 1, files, 0, NULL, 0, NULL);
    if (dwError != ERROR_SUCCESS) {
        RmEndSession(sessionHandle);
        return false;
    }

    // 4. 获取占用该文件的进程列表
    UINT nProcInfoNeeded = 0;
    UINT nProcInfo = 0; // 第一次传入 0，专门为了获取需要的数量
    DWORD dwReason = 0;

    // 第一次调用，获取所需的数组大小
    dwError = RmGetList(sessionHandle, &nProcInfoNeeded, &nProcInfo, NULL, &dwReason);

    if (dwError == ERROR_MORE_DATA || nProcInfoNeeded > 0) {
        // 分配所需大小的数组
        RM_PROCESS_INFO* ppi = new (std::nothrow) RM_PROCESS_INFO[nProcInfoNeeded];
        if (ppi == nullptr) {
            RmEndSession(sessionHandle);
            return false;
        }

        nProcInfo = nProcInfoNeeded;
        // 第二次调用，获取实际数据
        dwError = RmGetList(sessionHandle, &nProcInfoNeeded, &nProcInfo, ppi, &dwReason);

        if (dwError == ERROR_SUCCESS && nProcInfo > 0) {
            // 遍历找到的第一个有效进程 (可能有多个进程占用同一文件)
            for (UINT i = 0; i < nProcInfo; ++i) {
                std::wstring procPath;
                // 优先通过 PID 获取真实路径
                if (GetProcessPathByPID(ppi[i].Process.dwProcessId, procPath)) {
                    lockingProcessPath = procPath;
                    delete[] ppi;
                    RmEndSession(sessionHandle);
                    return true;
                }
                else {
                    // 如果由于权限不足无法获取路径，退而求其次使用 RM 提供的 AppName
                    // 注意：strAppName 通常不准确，可能是空或者缩写
                    if (wcslen(ppi[i].strAppName) > 0) {
                        lockingProcessPath = ppi[i].strAppName;
                        delete[] ppi;
                        RmEndSession(sessionHandle);
                        return true;
                    }
                }
            }
        }
        delete[] ppi;
    }
    else if (dwError == ERROR_SUCCESS && nProcInfo == 0) {
        // 文件没有被任何进程占用
    }

    RmEndSession(sessionHandle);
    return (dwError == ERROR_SUCCESS && nProcInfo > 0);
}

