/** \file computed_string.h
 * Computed string
 *
 * $Id: computed_string.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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
#include "nel/misc/smart_ptr.h"
#include "3d/primitive_block.h"
#include "3d/driver.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include <vector>




namespace NL3D {

class CTextureFont;
class CMatrix;
struct CFontDescriptor;


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
public:
	typedef NLMISC::CSmartPtr<NL3D::CMaterial>	PMaterial;
	typedef std::vector<PMaterial>	TMaterialVec;

public:
	CVertexBuffer Vertices;
	std::vector<CPrimitiveBlock>	Primitives;
	TMaterialVec					Materials;
	float StringWidth;
	float StringHeight;

	/**
	 * Hotspot positions (origine for the string placement)
	 */
	enum THotSpot 
	{
		BottomLeft=0,
		MiddleLeft, 
		TopLeft,
		MiddleBottom, 
		MiddleMiddle, 
		MiddleTop, 
		BottomRight, 
		MiddleRight,
		TopRight,

		HotSpotCount
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
	 *	Get the string's origin
	 * \param hotspot the origin of the string
	 */
	CVector getHotSpotVector(THotSpot hotspot);

	/** 
	 * Render the unicode string in a driver.
	 * \param driver the driver where to render the primitives
	 * \param x abscissa
	 * \param y ordinate
	 * \param hotspot position of string origine
	 * \param scaleX abscissa scale
	 * \param scaleY ordinate scale
	 * \param rotateY rotation angle (axe perpendicular to screen)
	 */	
	void render2D (IDriver& driver, 
					float x, float z,
					THotSpot hotspot = BottomLeft,
					float scaleX = 1, float scaleZ = 1,
					float rotateY = 0);

	/** 
	 * Render the unicode string in a driver.
	 * \param driver the driver where to render the primitives
	 * \param matrix transformation matrix
	 * \param hotspot position of string origine
	 */	
	void render3D (IDriver& driver,CMatrix matrix,THotSpot hotspot = MiddleMiddle);

};



} // NL3D


#endif // NL_COMPUTED_STRING_H

/* End of computed_string.h */

