/** \file async_texture_manager.cpp
 * <File description>
 *
 * $Id: async_texture_manager.cpp,v 1.1 2002/10/10 12:55:44 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "3d/async_texture_manager.h"
#include "3d/async_file_manager.h"
#include "3d/mesh_base_instance.h"
#include "3d/driver.h"


using	namespace std;
using	namespace NLMISC;

namespace NL3D 
{


// ***************************************************************************
CAsyncTextureManager::CTextureEntry::CTextureEntry()
{
	Loaded= false;
	UpLoaded= false;
	CanHaveLOD= false;
	BaseSize= 0;
}


// ***************************************************************************
CAsyncTextureManager::~CAsyncTextureManager()
{
	// For all remaining textures, delete them.
	for(uint i=0;i<_TextureEntries.size();i++)
	{
		if(_TextureEntries[i])
			deleteTexture(i);
	}

	// there must be no waitting textures, nor map, nor current upload texture
	nlassert(_WaitingTextures.empty() && _TextureEntryMap.empty() && _CurrentUploadTexture==NULL);
}

// ***************************************************************************
CAsyncTextureManager::CAsyncTextureManager()
{
	_BaseLodLevel= 3;
	_MaxLodLevel= 1;
	_MaxUploadPerFrame= 65536;
	_CurrentUploadTexture= NULL;
}


// ***************************************************************************
void			CAsyncTextureManager::setupLod(uint baseLevel, uint maxLevel)
{
	nlassert(baseLevel>=maxLevel);
	_BaseLodLevel= baseLevel;
	_MaxLodLevel= maxLevel;
}


// ***************************************************************************
void			CAsyncTextureManager::setupMaxUploadPerFrame(uint maxup)
{
	if(maxup>0)
		_MaxUploadPerFrame= maxup;
}


// ***************************************************************************
uint			CAsyncTextureManager::addTextureRef(const string &textName, CMeshBaseInstance *instance)
{
	uint	ret;

	ItTextureEntryMap	it;
	it= _TextureEntryMap.find(textName);

	// not found, create.
	if(it==_TextureEntryMap.end())
	{
		// search a free id.
		uint	i;
		for(i=0;i<_TextureEntries.size();i++)
		{
			if(_TextureEntries[i]==NULL)
				break;
		}
		// resize if needed.
		if(i>=_TextureEntries.size())
			_TextureEntries.push_back(NULL);

		// alloc new.
		CTextureEntry	*text= new CTextureEntry();
		_TextureEntries[i]= text;
		text->Texture= new CTextureFile;

		// add to map.
		it= _TextureEntryMap.insert(make_pair(textName, i)).first;
		// bkup the it for deletion
		text->ItMap= it;

		// Start Async loading.
		text->Texture->setFileName(textName);
		// start to load a small DDS version if possible
		text->Texture->setMipMapSkipAtLoad(_BaseLodLevel);
		// load it async.
		CAsyncFileManager::getInstance().loadTexture(text->Texture, &text->Loaded);
		// Add to a list so we can check each frame if it has ended.
		_WaitingTextures.push_back(i);
	}

	// get the id of the text entry.
	ret= it->second;

	// add this instance to the list of ones which use this texture.
	CTextureEntry	*text= _TextureEntries[ret];
	text->Instances.push_back(instance);

	// if the texture is not yet ready, must increment the instance refCount.
	if(!text->UpLoaded)
		instance->_AsyncTextureToLoadRefCount++;

	return ret;
}


// ***************************************************************************
void			CAsyncTextureManager::deleteTexture(uint id)
{
	CTextureEntry	*text= _TextureEntries[id];

	// stop async loading if not ended
	if(!text->Loaded)
	{
		CAsyncFileManager::getInstance().cancelLoadTexture(text->Texture);
	}

	// remove map entry
	_TextureEntryMap.erase(text->ItMap);

	// remove in list of waiting textures
	vector<uint>::iterator	itWait= find(_WaitingTextures.begin(),_WaitingTextures.end(), id);
	if(itWait!=_WaitingTextures.end())
		_WaitingTextures.erase(itWait);

	// If it was the currently uploaded one, abort
	if(_CurrentUploadTexture==text)
	{
		_CurrentUploadTexture= NULL;
	}

	// If not uploaded.
	if(!text->UpLoaded)
	{
		// For all its remainding instances, dec refcount
		for(uint i=0;i<text->Instances.size();i++)
		{
			text->Instances[i]->_AsyncTextureToLoadRefCount--;
		}
	}

	// delete texture entry.
	delete text;
	_TextureEntries[id]= NULL;
}


// ***************************************************************************
void			CAsyncTextureManager::releaseTexture(uint id, CMeshBaseInstance *instance)
{
	nlassert(id<_TextureEntries.size());
	nlassert(_TextureEntries[id]);

	// find an instance in this texture an remove it.
	CTextureEntry	*text= _TextureEntries[id];
	uint			instSize= text->Instances.size();
	for(uint i=0;i<instSize;i++)
	{
		if(text->Instances[i]== instance)
		{
			// Must first release the refCount if the texture is not uploaded
			if(!text->UpLoaded)
				text->Instances[i]->_AsyncTextureToLoadRefCount--;
			// remove it by swapping with last texture
			text->Instances[i]= text->Instances[instSize-1];
			text->Instances.pop_back();
			// must stop: remove only the first occurence of instance.
			break;
		}
	}

	// if no more instance occurence, the texture is no more used => release it.
	if(text->Instances.empty())
	{
		// do all the good stuff
		deleteTexture(id);
	}
}

// ***************************************************************************
bool			CAsyncTextureManager::isTextureUpLoaded(uint id) const
{
	nlassert(id<_TextureEntries.size());
	nlassert(_TextureEntries[id]);
	return _TextureEntries[id]->UpLoaded;
}

// ***************************************************************************
void			CAsyncTextureManager::update(IDriver *pDriver)
{
	uint	nTotalUploaded = 0;

	// if no texture to upload, get the next one
	if(_CurrentUploadTexture==NULL)
		getNextTextureToUpLoad();

	// while some texture to upload
	while(_CurrentUploadTexture)
	{
		ITexture	*pText= _CurrentUploadTexture->Texture;
		if(uploadTexturePart(pText, pDriver, nTotalUploaded))
		{
			// If we are here, the texture is finally entirely uploaded. Compile it!
			_CurrentUploadTexture->UpLoaded= true;
			// Can Have lod if texture is DXTC and have mipMaps!
			_CurrentUploadTexture->CanHaveLOD= validDXTCMipMap(pText);
			// compute the size it takes in VRAM (with estimation of all mipmaps)
			_CurrentUploadTexture->BaseSize= pText->getSize(0)*CBitmap::bitPerPixels[pText->getPixelFormat()]/8;
			_CurrentUploadTexture->BaseSize= (uint)(_CurrentUploadTexture->BaseSize*1.33f);
			// UpLoaded !! => signal all instances.
			for(uint i=0;i<_CurrentUploadTexture->Instances.size();i++)
			{
				_CurrentUploadTexture->Instances[i]->_AsyncTextureToLoadRefCount--;
			}

			// finally uploaded in VRAM, can release the RAM texture memory
			pText->release();

			// if not break because can't upload all parts, get next texture to upload
			_CurrentUploadTexture= NULL;
			getNextTextureToUpLoad();
		}
		else
			// Fail to upload all, abort.
			return;
	}
}


// ***************************************************************************
bool			CAsyncTextureManager::uploadTexturePart(ITexture *pText, IDriver *pDriver, uint &nTotalUploaded)
{
	uint		nMipMap;
	nMipMap = pText->getMipMapCount();


	// If this is the start of uploading, setup the texture in driver.
	if(_CurrentUploadTextureMipMap==0 && _CurrentUploadTextureLine==0)
	{
		// If the texture is not a valid DXTC with mipmap
		if(!validDXTCMipMap(pText))
		{
			/* For now, prefer do nothing, because this may be an error (texture not found)
				and the texture may not be used at all, so don't take VRAM for nothing.
				=> if the texture is used, it will be loaded synchronously by the caller later in the process
				=> frame freeze.
			*/
			/*
			// upload All now.
			// MipMap generation and compression may be done here => Maybe Big Freeze.
			// approximate*2 instead of *1.33 for mipmaps.
			uint	nWeight = pText->getSize (0) * 2;
			nWeight= (nWeight*CBitmap::bitPerPixels[pText->getPixelFormat()])/8;
			nTotalUploaded+= nWeight;
			pDriver->setupTexture(*pText);
			return true;*/
			return true;
		}
		else
		{
			// Create the texture only and do not upload anything
			bool isRel = pText->getReleasable ();
			pText->setReleasable (false);
			bool isAllUploaded = false;
			pDriver->setupTextureEx (*pText, false, isAllUploaded);
			pText->setReleasable (isRel);
			// if the texture is already uploaded, abort partial uploading.
			if (isAllUploaded)
				return true;
		}
	}


	// try to upload all mipmap
	for(; _CurrentUploadTextureMipMap<nMipMap; _CurrentUploadTextureMipMap++)
	{
		CRect zeRect;
		uint nMM= _CurrentUploadTextureMipMap;

		// What is left to upload ?
		uint	nWeight = pText->getSize (nMM) - _CurrentUploadTextureLine*pText->getWidth(nMM);
		nWeight= (nWeight*CBitmap::bitPerPixels[pText->getPixelFormat()])/8;

		if ((nTotalUploaded  + nWeight) > _MaxUploadPerFrame)
		{
			// We cannot upload the whole mipmap -> we have to cut it
			uint nSizeToUpload = _MaxUploadPerFrame - nTotalUploaded ;
			uint nLineWeight = (pText->getWidth(nMM)*CBitmap::bitPerPixels[pText->getPixelFormat()])/8;
			uint nNbLineToUpload = nSizeToUpload / nLineWeight;
			// Upload 4 line by 4 line, and upload at leat one 4*line.
			nNbLineToUpload = nNbLineToUpload / 4;
			nNbLineToUpload= max(nNbLineToUpload, 1U);
			nNbLineToUpload *= 4;
			// comput rect to upload
			uint32 nNewLine = _CurrentUploadTextureLine + nNbLineToUpload;
			nNewLine= min(nNewLine, pText->getHeight(nMM));
			zeRect.set (0, _CurrentUploadTextureLine, pText->getWidth(nMM), nNewLine);
			_CurrentUploadTextureLine = nNewLine;
			// if fill all the mipmap, must go to next
			if (_CurrentUploadTextureLine == pText->getHeight(nMM))
			{
				_CurrentUploadTextureLine = 0;
				_CurrentUploadTextureMipMap++;
			}
		}
		else
		{
			// We can upload the whole mipmap (or the whole rest of the mipmap)
			zeRect.set (0, _CurrentUploadTextureLine, pText->getWidth(nMM), pText->getHeight(nMM));
			_CurrentUploadTextureLine= 0;
		}

		// upload the texture 
		pDriver->uploadTexture (*pText, zeRect, (uint8)nMM);

		nTotalUploaded += nWeight;
		// If outpass max allocated upload, abort.
		if (nTotalUploaded > _MaxUploadPerFrame)
			return false;
	}

	return true;
}


// ***************************************************************************
void			CAsyncTextureManager::getNextTextureToUpLoad()
{
	// Reset texture uploading
	_CurrentUploadTexture= NULL;
	_CurrentUploadTextureMipMap= 0;
	_CurrentUploadTextureLine= 0;

	// Search in WaitingTextures if one has ended async loading
	vector<uint>::iterator	it;
	for(it=_WaitingTextures.begin();it!=_WaitingTextures.end();it++)
	{
		CTextureEntry	*text= _TextureEntries[*it];
		// If Async loading done.
		if(text->Loaded)
		{
			// upload this one
			_CurrentUploadTexture= text;
			// remove it from list of waiting textures
			_WaitingTextures.erase(it);
			// found => end.
			return;
		}
	}
}


// ***************************************************************************
bool			CAsyncTextureManager::validDXTCMipMap(ITexture *pText)
{
	return pText->getMipMapCount()>1 && (
		pText->getPixelFormat() == CBitmap::DXTC1 ||
		pText->getPixelFormat() == CBitmap::DXTC1Alpha ||
		pText->getPixelFormat() == CBitmap::DXTC3 ||
		pText->getPixelFormat() == CBitmap::DXTC5 );
}


} // NL3D
