/** \file animation.cpp
 * manage anmiation
 *
 * $Id: animation.cpp,v 1.1 2001/07/16 13:17:47 lecroart Exp $
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

#include <list>

#include <nel/misc/types_nl.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>

#include "nel/3d/u_scene.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_play_list_manager.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_transform.h"
#include "nel/3d/u_instance.h"

#include "animation.h"
#include "entities.h"
#include "client.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

UAnimationSet *AnimationSet = NULL;
UPlayListManager *PlayListManager = NULL;

uint WalkAnimId, IdleAnimId;

void	playAnimation (CEntity &entity, uint id)
{
	if (entity.PlayList == NULL)
		createAnimation (entity);

	if (entity.CurrentAnimId == id) return;

	nlinfo ("set animation for entity %u from %u to %u", entity.Id, entity.CurrentAnimId, id);

	entity.PlayList->setAnimation (0, id);
	entity.PlayList->setTimeOrigin (0, float(CTime::getLocalTime ())/1000.0f);
	entity.PlayList->setWrapMode (0, UPlayList::Repeat);

	entity.CurrentAnimId = id;
}

void	createAnimation (CEntity &entity)
{
	nlassert (entity.Instance != NULL && entity.Skeleton != NULL && AnimationSet != NULL);

	entity.PlayList = PlayListManager->createPlayList (AnimationSet);
	entity.PlayList->registerTransform (entity.Instance);
	entity.PlayList->registerTransform (entity.Skeleton);
}

void	deleteAnimation (CEntity &entity)
{
	nlassert (entity.PlayList != NULL);

	PlayListManager->deletePlayList (entity.PlayList);
	entity.PlayList= NULL;
}


void	initAnimation()
{
	AnimationSet = Scene->createAnimationSet ();
	IdleAnimId = AnimationSet->addAnimation ("idle_in_place.anim","IDLE");
	WalkAnimId = AnimationSet->addAnimation ("walk_in_place.anim","WALK");
	AnimationSet->build ();

	PlayListManager = Scene->createPlayListManager ();
}

void	updateAnimation()
{
	PlayListManager->animate (float(CTime::getLocalTime ())/1000.0f);
}

void	releaseAnimation()
{
	/// \todo virer les truc pour les anims
}
