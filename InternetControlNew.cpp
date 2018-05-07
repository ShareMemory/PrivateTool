#include "InternetControlNew.h"

using namespace std;

InternetControlServer::InternetControlServer(ConnectType type, short port)
{
	switch (type)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP:
		break;
	case CT_UDP_BROADCAST:
		WSAStartup(MAKEWORD(2, 2), &m_wsaData);

		m_socket = socket(AF_INET, SOCK_DGRAM, 0);

		if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &m_broadcast, sizeof(m_broadcast)) < 0)
		{
			g_logServer.ShowLog(TEXT("Error in setting Broadcast option"));
			closesocket(m_socket);
			goto Error;
		}

		m_recv_addr_len = sizeof(sockaddr_in);

		m_recv_addr.sin_family = AF_INET;
		m_recv_addr.sin_port = htons(port);
		m_recv_addr.sin_addr.s_addr = INADDR_BROADCAST;

		m_initOK = true;
		break;
	default:
		break;
	}
Error:
	;
}

InternetControlServer::~InternetControlServer()
{
	closesocket(m_socket);
	WSACleanup();
}

int InternetControlServer::Send(char *sendbuf, int size)
{
	if (!m_initOK)
	{
		return -1;
	}
	return sendto(m_socket, sendbuf, size, 0, (sockaddr *)&m_recv_addr, sizeof(m_recv_addr));
}

int InternetControlServer::Recv(char *recvbuf, int size)
{
	if (!m_initOK)
	{
		return -1;
	}
	return recvfrom(m_socket, recvbuf, size, 0, (sockaddr *)&m_recv_addr, &m_recv_addr_len);
}

InternetControlClient::InternetControlClient(ConnectType type, short port/*, const char *ip*/)
{
	switch (type)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP:
		break;
	case CT_UDP_BROADCAST:
		WSAStartup(MAKEWORD(2, 2), &m_wsaData);

		m_socket = socket(AF_INET, SOCK_DGRAM, 0);

		if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &m_broadcast, sizeof(m_broadcast)) < 0)
		{
			g_logServer.ShowLog(TEXT("Error in setting Broadcast option"));
			closesocket(m_socket);
			goto Error;
		}
		//...
		m_recv_addr_len = sizeof(sockaddr_in);

		m_recv_addr.sin_family = AF_INET;
		m_recv_addr.sin_port = htons(port);
		m_recv_addr.sin_addr.s_addr = INADDR_ANY;

		if (::bind(m_socket, (sockaddr*)&m_recv_addr, sizeof(m_recv_addr)) < 0)
		{
			printf("Error in BINDING. Error code: %d.", WSAGetLastError());
			closesocket(m_socket);
			goto Error;
		}

		m_initOK = true;
		break;
	default:
		break;
	}
Error:
	;
}

InternetControlClient::~InternetControlClient()
{
	closesocket(m_socket);
	WSACleanup();
}

int InternetControlClient::Send(char *sendbuf, int size)
{
	if (!m_initOK)
	{
		return -1;
	}
	return sendto(m_socket, sendbuf, size, 0, (sockaddr *)&m_sender_addr, sizeof(m_sender_addr));
}

int InternetControlClient::Recv(char *recvbuf, int size)
{
	if (!m_initOK)
	{
		return -1;
	}
	return recvfrom(m_socket, recvbuf, size, 0, (sockaddr *)&m_sender_addr, &m_recv_addr_len);
}
