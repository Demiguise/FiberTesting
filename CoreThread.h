
#ifndef __CCORETHREAD_H__
#define __CCORETHREAD_H__

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "MemoryManager.h"

class CFiber;

struct SThreadInfo
{
	SThreadInfo() 
		: m_threadID(0)
		, m_hHeap(0)
		, m_pStartingFiber(NULL) {}
	DWORD m_threadID;
	HANDLE m_hHeap;
	CFiber* m_pStartingFiber;
};

class CCoreThread
{
public:
	CCoreThread(SThreadInfo* pThreadInfo);
	virtual ~CCoreThread();
	LPDWORD GetID();
	static DWORD WINAPI Run(LPVOID lpThreadParameter);

	template <typename T>
	static T* Allocate(EMemTags tags)
	{
		void* pMem = g_pMemoryManager->Allocate(GetCurrentThreadId(), sizeof(T), tags);
		return new (pMem) T();
	}
	
	template <typename T>
	static T* AllocateArray(size_t numElements, EMemTags tags)
	{
		void* pMem = g_pMemoryManager->AllocateArray(GetCurrentThreadId(), sizeof(T) * numElements, tags);
		return new (pMem) T[numElements];
	}

	template <typename T>
	static void Deallocate(T* obj)
	{
		obj->~T();
		g_pMemoryManager->Deallocate(GetCurrentThreadId(), obj);
	}
	static void DeallocateTag(EMemTags tag);
private:
	DWORD m_tlsIndex;
	CFiber* m_pInitialFiber;
};

#endif //~__CCORETHREAD_H__
