/** \file driver_opengl_matrix.cpp
 * OpenGL driver implementation : matrix
 *
 * $Id: driver_opengl_matrix.cpp,v 1.14 2002/03/18 14:46:16 berenguier Exp $
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

#include "stdopengl.h"

namespace NL3D {

// ***************************************************************************
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

	// Backup znear and zfar for zbias setup
	_OODeltaZ = 1 / (zfar - znear);

	glMatrixMode(GL_MODELVIEW);
}


// ***************************************************************************
void CDriverGL::setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos)
{
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	_ViewMtx=changeBasis*mtx;
	// Reset the viewMtx position.
	_ViewMtx.setPos(CVector::Null);
	_PZBCameraPos= cameraPos;

	_MatrixSetupDirty= true;
	_ViewMatrixSetupDirty= true;

	_TexMtx = _ViewMtx;
	_TexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_TexMtx.invert();
	_TexMtx = changeBasis *	_TexMtx;
}


// ***************************************************************************
void CDriverGL::setupViewMatrix(const CMatrix& mtx)
{
	_UserViewMtx= mtx;

	// Setup the matrix to transform the CScene basis in openGL basis.
	CMatrix		changeBasis;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	changeBasis.identity();
	changeBasis.setRot(I,J,K, true);
	_ViewMtx=changeBasis*mtx;
	// Just set the PZBCameraPos to 0.
	_PZBCameraPos= CVector::Null;

	_MatrixSetupDirty= true;
	_ViewMatrixSetupDirty= true;

	_TexMtx = _ViewMtx;
	_TexMtx.setPos(CVector(0.0f,0.0f,0.0f));
	_TexMtx.invert();
	_TexMtx = changeBasis *	_TexMtx;

}

// ***************************************************************************
CMatrix CDriverGL::getViewMatrix(void) const
{
	return _UserViewMtx;
}

// ***************************************************************************
void CDriverGL::setupModelMatrix(const CMatrix& mtx, uint8 n)
{
	// Check args
	nlassert (n<IDriver::MaxModelMatrix);

	// profiling
	_NbSetupModelMatrixCall++;


	// Dirt flags.
	_MatrixSetupDirty= true;
	// because we don't know for which (skin/normal/paletteSkin) mode this will be used, we must set the 2 flags.
	_ModelViewMatrixDirty.set(n);
	_ModelViewMatrixDirtyPaletteSkin.set(n);


	// Put the matrix in the opengl eye space, and store it.
	CMatrix		mat= mtx;
	// remove first the _PZBCameraPos
	mat.setPos(mtx.getPos() - _PZBCameraPos);
	_ModelViewMatrix[n]= _ViewMtx*mat;
}

// ***************************************************************************
void CDriverGL::multiplyModelMatrix(const CMatrix& mtx, uint8 n)
{
	// Check args
	nlassert (n<IDriver::MaxModelMatrix);


	// Dirt flags.
	_MatrixSetupDirty= true;
	// because we don't know for which (skin/normal/paletteSkin) mode this will be used, we must set the 2 flags.
	_ModelViewMatrixDirty.set(n);
	_ModelViewMatrixDirtyPaletteSkin.set(n);


	// multiply this modelMatrix with the _ModelViewMatrix.
	_ModelViewMatrix[n]= _ModelViewMatrix[n]*mtx;
}



// ***************************************************************************
void	CDriverGL::CMatrix3x4::set(const CMatrix &mat)
{
	const float	*m =mat.get();
	a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12]; 
	a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13]; 
	a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14]; 
}


} // NL3D
