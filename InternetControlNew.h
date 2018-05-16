#pragma once
#include "PrivateDefine.h"
#include "LogServer.h"
#include <winsock2.h>
#include <deque>

#pragma comment (lib, "Ws2_32.lib")

#define PACKET_SIZE 1300
#define SYNC_FPS 30

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
	CT_UDP_BROADCAST = 2,
};

class InternetControlServer
{
private:
	ConnectType m_connectType = CT_UNKNOWN;

	WSADATA m_wsaData;
	std::deque<unsigned int> m_dBlockLocalIP;
	std::deque<std::deque<unsigned int> > m_dlocalIPs;
	std::deque<std::pair<SOCKET, sockaddr> > m_dSockets;
	std::deque<char> m_dBroadcastFlag;
	std::deque<sockaddr_in> m_dSend_addr;
	std::deque<int> m_dSend_addr_len;
	std::deque<sockaddr_in> m_dRecv_addr;
	std::deque<int> m_dRecv_addr_len;
	long long m_frameIndex;
	addrinfo *m_tcpAddrIn = NULL;

	std::deque<std::deque<unsigned int> > GetLocalIPsAndMasks(ULONG family, std::deque<unsigned int> blockIP);
	std::deque<PACKET> SplitPacket(char *buf, int size);
public:
	bool m_initOK = false;
	bool m_tcpConnected = false;

	InternetControlServer(ConnectType type, short port);
	~InternetControlServer();

	void Listen();
	std::deque<int> Send(char *sendbuf, int size);
	int Recv(char *recvbuf, int size);
	void ListenProc();
};

class InternetControlClient
{
private:
	ConnectType m_connectType = CT_UNKNOWN;
	WSADATA m_wsaData;
	SOCKET m_socket = INVALID_SOCKET;

	char m_broadcast = '1';
	sockaddr_in m_sender_addr;
	sockaddr_in m_recv_addr;
	int m_sender_addr_len = 0;
	char *m_needProcBuf = NULL;
	int m_needProcBufSize = 0;

	std::deque<PACKET> m_dOtherPackets;
	addrinfo *m_tcpAddrIn = NULL;

	int MergePacket(std::deque<PACKET> packets, char *recvbuf, int size);
public:
	bool m_initOK = false;
	bool m_tcpConnected = false;

	in_addr m_serverIp;

	InternetControlClient(ConnectType type, short port, const char *ip = nullptr);
	~InternetControlClient();

	int Send(char *sendbuf, int size);
	bool Recv(char *recvbuf, int size, int &reSize);
};

in_addr FindServerIP();
