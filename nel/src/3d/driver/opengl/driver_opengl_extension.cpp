/** \file driver_opengl_extension.cpp
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.cpp,v 1.33 2002/03/14 18:28:20 vizerie Exp $
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

#include "driver_opengl_extension.h"
#include "3d/material.h"

// ***************************************************************************
#ifdef NL_OS_WINDOWS
#define	nelglGetProcAddress wglGetProcAddress
#else	// NL_OS_WINDOWS
void (*nelglGetProcAddress(const char *procName))()
{
	return glXGetProcAddressARB((const GLubyte *)procName);
}
#endif	// NL_OS_WINDOWS


// ***************************************************************************
// The exported function names

// ARB_multitexture
//=================
NEL_PFNGLACTIVETEXTUREARBPROC nglActiveTextureARB;
NEL_PFNGLCLIENTACTIVETEXTUREARBPROC nglClientActiveTextureARB;

NEL_PFNGLMULTITEXCOORD1SARBPROC nglMultiTexCoord1sARB;
NEL_PFNGLMULTITEXCOORD1IARBPROC nglMultiTexCoord1iARB;
NEL_PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
NEL_PFNGLMULTITEXCOORD1DARBPROC nglMultiTexCoord1dARB;
NEL_PFNGLMULTITEXCOORD2SARBPROC nglMultiTexCoord2sARB;
NEL_PFNGLMULTITEXCOORD2IARBPROC nglMultiTexCoord2iARB;
NEL_PFNGLMULTITEXCOORD2FARBPROC nglMultiTexCoord2fARB;
NEL_PFNGLMULTITEXCOORD2DARBPROC nglMultiTexCoord2dARB;
NEL_PFNGLMULTITEXCOORD3SARBPROC nglMultiTexCoord3sARB;
NEL_PFNGLMULTITEXCOORD3IARBPROC nglMultiTexCoord3iARB;
NEL_PFNGLMULTITEXCOORD3FARBPROC nglMultiTexCoord3fARB;
NEL_PFNGLMULTITEXCOORD3DARBPROC nglMultiTexCoord3dARB;
NEL_PFNGLMULTITEXCOORD4SARBPROC nglMultiTexCoord4sARB;
NEL_PFNGLMULTITEXCOORD4IARBPROC nglMultiTexCoord4iARB;
NEL_PFNGLMULTITEXCOORD4FARBPROC nglMultiTexCoord4fARB;
NEL_PFNGLMULTITEXCOORD4DARBPROC nglMultiTexCoord4dARB;

NEL_PFNGLMULTITEXCOORD1SVARBPROC nglMultiTexCoord1svARB;
NEL_PFNGLMULTITEXCOORD1IVARBPROC nglMultiTexCoord1ivARB;
NEL_PFNGLMULTITEXCOORD1FVARBPROC nglMultiTexCoord1fvARB;
NEL_PFNGLMULTITEXCOORD1DVARBPROC nglMultiTexCoord1dvARB;
NEL_PFNGLMULTITEXCOORD2SVARBPROC nglMultiTexCoord2svARB;
NEL_PFNGLMULTITEXCOORD2IVARBPROC nglMultiTexCoord2ivARB;
NEL_PFNGLMULTITEXCOORD2FVARBPROC nglMultiTexCoord2fvARB;
NEL_PFNGLMULTITEXCOORD2DVARBPROC nglMultiTexCoord2dvARB;
NEL_PFNGLMULTITEXCOORD3SVARBPROC nglMultiTexCoord3svARB;
NEL_PFNGLMULTITEXCOORD3IVARBPROC nglMultiTexCoord3ivARB;
NEL_PFNGLMULTITEXCOORD3FVARBPROC nglMultiTexCoord3fvARB;
NEL_PFNGLMULTITEXCOORD3DVARBPROC nglMultiTexCoord3dvARB;
NEL_PFNGLMULTITEXCOORD4SVARBPROC nglMultiTexCoord4svARB;
NEL_PFNGLMULTITEXCOORD4IVARBPROC nglMultiTexCoord4ivARB;
NEL_PFNGLMULTITEXCOORD4FVARBPROC nglMultiTexCoord4fvARB;
NEL_PFNGLMULTITEXCOORD4DVARBPROC nglMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC	nglCompressedTexImage3DARB;
NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC	nglCompressedTexImage2DARB;
NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC	nglCompressedTexImage1DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	nglCompressedTexSubImage3DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	nglCompressedTexSubImage2DARB;
NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	nglCompressedTexSubImage1DARB;
NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	nglGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
NEL_PFNGLFLUSHVERTEXARRAYRANGENVPROC	nglFlushVertexArrayRangeNV;
NEL_PFNGLVERTEXARRAYRANGENVPROC			nglVertexArrayRangeNV;
#ifdef NL_OS_WINDOWS
PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;
#endif


// FenceNV.
//====================
NEL_PFNGLDELETEFENCESNVPROC				nglDeleteFencesNV;
NEL_PFNGLGENFENCESNVPROC				nglGenFencesNV;
NEL_PFNGLISFENCENVPROC					nglIsFenceNV;
NEL_PFNGLTESTFENCENVPROC				nglTestFenceNV;
NEL_PFNGLGETFENCEIVNVPROC				nglGetFenceivNV;
NEL_PFNGLFINISHFENCENVPROC				nglFinishFenceNV;
NEL_PFNGLSETFENCENVPROC					nglSetFenceNV;


// VertexWeighting.
//====================
NEL_PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
NEL_PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
NEL_PFNGLAREPROGRAMSRESIDENTNVPROC		nglAreProgramsResidentNV;
NEL_PFNGLBINDPROGRAMNVPROC				nglBindProgramNV;
NEL_PFNGLDELETEPROGRAMSNVPROC			nglDeleteProgramsNV;
NEL_PFNGLEXECUTEPROGRAMNVPROC			nglExecuteProgramNV;
NEL_PFNGLGENPROGRAMSNVPROC				nglGenProgramsNV;
NEL_PFNGLGETPROGRAMPARAMETERDVNVPROC	nglGetProgramParameterdvNV;
NEL_PFNGLGETPROGRAMPARAMETERFVNVPROC	nglGetProgramParameterfvNV;
NEL_PFNGLGETPROGRAMIVNVPROC				nglGetProgramivNV;
NEL_PFNGLGETPROGRAMSTRINGNVPROC			nglGetProgramStringNV;
NEL_PFNGLGETTRACKMATRIXIVNVPROC			nglGetTrackMatrixivNV;
NEL_PFNGLGETVERTEXATTRIBDVNVPROC		nglGetVertexAttribdvNV;
NEL_PFNGLGETVERTEXATTRIBFVNVPROC		nglGetVertexAttribfvNV;
NEL_PFNGLGETVERTEXATTRIBIVNVPROC		nglGetVertexAttribivNV;
NEL_PFNGLGETVERTEXATTRIBPOINTERVNVPROC	nglGetVertexAttribPointervNV;
NEL_PFNGLISPROGRAMNVPROC				nglIsProgramNV;
NEL_PFNGLLOADPROGRAMNVPROC				nglLoadProgramNV;
NEL_PFNGLPROGRAMPARAMETER4DNVPROC		nglProgramParameter4dNV;
NEL_PFNGLPROGRAMPARAMETER4DVNVPROC		nglProgramParameter4dvNV;
NEL_PFNGLPROGRAMPARAMETER4FNVPROC		nglProgramParameter4fNV;
NEL_PFNGLPROGRAMPARAMETER4FVNVPROC		nglProgramParameter4fvNV;
NEL_PFNGLPROGRAMPARAMETERS4DVNVPROC		nglProgramParameters4dvNV;
NEL_PFNGLPROGRAMPARAMETERS4FVNVPROC		nglProgramParameters4fvNV;
NEL_PFNGLREQUESTRESIDENTPROGRAMSNVPROC	nglRequestResidentProgramsNV;
NEL_PFNGLTRACKMATRIXNVPROC				nglTrackMatrixNV;
NEL_PFNGLVERTEXATTRIBPOINTERNVPROC		nglVertexAttribPointerNV;
NEL_PFNGLVERTEXATTRIB1DNVPROC			nglVertexAttrib1dNV;
NEL_PFNGLVERTEXATTRIB1DVNVPROC			nglVertexAttrib1dvNV;
NEL_PFNGLVERTEXATTRIB1FNVPROC			nglVertexAttrib1fNV;
NEL_PFNGLVERTEXATTRIB1FVNVPROC			nglVertexAttrib1fvNV;
NEL_PFNGLVERTEXATTRIB1SNVPROC			nglVertexAttrib1sNV;
NEL_PFNGLVERTEXATTRIB1SVNVPROC			nglVertexAttrib1svNV;
NEL_PFNGLVERTEXATTRIB2DNVPROC			nglVertexAttrib2dNV;
NEL_PFNGLVERTEXATTRIB2DVNVPROC			nglVertexAttrib2dvNV;
NEL_PFNGLVERTEXATTRIB2FNVPROC			nglVertexAttrib2fNV;
NEL_PFNGLVERTEXATTRIB2FVNVPROC			nglVertexAttrib2fvNV;
NEL_PFNGLVERTEXATTRIB2SNVPROC			nglVertexAttrib2sNV;
NEL_PFNGLVERTEXATTRIB2SVNVPROC			nglVertexAttrib2svNV;
NEL_PFNGLVERTEXATTRIB3DNVPROC			nglVertexAttrib3dNV;
NEL_PFNGLVERTEXATTRIB3DVNVPROC			nglVertexAttrib3dvNV;
NEL_PFNGLVERTEXATTRIB3FNVPROC			nglVertexAttrib3fNV;
NEL_PFNGLVERTEXATTRIB3FVNVPROC			nglVertexAttrib3fvNV;
NEL_PFNGLVERTEXATTRIB3SNVPROC			nglVertexAttrib3sNV;
NEL_PFNGLVERTEXATTRIB3SVNVPROC			nglVertexAttrib3svNV;
NEL_PFNGLVERTEXATTRIB4DNVPROC			nglVertexAttrib4dNV;
NEL_PFNGLVERTEXATTRIB4DVNVPROC			nglVertexAttrib4dvNV;
NEL_PFNGLVERTEXATTRIB4FNVPROC			nglVertexAttrib4fNV;
NEL_PFNGLVERTEXATTRIB4FVNVPROC			nglVertexAttrib4fvNV;
NEL_PFNGLVERTEXATTRIB4SNVPROC			nglVertexAttrib4sNV;
NEL_PFNGLVERTEXATTRIB4SVNVPROC			nglVertexAttrib4svNV;
NEL_PFNGLVERTEXATTRIB4UBVNVPROC			nglVertexAttrib4ubvNV;
NEL_PFNGLVERTEXATTRIBS1DVNVPROC			nglVertexAttribs1dvNV;
NEL_PFNGLVERTEXATTRIBS1FVNVPROC			nglVertexAttribs1fvNV;
NEL_PFNGLVERTEXATTRIBS1SVNVPROC			nglVertexAttribs1svNV;
NEL_PFNGLVERTEXATTRIBS2DVNVPROC			nglVertexAttribs2dvNV;
NEL_PFNGLVERTEXATTRIBS2FVNVPROC			nglVertexAttribs2fvNV;
NEL_PFNGLVERTEXATTRIBS2SVNVPROC			nglVertexAttribs2svNV;
NEL_PFNGLVERTEXATTRIBS3DVNVPROC			nglVertexAttribs3dvNV;
NEL_PFNGLVERTEXATTRIBS3FVNVPROC			nglVertexAttribs3fvNV;
NEL_PFNGLVERTEXATTRIBS3SVNVPROC			nglVertexAttribs3svNV;
NEL_PFNGLVERTEXATTRIBS4DVNVPROC			nglVertexAttribs4dvNV;
NEL_PFNGLVERTEXATTRIBS4FVNVPROC			nglVertexAttribs4fvNV;
NEL_PFNGLVERTEXATTRIBS4SVNVPROC			nglVertexAttribs4svNV;
NEL_PFNGLVERTEXATTRIBS4UBVNVPROC		nglVertexAttribs4ubvNV;


// SecondaryColor extension
//========================
NEL_PFNGLSECONDARYCOLOR3BEXTPROC		nglSecondaryColor3bEXT;
NEL_PFNGLSECONDARYCOLOR3BVEXTPROC		nglSecondaryColor3bvEXT;
NEL_PFNGLSECONDARYCOLOR3DEXTPROC		nglSecondaryColor3dEXT;
NEL_PFNGLSECONDARYCOLOR3DVEXTPROC		nglSecondaryColor3dvEXT;
NEL_PFNGLSECONDARYCOLOR3FEXTPROC		nglSecondaryColor3fEXT;
NEL_PFNGLSECONDARYCOLOR3FVEXTPROC		nglSecondaryColor3fvEXT;
NEL_PFNGLSECONDARYCOLOR3IEXTPROC		nglSecondaryColor3iEXT;
NEL_PFNGLSECONDARYCOLOR3IVEXTPROC		nglSecondaryColor3ivEXT;
NEL_PFNGLSECONDARYCOLOR3SEXTPROC		nglSecondaryColor3sEXT;
NEL_PFNGLSECONDARYCOLOR3SVEXTPROC		nglSecondaryColor3svEXT;
NEL_PFNGLSECONDARYCOLOR3UBEXTPROC		nglSecondaryColor3ubEXT;
NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC		nglSecondaryColor3ubvEXT;
NEL_PFNGLSECONDARYCOLOR3UIEXTPROC		nglSecondaryColor3uiEXT;
NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC		nglSecondaryColor3uivEXT;
NEL_PFNGLSECONDARYCOLOR3USEXTPROC		nglSecondaryColor3usEXT;
NEL_PFNGLSECONDARYCOLOR3USVEXTPROC		nglSecondaryColor3usvEXT;
NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC	nglSecondaryColorPointerEXT;


// Pbuffer extension
//==================
#ifdef NL_OS_WINDOWS
PFNWGLCREATEPBUFFERARBPROC			wglCreatePbufferARB;
PFNWGLGETPUFFERDCARBPROC			wglGetPbufferDCARB;
PFNWGLRELEASEPUFFERDCARBPROC		wglReleasePbufferDCARB;
PFNWGLDESTROYPUFFERARBPROC			wglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC			wglQueryPbufferARB;
#endif


// Get Pixel format extension
//===========================
#ifdef NL_OS_WINDOWS
PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribivARB;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfvARB;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB;
#endif


// WGL_ARB_extensions_string
#ifdef NL_OS_WINDOWS
PFNWGFGETEXTENSIONSSTRINGARB		wglGetExtensionsStringARB;
#endif





// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


namespace	NL3D
{


// ***************************************************************************
// Extensions registrations, and Windows function Registration.


// *********************************
static bool setupARBMultiTexture(const char	*glext)
{
	if(strstr(glext, "GL_ARB_multitexture")==NULL)
		return false;

	if(!(nglActiveTextureARB=(NEL_PFNGLACTIVETEXTUREARBPROC)nelglGetProcAddress("glActiveTextureARB")))return false;
	if(!(nglClientActiveTextureARB=(NEL_PFNGLCLIENTACTIVETEXTUREARBPROC)nelglGetProcAddress("glClientActiveTextureARB")))return false;

	if(!(nglMultiTexCoord1sARB=(NEL_PFNGLMULTITEXCOORD1SARBPROC)nelglGetProcAddress("glMultiTexCoord1sARB")))return false;
	if(!(nglMultiTexCoord1iARB=(NEL_PFNGLMULTITEXCOORD1IARBPROC)nelglGetProcAddress("glMultiTexCoord1iARB")))return false;
	if(!(nglMultiTexCoord1fARB=(NEL_PFNGLMULTITEXCOORD1FARBPROC)nelglGetProcAddress("glMultiTexCoord1fARB")))return false;
	if(!(nglMultiTexCoord1dARB=(NEL_PFNGLMULTITEXCOORD1DARBPROC)nelglGetProcAddress("glMultiTexCoord1dARB")))return false;
	if(!(nglMultiTexCoord2sARB=(NEL_PFNGLMULTITEXCOORD2SARBPROC)nelglGetProcAddress("glMultiTexCoord2sARB")))return false;
	if(!(nglMultiTexCoord2iARB=(NEL_PFNGLMULTITEXCOORD2IARBPROC)nelglGetProcAddress("glMultiTexCoord2iARB")))return false;
	if(!(nglMultiTexCoord2fARB=(NEL_PFNGLMULTITEXCOORD2FARBPROC)nelglGetProcAddress("glMultiTexCoord2fARB")))return false;
	if(!(nglMultiTexCoord2dARB=(NEL_PFNGLMULTITEXCOORD2DARBPROC)nelglGetProcAddress("glMultiTexCoord2dARB")))return false;
	if(!(nglMultiTexCoord3sARB=(NEL_PFNGLMULTITEXCOORD3SARBPROC)nelglGetProcAddress("glMultiTexCoord3sARB")))return false;
	if(!(nglMultiTexCoord3iARB=(NEL_PFNGLMULTITEXCOORD3IARBPROC)nelglGetProcAddress("glMultiTexCoord3iARB")))return false;
	if(!(nglMultiTexCoord3fARB=(NEL_PFNGLMULTITEXCOORD3FARBPROC)nelglGetProcAddress("glMultiTexCoord3fARB")))return false;
	if(!(nglMultiTexCoord3dARB=(NEL_PFNGLMULTITEXCOORD3DARBPROC)nelglGetProcAddress("glMultiTexCoord3dARB")))return false;
	if(!(nglMultiTexCoord4sARB=(NEL_PFNGLMULTITEXCOORD4SARBPROC)nelglGetProcAddress("glMultiTexCoord4sARB")))return false;
	if(!(nglMultiTexCoord4iARB=(NEL_PFNGLMULTITEXCOORD4IARBPROC)nelglGetProcAddress("glMultiTexCoord4iARB")))return false;
	if(!(nglMultiTexCoord4fARB=(NEL_PFNGLMULTITEXCOORD4FARBPROC)nelglGetProcAddress("glMultiTexCoord4fARB")))return false;
	if(!(nglMultiTexCoord4dARB=(NEL_PFNGLMULTITEXCOORD4DARBPROC)nelglGetProcAddress("glMultiTexCoord4dARB")))return false;

	if(!(nglMultiTexCoord1svARB=(NEL_PFNGLMULTITEXCOORD1SVARBPROC)nelglGetProcAddress("glMultiTexCoord1svARB")))return false;
	if(!(nglMultiTexCoord1ivARB=(NEL_PFNGLMULTITEXCOORD1IVARBPROC)nelglGetProcAddress("glMultiTexCoord1ivARB")))return false;
	if(!(nglMultiTexCoord1fvARB=(NEL_PFNGLMULTITEXCOORD1FVARBPROC)nelglGetProcAddress("glMultiTexCoord1fvARB")))return false;
	if(!(nglMultiTexCoord1dvARB=(NEL_PFNGLMULTITEXCOORD1DVARBPROC)nelglGetProcAddress("glMultiTexCoord1dvARB")))return false;
	if(!(nglMultiTexCoord2svARB=(NEL_PFNGLMULTITEXCOORD2SVARBPROC)nelglGetProcAddress("glMultiTexCoord2svARB")))return false;
	if(!(nglMultiTexCoord2ivARB=(NEL_PFNGLMULTITEXCOORD2IVARBPROC)nelglGetProcAddress("glMultiTexCoord2ivARB")))return false;
	if(!(nglMultiTexCoord2fvARB=(NEL_PFNGLMULTITEXCOORD2FVARBPROC)nelglGetProcAddress("glMultiTexCoord2fvARB")))return false;
	if(!(nglMultiTexCoord2dvARB=(NEL_PFNGLMULTITEXCOORD2DVARBPROC)nelglGetProcAddress("glMultiTexCoord2dvARB")))return false;
	if(!(nglMultiTexCoord3svARB=(NEL_PFNGLMULTITEXCOORD3SVARBPROC)nelglGetProcAddress("glMultiTexCoord3svARB")))return false;
	if(!(nglMultiTexCoord3ivARB=(NEL_PFNGLMULTITEXCOORD3IVARBPROC)nelglGetProcAddress("glMultiTexCoord3ivARB")))return false;
	if(!(nglMultiTexCoord3fvARB=(NEL_PFNGLMULTITEXCOORD3FVARBPROC)nelglGetProcAddress("glMultiTexCoord3fvARB")))return false;
	if(!(nglMultiTexCoord3dvARB=(NEL_PFNGLMULTITEXCOORD3DVARBPROC)nelglGetProcAddress("glMultiTexCoord3dvARB")))return false;
	if(!(nglMultiTexCoord4svARB=(NEL_PFNGLMULTITEXCOORD4SVARBPROC)nelglGetProcAddress("glMultiTexCoord4svARB")))return false;
	if(!(nglMultiTexCoord4ivARB=(NEL_PFNGLMULTITEXCOORD4IVARBPROC)nelglGetProcAddress("glMultiTexCoord4ivARB")))return false;
	if(!(nglMultiTexCoord4fvARB=(NEL_PFNGLMULTITEXCOORD4FVARBPROC)nelglGetProcAddress("glMultiTexCoord4fvARB")))return false;
	if(!(nglMultiTexCoord4dvARB=(NEL_PFNGLMULTITEXCOORD4DVARBPROC)nelglGetProcAddress("glMultiTexCoord4dvARB")))return false;

	return true;
}


// *********************************
static bool setupEXTTextureEnvCombine(const char	*glext)
{
	if(strstr(glext, "GL_EXT_texture_env_combine")==NULL)
		return false;

	return true;
}


// *********************************
static bool	setupARBTextureCompression(const char	*glext)
{
	if(strstr(glext, "GL_ARB_texture_compression")==NULL)
	  return false;

	if(!(nglCompressedTexImage3DARB=(NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)nelglGetProcAddress("glCompressedTexImage3DARB")))return false;
	if(!(nglCompressedTexImage2DARB=(NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)nelglGetProcAddress("glCompressedTexImage2DARB")))return false;
	if(!(nglCompressedTexImage1DARB=(NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)nelglGetProcAddress("glCompressedTexImage1DARB")))return false;
	if(!(nglCompressedTexSubImage3DARB=(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)nelglGetProcAddress("glCompressedTexSubImage3DARB")))return false;
	if(!(nglCompressedTexSubImage2DARB=(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)nelglGetProcAddress("glCompressedTexSubImage2DARB")))return false;
	if(!(nglCompressedTexSubImage1DARB=(NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)nelglGetProcAddress("glCompressedTexSubImage1DARB")))return false;
	if(!(nglGetCompressedTexImageARB=(NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)nelglGetProcAddress("glGetCompressedTexImageARB")))return false;

	return true;
}


// *********************************
static bool	setupNVVertexArrayRange(const char	*glext)
{
	// \todo_yoyo: TODO_LINUX: for now, just return false.
#ifndef NL_OS_WINDOWS
	return false;
#endif


	// Test if VAR is present.
	if(strstr(glext, "GL_NV_vertex_array_range")==NULL)
		return false;
	// Tess Fence too.
	if( strstr(glext, "GL_NV_fence")==NULL )
		return false;


	// Get VAR adress.
	if(!(nglFlushVertexArrayRangeNV=(NEL_PFNGLFLUSHVERTEXARRAYRANGENVPROC)nelglGetProcAddress("glFlushVertexArrayRangeNV")))return false;
	if(!(nglVertexArrayRangeNV=(NEL_PFNGLVERTEXARRAYRANGENVPROC)nelglGetProcAddress("glVertexArrayRangeNV")))return false;
#ifdef NL_OS_WINDOWS
	if(!(wglAllocateMemoryNV= (PFNWGLALLOCATEMEMORYNVPROC)nelglGetProcAddress("wglAllocateMemoryNV")))return false;
	if(!(wglFreeMemoryNV= (PFNWGLFREEMEMORYNVPROC)nelglGetProcAddress("wglFreeMemoryNV")))return false;
#endif

	// Get fence adress.
	if(!(nglDeleteFencesNV= (NEL_PFNGLDELETEFENCESNVPROC)nelglGetProcAddress("glDeleteFencesNV")))return false;
	if(!(nglGenFencesNV= (NEL_PFNGLGENFENCESNVPROC)nelglGetProcAddress("glGenFencesNV")))return false;
	if(!(nglIsFenceNV= (NEL_PFNGLISFENCENVPROC)nelglGetProcAddress("glIsFenceNV")))return false;
	if(!(nglTestFenceNV= (NEL_PFNGLTESTFENCENVPROC)nelglGetProcAddress("glTestFenceNV")))return false;
	if(!(nglGetFenceivNV= (NEL_PFNGLGETFENCEIVNVPROC)nelglGetProcAddress("glGetFenceivNV")))return false;
	if(!(nglFinishFenceNV= (NEL_PFNGLFINISHFENCENVPROC)nelglGetProcAddress("glFinishFenceNV")))return false;
	if(!(nglSetFenceNV= (NEL_PFNGLSETFENCENVPROC)nelglGetProcAddress("glSetFenceNV")))return false;


	return true;
}


// *********************************
static bool	setupEXTTextureCompressionS3TC(const char	*glext)
{
	if(strstr(glext, "GL_EXT_texture_compression_s3tc")==NULL)
		return false;

	return true;
}


// *********************************
static bool	setupEXTVertexWeighting(const char	*glext)
{
	if(strstr (glext, "GL_EXT_vertex_weighting")==NULL)
		return false;


	if(!(nglVertexWeightfEXT=(NEL_PFNGLVERTEXWEIGHTFEXTPROC)nelglGetProcAddress("glVertexWeightfEXT")))return false;
	if(!(nglVertexWeightfvEXT=(NEL_PFNGLVERTEXWEIGHTFVEXTPROC)nelglGetProcAddress("glVertexWeightfvEXT")))return false;
	if(!(nglVertexWeightPointerEXT=(NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC)nelglGetProcAddress("glVertexWeightPointerEXT")))return false;

	return true;
}


// *********************************
static bool	setupEXTSeparateSpecularColor(const char	*glext)
{
	if(strstr(glext, "GL_EXT_separate_specular_color")==NULL)
		return false;

	return true;
}


// *********************************
static bool	setupNVTextureEnvCombine4(const char	*glext)
{
	if(strstr(glext, "GL_NV_texture_env_combine4")==NULL)
		return false;

	return true;
}

// *********************************
static bool	setupARBTextureCubeMap(const char	*glext)
{
	if(strstr(glext, "GL_ARB_texture_cube_map")==NULL)
		return false;

	return true;
}


// *********************************
static bool	setupNVVertexProgram(const char	*glext)
{	
	if(strstr(glext, "GL_NV_vertex_program")==NULL)
		return false;


	if(!(nglAreProgramsResidentNV= (NEL_PFNGLAREPROGRAMSRESIDENTNVPROC)nelglGetProcAddress("glAreProgramsResidentNV"))) return false;
	if(!(nglBindProgramNV= (NEL_PFNGLBINDPROGRAMNVPROC)nelglGetProcAddress("glBindProgramNV"))) return false;
	if(!(nglDeleteProgramsNV= (NEL_PFNGLDELETEPROGRAMSNVPROC)nelglGetProcAddress("glDeleteProgramsNV"))) return false;
	if(!(nglExecuteProgramNV= (NEL_PFNGLEXECUTEPROGRAMNVPROC)nelglGetProcAddress("glExecuteProgramNV"))) return false;
	if(!(nglGenProgramsNV= (NEL_PFNGLGENPROGRAMSNVPROC)nelglGetProcAddress("glGenProgramsNV"))) return false;
	if(!(nglGetProgramParameterdvNV= (NEL_PFNGLGETPROGRAMPARAMETERDVNVPROC)nelglGetProcAddress("glGetProgramParameterdvNV"))) return false;
	if(!(nglGetProgramParameterfvNV= (NEL_PFNGLGETPROGRAMPARAMETERFVNVPROC)nelglGetProcAddress("glGetProgramParameterfvNV"))) return false;
	if(!(nglGetProgramivNV= (NEL_PFNGLGETPROGRAMIVNVPROC)nelglGetProcAddress("glGetProgramivNV"))) return false;
	if(!(nglGetProgramStringNV= (NEL_PFNGLGETPROGRAMSTRINGNVPROC)nelglGetProcAddress("glGetProgramStringNV"))) return false;
	if(!(nglGetTrackMatrixivNV= (NEL_PFNGLGETTRACKMATRIXIVNVPROC)nelglGetProcAddress("glGetTrackMatrixivNV"))) return false;
	if(!(nglGetVertexAttribdvNV= (NEL_PFNGLGETVERTEXATTRIBDVNVPROC)nelglGetProcAddress("glGetVertexAttribdvNV"))) return false;
	if(!(nglGetVertexAttribfvNV= (NEL_PFNGLGETVERTEXATTRIBFVNVPROC)nelglGetProcAddress("glGetVertexAttribfvNV"))) return false;
	if(!(nglGetVertexAttribivNV= (NEL_PFNGLGETVERTEXATTRIBIVNVPROC)nelglGetProcAddress("glGetVertexAttribivNV"))) return false;
	if(!(nglGetVertexAttribPointervNV= (NEL_PFNGLGETVERTEXATTRIBPOINTERVNVPROC)nelglGetProcAddress("glGetVertexAttribPointervNV"))) return false;
	if(!(nglIsProgramNV= (NEL_PFNGLISPROGRAMNVPROC)nelglGetProcAddress("glIsProgramNV"))) return false;
	if(!(nglLoadProgramNV= (NEL_PFNGLLOADPROGRAMNVPROC)nelglGetProcAddress("glLoadProgramNV"))) return false;
	if(!(nglProgramParameter4dNV= (NEL_PFNGLPROGRAMPARAMETER4DNVPROC)nelglGetProcAddress("glProgramParameter4dNV"))) return false;
	if(!(nglProgramParameter4dvNV= (NEL_PFNGLPROGRAMPARAMETER4DVNVPROC)nelglGetProcAddress("glProgramParameter4dvNV"))) return false;
	if(!(nglProgramParameter4fNV= (NEL_PFNGLPROGRAMPARAMETER4FNVPROC)nelglGetProcAddress("glProgramParameter4fNV"))) return false;
	if(!(nglProgramParameter4fvNV= (NEL_PFNGLPROGRAMPARAMETER4FVNVPROC)nelglGetProcAddress("glProgramParameter4fvNV"))) return false;
	if(!(nglProgramParameters4dvNV= (NEL_PFNGLPROGRAMPARAMETERS4DVNVPROC)nelglGetProcAddress("glProgramParameters4dvNV"))) return false;
	if(!(nglProgramParameters4fvNV= (NEL_PFNGLPROGRAMPARAMETERS4FVNVPROC)nelglGetProcAddress("glProgramParameters4fvNV"))) return false;
	if(!(nglRequestResidentProgramsNV= (NEL_PFNGLREQUESTRESIDENTPROGRAMSNVPROC)nelglGetProcAddress("glRequestResidentProgramsNV"))) return false;
	if(!(nglTrackMatrixNV= (NEL_PFNGLTRACKMATRIXNVPROC)nelglGetProcAddress("glTrackMatrixNV"))) return false;
	if(!(nglVertexAttribPointerNV= (NEL_PFNGLVERTEXATTRIBPOINTERNVPROC)nelglGetProcAddress("glVertexAttribPointerNV"))) return false;
	if(!(nglVertexAttrib1dNV= (NEL_PFNGLVERTEXATTRIB1DNVPROC)nelglGetProcAddress("glVertexAttrib1dNV"))) return false;
	if(!(nglVertexAttrib1dvNV= (NEL_PFNGLVERTEXATTRIB1DVNVPROC)nelglGetProcAddress("glVertexAttrib1dvNV"))) return false;
	if(!(nglVertexAttrib1fNV= (NEL_PFNGLVERTEXATTRIB1FNVPROC)nelglGetProcAddress("glVertexAttrib1fNV"))) return false;
	if(!(nglVertexAttrib1fvNV= (NEL_PFNGLVERTEXATTRIB1FVNVPROC)nelglGetProcAddress("glVertexAttrib1fvNV"))) return false;
	if(!(nglVertexAttrib1sNV= (NEL_PFNGLVERTEXATTRIB1SNVPROC)nelglGetProcAddress("glVertexAttrib1sNV"))) return false;
	if(!(nglVertexAttrib1svNV= (NEL_PFNGLVERTEXATTRIB1SVNVPROC)nelglGetProcAddress("glVertexAttrib1svNV"))) return false;
	if(!(nglVertexAttrib2dNV= (NEL_PFNGLVERTEXATTRIB2DNVPROC)nelglGetProcAddress("glVertexAttrib2dNV"))) return false;
	if(!(nglVertexAttrib2dvNV= (NEL_PFNGLVERTEXATTRIB2DVNVPROC)nelglGetProcAddress("glVertexAttrib2dvNV"))) return false;
	if(!(nglVertexAttrib2fNV= (NEL_PFNGLVERTEXATTRIB2FNVPROC)nelglGetProcAddress("glVertexAttrib2fNV"))) return false;
	if(!(nglVertexAttrib2fvNV= (NEL_PFNGLVERTEXATTRIB2FVNVPROC)nelglGetProcAddress("glVertexAttrib2fvNV"))) return false;
	if(!(nglVertexAttrib2sNV= (NEL_PFNGLVERTEXATTRIB2SNVPROC)nelglGetProcAddress("glVertexAttrib2sNV"))) return false;
	if(!(nglVertexAttrib2svNV= (NEL_PFNGLVERTEXATTRIB2SVNVPROC)nelglGetProcAddress("glVertexAttrib2svNV"))) return false;
	if(!(nglVertexAttrib3dNV= (NEL_PFNGLVERTEXATTRIB3DNVPROC)nelglGetProcAddress("glVertexAttrib3dNV"))) return false;
	if(!(nglVertexAttrib3dvNV= (NEL_PFNGLVERTEXATTRIB3DVNVPROC)nelglGetProcAddress("glVertexAttrib3dvNV"))) return false;
	if(!(nglVertexAttrib3fNV= (NEL_PFNGLVERTEXATTRIB3FNVPROC)nelglGetProcAddress("glVertexAttrib3fNV"))) return false;
	if(!(nglVertexAttrib3fvNV= (NEL_PFNGLVERTEXATTRIB3FVNVPROC)nelglGetProcAddress("glVertexAttrib3fvNV"))) return false;
	if(!(nglVertexAttrib3sNV= (NEL_PFNGLVERTEXATTRIB3SNVPROC)nelglGetProcAddress("glVertexAttrib3sNV"))) return false;
	if(!(nglVertexAttrib3svNV= (NEL_PFNGLVERTEXATTRIB3SVNVPROC)nelglGetProcAddress("glVertexAttrib3svNV"))) return false;
	if(!(nglVertexAttrib4dNV= (NEL_PFNGLVERTEXATTRIB4DNVPROC)nelglGetProcAddress("glVertexAttrib4dNV"))) return false;
	if(!(nglVertexAttrib4dvNV= (NEL_PFNGLVERTEXATTRIB4DVNVPROC)nelglGetProcAddress("glVertexAttrib4dvNV"))) return false;
	if(!(nglVertexAttrib4fNV= (NEL_PFNGLVERTEXATTRIB4FNVPROC)nelglGetProcAddress("glVertexAttrib4fNV"))) return false;
	if(!(nglVertexAttrib4fvNV= (NEL_PFNGLVERTEXATTRIB4FVNVPROC)nelglGetProcAddress("glVertexAttrib4fvNV"))) return false;
	if(!(nglVertexAttrib4sNV= (NEL_PFNGLVERTEXATTRIB4SNVPROC)nelglGetProcAddress("glVertexAttrib4sNV"))) return false;
	if(!(nglVertexAttrib4svNV= (NEL_PFNGLVERTEXATTRIB4SVNVPROC)nelglGetProcAddress("glVertexAttrib4svNV"))) return false;
	if(!(nglVertexAttrib4ubvNV= (NEL_PFNGLVERTEXATTRIB4UBVNVPROC)nelglGetProcAddress("glVertexAttrib4ubvNV"))) return false;
	if(!(nglVertexAttribs1dvNV= (NEL_PFNGLVERTEXATTRIBS1DVNVPROC)nelglGetProcAddress("glVertexAttribs1dvNV"))) return false;
	if(!(nglVertexAttribs1fvNV= (NEL_PFNGLVERTEXATTRIBS1FVNVPROC)nelglGetProcAddress("glVertexAttribs1fvNV"))) return false;
	if(!(nglVertexAttribs1svNV= (NEL_PFNGLVERTEXATTRIBS1SVNVPROC)nelglGetProcAddress("glVertexAttribs1svNV"))) return false;
	if(!(nglVertexAttribs2dvNV= (NEL_PFNGLVERTEXATTRIBS2DVNVPROC)nelglGetProcAddress("glVertexAttribs2dvNV"))) return false;
	if(!(nglVertexAttribs2fvNV= (NEL_PFNGLVERTEXATTRIBS2FVNVPROC)nelglGetProcAddress("glVertexAttribs2fvNV"))) return false;
	if(!(nglVertexAttribs2svNV= (NEL_PFNGLVERTEXATTRIBS2SVNVPROC)nelglGetProcAddress("glVertexAttribs2svNV"))) return false;
	if(!(nglVertexAttribs3dvNV= (NEL_PFNGLVERTEXATTRIBS3DVNVPROC)nelglGetProcAddress("glVertexAttribs3dvNV"))) return false;
	if(!(nglVertexAttribs3fvNV= (NEL_PFNGLVERTEXATTRIBS3FVNVPROC)nelglGetProcAddress("glVertexAttribs3fvNV"))) return false;
	if(!(nglVertexAttribs3svNV= (NEL_PFNGLVERTEXATTRIBS3SVNVPROC)nelglGetProcAddress("glVertexAttribs3svNV"))) return false;
	if(!(nglVertexAttribs4dvNV= (NEL_PFNGLVERTEXATTRIBS4DVNVPROC)nelglGetProcAddress("glVertexAttribs4dvNV"))) return false;
	if(!(nglVertexAttribs4fvNV= (NEL_PFNGLVERTEXATTRIBS4FVNVPROC)nelglGetProcAddress("glVertexAttribs4fvNV"))) return false;
	if(!(nglVertexAttribs4svNV= (NEL_PFNGLVERTEXATTRIBS4SVNVPROC)nelglGetProcAddress("glVertexAttribs4svNV"))) return false;
	if(!(nglVertexAttribs4ubvNV= (NEL_PFNGLVERTEXATTRIBS4UBVNVPROC)nelglGetProcAddress("glVertexAttribs4ubvNV"))) return false;


	return true;
}


// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	if(strstr(glext, "GL_EXT_secondary_color")==NULL)
		return false;


	if(!(nglSecondaryColor3bEXT= (NEL_PFNGLSECONDARYCOLOR3BEXTPROC)nelglGetProcAddress("glSecondaryColor3bEXT"))) return false;
	if(!(nglSecondaryColor3bvEXT= (NEL_PFNGLSECONDARYCOLOR3BVEXTPROC)nelglGetProcAddress("glSecondaryColor3bvEXT"))) return false;
	if(!(nglSecondaryColor3dEXT= (NEL_PFNGLSECONDARYCOLOR3DEXTPROC)nelglGetProcAddress("glSecondaryColor3dEXT"))) return false;
	if(!(nglSecondaryColor3dvEXT= (NEL_PFNGLSECONDARYCOLOR3DVEXTPROC)nelglGetProcAddress("glSecondaryColor3dvEXT"))) return false;
	if(!(nglSecondaryColor3fEXT= (NEL_PFNGLSECONDARYCOLOR3FEXTPROC)nelglGetProcAddress("glSecondaryColor3fEXT"))) return false;
	if(!(nglSecondaryColor3fvEXT= (NEL_PFNGLSECONDARYCOLOR3FVEXTPROC)nelglGetProcAddress("glSecondaryColor3fvEXT"))) return false;
	if(!(nglSecondaryColor3iEXT= (NEL_PFNGLSECONDARYCOLOR3IEXTPROC)nelglGetProcAddress("glSecondaryColor3iEXT"))) return false;
	if(!(nglSecondaryColor3ivEXT= (NEL_PFNGLSECONDARYCOLOR3IVEXTPROC)nelglGetProcAddress("glSecondaryColor3ivEXT"))) return false;
	if(!(nglSecondaryColor3sEXT= (NEL_PFNGLSECONDARYCOLOR3SEXTPROC)nelglGetProcAddress("glSecondaryColor3sEXT"))) return false;
	if(!(nglSecondaryColor3svEXT= (NEL_PFNGLSECONDARYCOLOR3SVEXTPROC)nelglGetProcAddress("glSecondaryColor3svEXT"))) return false;
	if(!(nglSecondaryColor3ubEXT= (NEL_PFNGLSECONDARYCOLOR3UBEXTPROC)nelglGetProcAddress("glSecondaryColor3ubEXT"))) return false;
	if(!(nglSecondaryColor3ubvEXT= (NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC)nelglGetProcAddress("glSecondaryColor3ubvEXT"))) return false;
	if(!(nglSecondaryColor3uiEXT= (NEL_PFNGLSECONDARYCOLOR3UIEXTPROC)nelglGetProcAddress("glSecondaryColor3uiEXT"))) return false;
	if(!(nglSecondaryColor3uivEXT= (NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC)nelglGetProcAddress("glSecondaryColor3uivEXT"))) return false;
	if(!(nglSecondaryColor3usEXT= (NEL_PFNGLSECONDARYCOLOR3USEXTPROC)nelglGetProcAddress("glSecondaryColor3usEXT"))) return false;
	if(!(nglSecondaryColor3usvEXT= (NEL_PFNGLSECONDARYCOLOR3USVEXTPROC)nelglGetProcAddress("glSecondaryColor3usvEXT"))) return false;
	if(!(nglSecondaryColorPointerEXT= (NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC)nelglGetProcAddress("glSecondaryColorPointerEXT"))) return false;


	return true;
}

// *********************************
static bool	setupWGLARBPBuffer(const char	*glext)
{
	if(strstr(glext, "WGL_ARB_pbuffer")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(wglCreatePbufferARB= (PFNWGLCREATEPBUFFERARBPROC)nelglGetProcAddress("wglCreatePbufferARB"))) return false;
	if(!(wglGetPbufferDCARB= (PFNWGLGETPUFFERDCARBPROC)nelglGetProcAddress("wglGetPbufferDCARB"))) return false;
	if(!(wglReleasePbufferDCARB= (PFNWGLRELEASEPUFFERDCARBPROC)nelglGetProcAddress("wglReleasePbufferDCARB"))) return false;
	if(!(wglDestroyPbufferARB= (PFNWGLDESTROYPUFFERARBPROC)nelglGetProcAddress("wglDestroyPbufferARB"))) return false;
	if(!(wglQueryPbufferARB= (PFNWGLQUERYPBUFFERARBPROC)nelglGetProcAddress("wglQueryPbufferARB"))) return false;
#endif

	return true;
}

// *********************************
static bool	setupWGLARBPixelFormat (const char	*glext)
{
	if(strstr(glext, "WGL_ARB_pixel_format")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(wglGetPixelFormatAttribivARB= (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)nelglGetProcAddress("wglGetPixelFormatAttribivARB"))) return false;
	if(!(wglGetPixelFormatAttribfvARB= (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)nelglGetProcAddress("wglGetPixelFormatAttribfvARB"))) return false;
	if(!(wglChoosePixelFormatARB= (PFNWGLCHOOSEPIXELFORMATARBPROC)nelglGetProcAddress("wglChoosePixelFormatARB"))) return false;
#endif

	return true;
}

// *********************************
static bool	setupNVTextureShader(const char	*glext)
{
	if(strstr(glext, "GL_NV_texture_shader")==NULL)
		return false;

	return true;
}


// ***************************************************************************
// Extension Check.
void	registerGlExtensions(CGlExtensions &ext)
{
	// OpenGL 1.2 ??
	const char	*glVersion= (const char *) glGetString (GL_VERSION);
	sint	a=0, b=0;
	// 1.2***  ???
	sscanf(glVersion, "%d.%d", &a, &b);
	ext.Version1_2= a>=1 && b>=2;


	// Extensions.
	const char	*glext= (const char*)glGetString(GL_EXTENSIONS);
	GLint	ntext;

	nldebug("GLExt: %s", glext);


	// Check ARBMultiTexture
	ext.ARBMultiTexture= setupARBMultiTexture(glext);
	if(ext.ARBMultiTexture)
	{
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &ntext);
		// We could have more than IDRV_MAT_MAXTEXTURES but the interface only 
		// support IDRV_MAT_MAXTEXTURES texture stages so take min
		ext.NbTextureStages= (ntext<IDRV_MAT_MAXTEXTURES?ntext:IDRV_MAT_MAXTEXTURES);
	}

	// Check EXTTextureEnvCombine
	ext.EXTTextureEnvCombine= setupEXTTextureEnvCombine(glext);

	// Check ARBTextureCompression
	ext.ARBTextureCompression= setupARBTextureCompression(glext);

	// Check NVVertexArrayRange
	// Disable feature ???
	if(!ext.DisableHardwareVertexArrayAGP)
		ext.NVVertexArrayRange= setupNVVertexArrayRange(glext);
	if(ext.NVVertexArrayRange)
	{
		GLint	nverts;
		glGetIntegerv((GLenum)GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV, &nverts);
		ext.NVVertexArrayRangeMaxVertex= nverts;
	}

	// Compression S3TC OK iff ARBTextureCompression.
	ext.EXTTextureCompressionS3TC= (ext.ARBTextureCompression && setupEXTTextureCompressionS3TC(glext)); 

	// Check if NVidia GL_EXT_vertex_weighting is available.
	ext.EXTVertexWeighting= setupEXTVertexWeighting(glext);

	// Check EXTSeparateSpecularColor.
	ext.EXTSeparateSpecularColor= setupEXTSeparateSpecularColor(glext);

	// Check NVTextureEnvCombine4.
	ext.NVTextureEnvCombine4= setupNVTextureEnvCombine4(glext);

	// Check for cube mapping
	ext.ARBTextureCubeMap = setupARBTextureCubeMap(glext);

	// Check vertex program
	// Disable feature ???
	if(!ext.DisableHardwareVertexProgram)
		ext.NVVertexProgram = setupNVVertexProgram(glext);

	// Check texture shaders
	// Disable feature ???
	if(!ext.DisableHardwareTextureShader)
		ext.NVTextureShader = setupNVTextureShader(glext);

	// For now, the only way to know if emulation, is to test some extension which exist only on GeForce3.
	// if GL_NV_texture_shader is not here, then we are not on GeForce3.
	ext.NVVertexProgramEmulated= ext.NVVertexProgram && (strstr(glext, "GL_NV_texture_shader")==NULL);

	// Check EXTSecondaryColor
	ext.EXTSecondaryColor= setupEXTSecondaryColor(glext);
}


#ifdef NL_OS_WINDOWS
// ***************************************************************************
void	registerWGlExtensions(CGlExtensions &ext, HDC hDC)
{
	// Get proc adress
	if(!(wglGetExtensionsStringARB=(PFNWGFGETEXTENSIONSSTRINGARB)nelglGetProcAddress("wglGetExtensionsStringARB")))
	{
		nlwarning ("wglGetExtensionsStringARB not supported");
		return;
	}

	// Get extension string
	const char *glext = wglGetExtensionsStringARB (hDC);
	if (glext == NULL)
	{
		nlwarning ("wglGetExtensionsStringARB failed");
		return;
	}

	nldebug("WGLExt: %s", glext);

	// Check for pbuffer
	ext.WGLARBPBuffer= setupWGLARBPBuffer(glext);

	// Check for pixel format
	ext.WGLARBPixelFormat= setupWGLARBPixelFormat(glext);
}
#endif // NL_OS_WINDOWS


}




