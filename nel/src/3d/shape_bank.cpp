/** \file shape_bank.cpp
 * <File description>
 *
 * $Id: shape_bank.cpp,v 1.14 2002/05/13 07:49:26 besson Exp $
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

IShape*CShapeBank::addRef(const string &shapeName)
{	
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
					TWaitingShapesMap::iterator delIt = wsmmIt;
					++wsmmIt;
					WaitingShapes.erase (delIt);
				}
				else
				{
					++wsmmIt;
				}
			break;

			default:
				nlstop; // This must never happen
			break;
		}

		// The increment is done in the AsyncLoad_Delete and error processes
		if ((rWS.State != AsyncLoad_Delete) && (rWS.State != AsyncLoad_Error))
		{
			++wsmmIt;
		}
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

CShapeBank::TShapeState CShapeBank::isPresent (const string &shapeName)
{
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

void CShapeBank::load (const string &shapeName)
{
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

void CShapeBank::loadAsync (const std::string &shapeName, IDriver *pDriver, bool *bSignal)
{
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
	CAsyncFileManager::getInstance().loadMesh (shapeName, &(wsmmIt->second.ShapePtr), pDriver);
}

// ***************************************************************************

void CShapeBank::cancelLoadAsync (const std::string &shapeName)
{
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find(shapeName);
	if (wsmmIt != WaitingShapes.end())
	{
		wsmmIt->second.RefCnt -= 1;
		if (wsmmIt->second.RefCnt == 0)
		{
			// nlinfo("unloadasync %s", shapeName);
			CAsyncFileManager::getInstance().cancelLoadMesh (shapeName);
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

void CShapeBank::add (const string &shapeName, IShape* pShp)
{
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
		// Is the shape has a shape cache ? 
		if( siTemp.pShpCache == NULL )
		{
			// No -> link to default (which do the UpdateShapeInfo)
			siTemp.pShpCache = getShapeCachePtrFromShapeName( "default" );
			ShapePtrToShapeInfo.insert(TShapeInfoMap::value_type(pShp,siTemp));
			// Add the shape to the default shape cache
			linkShapeToShapeCache( shapeName, "default" );
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

void CShapeBank::linkShapeToShapeCache(const string &shapeName, const string &shapeCacheName)
{
	// Is the shape cache exist ?
	CShapeCache *pShpCache = getShapeCachePtrFromShapeCacheName( shapeCacheName );
	if( pShpCache != NULL )
	{
		// Check if the shape cache contains the shape
		list<IShape*>::iterator lsIt = pShpCache->Elements.begin();
		while(lsIt != pShpCache->Elements.end())
		{
			string *sTemp = getShapeNameFromShapePtr(*lsIt);
			if( *sTemp == shapeName )
			{
				// Error the shape cache contains a pointer to the shape
				// We cannot link the shape to another shape cache
				return;
			}			
			++lsIt;
		}
		
	}
	else
	{
		// The shape cache does not exist
		return;
	}
	
	// Is the shape is present ?
	TShapeMap::iterator smIt = ShapeMap.find( shapeName );
	if( smIt != ShapeMap.end() )
	{
		// Yes -> UpdateShapeInfo
		updateShapeInfo(getShapePtrFromShapeName(shapeName), getShapeCachePtrFromShapeCacheName(shapeCacheName));
	}

	// If the shape is not linked to a cache add it, else change the cache name of the shape
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

void CShapeBank::updateShapeInfo(IShape* pShp, CShapeCache* pShpCache)
{
	// update the shapeinfo with the pointer to the new shape cache
	if( ( pShp == NULL ) || (pShpCache == NULL) )
		return;
	TShapeInfoMap::iterator simIt = ShapePtrToShapeInfo.find( pShp );
	if( simIt != ShapePtrToShapeInfo.end() )
	{
		simIt->second.pShpCache = pShpCache;
	}
}

}