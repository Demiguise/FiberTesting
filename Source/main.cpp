#include "stdafx.h"

#include <vector>
#include "Fibers/FiberScheduler.h"
#include "Common/Timer.h"
#include "RayTracer.h"
#include "Debug/PerfDB.h"

#include "glfw3.h"
#include <math.h>

static bool bTracingStarted = false;

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		{
			if (action == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GL_TRUE);
			}
		}
		break;
	case GLFW_KEY_1:
		{
			if (action == GLFW_PRESS)
			{
				g_pFiberScheduler->StartJobs();
				bTracingStarted = true;
			}
		}
		break;
	case GLFW_KEY_T:
		{
			if (action == GLFW_PRESS)
			{
				DebugLog("Outputting Image");
				g_pRayTracer->OutputImage();
			}
		}
	case GLFW_KEY_F:
		{
			if (action == GLFW_PRESS)
			{
				double posX, posY;
				glfwGetCursorPos(window, &posX, &posY);
				int posXi = (int)Clampf(posX, 0.f, gWinWidth);
				int posYi = (int)Clampf(posY, 0.f, gWinHeight);
				SPixelData* asd = g_pRayTracer->GetPixelDataForPos(posXi, posYi);
				DebugLog("p (%d, %d) (%f, %f, %f)", posXi, posYi, asd->finalColour.x, asd->finalColour.y, asd->finalColour.z);
			}
		}
		break;
	}
}

void Render(GLFWwindow* pWindow)
{
		//Render
		GLfloat* image = (GLfloat*)malloc(sizeof(GLfloat) * gWinHeight * gWinWidth * 4);
		g_pRayTracer->GetGLPixelOutput(image);

		if (image)
		{
			glDrawPixels(gWinWidth, gWinHeight, GL_RGBA, GL_FLOAT, image);
			free(image);
		}

		glfwSwapBuffers(pWindow);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwSetErrorCallback(error_callback);

	GLFWwindow* window = glfwCreateWindow(gWinWidth, gWinHeight, "My First Raytracer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	//DebugLog("%s", glGetString(GL_VERSION));

	g_pFiberScheduler = new CFiberScheduler();
	g_pRayTracer = new CRayTracer();
	g_pPerfDB = new PerformanceDB();

	Timer scheduleTimer, jobCheckTimer;
	scheduleTimer.startTimer();
	jobCheckTimer.startTimer();

	// Todo:
	// Add better Debugging. Perhaps a list of completed jobs / personal log etc?
	// Needs some way of identifying jobs.
	// Explore possiblity of openGL library that matt had for even better debugging (Can click on pixel, see job listing, etc) IMGUI?

	while (!glfwWindowShouldClose(window))
	{
		//Allocate
		if (bTracingStarted)
		{
			g_pFiberScheduler->AllocateJobs();
			if (!g_pFiberScheduler->IsActive())
			{
				DebugLog("Done?!");
				bTracingStarted = false;
			}

			if (jobCheckTimer.elapsedTime() > 1.f)
			{
				g_pFiberScheduler->PrintAverageJobTime();
				jobCheckTimer.resetTimer();
				
				g_pPerfDB->ReportAvgs();
			}
		}
		Render(window);
		glfwPollEvents();
	}
	
	g_pRayTracer->OutputImage();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
