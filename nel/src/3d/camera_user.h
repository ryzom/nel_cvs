/** \file camera_user.h
 * <File description>
 *
 * $Id: camera_user.h,v 1.4 2002/10/28 17:32:13 corvazier Exp $
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

#ifndef NL_CAMERA_USER_H
#define NL_CAMERA_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/3d/u_camera.h"
#include "3d/transform_user.h"
#include "3d/camera.h"

#define NL3D_MEM_CAMERA					NL_ALLOC_CONTEXT( 3dCam )

namespace NL3D
{


// ***************************************************************************
/**
 * UCamera implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCameraUser : virtual public UCamera, public CTransformUser
{
protected:
	/// This is the SAME pointer than _Transform, but correctly casted.
	CCamera			*_Camera;


public:

	/// \name Object
	// @{
	CCameraUser(CScene *scene) : 
	  CTransformUser(scene, scene->createModel(CameraId) )
	{
		  NL3D_MEM_CAMERA
		_Camera= NLMISC::safe_cast<CCamera*>(_Transform);

		// Default perspective.
		setFrustum(UCamera::DefLx, UCamera::DefLy, UCamera::DefLzNear, UCamera::DefLzFar);
	}
	virtual	~CCameraUser()
	{
		NL3D_MEM_CAMERA
		// deleted in CTransformUser.
		_Camera= NULL;
	}
	// @}


	/// \name Frustum
	// @{
	virtual	void		setFrustum(const CFrustum &f)
	{
		NL3D_MEM_CAMERA
		_Camera->setFrustum(f);
	}
	virtual	const CFrustum&	getFrustum() const 
	{
		NL3D_MEM_CAMERA
		return _Camera->getFrustum();
	}
	virtual	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true)
	{
		NL3D_MEM_CAMERA
		_Camera->setFrustum(left, right, bottom, top, znear, zfar, perspective);
	}
	virtual	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true)
	{
		NL3D_MEM_CAMERA
		_Camera->setFrustum(width, height, znear, zfar, perspective);
	}
	virtual	void		getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const 
	{
		NL3D_MEM_CAMERA
		_Camera->getFrustum(left, right, bottom, top, znear, zfar);
	}
	virtual	bool		isOrtho() const 
	{
		NL3D_MEM_CAMERA
		return _Camera->isOrtho();
	}
	virtual	bool		isPerspective() const 
	{
		NL3D_MEM_CAMERA
		return _Camera->isPerspective();
	}
	virtual	void		setPerspective(float fov, float aspectRatio, float znear, float zfar) 
	{
		NL3D_MEM_CAMERA
		_Camera->setPerspective(fov, aspectRatio, znear, zfar);
	}
	// @}


public:
	/// \name Accessor for CSeneUser.
	// @{
	CCamera		*getCamera()
	{
		NL3D_MEM_CAMERA
		return _Camera;
	}
	// @}

};


} // NL3D


#endif // NL_CAMERA_USER_H

/* End of camera_user.h */
