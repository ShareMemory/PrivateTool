#include "LogServer.h"
#include <thread>

using namespace std;

LogServer g_logServer;

LogServer::LogServer()
{

}

LogServer::LogServer(int logFPS)
{
	m_logFPS = logFPS;
	m_exit = (bool*)malloc(sizeof(bool));
	*m_exit = false;
	thread trd(&LogServer::ShowLogThread, this);
	trd.detach();
}

LogServer::~LogServer()
{
	m_closing = true;
	if (m_logFPS != -1)
	{
		if (m_exit != nullptr)
		{
			*m_exit = true;
			while (*m_exit)
			{
				;
			}
			free(m_exit);
			m_exit = nullptr;
		}
	}
}

void LogServer::ShowLog(tchar *tstr)
{
	if (m_closing)
		return;
	ShowLog(tstring(tstr));
}

void LogServer::ShowLog(tstring str)
{
	if (m_closing)
		return;
	str = str + TEXT("\r\n");
	if (m_logFPS == -1)
	{
		tprintf(str.c_str());
	}
	else
	{
		m_mtxLogCache.lock();
		m_dLogCache.push_back(str);
		m_mtxLogCache.unlock();
	}
}

void LogServer::ShowLogImmediate(tchar *tstr)
{
	ShowLogImmediate(tstring(tstr));
}

void LogServer::ShowLogImmediate(tstring str)
{
	str = str + TEXT("\n");
	tprintf(str.c_str());
}

void LogServer::ShowLogThread()
{
	bool readyToExit = false;
	bool showExitMsg = false;
	while (!readyToExit)
	{
		m_mtxLogCache.lock();
		if (*m_exit == true)
		{
			if (!showExitMsg)
			{
				tprintf(TEXT("\n---Exit_EVENT_RECV---\n"));
				showExitMsg = true;
			}
			if (m_dLogCache.size() == 0)
			{
				readyToExit = true;
			}
		}
		if (m_logFPS != 0 && clock() - m_logTick > m_logFPS && m_dLogCache.size() > 0)
		{
			tstring str = m_dLogCache[0];
			tprintf(str.c_str());
			m_dLogCache.pop_front();
			m_logTick = clock();
		}
		m_mtxLogCache.unlock();
	}
	*m_exit = false;
}