/** \file dru.cpp
 * Driver Utilities.
 *
 * $Id: dru.cpp,v 1.14 2000/12/18 11:13:54 lecroart Exp $
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

#else // NL_OS_WINDOWS

#include <dlfcn.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iso/stdlib_iso.h> /* EXIT_* */

#endif // NL_OS_WINDOWS

namespace NL3D 
{


typedef IDriver* (*IDRV_CREATE_PROC)(void); 
const char *IDRV_CREATE_PROC_NAME = "NL3D_createIDriverInstance";

typedef uint32 (*IDRV_VERSION_PROC)(void); 
const char *IDRV_VERSION_PROC_NAME = "NL3D_interfaceVersion";


IDriver		*CDRU::createGlDriver() throw (EDru)
{
	IDRV_CREATE_PROC	createDriver = NULL;
	IDRV_VERSION_PROC	versionDriver = NULL;

#ifdef NL_OS_WINDOWS

	// WINDOWS code.
	HINSTANCE			hInst;

	hInst=LoadLibrary(NL3D_DLL_NAME);

	if (!hInst)
	{
		throw EDruOpenglDriverNotFound();
	}

	createDriver = (IDRV_CREATE_PROC) GetProcAddress (hInst, IDRV_CREATE_PROC_NAME);
	if (createDriver == NULL)
	{
		throw EDruOpenglDriverCorrupted();
	}

	versionDriver = (IDRV_VERSION_PROC) GetProcAddress (hInst, IDRV_VERSION_PROC_NAME);
	if (versionDriver == NULL)
	{
		throw EDruOpenglDriverOldVersion();
		if (versionDriver()<IDriver::InterfaceVersion)
			throw EDruOpenglDriverOldVersion();
		else if (versionDriver()>IDriver::InterfaceVersion)
			throw EDruOpenglDriverUnknownVersion();
	}

#elif defined (NL_OS_UNIX)

	int		dlclose_status;
    int		dummy_status;
    void*	handle;	/* auch FILE* ? */
    const char*	dlopen_error_msg;

    char dummy_buf[2];

	void *handle = dlopen("libnel_drv_opengl.so", RTLD_NOW);

	if (handle == NULL)
	{
		throw EDruOpenglDriverNotFound();
	}

	/* Not ANSI. Might produce a warning */
	createDriver = (IDRV_CREATE_PROC) dlsym (handle, IDRV_CREATE_PROC_NAME);
	if (createDriver == NULL)
	{
		throw EDruOpenglDriverCorrupted();
	}

	versionDriver = (IDRV_VERSION_PROC) dlsym (handle, IDRV_VERSION_PROC_NAME);
	if (versionDriver == NULL)
	{
		throw EDruOpenglDriverOldVersion();
		if (versionDriver()<IDriver::InterfaceVersion)
			throw EDruOpenglDriverOldVersion();
		else if (versionDriver()>IDriver::InterfaceVersion)
			throw EDruOpenglDriverUnknownVersion();
	}

#else // NL_OS_LINUX
#error "Dynamic DLL loading not implemented!"
#endif // NL_OS_LINUX

	IDriver		*ret= createDriver();
	if (ret == NULL)
	{
		throw EDruOpenglDriverCantCreateDriver();
	}
	return ret;
}

void	CDRU::drawBitmap (float x, float y, float width, float height, ITexture& texture, IDriver& driver, CViewport viewport)
{
	CMatrix mtx;
	mtx.identity();
	driver.setupViewport (viewport);
	driver.setupViewMatrix (mtx);
	driver.setupModelMatrix (mtx);
	driver.setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	static CMaterial mat;
	mat.initUnlit ();
	mat.setTexture (&texture);

	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ|IDRV_VF_UV[0]);
	vb.setNumVertices (4);
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


void	CDRU::drawLine (float x0, float y0, float x1, float y1, IDriver& driver, CRGBA col, CViewport viewport)
{
	CMatrix mtx;
	mtx.identity();
	driver.setupViewport (viewport);
	driver.setupViewMatrix (mtx);
	driver.setupModelMatrix (mtx);
	driver.setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	static CMaterial mat;
	mat.initUnlit ();
	mat.setColor(col);

	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ);
	vb.setNumVertices (2);
	vb.setVertexCoord (0, CVector (x0, 0, y0));
	vb.setVertexCoord (1, CVector (x1, 0, y1));
	driver.activeVertexBuffer(vb);

	CPrimitiveBlock pb;
	pb.setNumLine (1);
	pb.setLine (0, 0, 1);

	driver.render(pb, mat);
}


} // NL3D

