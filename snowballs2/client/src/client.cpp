/** \file client.cpp
 * Snowballs 2 main file
 *
 * $Id: client.cpp,v 1.28 2001/07/17 12:27:42 legros Exp $
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

#include <nel/misc/types_nl.h>

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
#include <windows.h>
#endif

#include <nel/misc/command.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/i18n.h>
#include <nel/misc/config_file.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>

#include <string>
#include <deque>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
//#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include <nel/pacs/u_move_primitive.h>


#include "commands.h"
#include "landscape.h"
#include "entities.h"
#include "camera.h"
#include "pacs.h"
#include "animation.h"
#include "network.h"
#include "mouse_listener.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

//
// Globals
//

CConfigFile ConfigFile;

UDriver				*Driver = NULL;
UScene				*Scene = NULL;
UInstance			*Cube = NULL;

//U3dMouseListener	*MouseListener = NULL;
C3dMouseListener	*MouseListener = NULL;


UTextContext		*TextContext = NULL;

TTime				LastTime, 
					NewTime;


// true if you want to exit the main loop
bool				NeedExit = false;

bool				ShowRadar;
bool				ShowCommands;
bool				SnapSnowballs;

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

	ShowCommands = ConfigFile.getVar("ShowCommands").asInt () == 1;
	ShowRadar = ConfigFile.getVar("ShowRadar").asInt () == 1;

	SnapSnowballs = false;

	// Manage paths

	string dataPath = ConfigFile.getVar("DataPath").asString ();
	if (dataPath[dataPath.size()-1] != '/') dataPath += '/';
	CPath::addSearchPath (dataPath);
	CPath::addSearchPath (dataPath + "zones/");
	CPath::addSearchPath (dataPath + "tiles/");
	CPath::addSearchPath (dataPath + "meshes/");
	CPath::addSearchPath (dataPath + "materials/");
	CPath::addSearchPath (dataPath + "pacs/");

	// Create a driver
	Driver = UDriver::createDriver();

	// Text context
	Driver->setDisplay (UDriver::CMode(640, 480, 0));
	Driver->setFontManagerMaxMemory (2000000);

	Driver->showCursor(false);
	Driver->setCapture(true);
	Driver->setMousePos(0.5f, 0.5f);

	TextContext = Driver->createTextContext (CPath::lookup(ConfigFile.getVar("FontName").asString ()));

	// Create a scene
	Scene = Driver->createScene();

	// Init the landscape using the previously created UScene
	initLandscape();

	// Init the pacs
	initPACS();

	// Camera
	initCamera();

	// Create a 3D mouse listener
//	MouseListener = Driver->create3dMouseListener ();
	MouseListener = new C3dMouseListener();
	MouseListener->addToServer(Driver->EventServer);
	MouseListener->setCamera(Camera);
	MouseListener->setHotSpot (CVectorD (0,0,0));
	MouseListener->setFrustrum (Camera->getFrustum());
	MouseListener->setMatrix (Camera->getMatrix());
	MouseListener->setMouseMode (U3dMouseListener::firstPerson);
	MouseListener->setSpeed(PlayerSpeed);

	// Init the command control
	initCommands ();

	// Init the radar
	initRadar ();

	// Init the entities prefs
	initEntities();

	// Init animation
	initAnimation ();

	// Creates a self entity
	addEntity(0xFFFFFFFF, CEntity::Self, CVector(ConfigFile.getVar("StartPoint").asFloat(0),
												 ConfigFile.getVar("StartPoint").asFloat(1),
												 ConfigFile.getVar("StartPoint").asFloat(2)),
										 CVector(ConfigFile.getVar("StartPoint").asFloat(0),
												 ConfigFile.getVar("StartPoint").asFloat(1),
												 ConfigFile.getVar("StartPoint").asFloat(2)));

	// Init the network structure
	initNetwork();

	// Display the firsts line
	nlinfo ("Welcome to Snowballs 2");

	NewTime = CTime::getLocalTime();

	while ((!NeedExit) && Driver->isActive())
	{
		// Clear
		Driver->clearBuffers (CRGBA (192,192,200,0));

		// Update the time counters
		LastTime = NewTime;
		NewTime = CTime::getLocalTime();

		// Update animation
		updateAnimation ();

		// Update all entities positions
		MouseListener->updateKeys();
		updateEntities();

		// setup the camera
		MouseListener->updateCamera();
		updateCamera();

		// Update the landscape
		updateLandscape ();

		// Set new animation date
		Scene->animate (float(NewTime)/1000);

		// Render
		Scene->render ();

		// Update the commands panel
		if (ShowCommands) updateCommands ();

		// Update the radar
		if (ShowRadar) updateRadar ();

		renderEntitiesNames();

		TextContext->setHotSpot (UTextContext::TopRight);
		TextContext->setColor (isOnline()?CRGBA(0, 255, 0):CRGBA(255, 0, 0));
		TextContext->setFontSize (18);
		TextContext->printfAt (0.99f, 0.99f, isOnline()?"Online":"Offline");


		// Swap
		Driver->swapBuffers ();

		// Pump messages
		Driver->EventServer.pump();

		// Manage the keyboard
		if (Driver->AsyncListener.isKeyPushed (KeyESCAPE))
		{
			NeedExit = true;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF5))
		{
			ShowCommands = !ShowCommands;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF6))
		{
			ShowRadar = !ShowRadar;
		}
/// \todo virer a la fin
		else if (Driver->AsyncListener.isKeyPushed (KeyF7))
		{
			SnapSnowballs = !SnapSnowballs;
		}
/// \todo fin virer a la fin
		else if (Driver->AsyncListener.isKeyPushed (KeyF8))
		{
			clearCommands ();
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF9))
		{
			if (Self != NULL)
				resetEntityPosition(Self->Id);
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF12))
		{
			CBitmap btm;
			Driver->getBuffer (btm);
			string filename = CFile::findNewFile ("screenshot.tga");
			COFile fs (filename);
			btm.writeTGA (fs,24,true);
			nlinfo("Screenshot '%s' saved", filename.c_str());
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyCONTROL))
		{
			if (Self != NULL)
			{
				/// todo
/*
				CVector	Direction = CVector((float)cos(Self->Angle), (float)sin(Self->Angle), (2.0f-ViewHeight)/ViewLagBehind).normed();
				shotSnowball(Self->Id, Self->Position+Direction*100.0f);
*/
			}
		}

		// Update network messages
		updateNetwork ();

		// Check if the config file was modified by another program
		CConfigFile::checkConfigFiles ();
	}

	Driver->setCapture(false);
	Driver->showCursor(true);

	releaseNetwork ();
	releaseAnimation ();
	releasePACS();
	releaseLandscape();

	MouseListener->removeFromServer(Driver->EventServer);
	delete MouseListener;

	delete Driver;

	return EXIT_SUCCESS;
}

NLMISC_COMMAND(quit,"quit the client","")
{
	// check args, if there s not the right number of parameter, return bad
	if(args.size() != 0) return false;

	log.displayNL("Exit requested");

	NeedExit = true;

	return true;
}
