/** \file frustum.h
 * <File description>
 *
 * $Id: frustum.h,v 1.1 2000/12/06 14:42:21 berenguier Exp $
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

#ifndef NL_FRUSTUM_H
#define NL_FRUSTUM_H

#include "nel/misc/types_nl.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * A Screen frustum, perspective or orthogonal. No default ctor. Usefull for camera.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CFrustum
{
public:
	/// NB: znear and zfar must be >0 (if perspective).
	float			Left, Right, Bottom, Top, Near, Far;
	bool			Perspective;

	/// Init a frustum.
	void			init(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true);

	/// Init a centered frustum.
	void			init(float width, float height, float znear, float zfar, bool perspective= true);

	/** Setup a perspective frustum, giving a fov in radians.
	 * \param fov the horizontal angle of view, in radians. (Pi/2 as example)
	 * \param aspectRatio the ratio horizontal/vertical (1.33 as example).
	 * \param znear the front clipping plane distance.
	 * \param zfar the back clipping plane distance.
	 */
	void			initPerspective(float fov, float aspectRatio, float znear, float zfar);

	/// Get the value of the frustum.
	void			getValues(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const;
};


} // NL3D


#endif // NL_FRUSTUM_H

/* End of frustum.h */
