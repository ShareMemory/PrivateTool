#include "LogServer.h"

LogServer g_logServer;

LogServer::LogServer()
{

}

LogServer::LogServer(int logFPS)
{
	m_logFPS = logFPS;
}

void LogServer::ShowLog(TCHAR *tstr)
{
	ShowLog(std::tstring(tstr));
}

void LogServer::ShowLog(std::tstring str)
{
	if (m_logFPS != 0 && clock() - m_logTick > m_logFPS)
	{
		tprintf(str.c_str());
		m_logTick = clock();
	}
	else
	{
		tprintf(str.c_str());
	}
}