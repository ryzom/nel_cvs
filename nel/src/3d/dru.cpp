/** \file dru.cpp
 * Driver Utilities.
 *
 * $Id: dru.cpp,v 1.8 2000/12/04 10:12:54 corvazier Exp $
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

#include "nel/misc/types_nl.h"

#include "nel/3d/dru.h"
#include "nel/3d/driver.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

namespace NL3D 
{


typedef uint32 (*IDRV_VERSION_PROC)(void); 
typedef IDriver* (*IDRV_CREATE_PROC)(void); 


IDriver		*CDRU::createGlDriver() throw (EDru)
{
#ifdef NL_OS_WINDOWS

	// WINDOWS code.
	HINSTANCE			hInst;
	IDRV_CREATE_PROC	createDriver;
	IDRV_VERSION_PROC	versionDriver;

	hInst=LoadLibrary("driver_opengl.dll");
	if (!hInst)
	{
		throw EDruOpenglDriverNotFound();
	}

	createDriver=(IDRV_CREATE_PROC)GetProcAddress(hInst,"NL3D_createIDriverInstance");
	if (!createDriver)
	{
		throw EDruOpenglDriverCorrupted();
	}

	versionDriver=(IDRV_VERSION_PROC)GetProcAddress(hInst,"NL3D_interfaceVersion");
	if (!versionDriver)
	{
		throw EDruOpenglDriverOldVersion();
		if (versionDriver()<IDriver::InterfaceVersion)
			throw EDruOpenglDriverOldVersion();
		else if (versionDriver()>IDriver::InterfaceVersion)
			throw EDruOpenglDriverUnknownVersion();
	}

	IDriver		*ret= createDriver();
	if (!ret)
	{
		throw EDruOpenglDriverCantCreateDriver();
	}
	return ret;

#else // NL_OS_WINDOWS

	return NULL;	// Not yet implemented..

#endif // NL_OS_WINDOWS

}

void	CDRU::drawBitmap (float x, float y, float width, float height, ITexture& texture, IDriver& driver, CViewport viewport)
{
	CMatrix mtx;
	mtx.identity();
	driver.setupViewport (viewport);
	driver.setupViewMatrix (mtx);
	driver.setupModelMatrix (mtx);
	driver.setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	CMaterial mat;
	mat.initUnlit ();
	mat.setTexture (&texture);

	CVertexBuffer vb;
	vb.setNumVertices (4);
	vb.setVertexFormat (IDRV_VF_XYZ|IDRV_VF_UV[0]);
	vb.setVertexCoord (0, CVector (x, 0, y));
	vb.setVertexCoord (1, CVector (x+width, 0, y));
	vb.setVertexCoord (2, CVector (x+width, 0, y+height));
	vb.setVertexCoord (3, CVector (x, 0, y+height));
	vb.setTexCoord (0, 0, 0.f, 1.f);
	vb.setTexCoord (1, 0, 1.f, 1.f);
	vb.setTexCoord (2, 0, 1.f, 0.f);
	vb.setTexCoord (3, 0, 0.f, 0.f);
	driver.activeVertexBuffer(vb);

	CPrimitiveBlock pb;
	pb.setNumQuad (1);
	pb.setQuad (0, 0, 1, 2, 3);

	driver.render(pb, mat);
}


} // NL3D

