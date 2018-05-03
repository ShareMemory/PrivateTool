#include <Windows.h>
#include "FileControl.h"
#include <ShlObj.h>
#include <stdio.h>

#pragma comment(lib, "shell32.lib")

HANDLE g_hff = NULL;
tchar g_dirPath[MAX_PATH] = TEXT("");

void CleanFileFind();

enum FileType CheckFileType(tchar *path)
{
	enum FileType ft = FT_UNKNOWN;
	DWORD dw = 0;
	if (path == NULL) goto clean;
	dw = GetFileAttributes(path);
	if (dw == INVALID_FILE_ATTRIBUTES) goto clean;
	if (dw & FILE_ATTRIBUTE_DIRECTORY) ft = FT_DIR;
	else ft = FT_FILE;
clean:
	return ft;
}

int FindFirstChildFile(tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType)
{
	CleanFileFind();
	if (dirPath == NULL || findFilePath == NULL) return 0;
	tstrcpy(g_dirPath, dirPath);
	tstrcat(g_dirPath, TEXT("\\*"));
	return FindNextChildFile(dirPath, findFilePath, fileExtendType, sz_fileExtendType);
}

int FindNextChildFile(tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType)
{
	if (dirPath == NULL || findFilePath == NULL) goto clean;
	memset(findFilePath, 0, MAX_PATH * sizeof(tchar));
	WIN32_FIND_DATA fd;
	if (g_hff == NULL || g_hff == INVALID_HANDLE_VALUE)
	{
		g_hff = FindFirstFile(g_dirPath, &fd);
		if (g_hff == INVALID_HANDLE_VALUE) goto clean;
	}
	else
	{
		if (!FindNextFile(g_hff, &fd))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				;
			}
			goto clean;
		}
	}
	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		return FindNextChildFile(dirPath, findFilePath, fileExtendType, sz_fileExtendType);
	}
	else
	{
		int sz = (int)tstrlen(fd.cFileName);
		int index = -1;
		for (int i = sz - 1; i >= 0; i--)
		{
			if (fd.cFileName[i] == TEXT('.'))
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			tchar extendName[MAX_PATH] = TEXT("");
			tstrncpy(extendName, fd.cFileName + index + 1, sz - index - 1);
			for (int i = 0; i < sz_fileExtendType; i++)
			{
				if (tstrcmp(t_strlwr(extendName), fileExtendType[i]) == 0)
				{
					tstrcpy(findFilePath, dirPath);
					tstrcat(findFilePath, TEXT("\\"));
					tstrcat(findFilePath, fd.cFileName);
					return 1;
				}
			}
			return FindNextChildFile(dirPath, findFilePath, fileExtendType, sz_fileExtendType);
		}
		else
		{
			return FindNextChildFile(dirPath, findFilePath, fileExtendType, sz_fileExtendType);
		}
	}
clean:
	CleanFileFind();
	return 0;
}

void CleanFileFind()
{
	if (g_hff != NULL && g_hff != INVALID_HANDLE_VALUE)
	{
		FindClose(g_hff);
		g_hff = NULL;
	}
	memset(g_dirPath, 0, MAX_PATH * sizeof(tchar));
}

int CreateFolder(const tchar *dirPath)
{
	tchar dirPathCopy[MAX_PATH] = TEXT("");
	tstrcpy_s(dirPathCopy, MAX_PATH, dirPath);
	for (int i = MAX_PATH - 1; i >= 0; i--)
	{
		if (dirPathCopy[i] == TEXT('/'))
		{
			dirPathCopy[i] = TEXT('\\');
		}
	}
	int re = SHCreateDirectoryEx(NULL, dirPathCopy, NULL);
	if (re == ERROR_BAD_PATHNAME)
	{
		tchar exeFolderPathName[MAX_PATH] = TEXT("");
		GetModuleFileName(NULL, exeFolderPathName, MAX_PATH);
		for (int i = MAX_PATH - 1; i >= 0; i--)
		{
			if (exeFolderPathName[i] == TEXT('\\'))
			{
				break;
			}
			exeFolderPathName[i] = 0;
		}
		tstrcat_s(exeFolderPathName, MAX_PATH, dirPathCopy);
		re = SHCreateDirectoryEx(NULL, exeFolderPathName, NULL);
	}
	return re;
}

int CreateEmptyFile(const tchar *fileName)
{
	FILE *f = tfopen(fileName, TEXT("w"));
	fclose(f);
}

int CvtToWinTypePath(tchar *path, int nSize)
{
	for (int i = 0; i < nSize; i++)
	{
		if (path[i] == TEXT('/'))
		{
			path[i] = TEXT('\\');
		}
	}
	return 0;
}