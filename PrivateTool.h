#ifndef PRIVATETOOL_H
#define PRIVATETOOL_H
#include "PrivateDefine.h"
#include "LogServer.h"

#include <string>
#include <deque>

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
	//DMDO_DEFAULT DMDO_90 DMDO_180 DMDO_270
	FlipMode GetMonitorFlipMode();
	std::deque<std::string> SplitA(std::string str, char splitChar);
	std::deque<std::tstring> Split(std::tstring str, tchar splitChar);

	PrivateTool();
	~PrivateTool();
};

extern PrivateTool g_privateTool;
#endif