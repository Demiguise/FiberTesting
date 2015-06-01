#include "stdafx.h"
#include "Spinlock.h"


CSpinlock::CSpinlock()
{
	m_lock.store(false);
}


CSpinlock::~CSpinlock()
{
}

void CSpinlock::Lock()
{
	bool state = m_lock.load();
	while(m_lock.compare_exchange_weak(state, true))
	{
	}
}

void CSpinlock::Unlock()
{
	m_lock.store(false);
}