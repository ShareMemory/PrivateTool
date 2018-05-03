#ifndef SvcControlH
#define SvcControlH
#include "PrivateDefine.h"

#ifdef _WIN32
#include <tchar.h>
#else
  typedef char _TCHAR;
  #define _tmain main
#endif
#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
//#include <strsafe.h>
#include <aclapi.h>
#include <stdio.h>
#include <string>
#include "PrivateTool.h"

#pragma comment(lib, "advapi32.lib")

#define MTSERVER_NAME TEXT("MTDigitizer")
#define MTPLATFORM TEXT("MultiTouchPlatform.exe")
#define MTPLATFORMHELPER TEXT("MultiTouchPlatformHelper.exe")

class SvcControl {
private:
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	bool __stdcall StopDependentServices(void);
	void __stdcall CleanUp(SC_HANDLE schscmanager, SC_HANDLE schService);

	//Process

protected:

public:
	SvcControl();
	bool _stdcall ExtractProcessOwner( HANDLE hProcess_i, std::tstring & wsowner );
	bool _stdcall DoStopService();
	bool _stdcall DoStopProcess();
	bool _stdcall DoStopProcess(const tchar * processname);
	bool _stdcall DoStartSvc(const tchar *servicename);
	bool _stdcall DoStartProcess(const tchar* wccmd, const tchar* wcworkingdir = NULL);
	bool _stdcall DoStartProcess(const tchar* wccmd, DWORD * elapsedtime, DWORD * returnvalue);
	bool _stdcall QuaryProcess(const tchar *processname);
	bool _stdcall GetOsVersion(RTL_OSVERSIONINFOEXW* pk_OsVer);
	//bool _stdcall GetWinVersion(OSVERSIONINFOEXW * pVersion);
	bool _stdcall IsWindows8Later();
    bool _stdcall Is64bit();
};

extern SvcControl g_svcControl;
#endif