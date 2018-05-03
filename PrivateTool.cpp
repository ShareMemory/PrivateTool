#include "PrivateTool.h"
#include <Windows.h>
#include <ShlObj.h>

PrivateTool g_privateTool;

using namespace std;

PrivateTool::PrivateTool()
{
	m_tsExeFilePath = GetExeFilePath();
	m_tsExeFolderPath = GetExeFolderPath();
}

PrivateTool::~PrivateTool()
{
}

tstring PrivateTool::GetExeFilePath()
{
	tchar path[MAX_PATH] = TEXT("");
	GetModuleFileName(NULL, path, MAX_PATH);
    return path;
}

tstring PrivateTool::GetExeFolderPath()
{
	tstring tstr = GetExeFilePath();
	int index = (int)tstr.rfind(TEXT("\\"));
    tstr.substr(0, index);
    return tstr;
}

FileType PrivateTool::CheckFileType(const tchar *path)
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

int PrivateTool::CreateFolder(const tchar *dirPath)
{
	tchar dirPathCopy[MAX_PATH] = TEXT("");
	tstrcpy_s(dirPathCopy, MAX_PATH, dirPath);
	for (int i = MAX_PATH - 1; i >= 0; i--)
	{
		if (dirPathCopy[i] == '/')
		{
			dirPathCopy[i] = '\\';
		}
	}
	int re = SHCreateDirectoryEx(NULL, dirPathCopy, NULL);
	if (re == ERROR_BAD_PATHNAME)
	{
		tchar exeFolderPathName[MAX_PATH] = TEXT("");
		GetModuleFileName(NULL, exeFolderPathName, MAX_PATH);
		for (int i = MAX_PATH - 1; i >= 0; i--)
		{
			if (exeFolderPathName[i] == '\\')
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

FlipMode PrivateTool::GetMonitorFlipMode()
{
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);

	if (0 != EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
	{
		return (FlipMode)(dm.dmDisplayOrientation + 1);
	}
	else
	{
		return FlipMode::FM_UNKNOWN;
	}
}