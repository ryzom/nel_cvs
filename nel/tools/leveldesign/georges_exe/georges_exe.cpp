// logic_editor_exe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../georges/georges_interface.h"

using namespace std;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// Create a logic editor
	IGeorges* pGeorges = IGeorges::getInterface();

	if (pGeorges)
	{
		// Init ui
		pGeorges->initUI();
		char sCurDir[MAX_PATH];
		GetCurrentDirectory (MAX_PATH,sCurDir);
		pGeorges->SetRootDirectory (sCurDir);
		pGeorges->SetWorkDirectory (sCurDir);

		// Go
		pGeorges->go();

		// Release ui
		pGeorges->releaseUI ();

		// Delete the pointer
		IGeorges::releaseInterface (pGeorges);
	}

	return 0;
}
