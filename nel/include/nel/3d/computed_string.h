/** \file computed_string.h
 * Computed string
 *
 * $Id: computed_string.h,v 1.4 2000/11/28 13:21:56 coutelas Exp $
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

#ifndef NL_COMPUTED_STRING_H
#define NL_COMPUTED_STRING_H

#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"
#include <vector>




namespace NL3D {

class CTextureFont;
struct CFontDescriptor;

/*******************************************************************/


/**
 * CDisplayDescriptor
 * used to describe screen configuration :
 * - screen width
 * - screen height
 * - font ratio  
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
struct CDisplayDescriptor
{
	uint32	ResX;
	uint32	ResY;
	float	FontRatio;

	/**
	 * Default constructor
	 */
	CDisplayDescriptor()
	{
		ResX = ResY = 0;
	    FontRatio = 1;
	}
};



/*******************************************************************/



/**
 * CComputedString
 * A CComputedString is a structure which permits to render a string
 * in a driver. It computes 1 primitive block per char.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
struct CComputedString
{
	CVertexBuffer Vertices;
	std::vector<CPrimitiveBlock> Primitives;
	std::vector<CMaterial> Materials;
	float StringWidth;
	float StringHeight;

	/**
	 * Hotspot positions (origine for the string placement)
	 */
	enum THotSpot 
	{
		LeftBottom,
		LeftMiddle, 
		LeftTop,
		MiddleBottom, 
		MiddleMiddle, 
		MiddleTop, 
		RightBottom, 
		RightMiddle,
		RightTop
	};

	/**
	 * Default constructor
	 */
	CComputedString()
	{
		StringWidth = 0;
		StringHeight = 0;
	}


	/** 
	 * render the unicode string in a driver.
	 * user can provides a transformation matrix and a hot spot
	 * \param driver the driver where to render the primitives
	 * \param hotspot position of string origine
	 * \param transformation matrix
	 */	
	void render2D (IDriver& driver, 
					float x, float z,
					THotSpot hotspot = LeftBottom,
					float scaleX = 1, float scaleZ = 1,
					float rotateY = 0);


};



} // NL3D


#endif // NL_COMPUTED_STRING_H

/* End of computed_string.h */

