
#include <windows.h>

#include "debug.h"
#include "application.h"
#include "applicationdata.h"
#include "device.h"

Application::Application ()
{
	Data = new ApplicationData;
}

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Application *application = Application::Create ();

	application->Data->HInstance = hInstance;
	application->Data->HPrevInstance = hPrevInstance;
	application->Data->lpCmdLine = lpCmdLine;
	application->Data->nShowCmd = nShowCmd;

	Device::initDM (application);

	application->onStart ();
	bool Quit = false;
	while (!application->onIdle () && !Quit)
	{
		MSG msg;
		while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (GetMessage (&msg, NULL, 0, 0) == 0)
				Quit = true;
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}
	application->onStop ();

	// libere les devices et le device manager
	Device::releaseDM ();

	return 0;
}
