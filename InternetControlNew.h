#ifndef INTERNET_CONTROL_NEW_H
#define INTERNET_CONTROL_NEW_H
#include "PrivateDefine.h"

#include <winsock2.h>
#include <deque>
#include <string>

#define PACKET_SIZE 1300
#define SYNC_FPS 1

struct PACKET
{
	long long frameIndex = -1;
	int count = 0;
	int index = 0;
	int size = 0;
	int offset = 0;
	int dataSize = 0;
	char data[PACKET_SIZE] = {};
};

enum ConnectType
{
	CT_UNKNOWN = 0,
	CT_TCP = 1,
	CT_TCP_NONBLOCK = 2,
	CT_UDP_BROADCAST = 3,
};

class InternetControlServer
{
private:
	ConnectType m_connectType = CT_UNKNOWN;
    int m_sendbufSize = 1024 * 1024 * 1024;

	WSADATA m_wsaData;
	std::deque<unsigned int> m_dBlockLocalIP;
	std::deque<std::deque<unsigned int> > m_dlocalIPs;
	std::deque<std::pair<SOCKET, sockaddr> > m_dSockets;
	std::deque<char> m_dBroadcastFlag;
	std::deque<sockaddr_in> m_dSend_addr;
	std::deque<int> m_dSend_addr_len;
	std::deque<sockaddr_in> m_dRecv_addr;
	std::deque<int> m_dRecv_addr_len; 
	bool *m_broadcastIPExit = NULL;

	addrinfo *m_tcpAddrIn = NULL;
	SOCKET m_clientSocket;
	in_addr m_clientAddr;
	char *m_needProcBuf = NULL;
	int m_needProcBufSize = 0;

	long long m_frameIndex;

	std::deque<std::deque<unsigned int> > GetLocalIPsAndMasks(ULONG family, std::deque<unsigned int> blockIP);
public:
	bool m_initOK = false;
	bool m_tcpConnected = false;

	InternetControlServer(ConnectType type, short port);
	~InternetControlServer();

	std::deque<int> Send(char *sendbuf, int size);
	int Recv(char *recvbuf, int size, int &reSize);
	void ListenProc();
	//void Listen();
	void BroadcastIPProc(bool *exit);
	void BroadcastIP();
};

class InternetControlClient
{
private:
	ConnectType m_connectType = CT_UNKNOWN;
	WSADATA m_wsaData;
	SOCKET m_serverSocket = INVALID_SOCKET;
    int m_recvbufSize = 1024 * 1024 * 1024;

	char m_broadcast = '1';
	sockaddr_in m_sender_addr;
	sockaddr_in m_recv_addr;
	int m_sender_addr_len = 0;
	char *m_needProcBuf = NULL;
	int m_needProcBufSize = 0;
	long long m_frameIndex;

	std::deque<PACKET> m_dOtherPackets;
	addrinfo *m_tcpAddrIn = NULL;

public:
	bool m_initOK = false;
	bool m_tcpConnected = false;

	in_addr m_serverAddr;

	InternetControlClient(ConnectType type, short port, const char *ip = nullptr);
	~InternetControlClient();

	int Send(char *sendbuf, int size, bool withPacket = true);
	int Recv(char *recvbuf, int size, int &reSize);
};

std::deque<PACKET> SplitPacket(char *buf, int size, int frameIndex);
int MergePacket(std::deque<PACKET> packets, char *recvbuf, int size);
in_addr FindServerIP();

#endif
