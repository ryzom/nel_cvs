/** \file scene.cpp
 * <File description>
 *
 * $Id: scene.cpp,v 1.19 2001/03/16 16:50:14 berenguier Exp $
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
#include "nel/3d/landscape_model.h"
#include "nel/3d/driver.h"
#include "nel/3d/transform_shape.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
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
	CLandscapeModel::registerBasic();
	CTransformShape::registerBasic();
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
void	CScene::release()
{
	// First, delete models and un-register traversals.
	CMOT::release();

	// Unlink the rendertrav.
	RenderTraversals.clear();

	// Delete only the 4 default Traversals (owned by CScene), and the 4 default Root.
	if (HrcTrav != NULL)
	{
		delete	HrcTrav;
		HrcTrav= NULL;
	}

	if (ClipTrav != NULL)
	{
		delete	ClipTrav;
		ClipTrav= NULL;
	}

	if (LightTrav != NULL)
	{
		delete	LightTrav;
		LightTrav= NULL;
	}

	if (RenderTrav != NULL)
	{
		delete	RenderTrav;
		RenderTrav= NULL;
	}

	Root= NULL;
	CurrentCamera= NULL;
}
// ***************************************************************************
CScene::~CScene()
{
	release();
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
		RenderTraversals.insert( TTravMap::value_type(order, sv) );
	}

	// And register it normally.
	CMOT::addTrav(v);
}
// ***************************************************************************
void	CScene::render(bool	doHrcPass)
{
	nlassert(CurrentCamera);

	// validate models.
	CMOT::validateModels();

	// Use the camera to setup Clip / Render pass.
	float left, right, bottom, top, znear, zfar;
	CurrentCamera->getFrustum(left, right, bottom, top, znear, zfar);

	ClipTrav->setFrustum(left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
	ClipTrav->setCamMatrix(CurrentCamera->getMatrix());

	RenderTrav->setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
	RenderTrav->setCamMatrix (CurrentCamera->getMatrix());
	RenderTrav->setViewport (_Viewport);

	// Set the renderTrav for cliptrav.
	ClipTrav->setRenderTrav(RenderTrav);

	// For all render traversals, traverse them (except the Hrc one), in ascending order.
	TTravMap::iterator	it;
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
void	CScene::setDriver(IDriver *drv)
{
	if (RenderTrav != NULL)
		RenderTrav->setDriver(drv);
}

// ***************************************************************************
IDriver	*CScene::getDriver() const
{
	if (RenderTrav != NULL)
		return RenderTrav->getDriver();
	else
		return NULL;
}


// ***************************************************************************
// ***************************************************************************
// Shape mgt.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void	CScene::addShape(const std::string &shapeName, CSmartPtr<IShape> shape)
{
	ShapeMap[shapeName]= shape;
}
// ***************************************************************************
void	CScene::delShape(const std::string &shapeName)
{
	ShapeMap.erase(shapeName);
}
// ***************************************************************************
CTransformShape	*CScene::createInstance(const std::string &shapeName)
{
	TShapeMap::iterator		it;
	it= ShapeMap.find(shapeName);
	if(it==ShapeMap.end())
	{
		try
		{
			CShapeStream	mesh;
			CIFile meshfile(CPath::lookup(shapeName));
			meshfile.serial( mesh );
			meshfile.close();

			// Add the shape to the map.
			CSmartPtr<IShape>	spShape= mesh.getShapePointer();
			addShape(shapeName, spShape);
			it= ShapeMap.find(shapeName);
			nlassert(it!=ShapeMap.end());
		}
		catch(EPathNotFound &)
		{
			return NULL;
		}
	}

	return (*it).second->createInstance(*this);
}


}


