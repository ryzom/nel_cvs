/** \file quad_grid_clip_cluster.h
 * <File description>
 *
 * $Id: quad_grid_clip_cluster.h,v 1.2 2002/06/12 12:26:57 berenguier Exp $
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


namespace NL3D 
{


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		QuadGridClipClusterId=NLMISC::CClassId(0x31d517aa, 0x2c4357a0);


// ***************************************************************************
/**
 * A cluster of object for fast BBox clip. This is a simple model to not test sons if they are totaly
 *	clipped.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CQuadGridClipCluster : public IModel
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

	void		extendCluster(const NLMISC::CAABBox &worldBBox);

	void		setDistMax(float d) {_DistMax= d;}

	// never need to update, so unlink me from ValidateList.
	virtual void update();

protected:
	/// Constructor
	CQuadGridClipCluster() {_Empty= true; _DistMax= -1;}
	/// Destructor
	virtual ~CQuadGridClipCluster() {}

private:
	static IModel	*creator() {return new CQuadGridClipCluster;}
	friend class	CQuadGridClipClusterClipObs;

	NLMISC::CAABBox		_BBox;
	bool				_Empty;
	float				_DistMax;
	float				_Radius;
};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to do nothing
 * - implement the traverse() method
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CQuadGridClipClusterClipObs : public IBaseClipObs
{
	bool		_LastClipWasDistMaxClip;
	bool		_LastClipWasFrustumClip;

public:
	CQuadGridClipClusterClipObs();


	// not used by traverse().
	virtual	bool	isRenderable() const {return false;}
	virtual	bool	clip(IBaseClipObs *caller) {return true;}


	/// \name The base doit method.
	//@{
	/// clip the bbox of the model and traverse sons if clip result uspecified.
	virtual	void	traverse(IObs *caller);
	//@}

	static IObs	*creator() {return new CQuadGridClipClusterClipObs;}

};


} // NL3D


#endif // NL_QUAD_GRID_CLIP_CLUSTER_H

/* End of quad_grid_clip_cluster.h */
