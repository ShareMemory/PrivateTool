#ifndef INTERNET_CONTROL_H
#define INTERNET_CONTROL_H
#include "PrivateDefine.h"

#include <winsock2.h>

#ifdef __cplusplus
extern "C"
{
#endif
	int CreateTCPServer();
	int ListenClient();
	int Recv(char *recvbuf);
	int Send(char *sendbuf);
	int ShutDownConnect();
	int CleanUp();
	int CloseSocket(SOCKET *socket);
	int ReListenClient();

	int ConnectTCPServer();
	int ReConnectServer();
#ifdef __cplusplus
}
#endif
#endif
