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

class Player
{

};

class Knight : public Player
{
public:
	int32 hp;
	int32 mp;
};

int main()
{
	Map<int32, Knight> m;
	m[100] = Knight();
}
