/** \file rect.h
 * <File description>
 *
 * $Id: rect.h,v 1.2 2000/12/22 10:42:05 corvazier Exp $
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

#ifndef NL_RECT_H
#define NL_RECT_H

#include "nel/misc/types_nl.h"


namespace NLMISC {


/**
 * This class describe an integer 2d rectangle.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CRect
{
public:
	/// Constructor with a 2d point, a width and a height
	CRect (sint32 x, sint32 y, uint32 width, uint32 height)
	{
		X=x;
		Y=y;
		Width=width;
		Height=height;
	}

	/// Constructor with a single 2d point. Build a rectangle with width and height = 0.
	CRect (sint32 x, sint32 y)
	{
		X=x;
		Y=y;
		Width=0;
		Height=0;
	}

	/// Extend the box for including the point which coordinates are passed in parameter.
	void		extend (sint32 x, sint32 y);

	/// Return the lower X coordinate of the box
	sint32		left()
	{
		return X;
	}

	/// Return the higher X coordinate of the box + 1
	sint32		right()
	{
		return X+Width;
	}

	/// Return the lower Y coordinate of the box
	sint32		top()
	{
		return Y;
	}

	/// Return the higher Y coordinate of the box + 1
	sint32		bottom()
	{
		return Y+Height;
	}

	/// Compute the x center of the rectangle
	sint32		getXCenter()
	{
		return X+(sint32)(Width>>1);
	}

	/// Compute the y center of the rectangle
	sint32		getYCenter()
	{
		return Y+(sint32)(Height>>1);
	}

	/// Lower X coordinate of the rect
	sint32		X;

	/// Lower Y coordinate of the rect
	sint32		Y;

	/// Width of the rect
	uint32		Width;

	/// Height of the rect
	uint32		Height;
};


} // NLMISC


#endif // NL_RECT_H

/* End of rect.h */
