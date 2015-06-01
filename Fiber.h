#ifndef __CFIBER_H__
#define __CFIBER_H__

#include <assert.h>
#include <Windows.h>
#include <atomic>
#include <string>
#include <vector>

#define FIBER_ENABLE_DEBUG 1

class CFiber;

typedef std::atomic<int> TCounter;
class SFiberCounter
{
public:
	SFiberCounter() 
		: m_funcName("DEFAULT")
	{
		m_counter.store(0);
	}

	SFiberCounter(int initialCounter, std::string funcName = "Default")
		: m_funcName(funcName)
	{
		m_counter.store(initialCounter);
	}

	~SFiberCounter() {}

	bool IsComplete() const { return (m_counter.load() == 0); }
	void DecrementCounter() { m_counter--; }
	const char* GetName() const { return m_funcName.c_str(); }

private:
	// Shouldn't copy these due to the fact that we have an atomic counter. 
	SFiberCounter& operator= (SFiberCounter& rhs);
	std::string m_funcName;
	TCounter m_counter;
};


// The data here could be anything! This is quite worrying!
// Need to consider out 
struct SJobRequest
{
	SJobRequest()
		: m_pJob(NULL)
		, m_pData(NULL)
		, m_pCounter(NULL)
		, m_pFiber(NULL) {}

	SJobRequest(LPFIBER_START_ROUTINE job)
		: m_pJob(job)
		, m_pData(NULL)
		, m_pCounter(NULL)
		, m_pFiber(NULL) {}

	LPFIBER_START_ROUTINE m_pJob;
	void* m_pData;
	SFiberCounter* m_pCounter;
	CFiber* m_pFiber;
};

class CFiber
{
public:
	enum EFiberState
	{
		eFS_InActive = 0,
		eFS_WaitingForJob,
		eFS_Bound,
		eFS_Active,
		eFS_Finished,
		eFS_Releasing,
		eFS_Yielded
	};

	CFiber(LPFIBER_START_ROUTINE func, size_t stack = 0);
	CFiber(size_t stack);
	CFiber();
	~CFiber();

	void Init(UINT16 id, size_t stack);
	bool Bind(SJobRequest& job);
	void Release();

	static void __stdcall Run(LPVOID lpParam);

	//In-Fiber functions
	void StartJob(std::string funcName = "UNNAMED");
	static void SetThreadName(LPCSTR name);
	static void YieldForCounter(SFiberCounter* counter);
	static void* GetDataFromFiber();
	void EndJob();
	static void Log(std::string, ...);

	UINT16 GetID() const { return m_id; }
	void SetID(UINT16 id) { m_id = id; }
	void* GetData() const { return m_pData; }

	EFiberState GetState() const { return m_state; }
	void SetState(EFiberState newState);

	SFiberCounter* GetCounter() const { return m_pCounter; }
	LPFIBER_START_ROUTINE GetFunction() { return m_pFuncPointer; }

	void SetPrevious(CFiber* pPrevFiber) { m_pPrevFiber = pPrevFiber; }

	LPVOID Address() const { return m_pFiber; }
	bool IsActive() const { return m_state == eFS_Active; }
private:

	UINT16 m_id;
	size_t m_stackSize;
	EFiberState m_state;
	LPVOID m_pFiber;
	CFiber* m_pPrevFiber;
	SFiberCounter* m_pCounter;
	void* m_pData;
	LPFIBER_START_ROUTINE m_pFuncPointer; //Function pointer to job

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
#endif
};

#endif //~__CFIBER_H__
