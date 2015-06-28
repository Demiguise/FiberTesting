#ifndef TIMER_H
#define TIMER_H

#include <wtypes.h>

class Timer
{
public:
	Timer();
	~Timer();
	float elapsedTime();
	void startTimer();
	void stopTimer();
	void resetTimer();

	static UINT64 GetCountNow();
	static float GetTimeBetween(UINT64 start, UINT64 end);

private:
	static float secsPerCount;
	UINT64 startCount;
	UINT64 endCount;
	bool running;
};
#endif //~TIMER_H
