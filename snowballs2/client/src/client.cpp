/** \file client.cpp
 * Snowballs 2 main file
 *
<<<<<<< client.cpp
 * $Id: client.cpp,v 1.6 2001/07/11 16:11:36 lecroart Exp $
=======
 * $Id: client.cpp,v 1.6 2001/07/11 16:11:36 lecroart Exp $
>>>>>>> 1.5
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

//
// Includes
// 

#include "nel/misc/types_nl.h"

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
#include <windows.h>
#endif

#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"
<<<<<<< client.cpp
#include "nel/misc/config_file.h"

=======

#include "nel/misc/config_file.h"

#include <string>
#include <deque>

>>>>>>> 1.5
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>


#include "landscape.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

//
// Globals
//

CConfigFile ConfigFile;

UDriver		*Driver = NULL;
UScene		*Scene = NULL;

//
// Main
//

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	nlinfo ("Starting Snowballs 2");

	// Load config file

	ConfigFile.load ("client.cfg");

	// Manage paths

	string dataPath = ConfigFile.getVar("DataPath").asString ();
	if (dataPath[dataPath.size()-1] != '/') dataPath += '/';
	CPath::addSearchPath (dataPath);
//	CPath::addSearchPath (dataPath + "zones/");

	// Create a driver
	Driver=UDriver::createDriver();

	// Text context
	Driver->setDisplay (UDriver::CMode(640, 480, 0));
	Driver->setFontManagerMaxMemory (2000000);

	UTextContext *textContext=Driver->createTextContext (CPath::lookup(ConfigFile.getVar("FontName").asString ()));
	textContext->setHotSpot (UTextContext::TopLeft);
	textContext->setColor (CRGBA (255,255,255));
	textContext->setFontSize (12);

	// Create a scene
	Scene=Driver->createScene();

	// Init the landscape using the previously created UScene
	initLandscape();

	pDriver->setMatrixMode2D11 ();
	pDriver->drawQuad (0.1, 0.1, 0.9, 0.3, CRGBA (128, 255, 128, 128));

	while (Driver->isActive() && (!Driver->AsyncListener.isKeyPushed (KeyESCAPE)))
	{
		// Clear
		Driver->clearBuffers (CRGBA (64,64,64,0));

		// Update the landscape
		updateLandscape();

		// Render
		Scene->render ();

		// Output text
		textContext->printfAt (0,1,"flub");

		// Swap
		Driver->swapBuffers ();

		// Pump messages
		Driver->EventServer.pump();

	}

	releaseLandscape();

	delete Driver;

	return EXIT_SUCCESS;
}
