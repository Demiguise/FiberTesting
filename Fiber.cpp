#include "stdafx.h"
#include "Fiber.h"
#include "FiberScheduler.h"

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
	, m_pCounter(NULL)
	, m_pData(NULL)
	, m_stackSize(stack)
	, m_state(eFS_InActive)
{
}

CFiber::CFiber()
	: m_pFiber(NULL)
	, m_pPrevFiber(NULL)
	, m_pNextFiber(NULL)
	, m_pCounter(NULL)
	, m_pData(NULL)
	, m_stackSize(0)
	, m_state(eFS_InActive)
{
}

CFiber::~CFiber()
{
}

void CFiber::Init(UINT16 id, size_t stackSize)
{
	m_id = id;
	m_stackSize = stackSize;
}

bool CFiber::Bind(SJobRequest& job)
{
	if (m_pFiber)
	{
		DebugBreak();
		Log("Error: Attempting to bind an existing fiber!");
		return false;
	}

	m_pFiber = CreateFiber(m_stackSize, CFiber::Run, this);
	if (!m_pFiber)
	{
		Log("CreateFiber error (%d)\n", GetLastError());
		return false;
	}
	m_pData = job.m_pData;
	m_pCounter = job.m_pCounter;
	m_pFuncPointer = (LPFIBER_START_ROUTINE)job.m_pJob;
	m_funcName = job.m_jobName;
	job.m_pFiber = this;
	return true;
}

void CFiber::Release()
{
	assert(GetState() == eFS_Finished);
	if (m_pFiber)
	{
		DeleteFiber(m_pFiber);
		m_pData = NULL;
		m_pCounter = NULL;
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
	pFiber->SetThreadName(pFiber->m_funcName.c_str());
#endif

	pFiber->GetFunction()(lpVData);
	pFiber->EndJob();
}

void CFiber::ReleasePrevious()
{
	m_pNextFiber = NULL;

	//Release previous fiber
	if (m_pPrevFiber)
	{
		if (m_pPrevFiber->GetState() == eFS_Finished)
		{
			m_pPrevFiber->Release();
			m_pPrevFiber = NULL;
		}
		else
		{
			Log("Last fiber hasn't finished yet?");
			DebugBreak();
		}
	}
}

typedef struct tagTHREADNAME_INFO
{
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

/*Static*/ void CFiber::YieldForCounter(SFiberCounter* counter)
{
	CFiber* pFiber = (CFiber*)GetFiberData();
	Log("Yielding for function (%s) counter", counter->GetName());
	g_pFiberScheduler->FiberYield(pFiber, counter);
	Log("Finished yielding");
}

/*Static*/ void* CFiber::GetDataFromFiber()
{
	return ((CFiber*)GetFiberData())->GetData();
}

void CFiber::SetNextFiber(CFiber* nextFiber)
{
	SetState(eFS_Bound);
	if (!m_pNextFiber)
	{
		m_pNextFiber = nextFiber;
	}
	else
	{
		DebugBreak();
	}
}

void CFiber::EndJob()
{
	if (SFiberCounter* pCounter = GetCounter())
	{
		pCounter->DecrementCounter();
	}

	if (m_pNextFiber)
	{
		DebugBreak();
	}

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
		frmt.append("\n");
		va_list args;
		va_start(args, frmt);
		vsnprintf_s(buffer, 512, frmt.c_str(), args);
		va_end(args);
		PushToLogs(buffer);
#if FIBER_ENABLE_DEBUG
		pFiber->m_personalLog.push_back(PersonalLogEntry(GetCurrentThreadId(), buffer));
#endif
	}
}