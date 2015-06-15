#include "stdafx.h"
#include "Timer.h"

Timer::Timer()
{
	UINT64 countsPerSecond = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	secsPerCount = 1.0f / (float)countsPerSecond;
}

Timer::~Timer()
{
}

//Returns time in seconds.
float Timer::elapsedTime()
{
	if (running)
	{
		UINT64 tempCount;
		QueryPerformanceCounter((LARGE_INTEGER*)&tempCount);
		return ((tempCount - startCount) * secsPerCount);
	}
	else
	{
		return ((endCount - startCount) * secsPerCount);
	}
}

void Timer::startTimer()
{
	running = true;
	QueryPerformanceCounter((LARGE_INTEGER*)&startCount);
}

void Timer::stopTimer()
{
	running = false;
	QueryPerformanceCounter((LARGE_INTEGER*)&endCount);
}

void Timer::resetTimer()
{
	running = true;
	QueryPerformanceCounter((LARGE_INTEGER*)&startCount);
}