#include "stdafx.h"
#include "ScopedTimer.h"
#include "PerfDB.h"

#define LOG_SCOPED_TIMER 0

ScopedTimer::ScopedTimer(const char* name)
{
	UINT64 countsPerSecond = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	m_secsPerCount = 1.0f / (float)countsPerSecond;
	QueryPerformanceCounter((LARGE_INTEGER*)&m_startCount);
	memcpy(&m_timerName, &name, sizeof(char*));
}

ScopedTimer::~ScopedTimer()
{
	UINT64 tempCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&tempCount);
	float elapsedTime = ((tempCount - m_startCount) * m_secsPerCount);
	g_pPerfDB->AddData(m_timerName, elapsedTime);
#if LOG_SCOPED_TIMER
	DebugLog("[Timers] Timer for %s took %f seconds to complete", m_timerName, elapsedTime);
#endif // ~LOG_SCOPED_TIMER
}
