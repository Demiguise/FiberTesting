#include "stdafx.h"
#include "FiberScheduler.h"
#include "CoreThread.h"

extern CFiberScheduler* g_pFiberScheduler = 0;

std::mutex enQueueLock;

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
		if (fiber.GetState() == CFiber::eFS_InActive)
		{
			boundFiber = &fiber;
			break;
		}
	}

	for (int i = 0 ; i < eFP_Num ; ++i)
	{
		if (!m_jobQueue[i].empty())
		{
			enQueueLock.lock();
			SJobRequest job = m_jobQueue[i].front();
			boundFiber->Bind(job);
			m_jobQueue[i].pop();
			enQueueLock.unlock();
			break;
		}
	}

	if (boundFiber->GetState() != CFiber::eFS_Bound)
	{
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
		pInitialFiber->SetState(CFiber::eFS_Bound);
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
			CFiber::EFiberState state = pFiber->GetState();
			switch (state)
			{
			case CFiber::eFS_Yielded:
			case CFiber::eFS_WaitingForJob:
				if (CFiber* newFiber = AcquireNextFiber(pFiber))
				{
					pFiber->SetNextFiber(newFiber);
					m_activeFibers[i].second = newFiber;
				}
				break;
			}
		}
	}
}

/*Static*/ void CFiberScheduler::Schedule(SJobRequest& job, EFiberPriority prio, CFiberCounter* pCounter /*= NULL */, void* pVData /*= NULL */)
{
	job.m_pData = pVData;
	job.m_pCounter = pCounter;
	enQueueLock.lock();
	g_pFiberScheduler->m_jobQueue[prio].push(job);
	enQueueLock.unlock();
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