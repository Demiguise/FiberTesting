#include "stdafx.h"
#include "CoreThread.h"

#include "Fiber.h"
#include "FiberScheduler.h"
#include <thread>

#define MAX_THREAD_HEAP_SIZE 0

CCoreThread::CCoreThread()
{
	DebugLog("Spooling up a thread...");
	SThreadInfo info;
	if(HANDLE threadHandle = CreateThread(NULL, 0, Run, &info, 0, info.m_threadID))
	{

	}
	else
	{
		DebugLog("		Failure!");
	}
}


CCoreThread::~CCoreThread()
{
}

/*Static*/ DWORD WINAPI CCoreThread::Run(LPVOID lpThreadParameter)
{
	SThreadInfo* info = (SThreadInfo*)lpThreadParameter;
	DebugLog("		ID: %d", info->m_threadID);
	ConvertThreadToFiber(0);
	CFiber* next = g_pFiberScheduler->GetNextFiber(NULL);
	while (true)
	{
		SwitchToFiber(next->Address());
	}
	return 0;
}

/*Static*/ void CCoreThread::DeallocateTag(EMemTags tag)
{
	g_pMemoryManager->DeallocateTag(tag);
}