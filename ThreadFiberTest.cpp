#include "stdafx.h"

#include <vector>
#include "FiberScheduler.h"
#include "Timer.h"
#include "lodepng.h"

int _tmain(int argc, _TCHAR* argv[])
{
	g_pFiberScheduler = new CFiberScheduler();

	const int imgWidth = 1280;
	const int imgHeight = 720;

	std::vector<unsigned char> image;
	image.resize(imgWidth * imgHeight * 4);

	for (int w = 0 ; w < imgWidth ; ++w)
	{
		for (int h = 0 ; h < imgHeight ; ++h)
		{
			image[4 * imgWidth * h + 4 * w + 0] = 255 * !(w & h);
			image[4 * imgWidth * h + 4 * w + 1] = w ^ h;
			image[4 * imgWidth * h + 4 * w + 2] = w | h;
			image[4 * imgWidth * h + 4 * w + 3] = 255;
		}
	}

	unsigned error = lodepng::encode("D:\\test.png", image, imgWidth, imgHeight);

	g_pFiberScheduler->StartJobs();

	Timer scheduleTimer;
	scheduleTimer.startTimer();

	UINT64 cycleCount;

	while (true)
	{
		if (scheduleTimer.elapsedTime() > 0.5f)
		{
			cycleCount = __rdtsc();
			g_pFiberScheduler->AllocateJobs();
			DebugLog("CycleTimes");
			DebugLog("%d", (__rdtsc() - cycleCount));
			if (!g_pFiberScheduler->IsActive())
			{
				break;
			}
		}
	}


	return 0;
}
