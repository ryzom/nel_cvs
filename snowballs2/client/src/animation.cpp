/** \file animation.cpp
 * Animation interface between the game and NeL
 *
 * $Id: animation.cpp,v 1.8 2001/07/20 14:35:51 lecroart Exp $
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

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;

//
// Constantes
//

// Amount of time for the transistion between 2 animations
CAnimationTime TransitionTime = 0.25f;

//
// Variables
//

UAnimationSet *AnimationSet = NULL;
UPlayListManager *PlayListManager = NULL;

struct Anim
{
	char	*Name;
	bool	 Loop;
	uint	 Id;
};

Anim AnimIdArray[][2] =
{
	{ { "marche.anim", true }, { "" } },
	{ { "idle.anim", true }, { "" } },
	{ { "log_on.anim", false }, { "" } },
	{ { "log_off.anim", false }, { "" } },
	{ { "lancelaboule.anim", false }, { "" } },
	{ { "prepaboule.anim", true }, { "" } },
	{ { "impact.anim", false }, { "" } },
};


//
// Functions
//

void	playAnimation (CEntity &entity, EAnim anim)
{
	if (entity.Skeleton == NULL)
		return;

	// If the first time we play an animation, creates the animation class
	if (entity.PlayList == NULL)
		createAnimation (entity);

	// If we try to play the same animation as the current one, do nothing
	if (entity.CurrentAnim == anim) return;

	// Get the current time
	CAnimationTime CurrentTime = CAnimationTime(CTime::getLocalTime ())/1000.0f;

	// todo a virer
	nlinfo ("set animation for entity %u from %u to %u", entity.Id, entity.CurrentAnim, anim);

	// Find the new slot for the full animation (0 or 1)
	uint newSlot, oldSlot;
	if (entity.NextEmptySlot == 0)
	{
		newSlot = 0; oldSlot = 1; entity.NextEmptySlot = 1;
	}
	else
	{
		newSlot = 1; oldSlot = 0; entity.NextEmptySlot = 0;
	}

	// Fill the new animation slot with the new animation to play
	entity.PlayList->setAnimation (newSlot, AnimIdArray[anim][0].Id);
	entity.PlayList->setTimeOrigin (newSlot, CurrentTime);

	if (AnimIdArray[anim][0].Loop)
		entity.PlayList->setWrapMode (newSlot, UPlayList::Repeat);
	else
		entity.PlayList->setWrapMode (newSlot, UPlayList::Clamp);

	CAnimationTime OldStartWeight, OldEndWeight;
	CAnimationTime NewStartWeight, NewEndWeight;

	// Get the starting time of the old animation slot
	entity.PlayList->getStartWeight (oldSlot, OldStartWeight);
	
	// Compute the time delta between start of the old animation and now
	CAnimationTime dt = CurrentTime - OldStartWeight;

	// Compute the new transition value depending of the current time

	if (dt > TransitionTime)
		dt = TransitionTime;

	OldStartWeight = CurrentTime - (TransitionTime - dt);
	OldEndWeight = CurrentTime + dt;
		
	NewStartWeight = CurrentTime;
	NewEndWeight = CurrentTime + dt;

	// Set new weights on the old and the new animation slot

	entity.PlayList->setStartWeight (oldSlot, 1.0f, OldStartWeight);
	entity.PlayList->setEndWeight (oldSlot, 0.0f, OldEndWeight);
	entity.PlayList->setWeightSmoothness (oldSlot, 1.0f);

	entity.PlayList->setStartWeight (newSlot, 0.0f, NewStartWeight);
	entity.PlayList->setEndWeight (newSlot, 1.0f, OldEndWeight);
	entity.PlayList->setWeightSmoothness (newSlot, 1.0f);

	// Keep in mind what is the last animation id we set
	entity.CurrentAnim = anim;
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
	if (entity.PlayList == NULL)
		return;

	PlayListManager->deletePlayList (entity.PlayList);
	entity.PlayList= NULL;
}


void	initAnimation()
{
	AnimationSet = Scene->createAnimationSet ();
	
	// Add all animations in the animation set
	for (uint i = 0; i < sizeof (AnimIdArray) / sizeof (AnimIdArray[0]); i++)
	{
		if (AnimIdArray[i][0].Name[0] != '\0')
			AnimIdArray[i][0].Id = AnimationSet->addAnimation (AnimIdArray[i][0].Name, AnimIdArray[i][0].Name);

		if (AnimIdArray[i][1].Name[0] != '\0')
			AnimIdArray[i][1].Id = AnimationSet->addAnimation (AnimIdArray[i][1].Name, AnimIdArray[i][1].Name);
	}
	AnimationSet->build ();

	PlayListManager = Scene->createPlayListManager ();
}

void	updateAnimation()
{
/*	// Get the current time
	CAnimationTime CurrentTime = CAnimationTime(CTime::getLocalTime ())/1000.0f;

	for (EIT eit = Entities.begin (); eit != Entities.end (); )
	{
		CEntity	&entity = (*eit).second;
		if (entity.CurrentAnim == HitAnim)
		{
			// Hit special case
			uint HitSlot = 3;

			// Get the starting time of the old animation slot
			CAnimationTime OldStartWeight;
			entity.PlayList->getStartWeight (HitSlot, OldStartWeight);
			if (CurrentTime > OldStartWeight + TransitionTime)
			{
				entity.PlayList->setAnimation (HitSlot, AnimIdArray[anim][0].Id);
				entity.PlayList->setTimeOrigin (HitSlot, CurrentTime);
				entity.PlayList->setWrapMode (newSlot, UPlayList::Clamp);
				entity.PlayList->setStartWeight (HitSlot, 0.0f, CurrentTime);
				entity.PlayList->setEndWeight (HitSlot, 1.0f, CurrentTime+TransitionTime);
				entity.PlayList->setWeightSmoothness (HitSlot, 1.0f);
			}
		}
	}
*/
	// compute new animation position depending of the current time
	PlayListManager->animate (float(CTime::getLocalTime ())/1000.0f);
}

void	releaseAnimation()
{
	Scene->deletePlayListManager (PlayListManager);

// The next line doesn t work (say that AnimationSet is not a valid AnimationSet Ptr) so we comment it.
//	Scene->deleteAnimationSet (AnimationSet);
}
