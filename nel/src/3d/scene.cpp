/** \file scene.cpp
 * A 3d scene, manage model instantiation, tranversals etc..
 *
 * $Id: scene.cpp,v 1.101 2003/06/03 13:05:02 corvazier Exp $
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

#include "3d/scene.h"
#include "3d/trav_scene.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/light_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/load_balancing_trav.h"
#include "3d/render_trav.h"
#include "3d/transform.h"
#include "3d/camera.h"
#include "3d/landscape_model.h"
#include "3d/driver.h"
#include "3d/transform_shape.h"
#include "3d/mesh_base.h"
#include "3d/mesh_base_instance.h"
#include "3d/mesh_instance.h"
#include "3d/mesh_mrm_instance.h"
#include "3d/mesh_multi_lod_instance.h"
#include "3d/shape_bank.h"
#include "3d/skeleton_model.h"
#include "3d/particle_system_model.h"
#include "3d/coarse_mesh_manager.h"
#include "3d/cluster.h"
#include "3d/scene_group.h"
#include "3d/flare_model.h"
#include "3d/water_model.h"
#include "3d/vegetable_blend_layer_model.h"
#include "3d/root_model.h"
#include "3d/point_light_model.h"
#include "3d/animation.h"
#include "3d/lod_character_manager.h"
#include "3d/seg_remanence.h"
#include "3d/async_texture_manager.h"

#include <memory>

#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"


using namespace std;
using namespace NLMISC;

#define NL3D_SCENE_COARSE_MANAGER_TEXTURE	"nel_coarse_texture.tga"

#define NL3D_MEM_INSTANCE					NL_ALLOC_CONTEXT( 3dInst )
#define NL3D_MEM_MOT						NL_ALLOC_CONTEXT( 3dMot )

// The manager is limited to a square of 3000m*3000m around the camera. Beyond, models are clipped individually (bad!!).
const	float	NL3D_QuadGridClipManagerRadiusMax= 1500;
const	float	NL3D_QuadGridClipClusterSize= 400;
const	uint	NL3D_QuadGridClipNumDist= 10;
const	float	NL3D_QuadGridClipMaxDist= 1000;


namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

	
void	CScene::registerBasics()
{
	CTransform::registerBasic();
	CCamera::registerBasic();
	CMeshBaseInstance::registerBasic();
	CMeshInstance::registerBasic();
	CMeshMRMInstance::registerBasic();
	CLandscapeModel::registerBasic();
	CTransformShape::registerBasic();
	CSkeletonModel::registerBasic();
	CParticleSystemModel::registerBasic() ;
	CMeshMultiLodInstance::registerBasic();
	CCluster::registerBasic();
	CFlareModel::registerBasic();
	CWaterModel::registerBasic();
	CWaveMakerModel::registerBasic();
	CVegetableBlendLayerModel::registerBasic();
	CRootModel::registerBasic();
	CPointLightModel::registerBasic();
	CSegRemanence::registerBasic();
	CQuadGridClipManager::registerBasic();
}

	
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CScene::CScene()
{
	HrcTrav.Scene= this;
	ClipTrav.Scene= this;
	LightTrav.Scene= this;
	AnimDetailTrav.Scene= this;
	LoadBalancingTrav.Scene= this;
	RenderTrav.Scene= this;

	_ShapeBank = NULL;

	Root= NULL;
	RootCluster= NULL;
	SonsOfAncestorSkeletonModelGroup= NULL;
	_QuadGridClipManager= NULL;

	_CurrentTime = 0 ;
	_EllapsedTime = 0 ;
	_RealTime = 0 ;
	_FirstAnimateCall = true ;

	_LightingSystemEnabled= false;
	_CoarseMeshLightingUpdate= 50;

	_GlobalWindDirection.set(1,0,0);
	// Default as Sithikt wants.
	_GlobalWindPower= 0.2f;

	// global manager (created in CDriverUser)
	_LodCharacterManager= NULL;
	_AsyncTextureManager= NULL;

	_NumRender = 0;

	_MaxSkeletonsInNotCLodForm= 20;

	_FilterRenderFlags= ~0;

	_NextRenderProfile= false;

	// Init default _CoarseMeshManager
	_CoarseMeshManager= new CCoarseMeshManager;
	_CoarseMeshManager->setTextureFile (NL3D_SCENE_COARSE_MANAGER_TEXTURE);

	// Update model list to NULL
	_UpdateModelList= NULL;
}
// ***************************************************************************
void	CScene::release()
{
	// terminate async loading
	CAsyncFileManager::terminate();

	// reset the _QuadGridClipManager, => unlink models, and delete clusters.
	if( _QuadGridClipManager )
		_QuadGridClipManager->reset();

	// First, delete models
	set<CTransform*>::iterator	it;
	it= _Models.begin();
	while( it!=_Models.end())
	{
		deleteModel(*it);
		it= _Models.begin();
	}
	// No models at all.
	_UpdateModelList= NULL;

	// reset ptrs
	_ShapeBank = NULL;
	Root= NULL;
	RootCluster= NULL;
	SonsOfAncestorSkeletonModelGroup= NULL;
	CurrentCamera= NULL;
	_QuadGridClipManager= NULL;
	ClipTrav.setQuadGridClipManager(NULL);

	// reset the _LodCharacterManager
	if(_LodCharacterManager)
		_LodCharacterManager->reset();

	// delete the coarseMeshManager
	if(_CoarseMeshManager)
	{
		delete _CoarseMeshManager;
		_CoarseMeshManager= NULL;
	}
}
// ***************************************************************************
CScene::~CScene()
{
	release();
}
// ***************************************************************************
void	CScene::initDefaultRoots()
{
	// Create and set root the default models.
	Root= static_cast<CTransform*>(createModel(TransformId));

	// The root is always freezed (never move).
	Root->freeze();

	// Init the instance group that represent the world
	_GlobalInstanceGroup = new CInstanceGroup;
	RootCluster= (CCluster*)createModel (ClusterId);
	// unlink from hrc.
	RootCluster->hrcUnlink();
	RootCluster->Name = "ClusterRoot";
	RootCluster->Group = _GlobalInstanceGroup;
	_GlobalInstanceGroup->addCluster (RootCluster);

	// init the ClipTrav.RootCluster.
	ClipTrav.RootCluster = RootCluster;

	// Create a SonsOfAncestorSkeletonModelGroup, for models which have a skeleton ancestor
	SonsOfAncestorSkeletonModelGroup= static_cast<CRootModel*>(createModel(RootModelId));
	// must unlink it from all traversals, because special, only used in CClipTrav::traverse()
	SonsOfAncestorSkeletonModelGroup->hrcUnlink();
	Root->clipDelChild(SonsOfAncestorSkeletonModelGroup);
}

// ***************************************************************************
void	CScene::initQuadGridClipManager ()
{
	// Init clip features.
	if( !_QuadGridClipManager )
	{
		// create the model
		_QuadGridClipManager= static_cast<CQuadGridClipManager*>(createModel(QuadGridClipManagerId));
		// unlink it from hrc, and link it only to RootCluster. 
		// NB: hence the quadGridClipManager may be clipped by the cluster system
		_QuadGridClipManager->hrcUnlink();
		_QuadGridClipManager->clipUnlinkFromAll();
		RootCluster->clipAddChild(_QuadGridClipManager);

		// init _QuadGridClipManager.
		_QuadGridClipManager->init(NL3D_QuadGridClipClusterSize, 
			NL3D_QuadGridClipNumDist, 
			NL3D_QuadGridClipMaxDist, 
			NL3D_QuadGridClipManagerRadiusMax);
	}
}


// ***************************************************************************
void	CScene::render(bool	doHrcPass)
{
	double fNewGlobalSystemTime = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	_DeltaSystemTimeBetweenRender= fNewGlobalSystemTime - _GlobalSystemTime;
	_GlobalSystemTime = fNewGlobalSystemTime;
	//
	++ _NumRender;
	//
	nlassert(CurrentCamera);

	// update models.
	updateModels();

	// Use the camera to setup Clip / Render pass.
	float left, right, bottom, top, znear, zfar;
	CurrentCamera->getFrustum(left, right, bottom, top, znear, zfar);

	// setup basic camera.
	ClipTrav.setFrustum(left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());

	RenderTrav.setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());
	RenderTrav.setViewport (_Viewport);

	LoadBalancingTrav.setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());


	// Set Infos for cliptrav.
	ClipTrav.Camera = CurrentCamera;
	ClipTrav.setQuadGridClipManager (_QuadGridClipManager);


	// **** For all render traversals, traverse them (except the Hrc one), in ascending order.
	if( doHrcPass )
		HrcTrav.traverse();

	// Set Cam World Matrix for all trav that need it
	ClipTrav.setCamMatrix(CurrentCamera->getWorldMatrix());
	RenderTrav.setCamMatrix (CurrentCamera->getWorldMatrix());
	LoadBalancingTrav.setCamMatrix (CurrentCamera->getWorldMatrix());

	
	// clip
	ClipTrav.traverse();
	// animDetail
	AnimDetailTrav.traverse();
	// loadBalance
	LoadBalancingTrav.traverse();
	// Light
	LightTrav.traverse();
	// render
	RenderTrav.traverse();


	// **** Misc.

	/** Particle system handling (remove the resources of those which are too far, as their clusters may not have been parsed).
      * Note that only a few of them are tested at each call
	  */
	_ParticleSystemManager.refreshModels(ClipTrav.WorldFrustumPyramid, ClipTrav.CamPos);
	
	// Waiting Instance handling
	double deltaT = _DeltaSystemTimeBetweenRender;
	clamp (deltaT, 0.01, 0.1);
	updateWaitingInstances(deltaT);

	// Reset profiling
	_NextRenderProfile= false;
}

// ***************************************************************************
void CScene::updateWaitingInstances(double systemTimeEllapsed)
{	
	// First set up max AGP upload
	double fMaxBytesToUp = 100 * 256 * 256 * systemTimeEllapsed;	
	_ShapeBank->setMaxBytesToUpload ((uint32)fMaxBytesToUp);
	// Parse all the waiting instance
	_ShapeBank->processWaitingShapes ();	// Process waiting shapes load shape, texture, and lightmaps
											// and upload all maps to VRAM pieces by pieces
	TWaitingInstancesMMap::iterator wimmIt = _WaitingInstances.begin();
	while( wimmIt != _WaitingInstances.end() )
	{
		CShapeBank::TShapeState st = _ShapeBank->isPresent (wimmIt->first);
		if (st == CShapeBank::AsyncLoad_Error)
		{
			// Delete the waiting instance - Nobody can be informed of that...
			TWaitingInstancesMMap::iterator	itDel= wimmIt;
			++wimmIt;
			_WaitingInstances.erase(itDel);
		}
		else if (st == CShapeBank::Present)
		{
			// Then create a reference to the shape
			*(wimmIt->second) = _ShapeBank->addRef(wimmIt->first)->createInstance (*this);
			// Delete the waiting instance
			TWaitingInstancesMMap::iterator	itDel= wimmIt;
			++wimmIt;
			_WaitingInstances.erase(itDel);
		}
		else // st == CShapeBank::NotPresent or loading
		{
			++wimmIt;
		}
	}
}

// ***************************************************************************
void	CScene::setDriver(IDriver *drv)
{
	RenderTrav.setDriver(drv);
}

// ***************************************************************************
IDriver	*CScene::getDriver() const
{
	return (const_cast<CScene*>(this))->RenderTrav.getDriver();
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
	NL3D_MEM_INSTANCE

	// We must attach a bank to the scene (a ShapeBank handle the shape caches and 
	// the creation/deletion of the instances)
	nlassert( _ShapeBank != NULL );
	
	// If the shape is not present in the bank
	if (_ShapeBank->isPresent( shapeName ) != CShapeBank::Present)
	{
		// Load it from file
		_ShapeBank->load( shapeName );
		if (_ShapeBank->isPresent( shapeName ) != CShapeBank::Present)
		{
			return NULL;
		}
	}
	// Then create a reference to the shape
	CTransformShape *pTShp = _ShapeBank->addRef( shapeName )->createInstance(*this);
	if (pTShp) pTShp->setDistMax(pTShp->Shape->getDistMax());
#ifdef NL_DEBUG
	//pTShp->NameForDebug = shapeName; // \todo traptemp
#endif

	// Look if this instance get lightmap information
#if defined(__GNUC__) && __GNUC__ < 3
	CMeshBase *pMB = (CMeshBase*)((IShape*)(pTShp->Shape));
#else // not GNUC
	CMeshBase *pMB = dynamic_cast<CMeshBase*>((IShape*)(pTShp->Shape));
#endif // not GNUC
	CMeshBaseInstance *pMBI = dynamic_cast<CMeshBaseInstance*>( pTShp );
	if( ( pMB != NULL ) && ( pMBI != NULL ) )
	{ 
		// Init lightmap information
		pMBI->initAnimatedLightIndex (*this);

		// Auto animations
		//==========================

		if (_AutomaticAnimationSet)
		{
			if (pMB->getAutoAnim())
			{
				
				std::string animName = CFile::getFilenameWithoutExtension(shapeName);			
				animName = strlwr (animName);
				uint animID = _AutomaticAnimationSet->getAnimationIdByName(animName);
				if (animID != CAnimationSet::NotFound)
				{
					CChannelMixer *chanMix = new CChannelMixer;
					chanMix->setAnimationSet((CAnimationSet *) _AutomaticAnimationSet);
					chanMix->setSlotAnimation(0, animID);
					
					pMBI->registerToChannelMixer(chanMix, "");
					// Gives this object ownership of the channel mixer so we don't need to keep track of it
					pMBI->setChannelMixerOwnerShip(true);
				}
			}	
		}
	}

	CLandscapeModel *pLM = dynamic_cast<CLandscapeModel*>( pTShp );
	if( pLM != NULL ) 
	{ 
		// Init lightmap information
		pLM->Landscape.initAnimatedLightIndex (*this);
	}

	return pTShp;
}

// ***************************************************************************

void CScene::createInstanceAsync(const string &shapeName, CTransformShape **pInstance, const NLMISC::CVector &position)
{
	// We must attach a bank to the scene (a ShapeBank handle the shape caches and 
	// the creation/deletion of the instances)
	nlassert( _ShapeBank != NULL );
	*pInstance = NULL;
	// Add the instance request
	_WaitingInstances.insert(TWaitingInstancesMMap::value_type(shapeName,pInstance));
	// If the shape is not present in the bank
	if (_ShapeBank->isPresent( shapeName ) != CShapeBank::Present)
	{
		// Load it from file asynchronously
		_ShapeBank->loadAsync( strlwr(shapeName), getDriver(), position );
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

	if (pShp)
	{
		// Even if model already deleted by smarptr the release function works
		_ShapeBank->release( pShp );
	}
	
}

// ***************************************************************************
// ANIMATION FOR LIGHT MAPS
void CScene::setAutoAnim( CAnimation *pAnim )
{
	uint nAnimNb;
	// Reset the automatic animation if no animation wanted
	if( pAnim == NULL )
	{
		_AnimatedLight.clear();
		_AnimatedLightPtr.clear();
		_AnimatedLightNameToIndex.clear();
		nAnimNb = _LightmapAnimations.getAnimationIdByName("Automatic");
		if( nAnimNb != CAnimationSet::NotFound )
		{
			CAnimation *anim = _LightmapAnimations.getAnimation( nAnimNb );
			delete anim;
		}
		_LightmapAnimations.reset();
		_LMAnimsAuto.deleteAll();
		return;
	}


	set<string> setTrackNames;
	pAnim->getTrackNames( setTrackNames );

	nAnimNb = _LightmapAnimations.addAnimation( "Automatic", pAnim );
	_LightmapAnimations.build();
	CChannelMixer *cm = new CChannelMixer();
	cm->setAnimationSet( &_LightmapAnimations );

	set<string>::iterator itSel = setTrackNames.begin();
	while ( itSel != setTrackNames.end() )
	{
		string ate = *itSel;
		if( strncmp( itSel->c_str(), "LightmapController.", 19 ) == 0 )
		{
			// The light name
			const char *lightName = strrchr ((*itSel).c_str (), '.')+1;

			// Add an automatic animation
			_AnimatedLight.push_back ( CAnimatedLightmap (_LightGroupColor.size ()) );
			_AnimatedLightPtr.push_back ( &_AnimatedLight.back () );
			_AnimatedLightNameToIndex.insert ( std::map<std::string, uint>::value_type (lightName, _AnimatedLightPtr.size ()-1 ) );
			CAnimatedLightmap &animLM = _AnimatedLight.back ();
			animLM.setName( *itSel );

			cm->addChannel( animLM.getName(), &animLM, animLM.getValue(CAnimatedLightmap::FactorValue),
				animLM.getDefaultTrack(CAnimatedLightmap::FactorValue), CAnimatedLightmap::FactorValue, 
				CAnimatedLightmap::OwnerBit, false);
		}
		++itSel;
	}

	CAnimationPlaylist *pl = new CAnimationPlaylist();
	pl->setAnimation( 0, nAnimNb );
	pl->setWrapMode( 0, CAnimationPlaylist::Repeat );
	_LMAnimsAuto.addPlaylist(pl,cm);
}

// ***************************************************************************

void CScene::loadLightmapAutoAnim( const std::string &filename )
{
	try
	{
		CAnimation *anim = new CAnimation();
		CIFile fIn( CPath::lookup(filename) );
		anim->serial( fIn );

		setAutoAnim( anim );
	}
	catch(EPathNotFound &)
	{
		return;
	}
}

// ***************************************************************************
void CScene::animate( TGlobalAnimationTime atTime )
{
	if (_FirstAnimateCall)
	{
		_RealTime = atTime ;
		// dummy value for first frame
		_EllapsedTime = 0.01f ;
		_FirstAnimateCall = false ;
	}
	else
	{
		_EllapsedTime = (float) (atTime - _RealTime) ;
		//nlassert(_EllapsedTime >= 0);
		if (_EllapsedTime < 0.0f)	// NT WorkStation PATCH (Yes you are not dreaming
			_EllapsedTime = 0.01f;	// deltaTime can be less than zero!!)
		_EllapsedTime = fabsf(_EllapsedTime);
		_RealTime = atTime ;
		_CurrentTime += _EllapsedTime ;
	}
	
	_LMAnimsAuto.animate( atTime );
	_ParticleSystemManager.processAnimate(_EllapsedTime); // deals with permanently animated particle systems

	// Change PointLightFactors of all pointLights in registered Igs.
	//----------------

	// First list all current AnimatedLightmaps (for faster vector iteration per ig)
	const uint count = _AnimatedLightPtr.size ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Blend final colors
		_AnimatedLightPtr[i]->updateGroupColors (*this);
	}

	// For all registered igs.
	ItAnimatedIgSet		itAnIgSet;
	for(itAnIgSet= _AnimatedIgSet.begin(); itAnIgSet!=_AnimatedIgSet.end(); itAnIgSet++)
	{
		CInstanceGroup	*ig= *itAnIgSet;

		// Set the light factor
		ig->setPointLightFactor(*this);
	}
}


// ***************************************************************************
float	CScene::getNbFaceAsked () const
{
	return LoadBalancingTrav.getNbFaceAsked ();
}


// ***************************************************************************
void	CScene::setGroupLoadMaxPolygon(const std::string &group, uint nFaces)
{
	nFaces= max(nFaces, (uint)1);
	LoadBalancingTrav.setGroupNbFaceWanted(group, nFaces);
}
// ***************************************************************************
uint	CScene::getGroupLoadMaxPolygon(const std::string &group)
{
	return LoadBalancingTrav.getGroupNbFaceWanted(group);
}
// ***************************************************************************
float	CScene::getGroupNbFaceAsked (const std::string &group) const
{
	return LoadBalancingTrav.getGroupNbFaceAsked(group);
}



// ***************************************************************************
void	CScene::setPolygonBalancingMode(TPolygonBalancingMode polBalMode)
{
	LoadBalancingTrav.PolygonBalancingMode= (CLoadBalancingGroup::TPolygonBalancingMode)(uint)polBalMode;
}


// ***************************************************************************
CScene::TPolygonBalancingMode	CScene::getPolygonBalancingMode() const
{
	return (CScene::TPolygonBalancingMode)(uint)LoadBalancingTrav.PolygonBalancingMode;
}

// ***************************************************************************
void  CScene::setLayersRenderingOrder(bool directOrder /*= true*/)
{
	RenderTrav.setLayersRenderingOrder(directOrder);
}

// ***************************************************************************
bool  CScene::getLayersRenderingOrder() const
{
	return 	RenderTrav.getLayersRenderingOrder();
}

// ***************************************************************************
CParticleSystemManager &CScene::getParticleSystemManager()
{
	return _ParticleSystemManager;
}

// ***************************************************************************
void	CScene::enableElementRender(UScene::TRenderFilter elt, bool state)
{
	if(state)
		_FilterRenderFlags|= (uint32)elt;
	else
		_FilterRenderFlags&= ~(uint32)elt;
}


// ***************************************************************************
// ***************************************************************************
// Lighting Mgt.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CScene::enableLightingSystem(bool enable)
{
	_LightingSystemEnabled= enable;

	// Set to RenderTrav and LightTrav
	RenderTrav.LightingSystemEnabled= _LightingSystemEnabled;
	LightTrav.LightingSystemEnabled= _LightingSystemEnabled;
}


// ***************************************************************************
void			CScene::setAmbientGlobal(NLMISC::CRGBA ambient)
{
	RenderTrav.AmbientGlobal= ambient;
}
void			CScene::setSunAmbient(NLMISC::CRGBA ambient)
{
	RenderTrav.SunAmbient= ambient;
}
void			CScene::setSunDiffuse(NLMISC::CRGBA diffuse)
{
	RenderTrav.SunDiffuse= diffuse;
}
void			CScene::setSunSpecular(NLMISC::CRGBA specular)
{
	RenderTrav.SunSpecular= specular;
}
void			CScene::setSunDirection(const NLMISC::CVector &direction)
{
	RenderTrav.setSunDirection(direction);
}


// ***************************************************************************
NLMISC::CRGBA	CScene::getAmbientGlobal() const
{
	return RenderTrav.AmbientGlobal;
}
NLMISC::CRGBA	CScene::getSunAmbient() const
{
	return RenderTrav.SunAmbient;
}
NLMISC::CRGBA	CScene::getSunDiffuse() const
{
	return RenderTrav.SunDiffuse;
}
NLMISC::CRGBA	CScene::getSunSpecular() const
{
	return RenderTrav.SunSpecular;
}
NLMISC::CVector	CScene::getSunDirection() const
{
	return RenderTrav.getSunDirection();
}


// ***************************************************************************
void		CScene::setMaxLightContribution(uint nlights)
{
	LightTrav.LightingManager.setMaxLightContribution(nlights);
}
uint		CScene::getMaxLightContribution() const
{
	return LightTrav.LightingManager.getMaxLightContribution();
}

void		CScene::setLightTransitionThreshold(float lightTransitionThreshold)
{
	LightTrav.LightingManager.setLightTransitionThreshold(lightTransitionThreshold);
}
float		CScene::getLightTransitionThreshold() const
{
	return LightTrav.LightingManager.getLightTransitionThreshold();
}


// ***************************************************************************
void		CScene::addInstanceGroupForLightAnimation(CInstanceGroup *ig)
{
	nlassert( ig );
	nlassert( _AnimatedIgSet.find(ig) == _AnimatedIgSet.end() );
	_AnimatedIgSet.insert(ig);
}

// ***************************************************************************
void		CScene::removeInstanceGroupForLightAnimation(CInstanceGroup *ig)
{
	nlassert( ig );
	ItAnimatedIgSet		itIg= _AnimatedIgSet.find(ig);
	if ( itIg != _AnimatedIgSet.end() )
		_AnimatedIgSet.erase(itIg);
}


// ***************************************************************************
void		CScene::setCoarseMeshLightingUpdate(uint8 period)
{
	_CoarseMeshLightingUpdate= max((uint8)1, period);
}


// ***************************************************************************
// ***************************************************************************
/// Weather mgt
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void		CScene::setGlobalWindPower(float gwp)
{
	_GlobalWindPower= gwp;
}
// ***************************************************************************
void		CScene::setGlobalWindDirection(const CVector &gwd)
{
	_GlobalWindDirection= gwd;
	_GlobalWindDirection.z= 0;
	_GlobalWindDirection.normalize();
}


// ***************************************************************************
// ***************************************************************************
/// Private
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
CScene::ItSkeletonModelList	CScene::appendSkeletonModelToList(CSkeletonModel *skel)
{
	_SkeletonModelList.push_front(skel);
	return _SkeletonModelList.begin();
}

// ***************************************************************************
void					CScene::eraseSkeletonModelToList(CScene::ItSkeletonModelList	it)
{
	_SkeletonModelList.erase(it);
}

// ***************************************************************************
// ***************************************************************************
/// Misc
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void					CScene::profileNextRender()
{
	_NextRenderProfile= true;

	// Reset All Stats.
	BenchRes.reset();
}


// ***************************************************************************
// ***************************************************************************
// Old CMOT integrated methods
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
set<CScene::CModelEntry>	CScene::_RegModels;


// ***************************************************************************
void	CScene::registerModel(const CClassId &idModel, const CClassId &idModelBase, CTransform* (*creator)())
{
	nlassert(idModel!=CClassId::Null);
	nlassert(creator);
	// idModelBase may be Null...

	CModelEntry		e;
	e.BaseModelId= idModelBase;
	e.ModelId= idModel;
	e.Creator= creator;

	// Insert/replace e.
	_RegModels.erase(e);
	_RegModels.insert(e);
}


// ***************************************************************************
CTransform	*CScene::createModel(const CClassId &idModel)
{
	nlassert(idModel!=CClassId::Null);

	CModelEntry	e;
	e.ModelId= idModel;
	set<CModelEntry>::iterator	itModel;
	itModel= _RegModels.find(e);

	if(itModel==_RegModels.end())
	{
		nlstop;			// Warning, CScene::registerBasics () has not been called !
		return NULL;
	}
	else
	{
		CTransform	*m= (*itModel).Creator();
		if(!m)	return NULL;

		// Set the owner for the model.
		m->_OwnerScene= this;

		// link model to Root in HRC and in clip. NB: if exist!! (case for the Root and RootCluster :) )
		if(Root)
		{
			Root->hrcLinkSon(m);
			Root->clipAddChild(m);
		}

		// Insert the model into the set.
		_Models.insert(m);

		// By default the model is update() in CScene::updateModels().
		m->linkToUpdateList();

		// Once the model is correclty created, finish init him.
		m->initModel();

		// Ensure all the Traversals has enough space for visible list.
		ClipTrav.reserveVisibleList(_Models.size());
		AnimDetailTrav.reserveVisibleList(_Models.size());
		LoadBalancingTrav.reserveVisibleList(_Models.size());
		LightTrav.reserveLightedList(_Models.size());
		RenderTrav.reserveRenderList(_Models.size());

		return m;
	}
}
// ***************************************************************************
void	CScene::deleteModel(CTransform *model)
{
	if(model==NULL)
		return;
	set<CTransform*>::iterator	it= _Models.find(model);
	if(it!=_Models.end())
	{
		delete *it;
		_Models.erase(it);
	}
}


// ***************************************************************************
void	CScene::updateModels()
{
	// check all the models which must be checked.
	CTransform	*model= _UpdateModelList;
	CTransform	*next;
	while( model )
	{
		// next to update. get next now, because model->update() may remove model from the list.
		next= model->_NextModelToUpdate;

		// update the model.
		model->update();

		// next.
		model= next;
	}
}


// ***************************************************************************
void	CScene::setLightGroupColor(uint lightmapGroup, NLMISC::CRGBA color)
{
	// If too small, resize with white
	if (lightmapGroup >= _LightGroupColor.size ())
	{
		_LightGroupColor.resize (lightmapGroup+1, CRGBA::White);
	}

	// Set the color
	_LightGroupColor[lightmapGroup] = color;
}


// ***************************************************************************
sint CScene::getAnimatedLightNameToIndex (const std::string &name) const
{
	std::map<std::string, uint>::const_iterator ite = _AnimatedLightNameToIndex.find (name);
	if (ite != _AnimatedLightNameToIndex.end ())
		return (sint)ite->second;
	else
		return -1;
}

} // NL3D
