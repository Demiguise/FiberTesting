#include "stdafx.h"
#include "CoreThread.h"

#include "Fiber.h"
#include "FiberScheduler.h"
#include <thread>

#define MAX_THREAD_HEAP_SIZE 0

CCoreThread::CCoreThread(SThreadInfo* pThreadInfo)
{
	DebugLog("Spooling up a thread...");
	if(HANDLE threadHandle = CreateThread(NULL, 0, Run, pThreadInfo, 0, &pThreadInfo->m_threadID))
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
	if (info->m_pStartingFiber)
	{
		SwitchToFiber(info->m_pStartingFiber->Address());
	}
	else
	{
		DebugLog("		No starting fiber. Sad panda");
	}

	return 0;
}

/*Static*/ void CCoreThread::DeallocateTag(EMemTags tag)
{
	g_pMemoryManager->DeallocateTag(tag);
}