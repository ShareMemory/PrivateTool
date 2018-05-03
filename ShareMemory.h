#ifndef SHARE_MEMORY_H
#define SHARE_MEMORY_H
#include "PrivateDefine.h"

#include <windows.h>
#include "PrivateTool.h"

enum ShareMemoryMode
{
	SMM_Unknown = 0,
	Creator = 1,
	User = 2
};

class ShareMemory
{
private:
	tchar *m_shareMemoryName = nullptr;
	tchar *m_mutexName = nullptr;
	HANDLE m_hMutex = nullptr;
	HANDLE m_hMapFile = nullptr;
	char *m_pBuf = nullptr;

	unsigned int m_bufSize = 0;

public:
	ShareMemoryMode m_mode = ShareMemoryMode::SMM_Unknown;

	ShareMemory(const tchar *shareMemoryName, const tchar *mutexName, unsigned int bufSize);
	~ShareMemory();
	int WriteData(char *pBuf);
	int WriteData(char *pBuf, long long start, long long size);
	int WriteData(char **pBuf, long long *start, long long *size, int bufPartCount);
	int ReadData(char *pBuf);
	int ReadData(char *pBuf, long long start, long long size);
	int GetMutex2();
	int ReleaseMutex2();
};
#endif