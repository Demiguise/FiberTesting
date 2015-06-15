// ThreadFiberTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>
#include <stack>
#include <vector>
#include <atomic>
#include <random>
#include <time.h>

#include "MemoryManager.h"
#include "Fiber.h"
#include "FiberScheduler.h"
#include "CoreThread.h"
#include "Timer.h"

// Notes
//	(Q) Threads, how do we make one?
//	(Q) Fibers, how do we make one without converting FROM a thread? Can we? 
//		# If no, that's crap. We still incur penalties from creating one that we might avoid elsewise.
//	!A We need to convert a thread to fiber in order to actually schedule/deal with the fibers themselves.
//		# The initial thread is the beginning of the daisy chain that we use for fibers.
//	(Q) Thread/Fiber local storage (TLS/FLS), how do we manage the allocations? Do I need it?
//		# See 'The Last of Us' GDC video on the subject for more information?
//	(Q) How do heaps work? 
//	(Q) How should the atomic counters be spawned and decremented?
//		# How does Fiber (F) decrement the atomic counter Fiber (A) is waiting on?
//		# Grab it via an ID? StringCRC? Inject it into the new fiber? Injecting would mean that each of the child fibers/dependanct fibers need to be in roughly the same place?
//		# If I go with injecting, that's okish I suppose. Just means that the structure of the fibers has to be quite crrrrrrrrrrazily clear.
//		# Would an Audio fiber want to be spawned by some other fiber that's waiting on information about the footstep of a character? Is that even a problem I have right now? (No it isn't)
//		# Advantages of the injection would be substantionally easier code to follow, everything would be parallised but somewhat sequential?

std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> uni(1000, 5000);
std::uniform_int_distribution<int> uni2(0, 100000);

void __stdcall YieldingTestFunc2(LPVOID lpParam)
{
	SJobRequest* pJobInfo = static_cast<CFiber*>(GetFiberData())->GetJobInfo();
	srand (time(NULL));
	CFiber::Log("Started work on decrementing the yielded counter!");
	int* pInt = (int*) pJobInfo->m_pData;
	*pInt = uni2(rng);
	CFiber::Log("Set int to %i", *(pInt));
}

void __stdcall YieldingTestFunc(LPVOID lpParam)
{
	CFiber::Log("Started work on Yielding!");
	CFiberCounter yieldCounter(2, __FUNCTION__);

	int test1 = 0;
	int test2 = 0;

	CREATEJOB(yieldFunc, YieldingTestFunc2);
	CFiberScheduler::Schedule(yieldFunc, eFP_High, &yieldCounter, &test1);
	CFiberScheduler::Schedule(yieldFunc, eFP_High, &yieldCounter, &test2);;

	CFiber::YieldForCounter(&yieldCounter);

	CFiber::Log("Completed work on Yielding!");
	CFiber::Log("Int values are [%i, %i]", test1, test2);
}

void __stdcall WaitTest(LPVOID lpParam)
{
	auto random_int = uni(rng);
	CFiber::Log("Sleeping for %i ms", random_int);
	Sleep(random_int);
	CFiber::Log("Woke up!");
}

int _tmain(int argc, _TCHAR* argv[])
{
	g_pMemoryManager = new CMemoryManager();
	g_pFiberScheduler = new CFiberScheduler();

	CREATEJOB(yieldJob, YieldingTestFunc);
	CREATEJOB(waitJob, WaitTest);

	for (int i = 0 ; i < 40 ; i++)
	{
		CFiberScheduler::Schedule(yieldJob, eFP_Normal);
	};

	g_pFiberScheduler->StartJobs();

	Timer scheduleTimer;
	scheduleTimer.startTimer();

	while (true)
	{
		if (scheduleTimer.elapsedTime() > 0.5f)
		{
			g_pFiberScheduler->AllocateJobs();
		}
	}

	return 0;
}
