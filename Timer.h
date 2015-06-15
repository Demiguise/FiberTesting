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

private:
	float secsPerCount;
	UINT64 startCount;
	UINT64 endCount;
	bool running;
};
#endif //~TIMER_H
