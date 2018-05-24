#include "InternetControlNew.h"
#include <iphlpapi.h>
#include <WS2tcpip.h>
#include <thread>

#ifndef __BCPLUSPLUS__
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#else
#pragma comment (lib, "Ws2_32.a")
#pragma comment(lib, "IPHLPAPI.a")
#endif

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

using namespace std;

deque<deque<unsigned int> > InternetControlServer::GetLocalIPsAndMasks(ULONG family, deque<unsigned int> blockIP)
{
	deque<deque<unsigned int> > re;

	DWORD dwRetVal = 0;

	//// Set the flags to pass to GetAdaptersAddresses
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

	//// default to unspecified address family (both)

	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	ULONG outBufLen = 0;
	ULONG Iterations = 0;

	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
	IP_ADAPTER_PREFIX *pPrefix = NULL;

	//printf("Calling GetAdaptersAddresses function with family = ");
	//if (family == AF_INET)
		//printf("AF_INET\n");
	//if (family == AF_INET6)
		//printf("AF_INET6\n");
	//if (family == AF_UNSPEC)
		//printf("AF_UNSPEC\n\n");

	// Allocate a 15 KB buffer to start with.
	outBufLen = WORKING_BUFFER_SIZE;

	do {

		pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
		if (pAddresses == NULL) {
			printf
			("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
			//exit(1);
			return re;
		}

		dwRetVal =
			GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

		if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
			FREE(pAddresses);
			pAddresses = NULL;
		}
		else {
			break;
		}

		Iterations++;

	} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

	if (dwRetVal == NO_ERROR)
	{
		//// If successful, output some information from the data we received
		pCurrAddresses = pAddresses;
		while (pCurrAddresses) {
			//printf("\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n",
				//pCurrAddresses->Length);
			//printf("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
			//printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);

			pUnicast = pCurrAddresses->FirstUnicastAddress;
			if (pUnicast != NULL) {
				for (;pUnicast != NULL; pUnicast = pUnicast->Next)
				{
					deque<unsigned int> ipAndMask;
					sockaddr_in *sa_in = (sockaddr_in *)pUnicast->Address.lpSockaddr;
					for (int i = 0; i < blockIP.size(); i++)
					{
						if (sa_in->sin_addr.S_un.S_addr == blockIP[i])
						{
							goto Continue;
						}
					}
					{
						ULONG i32mask = 0;
						ConvertLengthToIpv4Mask(pUnicast->OnLinkPrefixLength, &i32mask);
						////同样mask只选一个
						////if (m_connectType)
						////{
						////	for (int i = 0; i < re.size(); i++)
						////	{
						////		if (i32mask == re[i][1])
						////		{
						////			goto Continue;
						////		}
						////	}
						////}
						ipAndMask.push_back(sa_in->sin_addr.S_un.S_addr);
						ipAndMask.push_back(i32mask);
						re.push_back(ipAndMask);
						////每块网卡只选一个ip
						break;
					}
				Continue:
					;
				}
			}
			else
				//printf("\tNo Unicast Addresses\n");

			if (pCurrAddresses->PhysicalAddressLength != 0) {
				//printf("\tPhysical address: ");
				for (unsigned int i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;
					i++) {
					//if (i == (pCurrAddresses->PhysicalAddressLength - 1))
						//printf("%.2X\n",
						//(int)pCurrAddresses->PhysicalAddress[i]);
					//else
						//printf("%.2X-",
						//(int)pCurrAddresses->PhysicalAddress[i]);
				}
			}
			//printf("\tFlags: %ld\n", pCurrAddresses->Flags);
			//printf("\tMtu: %lu\n", pCurrAddresses->Mtu);
			//printf("\tIfType: %ld\n", pCurrAddresses->IfType);
			//printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
			//printf("\tIpv6IfIndex (IPv6 interface): %u\n",
				//pCurrAddresses->Ipv6IfIndex);
			//printf("\tZoneIndices (hex): ");
			//for (int i = 0; i < 16; i++)
				//printf("%lx ", pCurrAddresses->ZoneIndices[i]);
			//printf("\n");

			//printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);
			//printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);

			pPrefix = pCurrAddresses->FirstPrefix;
			if (pPrefix) {
				for (int i = 0; pPrefix != NULL; i++)
				{
					pPrefix = pPrefix->Next;
					//printf("\tNumber of IP Adapter Prefix entries: %d\n", i);
				}
			}
			//else
				//printf("\tNumber of IP Adapter Prefix entries: 0\n");

			//printf("\n");

			pCurrAddresses = pCurrAddresses->Next;
		}
	}

	printf("[UDP_B]local ip and mask: \r\n");
	for (int i = 0; i < re.size(); i++)
	{
		in_addr ip = in_addr();
		in_addr mask = in_addr();
		in_addr broadcastIP = in_addr();
		ip.S_un.S_addr = re[i][0];
		mask.S_un.S_addr = re[i][1];
		printf("Index: %d, ip : %s, ", i, inet_ntoa(ip));
		printf("mask : %s, ", inet_ntoa(mask));
		re[i].push_back(re[i][0] | (re[i][1] ^ 0xFFFFFFFF));
		broadcastIP.S_un.S_addr = re[i][2];
		printf("broadcastIP : %s\r\n", inet_ntoa(broadcastIP));
	}
	return re;
}

InternetControlServer::InternetControlServer(ConnectType type, short port)
{
	m_connectType = type;
	WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	m_needProcBuf = (char*)malloc(sizeof(PACKET) * 10000);
	memset(m_needProcBuf, 0, sizeof(PACKET) * 10000);
	switch (type)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP_NONBLOCK:
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(NULL, to_string(port).c_str(), &hints, &m_tcpAddrIn);

		break;
	case CT_UDP_BROADCAST:
		m_dBlockLocalIP.push_back(inet_addr("127.0.0.1"));
		m_dlocalIPs = GetLocalIPsAndMasks(AF_INET, m_dBlockLocalIP);

		for (int i = 0; i < m_dlocalIPs.size(); i++)
		{
			SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
			char broadcastFlag = '1';
			if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastFlag, sizeof(broadcastFlag)) < 0)
			{
				printf("[UDP_B]Error in setting Broadcast option.\r\n");
				closesocket(sock);
				goto Error;
			}
			int send_addr_len = sizeof(sockaddr_in);
			sockaddr_in send_addr;
			send_addr.sin_family = AF_INET;
			send_addr.sin_port = htons(port);
			send_addr.sin_addr.s_addr = m_dlocalIPs[i][0];

 			if (::bind(sock, (sockaddr*)&send_addr, sizeof(send_addr)) < 0)
			{
				printf("[UDP_B]Error bind to %s. Error code: %d.\r\n", inet_ntoa(send_addr.sin_addr), WSAGetLastError());
				//删除没用的ip
				//m_dlocalIPs.erase(m_dlocalIPs.begin() + i);
				//i--;
				closesocket(sock);
				continue;
			}
			else
			{
				printf("[UDP_B]Bind to  %s.\r\n", inet_ntoa(send_addr.sin_addr));
			}

			m_dBroadcastFlag.push_back(broadcastFlag);
			m_dSend_addr_len.push_back(send_addr_len);
			m_dSend_addr.push_back(send_addr);

			int recv_addr_len = sizeof(sockaddr_in);
			sockaddr_in recv_addr;
			recv_addr.sin_family = AF_INET;
			recv_addr.sin_port = htons(port);
			recv_addr.sin_addr.s_addr = m_dlocalIPs[i][2];

			m_dRecv_addr_len.push_back(recv_addr_len);
			m_dRecv_addr.push_back(recv_addr);

			pair<SOCKET, sockaddr> socketAddr;
			socketAddr.first = sock;
			socketAddr.second = sockaddr();
			m_dSockets.push_back(socketAddr);

		}
		break;
	default:
		break;
	}
	m_initOK = true;
Error:
	;
}

InternetControlServer::~InternetControlServer()
{
	if (m_broadcastIPExit != NULL && m_broadcastIPExit == false)
	{
		*m_broadcastIPExit = true;
	}
	while (true)
	{
		if (*m_broadcastIPExit == false)
		{
			delete m_broadcastIPExit;
			m_broadcastIPExit = NULL;
			break;
		}
	}
	if (m_needProcBuf != NULL)
	{
		free(m_needProcBuf);
		m_needProcBuf = NULL;
	}
	if (m_tcpAddrIn != NULL)
	{
		freeaddrinfo(m_tcpAddrIn);
		m_tcpAddrIn = NULL;
	}
	if (m_clientSocket != INVALID_SOCKET)
	{
		closesocket(m_clientSocket);
		m_clientAddr = in_addr();
	}
	WSACleanup();
}

void InternetControlServer::BroadcastIPProc(bool *exit)
{
	unsigned char *sendbuf = (unsigned char*)malloc(MAX_SIZE);
	char *recvbuf = (char*)malloc(MAX_SIZE);
	memset(sendbuf, 0, MAX_SIZE);
	memset(recvbuf, 0, MAX_SIZE);
	sendbuf[0] = 172;
	bool sendSuccess = false;
	do
	{
		std::deque<int> res = Send((char *)sendbuf, MAX_SIZE);
		for (int i = 0; i < res.size(); i++)
		{
			//printf("[BroadcastIP]send byte: %d\r\n", res[i]);
			if (res[i] > 0)
			{
				sendSuccess = true;
			}
		}
		Sleep(1000);
	} while (!*exit);
	free(sendbuf);
	sendbuf = NULL;
	free(recvbuf);
	recvbuf = NULL;
	*exit = false;
}

void InternetControlServer::BroadcastIP()
{
	m_broadcastIPExit = new bool(false);
	std::thread broadcastIP = std::thread(&InternetControlServer::BroadcastIPProc, this, m_broadcastIPExit);
	broadcastIP.detach();
}

void InternetControlServer::ListenProc()
{
	//do
	//{
	SOCKET listenSocket = socket(m_tcpAddrIn->ai_family, m_tcpAddrIn->ai_socktype, m_tcpAddrIn->ai_protocol);
	int re = 0;
	re = ::bind(listenSocket, m_tcpAddrIn->ai_addr, (int)m_tcpAddrIn->ai_addrlen);
	if (re == SOCKET_ERROR)
	{
		printf("[TCP]bind failed with error: %d\r\n", WSAGetLastError());
		return;
	}
	printf("[TCP]---wait for client---\r\n");
	re = listen(listenSocket, SOMAXCONN);
	if (re == SOCKET_ERROR)
	{
		printf("[TCP]listen failed with error: %d\r\n", WSAGetLastError());
		return;
	}
	sockaddr clientAddr;
	int clientAddrLen = sizeof(sockaddr);
	SOCKET sock = accept(listenSocket, &clientAddr, &clientAddrLen);
	if (sock == INVALID_SOCKET)
	{
		printf("[TCP]accept failed with error: %d\r\n", WSAGetLastError());
	}
	else
	{
		printf("[TCP]%s connected.\r\n", inet_ntoa(((sockaddr_in*)&clientAddr)->sin_addr));
		if (m_connectType == CT_TCP_NONBLOCK)
		{
			u_long mode = 1;
			re = ioctlsocket(sock, FIONBIO, &mode);
			if (re != 0)
			{
				printf("[TCP]set nonblock failed with error: %d.\r\n", WSAGetLastError());
				return;
			}
		}
		m_clientSocket = sock;
		m_clientAddr = ((sockaddr_in*)&clientAddr)->sin_addr;
		{
			int re = ::setsockopt(m_clientSocket, SOL_SOCKET, SO_SNDBUF, (char *)&m_sendbufSize, sizeof(m_sendbufSize));
			if (re != 0)
			{
				printf("[TCP]Set sendbuf size failed, error code: %d\r\n", WSAGetLastError());
			}
		}
		m_tcpConnected = true;

	}
	closesocket(listenSocket);
	listenSocket = INVALID_SOCKET;
	//} while (true);
}

//void InternetControlServer::Listen()
//{
//	std::thread listenThread = thread(&InternetControlServer::ListenProc, this);
//	listenThread.detach();
//}

deque<int> InternetControlServer::Send(char *sendbuf, int size)
{
	deque<int> successRe;
	while (!m_initOK || (!m_tcpConnected && m_connectType == CT_TCP_NONBLOCK))
	{
		return successRe;
	}
	deque<PACKET> packets = SplitPacket(sendbuf, size, m_frameIndex);
	successRe.push_back(0);
	for (int i = 0; i < packets.size(); i++)
	{
		int re = 0;
		switch (m_connectType)
		{
		case CT_UNKNOWN:
			break;
		case CT_TCP_NONBLOCK:
			re = send(m_clientSocket, (char*)&packets[i], sizeof(PACKET), 0);
				
			if (re > 0)
			{
				successRe[0] += packets[i].dataSize;
				//printf("[TCP]Send to socket index %d %d bytes.\r\n", j, successRe[j]);
			}
			else
			{
				int errorCode = WSAGetLastError();
				printf("[TCP]Error send to %s, packet index: %d, error code: %d.\r\n", inet_ntoa(m_clientAddr), i, errorCode);
				if (errorCode == WSAEWOULDBLOCK)
				{
					printf("[TCP]Continue send frame.\r\n");
				}
				else
				{
					printf("[TCP]Connection terminated\r\n");
					closesocket(m_clientSocket);
					m_clientAddr = in_addr();
					m_tcpConnected = false;
				}
				goto NextFrame;
			}
			break;
		case CT_UDP_BROADCAST:
			for (int j = 0; j < m_dSockets.size(); j++)
			{
				if (successRe.size() == j)
					successRe.push_back(0);
				re = sendto(m_dSockets[j].first, (char*)&packets[i], sizeof(PACKET), 0, (sockaddr *)&m_dRecv_addr[j], m_dRecv_addr_len[j]);
				if (re > 0)
				{
					successRe[j] += packets[i].dataSize;
					//printf("[UDP_B]Send to %s %d bytes.\r\n", inet_ntoa(m_dRecv_addr[j].sin_addr), successRe[j]);
				}
				else
				{
					printf("[UDP_B]Error send to %s, packet index: %d, error code: %d.\r\n", inet_ntoa(m_dRecv_addr[j].sin_addr), i, WSAGetLastError());
					closesocket(m_dSockets[j].first);
					m_dSockets.erase(m_dSockets.begin() + j);
					j--;
				}
			}
			break;
		default:
			break;
		}
	}
NextFrame:
	;
	switch (m_connectType)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP_NONBLOCK:
		printf("[TCP]Send to %s %d bytes.\r\n", inet_ntoa(m_clientAddr), successRe[0]);
		break;
	case CT_UDP_BROADCAST:
		for (int j = 0; j < m_dSockets.size(); j++)
		{
			printf("[UDP_B]Send to %s %d bytes.\r\n", inet_ntoa(m_dRecv_addr[j].sin_addr), successRe[j]);
		}
		break;
	default:
		break;
	}
	m_frameIndex++;
	return successRe;
}

int InternetControlServer::Recv(char *recvbuf, int size, int &reSize)
{
	int recvSuccess = 0;
	while (!m_initOK || (!m_tcpConnected && m_connectType == CT_TCP_NONBLOCK))
	{
		recvSuccess = 4;
		return recvSuccess;
	}
	int successRe = 0;
	deque<PACKET> packets;
	PACKET firstPacket;
	bool isFirstPackets = false;
	int packetsCount = 0;
	clock_t tick = clock();
	int outTime = 1000 / SYNC_FPS;
	do
	{
		int re = 0;
		recvSuccess = 0;
		PACKET packet;
		switch (m_connectType)
		{
		case CT_UNKNOWN:
			break;
		case CT_TCP_NONBLOCK:
			re = recv(m_clientSocket, m_needProcBuf + m_needProcBufSize, sizeof(PACKET), 0);
			if (re > 0)
			{
				recvSuccess = 0;
			}
			else
			{
				int errorCode = WSAGetLastError();
				if (errorCode == WSAEWOULDBLOCK)
				{
					printf("[TCP]Continue recv frame.\r\n");
					recvSuccess = 5;
				}
				else
				{
					printf("[TCP]Error recv from %s. packet index: %d, error code: %d.\r\n", inet_ntoa(m_clientAddr), packetsCount, errorCode);
					printf("[TCP]Connection terminated\r\n");
					closesocket(m_clientSocket);
					m_clientAddr = in_addr();
					m_tcpConnected = false;
					recvSuccess = 1;
				}
				goto NextFrame;
			}
			break;
		case CT_UDP_BROADCAST:
			break;
		default:
			break;
		}

		//if (re != sizeof(PACKET))
		//{
		//	printf("re != sizeof(PACKET)\r\n");
		//}

		m_needProcBufSize += re;
		int procPacketCount = m_needProcBufSize / sizeof(PACKET);
		for (int i = 0; i < procPacketCount; i++)
		{
			PACKET packet;
			memcpy(&packet, m_needProcBuf, sizeof(PACKET));
			if (firstPacket.frameIndex == -1)
			{
                if(packet.index == 0)
                {
					firstPacket = packet;
                }
                else
                {
					m_needProcBufSize -= sizeof(PACKET);
					memcpy(m_needProcBuf, m_needProcBuf + sizeof(PACKET), m_needProcBufSize);
                    continue;
                }
			}
			if (packetsCount < firstPacket.count)
			{
				if (packet.frameIndex == firstPacket.frameIndex)
				{
					packets.push_back(packet);
					packetsCount++;
					m_needProcBufSize -= sizeof(PACKET);
					memcpy(m_needProcBuf, m_needProcBuf + sizeof(PACKET), m_needProcBufSize);
				}
				else
				{
					printf("[ERROR]packet recv data error.\r\n");
					recvSuccess = 2;
					goto NextFrame;
				}
			}
			else
			{
				printf("[ERROR]unknown error.\r\n");
				recvSuccess = 3;
			}
		}
	} while (packetsCount < firstPacket.count || firstPacket.frameIndex == -1);
NextFrame:
	successRe = MergePacket(packets, recvbuf, size);
	switch (m_connectType)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP_NONBLOCK:
		printf("[TCP]Recv from %s %d bytes.\r\n", inet_ntoa(m_clientAddr), successRe);
		break;
	case CT_UDP_BROADCAST:
		break;
	default:
		break;
	}
	reSize = successRe;
	return recvSuccess;
}

InternetControlClient::InternetControlClient(ConnectType type, short port, const char *ip)
{
	m_connectType = type;
	if(ip != NULL)
		m_serverAddr.S_un.S_addr = inet_addr(ip);
	WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	m_needProcBuf = (char*)malloc(sizeof(PACKET) * 10000);
	memset(m_needProcBuf, 0, sizeof(PACKET) * 10000);
	switch (type)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP:
		printf("[TCP]connect to %s...\r\n", ip);

		while (m_serverSocket == INVALID_SOCKET)
		{
			m_serverSocket = INVALID_SOCKET;
			addrinfo hints;
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			getaddrinfo(ip, to_string(port).c_str(), &hints, &m_tcpAddrIn);

			for (addrinfo *ptr = m_tcpAddrIn; ptr != NULL; ptr = ptr->ai_next)
			{
				m_serverSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				int re = connect(m_serverSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (re == SOCKET_ERROR)
				{
					closesocket(m_serverSocket);
					m_serverSocket = INVALID_SOCKET;
					int errorcode = WSAGetLastError();
					if (errorcode != 0)
						printf("[TCP]connect to %s failed, error code: %d\r\n", ip, errorcode);
					continue;
				}
				break;
			}
		}

{
        int re = ::setsockopt(m_serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&m_recvbufSize, sizeof(int));
        if(re != 0)
        {
        	printf("[TCP]Set recvbuf size failed, error code: %d\r\n", WSAGetLastError());
        }
}
		m_tcpConnected = true;
		printf("[TCP]connect to %s success.\r\n", ip);
		break;
	case CT_UDP_BROADCAST:

		m_serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

		if (setsockopt(m_serverSocket, SOL_SOCKET, SO_BROADCAST, &m_broadcast, sizeof(m_broadcast)) < 0)
		{
			printf("Error in setting Broadcast option\r\n");
			closesocket(m_serverSocket);
			goto Error;
		}
		//...
		m_sender_addr_len = sizeof(sockaddr_in);

		m_recv_addr.sin_family = AF_INET;
		m_recv_addr.sin_port = htons(port);
		m_recv_addr.sin_addr.s_addr = INADDR_ANY;

		if (::bind(m_serverSocket, (sockaddr*)&m_recv_addr, sizeof(m_recv_addr)) < 0)
		{
			printf("[UDP_B]Error in BINDING. Error code: %d.", WSAGetLastError());
			closesocket(m_serverSocket);
			goto Error;
		}

		break;
	default:
		break;
	}
	m_initOK = true;
Error:
	;
}

InternetControlClient::~InternetControlClient()
{
	if (m_needProcBuf != NULL)
	{
		free(m_needProcBuf);
		m_needProcBuf = NULL;
	}
	if (m_tcpAddrIn != NULL)
	{
		freeaddrinfo(m_tcpAddrIn);
		m_tcpAddrIn = NULL;
	}
	closesocket(m_serverSocket);
	WSACleanup();
}

int InternetControlClient::Send(char *sendbuf, int size, bool withPacket)
{
	while (!m_initOK || (!m_tcpConnected && m_connectType == CT_TCP))
	{
		Sleep(1);
	}
	int successRe = 0;
	if(withPacket)
	{
		deque<PACKET> packets = SplitPacket(sendbuf, size, m_frameIndex);
		for (int i = 0; i < packets.size(); i++)
		{
			int re = 0;
			switch (m_connectType)
			{
			case CT_UNKNOWN:
				break;
			case CT_TCP:
				re = send(m_serverSocket, (char*)&packets[i], sizeof(PACKET), 0);

				if (re > 0)
				{
					successRe += packets[i].dataSize;
					//printf("[TCP]Send to socket index %d %d bytes.\r\n", j, successRe[j]);
				}
				else
				{
					printf("[TCP]Error send to %s, packet index: %d, error code: %d.\r\n", inet_ntoa(m_serverAddr), i, WSAGetLastError());
					printf("[TCP]Connection terminated\r\n");
					closesocket(m_serverSocket);
					m_serverSocket = INVALID_SOCKET;
					m_tcpConnected = false;
					goto NextFrame;
				}
				break;
			case CT_UDP_BROADCAST:
				break;
			default:
				break;
			}
		}
		m_frameIndex++;
	}
	else
	{
    	int re = 0;
		switch (m_connectType)
		{
		case CT_UNKNOWN:
			break;
		case CT_TCP:
			re = send(m_serverSocket, sendbuf, size, 0);

			if (re > 0)
			{
				successRe = re;
				//printf("[TCP]Send to socket index %d %d bytes.\r\n", j, successRe[j]);
			}
			else
			{
				printf("[TCP]Error send to %s, error code: %d.\r\n", inet_ntoa(m_serverAddr), WSAGetLastError());
				printf("[TCP]Connection terminated\r\n");
				closesocket(m_serverSocket);
				m_serverSocket = INVALID_SOCKET;
				m_tcpConnected = false;
				goto NextFrame;
			}
			break;
		case CT_UDP_BROADCAST:
			break;
		default:
			break;
		}
    }
NextFrame:
	switch (m_connectType)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP:
		printf("[TCP]Send to %s %d bytes.\r\n", inet_ntoa(m_serverAddr), successRe);
		break;
	case CT_UDP_BROADCAST:
		break;
	default:
		break;
	}

	return successRe;
}

int InternetControlClient::Recv(char *recvbuf, int size, int &reSize)
{
	int recvSuccess = 0;
	while (!m_initOK || (!m_tcpConnected && m_connectType == CT_TCP))
	{
		recvSuccess = 4;
		return recvSuccess;
	}
	int successRe = 0;
	deque<PACKET> packets;
	PACKET firstPacket;
	bool isFirstPackets = false;
	int packetsCount = 0;
	clock_t tick = clock();
	int outTime = 1000 / SYNC_FPS;
	do
	{
		int re = 0;
		recvSuccess = 0;
		PACKET packet;
		switch (m_connectType)
		{
		case CT_UNKNOWN:
			break;
		case CT_TCP:
			re = recv(m_serverSocket, m_needProcBuf + m_needProcBufSize, sizeof(PACKET), 0);
			if (re > 0)
			{
				recvSuccess = 0;
			}
			else
			{
				printf("[TCP]Error recv from %s. packet index: %d, error code: %d.\r\n", inet_ntoa(m_serverAddr), packetsCount, WSAGetLastError());
				closesocket(m_serverSocket);
				m_serverSocket = INVALID_SOCKET;
				m_tcpConnected = false;
				recvSuccess = 1;
				goto NextFrame;
			}
			break;
		case CT_UDP_BROADCAST:
			if (clock() - tick > outTime)
			{
				printf("[UDP_B]Error recv from %s. packet index: %d, error code: time out.\r\n", inet_ntoa(m_serverAddr), packetsCount);
				goto NextFrame;
			}
			re = recvfrom(m_serverSocket, m_needProcBuf + m_needProcBufSize, sizeof(PACKET), 0, (sockaddr *)&m_sender_addr, &m_sender_addr_len);
			m_serverAddr = m_sender_addr.sin_addr;
			if (re > 0)
			{
				recvSuccess = 0;
			}
			else
			{
				printf("[UDP_B]Error recv. packet index: %d, error code: %d.\r\n", packetsCount, WSAGetLastError());
				closesocket(m_serverSocket);
				m_serverSocket = INVALID_SOCKET;
				m_serverAddr = in_addr();
				recvSuccess = 1;
				goto NextFrame;
			}
			break;
		default:
			break;
		}

		//if (re != sizeof(PACKET))
		//{
		//	printf("re != sizeof(PACKET)\r\n");
		//}

		m_needProcBufSize += re;
		int procPacketCount = m_needProcBufSize / sizeof(PACKET);
		for (int i = 0; i < procPacketCount; i++)
		{
			PACKET packet;
			memcpy(&packet, m_needProcBuf, sizeof(PACKET));
			if (firstPacket.frameIndex == -1)
			{
                if(packet.index == 0)
                {
					firstPacket = packet;
                }
                else
                {
					m_needProcBufSize -= sizeof(PACKET);
					memcpy(m_needProcBuf, m_needProcBuf + sizeof(PACKET), m_needProcBufSize);
                    continue;
                }
			}
			if (packetsCount < firstPacket.count)
			{
				if (packet.frameIndex == firstPacket.frameIndex)
				{
					packets.push_back(packet);
					packetsCount++;
					m_needProcBufSize -= sizeof(PACKET);
					memcpy(m_needProcBuf, m_needProcBuf + sizeof(PACKET), m_needProcBufSize);
				}
				else
				{
					printf("[ERROR]packet recv data error.\r\n");
					recvSuccess = 2;
					goto NextFrame;
				}
			}
			else
			{
				printf("[ERROR]unknown error.\r\n");
				recvSuccess = 3;
			}
		}
	}
	while (packetsCount < firstPacket.count || firstPacket.frameIndex == -1);
NextFrame:
	successRe = MergePacket(packets, recvbuf, size);
	switch (m_connectType)
	{
	case CT_UNKNOWN:
		break;
	case CT_TCP:
		printf("[TCP]Recv from %s %d bytes.\r\n", inet_ntoa(m_serverAddr), successRe);
		break;
	case CT_UDP_BROADCAST:
		printf("[UDP_B]Recv from %s %d bytes.\r\n", inet_ntoa(m_serverAddr), successRe);
		break;
	default:
		break;
	}
	reSize = successRe;
	return recvSuccess;
}

in_addr FindServerIP()
{
	InternetControlClient icc = InternetControlClient(ConnectType::CT_UDP_BROADCAST, 27586);
	char *sendbuf = (char*)malloc(MAX_SIZE);
	char *recvbuf = (char*)malloc(MAX_SIZE);
	memset(sendbuf, 0, MAX_SIZE);
	memset(recvbuf, 0, MAX_SIZE);
	bool sendSuccess = false;
	do
	{
		int reSize = 0;
		bool success = icc.Recv(recvbuf, MAX_SIZE, reSize);
		//printf("recv byte: %d\r\n", reSize);
		if ((unsigned char)recvbuf[0] == 172)
		{
			return icc.m_serverAddr;
		}
	} while (true);

}

std::deque<PACKET> SplitPacket(char *buf, int size, int frameIndex)
{
	int packetCount = size / PACKET_SIZE + 1;
	deque<PACKET> re;
	for (int i = 0; i < packetCount; i++)
	{
		PACKET packet;
		packet.frameIndex = frameIndex;
		packet.count = packetCount;
		packet.index = i;
		packet.size = size;
		packet.offset = i * PACKET_SIZE;
		if (i == packetCount - 1)
		{
			packet.dataSize = size - (packetCount - 1) * PACKET_SIZE;
		}
		else
		{
			packet.dataSize = PACKET_SIZE;
		}
		memcpy(packet.data, buf + packet.offset, packet.dataSize);
		re.push_back(packet);
	}
	return re;
}

int MergePacket(std::deque<PACKET> packets, char *recvbuf, int size)
{
	//bool swapp = true;
	//while (swapp) 
	//{
	//	swapp = false;
	//	for (size_t i = 0; i < packets.size() - 1; i++) 
	//	{
	//		if (packets[i].index > packets[i + 1].index) 
	//		{
	//			std::swap(packets[i], packets[i + 1]);
	//			swapp = true;
	//		}
	//	}
	//}
	int successRe = 0;
	for (int i = 0; i < packets.size(); i++)
	{
		if (packets[i].offset + packets[i].dataSize < size)
		{
			memcpy(recvbuf + packets[i].offset, packets[i].data, packets[i].dataSize);
			successRe += packets[i].dataSize;
		}
		else
		{
			memcpy(recvbuf + packets[i].offset, packets[i].data, size - packets[i].offset);
			successRe += size - packets[i].offset;
		}
	}
	return successRe;
}
