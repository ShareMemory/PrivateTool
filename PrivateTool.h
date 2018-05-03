#ifndef PRIVATETOOL_H
#define PRIVATETOOL_H
#include "PrivateDefine.h"

#ifdef UNICODE
#define tchar wchar_t
#define tstring wstring
#define tstrcpy wcscpy
#define tstrcpy_s wcscpy_s
#define tstrcat wcscat
#define tstrcat_s wcscat_s
#define tstrlen wcslen
#define to_tstring to_wstring
#define tstrcmp wcscmp
#define __TEXT(quote) L##quote      // r_winnt
#else
#define tchar char
#define tstring string
#define tstrcpy strcpy
#define tstrcpy_s strcpy_s
#define tstrcat strcat
#define tstrcat_s strcat_s
#define tstrlen strlen
#define to_tstring to_string
#define tstrcmp strcmp
#define __TEXT(quote) quote         // r_winnt
#endif
#define TEXT(quote) __TEXT(quote)   // r_winnt

#include <string>

enum FileType {
	FT_UNKNOWN = 0,
	FT_FILE = 1,
	FT_DIR = 2
};

enum FlipMode {
	FM_UNKNOWN = 0,
	FM_DEFAULT = 1,
	FM_90 = 2,
	FM_180 = 3,
	FM_270 = 4
};

class PrivateTool
{
public:
	std::tstring m_tsExeFilePath;
	std::tstring m_tsExeFolderPath;

	std::tstring GetExeFilePath();
    std::tstring GetExeFolderPath();
	FileType CheckFileType(const tchar *path);
	int CreateFolder(const tchar *dirPath);
	//DMDO_DEFAULT DMDO_90 DMDO_180 DMDO_270
	FlipMode GetMonitorFlipMode();

	PrivateTool();
	~PrivateTool();
};

extern PrivateTool g_privateTool;
#endif