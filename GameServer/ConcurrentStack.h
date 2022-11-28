#pragma once

#include <mutex>

template<typename T>
class LockStack
{
public:
	LockStack() {}
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_stack.empty())
			return false;

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		unique_lock<_mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _stack.empty() == false; });
		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};

template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) : data(value), next(nullptr)
		{

		}

		T data;
		Node* next;
	};

public:
	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = _head;

		// _head = node;
		while (_head.compare_exchange_weak(node->next, node) == false)
		{

		}
	}

	bool TryPop(T& value)
	{
		++_popCount;

		Node* oldHead = _head;


		//_head = _head->next;
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{

		}

		if (oldHead == nullptr)
		{
			--_popCount;
			return false;
		}

		value = oldHead->data;
		TryDelete(oldHead);

		return true;
	}

private:

	// 1. 데이터 분리
	// 2. Count 체크 나혼자면 삭제
	// 3. 
	void TryDelete(Node* oldHead)
	{
		if (_popCount == 1)
		{
			// pop count 가 1 이라면 head를 제거해도 문제x
			// CAS 들어가기전 pop count를 증가시키고,
			// head는 이미 바꿔치기 했음.
			// 즉, 지금의 head를 참조하는 경우가 지금 흐름 말고 없다는 것
			// 하는김에 예약된 노드들도 삭제
			Node* node = _pendingList.exchange(nullptr); // 원자적 값 교환, 데이터 분리

			if (--_popCount == 0) // 데이터 분리하는 순간 누가 진입했는지?
			{
				// 누가 pop을 진행 중이지 않을때
				DeleteNodes(node);
			}
			else if (node)
			{
				// 누가 pop을 하고 있으니 그 흐름이 삭제하길 기대, 다시 리스트 돌려줌
				ChainPendingNodeList(node);
			}

			delete oldHead;
		}
		else
		{
			// pop 의 흐름을 누가 사용중, node를 누가 head로 참조중.
			ChainPendingNode(oldHead);
			--_popCount;
		}
	}

	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = _pendingList;

		// _pendingList = first;
		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{

		}
	}

	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		while (last->next)
			last = last->next;

		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	static void DeleteNodes(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

private:
	atomic<Node*> _head;

	atomic<uint32> _popCount = 0;
	atomic<Node*> _pendingList;
};