/** \file commands.cpp
 * commands management with user interface
 *
 * $Id: commands.cpp,v 1.3 2001/07/11 16:39:07 lecroart Exp $
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

#include "nel/misc/types_nl.h"

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>


#include "client.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

void	initCommands()
{
}

void	updateCommands()
{
	// Display
	Driver->setMatrixMode2D11 ();
	Driver->drawQuad (0.1, 0.1, 0.9, 0.3, CRGBA (128, 255, 128, 128));

	// Output text
	TextContext->setHotSpot (UTextContext::BottomLeft);
	TextContext->printfAt (0.1, 0.1, "chat>");
}

void	releaseCommands()
{
}
