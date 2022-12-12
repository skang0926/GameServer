#include "pch.h"
#include <iostream>

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"
#include "memory.h"
#include "Allocator.h"
#include "MemoryPool.h"

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

	// 블로킹 소켓
	// accept -> 접속한 클라가 있을때
	// connect -> 서버 접속에 성공했을때
	// send, sendto -> 요청한 데이터를 송신 버퍼에 복사했을때
	// recv, recvto  -> 수신 버퍼에 도착한 데이터가 있고, 이를 유저레벨에 복사했을때

	// 논 블로킹

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		return 0;
	}

	u_long on = 1;
	if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
	{
		return 0;
	}

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		return 0;
	}

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		return 0;
	}

	SOCKADDR_IN clientAddr;
	int32 addrLen = sizeof(clientAddr);

	while (true)
	{
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			// 논블로킹이라 여기 들어와도 문제 상황인지 알수없음
			if (::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				continue;
			}

			break;
		}

		cout << "Client Connected" << endl;


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

			while (true)
			{
				if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR)
				{
					if (::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}

					break;
				}				
			
				cout << "Send Data Len = " << recvLen << endl;
				break;
				
			}
		}
	}

	::WSACleanup();
}
