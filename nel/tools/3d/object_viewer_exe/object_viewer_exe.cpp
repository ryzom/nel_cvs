// object_viewer_exe.cpp : Defines the entry point for the application.
//

#include "std_afx.h"


#include <3d/register_3d.h>
#include <3d/scene.h>
#include "../object_viewer/object_viewer_interface.h"

using namespace NL3D;
using namespace NLMISC;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Register 3d
	registerSerial3d ();
	CScene::registerBasics ();

	// Remove command line ""
	char commandLine[512];
	char *commandLinePtr=commandLine;
	strcpy (commandLine, lpCmdLine);
	if (commandLine[0]=='"')
		commandLinePtr++;
	if (commandLinePtr[strlen (commandLinePtr)-1]=='"')
		commandLinePtr[strlen (commandLinePtr)-1]=0;

 	// Create a object viewer
	IObjectViewer *objectViewer=IObjectViewer::getInterface();

	if (objectViewer)
	{
		// Init ui
		objectViewer->initUI ();

		// Argument ?
		if (strcmp (commandLinePtr, "")!=0)
		{
			// Try to load a shape
			if (objectViewer->loadMesh (commandLinePtr, ""))
			{
				// Reset the camera
				objectViewer->resetCamera ();
			}
			else
				return 1;
		}

		// Go
		objectViewer->go ();

		// Release ui
		objectViewer->releaseUI ();

		// Delete the pointer
		IObjectViewer::releaseInterface (objectViewer);
	}


	return 0;
}
