/** \file scene.cpp
 * A 3d scene, manage model instantiation, tranversals etc..
 *
 * $Id: scene.cpp,v 1.94 2003/03/20 14:55:18 berenguier Exp $
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
#include "3d/skip_model.h"
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

#define NL3D_SCENE_QUADGRID_CLIP_CLUSTER_SIZE	400
const	float	NL3D_QuadGridClipManagerMaxDist[]= {200, 400, 600};
// The manager is limited to a square of 3000m*3000m around the camera. Beyond, models are clipped individually (bad!!).
const	float	NL3D_QuadGridClipManagerRadiusMax= 1500;


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
	CSkipModel::registerBasic();
	CWaterModel::registerBasic();
	CWaveMakerModel::registerBasic();
	CVegetableBlendLayerModel::registerBasic();
	CRootModel::registerBasic();
	CPointLightModel::registerBasic();
	CSegRemanence::registerBasic();
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
	LoadBalancingTrav= NULL;
	RenderTrav= NULL;

	_ShapeBank = NULL;

	Root= NULL;
	SkipModelRoot= NULL;
	SonsOfAncestorSkeletonModelGroup= NULL;
	LightModelRoot= NULL;

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
}
// ***************************************************************************
void	CScene::release()
{
	// terminate async loading
	CAsyncFileManager::terminate();

	// reset the _QuadGridClipManager, => unlink models, and delete clusters.
	_QuadGridClipManager.reset();

	// First, delete models and un-register traversals.
	CMOT::release();

	for (uint k = 0; k < getNumTrav(); ++k)
	{
		ITravScene *ts = dynamic_cast<ITravScene *>(getTrav(k));
		if (ts)
		{
			ts->Scene =	NULL; 
		}
	}

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

	if (LoadBalancingTrav != NULL)
	{
		delete LoadBalancingTrav;
		LoadBalancingTrav= NULL;
	}

	if (RenderTrav != NULL)
	{
		delete	RenderTrav;
		RenderTrav= NULL;
	}

	_ShapeBank = NULL;
	Root= NULL;
	SkipModelRoot= NULL;
	SonsOfAncestorSkeletonModelGroup= NULL;
	LightModelRoot= NULL;
	CurrentCamera= NULL;

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
void	CScene::initDefaultTravs()
{
	NL3D_MEM_MOT

	// Add the 4 default traversals.
	HrcTrav= new CHrcTrav;
	ClipTrav= new CClipTrav;
	LightTrav= new CLightTrav;
	AnimDetailTrav= new CAnimDetailTrav;
	LoadBalancingTrav= new CLoadBalancingTrav;
	RenderTrav= new CRenderTrav;

	// Register them to the scene.
	addTrav(HrcTrav);
	addTrav(ClipTrav);
	addTrav(LightTrav);
	addTrav(AnimDetailTrav);
	addTrav(LoadBalancingTrav);
	addTrav(RenderTrav);
}
// ***************************************************************************
void	CScene::initDefaultRoots()
{
	// Create and set root the default models.
	Root= static_cast<CTransform*>(createModel(TransformId));
	HrcTrav->setRoot(Root);
	ClipTrav->setRoot(Root);

	// need no root for AnimDetailTrav, LoadBalancingTrav, LightTrav and RenderTrav
	// because all of them use either the ClipVisibilityList or their own list.
	// AnimDetailTrav->setRoot(Root);
	// LoadBalancingTrav->setRoot(Root);
	// LightTrav->setRoot(Root);
	// RenderTrav->setRoot(Root);


	// The root is always freezed (never move).
	Root->freeze();


	// Create a SkipModelRoot, for CTransform::freezeHRC().
	SkipModelRoot= static_cast<CSkipModel*>(createModel(SkipModelId));


	// Create a SonsOfAncestorSkeletonModelGroup, for models which have a skeleton ancestor
	SonsOfAncestorSkeletonModelGroup= static_cast<CRootModel*>(createModel(RootModelId));
	// must unlink it from all traversals, because special, only used in CClipTrav::traverse()
	HrcTrav->unlink(NULL, SonsOfAncestorSkeletonModelGroup);
	ClipTrav->unlink(NULL, SonsOfAncestorSkeletonModelGroup);
	AnimDetailTrav->unlink(NULL, SonsOfAncestorSkeletonModelGroup);
	LoadBalancingTrav->unlink(NULL, SonsOfAncestorSkeletonModelGroup);
	LightTrav->unlink(NULL, SonsOfAncestorSkeletonModelGroup);
	RenderTrav->unlink(NULL, SonsOfAncestorSkeletonModelGroup);
	// inform the clipTrav of this model.
	ClipTrav->setSonsOfAncestorSkeletonModelGroup(SonsOfAncestorSkeletonModelGroup);


	// init root for Lighting.
	LightModelRoot= static_cast<CRootModel*>(createModel(RootModelId));
	// unlink it from all traversals, because special, only used in CLightTrav::traverse()
	HrcTrav->unlink(NULL, LightModelRoot);
	ClipTrav->unlink(NULL, LightModelRoot);
	AnimDetailTrav->unlink(NULL, LightModelRoot);
	LoadBalancingTrav->unlink(NULL, LightModelRoot);
	LightTrav->unlink(NULL, LightModelRoot);
	RenderTrav->unlink(NULL, LightModelRoot);
	// inform the LightTrav of this model.
	LightTrav->setLightModelRoot(LightModelRoot);
}

// ***************************************************************************
void	CScene::initGlobalnstanceGroup ()
{
	// Init the instance group that represent the world
	_GlobalInstanceGroup = new CInstanceGroup;
	CCluster *pCluster = (CCluster*)createModel (ClusterId);
	CClipTrav *pClipTrav = (CClipTrav*)(getTrav (ClipTravId));
	pClipTrav->unlink (NULL, pCluster);
	pCluster->Name = "ClusterRoot";
	pCluster->Group = _GlobalInstanceGroup;
	_GlobalInstanceGroup->addCluster (pCluster);

	// init the ClipTrav->RootCluster.
	pClipTrav->RootCluster = _GlobalInstanceGroup->_ClusterInstances[0];
}


// ***************************************************************************
void	CScene::initQuadGridClipManager ()
{
	// Init clip features.
	// setup maxDists clip.
	vector<float>	maxDists;
	maxDists.resize( sizeof(NL3D_QuadGridClipManagerMaxDist) / sizeof(NL3D_QuadGridClipManagerMaxDist[0]) );
	for(uint i=0; i<maxDists.size(); i++)
		maxDists[i]= NL3D_QuadGridClipManagerMaxDist[i];
	// init _QuadGridClipManager.
	_QuadGridClipManager.init(this, NL3D_SCENE_QUADGRID_CLIP_CLUSTER_SIZE, maxDists, NL3D_QuadGridClipManagerRadiusMax);
}


// ***************************************************************************
void	CScene::addTrav(ITrav *v)
{
	nlassert(v);
	sint	order=0;

	ITravScene	*sv= dynamic_cast<ITravScene*>(v);
	if(sv)
	{		
		order= sv->getRenderOrder();
	}

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
	double fNewGlobalSystemTime = NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime());
	_DeltaSystemTimeBetweenRender= fNewGlobalSystemTime - _GlobalSystemTime;
	_GlobalSystemTime = fNewGlobalSystemTime;
	//
	++ _NumRender;
	//
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

	LoadBalancingTrav->setFrustum (left, right, bottom, top, znear, zfar, CurrentCamera->isPerspective());


	// Set the renderTrav for cliptrav.
	ClipTrav->setHrcTrav (HrcTrav);
	ClipTrav->setLightTrav (LightTrav);
	ClipTrav->setAnimDetailTrav (AnimDetailTrav);
	ClipTrav->setLoadBalancingTrav (LoadBalancingTrav);
	ClipTrav->setRenderTrav (RenderTrav);
	ClipTrav->Camera = CurrentCamera;
	ClipTrav->setQuadGridClipManager (&_QuadGridClipManager);

	// For all render traversals, traverse them (except the Hrc one), in ascending order.
	TTravMap::iterator	it;
	for(it= RenderTraversals.begin(); it!= RenderTraversals.end(); it++)
	{
		ITravScene	*trav= (*it).second;
		// TestYoyo
		/*if(AnimDetailTravId!=trav->getClassId() && 
			LoadBalancingTravId!=trav->getClassId() && 
			HrcTravId!=trav->getClassId() && 
			ClipTravId!=trav->getClassId() )
			continue;*/
		// maybe don't traverse HRC pass.
		if(doHrcPass || HrcTravId!=trav->getClassId())
		{

			// Go!
			trav->traverse();

		}
		// if HrcTrav done, set World Camera matrix for Clip and Render.
		if(HrcTravId==trav->getClassId())
		{
			CTransformHrcObs	*camObs= (CTransformHrcObs*)CMOT::getModelObs(CurrentCamera, HrcTravId);
			ClipTrav->setCamMatrix(camObs->WorldMatrix);
			RenderTrav->setCamMatrix (camObs->WorldMatrix);
			LoadBalancingTrav->setCamMatrix (camObs->WorldMatrix);
		}
	}

	/** Particle system handling (remove the resources of those which are too far, as their clusters may not have been parsed).
      * Not that only a few of them are tested at each call
	  */
	_ParticleSystemManager.refreshModels(ClipTrav->WorldFrustumPyramid, ClipTrav->CamPos);
	
	// Wainting Instance handling
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
//	pTShp->NameForDebug = shapeName; // \todo traptemp
#endif

	// Look if this instance get lightmap information
#if defined(__GNUC__) && __GNUC__ < 3
	CMeshBase *pMB = (CMeshBase*)((IShape*)(pTShp->Shape));
#else // not GNUC
	CMeshBase *pMB = dynamic_cast<CMeshBase*>((IShape*)(pTShp->Shape));
#endif // not GNUC
	CMeshBaseInstance *pMBI = dynamic_cast<CMeshBaseInstance*>( pTShp );
	if( ( pMB != NULL ) && ( pMBI != NULL ) )
	{ // Try to bind to automatic animation
		CMeshBase::TLightInfoMap::iterator itLM = pMB->_LightInfos.begin();
		while( itLM != pMB->_LightInfos.end() )
		{	// Is it the same name in the name of the lightmap ?
			set<CAnimatedLightmap*>::iterator itSet = _AnimatedLightmap.begin();
			while( itSet != _AnimatedLightmap.end() )
			{
				const char *GroupName = strchr( (*itSet)->getName().c_str(), '.')+1;
				if( GroupName == itLM->first )
				{
					// Ok bind automatic animation
					pMBI->setAnimatedLightmap( *itSet );
				}
				++itSet;
			}
			++itLM;
		}

		// Auto animations
		//==========================

		if (_AutomaticAnimationSet)
		{
			if (pMB->getAutoAnim())
			{
				
				std::string animName = CFile::getFilenameWithoutExtension(shapeName);			
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

	return pTShp;
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
	if (_ShapeBank->isPresent( shapeName ) != CShapeBank::Present)
	{
		// Load it from file asynchronously
		_ShapeBank->loadAsync( strlwr(shapeName), getDriver() );
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
		set<CAnimatedLightmap*>::iterator itSAL = _AnimatedLightmap.begin();
		while( itSAL != _AnimatedLightmap.end() )
		{
			delete *itSAL;
			++itSAL;
		}
		_AnimatedLightmap.clear();
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
			CAnimatedLightmap *animLM = new CAnimatedLightmap();
			animLM->setName( *itSel );

			cm->addChannel( animLM->getName(), animLM, animLM->getValue(CAnimatedLightmap::FactorValue),
				animLM->getDefaultTrack(CAnimatedLightmap::FactorValue), CAnimatedLightmap::FactorValue, 
				CAnimatedLightmap::OwnerBit, false);

			//animLM->registerToChannelMixer( cm, "" );
			_AnimatedLightmap.insert( animLM );

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
	static	vector<string>	anlNames;
	static	vector<CRGBA>	anlFactors;
	anlNames.clear();
	anlFactors.clear();
	// First list all current AnimatedLightmaps (for faster vector iteration per ig)
	std::set<CAnimatedLightmap*>::iterator	itAnlSet;
	for(itAnlSet= _AnimatedLightmap.begin(); itAnlSet!=_AnimatedLightmap.end(); itAnlSet++)
	{
		const char *GroupName = strchr( (*itAnlSet)->getName().c_str(), '.')+1;
		// Append to vector
		anlNames.push_back(GroupName);
		anlFactors.push_back( (*itAnlSet)->getFactor() );
	}

	// For all registered igs.
	ItAnimatedIgSet		itAnIgSet;
	for(itAnIgSet= _AnimatedIgSet.begin(); itAnIgSet!=_AnimatedIgSet.end(); itAnIgSet++)
	{
		CInstanceGroup	*ig= *itAnIgSet;
		// For all Animated Light Factor
		for(uint i= 0; i<anlNames.size(); i++)
		{
			ig->setPointLightFactor(anlNames[i], anlFactors[i]);
		}
	}
}


// ***************************************************************************
float	CScene::getNbFaceAsked () const
{
	nlassert(LoadBalancingTrav);
	return LoadBalancingTrav->getNbFaceAsked ();
}


// ***************************************************************************
void	CScene::setGroupLoadMaxPolygon(const std::string &group, uint nFaces)
{
	nlassert(LoadBalancingTrav);
	nFaces= max(nFaces, (uint)1);
	LoadBalancingTrav->setGroupNbFaceWanted(group, nFaces);
}
// ***************************************************************************
uint	CScene::getGroupLoadMaxPolygon(const std::string &group)
{
	nlassert(LoadBalancingTrav);
	return LoadBalancingTrav->getGroupNbFaceWanted(group);
}
// ***************************************************************************
float	CScene::getGroupNbFaceAsked (const std::string &group) const
{
	nlassert(LoadBalancingTrav);
	return LoadBalancingTrav->getGroupNbFaceAsked(group);
}



// ***************************************************************************
void	CScene::setPolygonBalancingMode(TPolygonBalancingMode polBalMode)
{
	nlassert(LoadBalancingTrav);
	LoadBalancingTrav->PolygonBalancingMode= (CLoadBalancingGroup::TPolygonBalancingMode)(uint)polBalMode;
}


// ***************************************************************************
CScene::TPolygonBalancingMode	CScene::getPolygonBalancingMode() const
{
	nlassert(LoadBalancingTrav);
	return (CScene::TPolygonBalancingMode)(uint)LoadBalancingTrav->PolygonBalancingMode;
}

// ***************************************************************************
void  CScene::setLayersRenderingOrder(bool directOrder /*= true*/)
{
	nlassert(RenderTrav);
	RenderTrav->setLayersRenderingOrder(directOrder);
}

// ***************************************************************************
bool  CScene::getLayersRenderingOrder() const
{
	nlassert(RenderTrav);
	return 	RenderTrav->getLayersRenderingOrder();
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
	RenderTrav->LightingSystemEnabled= _LightingSystemEnabled;
	LightTrav->LightingSystemEnabled= _LightingSystemEnabled;
}


// ***************************************************************************
void			CScene::setAmbientGlobal(NLMISC::CRGBA ambient)
{
	RenderTrav->AmbientGlobal= ambient;
}
void			CScene::setSunAmbient(NLMISC::CRGBA ambient)
{
	RenderTrav->SunAmbient= ambient;
}
void			CScene::setSunDiffuse(NLMISC::CRGBA diffuse)
{
	RenderTrav->SunDiffuse= diffuse;
}
void			CScene::setSunSpecular(NLMISC::CRGBA specular)
{
	RenderTrav->SunSpecular= specular;
}
void			CScene::setSunDirection(const NLMISC::CVector &direction)
{
	RenderTrav->setSunDirection(direction);
}


// ***************************************************************************
NLMISC::CRGBA	CScene::getAmbientGlobal() const
{
	return RenderTrav->AmbientGlobal;
}
NLMISC::CRGBA	CScene::getSunAmbient() const
{
	return RenderTrav->SunAmbient;
}
NLMISC::CRGBA	CScene::getSunDiffuse() const
{
	return RenderTrav->SunDiffuse;
}
NLMISC::CRGBA	CScene::getSunSpecular() const
{
	return RenderTrav->SunSpecular;
}
NLMISC::CVector	CScene::getSunDirection() const
{
	return RenderTrav->getSunDirection();
}


// ***************************************************************************
void		CScene::setMaxLightContribution(uint nlights)
{
	LightTrav->LightingManager.setMaxLightContribution(nlights);
}
uint		CScene::getMaxLightContribution() const
{
	return LightTrav->LightingManager.getMaxLightContribution();
}

void		CScene::setLightTransitionThreshold(float lightTransitionThreshold)
{
	LightTrav->LightingManager.setLightTransitionThreshold(lightTransitionThreshold);
}
float		CScene::getLightTransitionThreshold() const
{
	return LightTrav->LightingManager.getLightTransitionThreshold();
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
	nlassert( itIg != _AnimatedIgSet.end() );
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


} // NL3D
