/** \file render_trav.cpp
 * <File description>
 *
 * $Id: render_trav.cpp,v 1.13 2002/02/06 16:54:56 berenguier Exp $
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
#include "3d/light.h"
#include "3d/skeleton_model.h"

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

	_CacheLightContribution= NULL;

	// Default light Setup.
	LightingSystemEnabled= false;
	AmbientGlobal= CRGBA(50, 50, 50);
	SunAmbient= CRGBA::Black;
	SunDiffuse= SunSpecular= CRGBA::White;
	_SunDirection.set(0, 0.5, -0.5);
	_SunDirection.normalize();
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

	// reset the light setup, and set global ambient.
	resetLightSetup();

	// Sort the observers by distance from camera
	// This is done here and not in the addRenderObs because of the LoadBalancing traversal which can modify
	// the transparency flag (multi lod for instance)

	// clear the OTs.
	OrderOpaqueList.reset();
	OrderTransparentList.reset();

	// fill the OTs.
	std::vector<IBaseRenderObs*>::iterator it = RenderList.begin();
	uint32 nNbObs = RenderList.size();
	IBaseRenderObs *pObs;
	CTransform *pTransform;
	float rPseudoZ, rPseudoZ2;
	for( uint32 i = 0; i < nNbObs; ++i )
	{
		pObs = *it;
		pTransform = pObs->getTransformModel();

		if(pTransform!=NULL)
		{
			CTransformHrcObs	*trHrcObs= safe_cast<CTransformHrcObs*>(pObs->HrcObs);

			// If the object is binded to a skeleton (skined or sticked, or sticked indirectly), 
			// get the skeleton WM.
			if( trHrcObs->_AncestorSkeletonModel )
				rPseudoZ = (trHrcObs->_AncestorSkeletonModel->getWorldMatrix().getPos() - CamPos).norm();
			// else get the object WM.
			else
				rPseudoZ = (trHrcObs->WorldMatrix.getPos() - CamPos).norm();

			// rPseudoZ from 0.0 -> 1.0
			rPseudoZ =  sqrtf( rPseudoZ / this->Far );

			if( pTransform->isOpaque() )
			{
				rPseudoZ2 = rPseudoZ * OrderOpaqueList.getSize();
				clamp( rPseudoZ2, 0.0f, OrderOpaqueList.getSize() - 1 );
				OrderOpaqueList.insert( (uint32)rPseudoZ2, pObs );
			}
			if( pTransform->isTransparent() )
			{
				rPseudoZ2 = rPseudoZ * OrderTransparentList.getSize();
				rPseudoZ2 = OrderTransparentList.getSize() - rPseudoZ2;
				clamp( rPseudoZ2, 0.0f, OrderTransparentList.getSize() - 1 );
				OrderTransparentList.insert( pTransform->getOrderingLayer(), pObs, (uint32)rPseudoZ2 );
			}
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
	OrderTransparentList.begin(_LayersRenderingOrder);
	while( OrderTransparentList.get() != NULL )
	{
		pBRO = OrderTransparentList.get();
		pBRO->traverse(NULL);
		OrderTransparentList.next();
	}


	// END!
	// clean: reset the light setup
	resetLightSetup();

}
// ***************************************************************************
void		CRenderTrav::clearRenderList()
{
	RenderList.clear();
}
// ***************************************************************************
void		CRenderTrav::addRenderObs(IBaseRenderObs *o)
{
	RenderList.push_back(o);
}


// ***************************************************************************
void		CRenderTrav::setSunDirection(const CVector &dir)
{
	_SunDirection= dir;
	_SunDirection.normalize();
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


// ***************************************************************************
// ***************************************************************************
// LightSetup
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CRenderTrav::resetLightSetup()
{
	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;

	uint i;

	// Disable all lights.
	for(i=0; i<Driver->getMaxLight(); i++)
	{
		Driver->enableLight(i, false);
	}


	// setup the precise cache, and setup lights according to this cache?
	// setup blackSun (factor==0).
	_LastSunFactor= 0;
	CLight		light;
	light.setupDirectional(CRGBA::Black, CRGBA::Black, CRGBA::Black, _SunDirection);
	Driver->setLight(0, light);
	// setup NULL point lights (=> cache will fail), so no need to setup other lights in Driver.
	for(i=0; i<NL3D_MAX_LIGHT_CONTRIBUTION; i++)
	{
		_LastPointLight[i]= NULL;
	}


	// Set the global ambientColor
	Driver->setAmbientColor(AmbientGlobal);

	// clear the cache.
	_CacheLightContribution= NULL;
	_NumLightEnabled= 0;
}


// ***************************************************************************
void		CRenderTrav::changeLightSetup(CLightContribution	*lightContribution)
{
	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;

	uint		i;
	CLight		light;

	// if same lightContribution, no-op.
	if(_CacheLightContribution == lightContribution)
		return;
	// else, must setup the lights into driver.
	else
	{
		// if the setup is !NULL
		if(lightContribution)
		{
			// Setup the directionnal Sunlight.
			//-----------
			// expand 0..255 to 0..256, to avoid loss of precision.
			uint	ufactor= lightContribution->SunContribution;
			//	different SunLight as in cache ??
			//	NB: sunSetup can't change during renderPass, so need only to test factor.
			if(ufactor != _LastSunFactor)
			{
				// cache (before expanding!!)
				_LastSunFactor= ufactor;

				// expand to 0..256.
				ufactor+= ufactor>>7;	// add 0 or 1.
				// modulate color with factor of the lightContribution.
				CRGBA	sunAmbient, sunDiffuse, sunSpecular;
				sunAmbient.modulateFromuiRGBOnly(SunAmbient, ufactor);
				sunDiffuse.modulateFromuiRGBOnly(SunDiffuse, ufactor);
				sunSpecular.modulateFromuiRGBOnly(SunSpecular, ufactor);
				// setup driver light
				light.setupDirectional(sunAmbient, sunDiffuse, sunSpecular, _SunDirection);
				Driver->setLight(0, light);
			}


			// Setup other point lights
			//-----------
			uint	plId=0;
			// for the list of light.
			while(lightContribution->PointLight[plId]!=NULL)
			{
				CPointLight		*pl= lightContribution->PointLight[plId];
				uint			inf= lightContribution->Factor[plId];

				// different PointLight setup than in cache??
				// NB: pointLight setup can't change during renderPass, so need only to test pointer and factor.
				if( pl!=_LastPointLight[plId] || inf!=_LastPointLightFactor[plId])
				{
					// need to resetup the light. Cache it.
					_LastPointLight[plId]= pl;
					_LastPointLightFactor[plId]= inf;

					// compute the driver light
					pl->setupDriverLight(light, inf);

					// setup driver. decal+1 because of sun.
					Driver->setLight(plId+1, light);
				}

				// next light?
				plId++;
				if(plId>=NL3D_MAX_LIGHT_CONTRIBUTION)
					break;
			}


			// Disable olds, enable news, and cache.
			//-----------
			// count new number of light enabled.
			uint	newNumLightEnabled;
			// number of pointLight + the sun 
			newNumLightEnabled= plId + 1;

			// enable lights which are used now and were not before.
			for(i=_NumLightEnabled; i<newNumLightEnabled; i++)
			{
				Driver->enableLight(i, true);
			}

			// disable lights which are no more used.
			for(i=newNumLightEnabled; i<_NumLightEnabled; i++)
			{
				Driver->enableLight(i, false);
			}

			// cache the setup.
			_CacheLightContribution = lightContribution;
			_NumLightEnabled= newNumLightEnabled;
		}
		else
		{
			// Disable old lights, and cache.
			//-----------
			// disable lights which are no more used.
			for(i=0; i<_NumLightEnabled; i++)
			{
				Driver->enableLight(i, false);
			}

			// cache the setup.
			_CacheLightContribution = NULL;
			_NumLightEnabled= 0;
		}


	}
}


}
