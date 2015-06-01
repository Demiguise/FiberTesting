#include "stdafx.h"
#include "WebDebugger.h"
#include "FiberScheduler.h"

CWebDebugger::CWebDebugger()
{
	DebugLog("Creating Webdebugger");
	HANDLE threadHandle = CreateThread(NULL, 0, RunService, 0, 0, 0);
	if (!threadHandle)
	{
		DebugLog("	Failed to create WebDebugger Thread");
	}
}


CWebDebugger::~CWebDebugger()
{
}

/*Static*/ DWORD WINAPI CWebDebugger::RunService(LPVOID lpThreadParameter)
{
	DebugLog("Starting WebSocket Service");
	return 0;
}