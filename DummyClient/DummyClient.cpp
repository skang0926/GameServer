#include "pch.h"
#include <iostream>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

void HandleError(const char* cause)
{
	int32 errCode = ::WSAGetLastError();
	cout << "Socket ErrorCode : " << errCode << endl;
}

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 0;
	}

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		return 0;
	}

	u_long on = 1;
	if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		return 0;
	}

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777);


	while (true)
	{
		if (::connect(clientSocket,(SOCKADDR*)& serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			// 논블로킹이라 여기 들어와도 문제 상황인지 알수없음
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}

			// 이미 연결되어있는 상태
			if (::WSAGetLastError() == WSAEISCONN)
			{
				break;
			}

			break;
		}
	}

	cout << "Connected to Server" << endl;

	char sendBuffer[100] = "Hello World";

	while (true)
	{
		if (::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR)
		{
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}

			break;
		}

		cout << "Send Data Len = " << sizeof(sendBuffer) << endl;

		while (true)
		{
			char recvBuffer[1000];
			int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
			if (recvLen == SOCKET_ERROR)
			{
				if (::WSAGetLastError() == WSAEWOULDBLOCK)
				{
					continue;
				}

				break;
			}
			else if (recvLen == 0)
			{
				break;
			}

			cout << "Recv Data Len = " << recvLen << endl;
			break;
		}

		this_thread::sleep_for(1s);

	}
	::closesocket(clientSocket);

	::WSACleanup();
}

