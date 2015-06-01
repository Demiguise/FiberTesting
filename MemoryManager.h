#ifndef __CMEMORYMANAGER_H__
#define __CMEMORYMANAGER_H__

#include <vector>
#include <map>

enum EMemTags
{
	eMT_Tag1,
	eMT_Tag2,
};

class CMemBlock
{
public:
	CMemBlock(HANDLE hHeap, size_t max, EMemTags tag) 
		: m_pStart(NULL)
		, m_memUsed(0)
		, m_hHeap(hHeap)
		, m_tag(tag)
	{
		m_pStart = HeapAlloc(hHeap, 0, max);
		//memset(m_pStart, 0, max);
	}

	~CMemBlock()
	{
	}

	void* Allocate(size_t size)
	{
		void* pMem = (char*)m_pStart + m_memUsed;
		m_memUsed += size;
		return pMem;
	}

	void Free()
	{
		HeapFree(m_hHeap, 0, m_pStart);
	}

	void* m_pStart;
	size_t m_memUsed;
	HANDLE m_hHeap;
	EMemTags m_tag;
};

class CMemoryManager
{
public:
	CMemoryManager();
	~CMemoryManager();

	void* Allocate(DWORD threadID, size_t size, EMemTags tag);
	void* AllocateArray(DWORD threadID, size_t size, EMemTags tag);
	void Deallocate(DWORD threadID, void* pMem);
	void DeallocateTag(EMemTags tag);
private:

	HANDLE GetThreadHeap(DWORD threadID);

	const size_t m_maxSize;
	typedef std::vector<CMemBlock> TMemBlocks;
	typedef std::map<DWORD, HANDLE> TThreadHeaps;
	TThreadHeaps m_threadHeaps;
	TMemBlocks m_memBlocks;
};

// Overriding "New" operators?
namespace
{
#ifdef USE_CUSTOM_MEM_ALLOCATION
	void* operator new(size_t size)
	{
		DebugLog("Lol");
		return NULL;
	}

	void* operator new[](size_t size)
	{
		return NULL;
	}

	void operator delete(void* p)
	{

	}

	void operator delete[](void* p)
	{

	}
#endif
}

extern CMemoryManager* g_pMemoryManager;

#endif //~__CMEMORYMANAGER_H__
