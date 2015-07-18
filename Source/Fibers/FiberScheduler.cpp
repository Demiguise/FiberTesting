#include "stdafx.h"
#include "FiberScheduler.h"
#include "CoreThread.h"
#include "Common/Spinlock.h"

extern CFiberScheduler* g_pFiberScheduler = 0;

CSpinlock enQueueLock;

CFiberScheduler::CFiberScheduler()
{
	for (int i = 0 ; i < k_maxFiberPool ; ++i)
	{
		m_fiberPool[i].Init(i, FIBER_STACK_SIZE);
	}
}

CFiber* CFiberScheduler::AcquireNextFiber(CFiber* pOldFiber)
{
	CFiber* boundFiber = NULL;

	// Check if there are any yielded fibers waiting with a 0 counter
	TAtomicFiberMap::iterator it = m_yieldedFibers.begin();
	while (it != m_yieldedFibers.end())
	{
		if (it->second->IsComplete())
		{
			boundFiber = it->first;
			m_yieldedFibers.erase(it);
			boundFiber->SetPrevious(pOldFiber);
			return boundFiber;
		}
		it++;
	}

	for (int i = 0 ; i < k_maxFiberPool ; ++i)
	{
		CFiber& fiber = m_fiberPool[i];
		if (fiber.AtomicStateSwitch(CFiber::eFS_InActive, CFiber::eFS_WaitingForJob))
		{
			boundFiber = &fiber;
			break;
		}
	}

	for (int i = 0 ; i < eFP_Num ; ++i)
	{
		CScopedLock lock(&enQueueLock);
		if (!m_jobQueue[i].empty())
		{
			SJobRequest job = m_jobQueue[i].front();
			boundFiber->Bind(job);
			m_jobQueue[i].pop();
			break;
		}
	}

	if (!boundFiber->InState(CFiber::eFS_Bound))
	{
		DebugLog("Fiber (%d) failed to bind.", boundFiber->GetID());
		return NULL;
	}

	boundFiber->SetPrevious(pOldFiber);

	return boundFiber;
}

void CFiberScheduler::StartJobs()
{
	for (int i = 0 ; i< k_maxRunningFibers ; ++i)
	{
		SThreadInfo& info = m_activeFibers[i].first;
		CFiber* pInitialFiber = AcquireNextFiber(NULL);
		pInitialFiber->AtomicStateSwitch(CFiber::eFS_InActive, CFiber::eFS_Bound);
		m_activeFibers[i].second = pInitialFiber;
		info.m_pStartingFiber = pInitialFiber;
		CCoreThread thread(&info);
	}
}

void CFiberScheduler::AllocateJobs()
{
	for (int i = 0 ; i < k_maxRunningFibers ; ++i)
	{
		if (CFiber* pFiber = m_activeFibers[i].second)
		{
			if (pFiber->InState(CFiber::eFS_Yielded) || pFiber->InState(CFiber::eFS_WaitingForJob))
			{
				if (CFiber* newFiber = AcquireNextFiber(pFiber))
				{
					TTimerMap::iterator it = m_activeTimers.find(pFiber);
					if (it != m_activeTimers.end())
					{
						m_avgTimes.push_back(it->second.elapsedTime());
						m_activeTimers.erase(it);
					}

					pFiber->SetNextFiber(newFiber);
					Timer& newT = m_activeTimers[newFiber];
					newT.startTimer();
					m_activeFibers[i].second = newFiber;
				}
				break;
			}
		}
	}
}

/*Static*/ void CFiberScheduler::Schedule(SJobRequest& job, EFiberPriority prio, CFiberJobData& data, CFiberCounter* pCounter /*= NULL */)
{
	job.m_jobData = data;
	job.m_pCounter = pCounter;
	CScopedLock lock(&enQueueLock);
	g_pFiberScheduler->m_jobQueue[prio].push(job);
}


void CFiberScheduler::FiberYield(CFiber* pFiber, CFiberCounter* pCounter)
{
	pFiber->SetState(CFiber::eFS_Yielded);
	m_yieldedFibers.insert(TAtomicFiberPair(pFiber, pCounter));
}

void CFiberScheduler::UpdateActiveFibers(CFiber* pFiber)
{
	DWORD curThreadId = GetCurrentThreadId();
	for (int i = 0 ; i < k_maxRunningFibers ; ++i)
	{
		if (m_activeFibers[i].first.m_threadID == curThreadId)
		{
			m_activeFibers[i].second = pFiber;
			break;
		}
	}
}

void CFiberScheduler::PrintAverageJobTime()
{
	float avg = 0.f;
	float highest = 0.f;
	float lowest = 100.f;
	const int numJobs = m_avgTimes.size();
	for (int i = 0 ; i < numJobs ; ++i)
	{
		float& time = m_avgTimes[i];
		avg += m_avgTimes[i];
		if (time > highest)
		{
			highest = time;
		}
		else if (time < lowest)
		{
			lowest = time;
		}
	}

	avg /= numJobs;
	DebugLog("JPS: %d | ATPJ: %f | (%f...%f) | Total:%f", (int)numJobs, avg, lowest, highest, (numJobs * avg) / k_maxRunningFibers);
	m_avgTimes.clear();
}