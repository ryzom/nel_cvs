/** \file shape_bank.cpp
 * <File description>
 *
 * $Id: shape_bank.cpp,v 1.10 2002/04/17 12:09:22 besson Exp $
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

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

CShapeBank::CShapeBank()
{
	// Default cache creation
	addShapeCache( "default" );
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

bool CShapeBank::isPresent(const string &shapeName)
{
	// Process the waiting shapes
	TWaitingShapesMMap::iterator wsmmIt = WaitingShapes.begin();
	while( wsmmIt != WaitingShapes.end() )
	{
		IShape *pShp = wsmmIt->second;

		if( pShp != NULL )
		{
			add(wsmmIt->first, pShp);

			// Setup all textures of the shape
			CMesh *pMesh = dynamic_cast<CMesh*>(pShp);
			if( pMesh != NULL )
			{
				uint i,j;
				uint nNbMat = pMesh->getNbMaterial();

				for(i = 0; i < nNbMat; ++i)
				{
					const CMaterial &rMat = pMesh->getMaterial(i);
					// Parse all textures from this material and setup
					for(j = 0; j < IDRV_MAT_MAXTEXTURES; ++j)
					if( rMat.texturePresent(j) )
					{
						//--const_cast<CMaterial&>(rMat).setTexture(j,NULL);
						//---rMat.getTexture(j)->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
						_pDriver->setupTexture(*rMat.getTexture(j));
					}
				}
			}
			
			// Delete the waiting shape
			TWaitingShapesMMap::iterator delIt = wsmmIt;
			++wsmmIt;
			WaitingShapes.erase(delIt);
		}
		else
		{
			++wsmmIt;
		}
	}
	// Is the shape is found so return true
	TShapeMap::iterator smIt = ShapeMap.find( shapeName );
	if( smIt == ShapeMap.end() )
		return false;
	else
		return true;
}

// ***************************************************************************

void CShapeBank::load(const string &shapeName)
{
	TShapeMap::iterator smIt = ShapeMap.find(shapeName);
	if( smIt == ShapeMap.end() )
	{
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

void CShapeBank::loadAsync(const std::string &shapeName,IDriver *pDriver)
{
	_pDriver = pDriver; // Backup the pointer to the driver for later use
	TWaitingShapesMMap::iterator wsmmIt;
	wsmmIt = WaitingShapes.insert(TWaitingShapesMMap::value_type(shapeName,NULL));
	CAsyncFileManager::getInstance().loadMesh(shapeName, &(wsmmIt->second), pDriver);
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

void CShapeBank::add(const string &shapeName, IShape* pShp)
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