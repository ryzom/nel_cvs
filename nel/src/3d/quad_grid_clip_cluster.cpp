/** \file quad_grid_clip_cluster.cpp
 * <File description>
 *
 * $Id: quad_grid_clip_cluster.cpp,v 1.2 2002/02/28 12:59:51 besson Exp $
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

#include "3d/quad_grid_clip_cluster.h"


using namespace NLMISC;
using namespace std;


namespace NL3D {


// ***************************************************************************
void		CQuadGridClipCluster::registerBasic()
{
	CMOT::registerModel( QuadGridClipClusterId, 0, CQuadGridClipCluster::creator);
	CMOT::registerObs( ClipTravId, QuadGridClipClusterId, CQuadGridClipClusterClipObs::creator );
}


// ***************************************************************************
void		CQuadGridClipCluster::extendCluster(const NLMISC::CAABBox &worldBBox)
{
	if(_Empty)
	{
		_Empty= false;
		_BBox= worldBBox;
	}
	else
	{
		// extend the bbox with 2 corners of the incoming bbox (sufficient for an AABBox).
		_BBox.extend( worldBBox.getCenter() + worldBBox.getHalfSize() );
		_BBox.extend( worldBBox.getCenter() - worldBBox.getHalfSize() );
	}

	// update _Radius
	_Radius= _BBox.getRadius();
}


// ***************************************************************************
void		CQuadGridClipCluster::update()
{
	// never need to update, so unlink me from ValidateList.
	unlinkFromValidateList();
}


// ***************************************************************************
void		CQuadGridClipClusterClipObs::traverse(IObs *caller)
{
	CQuadGridClipCluster	*cluster= (CQuadGridClipCluster*)Model;
	CClipTrav			*clipTrav= (CClipTrav*)Trav;

	// if empty, just return (not visible at all).
	if(cluster->_Empty)
		return;

	// First, clip DistMax.
	CAABBox		&bbox= cluster->_BBox;
	// if clip DistMax enabled
	if(cluster->_DistMax!=-1)
	{
		CVector		c= bbox.getCenter();
		float		dist= (c - clipTrav->CamPos).norm();
		// it the bbox is entirely out the distMax
		if( dist-cluster->_Radius > cluster->_DistMax )
			return;
	}


	// Then clip against pyramid
	bool	unspecified= false;
	bool	visible= true;
	for(sint i=0;i<(sint)clipTrav->WorldPyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		if(!bbox.clipBack(clipTrav->WorldPyramid[i]))
		{
			visible= false;
			break;
		}
		// else test is the bbox is partially or fully in the plane
		else if(!unspecified)
		{
			// if clipFront AND clipBack, it means partially.
			if(bbox.clipFront(clipTrav->WorldPyramid[i]))
				unspecified= true;
		}
	}

	// if ! visible at all, just skip sons.
	if(!visible)
		return;
	else
	{
		if(unspecified)
		{
			// clip the sons individually 
			traverseSons();
		}
		else
		{
			// udpdate the sons, but don't clip, because we know they are fully visible.
			clipTrav->ForceNoFrustumClip= true;
			traverseSons();
			clipTrav->ForceNoFrustumClip= false;
		}
	}
}


} // NL3D
