/** \file scene.cpp
 * <File description>
 *
 * $Id: scene.cpp,v 1.29 2001/06/11 09:25:58 besson Exp $
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
#include "nel/3d/anim_detail_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/transform.h"
#include "nel/3d/camera.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/driver.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/shape_bank.h"
#include "nel/3d/skeleton_model.h"
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
	CMeshInstance::registerBasic();
	CLandscapeModel::registerBasic();
	CTransformShape::registerBasic();
	CSkeletonModel::registerBasic();
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
	AnimDetailTrav= NULL;
	RenderTrav= NULL;

	_ShapeBank = NULL;

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

	// Delete only the 5 default Traversals (owned by CScene).
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

	if (AnimDetailTrav != NULL)
	{
		delete	AnimDetailTrav;
		AnimDetailTrav= NULL;
	}

	if (RenderTrav != NULL)
	{
		delete	RenderTrav;
		RenderTrav= NULL;
	}
	_ShapeBank = NULL;
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
	AnimDetailTrav= new CAnimDetailTrav;
	RenderTrav= new CRenderTrav;

	// Register them to the scene.
	addTrav(HrcTrav);
	addTrav(ClipTrav);
	addTrav(LightTrav);
	addTrav(AnimDetailTrav);
	addTrav(RenderTrav);
}
// ***************************************************************************
void	CScene::initDefaultRoots()
{
	// Create and set root the default models.
	Root= static_cast<CTransform*>(createModel(TransformId));
	HrcTrav->setRoot(Root);
	ClipTrav->setRoot(Root);
	AnimDetailTrav->setRoot(Root);

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

	// setup basic camera.
	ClipTrav->setFrustum(left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());

	RenderTrav->setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
	RenderTrav->setViewport (_Viewport);

	// Set the renderTrav for cliptrav.
	ClipTrav->setRenderTrav(RenderTrav);

	// For all render traversals, traverse them (except the Hrc one), in ascending order.
	TTravMap::iterator	it;
	for(it= RenderTraversals.begin(); it!= RenderTraversals.end(); it++)
	{
		ITravScene	*trav= (*it).second;
		// maybe don't traverse HRC pass.
		if(doHrcPass || HrcTravId!=trav->getClassId())
			// Go!
			trav->traverse();

		// if HrcTrav done, set World Camera matrix for Clip and Render.
		if(HrcTravId==trav->getClassId())
		{
			CTransformHrcObs	*camObs= (CTransformHrcObs*)CMOT::getModelObs(CurrentCamera, HrcTravId);
			ClipTrav->setCamMatrix(camObs->WorldMatrix);
			RenderTrav->setCamMatrix (camObs->WorldMatrix);
		}
	}

	// Instance handling
	// Parse all the waiting instance
	TWaitingInstancesMMap::iterator wimmIt = _WaitingInstances.begin();
	while( wimmIt != _WaitingInstances.end() )
	{
		if( _ShapeBank->isPresent( wimmIt->first ) )
		{
			// Then create a reference to the shape
			*(wimmIt->second) = _ShapeBank->addRef( wimmIt->first )->createInstance(*this);
			// Delete the waiting instance
			TWaitingInstancesMMap::iterator	itDel= wimmIt;
			++wimmIt;
			_WaitingInstances.erase(itDel);
		}
		else
		{
			++wimmIt;
		}
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

void CScene::setShapeBank(CShapeBank*pShapeBank)
{
	_ShapeBank = pShapeBank;
}

// ***************************************************************************

CTransformShape	*CScene::createInstance(const string &shapeName)
{
	// We must attach a bank to the scene (a ShapeBank handle the shape caches and 
	// the creation/deletion of the instances)
	nlassert( _ShapeBank != NULL );
	
	// If the shape is not present in the bank
	if( !_ShapeBank->isPresent( shapeName ) )
	{
		// Load it from file
		_ShapeBank->load( shapeName );
		if( !_ShapeBank->isPresent( shapeName ) )
		{
			return NULL;
		}
	}
	// Then create a reference to the shape
	return _ShapeBank->addRef( shapeName )->createInstance(*this);
}

// ***************************************************************************

void CScene::createInstanceAsync(const string &shapeName, CTransformShape **pInstance)
{
	// We must attach a bank to the scene (a ShapeBank handle the shape caches and 
	// the creation/deletion of the instances)
	nlassert( _ShapeBank != NULL );
	*pInstance = NULL;
	// Add the instance request
	_WaitingInstances.insert(TWaitingInstancesMMap::value_type(shapeName,pInstance));
	// If the shape is not present in the bank
	if( !_ShapeBank->isPresent( shapeName ) )
	{
		// Load it from file asynchronously
		_ShapeBank->loadAsync( shapeName, getDriver() );
	}
}

// ***************************************************************************

void CScene::deleteInstance(CTransformShape *pTrfmShp)
{
	IShape *pShp = NULL;
	if( pTrfmShp == NULL )
		return;

	pShp = pTrfmShp->Shape;
	
	deleteModel( pTrfmShp );

	if(pShp)
	{
		// Even if model already deleted by smarptr the release function works
		_ShapeBank->release( pShp );
	}
	
}

}


