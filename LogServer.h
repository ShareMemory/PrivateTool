// #ifndef LOG_SERVER_H
// #define LOG_SERVER_H
// #include "PrivateDefine.h"

// #include <time.h>
// #include <string>
// #include <deque>
// #include <mutex>

// class LogServer
// {
// private:
// 	int m_logFPS = -1;
// 	clock_t m_logTick = 0;
// 	std::deque<std::tstring> m_dLogCache;
// 	std::mutex m_mtxLogCache;
// 	bool *m_exit = nullptr;
// 	volatile bool m_closing = false;

// 	void ShowLogThread();
// public:
// 	LogServer();
// 	LogServer(int logFPS);
// 	~LogServer();
// 	void ShowLogImmediate(tchar *tstr);
// 	void ShowLog(tchar *tstr);
// 	void ShowLogImmediate(std::tstring str);
// 	void ShowLog(std::tstring str);
// };

// extern LogServer g_logServer;
// #endif