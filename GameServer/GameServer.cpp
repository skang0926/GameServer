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

class Player
{

};

class Knight : public Player
{
public:
	Knight() : hp(0), mp(0)
	{
		cout << "Knight()" << endl;
	}

	~Knight()
	{
		cout << "~Knight()" << endl;
	}

	int32 hp;
	int32 mp;
};

int main()
{
	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([]()
		{
			while (true)
			{
				Vector<Knight> v(100);

				Map<int32, Knight> m;
				m[100] = Knight();

				this_thread::sleep_for(10ms);
			}
		});
	}

	GThreadManager->Join();
}
