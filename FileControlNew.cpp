#include "FileControlNew.h"
#include <Windows.h>
#include <ShlObj.h>
#include <stdio.h>
#include <fstream>

#ifndef __BCPLUSPLUS__
#pragma comment(lib, "shell32.lib")
#else
#pragma comment(lib, "shell32.a")
#endif

FileControl::FileControl()
{

}

FileControl::~FileControl()
{
	CleanFileFind();
}

enum FileType FileControl::CheckFileType(const tchar *path)
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

int FileControl::FindFirstChildFile(tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType)
{
	CleanFileFind();
	if (dirPath == NULL || findFilePath == NULL) return 0;
	tstrcpy_s(m_dirPath, sizeof(m_dirPath), dirPath);
	tstrcat_s(m_dirPath, sizeof(m_dirPath), TEXT("\\*"));
	return FindNextChildFile(dirPath, findFilePath, fileExtendType, sz_fileExtendType);
}

int FileControl::FindNextChildFile(tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType)
{
	if (dirPath == NULL || findFilePath == NULL) goto clean;
	memset(findFilePath, 0, MAX_PATH * sizeof(tchar));
	WIN32_FIND_DATA fd;
	if (m_hff == NULL || m_hff == INVALID_HANDLE_VALUE)
	{
		m_hff = FindFirstFile(m_dirPath, &fd);
		if (m_hff == INVALID_HANDLE_VALUE) goto clean;
	}
	else
	{
		if (!FindNextFile(m_hff, &fd))
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
			tstrncpy_s(extendName, sizeof(extendName), fd.cFileName + index + 1, sz - index - 1);

			for (int i = 0; i < sz_fileExtendType; i++)
			{
#ifndef __BCPLUSPLUS__
				_tstrlwr_s(extendName, MAX_PATH);
#else
				if (sizeof(tchar) > 1)
				{
					_wcslwr(extendName);
				}
				else
				{
					_strlwr(extendName);
				}
#endif

				if (tstrcmp(extendName, fileExtendType[i]) == 0)
				{
					tstrcpy_s(findFilePath, MAX_PATH * sizeof(tchar), dirPath);
					tstrcat_s(findFilePath, MAX_PATH * sizeof(tchar), TEXT("\\"));
					tstrcat_s(findFilePath, MAX_PATH * sizeof(tchar), fd.cFileName);
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

void FileControl::CleanFileFind()
{
	if (m_hff != NULL && m_hff != INVALID_HANDLE_VALUE)
	{
		FindClose(m_hff);
		m_hff = NULL;
	}
	memset(m_dirPath, 0, MAX_PATH * sizeof(tchar));
}

int FileControl::CreateFolder(const tchar *dirPath)
{
	tchar dirPathCopy[MAX_PATH] = TEXT("");
	tstrcpy_s(dirPathCopy, sizeof(dirPathCopy), dirPath);
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
		tstrcat_s(exeFolderPathName, sizeof(exeFolderPathName), dirPathCopy);
		re = SHCreateDirectoryEx(NULL, exeFolderPathName, NULL);
	}
	return re;
}

int FileControl::CreateEmptyFile(const tchar *fileName)
{
	FILE *f = NULL;
	tfopen_s(&f, fileName, TEXT("w"));
	fclose(f);
}

int FileControl::CvtToWinTypePath(tchar *path, int nSizeCount)
{
	for (int i = 0; i < nSizeCount; i++)
	{
		if (path[i] == TEXT('/'))
		{
			path[i] = TEXT('\\');
		}
	}
	return 0;
}

int FileControl::DeleteFileEx(tchar *path)
{
	return DeleteFile(path);
}

int FileControl::GetFileName(tchar *filePath, int sz_filePathSizeCount, tchar *fileName, int sz_fileNameSizeCount, bool withExtend)
{
	CvtToWinTypePath(filePath, sz_filePathSizeCount);
	int startIndex = 0;
	int finded = 0;
	for (int i = sz_fileNameSizeCount - 1; i >= 0; i--)
	{
		if (filePath[i] == TEXT('\\'))
		{
			finded = 1;
			startIndex = i + 1;
			break;
		}
	}
	if (finded)
	{
		tstrcpy_s(fileName, sz_filePathSizeCount * sizeof(tchar), filePath + startIndex);
	}
	else
	{
		tstrcpy_s(fileName, sz_filePathSizeCount * sizeof(tchar), filePath);
	}
	if (!withExtend)
	{
		int count = tstrlen(fileName);
		int endIndex = 0;
		finded = 0;
		for (int i = count - 1; i >= 0; i--)
		{
			if (fileName[i] == TEXT('.'))
			{
				finded = 1;
				endIndex = i;
				break;
			}
		}
		if (finded)
		{
			memset(fileName + endIndex, 0, sizeof(tchar) * (sz_filePathSizeCount - endIndex));
		}
	}
	return 0;
}

int FileControl::WriteBytesToFile(const tchar *filePath, const char *pBytes, int sz_BytesSize)
{
	int re = 0;
	FILE *file = NULL;
	tfopen_s(&file, filePath, "w+b");
	if (file)
	{
		fseek(file, 0, SEEK_SET);
		fwrite(pBytes, 1, sz_BytesSize, file);
		fclose(file);
	}
	else
	{
		re = 1;
	}
	return re;
}

int FileControl::ReadBytesFromFile(const tchar *filePath, char *pBytes, int sz_BytesSize, int *readedSize)
{
	int re = 0;
	int readedSize2 = 0;
	FILE *file = NULL;
	tfopen_s(&file, filePath, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		int length = ftell(file);
		if (length >= sz_BytesSize)
		{
			readedSize2 = sz_BytesSize;
		}
		else
		{
			readedSize2 = length;
		}
		fseek(file, 0, SEEK_SET);
		fread((void*)pBytes, 1, readedSize2, file);
		fclose(file);
	}
	else
	{
		re = 1;
	}
	if(readedSize != NULL)
		*readedSize = readedSize2;
	return re;
}

using namespace std;

void FileControl::ReadLinesFromFile(const tchar *filePath, std::deque<std::tstring> &dLines, tchar lineEnd)
{
	dLines.clear();
	ifstream stream(filePath);
	while (true)
	{
		tstring line;
		if (getline(stream, line, lineEnd))
		{
			dLines.push_back(line);
		}
		else
		{
			break;
		}
	}
}

void FileControl::WriteLinesToFile(const tchar * filePath, std::deque<std::tstring> dLines, std::deque<tchar> dLineEnd)
{
	char *pBytes = NULL;
	int sz_BytesSize = 0;
	int sz_tcharSize = sizeof(tchar);
	for (auto str : dLines)
	{
		sz_BytesSize += str.size();
		sz_BytesSize += sz_tcharSize;
	}
	pBytes = (char*)malloc(sz_BytesSize);
	memset(pBytes, 0, sz_BytesSize);
	char *p = pBytes;
	for (auto str : dLines)
	{
		int strSize = str.size();
		memcpy(p, str.c_str(), strSize);
		p += strSize;

		for (auto key : dLineEnd)
		{
			memcpy(p, tstring(sz_tcharSize, key).c_str(), sz_tcharSize);
			p += sz_tcharSize;
		}
	}

	WriteBytesToFile(filePath, pBytes, sz_BytesSize);
	free(pBytes);
	pBytes = NULL;
}
