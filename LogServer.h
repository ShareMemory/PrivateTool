#ifndef LOG_SERVER_H
#define LOG_SERVER_H
#include "PrivateDefine.h"

#include <time.h>
#include <string>
#include <tchar.h>
#ifdef UNICODE
#define tstring wstring
#define tprintf wprintf
#define stprintf swprintf
#define to_tstring to_wstring
#define tstrcpy_s wcscpy_s
#else
#define tstring string
#define tprintf printf
#define stprintf sprintf
#define stprintf sprintf
#define to_tstring to_string
#define tstrcpy_s strcpy_s
#endif

class LogServer
{
private:
	//debug
	int m_logFPS = 0;
	clock_t m_logTick = 0;
public:
	LogServer();
	LogServer(int logFPS);
	void ShowLog(TCHAR *tstr);
	void ShowLog(std::tstring str);
};

extern LogServer g_logServer;
#endif