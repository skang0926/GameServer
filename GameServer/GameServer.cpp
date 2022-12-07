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

#include "TypeCast.h"

using TL = TypeList<class Player, class Mage, class Knight, class Archer>;

class Player
{
public:
	Player()
	{
		INIT_TL(Player);
	}
	virtual ~Player() { }

	DECLARE_TL;
};

class Knight : public Player
{
public:
	Knight() { INIT_TL(Knight) }
};

class Mage : public Player
{

public:
	Mage() { INIT_TL(Mage) }
};

class Archer : public Player
{

public:
	Archer() { INIT_TL(Archer) }
};

class Dog
{

};

int main()
{
	TypeConversion<TL> t;
	t.s_convert[0][0];

	{
		Player* player = new Knight();
		bool canCast = CanCast<Archer*>(player);
		Knight* knight = TypeCast<Knight*>(player);
	}

	{
		shared_ptr<Player> player = MakeShared<Knight>();

		shared_ptr<Archer> archer = TypeCast<Archer>(player);
		bool canCast = CanCast<Mage>(player);

	}

	//{
	//	shared_ptr<Knight> player = MakeShared<Knight>();

	//	//shared_ptr<Archer> archer = TypeCast<Archer>(player);
	//	bool canCast = CanCast<Archer>(player);
	//}
	for (int32 i = 0; i < 3; i++)
	{
		GThreadManager->Launch([]()
		{
			while (true)
			{

			}
		});
	}

	GThreadManager->Join();
}
