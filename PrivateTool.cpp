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
    tstr = tstr.substr(0, index);
    return tstr;
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

deque<string> PrivateTool::SplitA(string str, char splitChar)
{
	deque<string> re;
	while (str.size() != 0)
	{
		string split;
		for (size_t i = 0; i < str.size(); i++)
		{
			if (str.at(i) == splitChar)
			{
				str = str.substr(i + 1, str.size() - (i + 1));
				goto Finded;
			}
			else
			{
				split = split + str.at(i);
			}
		}
		str = "";
	Finded:
		re.push_back(split);
	}
	return re;
}

deque<tstring> PrivateTool::Split(tstring str, tchar splitChar)
{
	deque<tstring> re;
	while (str.size() != 0)
	{
		tstring split;
		bool haveSplitChar = false;
		for (size_t i = 0; i < str.size(); i++)
		{
			if (str.at(i) == splitChar)
			{
				haveSplitChar = true;
				str = str.substr(i + 1, tstring::npos);
				break;
			}
			else
			{
				split = split + str.at(i);
			}
		}
		if (haveSplitChar)
		{
			re.push_back(split);
		}
		else
		{
			re.push_back(str);
			break;
		}
	}
	return re;
}

tstring PrivateTool::GetNowTimeString()
{
	tstring re;

	time_t rawtime;
	tm timeinfo[MAX_COUNT];
	tchar buffer[MAX_SIZE];

	time(&rawtime);
	localtime_s(timeinfo, &rawtime);

	tstrftime(buffer, 80, TEXT("%Y%m%d%H%M%S"), timeinfo);
	re = buffer;

	return re;
}

tstring PrivateTool::ReplaceRegularMacro(tstring str, tstring macro, tstring replaced)
{
	tstring re;
	int count = str.size();
	bool finded = false;
	for (int i = 0; i < count - 1; i++)
	{
		if (str[i] == TEXT('#') && str[i + 1] == TEXT('('))
		{
			for (int j = i; j < count; j++)
			{
				if (str[j] == TEXT(')'))
				{
					re = str.substr(0, i) + replaced + str.substr(j + 1, str.npos);
					finded = true;
				}
			}
		}
	}
	if (finded)
		return ReplaceRegularMacro(re, macro, replaced);
	else
		return str;
}

int PrivateTool::FindMinNumberIndex(unsigned __int64 * array, size_t count)
{
	int ret = -1;
	unsigned __int64 min = -1;
	for (int i = 0; i < count; i++)
	{
		if (array[i] <= min)
		{
			ret = i;
			min = array[i];
		}
	}
	return ret;
}

int PrivateTool::FindMaxNumberIndex(unsigned __int64 * array, size_t count)
{
	int ret = -1;
	unsigned __int64 max = 0;
	for (int i = 0; i < count; i++)
	{
		if (array[i] >= max)
		{
			ret = i;
			max = array[i];
		}
	}
	return ret;
}
