/** \file shape_bank.cpp
 * <File description>
 *
 * $Id: shape_bank.cpp,v 1.18 2002/12/06 12:41:26 corvazier Exp $
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

#include "3d/shape_bank.h"
#include "3d/mesh.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/rect.h"
#include "nel/misc/algo.h"
#include "nel/misc/progress_callback.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

CShapeBank::CShapeBank()
{
	// Default cache creation
	addShapeCache( "default" );
	_MaxUploadPerFrame = 16*1024;
}

// ***************************************************************************

CShapeBank::~CShapeBank()
{
}

// ***************************************************************************

IShape*CShapeBank::addRef(const string &shapeNameNotLwr)
{	
	string	shapeName= strlwr(shapeNameNotLwr);

	// If the shape is inserted in a shape cache remove it
	TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( getShapePtrFromShapeName( shapeName ) );
	if( scfpmIt != ShapePtrToShapeInfo.end() )
	{
		if( !scfpmIt->second.isAdded )
		{
			// The shape is not inserted into a cache
			return getShapePtrFromShapeName( shapeName );
		}
	}
	scfpmIt->second.isAdded = false;
	CShapeCache *pShpCache = scfpmIt->second.pShpCache;
	nlassert( pShpCache != NULL );
	// Search the shape cache for the shape we want to remove
	list<IShape*>::iterator lsIt = pShpCache->Elements.begin();
	while(lsIt != pShpCache->Elements.end())
	{
		string *sTemp = getShapeNameFromShapePtr(*lsIt);
		if( *sTemp == shapeName )
		{
			// Ok the shape cache contains the shape remove it and return
			pShpCache->Elements.erase( lsIt );				
			return getShapePtrFromShapeName( shapeName );
		}
		++lsIt;
	}
	nlassert( false );
	return getShapePtrFromShapeName( shapeName );
}

// ***************************************************************************

void CShapeBank::release(IShape* pShp)
{
	// Do we have the last smartPtr on the shape ?
	string* str = getShapeNameFromShapePtr( pShp );

	if (str == NULL)
	{
		nlwarning ("Trying to release a mesh that have not be added to the shape bank");
	}
	else
	{
		TShapeMap::iterator smIt = ShapeMap.find( *str );
		if( smIt != ShapeMap.end() )
		{
			if( smIt->second.getNbRef() == 1 )
			{
				// Yes -> add the shape to its shapeCache
				CShapeCache *pShpCache = getShapeCachePtrFromShapePtr( pShp );
				pShpCache->Elements.push_front( pShp );

				TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( pShp );
				if( scfpmIt != ShapePtrToShapeInfo.end() )
				{
					scfpmIt->second.isAdded = true;
				}
				
				// check the shape cache
				checkShapeCache(getShapeCachePtrFromShapePtr(pShp));
			}
		}
		else
		{
			nlassert( false );
		}
	}
}

// ***************************************************************************

void CShapeBank::processWaitingShapes ()
{
	uint32 nTotalUploaded = 0;
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.begin();
	while( wsmmIt != WaitingShapes.end() )
	{
		// Backup next iterator
		TWaitingShapesMap::iterator wsmmItNext = wsmmIt;
		wsmmItNext++;

		const string &shapeName = wsmmIt->first;
		CWaitingShape &rWS = wsmmIt->second;
		IShape *pShp = rWS.ShapePtr; // Take care this value is shared between thread so copy it in a local variable first

		switch (rWS.State)
		{
			case AsyncLoad_Shape: // Check if we can pass to the AsyncLoad_Texture state
				if (pShp != NULL)
				{
					if (pShp == (IShape*)-1)
						rWS.State = AsyncLoad_Error;
					else
						rWS.State = AsyncLoad_Texture;
				}
			break;

			case AsyncLoad_Texture:
			{
				// Setup all textures and lightmaps of the shape
				if (nTotalUploaded > _MaxUploadPerFrame)
					break;

				CMesh *pMesh = dynamic_cast<CMesh*>(pShp);
				if( pMesh != NULL )
				{
					uint8 j;
					uint32 i, CurrentProgress = 0;
					uint32 nNbMat = pMesh->getNbMaterial();

					for (i = 0; i < nNbMat; ++i)
					{
						const CMaterial &rMat = pMesh->getMaterial(i);
						// Parse all textures from this material and setup
						for (j = 0; j < IDRV_MAT_MAXTEXTURES; ++j)
						{
							if (CurrentProgress >= rWS.UpTextProgress)
							{
								if (rMat.texturePresent(j))
								{
									if ((!_pDriver->isTextureExist(*rMat.getTexture(j))) || 
										(rWS.UpTextLine > 0) || (rWS.UpTextMipMap > 0))
									{
										//_pDriver->setupTexture (*rMat.getTexture(j));

										if (!processWSUploadTexture (rWS, nTotalUploaded, rMat.getTexture(j)))
											break;
									}
								}
								++rWS.UpTextProgress;
							}
							++CurrentProgress;
							if (nTotalUploaded > _MaxUploadPerFrame)
								break;
						}

						if (nTotalUploaded > _MaxUploadPerFrame)
							break;

						// Do the same with lightmaps
						if (rMat.getShader() == CMaterial::LightMap)
						{
							uint j = 0; ITexture *pText = rMat.getLightMap (j);
							while (pText != NULL)
							{
								if (CurrentProgress >= rWS.UpTextProgress)
								{
									if ((!_pDriver->isTextureExist(*pText)) || 
										(rWS.UpTextLine > 0) || (rWS.UpTextMipMap > 0))
									{
										//_pDriver->setupTexture (*pText);
										
										if (!processWSUploadTexture (rWS, nTotalUploaded, pText))
											break;
									}
									++rWS.UpTextProgress;
								}
								++CurrentProgress;
								++j; pText = rMat.getLightMap (j);
								if (nTotalUploaded > _MaxUploadPerFrame)
									break;
							}
						}
						if (nTotalUploaded > _MaxUploadPerFrame)
							break;
					}
				}
				if (nTotalUploaded > _MaxUploadPerFrame)
					break;

				rWS.State = AsyncLoad_Ready;
			}				
			break;

			case AsyncLoad_Ready:
				add (wsmmIt->first, pShp);
				rWS.State = AsyncLoad_Delete;
			break;

			// The delete operation can take several frames to complete but this is not a problem

			// For error do the same as delete but let the flag to error if a shape is asked just after 
			// the error was found

			case AsyncLoad_Error:
			case AsyncLoad_Delete:
				rWS.RefCnt -= 1;
				if (rWS.RefCnt == 0)
				{
					// We have to signal if we are the last
					bool *bSignal = rWS.Signal;
					if (bSignal != NULL)
					{
						bool bFound = false;
						TWaitingShapesMap::iterator wsmmIt2 = WaitingShapes.begin();
						while (wsmmIt2 != WaitingShapes.end())
						{
							const string &shapeName2 = wsmmIt2->first;
							if ((wsmmIt2->second.Signal == bSignal) && (shapeName2 != shapeName))
							{
								bFound = true;
								break;
							}
							++wsmmIt2;
						}
						if (!bFound)
							*bSignal = true;
					}
					WaitingShapes.erase (wsmmIt);
				}
			break;

			default:
				nlstop; // This must never happen
			break;
		}

		wsmmIt = wsmmItNext;
	}
}

// ***************************************************************************
void CShapeBank::setMaxBytesToUpload (uint32 MaxUploadPerFrame)
{
	_MaxUploadPerFrame = MaxUploadPerFrame;
}

// ***************************************************************************
bool CShapeBank::processWSUploadTexture (CWaitingShape &rWS, uint32 &nTotalUploaded, ITexture *pText)
{
	CRect zeRect;
	uint32 nFace, nWeight = 0, nMipMap;
	
	if (pText->mipMapOn())
		nMipMap = pText->getMipMapCount();
	else
		nMipMap = 1;

	uint32 nMM;
	for (nMM = 0; nMM < nMipMap; ++nMM)
		nWeight += pText->getSize (nMM) * CBitmap::bitPerPixels[pText->getPixelFormat()]/8;
	if (pText->isTextureCube())
		nWeight *= 6;

	
	if ((rWS.UpTextMipMap == 0) && (rWS.UpTextLine == 0))
	{
		// Create the texture only and do not upload anything
		bool isRel = pText->getReleasable ();
		pText->setReleasable (false);
		bool isAllUploaded = false;
		_pDriver->setupTextureEx (*pText, false, isAllUploaded);
		pText->setReleasable (isRel);
		if (isAllUploaded)
			return true;
	}
	
	// Upload all mipmaps
	for (; rWS.UpTextMipMap < nMipMap; ++rWS.UpTextMipMap)
	{
		nMM = rWS.UpTextMipMap;
		// What is left to upload ?
		nWeight = pText->getSize (nMM) - rWS.UpTextLine*pText->getWidth(nMM);
		nWeight *= CBitmap::bitPerPixels[pText->getPixelFormat()]/8;
		if (pText->isTextureCube())
			nWeight *= 6;

		// Setup rectangle
		if ((nTotalUploaded + nWeight) > _MaxUploadPerFrame)
		{
			// We cannot upload the whole mipmap -> we have to cut it
			uint32 nSizeToUpload = _MaxUploadPerFrame - nTotalUploaded;
			uint32 nLineWeight = pText->getWidth(nMM)*CBitmap::bitPerPixels[pText->getPixelFormat()]/8;
			if (pText->isTextureCube())
				nLineWeight *= 6;
			uint32 nNbLineToUpload = nSizeToUpload / nLineWeight;
			nNbLineToUpload = nNbLineToUpload / 4;
			if (nNbLineToUpload == 0)
				nNbLineToUpload = 1;
			nNbLineToUpload *= 4; // Upload 4 line by 4 line
			uint32 nNewLine = rWS.UpTextLine + nNbLineToUpload;
			if (nNewLine > pText->getHeight(nMM))
				nNewLine = pText->getHeight(nMM);
			zeRect.set (0, rWS.UpTextLine, pText->getWidth(nMM), nNewLine);
			rWS.UpTextLine = nNewLine;
			if (rWS.UpTextLine == pText->getHeight(nMM))
			{
				rWS.UpTextLine = 0;
				rWS.UpTextMipMap += 1;
			}
		}
		else
		{
			// We can upload the whole mipmap (or the whole rest of the mipmap)
			zeRect.set (0, rWS.UpTextLine, pText->getWidth(nMM), pText->getHeight(nMM));
			rWS.UpTextLine = 0;
		}

		// Upload !
		if (pText->isTextureCube())
		{
			for (nFace = 0; nFace < 6; ++nFace)
				_pDriver->uploadTextureCube (*pText, zeRect, (uint8)nMM, (uint8)nFace);
		}
		else
		{
			_pDriver->uploadTexture (*pText, zeRect, (uint8)nMM);
		}

		nTotalUploaded += nWeight;
		if (nTotalUploaded > _MaxUploadPerFrame)
			return false;
	}
	rWS.UpTextMipMap = 0;
	rWS.UpTextLine = 0;
	return true;
}

// ***************************************************************************

CShapeBank::TShapeState CShapeBank::isPresent (const string &shapeNameNotLwr)
{
	string	shapeName= strlwr(shapeNameNotLwr);

	// Is the shape is found in the shape map so return Present
	TShapeMap::iterator smIt = ShapeMap.find (shapeName);
	if( smIt != ShapeMap.end() )
		return Present;
	// Look in the waiting shapes
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find (shapeName);
	if (wsmmIt != WaitingShapes.end())
		return wsmmIt->second.State; // AsyncLoad_*
	return NotPresent;
}

// ***************************************************************************

void CShapeBank::load (const string &shapeNameNotLwr)
{
	string	shapeName= strlwr(shapeNameNotLwr);

	TShapeMap::iterator smIt = ShapeMap.find(shapeName);
	if( smIt == ShapeMap.end() )
	{
		// If we are loading it asynchronously so we do not have to try to load it in sync mode
		TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find (shapeName);
		if (wsmmIt != WaitingShapes.end())
			return;
		try
		{
			CShapeStream mesh;
			CIFile meshfile(CPath::lookup(shapeName));
			meshfile.serial( mesh );
			meshfile.close();

			// Add the shape to the map.
			add( shapeName, mesh.getShapePointer() );
		}
		catch(EPathNotFound &)
		{
			return;
		}
	}	
}

// ***************************************************************************

void CShapeBank::loadAsync (const std::string &shapeNameNotLwr, IDriver *pDriver, bool *bSignal)
{
	string	shapeName= strlwr(shapeNameNotLwr);

	TShapeMap::iterator smIt = ShapeMap.find(shapeName);
	if (smIt != ShapeMap.end())
		return;
	_pDriver = pDriver; // Backup the pointer to the driver for later use
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find (shapeName);
	if (wsmmIt != WaitingShapes.end())
	{
		// Add a reference to it
		CWaitingShape &rWS = wsmmIt->second;
		rWS.RefCnt += 1;
		return; // Do not load 2 shapes with the same names
	}
	wsmmIt = WaitingShapes.insert (TWaitingShapesMap::value_type(shapeName, CWaitingShape(bSignal))).first;
	CAsyncFileManager3D::getInstance().loadMesh (shapeName, &(wsmmIt->second.ShapePtr), pDriver);
}

// ***************************************************************************

void CShapeBank::cancelLoadAsync (const std::string &shapeNameNotLwr)
{
	string	shapeName= strlwr(shapeNameNotLwr);

	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find(shapeName);
	if (wsmmIt != WaitingShapes.end())
	{
		wsmmIt->second.RefCnt -= 1;
		if (wsmmIt->second.RefCnt == 0)
		{
			// nlinfo("unloadasync %s", shapeName);
			CAsyncFileManager3D::getInstance().cancelLoadMesh (shapeName);
			// TODO : Cancel the texture upload
			WaitingShapes.erase (wsmmIt); // Delete the waiting shape
		}
	}
}

// ***************************************************************************

bool CShapeBank::isShapeWaiting ()
{
	if (WaitingShapes.size() == 0)
		return false;
	else
		return true;
}

// ***************************************************************************

void CShapeBank::add (const string &shapeNameNotLwr, IShape* pShp)
{
	string	shapeName= strlwr(shapeNameNotLwr);

	// Is the shape name already used ?
	TShapeMap::iterator smIt = ShapeMap.find( shapeName );
	if( smIt == ShapeMap.end() )
	{
		// No ok so lets add the smart pointer
		CSmartPtr<IShape> spShape = pShp;
		ShapeMap[shapeName] = spShape;

		// create the shape info
		CShapeInfo siTemp;
		siTemp.sShpName = shapeName;
		siTemp.pShpCache = getShapeCachePtrFromShapeName( shapeName );
		// Is the shape has a valid shape cache ? 
		if( siTemp.pShpCache == NULL )
		{
			// No -> link to default (which do the UpdateShapeInfo)
			siTemp.pShpCache = getShapeCachePtrFromShapeCacheName( "default" );
			// Add the shape to the default shape cache
			ShapePtrToShapeInfo[pShp]= siTemp;
			ShapeNameToShapeCacheName[shapeName]= "default";
		}
		else
		{
			// Yes -> add or replace the shape info
			ShapePtrToShapeInfo[pShp] = siTemp;
		}
	}
}

// ***************************************************************************

void CShapeBank::addShapeCache(const string &shapeCacheName)
{
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt == ShapeCacheNameToShapeCache.end() )
	{
		// Not found so add it		
		ShapeCacheNameToShapeCache.insert(TShapeCacheMap::value_type(shapeCacheName,CShapeCache()));
	}
}

// ***************************************************************************

void CShapeBank::removeShapeCache(const std::string &shapeCacheName)
{
	if( shapeCacheName == "default" )
		return;

	// Free the shape cache
	CShapeCache *pShpCache = getShapeCachePtrFromShapeCacheName( shapeCacheName );
	if( pShpCache == NULL )
		return;
	pShpCache->MaxSize = 0;
	checkShapeCache( pShpCache );

	// Remove it
	ShapeCacheNameToShapeCache.erase( shapeCacheName );

	// All links are redirected to the default cache
	TShapeCacheNameMap::iterator scnIt = ShapeNameToShapeCacheName.begin();
	while( scnIt != ShapeNameToShapeCacheName.end() )	
	{
		if( scnIt->second == shapeCacheName )
			scnIt->second = "default";
		++scnIt;
	}
}

// ***************************************************************************

void CShapeBank::reset()
{
	// Parse la map ShapeCacheNameToShapeCache pour supprimer tout les caches
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.begin();
	while( scmIt != ShapeCacheNameToShapeCache.end() )
	{
		CShapeCache *pShpCache = getShapeCachePtrFromShapeCacheName( scmIt->first );
		if( pShpCache == NULL )
			nlstop; // Should never happen
		pShpCache->MaxSize = 0;
		checkShapeCache( pShpCache );

		++scmIt;
	}
	ShapeNameToShapeCacheName.clear();
	ShapeCacheNameToShapeCache.clear();	
	addShapeCache( "default" );
}

// ***************************************************************************

void CShapeBank::setShapeCacheSize(const string &shapeCacheName, sint32 maxSize)
{
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt != ShapeCacheNameToShapeCache.end() )
	{
		scmIt->second.MaxSize = maxSize;
		checkShapeCache(getShapeCachePtrFromShapeCacheName(shapeCacheName));
	}
}

// ***************************************************************************
sint CShapeBank::getShapeCacheFreeSpace(const std::string &shapeCacheName) const
{
	TShapeCacheMap::const_iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt != ShapeCacheNameToShapeCache.end() )
	{
		return scmIt->second.MaxSize - scmIt->second.Elements.size();
	}
	return 0;
}

// ***************************************************************************

void CShapeBank::linkShapeToShapeCache(const string &shapeNameNotLwr, const string &shapeCacheName)
{
	string	shapeName= strlwr(shapeNameNotLwr);

	bool	canSet= true;
	while(1)
	{
		// Shape exist?
		IShape	*shapePtr= getShapePtrFromShapeName(shapeName);
		if(shapePtr == NULL)
			// No, but still link the shape name to the shapeCache name.
			break;
		// Is the shape cache exist ?
		CShapeCache *shapeCachePtr = getShapeCachePtrFromShapeCacheName( shapeCacheName );
		if( shapeCachePtr == NULL )
			// abort, since cannot correctly link to a valid shapeCache
			return;

		// Try to set to the same shape Cache as before?
		CShapeInfo	&shapeInfo= ShapePtrToShapeInfo[shapePtr];
		if( shapeCachePtr ==  shapeInfo.pShpCache)
			// abort, since same cache name / cache ptr
			return;

		// If The shape is In the cache of an other Shape Cache, abort.
		if( shapeInfo.isAdded )
			// Abort, because impossible.
			return;
		
		// Is the shape is present ?
		// Yes -> Update the ShapeInfo
		shapeInfo.pShpCache= shapeCachePtr;

		break;
	}

	// change the cache name of the shape
	ShapeNameToShapeCacheName[shapeName] = shapeCacheName;
}

// ***************************************************************************

CShapeBank::CShapeCache* CShapeBank::getShapeCachePtrFromShapePtr(IShape* pShp)
{
	TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( pShp );
	if( scfpmIt != ShapePtrToShapeInfo.end() )
	{
		return scfpmIt->second.pShpCache;
	}
	return NULL;
}

// ***************************************************************************

IShape* CShapeBank::getShapePtrFromShapeName(const std::string &pShpName)
{
	TShapeMap::iterator smIt = ShapeMap.find(pShpName);
	if( smIt != ShapeMap.end() )
	{
		return (IShape*)(smIt->second);
	}
	return NULL;
}

// ***************************************************************************

CShapeBank::CShapeCache* CShapeBank::getShapeCachePtrFromShapeCacheName(const string &shapeCacheName)
{
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt != ShapeCacheNameToShapeCache.end())
	{
		return &(scmIt->second);
	}
	return NULL;
}

// ***************************************************************************

string* CShapeBank::getShapeNameFromShapePtr(IShape* pShp)
{
	TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( pShp );
	if( scfpmIt != ShapePtrToShapeInfo.end() )
	{
		return &(scfpmIt->second.sShpName);
	}
	return NULL;
}

// ***************************************************************************

CShapeBank::CShapeCache* CShapeBank::getShapeCachePtrFromShapeName(const std::string &shapeName)
{
	TShapeCacheNameMap::iterator scnIt = ShapeNameToShapeCacheName.find( shapeName );
	if( scnIt != ShapeNameToShapeCacheName.end() )
	{
		return getShapeCachePtrFromShapeCacheName(scnIt->second);
	}
	return NULL;
}

// ***************************************************************************

void CShapeBank::checkShapeCache(CShapeCache* pShpCache)
{
	if( pShpCache != NULL )
	while( (sint)pShpCache->Elements.size() > pShpCache->MaxSize )
	{
		// Suppress the last shape of the cache
		IShape *pShp = pShpCache->Elements.back();
		// Physical suppression because we own the last smart pointer on the shape
		ShapeMap.erase(*getShapeNameFromShapePtr(pShp));
		// delete information associated with the shape
		ShapePtrToShapeInfo.erase( pShp );
		// remove from queue
		pShpCache->Elements.pop_back();
	}
}


// ***************************************************************************
bool CShapeBank::isShapeCache(const std::string &shapeCacheName) const
{
	return ShapeCacheNameToShapeCache.find(shapeCacheName) != ShapeCacheNameToShapeCache.end();
}

// ***************************************************************************
void CShapeBank::preLoadShapes(const std::string &shapeCacheName, 
	const std::vector<std::string> &listFile, const std::string &wildCardNotLwr, NLMISC::IProgressCallback *progress)
{
	// Abort if cache don't exist.
	if(!isShapeCache(shapeCacheName))
		return;

	// strlwr
	string wildCard= strlwr(wildCardNotLwr);

	// For all files
	for(uint i=0;i<listFile.size();i++)
	{
		// Progress bar
		if (progress)
			progress->progress ((float)i/(float)listFile.size ());

		string	fileName= CFile::getFilename(listFile[i]);
		strlwr(fileName);
		// if the file is ok for the wildCard, process it
		if( testWildCard(fileName.c_str(), wildCard.c_str()) )
		{
			// link the shape to the shapeCache
			linkShapeToShapeCache(fileName, shapeCacheName);

			// If !present in the shapeBank
			if( isPresent(fileName)==CShapeBank::NotPresent )
			{
				// Don't load it if no more space in the cache
				if( getShapeCacheFreeSpace(shapeCacheName)>0 )
				{
					// load it.
					load(fileName);

					// If success
					if( isPresent(fileName)!=CShapeBank::NotPresent )
					{
						// When a shape is first added to the bank, it is not in the cache. 
						// add it and release it to force it to be in the cache.
						IShape	*shp= addRef(fileName);
						if(shp)
							release(shp);
					}
				}
			}
		}
	}

}


}