/** \file render_trav.cpp
 * <File description>
 *
 * $Id: render_trav.cpp,v 1.35 2002/08/14 12:39:25 berenguier Exp $
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

#include "std3d.h"

#include "3d/render_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/light_trav.h"
#include "3d/driver.h"
#include "3d/light.h"
#include "3d/skeleton_model.h"
#include "3d/scene.h"
#include "3d/coarse_mesh_manager.h"
#include "3d/lod_character_manager.h"
#include "nel/misc/hierarchical_timer.h"

#include "3d/transform.h"
#include "3d/fast_floor.h"
#include "3d/mesh_skin_manager.h"

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

	_StrongestLightTouched = true;

	_MeshSkinManager= NULL;
}
// ***************************************************************************
IObs		*CRenderTrav::createDefaultObs() const
{
	return new CDefaultRenderObs;
}
// ***************************************************************************
void		CRenderTrav::traverse()
{
	H_AUTO( NL3D_TravRender );

	ITravCameraScene::update();

	// Bind to Driver.
	getDriver()->setFrustum(Left, Right, Bottom, Top, Near, Far, Perspective);
	// Use setupViewMatrixEx() for ZBuffer precision.
	getDriver()->setupViewMatrixEx(ViewMatrix, CamPos);
	getDriver()->setupViewport(_Viewport);

	// reset the light setup, and set global ambient.
	resetLightSetup();


	// reset the Skin manager, if needed
	if(_MeshSkinManager)
	{
		if(Driver!=_MeshSkinManager->getDriver())
		{
			_MeshSkinManager->release();
			_MeshSkinManager->init(Driver, NL3D_MESH_SKIN_MANAGER_VERTEXFORMAT, NL3D_MESH_SKIN_MANAGER_MAXVERTICES);
		}
	}


	// Fill OT with observers, for both Opaque and transparent pass
	// =============================

	// Sort the observers by distance from camera
	// This is done here and not in the addRenderObs because of the LoadBalancing traversal which can modify
	// the transparency flag (multi lod for instance)

	// clear the OTs, and prepare to allocate max element space
	OrderOpaqueList.reset(RenderList.size());
	OrderTransparentList.reset(RenderList.size());

	// fill the OTs.
	IBaseRenderObs		**itRdrObs= NULL;
	uint32				nNbObs = RenderList.size();
	if(nNbObs)	
		itRdrObs= &RenderList[0];
	float	rPseudoZ, rPseudoZ2;

	// Some precalc
	float	OOFar= 1.0f / this->Far;
	uint32	opaqueOtSize= OrderOpaqueList.getSize();
	uint32	opaqueOtMax= OrderOpaqueList.getSize()-1;
	uint32	transparentOtSize= OrderTransparentList.getSize();
	uint32	transparentOtMax= OrderTransparentList.getSize()-1;
	uint32	otId;
	// fast floor
	OptFastFloorBegin();
	// For all rdr observers
	for( ; nNbObs>0; itRdrObs++, nNbObs-- )
	{
		IBaseRenderObs		*pObs= *itRdrObs;
		// Only rdrObserver of transform models can be inserted!! It's a requirement
		CTransform			*pTransform = safe_cast<CTransform*>(pObs->Model);
		CTransformHrcObs	*trHrcObs= safe_cast<CTransformHrcObs*>(pObs->HrcObs);

		// Yoyo: skins are rendered through skeletons, so models WorldMatrix are all good here (even sticked objects)
		rPseudoZ = (trHrcObs->WorldMatrix.getPos() - CamPos).norm();

		// rPseudoZ from 0.0 -> 1.0
		rPseudoZ =  sqrtf( rPseudoZ * OOFar );

		if( pTransform->isOpaque() )
		{
			// since norm, we are sure that rPseudoZ>=0
			rPseudoZ2 = rPseudoZ * opaqueOtSize;
			otId= OptFastFloor(rPseudoZ2);
			otId= min(otId, opaqueOtMax);
			OrderOpaqueList.insert( otId, pObs );
		}
		if( pTransform->isTransparent() )
		{
			// since norm, we are sure that rPseudoZ>=0
			rPseudoZ2 = rPseudoZ * transparentOtSize;
			otId= OptFastFloor(rPseudoZ2);
			otId= min(otId, transparentOtMax);
			// must invert id, because transparent, sort from back to front
			OrderTransparentList.insert( pTransform->getOrderingLayer(), pObs, transparentOtMax-otId );
		}

	}
	// fast floor
	OptFastFloorEnd();


	// Standard traverse (maybe not used)
	// =============================

	// First traverse the root.
	if(Root)
		Root->traverse(NULL);



	// Render Opaque stuff.
	// =============================

	// TestYoyo
	//OrderOpaqueList.reset(0);
	//OrderTransparentList.reset(0);

	// Start LodCharacter Manager render.
	Scene->getLodCharacterManager()->beginRender(getDriver(), CamPos);

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

	/* Render MeshBlock Manager. 
		Some Meshs may be render per block. Interesting to remove VertexBuffer and Material setup overhead.
		Faster if rendered before lods, for ZBuffer optimisation: render first near objects then far. 
		Lods are usually far objects.
	*/
	MeshBlockManager.flush(Driver, Scene, this);


	// End LodCharacter Manager render.
	Scene->getLodCharacterManager()->endRender();


	/* Render Scene CoarseMeshManager. 
		Important to render them at end of Opaque rendering, because coarses instances are created/removed during
		this model opaque rendering pass.
	*/
	// Render dynamic one.
	Scene->getDynamicCoarseMeshManager()->render(Driver);
	// Render static one.
	Scene->getStaticCoarseMeshManager()->render(Driver);


	// Render Transparent stuff.
	// =============================

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
	// =============================

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
void		CRenderTrav::setMeshSkinManager(CMeshSkinManager *msm)
{
	_MeshSkinManager= msm;
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
	{
		// Dont modify Driver lights, but setup default lighting For VertexProgram Lighting.
		_NumLightEnabled= 1;
		// Setup A default directionnal.
		CVector		defDir(-0.5f, 0.0, -0.85f);
		defDir.normalize();
		CRGBA		aday= CRGBA(130,  105,  119);
		CRGBA		dday= CRGBA(238, 225, 204);
		_DriverLight[0].setupDirectional(aday, dday, dday, defDir);

		return;
	}
	else
	{
		uint i;

		// Disable all lights.
		for(i=0; i<Driver->getMaxLight(); i++)
		{
			Driver->enableLight(i, false);
		}


		// setup the precise cache, and setup lights according to this cache?
		// setup blackSun (factor==0).
		_LastSunFactor= 0;
		_LastSunAmbient.set(0,0,0,255);
		_DriverLight[0].setupDirectional(CRGBA::Black, CRGBA::Black, CRGBA::Black, _SunDirection);
		Driver->setLight(0, _DriverLight[0]);
		// setup NULL point lights (=> cache will fail), so no need to setup other lights in Driver.
		for(i=0; i<NL3D_MAX_LIGHT_CONTRIBUTION; i++)
		{
			_LastPointLight[i]= NULL;
		}


		// setup the precise cache, and setup lights according to this cache?
		// setup blackSun (factor==0).
		_LastSunFactor= 0;
		_LastSunAmbient.set(0,0,0,255);
		CLight		light;
		light.setupDirectional(CRGBA::Black, CRGBA::Black, CRGBA::Black, _SunDirection);
		_DriverLight[0].setupDirectional(CRGBA::Black, CRGBA::Black, CRGBA::Black, _SunDirection);
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

		_StrongestLightTouched = true;
	}
}


// ***************************************************************************
void		CRenderTrav::changeLightSetup(CLightContribution	*lightContribution, bool useLocalAttenuation)
{
	// If lighting System disabled, skip
	if(!LightingSystemEnabled)
		return;

	uint		i;

	// if same lightContribution, no-op.
	if(_CacheLightContribution == lightContribution &&  _LastLocalAttenuation == useLocalAttenuation)
		return;
	// else, must setup the lights into driver.
	else
	{
		_StrongestLightTouched = true;
		// if the setup is !NULL
		if(lightContribution)
		{
			// Compute SunAmbient / LocalAmbient
			//-----------
			CRGBA	finalAmbient;
			// Different case if the contribution is frozen or not.
			if(lightContribution->FrozenStaticLightSetup)
			{
				// Any FrozenAmbientLight provided??
				if(lightContribution->FrozenAmbientLight)
					// Take his current (maybe animated) ambient
					finalAmbient= lightContribution->FrozenAmbientLight->getAmbient();
				else
					// Take the sun ones.
					finalAmbient= SunAmbient;
			}
			else
			{
				// must interpolate between SunAmbient and localAmbient
				uint	uAmbFactor= lightContribution->LocalAmbient.A;
				// expand 0..255 to 0..256, to avoid loss of precision.
				uAmbFactor+= uAmbFactor>>7;
				// Blend, but LocalAmbient.r/g/b is already multiplied by a.
				finalAmbient.modulateFromuiRGBOnly(SunAmbient, 256 - uAmbFactor);
				finalAmbient.addRGBOnly(finalAmbient, lightContribution->LocalAmbient);
			}
			// Force Alpha to 255 for good cache test.
			finalAmbient.A= 255;


			// Setup the directionnal Sunlight.
			//-----------
			// expand 0..255 to 0..256, to avoid loss of precision.
			uint	ufactor= lightContribution->SunContribution;
			//	different SunLight as in cache ??
			//	NB: sunSetup can't change during renderPass, so need only to test factor.
			if(ufactor != _LastSunFactor || finalAmbient != _LastSunAmbient)
			{
				// cache (before expanding!!)
				_LastSunFactor= ufactor;
				// Cache final ambient light
				_LastSunAmbient= finalAmbient;

				// expand to 0..256.
				ufactor+= ufactor>>7;	// add 0 or 1.
				// modulate color with factor of the lightContribution.
				CRGBA	sunDiffuse, sunSpecular;
				sunDiffuse.modulateFromuiRGBOnly(SunDiffuse, ufactor);
				sunSpecular.modulateFromuiRGBOnly(SunSpecular, ufactor);
				// setup driver light
				_DriverLight[0].setupDirectional(finalAmbient, sunDiffuse, sunSpecular, _SunDirection);
				Driver->setLight(0, _DriverLight[0]);
			}


			// Setup other point lights
			//-----------
			uint	plId=0;
			// for the list of light.
			while(lightContribution->PointLight[plId]!=NULL)
			{
				CPointLight		*pl= lightContribution->PointLight[plId];
				uint			inf;
				if(useLocalAttenuation)
					inf= lightContribution->Factor[plId];
				else
					inf= lightContribution->AttFactor[plId];

				// different PointLight setup than in cache??
				// NB: pointLight setup can't change during renderPass, so need only to test pointer, 
				// attenuation mode and factor.
				if( pl!=_LastPointLight[plId] || 
					inf!=_LastPointLightFactor[plId] ||
					useLocalAttenuation!=_LastPointLightLocalAttenuation[plId] )
				{
					// need to resetup the light. Cache it.
					_LastPointLight[plId]= pl;
					_LastPointLightFactor[plId]= inf;
					_LastPointLightLocalAttenuation[plId]= useLocalAttenuation;

					// compute the driver light
					if(useLocalAttenuation)
						pl->setupDriverLight(_DriverLight[plId+1], inf);
					else
						// Compute it with user Attenuation
						pl->setupDriverLightUserAttenuation(_DriverLight[plId+1], inf);

					// setup driver. decal+1 because of sun.
					Driver->setLight(plId+1, _DriverLight[plId+1]);
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
			_LastLocalAttenuation= useLocalAttenuation;
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

// ***************************************************************************
// ***************************************************************************
// VertexProgram LightSetup
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CRenderTrav::beginVPLightSetup(uint ctStart, bool supportSpecular, const CMatrix &invObjectWM)
{	
	uint	i;		 
	nlassert(MaxVPLight==4);
	_VPNumLights= min(_NumLightEnabled, (uint)MaxVPLight);
	_VPCurrentCtStart= ctStart;
	_VPSupportSpecular= supportSpecular;
	
	// Prepare Colors (to be multiplied by material)
	//================
	// Ambient. _VPCurrentCtStart+0
	_VPFinalAmbient= AmbientGlobal;
	for(i=0; i<_VPNumLights; i++)
	{
		_VPFinalAmbient+= _DriverLight[i].getAmbiant();		
	}
	// Diffuse. _VPCurrentCtStart+1 to 4
	for(i=0; i<_VPNumLights; i++)
	{
		_VPLightDiffuse[i]= _DriverLight[i].getDiffuse();
	}
	// reset other to 0.
	for(; i<MaxVPLight; i++)
	{
		_VPLightDiffuse[i]= CRGBA::Black;
		Driver->setConstant(_VPCurrentCtStart+1+i, 0.f, 0.f, 0.f, 0.f);
	}
	// Specular. _VPCurrentCtStart+5 to 8 (only if supportSpecular)
	if(supportSpecular)
	{
		for(i=0; i<_VPNumLights; i++)
		{
			_VPLightSpecular[i]= _DriverLight[i].getSpecular();
		}
		// reset other to 0.
		for(; i<MaxVPLight; i++)
		{
			_VPLightSpecular[i]= CRGBA::Black;
			Driver->setConstant(_VPCurrentCtStart+5+i, 0.f, 0.f, 0.f, 0.f);
		}
	}


	// Compute Eye position in Object space.
	CVector		eye= invObjectWM * CamPos;


	// Setup Sun Directionnal light.
	//================
	CVector		lightDir;
	// in objectSpace.
	lightDir= invObjectWM.mulVector(_DriverLight[0].getDirection());
	lightDir.normalize();
	lightDir= -lightDir; 
	if(supportSpecular)
	{
		// Setup lightDir.
		Driver->setConstant(_VPCurrentCtStart+9, &lightDir);
	}
	else
	{
		// Setup lightDir. NB: no specular color!
		Driver->setConstant(_VPCurrentCtStart+5, &lightDir);
	}


	// Setup PointLights
	//================
	CVector		lightPos;
	uint		startPLPos;
	if(supportSpecular)
	{
		// Setup eye in objectSpace for localViewer
		Driver->setConstant(_VPCurrentCtStart+11, &eye);
		// Start at 12.
		startPLPos= 12;
	}
	else
	{
		// Start at 6.
		startPLPos= 6;
	}
	// For all pointLight enabled (other are black: don't matter)
	for(i=1; i<_VPNumLights; i++)
	{
		// Setup position of light.
		CVector		lightPos;
		lightPos= invObjectWM * _DriverLight[i].getPosition();
		Driver->setConstant(_VPCurrentCtStart+startPLPos+(i-1), &lightPos);
	}


	// Must force real light setup at least the first time, in changeVPLightSetupMaterial()
	_VPMaterialCacheDirty= true;
}

// ***************************************************************************
void		CRenderTrav::changeVPLightSetupMaterial(const CMaterial &mat, bool excludeStrongest)
{
	// Must test if at least done one time.
	if(!_VPMaterialCacheDirty)
	{
		// Must test if same as in cache
		if( _VPMaterialCacheEmissive == mat.getEmissive().getPacked() &&
			_VPMaterialCacheAmbient == mat.getAmbient().getPacked() &&
			_VPMaterialCacheDiffuse == mat.getDiffuse().getPacked() )
		{
			// Same Diffuse part, test if same specular if necessary
			if( !_VPSupportSpecular ||
				( _VPMaterialCacheSpecular == mat.getSpecular().getPacked() &&
				  _VPMaterialCacheShininess == mat.getShininess() )  )
			{
				// Then ok, skip.
				return;
			}
		}
	}

	// If not skiped, cache now. cache all for simplification
	_VPMaterialCacheDirty= false;
	_VPMaterialCacheEmissive= mat.getEmissive().getPacked();
	_VPMaterialCacheAmbient= mat.getDiffuse().getPacked();
	_VPMaterialCacheDiffuse= mat.getDiffuse().getPacked();
	_VPMaterialCacheSpecular= mat.getSpecular().getPacked();
	_VPMaterialCacheShininess= mat.getShininess();

	// Setup constants
	CRGBAF	color;
	uint	i;
	CRGBAF	matDiff= mat.getDiffuse();
	CRGBAF	matSpec= mat.getSpecular();
	float	specExp= mat.getShininess();
	
	uint strongestLightIndex = excludeStrongest ? getStrongestLightIndex() : _VPNumLights;

	// setup Ambient + Emissive
	color= _VPFinalAmbient * mat.getAmbient();
	color+= mat.getEmissive();
	Driver->setConstant(_VPCurrentCtStart+0, 1, &color.R);
	

	// is the strongest light is not excluded, its index should have been setup to _VPNumLights

	// setup Diffuse.
	for(i = 0; i < strongestLightIndex; ++i)
	{
		color= _VPLightDiffuse[i] * matDiff;
		Driver->setConstant(_VPCurrentCtStart+1+i, 1, &color.R);
	}

	
	if (i != _VPNumLights)
	{
		color= _VPLightDiffuse[i] * matDiff;
		_StrongestLightDiffuse.set((uint8) (255.f * color.R), (uint8) (255.f * color.G), (uint8) (255.f * color.B), (uint8) (255.f * color.A));
		// setup strongest light to black for the gouraud part
		Driver->setConstant(_VPCurrentCtStart + 1 + i, 0.f, 0.f, 0.f, 0.f);
		++i;
		// setup other lights
		for(; i < _VPNumLights; i++)
		{
			color= _VPLightDiffuse[i] * matDiff;
			Driver->setConstant(_VPCurrentCtStart + 1 + i, 1, &color.R);
		}
	}

	// setup Specular
	if(_VPSupportSpecular)
	{
		for(i = 0; i < strongestLightIndex; ++i)
		{
			color= _VPLightSpecular[i] * matSpec;
			color.A= specExp;
			Driver->setConstant(_VPCurrentCtStart+5+i, 1, &color.R);
		}

		if (i != _VPNumLights)
		{
			color= _VPLightSpecular[i] * matSpec;
			_StrongestLightSpecular.set((uint8) (255.f * color.R), (uint8) (255.f * color.G), (uint8) (255.f * color.B), (uint8) (255.f * color.A));

			// setup strongest light to black (for gouraud part)
			Driver->setConstant(_VPCurrentCtStart + 5 + i, 0.f, 0.f, 0.f, 0.f);
			++i;
			// setup other lights
			for(; i < _VPNumLights; i++)
			{
				color= _VPLightSpecular[i] * matSpec;
				color.A= specExp;
				Driver->setConstant(_VPCurrentCtStart + 5 + i, 1, &color.R);
			}
		}
	}

	// setup alpha.
	static	float	alphaCte[4]= {0,0,1,0};
	alphaCte[3]= matDiff.A;
	// setup at good place
	if(_VPSupportSpecular)
			Driver->setConstant(_VPCurrentCtStart+10, 1, alphaCte);
	else
			Driver->setConstant(_VPCurrentCtStart+9, 1, alphaCte);
}

// ***************************************************************************
sint CRenderTrav::getStrongestLightIndex() const
{
	if (!_StrongestLightTouched) return -1;
	uint vpNumLights = min(_NumLightEnabled, (uint)MaxVPLight);
	// If there is only a directionnal light, use it
	// If there is any point light, use the nearest, or the directionnal light if it is brighter
	if (vpNumLights == 0) return -1;
	if (vpNumLights == 1) return 0;
	// First point light is brightest ?
	float lumDir = _VPLightDiffuse[0].R + _VPLightDiffuse[0].G + _VPLightDiffuse[0].B + _VPLightDiffuse[0].A
				   + _VPLightSpecular[0].R + _VPLightSpecular[0].G + _VPLightSpecular[0].B + _VPLightSpecular[0].A;
	float lumOmni = _VPLightDiffuse[1].R + _VPLightDiffuse[1].G + _VPLightDiffuse[1].B + _VPLightDiffuse[1].A
				   + _VPLightSpecular[1].R + _VPLightSpecular[1].G + _VPLightSpecular[1].B + _VPLightSpecular[1].A;
	return lumDir > lumOmni ? 0 : 1;
}

// ***************************************************************************
void	CRenderTrav::getStrongestLightColors(NLMISC::CRGBA &diffuse, NLMISC::CRGBA &specular)
{
	sint strongestLightIndex = getStrongestLightIndex();
	if (strongestLightIndex == -1)
	{
		diffuse = specular = NLMISC::CRGBA::Black;		
	}
	else
	{
		diffuse = _StrongestLightDiffuse;
		specular = _StrongestLightSpecular;		
	}
}


// ***************************************************************************
static const char*	LightingVPFragmentNormalize=
"	# normalize normal																	\n\
	DP3	R6.w, R6, R6;																	\n\
	RSQ	R6.w, R6.w;																		\n\
	MUL	R6, R6, R6.w;																	\n\
";


// ***************************************************************************
// NB: all CTS+x are replaced with good cte index.
static const char*	LightingVPFragmentNoSpecular_Begin=
"																						\n\
	# Global Ambient.																	\n\
	MOV	R2, c[CTS+0];																	\n\
																						\n\
	# Diffuse Sun																		\n\
	DP3	R0.x, R6, c[CTS+5];			# R0.x= normal*-lightDir							\n\
	LIT	R0, R0;						# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+1], R2;		# R2= summed vertex color.							\n\
";

// The 3 point Light code.
static const char*	LightingVPFragmentNoSpecular_PL[]=
{
"	# Diffuse PointLight 0.																\n\
	ADD	R0, c[CTS+6], -R5;			# R0= lightPos-vertex								\n\
	DP3	R0.w, R0, R0;				# normalize R0.										\n\
	RSQ	R0.w, R0.w;																		\n\
	MUL	R0, R0, R0.w;																	\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	LIT	R0, R0;						# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+2], R2;		# R2= summed vertex color.							\n\
",
"	# Diffuse PointLight 1.																\n\
	ADD	R0, c[CTS+7], -R5;			# R0= lightPos-vertex								\n\
	DP3	R0.w, R0, R0;				# normalize R0.										\n\
	RSQ	R0.w, R0.w;																		\n\
	MUL	R0, R0, R0.w;																	\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	LIT	R0, R0;						# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+3], R2;		# R2= summed vertex color.							\n\
",
"	# Diffuse PointLight 2.																\n\
	ADD	R0, c[CTS+8], -R5;			# R0= lightPos-vertex								\n\
	DP3	R0.w, R0, R0;				# normalize R0.										\n\
	RSQ	R0.w, R0.w;																		\n\
	MUL	R0, R0, R0.w;																	\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	LIT	R0, R0;						# R0.y= R0.x clamped								\n\
	MAD	R2, R0.y, c[CTS+4], R2;		# R2= summed vertex color.							\n\
"
};

// The End code.
static const char*	LightingVPFragmentNoSpecular_End=
"	# output to o[COL0] only, replacing alpha with material alpha.						\n\
	MAD	o[COL0], R2, c[CTS+9].zzzx, c[CTS+9].xxxw;										\n\
";


// ***************************************************************************
// NB: all CTS+x are replaced with good cte index.
static const char*	LightingVPFragmentSpecular_Begin=
"																						\n\
	# Global Ambient.																	\n\
	MOV	R2, c[CTS+0];																	\n\
																						\n\
	# Always keep Specular exponent in R0.w												\n\
	MOV	R0.w, c[CTS+5].w;																\n\
																						\n\
	# Compute vertex-to-eye vector normed.												\n\
	ADD	R4, c[CTS+11], -R5;																\n\
	DP3	R4.w, R4, R4;																	\n\
	RSQ	R4.w, R4.w;																		\n\
	MUL R4, R4, R4.w;																	\n\
																						\n\
	# Diffuse-Specular Sun																\n\
	# Compute R1= halfAngleVector= (lightDir+R4).normed().								\n\
	ADD	R1.xyz, c[CTS+9], R4;		# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, c[CTS+9];			# R0.x= normal*-lightDir							\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+1], R2;		# R2= summed vertex color.							\n\
	MUL	R3, R0.z, c[CTS+5];			# R3= specular color.								\n\
";

// The 3 point Light code.
static const char*	LightingVPFragmentSpecular_PL[]=
{
"	# Diffuse-Specular PointLight 0.													\n\
	# Compute R0= (lightPos-vertex).normed().											\n\
	ADD	R0.xyz, c[CTS+12], -R5;		# R0= lightPos-vertex								\n\
	DP3	R1.w, R0, R0;				# normalize R0.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R0.xyz, R0, R1.w;																\n\
	# Compute R1= halfAngleVector= (R0+R4).normed().									\n\
	ADD	R1.xyz, R0, R4;				# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+2], R2;		# R2= summed vertex color.							\n\
	MAD	R3, R0.z, c[CTS+6], R3;		# R3= summed specular color.						\n\
",
"	# Diffuse-Specular PointLight 1.													\n\
	# Compute R0= (lightPos-vertex).normed().											\n\
	ADD	R0.xyz, c[CTS+13], -R5;		# R0= lightPos-vertex								\n\
	DP3	R1.w, R0, R0;				# normalize R0.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R0.xyz, R0, R1.w;																\n\
	# Compute R1= halfAngleVector= (R0+R4).normed().									\n\
	ADD	R1.xyz, R0, R4;				# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+3], R2;		# R2= summed vertex color.							\n\
	MAD	R3, R0.z, c[CTS+7], R3;		# R3= summed specular color.						\n\
",
"	# Diffuse-Specular PointLight 2.													\n\
	# Compute R0= (lightPos-vertex).normed().											\n\
	ADD	R0.xyz, c[CTS+14], -R5;		# R0= lightPos-vertex								\n\
	DP3	R1.w, R0, R0;				# normalize R0.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R0.xyz, R0, R1.w;																\n\
	# Compute R1= halfAngleVector= (R0+R4).normed().									\n\
	ADD	R1.xyz, R0, R4;				# R1= halfAngleVector								\n\
	DP3	R1.w, R1, R1;				# normalize R1.										\n\
	RSQ	R1.w, R1.w;																		\n\
	MUL	R1.xyz, R1, R1.w;																\n\
	# Compute Factors and colors.														\n\
	DP3	R0.x, R6, R0;				# R0.x= normal*lightDir								\n\
	DP3	R0.yz, R6, R1;				# R0.yz= normal*halfAngleVector						\n\
	LIT	R0.yz, R0;					# R0.y= R0.x clamped, R0.z= pow(spec, R0.w) clamp	\n\
	MAD	R2, R0.y, c[CTS+4], R2;		# R2= summed vertex color.							\n\
"
};


// The End code.
static const char*	LightingVPFragmentSpecular_End=
"	# output directly to secondary color.												\n\
	MAD	o[COL1], R0.z, c[CTS+8], R3;	# final summed specular color.					\n\
																						\n\
	# output diffuse to o[COL0], replacing alpha with material alpha.					\n\
	MAD	o[COL0], R2, c[CTS+10].zzzx, c[CTS+10].xxxw;									\n\
";

// ***************************************************************************
static	void	strReplaceAll(string &strInOut, const string &tokenSrc, const string &tokenDst)
{
	sint	pos;
	sint	srcLen= tokenSrc.size();
	while( (pos=strInOut.find(tokenSrc)) != string::npos)
	{
		strInOut.replace(pos, srcLen, tokenDst);
	}
}

// ***************************************************************************
std::string		CRenderTrav::getLightVPFragment(uint numActivePointLights, uint ctStart, bool supportSpecular, bool normalize)
{
	string	ret;

	// Code frag written for 4 light max.
	nlassert(MaxVPLight==4);
	nlassert(numActivePointLights<=MaxVPLight-1);

	// Add LightingVPFragmentNormalize fragment?
	if(normalize)
		ret+= LightingVPFragmentNormalize;

	// Which fragment to use...
	if(supportSpecular)
	{
		// Add start of VP.
		ret+= LightingVPFragmentSpecular_Begin;

		// Add needed pointLights.
		for(uint i=0;i<numActivePointLights;i++)
		{
			ret+= LightingVPFragmentSpecular_PL[i];
		}

		// Add end of VP.
		ret+= LightingVPFragmentSpecular_End;
	}
	else
	{
		// Add start of VP.
		ret+= LightingVPFragmentNoSpecular_Begin;

		// Add needed pointLights.
		for(uint i=0;i<numActivePointLights;i++)
		{
			ret+= LightingVPFragmentNoSpecular_PL[i];
		}

		// Add end of VP.
		ret+= LightingVPFragmentNoSpecular_End;
	}

	// Replace all CTS+x with good index. do it for 15 possible indices: 0 to 14 if specular.
	// run from 14 to 0 so CTS+14 will not be taken for a CTS+1 !!
	for(sint i=14; i>=0; i--)
	{
		char	tokenSrc[256];
		sprintf(tokenSrc, "CTS+%d", i);
		char	tokenDst[256];
		sprintf(tokenDst, "%d", ctStart+i);
		// replace all in the string
		strReplaceAll(ret, tokenSrc, tokenDst);
	}

	// verify no CTS+ leaved... (not all ctes parsed!!!)
	nlassert( ret.find("CTS+")==string::npos );

	return ret;
}


}
