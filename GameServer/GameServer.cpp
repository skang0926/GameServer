#include "pch.h"
#include <iostream>

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"

#include "SocketUtils.h"

#include "Listener.h"

int main()
{
	// TODO : 매니저 클래스 필요
	Listener listener;
	listener.StartAccept(NetAddress(L"127.0.0.1", 7777));

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
		{
			while (true)
			{
				GIocpCore.Dispatch();
			}
		});
	}

	GThreadManager->Join();
}