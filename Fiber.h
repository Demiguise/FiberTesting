#ifndef __CFIBER_H__
#define __CFIBER_H__

#include <assert.h>
#include <Windows.h>
#include <atomic>
#include <string>
#include <vector>

#define FIBER_ENABLE_DEBUG 1

class CFiber;

class CFiberJobData
{
public:
	CFiberJobData() {}
	~CFiberJobData() {}
};

typedef std::atomic<int> TCounter;
class CFiberCounter
{
public:
	CFiberCounter(int initialCount, std::string funcName = "Default")
		: m_funcName(funcName)
	{
		m_counter.store(initialCount);
	}

	~CFiberCounter() {}

	bool IsComplete() const { return (m_counter.load() == 0); }
	void DecrementCounter() { m_counter--; }
	const char* GetName() const { return m_funcName.c_str(); }

private:
	CFiberCounter& operator= (CFiberCounter& rhs); //Forcing no copy operator
	std::string m_funcName;
	TCounter m_counter;
};


#define CREATEJOB(name, func) SJobRequest name((LPFIBER_START_ROUTINE)func, #func)

struct SJobRequest
{
	SJobRequest()
		: m_pFunc(NULL)
		, m_pData(NULL)
		, m_pCounter(NULL)
		, m_pFiber(NULL) {}

	SJobRequest(LPFIBER_START_ROUTINE job, std::string jobName)
		: m_jobName(jobName)
		, m_pFunc(job)
		, m_pData(NULL)
		, m_pCounter(NULL)
		, m_pFiber(NULL) {}

	SJobRequest& operator= (SJobRequest rhs)
	{
		m_jobName = rhs.m_jobName;
		m_pFunc = rhs.m_pFunc;
		m_pData = rhs.m_pData;
		m_pCounter = rhs.m_pCounter;
		m_pFiber = rhs.m_pFiber;
		return *this;
	}

	std::string m_jobName;
	LPFIBER_START_ROUTINE m_pFunc;
	void* m_pData;
	CFiberCounter* m_pCounter;
	CFiber* m_pFiber;
};

class CFiber
{
public:
	enum EFiberState
	{
		eFS_InActive = 0,
		eFS_Active,
		eFS_WaitingForJob,
		eFS_Bound,
		eFS_HasNextFiber,
		eFS_Finished,
		eFS_Releasing,
		eFS_Yielded
	};

	CFiber(LPFIBER_START_ROUTINE func, size_t stack = 0);
	CFiber(size_t stack);
	CFiber();
	~CFiber() 
	{
	}

	void Init(UINT16 id, size_t stack);
	bool Bind(SJobRequest& job);
	void Release();

	static void __stdcall Run(LPVOID lpParam);

	//In-Fiber functions
	void ReleasePrevious();
	static void YieldForCounter(CFiberCounter* counter);
	void SetNextFiber(CFiber* nextFiber);
	void EndJob();
	static void Log(std::string, ...);

	UINT16 GetID() const { return m_id; }
	void SetID(UINT16 id) { m_id = id; }
	SJobRequest* GetJobInfo() { return &m_job; }

	EFiberState GetState() const { return m_state; }
	void SetState(EFiberState newState);

	void SetPrevious(CFiber* pPrevFiber) { m_pPrevFiber = pPrevFiber; }

	LPVOID Address() const { return m_pFiber; }
	bool IsActive() const { return m_state == eFS_Active; }

private:
	SJobRequest m_job;
	UINT16 m_id;
	size_t m_stackSize;
	EFiberState m_state;
	LPVOID m_pFiber;
	CFiber* m_pPrevFiber;
	CFiber* m_pNextFiber;

#if FIBER_ENABLE_DEBUG
	struct PersonalLogEntry
	{
		PersonalLogEntry(DWORD id, std::string line)
			: threadID(id)
			,	entry(line) {}
		DWORD threadID;
		std::string entry;
	};
	typedef std::vector<PersonalLogEntry> TFiberPersonalLog;
	TFiberPersonalLog m_personalLog;

	static void SetThreadName(LPCSTR name = "UNNAMED");
#endif
};

#endif //~__CFIBER_H__
