/** \file camera.h
 * 
 *
 * $Id: camera.h,v 1.7 2000/12/06 14:32:24 berenguier Exp $
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

#ifndef NL_CAMERA_H
#define NL_CAMERA_H

#include "nel/3d/transform.h"
#include "nel/3d/frustum.h"


namespace	NL3D
{


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		CameraId=NLMISC::CClassId(0x5752634c, 0x6abe76f5);


// ***************************************************************************
/**
 * A Camera node, based on a CTransform node.
 * The camera look in his local Y direction (see CScene).
 *
 * No observer is provided, since a camera:
 *	- has default behavior of a transform.
 *	- can't be clipped (well...  :) ).
 *	- is not lightable
 *	- is not renderable
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CCamera : public CTransform
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();


public:

	/// Set the frustum of the camera.
	void		setFrustum(const CFrustum &f) {_Frustum= f;}
	/// Get the frustum of the camera.
	CFrustum	getFrustum() const {return _Frustum;}
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true);
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true);
	/// Get the camera frustum.
	void		getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const;
	/// Is a ortho camera?
	bool		isOrtho() const;
	/// Is a perspective camera?
	bool		isPerspective() const;
	/** Setup a perspective camera, giving a fov in radians.
	 * \param fov the horizontal angle of view, in radians. (Pi/2 as example)
	 * \param aspectRatio the ratio horizontal/vertical (1.33 as example).
	 * \param znear the front clipping plane distance.
	 * \param zfar the back clipping plane distance.
	 */
	void		setPerspective(float fov, float aspectRatio, float znear, float zfar);


	/** 
	  * Setup camera by the lookAt method. Yes Lionel, it's really usefull!!
	  * 
	  * \param eye is the coordinate of the camera.
	  * \param target is the point the camera look at.
	  * \param roll is the roll angle in radian along the camera's Y axis.
	  */
	void		lookAt (const CVector& eye, const CVector& target, float roll=0.f);


protected:
	/// Constructor
	CCamera();
	/// Destructor
	virtual ~CCamera() {}

	// NB: znear and zfar are be >0 (if perspective).
	CFrustum	_Frustum;

private:
	static IModel	*creator() {return new CCamera;}

};



}


#endif // NL_CAMERA_H

/* End of camera.h */
