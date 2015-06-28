#include "stdafx.h"
#include "MemoryManager.h"

#include <algorithm>	

extern CMemoryManager* g_pMemoryManager = 0;

// Max size is currently set to 2mb
CMemoryManager::CMemoryManager()
	: m_maxSize(8)
{
}


CMemoryManager::~CMemoryManager()
{
}

void* CMemoryManager::Allocate(DWORD threadID, size_t size, EMemTags tag)
{
	HANDLE hHeap = GetThreadHeap(threadID);

	//Figure out if we can use some other memory
	TMemBlocks::iterator itBlocks = m_memBlocks.begin();
	for ( ; itBlocks != m_memBlocks.end() ; ++itBlocks)
	{
		if ((itBlocks->m_tag == tag) && ((itBlocks->m_memUsed - m_maxSize) >= size))
		{
			return itBlocks->Allocate(size);
		}
	}

	//If not, allocate new block on the heap and return a pointer to it.
	m_memBlocks.emplace_back(hHeap, m_maxSize, tag);
	return m_memBlocks.back().Allocate(size);
}

void* CMemoryManager::AllocateArray(DWORD threadID, size_t size, EMemTags tag)
{
	HANDLE hHeap = GetThreadHeap(threadID);

	int numBlocks = 0;
	size_t memLeft = size;

	while (memLeft != 0)
	{
		m_memBlocks.emplace_back(hHeap, m_maxSize, tag);
		size_t memToAlloc = (memLeft > m_maxSize) ? m_maxSize : memLeft;
		m_memBlocks.back().Allocate(memToAlloc);
		memLeft -= memToAlloc;
		++numBlocks;
	}

	return m_memBlocks[m_memBlocks.size() - numBlocks].m_pStart;
}

void CMemoryManager::Deallocate(DWORD threadID, void* pMem)
{

}

void CMemoryManager::DeallocateTag(EMemTags tag)
{
	m_memBlocks.erase (std::remove_if(m_memBlocks.begin(), m_memBlocks.end(), [tag](CMemBlock& block) { block.Free(); return block.m_tag == tag; }));
}

HANDLE CMemoryManager::GetThreadHeap(DWORD threadID)
{
	TThreadHeaps::iterator itHeap = m_threadHeaps.find(threadID);
	if (itHeap == m_threadHeaps.end())
	{
		m_threadHeaps[threadID] = HeapCreate(0, 0, 0);
	}
	return m_threadHeaps[threadID];
}