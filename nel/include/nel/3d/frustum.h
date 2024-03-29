/** \file frustum.h
 * TODO: File description
 *
 * $Id: frustum.h,v 1.9 2004/11/15 10:24:16 lecroart Exp $
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
#include "nel/misc/vector.h"

#define NL3D_MEM_FRUSTRUM						NL_ALLOC_CONTEXT( 3dFrtrm )

namespace NL3D
{

using NLMISC::CVector;


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


	/// Un-initialized frustum.
	CFrustum() 
	{
		NL3D_MEM_FRUSTRUM
	}
	/// ctor.
	CFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true)
	{
		NL3D_MEM_FRUSTRUM
		init( left,  right,  bottom,  top,  znear,  zfar, perspective);
	}


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


	/** project a vector (x,y,z) onto frustum.
	  * \param vec the point in 3D frustum space. Axis is NL3D axis: Xright, Yfront, Ztop.
	  * \return the point in 2D: Xright, Ytop, Z=0.  Screen is mapped to X:[0..1], Y: [0..1].
	  */
	CVector			project(const CVector &vec) const;

	/** project a vector (x,y,z) onto frustum.
	  * \param vec the point in 3D frustum space. Axis is NL3D axis: Xright, Yfront, Ztop.
	  * \return the point in 2D: Xright, Ytop, Z=0.  Screen is mapped to X:[0..1], Y: [0..1] Z is a positive screen depth in world units.
	  */
	CVector			projectZ(const CVector &vec) const;

	/** unproject a point from screen to the frustum space.
	 * \param vec the point on the screen, in Left hand axis (XRight, YTop, ZFront). Z must be in [0..1] and hyperbolic.
	 * \return the point in the frustum space (NL3D right hand axis).
	 */
	CVector			unProject(const CVector &vec) const;

	/** unproject a point from screen to the frustum space.
	 * \param vec the point on the screen, in Left hand axis (XRight, YTop, ZFront). Z is a positive depth in world unit.
	 * \return the point in the frustum space (NL3D right hand axis).
	 */
	CVector			unProjectZ(const CVector &vec) const;

	/** return the aspect ration of this frustum. this is simply (Right-Left)/(Top-Bottom)
	 */
	float			getAspectRatio() const;
};


} // NL3D


#endif // NL_FRUSTUM_H

/* End of frustum.h */
