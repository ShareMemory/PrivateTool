#pragma once
#include "PrivateDefine.h"
#include "LogServer.h"
#include "winsock2.h"

#pragma comment (lib, "Ws2_32.lib")

enum ConnectType
{
	CT_UNKNOWN = 0,
	CT_TCP = 1,
	CT_UDP_BROADCAST = 2,
};

class InternetControlServer
{
private:
	WSADATA m_wsaData;
	SOCKET m_socket;

	char m_broadcast = '1';
	sockaddr_in m_recv_addr;
	int m_recv_addr_len = 0;

public:
	bool m_initOK = false;

	InternetControlServer(ConnectType type, short port);
	~InternetControlServer();

	int Send(char *sendbuf, int size);
	int Recv(char *recvbuf, int size);
};

class InternetControlClient
{
private:
	WSADATA m_wsaData;
	SOCKET m_socket;

	char m_broadcast = '1';
	sockaddr_in m_sender_addr;
	sockaddr_in m_recv_addr;
	int m_recv_addr_len = 0;

	char m_serverIp[MAX_SIZE];
public:
	bool m_initOK = false;

	InternetControlClient(ConnectType type, short port/*, const char *ip = nullptr*/);
	~InternetControlClient();

	int Send(char *sendbuf, int size);
	int Recv(char *recvbuf, int size);
};