/* scene.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: scene.cpp,v 1.3 2000/10/10 16:12:56 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */


#include "nel/3d/scene.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/light_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/transform.h"
#include "nel/3d/camera.h"
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
void	CScene::setCam(const CSmartPtr<CCamera>	&cam)
{
	CurrentCamera= cam;
}

}


