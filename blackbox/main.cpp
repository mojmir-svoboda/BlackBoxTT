#include <cstdio>
#include "BlackBox.h"
#include <bblib/logging.h>

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	bb::initTrace("BBTT", "127.0.0.1", "13127");
	TRACE_MSG(LL_INFO, CTX_BB, "Loading...");
	::CoInitialize(nullptr);

	bb::BlackBox & bb = bb::BlackBox::Instance();
	bb.Init(hInstance);
	bb.Run();
	bb.Done();
	::CoUninitialize();

	TRACE_MSG(LL_INFO, CTX_BB, "Terminating log and exiting main");
	TRACE_DISCONNECT();
}
