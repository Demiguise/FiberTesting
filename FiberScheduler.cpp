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
	for (int i = 0 ; i < m_maxFiberPool ; ++i)
	{
		m_fiberPool[i].Init(i, FIBER_STACK_SIZE);
	}
	memset(&m_activeFibers, 0, sizeof(CFiber*) * 4);
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
	for (int i = 0 ; i < m_maxFiberPool ; ++i)
	{
		CFiber& fiber = m_fiberPool[i];
		if (fiber.GetState() == CFiber::eFS_InActive)
		{
			boundFiber = &fiber;
			break;
		}
	}

	if (!boundFiber)
	{
		return NULL;
	}

	for (int i = 0 ; i < eFP_Num ; ++i)
	{
		if (!m_jobQueue[i].Empty())
		{
			SJobRequest job = m_jobQueue[i].Dequeue();
			boundFiber->Bind(job);
			break;
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

void CFiberScheduler::StartJobs()
{
	for (int i = 0 ; i< m_maxRunningFibers ; ++i)
	{
		SThreadInfo& info = m_activeFibers[i].first;
		CFiber* pInitialFiber = GetNextFiber(NULL);
		pInitialFiber->SetState(CFiber::eFS_Bound);
		m_activeFibers[i].second = pInitialFiber;
		info.m_pStartingFiber = pInitialFiber;
		CCoreThread thread(&info);
	}
}

void CFiberScheduler::AllocateJobs()
{
	for (int i = 0 ; i < m_maxRunningFibers ; ++i)
	{
		CFiber* pFiber = m_activeFibers[i].second;
		if (pFiber && pFiber->GetState() == CFiber::eFS_WaitingForJob)
		{
			//Has finished, set new one.
			CFiber* newFiber = GetNextFiber(pFiber);
			pFiber->SetNextFiber(newFiber);
			m_activeFibers[i].second = newFiber;
		}
	}
}

/*Static*/ void CFiberScheduler::Schedule(SJobRequest& job, EFiberPriority prio, SFiberCounter* pCounter /*= NULL */, void* data /*= NULL */)
{
	job.m_pData = data;
	job.m_pCounter = pCounter;
	g_pFiberScheduler->m_jobQueue[prio].Enqueue(job);
}

void CFiberScheduler::FiberYield(CFiber* fiber, SFiberCounter* counter)
{
	//First, find out which thread we're on.
	int fiberIndex = -1;
	DWORD curThreadId = GetCurrentThreadId();
	for (int i = 0 ; i < m_maxRunningFibers ; ++i)
	{
		if (m_activeFibers[i].first.m_threadID == curThreadId)
		{
			fiberIndex = i;
			break;
		}
	}
	
	if (fiberIndex == -1)
	{
		DebugBreak();
	}

	TAtomicFiberMap::iterator it = m_savedFibers.find(fiber);
	if (it == m_savedFibers.end())
	{
		m_savedFibers.insert(TAtomicFiberPair(fiber, counter));
	}
	fiber->SetState(CFiber::eFS_Yielded);
	CFiber* pNextFiber = GetNextFiber(NULL);
	m_activeFibers[fiberIndex].second = pNextFiber;
	SwitchToFiber(pNextFiber->Address());

	curThreadId = GetCurrentThreadId();
	for (int i = 0 ; i < m_maxRunningFibers ; ++i)
	{
		if (m_activeFibers[i].first.m_threadID == curThreadId)
		{
			fiberIndex = i;
			break;
		}
	}

	m_activeFibers[fiberIndex].second = pNextFiber;
	fiber->SetState(CFiber::eFS_Active);
}
