/** \file scene_user.cpp
 * <File description>
 *
 * $Id: scene_user.cpp,v 1.31 2002/11/14 12:55:32 berenguier Exp $
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
#include "3d/lod_character_manager.h"
#include "3d/lod_character_shape.h"
#include "3d/lod_character_shape_bank.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/async_texture_manager.h"

using namespace NLMISC;

namespace NL3D 
{

H_AUTO_DECL( NL3D_UI_Scene )
H_AUTO_DECL( NL3D_Misc_Scene_CreateDel_Element )
H_AUTO_DECL( NL3D_Load_AnimationSet )
H_AUTO_DECL( NL3D_CreateOrLoad_Instance )
H_AUTO_DECL( NL3D_CreateOrLoad_Skeleton )
H_AUTO_DECL( NL3D_Load_CLodOrCoarseMesh )
H_AUTO_DECL( NL3D_Load_AsyncIG )

#define	NL3D_HAUTO_UI_SCENE						H_AUTO_USE( NL3D_UI_Scene )
#define	NL3D_HAUTO_ELT_SCENE					H_AUTO_USE( NL3D_Misc_Scene_CreateDel_Element )
#define	NL3D_HAUTO_LOAD_ANIMSET					H_AUTO_USE( NL3D_Load_AnimationSet )
#define	NL3D_HAUTO_CREATE_INSTANCE				H_AUTO_USE( NL3D_CreateOrLoad_Instance )
#define	NL3D_HAUTO_CREATE_SKELETON				H_AUTO_USE( NL3D_CreateOrLoad_Skeleton )
#define	NL3D_HAUTO_LOAD_LOD						H_AUTO_USE( NL3D_Load_CLodOrCoarseMesh )
#define	NL3D_HAUTO_ASYNC_IG						H_AUTO_USE( NL3D_Load_AsyncIG )

// Render/Animate.
H_AUTO_DECL( NL3D_Render_Scene )
H_AUTO_DECL( NL3D_Render_Animate_Scene )

#define	NL3D_HAUTO_RENDER_SCENE					H_AUTO_USE( NL3D_Render_Scene )
#define	NL3D_HAUTO_RENDER_SCENE_ANIMATE			H_AUTO_USE( NL3D_Render_Animate_Scene )

#define NL3D_MEM_LIGHT						NL_ALLOC_CONTEXT( 3dLight )
#define NL3D_MEM_IG							NL_ALLOC_CONTEXT( 3dIg )
#define NL3D_MEM_LOD						NL_ALLOC_CONTEXT( 3dLod )
#define NL3D_MEM_SCENE_RENDER				NL_ALLOC_CONTEXT( 3dScRdr )
#define NL3D_MEM_INSTANCE					NL_ALLOC_CONTEXT( 3dInst )
#define NL3D_MEM_LANDSCAPE					NL_ALLOC_CONTEXT( 3dLand )
#define NL3D_MEM_CLOUDS						NL_ALLOC_CONTEXT( 3dCloud )
#define NL3D_MEM_VISUAL_COLLISION			NL_ALLOC_CONTEXT( 3dVsCol )

// ***************************************************************************
UAnimationSet			*CSceneUser::createAnimationSet() 
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	NL3D_HAUTO_ELT_SCENE;

	return new CAnimationSetUser();
}
// ***************************************************************************
UAnimationSet			*CSceneUser::createAnimationSet(const std::string &animationSetFile) 
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	NL3D_HAUTO_LOAD_ANIMSET;

	NLMISC::CIFile	f;
	// throw exception if not found.
	std::string	path= CPath::lookup(animationSetFile);
	f.open(path);
	return _AnimationSets.insert(new CAnimationSetUser(f));
}
// ***************************************************************************
void			CSceneUser::deleteAnimationSet(UAnimationSet	*animationSet) 
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	NL3D_HAUTO_ELT_SCENE;

	_AnimationSets.erase((CAnimationSetUser*)animationSet, "deleteAnimationSet(): Bad AnimationSet ptr");
}

// ***************************************************************************
void			CSceneUser::setAutomaticAnimationSet(UAnimationSet *as)
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	NL3D_HAUTO_UI_SCENE;

	nlassert(as);
	as->build();
	CAnimationSetUser *asu = NLMISC::safe_cast<CAnimationSetUser *>(as);
	_Scene.setAutomaticAnimationSet(asu->_AnimationSet);
}

// ***************************************************************************
UPlayListManager			*CSceneUser::createPlayListManager() 
{
	NL_ALLOC_CONTEXT( 3dAnim )
	NL3D_HAUTO_ELT_SCENE;

	return _PlayListManagers.insert(new CPlayListManagerUser());
}
// ***************************************************************************
void			CSceneUser::deletePlayListManager(UPlayListManager	*playListManager) 
{
	NL_ALLOC_CONTEXT( 3dAnim )
	NL3D_HAUTO_ELT_SCENE;

	_PlayListManagers.erase((CPlayListManagerUser*)playListManager, "deletePlayListManager(): Bad PlayListManager ptr");
}

// ***************************************************************************

void			CSceneUser::setPolygonBalancingMode(CSceneUser::TPolygonBalancingMode polBalMode)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	_Scene.setPolygonBalancingMode((CScene::TPolygonBalancingMode)(uint)(polBalMode));
}

// ***************************************************************************

CSceneUser::TPolygonBalancingMode	CSceneUser::getPolygonBalancingMode() const
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	nlassert( (uint)CScene::CountPolygonBalancing == (uint)CSceneUser::CountPolygonBalancing );
	return (CSceneUser::TPolygonBalancingMode)(uint)_Scene.getPolygonBalancingMode();
}


// ***************************************************************************
float			CSceneUser::getNbFaceAsked () const
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getNbFaceAsked ();
}

// ***************************************************************************
void			CSceneUser::setGroupLoadMaxPolygon(const std::string &group, uint nFaces)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	_Scene.setGroupLoadMaxPolygon(group, nFaces);
}
// ***************************************************************************
uint			CSceneUser::getGroupLoadMaxPolygon(const std::string &group)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGroupLoadMaxPolygon(group);
}
// ***************************************************************************
float			CSceneUser::getGroupNbFaceAsked (const std::string &group) const
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGroupNbFaceAsked (group);
}


// ***************************************************************************

void CSceneUser::setStaticCoarseMeshManagerTexture (const char *sPath)
{
	NL3D_MEM_LOD
	NL3D_HAUTO_LOAD_LOD;

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
	NL3D_MEM_LOD
	NL3D_HAUTO_LOAD_LOD;

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
void				CSceneUser::setCoarseMeshLightingUpdate(uint8 period)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setCoarseMeshLightingUpdate(period);
}

// ***************************************************************************
uint8				CSceneUser::getCoarseMeshLightingUpdate() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getCoarseMeshLightingUpdate();
}

// ***************************************************************************
void				CSceneUser::enableLightingSystem(bool enable)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.enableLightingSystem(enable);
}

// ***************************************************************************
void				CSceneUser::setAmbientGlobal(NLMISC::CRGBA ambient)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setAmbientGlobal(ambient);
}
void				CSceneUser::setSunAmbient(NLMISC::CRGBA ambient)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunAmbient(ambient);
}
void				CSceneUser::setSunDiffuse(NLMISC::CRGBA diffuse)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunDiffuse(diffuse);
}
void				CSceneUser::setSunSpecular(NLMISC::CRGBA specular)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunSpecular(specular);
}
void				CSceneUser::setSunDirection(const NLMISC::CVector &direction)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setSunDirection(direction);
}


// ***************************************************************************
NLMISC::CRGBA		CSceneUser::getAmbientGlobal() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getAmbientGlobal();
}
NLMISC::CRGBA		CSceneUser::getSunAmbient() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunAmbient();
}
NLMISC::CRGBA		CSceneUser::getSunDiffuse() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunDiffuse();
}
NLMISC::CRGBA		CSceneUser::getSunSpecular() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunSpecular();
}
NLMISC::CVector		CSceneUser::getSunDirection() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getSunDirection();
}


// ***************************************************************************
void				CSceneUser::setMaxLightContribution(uint nlights)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setMaxLightContribution(nlights);
}
uint				CSceneUser::getMaxLightContribution() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getMaxLightContribution();
}

void				CSceneUser::setLightTransitionThreshold(float lightTransitionThreshold)
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	_Scene.setLightTransitionThreshold(lightTransitionThreshold);
}
float				CSceneUser::getLightTransitionThreshold() const
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getLightTransitionThreshold();
}


// ***************************************************************************
UPointLight		*CSceneUser::createPointLight()
{
	NL3D_MEM_LIGHT
	NL3D_HAUTO_ELT_SCENE;

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
	NL3D_MEM_LIGHT
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Transforms.erase(dynamic_cast<CTransformUser*>(light));
}


// ***************************************************************************
void			CSceneUser::setGlobalWindPower(float gwp)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	_Scene.setGlobalWindPower(gwp);
}
// ***************************************************************************
float			CSceneUser::getGlobalWindPower() const
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

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
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getGlobalWindDirection();
}

// ***************************************************************************
void CSceneUser::updateWaitingIG()
{
	NL3D_MEM_IG
	for(TWaitingIGList::iterator it = _WaitingIGs.begin(); it != _WaitingIGs.end();)
	{
		bool	erased= false;
		if (it->IGToLoad != NULL) // ig loaded ?
		{			
			if (it->IGToLoad != (UInstanceGroup *) -1)
			{				
				switch (it->IGToLoad->getAddToSceneState())
				{
					case UInstanceGroup::StateNotAdded:
						// start loading										
						it->IGToLoad->addToSceneAsync(*this, _DriverUser);
					break;
					case UInstanceGroup::StateAdded:
						it->IGToLoad->setPos(it->Offset);
						this->setToGlobalInstanceGroup(it->IGToLoad);
						*it->CallerPtr = it->IGToLoad;
						// remove from list
						it = _WaitingIGs.erase(it);
						erased= true;
					break;
					default:
					break;
				}
			}
			else
			{
				// loading failed
				*it->CallerPtr = it->IGToLoad;
				it = _WaitingIGs.erase(it);
				erased= true;
			}
		}
		// next IG.
		if(!erased)
			it++;
	}
}


// ***************************************************************************
void				CSceneUser::resetCLodManager()
{
	NL3D_MEM_LOD
	NL3D_HAUTO_UI_SCENE;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	_Scene.getLodCharacterManager()->reset();
}

// ***************************************************************************
uint32				CSceneUser::loadCLodShapeBank(const std::string &fileName)
{
	NL3D_MEM_LOD
	NL3D_HAUTO_LOAD_LOD;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	// Open the file
	CIFile	file(CPath::lookup(fileName));

	// create the shape bank
	uint32	bankId= _Scene.getLodCharacterManager()->createShapeBank();

	// get the bank
	CLodCharacterShapeBank	*bank= _Scene.getLodCharacterManager()->getShapeBank(bankId);
	nlassert(bank);

	// read the bank.
	file.serial(*bank);

	// recompile the shape Map.
	_Scene.getLodCharacterManager()->compile();

	return bankId;
}

// ***************************************************************************
void				CSceneUser::deleteCLodShapeBank(uint32 bankId)
{
	NL3D_MEM_LOD
	NL3D_HAUTO_LOAD_LOD;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	// delete the bank
	_Scene.getLodCharacterManager()->deleteShapeBank(bankId);

	// recompile the shape Map.
	_Scene.getLodCharacterManager()->compile();
}

// ***************************************************************************
sint32				CSceneUser::getCLodShapeIdByName(const std::string &name) const
{
	NL3D_MEM_LOD
	NL3D_HAUTO_UI_SCENE;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	return _Scene.getLodCharacterManager()->getShapeIdByName(name);
}

// ***************************************************************************
sint32				CSceneUser::getCLodAnimIdByName(uint32 shapeId, const std::string &name) const
{
	NL3D_MEM_LOD
	NL3D_HAUTO_UI_SCENE;

	// DriverUser always setup the lod manager
	nlassert(_Scene.getLodCharacterManager());

	const CLodCharacterShape	*shape= _Scene.getLodCharacterManager()->getShape(shapeId);
	if(shape)
		return shape->getAnimIdByName(name);
	else
		return -1;
}


// ***************************************************************************
void			CSceneUser::render()
{	
	NL3D_MEM_SCENE_RENDER

	// render the scene.
	{
		NL3D_HAUTO_RENDER_SCENE

		if(_CurrentCamera==NULL)
			nlerror("render(): try to render with no camera linked (may have been deleted)");
		_Scene.render();
	}

	updateWaitingInstances();

	// Must restore the matrix context, so 2D/3D interface not disturbed.
	_DriverUser->restoreMatrixContext();
}


// ***************************************************************************
/*virtual*/ void CSceneUser::updateWaitingInstances(double ellapsedTime)
{
	NL3D_MEM_INSTANCE
	_Scene.updateWaitingInstances(ellapsedTime);
	updateWaitingInstances();
}


// ***************************************************************************
void CSceneUser::updateWaitingInstances()
{
	NL3D_MEM_INSTANCE
	// Update waiting instances
	{
		NL3D_HAUTO_ASYNC_IG

		// Done after the _Scene.render because in this method the instance are checked for creation
		std::map<UInstance**,CTransformShape*>::iterator it = _WaitingInstances.begin();
		while( it != _WaitingInstances.end() )
		{
			if( it->second != NULL )
			{
				*(it->first) = dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, it->second)) );
				std::map<UInstance**,CTransformShape*>::iterator delIt = it;
				++it;
				_WaitingInstances.erase(delIt);
			}
			else
			{
				++it;
			}
		}
	}

	// update waiting instances groups;
	{
		NL3D_HAUTO_ASYNC_IG

		updateWaitingIG();
	}	
}


void			CSceneUser::animate(TGlobalAnimationTime time)
{
	NL_ALLOC_CONTEXT( 3dAnim )
	NL3D_HAUTO_RENDER_SCENE_ANIMATE;

	_Scene.animate(time);
}


// ***************************************************************************
void			CSceneUser::setCam(UCamera *cam)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	if(!cam)
		nlerror("setCam(): cannot set a NULL camera");
	CCameraUser		*newCam= dynamic_cast<CCameraUser*>(cam);
	if( newCam->getScene() != &_Scene)
		nlerror("setCam(): try to set a current camera not created from this scene");

	_CurrentCamera= newCam;
	_Scene.setCam(newCam->getCamera());
}
UCamera			*CSceneUser::getCam()
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	return dynamic_cast<UCamera*>(_CurrentCamera);
}
void			CSceneUser::setViewport(const class CViewport& viewport)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	_Scene.setViewport(viewport);
}
CViewport		CSceneUser::getViewport()
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_UI_SCENE;

	return _Scene.getViewport();
}

// ***************************************************************************
UCamera			*CSceneUser::createCamera()
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return dynamic_cast<UCamera*>( _Transforms.insert(new CCameraUser(&_Scene)) );
}
void			CSceneUser::deleteCamera(UCamera *cam)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_ELT_SCENE;

	CCameraUser		*oldCam= dynamic_cast<CCameraUser*>(cam);
	// Is this the current camera??
	if(oldCam==_CurrentCamera)
		_CurrentCamera=NULL;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Transforms.erase(oldCam);
}

UInstance		*CSceneUser::createInstance(const std::string &shapeName)
{
	NL3D_MEM_INSTANCE
	NL3D_HAUTO_CREATE_INSTANCE;

	IModel	*model= _Scene.createInstance(shapeName);
	// If not found, return NULL.
	if(model==NULL)
		return NULL;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	if (dynamic_cast<CParticleSystemModel *>(model))
	{
		/// particle system
		return dynamic_cast<UInstance*>( _Transforms.insert(new CParticleSystemInstanceUser(&_Scene, model)) );
	}
	else
	{
		/// mesh
		return dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, model)) );
	}
}


void CSceneUser::createInstanceAsync(const std::string &shapeName, UInstance**ppInstance)
{
	NL3D_MEM_INSTANCE
	NL3D_HAUTO_CREATE_INSTANCE;

	_WaitingInstances[ppInstance] = NULL;
	_Scene.createInstanceAsync(shapeName,&_WaitingInstances[ppInstance]);
//		IModel	*model= _Scene.createInstance(shapeName);
	// If not found, return NULL.
//		if(model==NULL)
//			return NULL;

//		if( dynamic_cast<CMeshInstance*>(model)==NULL )
//			nlerror("UScene::createInstance(): shape is not a mesh");

	// The component is auto added/deleted to _Scene in ctor/dtor.
//		return dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, model)) );
}

void			CSceneUser::deleteInstance(UInstance *inst)
{
	NL3D_MEM_INSTANCE
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Transforms.erase(dynamic_cast<CTransformUser*>(inst));
}


void CSceneUser::createInstanceGroupAndAddToSceneAsync (const std::string &instanceGroup, UInstanceGroup **pIG, const NLMISC::CVector &offset)
{
	NL3D_MEM_IG
	NL3D_HAUTO_ASYNC_IG;

	_WaitingIGs.push_front(CWaitingIG(pIG, offset));
	UInstanceGroup::createInstanceGroupAsync(instanceGroup, &(_WaitingIGs.begin()->IGToLoad));
	// this list updat will be performed at each render, see updateWaitingIG
}

void CSceneUser::stopCreatingAndAddingIG(UInstanceGroup **pIG)
{
	NL3D_MEM_IG
	NL3D_HAUTO_ASYNC_IG;

	for(TWaitingIGList::iterator it = _WaitingIGs.begin(); it != _WaitingIGs.end(); ++it)
	{
		if (it->CallerPtr == pIG)
		{
			if (!it->IGToLoad)
			{
				UInstanceGroup::stopCreateInstanceGroupAsync(pIG);										
			}
			else
			{
				switch(it->IGToLoad->getAddToSceneState())
				{
					case UInstanceGroup::StateAdding:
						it->IGToLoad->stopAddToSceneAsync();
					break;
					case UInstanceGroup::StateAdded:
						it->IGToLoad->removeFromScene(*this);
						delete it->IGToLoad;
					break;
					case UInstanceGroup::StateNotAdded:
						delete it->IGToLoad;
					break;
				}
			}
			_WaitingIGs.erase(it);
			return;
		}
	}		
}


UTransform *CSceneUser::createTransform()
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_ELT_SCENE;

	IModel	*model= _Scene.createModel(TransformId);
	// If not found, return NULL.
	if(model==NULL)
		return NULL;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return dynamic_cast<UTransform*>( _Transforms.insert(new CTransformUser(&_Scene, model)) );
}

void			CSceneUser::deleteTransform(UTransform *tr)
{
	NL3D_MEM_SCENE
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Transforms.erase(dynamic_cast<CTransformUser*>(tr));
}


USkeleton		*CSceneUser::createSkeleton(const std::string &shapeName)
{
	NL3D_MEM_SKELETON
	NL3D_HAUTO_CREATE_SKELETON;

	IModel	*model= _Scene.createInstance(shapeName);
	// If not found, return NULL.
	if(model==NULL)
		return NULL;

	if( dynamic_cast<CSkeletonModel*>(model)==NULL )
		nlerror("UScene::createSkeleton(): shape is not a skeletonShape");

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return dynamic_cast<USkeleton*>( _Transforms.insert(new CSkeletonUser(&_Scene, model)) );
}
void			CSceneUser::deleteSkeleton(USkeleton *skel)
{
	NL3D_MEM_SKELETON
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Transforms.erase(dynamic_cast<CTransformUser*>(skel));
}


ULandscape		*CSceneUser::createLandscape()
{
	NL3D_MEM_LANDSCAPE
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return _Landscapes.insert(new CLandscapeUser(&_Scene));
}
void			CSceneUser::deleteLandscape(ULandscape *land)
{
	NL3D_MEM_LANDSCAPE
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_Landscapes.erase((CLandscapeUser*) land);
}

UCloudScape *CSceneUser::createCloudScape()
{
	NL3D_MEM_CLOUDS
	NL3D_HAUTO_ELT_SCENE;

	// The component is auto added/deleted to _Scene in ctor/dtor.
	return _CloudScapes.insert(new CCloudScapeUser(&_Scene));
}
void CSceneUser::deleteCloudScape(UCloudScape *cs)
{
	NL3D_MEM_CLOUDS

	// The component is auto added/deleted to _Scene in ctor/dtor.
	_CloudScapes.erase((CCloudScapeUser*) cs);
}
/*

UInstanceGroup	*CSceneUser::createInstanceGroup (const std::string &instanceGroup)
{
	// Create the instance group
	CInstanceGroupUser *user=new CInstanceGroupUser;

	// Init the class
	if (!user->load (instanceGroup))
	{
		// Prb, erase it
		delete user;

		// Return error code
		return NULL;
	}

	// Insert the pointer in the pointer list
	_InstanceGroups.insert (user);

	// return the good value
	return user;
}

void			CSceneUser::deleteInstanceGroup (UInstanceGroup	*group)
{
	// The component is auto added/deleted to _Scene in ctor/dtor.
	_InstanceGroups.erase (dynamic_cast<CInstanceGroupUser*>(group));
}
*/

void CSceneUser::setToGlobalInstanceGroup(UInstanceGroup *pIG)
{
	NL3D_MEM_IG
	NL3D_HAUTO_UI_SCENE;

	CInstanceGroupUser *pIGU = (CInstanceGroupUser*)pIG;
	pIGU->_InstanceGroup.setClusterSystem (_Scene.getGlobalInstanceGroup());
}

// ***************************************************************************
UVisualCollisionManager		*CSceneUser::createVisualCollisionManager()
{
	NL3D_MEM_VISUAL_COLLISION
	NL3D_HAUTO_ELT_SCENE;

	return _VisualCollisionManagers.insert(new CVisualCollisionManagerUser);
}
void						CSceneUser::deleteVisualCollisionManager(UVisualCollisionManager *mgr)
{
	NL3D_MEM_VISUAL_COLLISION
	NL3D_HAUTO_ELT_SCENE;

	_VisualCollisionManagers.erase(dynamic_cast<CVisualCollisionManagerUser*>(mgr));
}


// ***************************************************************************
CSceneUser::CSceneUser(CDriverUser *drv)
{
	NL3D_MEM_SCENE_INIT
	nlassert(drv);
	_DriverUser= drv;
	_CurrentCamera = NULL;

	// Init Scene.
	_Scene.initDefaultTravs();

	// Don't add any user trav.
	// init default Roots.
	_Scene.initDefaultRoots();

	// Set driver.
	_Scene.setDriver(_DriverUser->getDriver());

	// Set viewport
	_Scene.setViewport (CViewport());

	// Init the world instance group
	_Scene.initGlobalnstanceGroup();

	// Init coarse mesh manager
	_Scene.initCoarseMeshManager ();

	// init QuadGridClipManager
	_Scene.initQuadGridClipManager ();

	// Create default camera, and active!!
	setCam(createCamera());
}

CSceneUser::~CSceneUser()
{
	NL3D_MEM_SCENE
	_VisualCollisionManagers.clear();
	_Transforms.clear();
	_Landscapes.clear();
	_CloudScapes.clear();
	_Scene.release();
	_Scene.setDriver(NULL);
	_Scene.setCam(NULL);
	_CurrentCamera= NULL;
	_DriverUser= NULL;
}

// ***************************************************************************
void		CSceneUser::setMaxSkeletonsInNotCLodForm(uint m)
{
	_Scene.setMaxSkeletonsInNotCLodForm(m);
}

// ***************************************************************************
uint		CSceneUser::getMaxSkeletonsInNotCLodForm() const
{
	return _Scene.getMaxSkeletonsInNotCLodForm();
}


} // NL3D
