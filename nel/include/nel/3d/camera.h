/* camera.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: camera.h,v 1.2 2000/10/10 16:12:05 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_CAMERA_H
#define NL_CAMERA_H

#include "nel/3d/transform.h"


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

	/// Constructor
	CCamera();

	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective= true);
	/// Setup the camera mode as a perspective/ortho camera. NB: znear and zfar must be >0 (if perspective).
	void		setFrustum(float width, float height, float znear, float zfar, bool perspective= true);
	/// Get the camera frustum.
	void		getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar);
	/// Is a ortho camera?
	bool		isOrtho();
	/// Is a perspective camera?
	bool		isPerspective();

protected:
	// NB: znear and zfar are be >0 (if perspective).
	float			Left, Right, Bottom, Top, Near, Far;
	bool			Perspective;
};



}


#endif // NL_CAMERA_H

/* End of camera.h */
