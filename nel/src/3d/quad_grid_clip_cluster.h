/** \file quad_grid_clip_cluster.h
 * <File description>
 *
 * $Id: quad_grid_clip_cluster.h,v 1.4 2003/03/20 15:00:03 berenguier Exp $
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

#ifndef NL_QUAD_GRID_CLIP_CLUSTER_H
#define NL_QUAD_GRID_CLIP_CLUSTER_H

#include "nel/misc/types_nl.h"
#include "3d/mot.h"
#include "3d/clip_trav.h"
#include "nel/misc/aabbox.h"
#include "3d/fast_ptr_list.h"


namespace NL3D 
{


class	CTransformShapeClipObs;

// ***************************************************************************
/**
 * A cluster of object for fast BBox clip.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CQuadGridClipCluster
{
public:
	// For insertion in the QuadGridClipManager
	CFastPtrListNode		ListNode;


public:
	/// Constructor
	CQuadGridClipCluster(float distMax);
	~CQuadGridClipCluster();

	void		addModel(const NLMISC::CAABBox &worldBBox, CTransformShapeClipObs *clipObs);
	// NB: the BBox is not recomputed.
	void		removeModel(CTransformShapeClipObs *clipObs);

	void		clip(CClipTrav *clipTrav);

	// NB it is possible that getNumChildren()==0 and isEmpty()==false!!
	bool					isEmpty() const {return _Empty;}
	const NLMISC::CAABBox	&getBBox() const {return _BBox;}
	sint					getNumChildren() const {return _Models.size();}

	void		resetSons(CClipTrav *clipTrav);

protected:

	NLMISC::CAABBox							_BBox;
	NLMISC::CAABBoxExt						_BBoxExt;
	float									_DistMax;
	float									_SqrDistMaxRadius;
	CFastPtrList<CTransformShapeClipObs>	_Models;
	bool									_Empty;
	bool									_TestDistMax;

	bool									_LastClipWasFrustumClip;

	void		clipSons();

};


} // NL3D


#endif // NL_QUAD_GRID_CLIP_CLUSTER_H

/* End of quad_grid_clip_cluster.h */
