/** \file shape_bank.h
 * <File description>
 *
 * $Id: shape_bank.h,v 1.2 2001/04/17 13:28:54 besson Exp $
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

#include "nel/3d/shape.h"
#include "nel/misc/smart_ptr.h"
#include <map>
#include <list>


namespace NL3D
{

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

	CShapeBank();
	~CShapeBank();

	/// \name Instance Management
	//@{
	/// Add a reference to a shape and return the instance created.
	IShape*			addRef(const std::string &shapeName);

	/** Release a reference to a shape by its instance. If the shape has no more reference it is added to
	 * its own shape cache. When the shape cache is full the last entry is deleted.
	 */
	void			release(IShape* pShp);

	/// Return TRUE if the shape is present in the bank.
	bool			isPresent(const std::string &shapeName);

	/// Load the corresponding file from disk and add it to the bank.
	void			load(const std::string &shapeName);

	/// Add directly a shape to the bank. If the shape name is already used do nothing.
	void			add(const std::string &shapeName, IShape* shape);
	//@}

	/// \name Shape cache management
	//@{
	/// Add a new ShapeCache. If already exist do nothing.
	void			addShapeCache(const std::string &shapeCacheName);

	/// Set the shapeCache shapeCacheName the new size.(delete shapes if maxsize<shapeCacheSize).
	void			setShapeCacheSize(const std::string &shapeCacheName, sint32 maxSize);

	/// Link a shape to a ShapeCache. The ShapeCache must exist and must not contains the shape.
	void			linkShapeToShapeCache(const std::string &shapeName, const std::string &shapeCacheName);
	//@}

private:
	/// \name Shape/Instances.
	//@{
	typedef		NLMISC::CSmartPtr<IShape>		PShape;
	typedef		std::map<std::string, PShape>	TShapeMap;
	TShapeMap	ShapeMap;
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
