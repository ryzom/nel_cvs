/** \file quad_grid_clip_cluster.cpp
 * <File description>
 *
 * $Id: quad_grid_clip_cluster.cpp,v 1.6 2003/03/26 10:20:55 berenguier Exp $
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
#include "nel/misc/hierarchical_timer.h"
#include "3d/transform_shape.h"
#include "3d/cluster.h"


using namespace NLMISC;
using namespace std;


namespace NL3D 
{


H_AUTO_DECL( NL3D_QuadClip_ClusterClip );


// ***************************************************************************
CQuadGridClipCluster::CQuadGridClipCluster(float distMax) : _DistMax(distMax)
{
	_Empty= true;
	_TestDistMax= _DistMax!=-1;
}

// ***************************************************************************
CQuadGridClipCluster::~CQuadGridClipCluster()
{
	nlassert(_Models.empty());
}

// ***************************************************************************
void		CQuadGridClipCluster::addModel(const NLMISC::CAABBox &worldBBox, CTransformShape *model)
{
	// check not already inserted
	nlassert(!model->_QuadClusterListNode.isLinked());

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
	_SqrDistMaxRadius= sqr(_DistMax + _BBox.getRadius());

	// update bboxExt
	_BBoxExt= _BBox;

	// Add the model
	_Models.insert(model, &model->_QuadClusterListNode);
}


// ***************************************************************************
void		CQuadGridClipCluster::removeModel(CTransformShape *model)
{
	_Models.erase(&model->_QuadClusterListNode);
}


// ***************************************************************************
void		CQuadGridClipCluster::clip(CClipTrav *clipTrav)
{
	H_AUTO_USE( NL3D_QuadClip_ClusterClip );

	// First, clip DistMax.
	if( _TestDistMax )
	{
		CVector		c= _BBoxExt.getCenter();
		float		sqrDist= (c - clipTrav->CamPos).sqrnorm();
		// it the bbox is entirely out the distMax
		if( sqrDist > _SqrDistMaxRadius )
		{
			// quit
			return;
		}
	}

	// Then clip against pyramid
	bool	unspecified= false;
	bool	visible= true;
	for(sint i=0;i<(sint)clipTrav->WorldPyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		if(!_BBoxExt.clipBack(clipTrav->WorldPyramid[i]))
		{
			visible= false;
			break;
		}
		// else test is the bbox is partially or fully in the plane
		else if(!unspecified)
		{
			// if clipFront AND clipBack, it means partially.
			if(_BBoxExt.clipFront(clipTrav->WorldPyramid[i]))
				unspecified= true;
		}
	}

	// if visible, parse sons
	if(visible)
	{
		// clip sons.
		if(unspecified)
		{
			H_AUTO( NL3D_QuadClip_SonsClip );

			// clip the sons individually 
			clipSons();
		}
		else
		{
			H_AUTO( NL3D_QuadClip_SonsShowNoClip );

			// udpdate the sons, but don't clip, because we know they are fully visible.
			clipTrav->ForceNoFrustumClip= true;
			clipSons();
			clipTrav->ForceNoFrustumClip= false;
		}
	}
}

// ***************************************************************************
void		CQuadGridClipCluster::clipSons()
{
	CTransformShape	** pModel= _Models.begin();
	uint	nSons= _Models.size();
	for(;nSons>0;nSons--, pModel++)
	{
		(*pModel)->traverseClip(NULL);
	}
}

// ***************************************************************************
void		CQuadGridClipCluster::resetSons(CClipTrav *clipTrav)
{
	// clean up model list
	CTransformShape	** pModel= _Models.begin();
	uint	nSons= _Models.size();
	for(;nSons>0;nSons--, pModel++)
	{
		// link the model to the rootCluster
		clipTrav->RootCluster->clipAddChild(*pModel);
	}
	// unlink all my sons from me
	_Models.clear();

}


} // NL3D
