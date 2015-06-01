// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here

#include <Windows.h>

namespace 
{
#define SCHEDULEJOB(func, prio, data) 

	static const char* AppendNewlineChar(const char* input)
	{
		int inputSize = strlen(input);
		char* output = new char[inputSize+2];
		memmove(output, input, inputSize);
		output[inputSize] = '\n';
		output[inputSize+1] = '\0';
		return output;
	}

	static void PushToLogs(const char* logLine)
	{
		OutputDebugStringA(logLine);
	}

	static void DebugLog(const char* frmt, ...)
	{
		char buffer[512];
		const char* newLogLine = AppendNewlineChar(frmt);
		va_list args;
		va_start(args, frmt);
		vsnprintf_s(buffer, 512, newLogLine, args);
		va_end(args);
		PushToLogs((LPCSTR)buffer);
		delete newLogLine;
	}
}
