/** \file landscape.cpp
 * Landscape management with user interface
 *
 * $Id: landscape.cpp,v 1.8 2001/07/18 15:24:26 legros Exp $
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

#include "nel/misc/command.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"
#include "nel/misc/config_file.h"

#include <string>
#include <deque>

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

#include <nel/3d/u_visual_collision_entity.h>
#include <nel/3d/u_visual_collision_manager.h>

#include "client.h"
#include "pacs.h"
#include "mouse_listener.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

ULandscape				*Landscape = NULL;
UVisualCollisionEntity	*AimingEntity = NULL;


void	initLandscape()
{
	// create the landscape
	Landscape = Scene->createLandscape();

	// load the bank files
	Landscape->loadBankFiles (CPath::lookup(ConfigFile.getVar("BankName").asString()), 
							  CPath::lookup(ConfigFile.getVar("FarBankName").asString()));

	// setup the zone path
	Landscape->setZonePath (ConfigFile.getVar("DataPath").asString() + "zones/");

	// and eventually, load the zones around the starting point.
	Landscape->loadAllZonesAround (CVector(ConfigFile.getVar("StartPoint").asFloat(0),
										   ConfigFile.getVar("StartPoint").asFloat(1),
										   ConfigFile.getVar("StartPoint").asFloat(2)), 
								   1000.0f);

	// color of the shadow
	Landscape->setupStaticLight(CRGBA(241, 226, 244), CRGBA(17, 54, 100), 1.1f);	// Init light color
}

void	updateLandscape()
{
	// load the zones around the viewpoint
	Landscape->refreshZonesAround (MouseListener->getViewMatrix().getPos(), 1000.0f);
}

void	releaseLandscape()
{
}

void	initAiming()
{
	// Create an aiming entity
	AimingEntity = VisualCollisionManager->createEntity();
	AimingEntity->setCeilMode(true);
}

void	releaseAiming()
{
	VisualCollisionManager->deleteEntity(AimingEntity);
}

CVector	getTarget(const CVector &start, const CVector &step, uint numSteps)
{
	CVector	testPos = start;

	uint	i;
	for (i=0; i<numSteps; ++i)
	{
		// For each step, check if the snapped position is backward the normal
		CVector	snapped = testPos;
		CVector	normal;
		// here use normal to check if we have collision
		if (AimingEntity->snapToGround(snapped, normal) && (testPos.z-snapped.z)*normal.z < 0.0f)
		{
			testPos -= step*0.5f;
			break;
		}
		testPos += step;
	}

	return testPos;
}

