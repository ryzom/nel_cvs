/** \file u_shape_bank.h
 * Game interface for managing shape bank.
 *
 * $Id: u_shape_bank.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_U_SHAPE_BANK_H
#define NL_U_SHAPE_BANK_H

#include "nel/misc/types_nl.h"

namespace NL3D 
{

/**
 * Game interface for managing shape bank
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class UShapeBank
{
public:

	/**
	  * Add a new ShapeCache. If already exist do nothing.
	  */
	virtual void addShapeCache(const std::string &shapeCacheName) = 0;

	/**
	  * Remove a ShapeCache. All shapes in the shape cache are deleted. All links are redirected
	  * to the default ShapeCache.
	  */
	virtual void removeShapeCache(const std::string &shapeCacheName) = 0;

	/**
	  * Remove all ShapeCache and suppress all links (even the link to the default cache are removed)
	  */
	virtual void reset() = 0;

	/**
	  * Set the shapeCache shapeCacheName the new size.(delete shapes if maxsize<shapeCacheSize).
	  */
	virtual void setShapeCacheSize(const std::string &shapeCacheName, sint32 maxSize) = 0;

	/** 
	  * Link a shape to a ShapeCache. The ShapeCache must exist and must not contains the shape.
	  */
	virtual void linkShapeToShapeCache(const std::string &shapeName, const std::string &shapeCacheName) = 0;

};


} // NL3D


#endif // NL_U_SHAPE_BANK_H

/* End of u_shape_bank.h */
