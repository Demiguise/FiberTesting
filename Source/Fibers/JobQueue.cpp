#include "stdafx.h"
#include "JobQueue.h"

CJobQueue::CJobQueue()
{
}


CJobQueue::~CJobQueue()
{
}

void CJobQueue::Enqueue(SJobRequest& job)
{
	CScopedLock lock(&m_enqueueLock);
	m_queue.push(job);
}

SJobRequest CJobQueue::Dequeue()
{
	CScopedLock lock(&m_dequeueLock);
	SJobRequest topJob = m_queue.front();
	m_queue.pop();
	return topJob;
}