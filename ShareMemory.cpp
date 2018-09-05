#include "ShareMemory.h"
#include "string"
#include "AccCtrl.h"
#include "Aclapi.h"

ShareMemory::ShareMemory(const tchar *shareMemoryName, const tchar *mutexName, unsigned int bufSize, ShareMemoryMode shareMemoryMode)
{
	m_shareMemoryName = (tchar*)malloc((tstrlen(shareMemoryName) + 1) * 2);
	memset(m_shareMemoryName, 0, (tstrlen(shareMemoryName) + 1) * 2);
	memcpy(m_shareMemoryName, shareMemoryName, (tstrlen(shareMemoryName) + 1) * 2);
	m_mutexName = (tchar*)malloc((tstrlen(mutexName) + 1) * 2);
	memset(m_mutexName, 0, (tstrlen(mutexName) + 1) * 2);
	memcpy(m_mutexName, mutexName, (tstrlen(mutexName) + 1) * 2);
	m_bufSize = bufSize;

    if(shareMemoryMode == ShareMemoryMode::SMM_Unknown)
    {
        m_mode = ShareMemoryMode::Any;
    }
    else
    {
        m_mode = shareMemoryMode;
    }

	switch (shareMemoryMode)
	{
	default:
	case ShareMemoryMode::Any:
		m_hMapFile = OpenFileMapping(
			FILE_MAP_READ,   // read/write access
			FALSE,                 // do not inherit the name
			m_shareMemoryName);               // name of mapping object
		break;
	case ShareMemoryMode::Creator:
		while (true)
		{
			m_hMapFile = OpenFileMapping(
				FILE_MAP_READ,   // read/write access
				FALSE,                 // do not inherit the name
				m_shareMemoryName);               // name of mapping object
			if (m_hMapFile == NULL)
			{
				break;
			}
			else
			{
				CloseHandle(m_hMapFile);
			}
			Sleep(100);
		}
		break;
	case ShareMemoryMode::User:
		while (true)
		{
			m_hMapFile = OpenFileMapping(
				FILE_MAP_READ,   // read/write access
				FALSE,                 // do not inherit the name
				m_shareMemoryName);               // name of mapping object
			if (m_hMapFile != NULL)
			{
				break;
			}
			Sleep(100);
		}
		break;
	case ShareMemoryMode::UserEx:
		while (true)
		{
			m_hMapFile = OpenFileMapping(
				FILE_MAP_READ | FILE_MAP_WRITE,   // read/write access
				FALSE,                 // do not inherit the name
				m_shareMemoryName);               // name of mapping object
			if (m_hMapFile != NULL)
			{
				break;
			}
			Sleep(100);
		}
		break;
	}
	if (m_hMapFile != NULL)
	{
		m_hMutex = OpenMutex(SYNCHRONIZE, FALSE, m_mutexName);
		if (m_hMutex == NULL)
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("OpenMutex Error: ") + std::to_tstring(dwError)).c_str());
		}
        if(shareMemoryMode == ShareMemoryMode::Any)
        {
		    m_mode = ShareMemoryMode::User;
        }
        if(m_mode == ShareMemoryMode::UserEx)
        {
            m_pBuf = (char*)MapViewOfFile(m_hMapFile,   // handle to map object
                FILE_MAP_READ | FILE_MAP_WRITE, // read/write permission
                0,
                0,
                m_bufSize);
        }
        else
        {
            m_pBuf = (char*)MapViewOfFile(m_hMapFile,   // handle to map object
                FILE_MAP_READ, // read/write permission
                0,
                0,
                m_bufSize);
        }
		if (m_pBuf == NULL)
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("MapViewOfFile Error: ") + std::to_tstring(dwError)).c_str());
		}
	}
	else
	{
		SECURITY_ATTRIBUTES sa = {};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = FALSE;

		SECURITY_DESCRIPTOR sd = {};
		if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("InitializeSecurityDescriptor Error: ") + std::to_tstring(dwError)).c_str());
		}

		PACL pACL = NULL;
		EXPLICIT_ACCESS ea[2];
		ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

		PSID pEveryoneSID = NULL;
		SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
		if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
			0, 0, 0, 0, 0, 0, 0, &pEveryoneSID))
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("AllocateAndInitializeSid Error: ") + std::to_tstring(dwError)).c_str());
		}
		ea[0].grfAccessPermissions = FILE_MAP_ALL_ACCESS | SYNCHRONIZE;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance = NO_INHERITANCE;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[0].Trustee.ptstrName = (LPTSTR)pEveryoneSID;

		PSID pCreatorSID = NULL;
		SID_IDENTIFIER_AUTHORITY SIDAuthCreator = SECURITY_CREATOR_SID_AUTHORITY;
		if (!AllocateAndInitializeSid(&SIDAuthCreator, 1, SECURITY_CREATOR_OWNER_RID,
			0, 0, 0, 0, 0, 0, 0, &pCreatorSID))
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("AllocateAndInitializeSid Error: ") + std::to_tstring(dwError)).c_str());
		}
		ea[1].grfAccessPermissions = FILE_ALL_ACCESS | SYNCHRONIZE;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance = NO_INHERITANCE;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[1].Trustee.ptstrName = (LPTSTR)pCreatorSID;

		DWORD dwResult = SetEntriesInAcl(2, ea, NULL, &pACL);
		if (dwResult != ERROR_SUCCESS)
		{
			OutputDebugString((TEXT("SetEntriesInAcl Error: ") + std::to_tstring(dwResult)).c_str());
		}

		if (!SetSecurityDescriptorDacl(&sd, TRUE, pACL, FALSE))
		{
			OutputDebugString((TEXT("SetSecurityDescriptorDacl Error: ") + std::to_tstring(dwResult)).c_str());
		}

		sa.lpSecurityDescriptor = &sd;
		
		m_hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			&sa,                   // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			m_bufSize,                // maximum object size (low-order DWORD)
			m_shareMemoryName);
		if (m_hMapFile == NULL)
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("CreateFileMapping Error: ") + std::to_tstring(dwError)).c_str());
		}
		m_hMutex = CreateMutex(
			&sa,
			FALSE,
			m_mutexName
		);
		if (m_hMutex == NULL)
		{
			DWORD dwError = GetLastError();
			OutputDebugString((TEXT("CreateMutex Error: ") + std::to_tstring(dwError)).c_str());
		}
        if(m_mode == ShareMemoryMode::Any)
        {
		    m_mode = ShareMemoryMode::Creator;
        }
		m_pBuf = (char*)MapViewOfFile(m_hMapFile,   // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0,
			0,
			m_bufSize);
		//LocalFree(&sd);
		//if (pACL != NULL)
		//{
		//	LocalFree(pACL);
		//}
		//if (pEveryoneSID != NULL)
		//{
		//	FreeSid(pEveryoneSID);
		//}
		//if (pCreatorSID != NULL)
		//{
		//	FreeSid(pCreatorSID);
		//}
	}
}

ShareMemory::~ShareMemory()
{
	free(m_shareMemoryName);
	m_shareMemoryName = nullptr;
	free(m_mutexName);
	m_mutexName = nullptr;
	bool re = ReleaseMutex(m_hMutex);
	bool re2 = CloseHandle(m_hMapFile);
	bool re3 = CloseHandle(m_hMutex);
	//printf("%f, %f, %f\r\n", re, re2, re3);
}

int ShareMemory::WriteData(char *pBuf)
{
	return WriteData(pBuf, 0, m_bufSize);
}

int ShareMemory::WriteData(char *pBuf, long long start, long long size)
{
	return WriteData(&pBuf, &start, &size, 1);
}

int ShareMemory::WriteData(char **pBuf, long long *start, long long *size, int bufPartCount)
{
	if (m_hMapFile == NULL || m_hMutex == NULL)
	{
		return 4;
	}
	if (m_mode == ShareMemoryMode::User)
	{
		return 3;
	}
	bool excecution = false;
	while (!excecution)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hMutex, 100);
		DWORD dwLastError = 0;
		switch (dwWaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			try {
				for (int i = 0; i < bufPartCount; i++)
				{
					CopyMemory((PVOID)(m_pBuf + start[i]), pBuf[i], size[i]);
				}
				excecution = true;
			}
			catch (...)
			{
				throw "mutex is using.";
			}
			if (!ReleaseMutex(m_hMutex))
			{
				// Handle error.
			}
			break;

			// The thread got ownership of an abandoned mutex
			// The database is in an indeterminate state
		case WAIT_FAILED:
			dwLastError = GetLastError();
			return 1;
			break;
		case WAIT_ABANDONED:
			return 2;
			break;
		}
	}
	return 0;
}

int ShareMemory::ReadData(char *pBuf)
{
	return ReadData(pBuf, 0, m_bufSize);
}

int ShareMemory::ReadData(char *pBuf, long long start, long long size)
{
	if (m_hMapFile == NULL || m_hMutex == NULL)
	{
		return 4;
	}
	bool excecution = false;
	while (!excecution)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hMutex, 5);
		DWORD dwLastError = 0;
		switch (dwWaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			try {
				CopyMemory(pBuf, m_pBuf + start, size);
				excecution = true;
			}
			catch (...)
			{
				throw "mutex is using.";
			}
			if (!ReleaseMutex(m_hMutex))
			{
				// Handle error.
			}
			break;

			// The thread got ownership of an abandoned mutex
			// The database is in an indeterminate state
		case WAIT_FAILED:
			dwLastError = GetLastError();
			return 1;
			break;
		case WAIT_ABANDONED:
			return 2;
			break;
		}
	}
	return 0;
}

int ShareMemory::GetMutex2()
{
	if (m_hMapFile == NULL || m_hMutex == NULL)
	{
		return 4;
	}
	bool excecution = false;
	while (!excecution)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hMutex, 5);
		DWORD dwLastError = 0;
		switch (dwWaitResult)
		{
			// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			break;

			// The thread got ownership of an abandoned mutex
			// The database is in an indeterminate state
		case WAIT_FAILED:
			dwLastError = GetLastError();
			return 1;
			break;
		case WAIT_ABANDONED:
			return 2;
			break;
		}
	}
	return 0;
}

int ShareMemory::ReleaseMutex2()
{
	if (m_hMapFile == NULL || m_hMutex == NULL)
	{
		return 4;
	}
	if (!ReleaseMutex(m_hMutex))
	{
		return 1;
	}
	return 0;
}