#ifndef __WEBDEBUGGER_H__
#define __WEBDEBUGGER_H__

class CWebDebugger
{
public:
	CWebDebugger();
	~CWebDebugger();

	static DWORD WINAPI RunService(LPVOID lpThreadParameter);
};

#endif //~__WEBDEBUGGER_H__
