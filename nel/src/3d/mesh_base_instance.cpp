/** \file mesh_base_instance.cpp
 * <File description>
 *
 * $Id: mesh_base_instance.cpp,v 1.16 2002/10/29 14:40:00 berenguier Exp $
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

#include "3d/mesh_base_instance.h"
#include "3d/mesh_base.h"
#include "3d/scene.h"
#include "3d/animation.h"
#include "nel/misc/debug.h"
#include "3d/anim_detail_trav.h"
#include "3d/texture_file.h"
#include "3d/async_texture_manager.h"


using namespace NLMISC;

namespace NL3D 
{


// ***************************************************************************
CMeshBaseInstance::CMeshBaseInstance()
{
	IAnimatable::resize(AnimValueLast);
	_OwnerScene= NULL;
	_AsyncTextureToLoadRefCount= 0;
	_AsyncTextureMode= false;
	_AsyncTextureReady= true;
	_AsyncTextureDirty= false;
	_AsyncTextureDistance= 0;

	// I am a CMeshBaseInstance!!
	CTransform::setIsMeshBaseInstance(true);
}

// ***************************************************************************
CMeshBaseInstance::~CMeshBaseInstance()
{
	// If AsyncTextureMode, must disable. This ensure that async loading stop, and that no ref still exist
	// in the AsyncTextureManager
	if(_AsyncTextureMode)
		enableAsyncTextureMode(false);
}


// ***************************************************************************
void		CMeshBaseInstance::registerBasic()
{
	CMOT::registerModel(MeshBaseInstanceId, TransformShapeId, CMeshBaseInstance::creator);
	CMOT::registerObs(AnimDetailTravId, MeshBaseInstanceId, CMeshBaseInstanceAnimDetailObs::creator);
}


// ***************************************************************************
void		CMeshBaseInstance::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	uint32 i;
	CTransformShape::registerToChannelMixer(chanMixer, prefix);

	// Add any materials.
	for (i = 0; i < _AnimatedMaterials.size(); i++)
	{
		// append material  matname.*
		_AnimatedMaterials[i].registerToChannelMixer(chanMixer, prefix + _AnimatedMaterials[i].getMaterialName() + ".");
	}

	// Add any morph
	for (i = 0; i < _AnimatedMorphFactor.size(); i++)
	{
		_AnimatedMorphFactor[i].registerToChannelMixer(chanMixer, prefix + _AnimatedMorphFactor[i].getName());
	}
}


// ***************************************************************************
ITrack*		CMeshBaseInstance::getDefaultTrack (uint valueId)
{
	// Pointer on the CMeshBase
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;

	// Switch the value
	switch (valueId)
	{
	case CTransform::PosValue:			
		return pMesh->getDefaultPos();
	case CTransform::RotEulerValue:		
		return pMesh->getDefaultRotEuler();
	case CTransform::RotQuatValue:		
		return pMesh->getDefaultRotQuat();
	case CTransform::ScaleValue:		
		return pMesh->getDefaultScale();
	case CTransform::PivotValue:		
		return pMesh->getDefaultPivot();
	default:
		// Problem, new values ?
		nlstop;
	};
	return NULL;
}


// ***************************************************************************
uint32 CMeshBaseInstance::getNbLightMap()
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	return pMesh->_LightInfos.size();
}

// ***************************************************************************
void CMeshBaseInstance::getLightMapName( uint32 nLightMapNb, std::string &LightMapName )
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	if( nLightMapNb >= pMesh->_LightInfos.size() )
		return;
	CMeshBase::TLightInfoMap::iterator itMap = pMesh->_LightInfos.begin();
	for( uint32 i = 0; i < nLightMapNb; ++i ) ++itMap;
	LightMapName = itMap->first;
}

// ***************************************************************************
void CMeshBaseInstance::setLightMapFactor( const std::string &LightMapName, CRGBA Factor )
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	CMeshBase::TLightInfoMap::iterator itMap = pMesh->_LightInfos.find( LightMapName );
	if( itMap == pMesh->_LightInfos.end() )
		return;
	CMeshBase::CLightInfoMapList::iterator itList = itMap->second.begin();
	uint32 nNbElt = itMap->second.size();
	for( uint32 i = 0; i < nNbElt; ++i )
	{
		Materials[itList->nMatNb].setLightMapFactor( itList->nStageNb, Factor );
		++itList;
	}
}

// ***************************************************************************
uint32 CMeshBaseInstance::getNbBlendShape()
{
	return _AnimatedMorphFactor.size();
}

// ***************************************************************************
void CMeshBaseInstance::getBlendShapeName (uint32 nBlendShapeNb, std::string &BlendShapeName )
{
	if (nBlendShapeNb >= _AnimatedMorphFactor.size())
		return;
	BlendShapeName = _AnimatedMorphFactor[nBlendShapeNb].getName();
}

// ***************************************************************************
void CMeshBaseInstance::setBlendShapeFactor (const std::string &BlendShapeName, float rFactor)
{
	for (uint32 i = 0; i < _AnimatedMorphFactor.size(); ++i)
		if (BlendShapeName == _AnimatedMorphFactor[i].getName())
		{
			_AnimatedMorphFactor[i].setFactor (rFactor);
		}
}


// ***************************************************************************
void CMeshBaseInstanceAnimDetailObs::traverse(IObs *caller)
{
	
	CMeshBaseInstance	*mi = (CMeshBaseInstance*)Model;
	CMeshBase			*mb = NLMISC::safe_cast<CMeshBase *>((IShape *) mi->Shape);

	// if the base instance uses automatic animations, we must also setup the date of the channel mixer controlling this object
	if (mb->getAutoAnim())
	{
		// setup the channel mixer date
		CChannelMixer *chanMix = mi->getChannelMixer();
		if (chanMix)
		{			
			ITravScene	*ts = NLMISC::safe_cast<ITravScene *>(Trav);
			nlassert(ts->Scene);
			const CAnimation *anim = chanMix->getSlotAnimation(0);
			/** We perform wrapping ourselves.
			  * We avoid using a playlist, to not create one more obj.
			  */
			if (anim)
			{
				float animLenght = anim->getEndTime() - anim->getBeginTime();
				if (animLenght > 0)
				{
					float currTime = (TAnimationTime) ts->Scene->getCurrentTime();
					float startTime = (uint) (currTime / animLenght) * animLenght;
					// Set the channel mixer date using the global date of the scene
					chanMix->setSlotTime(0, anim->getBeginTime() + currTime - startTime);
				}
				else
				{
					chanMix->setSlotTime(0, anim->getBeginTime());
				}

				/** temp: eval non detail animation 
				  * The issue here is that this evaluation is performed after clipping.
				  * This means that rotation will be ok, but translation may not work
				  */
				chanMix->eval(false);
			}
		}
	}

	CTransformAnimDetailObs::traverse(caller);

	
	// update animated materials.
	// test if animated materials must be updated.
	if(mi->IAnimatable::isTouched(CMeshBaseInstance::OwnerBit))
	{
		// must test / update all AnimatedMaterials.
		for(uint i=0;i<mi->_AnimatedMaterials.size();i++)
		{
			// This test and update the pointed material.
			mi->_AnimatedMaterials[i].update();
		}

		mi->IAnimatable::clearFlag(CMeshBaseInstance::OwnerBit);
	}

	// Lightmap automatic animation
	for( uint i = 0; i < mi->_AnimatedLightmap.size(); ++i )
	{
		const char *LightGroupName = strchr( mi->_AnimatedLightmap[i]->getName().c_str(), '.' )+1;
		mi->setLightMapFactor(	LightGroupName,
								mi->_AnimatedLightmap[i]->getFactor() );
	}
}


// ***************************************************************************
void CMeshBaseInstance::selectTextureSet(uint id)
{
	nlassert(Shape);
	CMeshBase *mb = NLMISC::safe_cast<CMeshBase *>((IShape *) Shape);
	const uint numMat = mb->getNbMaterial();
	nlassert(numMat == Materials.size());
	// see which material are selectable
	for(uint k = 0; k < numMat; ++k)
	{
		CMaterial &mat = mb->getMaterial(k);
		for(uint l = 0; l < IDRV_MAT_MAXTEXTURES; ++l)
		{
			if (mat.getTexture(l) && mat.getTexture(l)->isSelectable())
			{
				// use a smartPtr so the textFile will be released if just used to set the name for AsyncTextures.
				CSmartPtr<ITexture>		texNSV= mat.getTexture(l)->buildNonSelectableVersion(id);

				// std case: just replace the texture.
				if(!_AsyncTextureMode)
				{
					Materials[k].setTexture(l, texNSV);
				}
				// Async case
				else
				{
					// If texture file, must copy the texture name
					if(AsyncTextures[k].IsTextureFile[l])
					{
						CTextureFile	*textFile= safe_cast<CTextureFile*>((ITexture*)texNSV);
						AsyncTextures[k].TextureNames[l]= textFile->getFileName();
					}
					// else replace the texture.
					else
						Materials[k].setTexture(l, texNSV);
				}
			}
		}
	}

	// Flag the instance as AsyncTextureDirty if in this mode
	if(_AsyncTextureMode)
	{
		setAsyncTextureDirty(true);
	}
}


// ***************************************************************************
void CMeshBaseInstance::setAnimatedLightmap (CAnimatedLightmap *alm)
{
	_AnimatedLightmap.push_back( alm );
	// Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);
}


// ***************************************************************************
uint CMeshBaseInstance::getNumMaterial () const
{
	return Materials.size ();
}


// ***************************************************************************
const CMaterial	*CMeshBaseInstance::getMaterial (uint materialId) const
{
	return &(Materials[materialId]);
}


// ***************************************************************************
CMaterial	*CMeshBaseInstance::getMaterial (uint materialId)
{
	return &(Materials[materialId]);
}


// ***************************************************************************
// ***************************************************************************
// Async texture loading
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBaseInstance::enableAsyncTextureMode(bool enable)
{
	// if same, no-op.
	if(_AsyncTextureMode==enable)
		return;
	_AsyncTextureMode= enable;

	// if comes to async texture mode, must prepare AsyncTexturing
	if(_AsyncTextureMode)
	{
		_AsyncTextureReady= true;

		// For all TextureFiles in material
		for(uint i=0;i<Materials.size();i++)
		{
			for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
			{
				// test if really a CTextureFile
				CTextureFile	*text= dynamic_cast<CTextureFile*>(Materials[i].getTexture(stage));
				if(text)
				{
					// Must setup the AsyncTextures
					AsyncTextures[i].IsTextureFile[stage]= true;
					AsyncTextures[i].TextureNames[stage]= text->getFileName();
					// Now, must copy the textureFile, to Avoid writing in CMeshBase TextureFile descriptor !!!
					CTextureFile *tf = new CTextureFile(*text);
					// setup a dummy texture => Instance won't block rendering because texture not yet ready
					tf->setFileName("blank.tga");
					Materials[i].setTexture(stage, tf);
				}
				else
				{
					AsyncTextures[i].IsTextureFile[stage]= false;
				}
			}
		}

		// For convenience, flag the instance as Dirty.
		setAsyncTextureDirty(true);
	}
	// else, AsyncTextureMode disabled
	else
	{
		// first, must stop and release all textures in the async manager.
		releaseCurrentAsyncTextures();
		nlassert(_AsyncTextureToLoadRefCount==0);
		// clear the array => ensure good work if enableAsyncTextureMode(true) is made later
		contReset(_CurrentAsyncTextures);

		// For all TextureFiles in material, copy User setup from AsyncTextures, to real fileName
		for(uint i=0;i<Materials.size();i++)
		{
			for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
			{
				// if an async texture file
				if(AsyncTextures[i].IsTextureFile[stage])
				{
					// copy the texture name into the texture file.
					CTextureFile	*text= safe_cast<CTextureFile*>(Materials[i].getTexture(stage));
					text->setFileName(AsyncTextures[i].TextureNames[stage]);
					// clear string space
					AsyncTextures[i].TextureNames[stage].clear();
				}
			}
		}
	}
}


// ***************************************************************************
void			CMeshBaseInstance::startAsyncTextureLoading()
{
	if(!getAsyncTextureMode())
		return;

	// If the async texutre manager is not setuped in the scene, abort.
	CAsyncTextureManager	*asyncTextMgr= _OwnerScene->getAsyncTextureManager();
	if(!asyncTextMgr)
		return;

	uint	i;


	/* for all new texture names to load, add them to the manager
		NB: done first before release because of RefCount Management (in case of same texture name).
	*/
	for(i=0;i<AsyncTextures.size();i++)
	{
		for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
		{
			if(AsyncTextures[i].IsTextureFile[stage])
			{
				uint	id;
				id= asyncTextMgr->addTextureRef(AsyncTextures[i].TextureNames[stage], this);
				AsyncTextures[i].TextIds[stage]= id;
			}
		}
	}

	/* For all old textures (0 for the first time...), release them.
	*/
	releaseCurrentAsyncTextures();

	// OK! bkup the setup
	_CurrentAsyncTextures= AsyncTextures;

	// texture async is not ready.
	_AsyncTextureReady= false;
}

// ***************************************************************************
void			CMeshBaseInstance::releaseCurrentAsyncTextures()
{
	// If the async texutre manager is not setuped in the scene, abort.
	CAsyncTextureManager	*asyncTextMgr= _OwnerScene->getAsyncTextureManager();
	if(!asyncTextMgr)
		return;

	// release all texture in the manager
	for(uint i=0;i<_CurrentAsyncTextures.size();i++)
	{
		for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
		{
			if(_CurrentAsyncTextures[i].IsTextureFile[stage])
			{
				asyncTextMgr->releaseTexture(_CurrentAsyncTextures[i].TextIds[stage], this);
			}
		}
	}
}

// ***************************************************************************
bool			CMeshBaseInstance::isAsyncTextureReady()
{
	// if ok, just quit
	if(_AsyncTextureReady)
		return true;

	// test if async loading ended
	if(_AsyncTextureToLoadRefCount==0)
	{
		// must copy all fileNames into the actual Texture Files. Those are the valid ones now!
		for(uint i=0;i<_CurrentAsyncTextures.size();i++)
		{
			for(uint stage=0;stage<IDRV_MAT_MAXTEXTURES;stage++)
			{
				if(_CurrentAsyncTextures[i].IsTextureFile[stage])
				{
					// copy the texture name into the texture file.
					CTextureFile	*text= safe_cast<CTextureFile*>(Materials[i].getTexture(stage));
					// Since the texture is really uploaded in the driver, the true driver Texture Id will
					// be bound to this texture.
					text->setFileName(_CurrentAsyncTextures[i].TextureNames[stage]);
					/* Since driver setup will only occurs when object become visible, it is a good idea to release
					   Old driver info, because it may points to old driver texture data (eg: old shared textureFile).
					   thus doing so release VRAM Texture Memory
					*/
					text->releaseDriverSetup();
				}
			}
		}

		// Ok, we are now ready.
		_AsyncTextureReady= true;
		return true;
	}
	else
		return false;
}



} // NL3D
