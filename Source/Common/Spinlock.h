#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <atomic>

class CSpinlock
{
public:
	CSpinlock();
	~CSpinlock();

	void Lock();
	void Unlock();

private:
	std::atomic_bool m_lock;
};

class CScopedLock
{
public:
	CScopedLock(CSpinlock* lock)
	{
		if (lock)
		{
			lock->Lock();
			m_lock = lock;
		}
	}
	~CScopedLock()
	{
		if (m_lock)
		{
			m_lock->Unlock();
			m_lock = NULL;
		}
	}

	CSpinlock* m_lock;
};

#endif //~__SPINLOCK_H__
