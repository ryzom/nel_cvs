/** \file anim_detail_trav.cpp
 * <File description>
 *
 * $Id: anim_detail_trav.cpp,v 1.12 2003/03/27 16:51:45 berenguier Exp $
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
#include "3d/transform.h"
#include "3d/skeleton_model.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/debug.h"


using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
CAnimDetailTrav::CAnimDetailTrav()
{
	CurrentDate=0;
	// prepare some space
	_VisibleList.reserve(1024);
}

// ***************************************************************************
void				CAnimDetailTrav::clearVisibleList()
{
	_VisibleList.clear();
}


// ***************************************************************************
void				CAnimDetailTrav::traverse()
{
	H_AUTO( NL3D_TravAnimDetail );

	// Inc the date.
	CurrentDate++;

	// Traverse all nodes of the visibility list.
	uint	nModels= _VisibleList.size();
	for(uint i=0; i<nModels; i++)
	{
		CTransform		*model= _VisibleList[i];
		// If this object has an ancestorSkeletonModel
		if(model->_AncestorSkeletonModel)
		{
			// then just skip it! because it will be parsed hierarchically by the first 
			// skeletonModel whith _AncestorSkeletonModel==NULL. (only if this one is visible)
			continue;
		}
		else
		{
			// If this is a skeleton model, and because _AncestorSkeletonModel==NULL,
			// then it means that it is the Root of a hierarchy of transform that have 
			// _AncestorSkeletonModel!=NULL.
			if( model->isSkeleton() )
			{
				// Then I must update hierarchically me and the sons (according to HRC hierarchy graph) of this model.
				traverseHrcRecurs(model);
			}
			else
			{
				// else, just traverse AnimDetail, an do nothing for Hrc sons
				model->traverseAnimDetail(NULL);
			}
		}
	}
}


// ***************************************************************************
void	CAnimDetailTrav::traverseHrcRecurs(CTransform *model)
{
	// first, just doIt me
	model->traverseAnimDetail(NULL);


	// then doIt my sons in Hrc.
	uint	num= model->hrcGetNumChildren();
	for(uint i=0;i<num;i++)
		traverseHrcRecurs(model->hrcGetChild(i));
}


} // NL3D
