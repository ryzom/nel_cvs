// object_viewer_exe.cpp : Defines the entry point for the application.
//

#include "../object_viewer/std_afx.h"
#include "../object_viewer/object_viewer.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// Create a object viewer
	CObjectViewer objectViewer;

	// Init ui
	objectViewer.initUI ();

	// Go
	objectViewer.go ();

	// Release ui
	objectViewer.releaseUI ();

	return 0;
}



