#include "pch.h"
#include <iostream>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 0;
	}

	SOCKET clientSocket =  ::socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}
	
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1"); << deprecated
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = ::htons(7777); // 80 : HTTP
	// host to network short , Little-Endian vs Big-Endian 이슈
	// 저장되는 메모리의 주소 진행방향 차이
	// 0x12345678 4바이트 정수
	// low [0x78][0x56][0x34][0x12] high : little = 일반적 컴퓨터, 모바일 환경
	// low [0x12][0x34][0x56][0x78] high : high = network 표준
	//

	if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int32 errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	cout << "Connected To Server!" << endl;

	while (true)
	{
		char sendBuffer[100] = "Hello World!";

		for (int32 i = 0; i < 10; i++)
		{
			int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
			if (resultCode == SOCKET_ERROR)
			{
				int32 errCode = ::WSAGetLastError();
				cout << "Send ErrorCode : " << errCode << endl;
				return 0;
			}
		}

		cout << "Send Data! Len = " << sizeof(sendBuffer) << endl;

		//char recvBuffer[1000];

		//int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		//if (recvLen <= 0)
		//{
		//	int32 errCode = ::WSAGetLastError();
		//	cout << "Send ErrorCode : " << errCode << endl;
		//	return 0;
		//}

		//cout << "Recv Data! Data = " << recvBuffer << endl;
		//cout << "Recv Data! Len = " << recvLen << endl;

		this_thread::sleep_for(1s);
	}

	::closesocket(clientSocket);

	::WSACleanup();
}

