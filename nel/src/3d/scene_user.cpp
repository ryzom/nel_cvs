/** \file scene_user.cpp
 * <File description>
 *
 * $Id: scene_user.cpp,v 1.13 2002/03/29 13:13:45 berenguier Exp $
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

#include "3d/scene_user.h"
#include "3d/coarse_mesh_manager.h"
#include "3d/point_light_user.h"
#include "3d/point_light_model.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************
UAnimationSet			*CSceneUser::createAnimationSet() 
{
	return new CAnimationSetUser();
}
// ***************************************************************************
UAnimationSet			*CSceneUser::createAnimationSet(const std::string &animationSetFile) 
{
	NLMISC::CIFile	f;
	// throw exception if not found.
	std::string	path= CPath::lookup(animationSetFile);
	f.open(path);
	return _AnimationSets.insert(new CAnimationSetUser(f));
}
// ***************************************************************************
void			CSceneUser::deleteAnimationSet(UAnimationSet	*animationSet) 
{
	_AnimationSets.erase((CAnimationSetUser*)animationSet, "deleteAnimationSet(): Bad AnimationSet ptr");
}
// ***************************************************************************
UPlayListManager			*CSceneUser::createPlayListManager() 
{
	return _PlayListManagers.insert(new CPlayListManagerUser());
}
// ***************************************************************************
void			CSceneUser::deletePlayListManager(UPlayListManager	*playListManager) 
{
	_PlayListManagers.erase((CPlayListManagerUser*)playListManager, "deletePlayListManager(): Bad PlayListManager ptr");
}

// ***************************************************************************

void			CSceneUser::setPolygonBalancingMode(CSceneUser::TPolygonBalancingMode polBalMode)
{
	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	_Scene.setPolygonBalancingMode((CScene::TPolygonBalancingMode)(uint)(polBalMode));
}

// ***************************************************************************

CSceneUser::TPolygonBalancingMode	CSceneUser::getPolygonBalancingMode() const
{
	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	return (CSceneUser::TPolygonBalancingMode)(uint)_Scene.getPolygonBalancingMode();
}


// ***************************************************************************
void			CSceneUser::setLoadMaxPolygon(uint nFaces)
{
	_Scene.setLoadMaxPolygon(nFaces);
}
// ***************************************************************************
uint			CSceneUser::getLoadMaxPolygon()
{
	return _Scene.getLoadMaxPolygon();
}
// ***************************************************************************
float			CSceneUser::getNbFaceAsked () const
{
	return _Scene.getNbFaceAsked ();
}

// ***************************************************************************
void			CSceneUser::setGroupLoadMaxPolygon(const std::string &group, uint nFaces)
{
	_Scene.setGroupLoadMaxPolygon(group, nFaces);
}
// ***************************************************************************
uint			CSceneUser::getGroupLoadMaxPolygon(const std::string &group)
{
	return _Scene.getGroupLoadMaxPolygon(group);
}
// ***************************************************************************
float			CSceneUser::getGroupNbFaceAsked (const std::string &group) const
{
	return _Scene.getGroupNbFaceAsked (group);
}


// ***************************************************************************

void CSceneUser::setStaticCoarseMeshManagerTexture (const char *sPath)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getStaticCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setTextureFile (sPath);
	}
}

// ***************************************************************************

void CSceneUser::setDynamicCoarseMeshManagerTexture (const char *sPath)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getDynamicCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setTextureFile (sPath);
	}
}

// ***************************************************************************

void CSceneUser::setStaticCoarseMeshManagerColor (const CRGBA& color)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getStaticCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setColor (color);
	}
}

// ***************************************************************************

void CSceneUser::setDynamicCoarseMeshManagerColor (const CRGBA& color)
{
	// Get the manager
	CCoarseMeshManager *manager=_Scene.getDynamicCoarseMeshManager ();

	// Does it exist ?
	if (manager)
	{
		// Set the texture
		manager->setColor (color);
	}
}


// ***************************************************************************
void				CSceneUser::enableLightingSystem(bool enable)
{
	_Scene.enableLightingSystem(enable);
}

// ***************************************************************************
void				CSceneUser::setAmbientGlobal(NLMISC::CRGBA ambient)
{
	_Scene.setAmbientGlobal(ambient);
}
void				CSceneUser::setSunAmbient(NLMISC::CRGBA ambient)
{
	_Scene.setSunAmbient(ambient);
}
void				CSceneUser::setSunDiffuse(NLMISC::CRGBA diffuse)
{
	_Scene.setSunDiffuse(diffuse);
}
void				CSceneUser::setSunSpecular(NLMISC::CRGBA specular)
{
	_Scene.setSunSpecular(specular);
}
void				CSceneUser::setSunDirection(const NLMISC::CVector &direction)
{
	_Scene.setSunDirection(direction);
}


// ***************************************************************************
NLMISC::CRGBA		CSceneUser::getAmbientGlobal() const
{
	return _Scene.getAmbientGlobal();
}
NLMISC::CRGBA		CSceneUser::getSunAmbient() const
{
	return _Scene.getSunAmbient();
}
NLMISC::CRGBA		CSceneUser::getSunDiffuse() const
{
	return _Scene.getSunDiffuse();
}
NLMISC::CRGBA		CSceneUser::getSunSpecular() const
{
	return _Scene.getSunSpecular();
}
NLMISC::CVector		CSceneUser::getSunDirection() const
{
	return _Scene.getSunDirection();
}


// ***************************************************************************
void				CSceneUser::setMaxLightContribution(uint nlights)
{
	_Scene.setMaxLightContribution(nlights);
}
uint				CSceneUser::getMaxLightContribution() const
{
	return _Scene.getMaxLightContribution();
}

void				CSceneUser::setLightTransitionThreshold(float lightTransitionThreshold)
{
	_Scene.setLightTransitionThreshold(lightTransitionThreshold);
}
float				CSceneUser::getLightTransitionThreshold() const
{
	return _Scene.getLightTransitionThreshold();
}


// ***************************************************************************
UPointLight		*CSceneUser::createPointLight()
{
	IModel	*model= _Scene.createModel(PointLightModelId);
	// If not found, return NULL.
	if(model==NULL)
		return NULL;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return dynamic_cast<UPointLight*>( _Transforms.insert(new CPointLightUser(&_Scene, model)) );
}
// ***************************************************************************
void			CSceneUser::deletePointLight(UPointLight *light)
{
	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Transforms.erase(dynamic_cast<CTransformUser*>(light));
}


// ***************************************************************************
void			CSceneUser::setGlobalWindPower(float gwp)
{
	_Scene.setGlobalWindPower(gwp);
}
// ***************************************************************************
float			CSceneUser::getGlobalWindPower() const
{
	return _Scene.getGlobalWindPower();
}
// ***************************************************************************
void			CSceneUser::setGlobalWindDirection(const CVector &gwd)
{
	_Scene.setGlobalWindDirection(gwd);
}
// ***************************************************************************
const CVector	&CSceneUser::getGlobalWindDirection() const
{
	return _Scene.getGlobalWindDirection();
}


} // NL3D
