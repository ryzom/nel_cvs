/** \file client.cpp
 * Snowballs 2 main file
 *
 * $Id: client.cpp,v 1.34 2001/07/18 12:16:21 legros Exp $
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

#include <nel/net/login_client.h>

#include "commands.h"
#include "landscape.h"
#include "entities.h"
#include "camera.h"
#include "pacs.h"
#include "animation.h"
#include "network.h"
#include "sound.h"
#include "interface.h"
#include "lens_flare.h"
#include "mouse_listener.h"
#include "radar.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLNET;

//
// Globals
//

CConfigFile ConfigFile;

UDriver				*Driver = NULL;
UScene				*Scene = NULL;
UInstance			*Cube = NULL;

C3dMouseListener	*MouseListener = NULL;

UTextContext		*TextContext = NULL;

// The previous and current frame dates
TTime				LastTime, 
					NewTime;


// true if you want to exit the main loop
bool				NeedExit = false;

bool				ShowCommands;

bool				CaptureState = false;


uint loginState = 0;
string login;

void startLoginInterface ()
{
	loginState = 1;
	askString ("Please enter your login:", ConfigFile.getVar("Login").asString ());
	login = "";
}

void updateLoginInterface ()
{
	string str;
	if (haveAnswer (str))
	{
		nlinfo ("result %s", str.c_str());

		// esc pressed, stop all only before the shard selection
		if (str.empty () && loginState < 3)
		{
			loginState = 0;
			return;
		}

		switch (loginState)
		{
		case 1:
			nlinfo ("login entered");
			login = str;
			askString ("Please enter your password:", ConfigFile.getVar("Password").asString (), 1);
			loginState++;
			break;
		case 2:
			{
				nlinfo ("password entered");
				string password = str;

				string LoginSystemHost = ConfigFile.getVar("LoginSystemHost").asString ();
				string res = CLoginClient::authenticate (LoginSystemHost+":49999", login, password, 1);

				if (!res.empty ())
				{
					string err = string ("Authentification failed: ") + res;
					askString (err, "", 2, CRGBA(64,0,0,128));
					loginState = 0;
				}
				else
				{
					string list = "Please enter the shard number you want to connected to\n\n";
					for (uint32 i = 0; i < CLoginClient::ShardList.size (); i++)
					{
						list += "Shard "+toString (i)+": "+CLoginClient::ShardList[i].ShardName+" ("+CLoginClient::ShardList[i].WSAddr+")\n";
					}
					askString (list, toString(ConfigFile.getVar("ShardNumber").asInt()));
					loginState++;
				}
			}
			break;
		case 3:
			{
				nlinfo ("shard selected");
				uint32 shardNum = atoi (str.c_str());

				string res = CLoginClient::connectToShard (shardNum, *Connection);
				if (!res.empty ())
				{
					string err = string ("Connection to shard failed: ") + res;
					askString (err, "", 2, CRGBA(64,0,0,128));
					loginState = 0;
				}
				else
				{
					askString ("You are online!!!", "", 2, CRGBA(0,64,0,128));
					loginState = 0;
				}
			}
			break;
		}
	}
}

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

	// Manage paths

	string dataPath = ConfigFile.getVar("DataPath").asString ();
	if (dataPath[dataPath.size()-1] != '/') dataPath += '/';
	CPath::addSearchPath (dataPath);
	CPath::addSearchPath (dataPath + "zones/");
	CPath::addSearchPath (dataPath + "tiles/");
	CPath::addSearchPath (dataPath + "shapes/");
	CPath::addSearchPath (dataPath + "maps/");
	CPath::addSearchPath (dataPath + "pacs/");
	CPath::addSearchPath (dataPath + "anims/");

	// Create a driver
	Driver = UDriver::createDriver();

	// Text context
	Driver->setDisplay (UDriver::CMode(640, 480, 0));
	Driver->setFontManagerMaxMemory (2000000);

	// Init the mouse so it's trapped by the main window.
	Driver->showCursor(false);
	Driver->setCapture(true);
	Driver->setMousePos(0.5f, 0.5f);

	// Create a Text context for later text display
	TextContext = Driver->createTextContext (CPath::lookup(ConfigFile.getVar("FontName").asString ()));

	// Create a scene
	Scene = Driver->createScene();

	// Init the landscape using the previously created UScene
	initLandscape();

	// Init the pacs
	initPACS();

	// Init the aiming system
	initAiming();

	// Camera
	initCamera();

	// Create a 3D mouse listener
	MouseListener = new C3dMouseListener();
	MouseListener->addToServer(Driver->EventServer);
	MouseListener->setCamera(Camera);
	MouseListener->setHotSpot (CVectorD (0,0,0));
	MouseListener->setFrustrum (Camera->getFrustum());
	MouseListener->setMatrix (Camera->getMatrix());
	MouseListener->setSpeed(PlayerSpeed);
	initMouseListenerConfig();

	// Init interface
	initInterface();

	// Init radar
	initRadar();

	// Init sound control
	initSound();

	// Init the command control
	initCommands ();

	// Init the radar
	initRadar ();

	// Init the entities prefs
	initEntities();

	// Init animation
	initAnimation ();

	// Init animation
	initLensFlare ();

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

	if (ConfigFile.getVar("AutoLogin").asInt() == 1)
		startLoginInterface ();

	while ((!NeedExit) && Driver->isActive())
	{
		updateLoginInterface ();
		
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
		// -> first update camera position directly from the mouselistener
		// -> then update stuffs linked to the camera (snow, sky, lens flare etc.)
		MouseListener->updateCamera();
		updateCamera();

		// Update the landscape
		updateLandscape ();

		// Update the sound
		updateSound ();

		// Set new animation date
		Scene->animate (float(NewTime)/1000);

		// Render the sky first
		updateSky ();

		// Render
		Scene->render ();

		// Render the lens flare
		updateLensFlare ();

		// Update the commands panel
		if (ShowCommands) updateCommands ();

		// Update the radar
		updateRadar ();

		renderEntitiesNames();

		updateInterface ();

		TextContext->setHotSpot (UTextContext::TopRight);
		TextContext->setColor (isOnline()?CRGBA(0, 255, 0):CRGBA(255, 0, 0));
		TextContext->setFontSize (18);
		TextContext->printfAt (0.99f, 0.99f, isOnline()?"Online":"Offline");


		// Swap
		Driver->swapBuffers ();

		// Pump messages
		Driver->EventServer.pump();

		// Manage the keyboard

		// Shift Escape -> quit
		if (Driver->AsyncListener.isKeyDown (KeySHIFT) && Driver->AsyncListener.isKeyDown (KeyESCAPE))
		{
			NeedExit = true;
		}
		// F3 -> radar zoom out
		else if(Driver->AsyncListener.isKeyDown(KeyF3))
		{
			RadarDistance += 50;
		}
		// F4 -> radar zoom in
		else if(Driver->AsyncListener.isKeyDown(KeyF4))
		{
			RadarDistance -= 50;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF5))
		{
			ShowCommands = !ShowCommands;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF6))
		{
			RadarState = (RadarState+1)%3;
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF8))
		{
			clearCommands ();
		}
		// F9 -> release/capture the mouse
		else if (Driver->AsyncListener.isKeyPushed (KeyF9))
		{
			if (!CaptureState)
			{
				Driver->setCapture(false);
				Driver->showCursor(true);
				MouseListener->removeFromServer(Driver->EventServer);
			}
			else
			{
				Driver->setCapture(true);
				Driver->showCursor(false);
				MouseListener->addToServer(Driver->EventServer);
			}
			CaptureState = !CaptureState;
		}
		// F11 -> reset the PACS global position of the self entity (in case of a pacs failure :-\)
		else if (Driver->AsyncListener.isKeyPushed (KeyF11))
		{
			if (Self != NULL)
				resetEntityPosition(Self->Id);
		}
		// F12 -> take a screenshot
		else if (Driver->AsyncListener.isKeyPushed (KeyF12))
		{
			CBitmap btm;
			Driver->getBuffer (btm);
			string filename = CFile::findNewFile ("screenshot.tga");
			COFile fs (filename);
			btm.writeTGA (fs,24,true);
			nlinfo("Screenshot '%s' saved", filename.c_str());
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF1))
		{
			if (!isOnline())
				startLoginInterface ();
		}
		else if (Driver->AsyncListener.isKeyPushed (KeyF2))
		{
			if (isOnline())
				Connection->disconnect ();
		}


		// Update network messages
		updateNetwork ();

		// Check if the config file was modified by another program
		CConfigFile::checkConfigFiles ();
	}

	// release the mouse
	Driver->setCapture(false);
	Driver->showCursor(true);

	// release all before quit
	releaseLensFlare ();
	releaseInterface ();
	releaseNetwork ();
	releaseAnimation ();
	releaseAiming();
	releasePACS();
	releaseLandscape();
	releaseAnimation ();
	releaseSound ();

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
