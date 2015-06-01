#include "stdafx.h"
#include "FiberScheduler.h"


extern CFiberScheduler* g_pFiberScheduler = 0;

CFiberScheduler::CFiberScheduler()
	: m_nextCounterID(0)
	, m_fiberIndex(0)
	, m_staleFiber(NULL)
{
	m_bHasJobs = true;
	// Initialise stack size of fiber pool
	for (int i = 0 ; i < MAX_FIBER_POOL ; ++i)
	{
		m_fiberPool[i].Init(i, FIBER_STACK_SIZE);
	}
}

CFiberScheduler::~CFiberScheduler()
{
}

CFiber* CFiberScheduler::GetNextFiber(CFiber* terminatingFiber)
{
	// Calling this implicitly means that a fiber has finished or that one is yielding.

	CFiber* boundFiber = NULL;

	// Check if there are any yielded fibers waiting with a 0 counter
	TAtomicFiberMap::iterator it = m_savedFibers.begin();
	while (it != m_savedFibers.end())
	{
		if (it->second->IsComplete())
		{
			boundFiber = it->first;
			m_savedFibers.erase(it);
			return boundFiber;
		}
		it++;
	}

	// Find the first inactive fiber, will loop while waiting to find one
	// This is bad as the finished fiber is sat and waiting for a new job.
	// Think about bridgeFibers that literally do nothing but end?
	while(!boundFiber)
	{
		for (int i = 0 ; i < MAX_FIBER_POOL ; ++i)
		{
			CFiber& fiber = m_fiberPool[i];
			if (fiber.GetState() == CFiber::eFS_InActive)
			{
				fiber.SetState(CFiber::eFS_WaitingForJob);
				boundFiber = &fiber;
				break;
			}
		}
	}

	while(boundFiber->GetState() != CFiber::eFS_Bound)
	{
		for (int i = 0 ; i < eFP_Num ; ++i)
		{
			//Lock while we sort out stuff
			if (!m_jobQueue[i].Empty())
			{
				SJobRequest job = m_jobQueue[i].Dequeue();
				boundFiber->Bind(job);
				break;
			}
		}
	}

	boundFiber->SetPrevious(terminatingFiber);

	//Temp shite
	for (int i = 0 ; i < eFP_Num ; ++i)
	{
		if (!m_jobQueue[i].Empty())
		{
			m_bHasJobs = true;
		}
	}

	return boundFiber;
}

/*Static*/ void CFiberScheduler::Schedule(SJobRequest& job, EFiberPriority prio, SFiberCounter* pCounter /*= NULL */, void* data /*= NULL */)
{
	job.m_pData = data;
	job.m_pCounter = pCounter;
	g_pFiberScheduler->m_jobQueue[prio].Enqueue(job);
}

void CFiberScheduler::FiberYield(CFiber* fiber, SFiberCounter* counter)
{
	TAtomicFiberMap::iterator it = m_savedFibers.find(fiber);
	if (it == m_savedFibers.end())
	{
		m_savedFibers.insert(TAtomicFiberPair(fiber, counter));
	}
}
