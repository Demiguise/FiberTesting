#ifndef __CFIBERSCHEDULER_H__
#define __CFIBERSCHEDULER_H__

#include <list>
#include <queue>
#include <vector>
#include <map>
#include <atomic>
#include <string>
#include <Windows.h>
#include <mutex>

#include "Fiber.h"
#include "JobQueue.h"

#define MAX_FIBER_POOL 32
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
	~CFiberScheduler();
	
	CFiber* GetNextFiber(CFiber* terminatingFiber);
	static void Schedule(SJobRequest& job, EFiberPriority prio, SFiberCounter* pCounter = NULL, void* data = NULL);
	void FiberYield(CFiber* fiber, SFiberCounter* counter);

	bool HasJobs() { return m_bHasJobs; }

private:
	typedef std::map<CFiber*, SFiberCounter*> TAtomicFiberMap;
	typedef std::pair<CFiber*, SFiberCounter*> TAtomicFiberPair;
	typedef std::vector<CFiber*> TOldFibers;

	CFiber m_fiberPool[32];
	TAtomicFiberMap m_savedFibers;
	TOldFibers m_oldFibers;
	CJobQueue m_jobQueue[eFP_Num];
	UINT64 m_nextID;
	UINT64 m_nextCounterID;
	UINT16 m_fiberIndex;
	CFiber* m_staleFiber;

	bool m_bHasJobs;

	std::mutex m_fiberLock;
};

extern CFiberScheduler* g_pFiberScheduler;

#endif //~__CFIBERSCHEDULER_H__
