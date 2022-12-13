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

#include "Memory.h"

void HandleError(const char* cause)
{
	int32 errCode = ::WSAGetLastError();
	cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000;

struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuffer[BUFSIZE] = {};
	int32 recvBytes = 0;
};

enum IO_TYPE
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
};

struct OverlappedEx
{
	WSAOVERLAPPED overlapped = {};
	int32 type = 0; // read, write, accept, connect ...
};

void WorkerThreadMain(HANDLE iocpHandle)
{
	while (true)
	{
		DWORD bytesTransferred = 0;
		Session* session = nullptr;
		OverlappedEx* overlappedEx = nullptr;

		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
			(ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

		if (ret == FALSE || bytesTransferred == 0)
		{
			// TODO : 연결 끊김
			continue;
		}

		ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ);

		cout << "Recv Data IOCP = " << bytesTransferred << endl;

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
		// Recv를 또 해서 다시 queue 에 등록 할 수 있도록 함수 호출
	}
}

int main()
{
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(7777);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;

	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;

	cout << "Accept" << endl;

	// Overlapped 모델 (Completion Routine 콜백 기반)
	// - 비동기 입출력 함수 완료되면, 쓰레드마다 있는 APC 큐에 일감이 쌓임
	// - Alertable Wait 상태로 들어가서 APC 큐 비우기 (콜백 함수)
	// 단점) APC큐 쓰레드마다 있다! Alertable Wait 자체도 조금 부담!
	// 단점) 이벤트 방식 소켓:이벤트 1:1 대응

	// IOCP (Completion Port) 모델
	// - APC -> Completion Port (쓰레드마다 있는건 아니고 1개. 중앙에서 관리하는 APC 큐?)
	// - Alertable Wait -> CP 결과 처리를 GetQueuedCompletionStatus
	// 쓰레드랑 궁합이 굉장히 좋다!

	// CreateIoCompletionPort
	// GetQueuedCompletionStatus 기본적으로 Thread - Safe 한 함수

	vector<Session*> sessionManager;

	// CP 생성
	HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// WorkerThreads
	for (int32 i = 0; i < 5; i++)
		GThreadManager->Launch([=]() { WorkerThreadMain(iocpHandle); });

	// Main Thread = Accept 담당
	while (true)
	{
		SOCKADDR_IN clientAddr;
		int32 addrLen = sizeof(clientAddr);

		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
			return 0;

		Session* session = xnew<Session>();
		session->socket = clientSocket;
		sessionManager.push_back(session);

		cout << "Client Connected !" << endl;

		// 소켓을 CP에 등록
		::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*Key*/(ULONG_PTR)session, 0);

		WSABUF wsaBuf;
		wsaBuf.buf = session->recvBuffer;
		wsaBuf.len = BUFSIZE;

		OverlappedEx* overlappedEx = new OverlappedEx();
		overlappedEx->type = IO_TYPE::READ;

		// ADD_REF
		DWORD recvLen = 0;
		DWORD flags = 0;
		::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
		// Recv를 걸어서 비동기 실행하여 결과를 Queue에 등록하도록 호출

		/*
		실행 흐름
		accept -> 새로운 클라 등록, 아니면 blocking 으로 대기 상태
		클라 들어오면 cp에 소켓을 등록하고
		recv를 한번 호출해서 Queue에 실행 결과를 받도록 함
		비동기로 실행한 WSARecv가 완료되면 Queue에 등록 됨
		-> Queue가 채워지길 기다리던 쓰레드가 데이터를 받고 흐름 진행
		-> 다시 Recv를 하여 Queue 를 채울 수 있도록 함 (다른 쓰레드가 결과를 받을 수 도 있음)

		이 흐름에서 Session이 접속 종료하면(delete), 해당 흐름을 진행 할 수 없도록 막아야함, 유효하지 않은 메모리 접근이기 때문
		*/

		// 유저가 게임 접속 종료!
		//Session* s = sessionManager.back();
		//sessionManager.pop_back();
		//xdelete(s);

		//::closesocket(session.socket);
		//::WSACloseEvent(wsaEvent);
	}

	GThreadManager->Join();

	// 윈속 종료
	::WSACleanup();
}