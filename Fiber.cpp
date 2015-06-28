#include "stdafx.h"
#include "Fiber.h"
#include "FiberScheduler.h"
#include "ScopedTimer.h"

static const char* FiberStateStrings[] =
{
	"InActive",
	"WaitingForJob",
	"Bound",
	"Active",
	"Finished",
	"Releasing",
	"Yielded"
};

CFiber::CFiber(size_t stack)
	: m_pFiber(NULL)
	, m_pPrevFiber(NULL)
	, m_pNextFiber(NULL)
	, m_stackSize(stack)
	, m_state(eFS_InActive)
{
}

CFiber::CFiber()
	: m_pFiber(NULL)
	, m_pPrevFiber(NULL)
	, m_pNextFiber(NULL)
	, m_stackSize(0)
	, m_state(eFS_InActive)
{
}

void CFiber::Init(UINT16 id, size_t stackSize)
{
	m_id = id;
	m_stackSize = stackSize;
}

void CFiber::Bind(SJobRequest& job)
{
	assert(!m_pFiber);
	
	if (m_pFiber = CreateFiber(m_stackSize, CFiber::Run, this))
	{
		m_job = job;
		SetState(eFS_Bound);
	}
	else
	{
		Log("CreateFiber error (%d)\n", GetLastError());
	}
}

void CFiber::Release()
{
	assert(GetState() == eFS_Finished);
	if (m_pFiber)
	{
		DeleteFiber(m_pFiber);
		m_pFiber = NULL;
		SetState(eFS_InActive);
	}
}

void CFiber::SetState(EFiberState newState)
{
	if (m_state != newState)
	{
		m_state = newState;
	}
}

/*Static*/ void __stdcall CFiber::Run(LPVOID lpVData)
{
	CFiber* pFiber = (CFiber*)GetFiberData();
	pFiber->SetState(eFS_Active);
	pFiber->ReleasePrevious();

#if FIBER_ENABLE_DEBUG
	pFiber->SetThreadName(pFiber->m_job.m_jobName.c_str());
#endif

	pFiber->m_job.m_pFunc(lpVData);
	pFiber->EndJob();
}

void CFiber::ReleasePrevious()
{
	PERFTIMER_FUNC();
	m_pNextFiber = NULL;

	if (m_pPrevFiber)
	{
		if (m_pPrevFiber->GetState() == eFS_Finished)
		{
			m_pPrevFiber->Release();
			m_pPrevFiber = NULL;
		}
		else if (m_pPrevFiber->GetState() != eFS_Yielded)
		{
			Log("Last fiber hasn't finished yet?");
			DebugBreak();
		}
	}
}

/*Static*/ void CFiber::YieldForCounter(CFiberCounter* counter)
{
	CFiber* pFiber = (CFiber*)GetFiberData();
	g_pFiberScheduler->FiberYield(pFiber, counter);

	assert(!pFiber->m_pNextFiber);

	while (!pFiber->m_pNextFiber)
	{
	}

	SwitchToFiber(pFiber->m_pNextFiber->Address());

	pFiber->ReleasePrevious();
	pFiber->SetState(CFiber::eFS_Active);
}

void CFiber::SetNextFiber(CFiber* nextFiber)
{
	if (!m_pNextFiber)
	{
		if (m_state != eFS_Yielded)
		{
			SetState(eFS_HasNextFiber);
		}
		m_pNextFiber = nextFiber;
	}
	else
	{
		DebugBreak();
	}
}

void CFiber::EndJob()
{
	PERFTIMER_FUNC();
	if (CFiberCounter* pCounter = m_job.m_pCounter)
	{
		pCounter->DecrementCounter();
	}

	assert(!m_pNextFiber);

	SetState(eFS_WaitingForJob);

	while (!m_pNextFiber)
	{
	}

	SetState(eFS_Finished);

	SwitchToFiber(m_pNextFiber->Address());
}



/*Static*/ void CFiber::Log(std::string frmt, ...)
{
	if (CFiber* pFiber = (CFiber*)GetFiberData())
	{
		char buffer[512];
		sprintf_s(buffer, "[F:%u] ", pFiber->GetID());

		frmt.insert(0, buffer);
		if (!pFiber->m_personalLog.empty())
		{
			float timeFromLast = Timer::GetTimeBetween(pFiber->m_personalLog.back().timeStamp, Timer::GetCountNow());
			sprintf_s(buffer, " [%f]", timeFromLast);
			frmt.insert(frmt.length(), buffer);
		}
		frmt.append("\n");
		va_list args;
		va_start(args, frmt);
		vsnprintf_s(buffer, 512, frmt.c_str(), args);
		va_end(args);
#if FIBER_OUTPUT_ON
		PushToLogs(buffer);
#endif //~FIBER_OUTPUT_ON

		pFiber->m_personalLog.push_back(PersonalLogEntry(GetCurrentThreadId(), buffer));
	}
}

typedef struct tagTHREADNAME_INFO
{ 
	// I think I copied this from an MSDN article somewhere
  DWORD dwType; // must be 0x1000
  LPCSTR szName; // pointer to name (in user addr space)
  DWORD dwThreadID; // thread ID (-1=caller thread)
  DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

#if FIBER_ENABLE_DEBUG
/*Static*/ void CFiber::SetThreadName(LPCSTR name)
{
	static const DWORD MS_VC_EXCEPTION=0x406D1388;
	THREADNAME_INFO info;
	{
		info.dwType = 0x1000;
		info.szName = name;
		info.dwThreadID = -1;
		info.dwFlags = 0;
	}
	__try
	{
		RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#endif
