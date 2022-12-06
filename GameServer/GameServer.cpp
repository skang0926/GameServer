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
#include "Memory.h"
#include "MemoryPool.h"

class Knight
{
public:
	int32 _hp = rand() % 1000;
};

int main()
{
	for (int32 i = 0; i < 3; i++)
	{
		GThreadManager->Launch([]()
		{
			while (true)
			{
				Knight* k = xnew<Knight>();

				cout << k->_hp << endl;

				this_thread::sleep_for(10ms);

				xdelete(k);
			}
		});
	}

	GThreadManager->Join();
}
