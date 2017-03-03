#include <cstdio>
#include "BlackBox.h"
#include <blackbox/common.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	bb::initTrace("BBTT", "127.0.0.1", "13127");
	TRACE_MSG(LL_INFO, CTX_BB, "Loading...");
	::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

	bb::BlackBox & bb = bb::BlackBox::Instance();
	if (bb.Init(hInstance))
	{
		bb.Run();
	}
	else
	{
		TRACE_MSG(LL_ERROR, CTX_BB, "Cannot initialize blackbox, terminating");
	}
	bb.Done();

	::CoUninitialize();

	TRACE_MSG(LL_INFO, CTX_BB, "Terminating log and exiting main");
	TRACE_DISCONNECT();
}
