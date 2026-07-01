


#include <windows.h>
#include <winsvc.h>
#include <conio.h>
#include <stdio.h>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")




BOOL LoadNTDriver(WCHAR* lpszDriverName, WCHAR* lpszDriverPath, int servicetype, int boottype, WCHAR* groupname)
{
	WCHAR szshow[256];
	WCHAR szDriverImagePath[256];

	GetFullPathNameW(lpszDriverPath, 256, szDriverImagePath, NULL);

	char szfullpath[1024] = { 0 };
	int len = WideCharToMultiByte(CP_ACP, 0, szDriverImagePath, -1, szfullpath, sizeof(szfullpath), 0, 0);
	szfullpath[len] = 0;
	printf("full path:%s\r\n", szfullpath);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;

	hServiceMgr = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenSCManager() ok ! \n");
	}

	hServiceDDK = CreateServiceW(hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字
		lpszDriverName, // 注册表驱动程序的 DisplayName 值
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限
		servicetype,
		//SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序
		boottype,
		//SERVICE_AUTO_START, // 注册表驱动程序的 Start 值
		SERVICE_ERROR_NORMAL, // 注册表驱动程序的 ErrorControl 值
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值
		groupname,
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;

	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			printf("CreateServiceW() Faild %d ! \n", dwRtn);

			wsprintfW(szshow, L"CreateServiceW() Faild %d!driver name:%ws,path:%ws,service type:%d,boot:%d\n",
				dwRtn, lpszDriverName, szDriverImagePath, servicetype, boottype);
			MessageBoxW(0, szshow, szshow, MB_OK);

			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			printf("CreateServiceW() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
		}

		hServiceDDK = OpenServiceW(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			dwRtn = GetLastError();
			printf("OpenService() Faild %d ! \n", dwRtn);

			wsprintfW(szshow, L"OpenService() Faild %d ! \n", dwRtn);
			MessageBoxW(0, szshow, szshow, MB_OK);

			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			printf("OpenService() ok ! \n");
		}
	}
	else
	{
		printf("CreateServiceW() ok ! \n");
	}

	bRet = StartServiceW(hServiceDDK, 0, 0);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			wsprintfW(szshow, L"StartService() Faild %d ! \n", dwRtn);
			MessageBoxW(0, szshow, szshow, MB_OK);

			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				printf("StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				printf("StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;

BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

BOOL UnloadNTDriver(WCHAR* szSvrName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;
	SERVICE_STATUS SvrSta;

	hServiceMgr = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		printf("OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenSCManager() ok ! \n");
	}

	hServiceDDK = OpenServiceW(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);
	if (hServiceDDK == NULL)
	{
		printf("OpenService() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenService() ok ! \n");
	}

	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
	{
		printf("ControlService() Faild %d !\n", GetLastError());
	}
	else
	{
		printf("ControlService() ok !\n");
	}
	if (!DeleteService(hServiceDDK))
	{
		printf("DeleteSrevice() Faild %d !\n", GetLastError());
	}
	else
	{
		printf("DelServer:eleteSrevice() ok !\n");
	}
	bRet = TRUE;
BeforeLeave:

	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}


void TestDriver(char * drvname)
{
	char drivername[1024];
	wsprintfA(drivername, "\\\\.\\%s", drvname);
	HANDLE hDevice = CreateFileA(drivername,
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		printf("Create Device ok ! \n");
	}
	else
	{
		printf("Create Device faild %d ! \n", GetLastError());
	}
	CloseHandle(hDevice);
}



int RemoveDriver(char* drivername) {
	int bRet = 0;
	wchar_t drvname[1024] = { 0 };

	MultiByteToWideChar(CP_ACP, 0, drivername, -1, drvname, sizeof(drvname) / 2);
	bRet = UnloadNTDriver(drvname);
	if (!bRet)
	{
		printf("UnloadNTDriver error\n");
		return 0;
	}
	return 0;
}




int LoadDriver(char * driverpath)
{
	char * filename = PathFindFileNameA(driverpath);
	wchar_t drvname[1024] = { 0 };
	wchar_t drvpath[1024] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, filename, -1, drvname, sizeof(drvname)/2);
	MultiByteToWideChar(CP_ACP, 0, driverpath, -1, drvpath, sizeof(drvpath)/2);

	BOOL bRet = LoadNTDriver(drvname, drvpath, SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START, 0);
	if (!bRet)
	{
		printf("LoadNTDriver error\n");
		return 0;
	}

	return 0;
}
