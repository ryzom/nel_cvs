// object_viewer_exe.cpp : Defines the entry point for the application.
//

#include "../object_viewer/std_afx.h"
#include <nel/3d/register_3d.h>
#include <nel/3d/scene.h>
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

 	// Create a object viewer
	IObjectViewer *objectViewer=IObjectViewer::getInterface();

	if (objectViewer)
	{
		// Init ui
		objectViewer->initUI ();

		// Argument ?
		if (strcmp (lpCmdLine, "")!=0)
		{
			// Try to load a shape
			if (objectViewer->loadShape (lpCmdLine))
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
