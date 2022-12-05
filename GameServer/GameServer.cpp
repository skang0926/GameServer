#include "pch.h"
#include <iostream>

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>
#include "CoreMacro.h"
#include "ThreadManager.h"
#include "memory.h"

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
	Player* p = xnew<Player>();

	Knight* k = static_cast<Knight*>(p);

	k->hp = 100;
}
