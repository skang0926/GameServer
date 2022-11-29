﻿#include "pch.h"
#include <iostream>

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"

LockFreeQueue<int32> q;
LockFreeStack<int32> s;

void Push()
{
	while (true)
	{
		int32 value = rand() % 100;
		q.Push(value);

		this_thread::sleep_for(10ms);
	}
}

void Pop()
{
	while (true)
	{
		auto data = q.TryPop();
		if (data != nullptr)
		{
			cout << (*data) << endl;
		}
	}
}

int main()
{
	thread t1(Push);
	thread t2(Pop);
	thread t3(Pop);

	t1.join();
	t2.join();
	t2.join();
	
}
