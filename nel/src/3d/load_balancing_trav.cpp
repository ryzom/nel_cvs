/** \file load_balancing_trav.cpp
 * The LoadBalancing traversal.
 *
 * $Id: load_balancing_trav.cpp,v 1.6 2002/02/28 12:59:49 besson Exp $
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

#include "3d/load_balancing_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "nel/misc/common.h"


using namespace NLMISC;


// ***************************************************************************
#define	NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER	50

namespace NL3D 
{

// ***************************************************************************
IObs				*CLoadBalancingTrav::createDefaultObs() const
{
	return new CDefaultLoadBalancingObs;
}


// ***************************************************************************
CLoadBalancingTrav::CLoadBalancingTrav()
{
	_NbFaceWanted= 20000;
	PolygonBalancingMode= PolygonBalancingOff;
	_PrecPolygonBalancingMode= PolygonBalancingOff;

	_ValueSmoother.init(NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER);
}


// ***************************************************************************
void	IBaseLoadBalancingObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
}


// ***************************************************************************
void				CLoadBalancingTrav::traverse()
{
	ITravCameraScene::update();

	// Traverse the graph 2 times.

	// 1st pass, count NBFaces drawed.
	//================
	_LoadPass= 0;
	// reset NbFacePass0.
	NbFacePass0= 0;
	// count NbFacePass0.
	traverseVisibilityList();

	// Compute ratio
	switch(PolygonBalancingMode)
	{
	case PolygonBalancingOff:
		_FaceRatio= 1;
		break;
	case PolygonBalancingOn	:
		if(NbFacePass0!=0)
			_FaceRatio= (float)_NbFaceWanted / NbFacePass0;
		else
			_FaceRatio= 1;
		break;
	case PolygonBalancingClamp:
		if(NbFacePass0!=0)
			_FaceRatio= (float)_NbFaceWanted / NbFacePass0;
		else
			_FaceRatio= 1;
		clamp(_FaceRatio, 0, 1);
		break;
	};

	// smooth the value.
	// if change of PolygonBalancingMode, reset the _ValueSmoother.
	if(PolygonBalancingMode!=_PrecPolygonBalancingMode)
	{
		_ValueSmoother.init(NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER);
		_PrecPolygonBalancingMode= PolygonBalancingMode;
	}
	// if not PolygonBalancingOff, smooth the ratio.
	if(PolygonBalancingMode!=PolygonBalancingOff)
	{
		_ValueSmoother.addValue(_FaceRatio);
		_FaceRatio= _ValueSmoother.getSmoothValue();
	}


	// 2nd pass, compute Faces that will be drawed.
	//================
	_LoadPass= 1;
	traverseVisibilityList();

}


// ***************************************************************************
void				CLoadBalancingTrav::traverseVisibilityList()
{
	// Traverse all nodes of the visibility list.
	uint	nObs= _ClipTrav->numVisibleObs();
	for(uint i=0; i<nObs; i++)
	{
		IBaseClipObs	*clipObs= _ClipTrav->getVisibleObs(i);
		clipObs->LoadBalancingObs->traverse(NULL);
	}
}


// ***************************************************************************
float				CLoadBalancingTrav::computeModelNbFace(float faceIn, float camDist)
{
	return faceIn * _FaceRatio;
}


// ***************************************************************************
float				CLoadBalancingTrav::getNbFaceAsked () const
{
	return NbFacePass0;
}


} // NL3D
