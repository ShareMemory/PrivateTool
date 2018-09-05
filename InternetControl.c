#include <stdio.h>
#include <ws2tcpip.h>
#include "InternetControl.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#ifndef LOG 
#define printf
#endif

#define WIN32_LEAN_AND_MEAN
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "26701"
//#define DEFAULT_IP_ADDRESS "192.168.11.100"
#define DEFAULT_IP_ADDRESS "127.0.0.1"

SOCKET g_listenSocket = INVALID_SOCKET;
SOCKET g_connectSocket = INVALID_SOCKET;

int g_bufSize = DEFAULT_BUFLEN;

int CreateTCPServer()
{
	//WSADATA wsaData;
	int iResult = 0;

	g_listenSocket = INVALID_SOCKET;
	g_connectSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	//iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//printf("+++++++++++++++startup\r\n");
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	// Create a SOCKET for connecting to server
	g_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (g_listenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	// Setup the TCP listening socket
	iResult = bind(g_listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		CloseSocket(&g_listenSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	freeaddrinfo(result);
	return 0;
}

int ListenClient()
{
	int iResult;
	printf("---wait for client---\n");
	iResult = listen(g_listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		CloseSocket(&g_listenSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	// Accept a client socket
	g_connectSocket = accept(g_listenSocket, NULL, NULL);
	if (g_connectSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		CloseSocket(&g_listenSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	// No longer need server socket
	CloseSocket(&g_listenSocket);
	return 0;
}

int Recv(char *recvbuf)
{
	int iResult;

	// Receive until the peer shuts down the connection
	iResult = recv(g_connectSocket, recvbuf, g_bufSize, 0);
	if (iResult > 0) {
		//printf("Bytes received: %d\n", iResult);
		return iResult;
	}
	else if (iResult == 0)
	{
		printf("Connection closing...\n");
		CloseSocket(&g_connectSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return iResult;
	}
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
		CloseSocket(&g_connectSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}
}

int Send(char *sendbuf)
{
	int iResult = 0;
	iResult = send(g_connectSocket, sendbuf, g_bufSize, 0);
	if (iResult > 0) {
		//printf("Bytes sent: %d\n", iResult);
		return iResult;
	}
	// Echo the buffer back to the sender
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		CloseSocket(&g_connectSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}
}

int ShutDownConnect()
{
	int iResult;
	// shutdown the connection since we're done
	iResult = shutdown(g_connectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		CloseSocket(&g_connectSocket);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}
	CloseSocket(&g_connectSocket);
	return 0;
}

int CleanUp()
{
	// cleanup
	printf("cleanup...\n");
	CloseSocket(&g_connectSocket);
	CloseSocket(&g_listenSocket);
	//WSACleanup();
	//printf("+++++++++++++++Cleanup\r\n");
	return 0;
}

int CloseSocket(SOCKET *socket)
{
	if (*socket != INVALID_SOCKET)
	{
		closesocket(*socket);
		*socket = INVALID_SOCKET;
	}
	return 0;
}

int ReListenClient()
{
	int re = 0;
	re = CleanUp();
	if (re != 0)
		goto Error;
	re = CreateTCPServer();
	if (re != 0)
		goto Error;
	re = ListenClient();
	if (re != 0)
		goto Error;
Error:
	return re;
}

int ConnectTCPServer()
{
	//WSADATA wsaData;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult = 0;

	// Initialize Winsock
	//iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//printf("+++++++++++++++startup\r\n");
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return -1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(DEFAULT_IP_ADDRESS, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		g_connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (g_connectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			//WSACleanup();
			//printf("+++++++++++++++Cleanup\r\n");
			return -1;
		}

		// Connect to server.
		iResult = connect(g_connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			CloseSocket(&g_connectSocket);
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (g_connectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		//WSACleanup();
		//printf("+++++++++++++++Cleanup\r\n");
		return -1;
	}

	return 0;
}

int ReConnectServer()
{
	int re = 0;
	re = CleanUp();
	if (re != 0)
		goto Error;
	re = ConnectTCPServer();
	if (re != 0)
		goto Error;
Error:
	return re;
}