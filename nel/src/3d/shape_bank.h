/** \file shape_bank.h
 * <File description>
 *
 * $Id: shape_bank.h,v 1.4 2002/05/02 12:41:40 besson Exp $
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

#ifndef NL_SHAPE_BANK_H
#define NL_SHAPE_BANK_H

#include "3d/shape.h"
#include "nel/misc/smart_ptr.h"
#include <map>
#include <list>

#include "3d/async_file_manager.h"

namespace NL3D
{

class IDriver;

// ***************************************************************************
/**
 * A CShapeBank handle all the instance of the shapes and the cache management
 * system.
 * There is a default cache. If the shape is not linked explicitly to any cache
 * it is linked to the default cache. The comportement of this cache is to not
 * do any cache. When the release is called on the last reference to a shape 
 * linked to this cache, the shape is removed instantly. This is the behavior
 * of all newly created cache before we call the setShapeCacheSize method.
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2000
 */
class CShapeBank
{
public:

	/// \name State of a shape
	//@{
	/** NotPresent : Not present in the bank
	  * Present : Present in the bank and ready to be used
	  * AsyncLoad_Error : Asynchronous loading failed
	  * AsyncLoad_Shape : Asynchronous loading is currently loading the .shape file, textures and lightmaps
	  * AsyncLoad_Texture : Asynchronous loading is currently uploading textures and lightmaps to VRAM
	  */
	enum TShapeState {	NotPresent, Present, 
						AsyncLoad_Error, AsyncLoad_Shape, AsyncLoad_Texture, AsyncLoad_Ready, AsyncLoad_Delete };
	//@}

	CShapeBank();
	~CShapeBank();

	/// \name Instance Management
	//@{
	/// Add a reference to a shape and return the instance created.
	IShape*			addRef (const std::string &shapeName);

	/** 
	  * Release a reference to a shape by its instance. If the shape has no more reference it is added to
	  * its own shape cache. When the shape cache is full the last entry is deleted.
	  */
	void			release (IShape* pShp);

	/// Return TRUE if the shape is present in the bank. Process the waiting shapes.
	TShapeState		isPresent (const std::string &shapeName);

	/// Load the corresponding file from disk and add it to the bank.
	void			load (const std::string &shapeName);

	/** Load the corresponding file from disk asynchronously and add it to the bank.
	 * The driver passed to this function is used to know if we have to load the textures.
	 */
	void			loadAsync (const std::string &shapeName, IDriver *pDriver, bool *bSignal=NULL);
	void			cancelLoadAsync (const std::string &shapeName);
	bool			isShapeWaiting ();
	/// processWaitingShapes must be done one time per frame
	void			processWaitingShapes ();

	/// Add directly a shape to the bank. If the shape name is already used do nothing.
	void			add (const std::string &shapeName, IShape* shape);
	//@}

	/// \name Shape cache management
	//@{
	/// Add a new ShapeCache. If already exist do nothing.
	void			addShapeCache (const std::string &shapeCacheName);

	/** 
	  * Remove a ShapeCache. All shapes in the shape cache are deleted. All links are redirected to 
	  * the default ShapeCache
	  */
	void			removeShapeCache (const std::string &shapeCacheName);

	/**
	  * Remove all ShapeCache and suppress all links (even the link to the default cache are removed)
	  */
	void			reset ();
	
	/// Set the shapeCache shapeCacheName the new size.(delete shapes if maxsize<shapeCacheSize).
	void			setShapeCacheSize (const std::string &shapeCacheName, sint32 maxSize);

	/// Link a shape to a ShapeCache. The ShapeCache must exist and must not contains the shape.
	void			linkShapeToShapeCache (const std::string &shapeName, const std::string &shapeCacheName);
	//@}

private:
	/// \name Shape/Instances.
	//@{
	typedef		NLMISC::CSmartPtr<IShape>		PShape;
	typedef		std::map<std::string, PShape>	TShapeMap;
	TShapeMap	ShapeMap;

	struct CWaitingShape
	{
		IShape *ShapePtr; // Do not work with this value that is shared between threads
		uint32 RefCnt;
		TShapeState State;
		bool *Signal;// To signal when all is done
		uint32 UpTextProgress; // Upload Texture progress
		CWaitingShape (bool *bSignal = NULL)
		{
			State = AsyncLoad_Shape;
			RefCnt = 1;
			ShapePtr = NULL;
			Signal = bSignal;
			UpTextProgress = 0;
		}
	};
	typedef		std::map< std::string, CWaitingShape > TWaitingShapesMap;
	TWaitingShapesMap	WaitingShapes;

	IDriver *_pDriver;
	//@}

	/// \name Shape/Caches.
	//@{
	struct CShapeCache
	{
		std::list<IShape*> Elements;
		sint32 MaxSize;
		CShapeCache() { MaxSize = 0; }
	};

	struct CShapeInfo
	{
		CShapeCache*	pShpCache;
		std::string		sShpName;
		bool			isAdded;
		CShapeInfo() { isAdded = false; pShpCache = NULL; }
	};

private:
	std::string*	getShapeNameFromShapePtr(IShape* pShp);
	IShape*			getShapePtrFromShapeName(const std::string &pShpName);
	CShapeCache*	getShapeCachePtrFromShapePtr(IShape* pShp);
	CShapeCache*	getShapeCachePtrFromShapeCacheName(const std::string &shapeCacheName);
	CShapeCache*	getShapeCachePtrFromShapeName(const std::string &shapeName);
	void			checkShapeCache(CShapeCache* pShpCache);
	void			updateShapeInfo(IShape* pShp, CShapeCache* pShpCache);

	typedef		std::map<std::string,std::string>	TShapeCacheNameMap;
	typedef		std::map<std::string,CShapeCache>	TShapeCacheMap;
	typedef		std::map<IShape*,CShapeInfo>		TShapeInfoMap;
	TShapeCacheNameMap		ShapeNameToShapeCacheName;
	TShapeCacheMap			ShapeCacheNameToShapeCache;
	TShapeInfoMap			ShapePtrToShapeInfo;
	//@}
};

}

#endif // NL_SHAPE_BANK_H

/* End of shape_bank.h */
