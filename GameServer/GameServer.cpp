#include "pch.h"
#include <iostream>

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"

#include "AccountManager.h"
#include "PlayerManager.h"

int main()
{
	GThreadManager->Launch([=]
	{
		while (true)
		{
			cout << "PlayerThenAccount" << endl;
			GPlayerManager.PlayerThenAccount();
			this_thread::sleep_for(1s);
		}
	});

	GThreadManager->Launch([=]
	{
		while (true)
		{
			cout << "AccountThenPlayer" << endl;
			GAccountManager.AccountThenPlayer();
			this_thread::sleep_for(1s);
		}
	});


	GThreadManager->Join();
}
