/** \file viewport.h
 * TODO: File description
 *
 * $Id: viewport.h,v 1.10 2005/02/22 10:14:13 besson Exp $
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
#include "nel/misc/matrix.h"
#include "frustum.h"

#define NL3D_MEM_VIEWPORT						NL_ALLOC_CONTEXT( 3dVwPrt )

namespace NL3D
{

using NLMISC::CVector;
using NLMISC::CMatrix;



/**
 * CViewport is the description of the viewport used to render with a driver
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
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
	  * \param camMatrix is the matrix of the camera in use in this viewport.
	  * \param camFrust is the frustum of the camera in use in this viewport.
	  */
	void getRayWithPoint (float x, float y, CVector& pos, CVector& dir, const CMatrix& camMatrix, const CFrustum& camFrust) const;

	/** Get the viewport values
	  * 
	  * \param x get the x coordinate of the left edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param y get the y coordinate of the bottom edge of the viewport in the window coordinate system . Must be between 0.f and 1.f.
	  * \param width get the width of the view port. Must be between 0.f and 1.f-x.
	  * \param height get the height of the view port. Must be between 0.f and 1.f-y.
	  */
	void getValues (float& x, float& y, float& width, float& height) const
	{
		NL3D_MEM_VIEWPORT
		x=_X;
		y=_Y;
		width=_Width;
		height=_Height;
	}

	float getX() const { return _X; }	
	float getY() const { return _Y; }	
	float getWidth() const { return _Width; }	
	float getHeight() const { return _Height; }	


private:
	float	_X;
	float	_Y;
	float	_Width;
	float	_Height;
};


} // NL3D


#endif // NL_VIEWPORT_H

/* End of viewport.h */
