/** \file viewport.h
 * <File description>
 *
 * $Id: viewport.h,v 1.1 2000/12/01 10:34:16 corvazier Exp $
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

#ifndef NL_VIEWPORT_H
#define NL_VIEWPORT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

namespace NL3D {


/**
 * CViewport is the description of the viewport used to render with a driver
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CViewport
{
public:

	/// Default constructor. Setup a fullscreen viewport
	CViewport ();

	/**
	  * Constructor 
	  *
	  * \param x coordinate of the left edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param y coordinate of the bottom edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param width of the view port. Must be between 0.f and 1.f-x.
	  * \param height of the view port. Must be between 0.f and 1.f-y.
	  */
	void init (float x, float y, float width, float height);

	/// Setup a fullscreen viewport.
	void initFullScreen ();

	/// Setup a 16/9 viewport.
	void init16_9 ();

	/** Get a 3d ray with a 2d point
	  * 
	  * \param x is the x coordinate in the window coordinate system of the 2d point.
	  * \param y is the y coordinate in the window coordinate system of the 2d point.
	  * \param pos gets the position of a 3d point on the ray. It is also the position of the camera
	  * \param dir gets the direction of the ray. The direction is the same than the camera one. It is NOT normalized.
	  * \param camera is the camera in use in this viewport.
	  */
	void getRayWithPoint (float x, float y, NLMISC::CVector& pos, NLMISC::CVector& dir, const class CCamera& camera) const;

	/** Get the viewport values
	  * 
	  * \param x get the x coordinate of the left edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param y get the y coordinate of the bottom edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param width get the width of the view port. Must be between 0.f and 1.f-x.
	  * \param height get the height of the view port. Must be between 0.f and 1.f-y.
	  */
	void getValues (float& x, float& y, float& width, float& height) const
	{
		x=_X;
		y=_Y;
		width=_Width;
		height=_Height;
	}

private:
	float	_X;
	float	_Y;
	float	_Width;
	float	_Height;
};


} // NL3D


#endif // NL_VIEWPORT_H

/* End of viewport.h */
