/** \file scene.cpp
 * <File description>
 *
 * $Id: scene.cpp,v 1.5 2000/11/07 15:34:45 berenguier Exp $
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

#include "nel/3d/scene.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/light_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/transform.h"
#include "nel/3d/camera.h"
#include "nel/3d/driver.h"
using namespace std;
using namespace NLMISC;


namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

	
void	CScene::registerBasics()
{
	CTransform::registerBasic();
	CCamera::registerBasic();
}

	
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CScene::CScene()
{
	HrcTrav= NULL;
	ClipTrav= NULL;
	LightTrav= NULL;
	RenderTrav= NULL;

	Root= NULL;
	// TODO: init NULL ligthgroup root.
}
// ***************************************************************************
CScene::~CScene()
{
	RenderTraversals.clear();

	// Delete only the 4 default Traversals (owned by CScene), and the 4 default Root.
	delete	HrcTrav;
	delete	ClipTrav;
	delete	LightTrav;
	delete	RenderTrav;

	delete Root;
	// TODO: delete the ligthgroup root.
}
// ***************************************************************************
void	CScene::initDefaultTravs()
{
	// Add the 4 default traversals.
	HrcTrav= new CHrcTrav;
	ClipTrav= new CClipTrav;
	LightTrav= new CLightTrav;
	RenderTrav= new CRenderTrav;

	// Register them to the scene.
	addTrav(HrcTrav);
	addTrav(ClipTrav);
	addTrav(LightTrav);
	addTrav(RenderTrav);
}
// ***************************************************************************
void	CScene::initDefaultRoots()
{
	// Create and set root the default models.
	Root= static_cast<CTransform*>(createModel(TransformId));
	HrcTrav->setRoot(Root);
	ClipTrav->setRoot(Root);

	// TODO: create / setRoot the lightgroup.
}

// ***************************************************************************
void	CScene::addTrav(ITrav *v)
{
	nlassert(v);
	sint	order=0;

	ITravScene	*sv= dynamic_cast<ITravScene*>(v);
	if(sv)	order= sv->getRenderOrder();

	// If ok, add it to the render traversal list.
	if(order)
	{
		RenderTraversals.insert( TravMap::value_type(order, sv) );
	}

	// And register it normally.
	CMOT::addTrav(v);
}
// ***************************************************************************
void	CScene::render(bool	doHrcPass)
{
	nlassert(CurrentCamera);

	// Use the camera to setup Clip / Render pass.
	float left, right, bottom, top, znear, zfar;
	CurrentCamera->getFrustum(left, right, bottom, top, znear, zfar);

	ClipTrav->setFrustum(left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
	ClipTrav->setCamMatrix(CurrentCamera->getMatrix());

	RenderTrav->setFrustum(left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
	RenderTrav->setCamMatrix(CurrentCamera->getMatrix());

	// Set the renderTrav for cliptrav.
	ClipTrav->setRenderTrav(RenderTrav);

	// For all render traversals, traverse them (except the Hrc one), in ascending order.
	TravMap::iterator	it;
	for(it= RenderTraversals.begin(); it!= RenderTraversals.end(); it++)
	{
		ITravScene	*trav= (*it).second;
		if(!doHrcPass && HrcTravId==trav->getClassId())
			continue;

		// Go!
		trav->traverse();
	}
}

// ***************************************************************************
void	CScene::clear(CRGBA col)
{
	RenderTrav->getDriver()->clear2D(col);
	RenderTrav->getDriver()->clearZBuffer();
}


// ***************************************************************************
void	CScene::setCam(const CSmartPtr<CCamera>	&cam)
{
	CurrentCamera= cam;
}

// ***************************************************************************
void	CScene::setDriver(IDriver *drv)
{
	RenderTrav->setDriver(drv);
}


}


