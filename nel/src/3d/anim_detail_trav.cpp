/** \file anim_detail_trav.cpp
 * <File description>
 *
 * $Id: anim_detail_trav.cpp,v 1.6 2002/02/28 12:59:49 besson Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "std3d.h"

#include "3d/anim_detail_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/transform.h"
#include "3d/skeleton_model.h"


namespace NL3D 
{


// ***************************************************************************
IObs				*CAnimDetailTrav::createDefaultObs() const
{
	return new CDefaultAnimDetailObs;
}


// ***************************************************************************
void				CAnimDetailTrav::traverse()
{
	// Inc the date.
	CurrentDate++;

	// Traverse all nodes of the visibility list.
	uint	nObs= _ClipTrav->numVisibleObs();
	for(uint i=0; i<nObs; i++)
	{
		IBaseClipObs		*clipObs= _ClipTrav->getVisibleObs(i);
		// If this object is a transform and if it has an ancestorSkeletonModel
		CTransformHrcObs*	hrcObs= dynamic_cast<CTransformHrcObs*>(clipObs->HrcObs);
		if(hrcObs && hrcObs->_AncestorSkeletonModel)
		{
			// then just skip it! because it will be parsed hierarchically by the first 
			// skeletonModel whith hrcObs->_AncestorSkeletonModel==NULL. (only if this one is visible)
			continue;
		}
		else
		{
			// If this is a skeleton model, and because hrcObs->_AncestorSkeletonModel==NULL,
			// then it means that it is the Root of a hierarchy of transform that have 
			// hrcObs->_AncestorSkeletonModel!=NULL.
			CSkeletonModelAnimDetailObs*	skelObs= dynamic_cast<CSkeletonModelAnimDetailObs*>(clipObs->AnimDetailObs);
			if(skelObs)
			{
				// Then I must update hierarchically me and the sons (according to HRC hierarchy graph) of this model.
				traverseHrcRecurs(skelObs);
			}
			else
			{
				// else, just traverse AnimDetail, an do nothing for Hrc sons
				clipObs->AnimDetailObs->traverse(NULL);
			}
		}
	}
}


// ***************************************************************************
void	CAnimDetailTrav::traverseHrcRecurs(IBaseAnimDetailObs *adObs)
{
	// first, just doIt me
	adObs->traverse(NULL);


	// then doIt my sons in Hrc.
	// get the  hrc observer.
	IBaseHrcObs		*hrcObs= adObs->HrcObs;
	// for all sons in hrc.
	IBaseHrcObs		*sonHrcObs= static_cast<IBaseHrcObs*>(hrcObs->getFirstChild());
	while( sonHrcObs )
	{
		// get the animDetailObs, and traverse it, recursively
		traverseHrcRecurs(sonHrcObs->AnimDetailObs);

		// brother in HRC
		sonHrcObs= static_cast<IBaseHrcObs*>(hrcObs->getNextChild());
	}
}


// ***************************************************************************
void	IBaseAnimDetailObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
}



} // NL3D
