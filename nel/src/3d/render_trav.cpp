/** \file render_trav.cpp
 * <File description>
 *
 * $Id: render_trav.cpp,v 1.10 2001/07/05 09:38:49 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "3d/render_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/light_trav.h"
#include "3d/driver.h"

#include "3d/transform.h"

using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
// ***************************************************************************
// CRenderTrav
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CRenderTrav::CRenderTrav()
{
	RenderList.reserve(1024);
	OrderOpaqueList.init(1024);
	OrderTransparentList.init(1024);
	Driver = NULL;
	_CurrentPassOpaque = true;
}
// ***************************************************************************
IObs		*CRenderTrav::createDefaultObs() const
{
	return new CDefaultRenderObs;
}
// ***************************************************************************
void		CRenderTrav::traverse()
{
	ITravCameraScene::update();

	// Bind to Driver.
	getDriver()->setFrustum(Left, Right, Bottom, Top, Near, Far, Perspective);
	getDriver()->setupViewMatrix(ViewMatrix);
	getDriver()->setupViewport(_Viewport);


	// Sort the observers by distance from camera
	// This is done here and not in the addRenderObs because of the LoadBalancing traversal which can modify
	// the transparency flag (multi lod for instance)
	std::vector<IBaseRenderObs*>::iterator it = RenderList.begin();
	uint32 nNbObs = RenderList.size();
	IBaseRenderObs *pObs;
	CTransform *pTransform;
	float rPseudoZ, rPseudoZ2;
	for( uint32 i = 0; i < nNbObs; ++i )
	{
		pObs = *it;
		pTransform = pObs->getTransformModel();

		rPseudoZ = (pObs->HrcObs->WorldMatrix.getPos() - CamPos).norm();
		rPseudoZ =  sqrtf( rPseudoZ / this->Far );
		// rPseudoZ from 0.0 -> 1.0

		if( ( pTransform != NULL ) && ( pTransform->isOpaque() ) )
		{
			rPseudoZ2 = rPseudoZ * OrderOpaqueList.getSize();
			clamp( rPseudoZ2, 0.0f, OrderOpaqueList.getSize() - 1 );
			OrderOpaqueList.insert( (uint32)rPseudoZ2, pObs );
		}
		if( ( pTransform != NULL ) && ( pTransform->isTransparent() ) )
		{
			rPseudoZ2 = rPseudoZ * OrderTransparentList.getSize();
			rPseudoZ2 = OrderTransparentList.getSize() - rPseudoZ2;
			clamp( rPseudoZ2, 0.0f, OrderOpaqueList.getSize() - 1 );
			OrderTransparentList.insert( (uint32)rPseudoZ2, pObs );
		}
		++it;
	}



	// Don't Clear screen, leave it to caller.

	// First traverse the root.
	if(Root)
		Root->traverse(NULL);

	// Then traverse the render list.
	// Render the opaque materials
	_CurrentPassOpaque = true;
	OrderOpaqueList.begin();
	IBaseRenderObs *pBRO;
	while( OrderOpaqueList.get() != NULL )
	{
		pBRO = OrderOpaqueList.get();
		pBRO->traverse(NULL);
		OrderOpaqueList.next();
	}

	 // Render transparent materials
	_CurrentPassOpaque = false;
	OrderTransparentList.begin();
	while( OrderTransparentList.get() != NULL )
	{
		pBRO = OrderTransparentList.get();
		pBRO->traverse(NULL);
		OrderTransparentList.next();
	}

}
// ***************************************************************************
void		CRenderTrav::clearRenderList()
{
	RenderList.clear();
	OrderOpaqueList.reset();
	OrderTransparentList.reset();
}
// ***************************************************************************
void		CRenderTrav::addRenderObs(IBaseRenderObs *o)
{
	RenderList.push_back(o);
}



// ***************************************************************************
// ***************************************************************************
// IBaseClipObs
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		IBaseRenderObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
	nlassert( dynamic_cast<IBaseLightObs*> (getObs(LightTravId)) );
	LightObs= static_cast<IBaseLightObs*> (getObs(LightTravId));
}


}
