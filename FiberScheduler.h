#ifndef __CFIBERSCHEDULER_H__
#define __CFIBERSCHEDULER_H__

#include <queue>
#include <map>
#include <string>

#include "Fiber.h"
#include "CoreThread.h"

#define FIBER_STACK_SIZE 0

enum EFiberPriority
{
	eFP_High = 0,
	eFP_Normal,
	eFP_Low,
	eFP_Num
};

class CFiberScheduler
{
public:
	CFiberScheduler();
	~CFiberScheduler() 
	{
	}
	
	CFiber* AcquireNextFiber(CFiber* pOldFiber);
	static void Schedule(SJobRequest& job, EFiberPriority prio, CFiberCounter* pCounter = NULL, void* pVData = NULL);
	void FiberYield(CFiber* pFiber, CFiberCounter* pCounter);

	void StartJobs();
	void AllocateJobs();
	bool IsActive()
	{
		for (int i = 0 ; i < k_maxRunningFibers; ++i)
		{
			CFiber::EFiberState state = m_activeFibers[i].second->GetState();
			if ((state == CFiber::eFS_Active)				|| 
					(state == CFiber::eFS_HasNextFiber) ||
					(state == CFiber::eFS_Yielded)			||
					(state == CFiber::eFS_Finished))
			{
				return true;
			}
		}
		for (int i = 0 ; i < eFP_Num ; ++i)
		{
			if (!m_jobQueue[i].empty())
			{
				return true;
			}
		}
		return false;
	}

private:
	typedef std::pair<SThreadInfo, CFiber*> TActiveFibers;
	typedef std::map<CFiber*, CFiberCounter*> TAtomicFiberMap;
	typedef std::pair<CFiber*, CFiberCounter*> TAtomicFiberPair;

	void UpdateActiveFibers(CFiber* pFiber);

	const static UINT16 k_maxFiberPool = 32;
	const static UINT16 k_maxRunningFibers = 1;

	CFiber m_fiberPool[k_maxFiberPool];
	TActiveFibers m_activeFibers[k_maxRunningFibers];
	TAtomicFiberMap m_yieldedFibers;
	std::queue<SJobRequest> m_jobQueue[eFP_Num];
};

extern CFiberScheduler* g_pFiberScheduler;

#endif //~__CFIBERSCHEDULER_H__
