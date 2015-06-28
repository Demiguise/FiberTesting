#ifndef SCOPEDTIMER_H
#define SCOPEDTIMER_H

#include <wtypes.h>

#define PERFTIMER_FUNC() ScopedTimer funcTimer  = ScopedTimer(__FUNCTION__)
#define PERFTIMER(x) ScopedTimer perfTimer = ScopedTimer(__FUNCTION__ " (" x ")")

class ScopedTimer
{
public:
	ScopedTimer(const char* name);
	~ScopedTimer();
private:
	float m_secsPerCount;
	UINT64 m_startCount;
	const char* m_timerName;
};
#endif //~SCOPEDTIMER_H
