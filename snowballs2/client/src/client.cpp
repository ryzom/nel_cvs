/** \file client.cpp
 * Client prototype
 *
 * $Id: client.cpp,v 1.2 2001/07/11 15:32:26 legros Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "nel/misc/types_nl.h"

#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
#include <windows.h>
#endif

#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"

#include <string>
#include <deque>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;


/****************************************************************\
							MAIN
\****************************************************************/
#if defined(NL_OS_WINDOWS) && defined (NL_RELEASE)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	nlinfo ("Starting Snowballs 2");

	// Create a driver
	UDriver	*pDriver=UDriver::createDriver();

	// Text context
	pDriver->setDisplay (UDriver::CMode(640, 480, 0));
	pDriver->setFontManagerMaxMemory (2000000);
	UTextContext *textContext=pDriver->createTextContext ("\\\\server\\code\\fonts\\arialuni.ttf");
	textContext->setHotSpot (UTextContext::TopLeft);
	textContext->setColor (CRGBA (255,255,255));
	textContext->setFontSize (12);

	// Create a scene
	UScene *pScene=pDriver->createScene();


	while (pDriver->isActive() && (!pDriver->AsyncListener.isKeyPushed (KeyESCAPE)))
	{
		// Clear
		pDriver->clearBuffers (CRGBA (64,64,64,0));

		// Render
		pScene->render ();

		// Output text
		textContext->printfAt (0,1,"flub");

		// Swap
		pDriver->swapBuffers ();

		// Pump messages
		pDriver->EventServer.pump();

	}

	delete pDriver;

	return EXIT_SUCCESS;
}
