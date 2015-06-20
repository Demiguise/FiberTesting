#include "stdafx.h"

#include <vector>
#include "FiberScheduler.h"
#include "Timer.h"
#include "RayTracer.h"

int _tmain(int argc, _TCHAR* argv[])
{
	g_pFiberScheduler = new CFiberScheduler();
	CRayTracer rayTracer = CRayTracer();
	
	g_pFiberScheduler->StartJobs();

	Timer scheduleTimer;
	scheduleTimer.startTimer();

	while (true)
	{
		if (scheduleTimer.elapsedTime() > 0.5f)
		{
			g_pFiberScheduler->AllocateJobs();
			if (!g_pFiberScheduler->IsActive())
			{
				break;
			}
		}
	}

	DebugLog("Outputting Image");
	rayTracer.OutputImage();

	return 0;
}
