#ifndef PRIVATETOOL_H
#define PRIVATETOOL_H
#include "PrivateDefine.h"
#include "LogServer.h"

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