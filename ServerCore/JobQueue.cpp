#include "pch.h"
#include "JobQueue.h"

/*---------------
	JobQueue
----------------*/

void JobQueue::Push(JobRef&& job)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // WRITE_LOCK

	// 첫번째 Job을 넣은 쓰레드가 실행까지 담당
	if (prevCount == 0)
	{
		Execute();
	}
}

// 1) 일감이 너무 몰리면?
// 2) DoAsync 타고 타고 가서 절대 끝나지 않는 상황 (일감이 한 쓰레드한테 몰림)
// 3) 쓰레드 여러개가 Job을 꺼내가서 작업하는게 멀티쓰레드의 장점인데 하나의 쓰레드가 모든 일을 다 하는 형태
// 4) 이를 담당하는 JobQueue 의 Owner가 유저라면, 그 유저는 엄청난 렉이 걸릴 수 있음
void JobQueue::Execute()
{
	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		// 남은 일감이 0개라면 종료
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			return;
		}
	}
}
