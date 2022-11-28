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

	// 1. ������ �и�
	// 2. Count üũ ��ȥ�ڸ� ����
	// 3. 
	void TryDelete(Node* oldHead)
	{
		if (_popCount == 1)
		{
			// pop count �� 1 �̶�� head�� �����ص� ����x
			// CAS ������ pop count�� ������Ű��,
			// head�� �̹� �ٲ�ġ�� ����.
			// ��, ������ head�� �����ϴ� ��찡 ���� �帧 ���� ���ٴ� ��
			// �ϴ±迡 ����� ���鵵 ����
			Node* node = _pendingList.exchange(nullptr); // ������ �� ��ȯ, ������ �и�

			if (--_popCount == 0) // ������ �и��ϴ� ���� ���� �����ߴ���?
			{
				// ���� pop�� ���� ������ ������
				DeleteNodes(node);
			}
			else if (node)
			{
				// ���� pop�� �ϰ� ������ �� �帧�� �����ϱ� ���, �ٽ� ����Ʈ ������
				ChainPendingNodeList(node);
			}

			delete oldHead;
		}
		else
		{
			// pop �� �帧�� ���� �����, node�� ���� head�� ������.
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