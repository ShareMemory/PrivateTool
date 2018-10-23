#ifndef FILECONTROLNEW_H
#define FILECONTROLNEW_H

#include "PrivateDefine.h"
#include <string>
#include <deque>

enum FileType {
	FT_UNKNOWN = 0,
	FT_FILE = 1,
	FT_DIR = 2
};

class FileControl
{
public:
	FileControl();
	~FileControl();

	enum FileType CheckFileType(const tchar *path);
	int FindFirstChildFile(const tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType);
	int FindNextChildFile(const tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType);
	int CreateFolder(const tchar *dirPath);
	int CreateEmptyFile(const tchar *fileName);
	int CvtToWinTypePath(tchar *path, int nSizeCount);
	int DeleteFileEx(tchar *path);
	int GetFileName(tchar *filePath, int sz_filePathSizeCount, tchar *fileName, int sz_fileNameSizeCount, bool withExtend = true);
	int ReadBytesFromFile(const tchar *filePath, char *pBytes, int sz_BytesSize, int *readedSize = NULL);
	int WriteBytesToFile(const tchar *filePath, const char *pBytes, int sz_BytesSize);
	void ReadLinesFromFile(const tchar *filePath, std::deque<std::tstring> &dLines, tchar lineEnd = TEXT('\n'));
	void WriteLinesToFile(const tchar *filePath, std::deque<std::tstring> dLines, std::deque<tchar> dLineEnd = std::deque<tchar>({ TEXT('\r'), TEXT('\n') }));
	void CleanFileFind();
private:
	void * m_hff = NULL;
	tchar m_dirPath[MAX_PATH] = TEXT("");
};

extern FileControl g_constFileControl;
#endif
