#ifndef __JOBQUEUE_H__
#define __JOBQUEUE_H__

#include "Common/Spinlock.h"
#include "Fiber.h"
#include <queue>

class CJobQueue
{
public:
	CJobQueue();
	~CJobQueue();

	void Enqueue(SJobRequest& job);
	SJobRequest Dequeue();
	bool Empty() { return m_queue.empty(); }

private:
	CSpinlock m_dequeueLock;
	CSpinlock m_enqueueLock;
	typedef std::queue<SJobRequest> TJobRequests;
	TJobRequests m_queue;
};

#endif //~__JOBQUEUE_H__
