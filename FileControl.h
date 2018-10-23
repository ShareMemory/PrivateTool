#ifndef FILE_CONTROL_H
#define FILE_CONTROL_H
#include "PrivateDefine.h"

enum FileType {
	FT_UNKNOWN = 0,
	FT_FILE = 1,
	FT_DIR = 2
};

#ifdef __cplusplus
extern "C" {
#endif 
enum FileType CheckFileType(tchar *path);
int FindFirstChildFile(tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType);
int FindNextChildFile(tchar *dirPath, tchar findFilePath[MAX_PATH], tchar **fileExtendType, int sz_fileExtendType);
int CreateFolder(const tchar *dirPath);
int CreateEmptyFile(const tchar *fileName);
int CvtToWinTypePath(tchar *path, int nSizeCount);
int DeleteFileEx(tchar *path);
int GetFileName(tchar *filePath, int sz_filePathSizeCount, tchar *fileName, int sz_fileNameSizeCount);
#ifdef __cplusplus
}
#endif
#endif