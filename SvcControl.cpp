#include "SvcControl.h"
#include <sstream>

SvcControl g_svcControl;

SvcControl::SvcControl()
{}

bool _stdcall SvcControl::ExtractProcessOwner( HANDLE hProcess_i,
						  std::tstring & wsowner )
{
   // Get process token
   HANDLE hProcessToken = NULL;
   if ( !::OpenProcessToken( hProcess_i, TOKEN_READ, &hProcessToken ) || !hProcessToken )
   {
	  return false;
   }

   // First get size needed, TokenUser indicates we want user information from given token
   DWORD dwProcessTokenInfoAllocSize = 0;
   ::GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &dwProcessTokenInfoAllocSize);

   // Call should have failed due to zero-length buffer.
   if( ::GetLastError() == ERROR_INSUFFICIENT_BUFFER )
   {
      // Allocate buffer for user information in the token.
	  PTOKEN_USER pUserToken = reinterpret_cast<PTOKEN_USER>( new BYTE[dwProcessTokenInfoAllocSize] );
      if (pUserToken != NULL)
	  {
         // Now get user information in the allocated buffer
		 if (::GetTokenInformation( hProcessToken, TokenUser, pUserToken, dwProcessTokenInfoAllocSize, &dwProcessTokenInfoAllocSize ))
         {
            // Some vars that we may need
            SID_NAME_USE   snuSIDNameUse;
            TCHAR          szUser[MAX_PATH] = { 0 };
            DWORD          dwUserNameLength = MAX_PATH;
            TCHAR          szDomain[MAX_PATH] = { 0 };
            DWORD          dwDomainNameLength = MAX_PATH;

			// Retrieve user name and domain name based on user's SID.
			if ( ::LookupAccountSid( NULL,
									 pUserToken->User.Sid,
                                     szUser,
                                     &dwUserNameLength,
                                     szDomain,
                                     &dwDomainNameLength,
                                     &snuSIDNameUse ))
            {
			   // Prepare user name string
			   wsowner = TEXT("\\\\");
			   wsowner += szDomain;
			   wsowner += TEXT("\\");
			   wsowner += szUser;

               // We are done!
			   CloseHandle( hProcessToken );
			   delete [] pUserToken;

               // We succeeded
			   return true;
            }//End if
         }// End if

         delete [] pUserToken;
      }// End if
   }// End if

   CloseHandle( hProcessToken );

   // Oops trouble
   return false;
}// End GetProcessOwner

bool _stdcall SvcControl::DoStopService()
{
	SERVICE_STATUS_PROCESS ssp;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwWaitTime;

	// Get a handle to the SCM database.

	schSCManager = OpenSCManager(NULL, // local computer
		NULL, // ServicesActive database
		SC_MANAGER_ALL_ACCESS); // full access rights

	if (schSCManager == NULL)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return false;
	}

	// Get a handle to the service.

	schService = OpenService(schSCManager, // SCM database
		MTSERVER_NAME, // name of service
		SERVICE_STOP |
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return true;
	}

	// Make sure the service is not already stopped.

	if ( !QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded ) )
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CleanUp(schSCManager, schService);
		return true;
	}

	if ( ssp.dwCurrentState == SERVICE_STOPPED )
	{
		printf("Service is already stopped.\n");
		CleanUp(schSCManager, schService);
		return true;;
	}

	 // If a stop is pending, wait for it.

	while ( ssp.dwCurrentState == SERVICE_STOP_PENDING )
	{
		printf("Service stop pending...\n");

		// Do not wait longer than the wait hint. A good interval is
		// one-tenth of the wait hint but not less than 1 second
		// and not more than 10 seconds.

		dwWaitTime = ssp.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		if ( !QueryServiceStatusEx(
				 schService,
				 SC_STATUS_PROCESS_INFO,
				 (LPBYTE)&ssp,
				 sizeof(SERVICE_STATUS_PROCESS),
				 &dwBytesNeeded ) )
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			CleanUp(schSCManager, schService);
			return false;
		}

		if ( ssp.dwCurrentState == SERVICE_STOPPED )
		{
			printf("Service stopped successfully.\n");
			CleanUp(schSCManager, schService);
			return true;
		}

		if ( GetTickCount() - dwStartTime > dwTimeout )
		{
			printf("Service stop timed out.\n");
			CleanUp(schSCManager, schService);
			return false;
		}
	}

	// If the service is running, dependencies must be stopped first.

	StopDependentServices();

	// Send a stop code to the service.

	if ( !ControlService(
			schService,
			SERVICE_CONTROL_STOP,
			(LPSERVICE_STATUS) &ssp ) )
	{
		printf( "ControlService failed (%d)\n", GetLastError() );
		CleanUp(schSCManager, schService);
		return false;
	}

	// Wait for the service to stop.

	while ( ssp.dwCurrentState != SERVICE_STOPPED )
	{
		Sleep( ssp.dwWaitHint );
		if ( !QueryServiceStatusEx(
				schService,
				SC_STATUS_PROCESS_INFO,
				(LPBYTE)&ssp,
				sizeof(SERVICE_STATUS_PROCESS),
				&dwBytesNeeded ) )
		{
				printf( "QueryServiceStatusEx failed (%d)\n", GetLastError() );
			CleanUp(schSCManager, schService);
			return false;
		}

		if ( ssp.dwCurrentState == SERVICE_STOPPED )
			break;

		if ( GetTickCount() - dwStartTime > dwTimeout )
		{
//				printf( "Wait timed out\n" );
			CleanUp(schSCManager, schService);
			return false;
		}
	}
	printf("Service stopped successfully\n");
	CleanUp(schSCManager, schService);
	return true;
}

bool __stdcall SvcControl::StopDependentServices()
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if ( EnumDependentServices( schService, SERVICE_ACTIVE,
		 lpDependencies, 0, &dwBytesNeeded, &dwCount ) )
	{
		 // If the Enum call succeeds, then there are no dependent
		 // services, so do nothing.
		 return true;
	}
	else
	{
		if ( GetLastError() != ERROR_MORE_DATA )
			return false; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );

		if ( !lpDependencies )
			return false;

		__try {
			// Enumerate the dependencies.
			if ( !EnumDependentServices( schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount ) )
			return false;

			for ( i = 0; i < dwCount; i++ )
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService( schSCManager,
				   ess.lpServiceName,
				   SERVICE_STOP | SERVICE_QUERY_STATUS );

				if ( !hDepService )
				   return false;

				__try {
					// Send a stop code.
					if ( !ControlService( hDepService,
							SERVICE_CONTROL_STOP,
							(LPSERVICE_STATUS) &ssp ) )
					return false;

					// Wait for the service to stop.
					while ( ssp.dwCurrentState != SERVICE_STOPPED )
					{
						Sleep( ssp.dwWaitHint );
						if ( !QueryServiceStatusEx(
								hDepService,
								SC_STATUS_PROCESS_INFO,
								(LPBYTE)&ssp,
								sizeof(SERVICE_STATUS_PROCESS),
								&dwBytesNeeded ) )
						return false;

						if ( ssp.dwCurrentState == SERVICE_STOPPED )
							break;

						if ( GetTickCount() - dwStartTime > dwTimeout )
							return false;
					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle( hDepService );
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree( GetProcessHeap(), 0, lpDependencies );
		}
	}
	return true;
}

void __stdcall SvcControl::CleanUp(SC_HANDLE schscmanager, SC_HANDLE schService)
{
	if (schscmanager != NULL)
		CloseServiceHandle(schscmanager);
	if (schService != NULL)
		CloseServiceHandle(schService);
}

bool __stdcall SvcControl::DoStopProcess()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return false;
	}
	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );          // clean the snapshot object
		return false;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if(tstrcmp(pe32.szExeFile, MTPLATFORM) == 0 || tstrcmp(pe32.szExeFile, MTPLATFORMHELPER) == 0)
		{
			hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID );
			if(hProcess == NULL)
				return false;
			TerminateProcess(
				 hProcess,
				 0
			);
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );
	CloseHandle( hProcessSnap );
	return true;
}

bool _stdcall SvcControl::DoStopProcess(const tchar * processname)
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return false;
	}
	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );          // clean the snapshot object
		return false;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if(tstrcmp(pe32.szExeFile, processname) == 0)
		{
			hProcess = OpenProcess( /*PROCESS_ALL_ACCESS*/PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID );
			std::tstring tmpwstr;
			if(hProcess == NULL)
			{
#ifdef LOG
				std::ostringstream tmpstream;
				tmpstream << "OpenProcess failed. Continue. ProcessID: " << pe32.th32ProcessID;
				std::string tmplogstr = tmpstream.str();
				logThread->AddLog(tmplogstr);
#endif
				continue;
			}
			if(this->ExtractProcessOwner(hProcess, tmpwstr))
			{
//				if(tmpwstr == TEXT("\\\\NT AUTHORITY\\SYSTEM")
//				{
//#ifdef LOG
//					logThread->AddLog("continue 1 process");
//#endif
//					continue;
//				}
			}
			TerminateProcess(
				 hProcess,
				 0
			);
#ifdef LOG
			logThread->AddLog("terminate 1 process");
#endif
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );
	CloseHandle( hProcessSnap );
	return true;
}

bool _stdcall SvcControl::DoStartSvc(const tchar *servicename)
{
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database.

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database
		SC_MANAGER_ALL_ACCESS);  // full access rights

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return false;
	}

    // Get a handle to the service.

	schService = OpenService(
		schSCManager,         // SCM database
		servicename,            // name of service
		SERVICE_ALL_ACCESS);  // full access

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return false;
	}

	// Check the status in case the service is not stopped.

    if (!QueryServiceStatusEx(
            schService,                     // handle to service
			SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // size needed if buffer is too small
    {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
		return false;
	}

    // Check if the service is already running. It would be possible
    // to stop the service here, but for simplicity this example just returns.

    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        printf("Cannot start the service because it is already running\n");
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
		return true;
    }

    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.

    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        // Check the status until the service is no longer stop pending.

        if (!QueryServiceStatusEx(
                schService,                     // handle to service
                SC_STATUS_PROCESS_INFO,         // information level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) )              // size needed if buffer is too small
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
			return false;
        }

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                printf("Timeout waiting for service to stop\n");
                CloseServiceHandle(schService);
                CloseServiceHandle(schSCManager);
				return false;
            }
        }
    }

    // Attempt to start the service.

    if (!StartService(
            schService,  // handle to service
            0,           // number of arguments
            NULL) )      // no arguments
    {
        printf("StartService failed (%d)\n", GetLastError());
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
		return false;
    }
	else printf("Service start pending...\n");

    // Check the status until the service is no longer start pending.

    if (!QueryServiceStatusEx(
            schService,                     // handle to service
            SC_STATUS_PROCESS_INFO,         // info level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // if buffer too small
    {
        printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return false;
    }

    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth the wait hint, but no less than 1 second and no
        // more than 10 seconds.

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        // Check the status again.

        if (!QueryServiceStatusEx(
            schService,             // handle to service
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // if buffer too small
        {
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			break;
        }

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                // No progress made within the wait hint.
                break;
            }
        }
    }

    // Determine whether the service is running.

    if (ssStatus.dwCurrentState == SERVICE_RUNNING)
    {
		printf("Service started successfully.\n");
    }
    else
    {
        printf("Service not started. \n");
        printf("  Current State: %d\n", ssStatus.dwCurrentState);
        printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
        printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
    }

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return true;
}

bool _stdcall SvcControl::DoStartProcess(const tchar* wccmd, const tchar* wcworkingdir)
{
	tchar wc[128] = TEXT("");
	std::tstrcpy(wc, wccmd);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

    // Start the child process.
	if( !CreateProcess( NULL,   // No module name (use command line)
		wc,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
		NULL,           // Use parent's environment block
		wcworkingdir,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
	)
	{
		printf( "CreateProcess failed (%d).\n", GetLastError() );
		return false;
	}

//	// Wait until child process exits.
//	WaitForSingleObject( pi.hProcess, INFINITE );
//
//	// Close process and thread handles.
//	CloseHandle( pi.hProcess );
//	CloseHandle( pi.hThread );
	return true;
}

bool _stdcall SvcControl::DoStartProcess(const tchar* wccmd, DWORD * elapsedtime, DWORD * returnvalue)
{
	tchar wc[128] = TEXT("");
	std::tstrcpy(wc, wccmd);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

    // Start the child process.
	if( CreateProcess( NULL,   // No module name (use command line)
		wc,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
	)
	{
		DWORD oldTime = GetTickCount();
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD newTime = GetTickCount();
		*elapsedtime = (newTime - oldTime) / 1000;
		while(GetExitCodeProcess(pi.hProcess, returnvalue) == STILL_ACTIVE)
		{
			Sleep(1000);
		}
		return true;
	}
	printf( "CreateProcess failed (%d).\n", GetLastError());
	return false;
	// // Wait until child process exits.
//	WaitForSingleObject( pi.hProcess, INFINITE );
//
//	// Close process and thread handles.
//	CloseHandle( pi.hProcess );
//	CloseHandle( pi.hThread );
}

bool _stdcall SvcControl::QuaryProcess(const tchar *processname)
{
	bool have = false;

	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return false;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if (tstrcmp(pe32.szExeFile, processname) == 0)
		{
			hProcess = OpenProcess( /*PROCESS_ALL_ACCESS*/PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
			std::tstring tmpwstr;
			if (hProcess == NULL)
			{
#ifdef LOG
				std::ostringstream tmpstream;
				tmpstream << "OpenProcess failed. Continue. ProcessID: " << pe32.th32ProcessID;
				std::string tmplogstr = tmpstream.str();
				logThread->AddLog(tmplogstr);
#endif
				continue;
			}
			if (this->ExtractProcessOwner(hProcess, tmpwstr))
			{
//				if(tmpwstr == TEXT("\\\\NT AUTHORITY\\SYSTEM")
//				{
//#ifdef LOG
//					logThread->AddLog("continue 1 process");
//#endif
//					continue;
//				}
				have = true;
			}
#ifdef LOG
			logThread->AddLog("terminate 1 process");
#endif
		}
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	return have;
}

bool _stdcall SvcControl::GetOsVersion(RTL_OSVERSIONINFOEXW* pk_OsVer)
{
    typedef LONG (WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);

    memset(pk_OsVer, 0, sizeof(RTL_OSVERSIONINFOEXW));
    pk_OsVer->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    HMODULE h_NtDll = GetModuleHandle(TEXT("ntdll.dll"));
    tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion)GetProcAddress(h_NtDll, "RtlGetVersion");

    if (!f_RtlGetVersion)
		return false; // This will never happen (all processes load ntdll.dll)

    LONG Status = f_RtlGetVersion(pk_OsVer);
    return Status == 0; // STATUS_SUCCESS;
}

//bool _stdcall SvcControl::GetWinVersion(OSVERSIONINFOEXW * pVersion)
//{
//	static int queryCount = 0;
//	static OSVERSIONINFOEXW osvi;
//	if(0 == queryCount){ // only query once;
//		if(GetOsVersion(&osvi)){
//			*pVersion = osvi;
//			++ queryCount;
//			return true;
//		}
//
//		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
//		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
//
//		if(! ::GetVersionExW(LPOSVERSIONINFOW(&osvi))){
////			LOG_ERROR("\tfail to get version:%d\n", ::GetLastError());
//			queryCount = 0;
//		}else{
//			++ queryCount;
//			*pVersion = osvi;
//			return true;
//		}
//		return false;
//	}else{
//		*pVersion = osvi;
//		return true;
//	}
//}

//bool _stdcall SvcControl::IsWindows8Later()
//{
//	static bool isWindows8 = false;
//	OSVERSIONINFOEXW osvi;
//	if(GetWinVersion(&osvi)){
//		if(6 < osvi.dwMajorVersion || 6 <= osvi.dwMajorVersion && 2 <= osvi.dwMinorVersion /*&& osvi.wProductType == VER_NT_WORKSTATION*/){ //It's windows 8 or windows server 2012
//			isWindows8 = true;
//		}else{
//			isWindows8 = false;
//		}
//	}
//	return isWindows8;
//}
//
//int GetProgramBits()
//{
//    return sizeof(int*) * 8;
//}

void SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
{
    if (NULL==lpSystemInfo)    return;
    typedef VOID (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
    LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress( GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");;
    if (NULL != fnGetNativeSystemInfo)
    {
        fnGetNativeSystemInfo(lpSystemInfo);
    }
    else
    {
        GetSystemInfo(lpSystemInfo);
    }
}
bool _stdcall SvcControl::Is64bit()
{
	SYSTEM_INFO si;
    SafeGetNativeSystemInfo(&si);
     if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
        si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
    {
		return true;
    }
	return false;
}
