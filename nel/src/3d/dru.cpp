/** \file dru.cpp
 * Driver Utilities.
 *
 * $Id: dru.cpp,v 1.26 2001/01/18 16:13:22 berenguier Exp $
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
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/primitive_block.h"

#ifdef NL_OS_WINDOWS

#include <windows.h>

#else // NL_OS_WINDOWS

#include <dlfcn.h>

#endif // NL_OS_WINDOWS

namespace NL3D 
{


typedef IDriver* (*IDRV_CREATE_PROC)(void); 
const char *IDRV_CREATE_PROC_NAME = "NL3D_createIDriverInstance";

typedef uint32 (*IDRV_VERSION_PROC)(void); 
const char *IDRV_VERSION_PROC_NAME = "NL3D_interfaceVersion";


// ***************************************************************************
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

	void *handle = dlopen(NL3D_DLL_NAME, RTLD_NOW);

	if (handle == NULL)
	{
	  nlwarning ("when loading dynamic library '%s': %s", NL3D_DLL_NAME, dlerror());
	  throw EDruOpenglDriverNotFound();
	}

	/* Not ANSI. Might produce a warning */
	createDriver = (IDRV_CREATE_PROC) dlsym (handle, IDRV_CREATE_PROC_NAME);
	if (createDriver == NULL)
	{
	  nlwarning ("when getting function in dynamic library '%s': %s", NL3D_DLL_NAME, dlerror());
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


// ***************************************************************************
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
	mat.setTexture (0, &texture);
	mat.setBlend(true);

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


// ***************************************************************************
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


// ***************************************************************************
void	CDRU::drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, IDriver& driver, CRGBA col, CViewport viewport)
{
	CMatrix mtx;
	mtx.identity();
	driver.setupViewport (viewport);
	driver.setupViewMatrix (mtx);
	driver.setupModelMatrix (mtx);
	driver.setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	static CMaterial mat;
	mat.initUnlit();
	mat.setSrcBlend(CMaterial::srcalpha);
	mat.setDstBlend(CMaterial::invsrcalpha);
	mat.setBlend(true);
	mat.setColor(col);

	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ);
	vb.setNumVertices (3);
	vb.setVertexCoord (0, CVector (x0, 0, y0));
	vb.setVertexCoord (1, CVector (x1, 0, y1));
	vb.setVertexCoord (2, CVector (x2, 0, y2));
	driver.activeVertexBuffer(vb);

	CPrimitiveBlock pb;
	pb.setNumTri (1);
	pb.setTri (0, 0, 1, 2);

	driver.render(pb, mat);
}



// ***************************************************************************
void	CDRU::drawQuad (float x0, float y0, float x1, float y1, IDriver& driver, CRGBA col, CViewport viewport)
{
	CMatrix mtx;
	mtx.identity();
	driver.setupViewport (viewport);
	driver.setupViewMatrix (mtx);
	driver.setupModelMatrix (mtx);
	driver.setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	static CMaterial mat;
	mat.initUnlit();
	mat.setSrcBlend(CMaterial::srcalpha);
	mat.setDstBlend(CMaterial::invsrcalpha);
	mat.setBlend(true);
	mat.setColor(col);
	
	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ);
	vb.setNumVertices (4);
	vb.setVertexCoord (0, CVector (x0, 0, y0));
	vb.setVertexCoord (1, CVector (x1, 0, y0));
	vb.setVertexCoord (2, CVector (x1, 0, y1));
	vb.setVertexCoord (3, CVector (x0, 0, y1));
	
	driver.activeVertexBuffer(vb);

	CPrimitiveBlock pb;
	pb.setNumQuad (1);
	pb.setQuad (0, 0, 1, 2, 3);

	driver.render(pb, mat);
}


// ***************************************************************************
void	CDRU::drawQuad (float xcenter, float ycenter, float radius, IDriver& driver, CRGBA col, CViewport viewport)
{
	CMatrix mtx;
	mtx.identity();
	driver.setupViewport (viewport);
	driver.setupViewMatrix (mtx);
	driver.setupModelMatrix (mtx);
	driver.setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	static CMaterial mat;
	mat.initUnlit();
	mat.setSrcBlend(CMaterial::srcalpha);
	mat.setDstBlend(CMaterial::invsrcalpha);
	mat.setBlend(true);
	mat.setColor(col);
	
	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ);
	vb.setNumVertices (4);
	vb.setVertexCoord (0, CVector (xcenter-radius, 0, ycenter-radius));
	vb.setVertexCoord (1, CVector (xcenter+radius, 0, ycenter-radius));
	vb.setVertexCoord (2, CVector (xcenter+radius, 0, ycenter+radius));
	vb.setVertexCoord (3, CVector (xcenter-radius, 0, ycenter+radius));
	
	driver.activeVertexBuffer(vb);

	CPrimitiveBlock pb;
	pb.setNumQuad (1);
	pb.setQuad (0, 0, 1, 2, 3);

	driver.render(pb, mat);
}


// ***************************************************************************
void			CDRU::drawTrianglesUnlit(const CTriangleUV	*trilist, sint ntris, CMaterial &mat, IDriver& driver)
{
	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ | IDRV_VF_UV[0]);
	vb.setNumVertices (ntris*3);

	static	CPrimitiveBlock pb;
	pb.setNumTri(ntris);

	for(sint i=0;i<ntris;i++)
	{
		vb.setVertexCoord (i*3+0, trilist[i].V0);
		vb.setVertexCoord (i*3+1, trilist[i].V1);
		vb.setVertexCoord (i*3+2, trilist[i].V2);
		vb.setTexCoord (i*3+0, 0, trilist[i].Uv0);
		vb.setTexCoord (i*3+1, 0, trilist[i].Uv1);
		vb.setTexCoord (i*3+2, 0, trilist[i].Uv2);
		pb.setTri(i, i*3+0, i*3+1, i*3+2);
	}
	
	driver.activeVertexBuffer(vb);
	driver.render(pb, mat);
}


// ***************************************************************************
void			CDRU::drawTrianglesUnlit(const std::vector<CTriangleUV> &trilist, CMaterial &mat, IDriver& driver)
{
	if(trilist.size()==0)
		return;
	
	CDRU::drawTrianglesUnlit( &(*trilist.begin()), trilist.size(), mat, driver);
}


// ***************************************************************************
void			CDRU::drawLinesUnlit(const CLine	*linelist, sint nlines, CMaterial &mat, IDriver& driver)
{
	static CVertexBuffer vb;
	vb.setVertexFormat (IDRV_VF_XYZ);
	vb.setNumVertices (nlines*2);

	static	CPrimitiveBlock pb;
	pb.setNumLine(nlines);

	for(sint i=0;i<nlines;i++)
	{
		vb.setVertexCoord (i*2+0, linelist[i].V0);
		vb.setVertexCoord (i*2+1, linelist[i].V1);
		pb.setLine(i, i*2+0, i*2+1);
	}
	
	driver.activeVertexBuffer(vb);
	driver.render(pb, mat);
}
// ***************************************************************************
void			CDRU::drawLinesUnlit(const std::vector<CLine> &linelist, CMaterial &mat, IDriver& driver)
{
	if(linelist.size()==0)
		return;
	CDRU::drawLinesUnlit( &(*linelist.begin()), linelist.size(), mat, driver);
}
// ***************************************************************************
void			CDRU::drawLine(const CVector &a, const CVector &b, CRGBA color, IDriver& driver)
{
	static	CLine		line;
	static	CMaterial	mat;
	static	bool		inited= false;

	// Setup material.
	if(!inited)
	{
		inited= true;
		mat.initUnlit();
	}
	mat.setColor(color);


	line.V0= a;
	line.V1= b;
	CDRU::drawLinesUnlit(&line, 1, mat, driver);
}



} // NL3D

