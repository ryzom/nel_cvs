/** \file u_camera.h
 * <File description>
 *
 * $Id: u_camera.h,v 1.2 2003/11/06 09:17:26 berenguier Exp $
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

#ifndef NL_U_CAMERA_H
#define NL_U_CAMERA_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_transform.h"
#include "nel/3d/frustum.h"


namespace NL3D
{


// ***************************************************************************
/**
 * Game interface for manipulating Camera.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UCamera : virtual public UTransform
{
protected:

	/// \name Object
	// @{
	UCamera() {}
	virtual	~UCamera() {}
	// @}

public:
	/// \name Default Camera frustum (perspective).
	//@{
	static const float		DefLx;		//=0.26f;
	static const float		DefLy;		//=0.2f;
	static const float		DefLzNear;	//=0.15f;
	static const float		DefLzFar;	//=1000.0f;
	//@}


public:


	/// \name Frustum
	// @{
	/// Set the frustum of the camera.
	virtual	void		setFrustum(const CFrustum &f)=0;
	/// Get the frustum of the camera.
	virtual	const CFrustum&	getFrustum() const =0;
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true)=0;
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	virtual	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true)=0;
	/// Get the camera frustum.
	virtual	void		getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const =0;
	/// Is a ortho camera?
	virtual	bool		isOrtho() const =0;
	/// Is a perspective camera?
	virtual	bool		isPerspective() const =0;
	/** Setup a perspective camera, giving a fov in radians.
	 * \param fov the horizontal angle of view, in radians. (Pi/2 as example)
	 * \param aspectRatio the ratio horizontal/vertical (1.33 as example).
	 * \param znear the front clipping plane distance.
	 * \param zfar the back clipping plane distance.
	 */
	virtual	void		setPerspective(float fov, float aspectRatio, float znear, float zfar) =0;
	// @}

	/// \name Misc
	// @{
	virtual	void		buildCameraPyramid(std::vector<NLMISC::CPlane>	&pyramid, bool useWorldMatrix) =0;
	// @}
	

};


} // NL3D


#endif // NL_U_CAMERA_H

/* End of u_camera.h */
