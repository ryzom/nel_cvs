/** \file driver_matrix.cpp
 * OpenGL driver implementation : matrix
 *
 * $Id: driver_opengl_matrix.cpp,v 1.2 2000/11/07 15:32:46 berenguier Exp $
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <gl/gl.h>
#include "driver_opengl.h"

// --------------------------------------------------

namespace NL3D
{

// --------------------------------------------------

void CDriverGL::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (perspective)
	{
		glFrustum(left,right,bottom,top,znear,zfar);
	}
	else
	{
		glOrtho(left,right,bottom,top,znear,zfar);
	}
	glMatrixMode(GL_MODELVIEW);
}

// --------------------------------------------------

void CDriverGL::setupViewMatrix(const CMatrix& mtx)
{
	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	// Optimize it...
	changeBasis*= mtx;
	_ViewMtx=changeBasis;
}

CMatrix CDriverGL::getViewMatrix(void) const
{
	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,1);
	CVector		K(0,-1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	return(changeBasis*_ViewMtx);
}

void CDriverGL::setupModelMatrix(const CMatrix& mtx, uint8 n)
{
	CMatrix tmp;

//	nlassert(n);
	tmp=_ViewMtx*mtx;
	glLoadMatrixf( tmp.get() );
}

// --------------------------------------------------

}
