/** \file driver_opengl_extension.cpp
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.cpp,v 1.29 2002/02/11 10:01:34 berenguier Exp $
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
#define	nelglGetProcAddress glXGetProcAddressARB
#endif	// NL_OS_WINDOWS


// ***************************************************************************
// The exported function names

// ARB_multitexture
//=================
PFNGLACTIVETEXTUREARBPROC nglActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC nglClientActiveTextureARB;

PFNGLMULTITEXCOORD1SARBPROC nglMultiTexCoord1sARB;
PFNGLMULTITEXCOORD1IARBPROC nglMultiTexCoord1iARB;
PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
PFNGLMULTITEXCOORD1DARBPROC nglMultiTexCoord1dARB;
PFNGLMULTITEXCOORD2SARBPROC nglMultiTexCoord2sARB;
PFNGLMULTITEXCOORD2IARBPROC nglMultiTexCoord2iARB;
PFNGLMULTITEXCOORD2FARBPROC nglMultiTexCoord2fARB;
PFNGLMULTITEXCOORD2DARBPROC nglMultiTexCoord2dARB;
PFNGLMULTITEXCOORD3SARBPROC nglMultiTexCoord3sARB;
PFNGLMULTITEXCOORD3IARBPROC nglMultiTexCoord3iARB;
PFNGLMULTITEXCOORD3FARBPROC nglMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3DARBPROC nglMultiTexCoord3dARB;
PFNGLMULTITEXCOORD4SARBPROC nglMultiTexCoord4sARB;
PFNGLMULTITEXCOORD4IARBPROC nglMultiTexCoord4iARB;
PFNGLMULTITEXCOORD4FARBPROC nglMultiTexCoord4fARB;
PFNGLMULTITEXCOORD4DARBPROC nglMultiTexCoord4dARB;

PFNGLMULTITEXCOORD1SVARBPROC nglMultiTexCoord1svARB;
PFNGLMULTITEXCOORD1IVARBPROC nglMultiTexCoord1ivARB;
PFNGLMULTITEXCOORD1FVARBPROC nglMultiTexCoord1fvARB;
PFNGLMULTITEXCOORD1DVARBPROC nglMultiTexCoord1dvARB;
PFNGLMULTITEXCOORD2SVARBPROC nglMultiTexCoord2svARB;
PFNGLMULTITEXCOORD2IVARBPROC nglMultiTexCoord2ivARB;
PFNGLMULTITEXCOORD2FVARBPROC nglMultiTexCoord2fvARB;
PFNGLMULTITEXCOORD2DVARBPROC nglMultiTexCoord2dvARB;
PFNGLMULTITEXCOORD3SVARBPROC nglMultiTexCoord3svARB;
PFNGLMULTITEXCOORD3IVARBPROC nglMultiTexCoord3ivARB;
PFNGLMULTITEXCOORD3FVARBPROC nglMultiTexCoord3fvARB;
PFNGLMULTITEXCOORD3DVARBPROC nglMultiTexCoord3dvARB;
PFNGLMULTITEXCOORD4SVARBPROC nglMultiTexCoord4svARB;
PFNGLMULTITEXCOORD4IVARBPROC nglMultiTexCoord4ivARB;
PFNGLMULTITEXCOORD4FVARBPROC nglMultiTexCoord4fvARB;
PFNGLMULTITEXCOORD4DVARBPROC nglMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC	nglCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC	nglCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC	nglCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	nglCompressedTexSubImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	nglCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	nglCompressedTexSubImage1DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	nglGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
PFNGLFLUSHVERTEXARRAYRANGENVPROC	nglFlushVertexArrayRangeNV;
PFNGLVERTEXARRAYRANGENVPROC			nglVertexArrayRangeNV;
#ifdef NL_OS_WINDOWS
PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;
#endif


// FenceNV.
//====================
PFNGLDELETEFENCESNVPROC				nglDeleteFencesNV;
PFNGLGENFENCESNVPROC				nglGenFencesNV;
PFNGLISFENCENVPROC					nglIsFenceNV;
PFNGLTESTFENCENVPROC				nglTestFenceNV;
PFNGLGETFENCEIVNVPROC				nglGetFenceivNV;
PFNGLFINISHFENCENVPROC				nglFinishFenceNV;
PFNGLSETFENCENVPROC					nglSetFenceNV;


// VertexWeighting.
//====================
PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
PFNGLAREPROGRAMSRESIDENTNVPROC		nglAreProgramsResidentNV;
PFNGLBINDPROGRAMNVPROC				nglBindProgramNV;
PFNGLDELETEPROGRAMSNVPROC			nglDeleteProgramsNV;
PFNGLEXECUTEPROGRAMNVPROC			nglExecuteProgramNV;
PFNGLGENPROGRAMSNVPROC				nglGenProgramsNV;
PFNGLGETPROGRAMPARAMETERDVNVPROC	nglGetProgramParameterdvNV;
PFNGLGETPROGRAMPARAMETERFVNVPROC	nglGetProgramParameterfvNV;
PFNGLGETPROGRAMIVNVPROC				nglGetProgramivNV;
PFNGLGETPROGRAMSTRINGNVPROC			nglGetProgramStringNV;
PFNGLGETTRACKMATRIXIVNVPROC			nglGetTrackMatrixivNV;
PFNGLGETVERTEXATTRIBDVNVPROC		nglGetVertexAttribdvNV;
PFNGLGETVERTEXATTRIBFVNVPROC		nglGetVertexAttribfvNV;
PFNGLGETVERTEXATTRIBIVNVPROC		nglGetVertexAttribivNV;
PFNGLGETVERTEXATTRIBPOINTERVNVPROC	nglGetVertexAttribPointervNV;
PFNGLISPROGRAMNVPROC				nglIsProgramNV;
PFNGLLOADPROGRAMNVPROC				nglLoadProgramNV;
PFNGLPROGRAMPARAMETER4DNVPROC		nglProgramParameter4dNV;
PFNGLPROGRAMPARAMETER4DVNVPROC		nglProgramParameter4dvNV;
PFNGLPROGRAMPARAMETER4FNVPROC		nglProgramParameter4fNV;
PFNGLPROGRAMPARAMETER4FVNVPROC		nglProgramParameter4fvNV;
PFNGLPROGRAMPARAMETERS4DVNVPROC		nglProgramParameters4dvNV;
PFNGLPROGRAMPARAMETERS4FVNVPROC		nglProgramParameters4fvNV;
PFNGLREQUESTRESIDENTPROGRAMSNVPROC	nglRequestResidentProgramsNV;
PFNGLTRACKMATRIXNVPROC				nglTrackMatrixNV;
PFNGLVERTEXATTRIBPOINTERNVPROC		nglVertexAttribPointerNV;
PFNGLVERTEXATTRIB1DNVPROC			nglVertexAttrib1dNV;
PFNGLVERTEXATTRIB1DVNVPROC			nglVertexAttrib1dvNV;
PFNGLVERTEXATTRIB1FNVPROC			nglVertexAttrib1fNV;
PFNGLVERTEXATTRIB1FVNVPROC			nglVertexAttrib1fvNV;
PFNGLVERTEXATTRIB1SNVPROC			nglVertexAttrib1sNV;
PFNGLVERTEXATTRIB1SVNVPROC			nglVertexAttrib1svNV;
PFNGLVERTEXATTRIB2DNVPROC			nglVertexAttrib2dNV;
PFNGLVERTEXATTRIB2DVNVPROC			nglVertexAttrib2dvNV;
PFNGLVERTEXATTRIB2FNVPROC			nglVertexAttrib2fNV;
PFNGLVERTEXATTRIB2FVNVPROC			nglVertexAttrib2fvNV;
PFNGLVERTEXATTRIB2SNVPROC			nglVertexAttrib2sNV;
PFNGLVERTEXATTRIB2SVNVPROC			nglVertexAttrib2svNV;
PFNGLVERTEXATTRIB3DNVPROC			nglVertexAttrib3dNV;
PFNGLVERTEXATTRIB3DVNVPROC			nglVertexAttrib3dvNV;
PFNGLVERTEXATTRIB3FNVPROC			nglVertexAttrib3fNV;
PFNGLVERTEXATTRIB3FVNVPROC			nglVertexAttrib3fvNV;
PFNGLVERTEXATTRIB3SNVPROC			nglVertexAttrib3sNV;
PFNGLVERTEXATTRIB3SVNVPROC			nglVertexAttrib3svNV;
PFNGLVERTEXATTRIB4DNVPROC			nglVertexAttrib4dNV;
PFNGLVERTEXATTRIB4DVNVPROC			nglVertexAttrib4dvNV;
PFNGLVERTEXATTRIB4FNVPROC			nglVertexAttrib4fNV;
PFNGLVERTEXATTRIB4FVNVPROC			nglVertexAttrib4fvNV;
PFNGLVERTEXATTRIB4SNVPROC			nglVertexAttrib4sNV;
PFNGLVERTEXATTRIB4SVNVPROC			nglVertexAttrib4svNV;
PFNGLVERTEXATTRIB4UBVNVPROC			nglVertexAttrib4ubvNV;
PFNGLVERTEXATTRIBS1DVNVPROC			nglVertexAttribs1dvNV;
PFNGLVERTEXATTRIBS1FVNVPROC			nglVertexAttribs1fvNV;
PFNGLVERTEXATTRIBS1SVNVPROC			nglVertexAttribs1svNV;
PFNGLVERTEXATTRIBS2DVNVPROC			nglVertexAttribs2dvNV;
PFNGLVERTEXATTRIBS2FVNVPROC			nglVertexAttribs2fvNV;
PFNGLVERTEXATTRIBS2SVNVPROC			nglVertexAttribs2svNV;
PFNGLVERTEXATTRIBS3DVNVPROC			nglVertexAttribs3dvNV;
PFNGLVERTEXATTRIBS3FVNVPROC			nglVertexAttribs3fvNV;
PFNGLVERTEXATTRIBS3SVNVPROC			nglVertexAttribs3svNV;
PFNGLVERTEXATTRIBS4DVNVPROC			nglVertexAttribs4dvNV;
PFNGLVERTEXATTRIBS4FVNVPROC			nglVertexAttribs4fvNV;
PFNGLVERTEXATTRIBS4SVNVPROC			nglVertexAttribs4svNV;
PFNGLVERTEXATTRIBS4UBVNVPROC		nglVertexAttribs4ubvNV;


// SecondaryColor extension
//========================
PFNGLSECONDARYCOLOR3BEXTPROC		nglSecondaryColor3bEXT;
PFNGLSECONDARYCOLOR3BVEXTPROC		nglSecondaryColor3bvEXT;
PFNGLSECONDARYCOLOR3DEXTPROC		nglSecondaryColor3dEXT;
PFNGLSECONDARYCOLOR3DVEXTPROC		nglSecondaryColor3dvEXT;
PFNGLSECONDARYCOLOR3FEXTPROC		nglSecondaryColor3fEXT;
PFNGLSECONDARYCOLOR3FVEXTPROC		nglSecondaryColor3fvEXT;
PFNGLSECONDARYCOLOR3IEXTPROC		nglSecondaryColor3iEXT;
PFNGLSECONDARYCOLOR3IVEXTPROC		nglSecondaryColor3ivEXT;
PFNGLSECONDARYCOLOR3SEXTPROC		nglSecondaryColor3sEXT;
PFNGLSECONDARYCOLOR3SVEXTPROC		nglSecondaryColor3svEXT;
PFNGLSECONDARYCOLOR3UBEXTPROC		nglSecondaryColor3ubEXT;
PFNGLSECONDARYCOLOR3UBVEXTPROC		nglSecondaryColor3ubvEXT;
PFNGLSECONDARYCOLOR3UIEXTPROC		nglSecondaryColor3uiEXT;
PFNGLSECONDARYCOLOR3UIVEXTPROC		nglSecondaryColor3uivEXT;
PFNGLSECONDARYCOLOR3USEXTPROC		nglSecondaryColor3usEXT;
PFNGLSECONDARYCOLOR3USVEXTPROC		nglSecondaryColor3usvEXT;
PFNGLSECONDARYCOLORPOINTEREXTPROC	nglSecondaryColorPointerEXT;


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

	if(!(nglActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC)nelglGetProcAddress("glActiveTextureARB")))return false;
	if(!(nglClientActiveTextureARB=(PFNGLCLIENTACTIVETEXTUREARBPROC)nelglGetProcAddress("glClientActiveTextureARB")))return false;

	if(!(nglMultiTexCoord1sARB=(PFNGLMULTITEXCOORD1SARBPROC)nelglGetProcAddress("glMultiTexCoord1sARB")))return false;
	if(!(nglMultiTexCoord1iARB=(PFNGLMULTITEXCOORD1IARBPROC)nelglGetProcAddress("glMultiTexCoord1iARB")))return false;
	if(!(nglMultiTexCoord1fARB=(PFNGLMULTITEXCOORD1FARBPROC)nelglGetProcAddress("glMultiTexCoord1fARB")))return false;
	if(!(nglMultiTexCoord1dARB=(PFNGLMULTITEXCOORD1DARBPROC)nelglGetProcAddress("glMultiTexCoord1dARB")))return false;
	if(!(nglMultiTexCoord2sARB=(PFNGLMULTITEXCOORD2SARBPROC)nelglGetProcAddress("glMultiTexCoord2sARB")))return false;
	if(!(nglMultiTexCoord2iARB=(PFNGLMULTITEXCOORD2IARBPROC)nelglGetProcAddress("glMultiTexCoord2iARB")))return false;
	if(!(nglMultiTexCoord2fARB=(PFNGLMULTITEXCOORD2FARBPROC)nelglGetProcAddress("glMultiTexCoord2fARB")))return false;
	if(!(nglMultiTexCoord2dARB=(PFNGLMULTITEXCOORD2DARBPROC)nelglGetProcAddress("glMultiTexCoord2dARB")))return false;
	if(!(nglMultiTexCoord3sARB=(PFNGLMULTITEXCOORD3SARBPROC)nelglGetProcAddress("glMultiTexCoord3sARB")))return false;
	if(!(nglMultiTexCoord3iARB=(PFNGLMULTITEXCOORD3IARBPROC)nelglGetProcAddress("glMultiTexCoord3iARB")))return false;
	if(!(nglMultiTexCoord3fARB=(PFNGLMULTITEXCOORD3FARBPROC)nelglGetProcAddress("glMultiTexCoord3fARB")))return false;
	if(!(nglMultiTexCoord3dARB=(PFNGLMULTITEXCOORD3DARBPROC)nelglGetProcAddress("glMultiTexCoord3dARB")))return false;
	if(!(nglMultiTexCoord4sARB=(PFNGLMULTITEXCOORD4SARBPROC)nelglGetProcAddress("glMultiTexCoord4sARB")))return false;
	if(!(nglMultiTexCoord4iARB=(PFNGLMULTITEXCOORD4IARBPROC)nelglGetProcAddress("glMultiTexCoord4iARB")))return false;
	if(!(nglMultiTexCoord4fARB=(PFNGLMULTITEXCOORD4FARBPROC)nelglGetProcAddress("glMultiTexCoord4fARB")))return false;
	if(!(nglMultiTexCoord4dARB=(PFNGLMULTITEXCOORD4DARBPROC)nelglGetProcAddress("glMultiTexCoord4dARB")))return false;

	if(!(nglMultiTexCoord1svARB=(PFNGLMULTITEXCOORD1SVARBPROC)nelglGetProcAddress("glMultiTexCoord1svARB")))return false;
	if(!(nglMultiTexCoord1ivARB=(PFNGLMULTITEXCOORD1IVARBPROC)nelglGetProcAddress("glMultiTexCoord1ivARB")))return false;
	if(!(nglMultiTexCoord1fvARB=(PFNGLMULTITEXCOORD1FVARBPROC)nelglGetProcAddress("glMultiTexCoord1fvARB")))return false;
	if(!(nglMultiTexCoord1dvARB=(PFNGLMULTITEXCOORD1DVARBPROC)nelglGetProcAddress("glMultiTexCoord1dvARB")))return false;
	if(!(nglMultiTexCoord2svARB=(PFNGLMULTITEXCOORD2SVARBPROC)nelglGetProcAddress("glMultiTexCoord2svARB")))return false;
	if(!(nglMultiTexCoord2ivARB=(PFNGLMULTITEXCOORD2IVARBPROC)nelglGetProcAddress("glMultiTexCoord2ivARB")))return false;
	if(!(nglMultiTexCoord2fvARB=(PFNGLMULTITEXCOORD2FVARBPROC)nelglGetProcAddress("glMultiTexCoord2fvARB")))return false;
	if(!(nglMultiTexCoord2dvARB=(PFNGLMULTITEXCOORD2DVARBPROC)nelglGetProcAddress("glMultiTexCoord2dvARB")))return false;
	if(!(nglMultiTexCoord3svARB=(PFNGLMULTITEXCOORD3SVARBPROC)nelglGetProcAddress("glMultiTexCoord3svARB")))return false;
	if(!(nglMultiTexCoord3ivARB=(PFNGLMULTITEXCOORD3IVARBPROC)nelglGetProcAddress("glMultiTexCoord3ivARB")))return false;
	if(!(nglMultiTexCoord3fvARB=(PFNGLMULTITEXCOORD3FVARBPROC)nelglGetProcAddress("glMultiTexCoord3fvARB")))return false;
	if(!(nglMultiTexCoord3dvARB=(PFNGLMULTITEXCOORD3DVARBPROC)nelglGetProcAddress("glMultiTexCoord3dvARB")))return false;
	if(!(nglMultiTexCoord4svARB=(PFNGLMULTITEXCOORD4SVARBPROC)nelglGetProcAddress("glMultiTexCoord4svARB")))return false;
	if(!(nglMultiTexCoord4ivARB=(PFNGLMULTITEXCOORD4IVARBPROC)nelglGetProcAddress("glMultiTexCoord4ivARB")))return false;
	if(!(nglMultiTexCoord4fvARB=(PFNGLMULTITEXCOORD4FVARBPROC)nelglGetProcAddress("glMultiTexCoord4fvARB")))return false;
	if(!(nglMultiTexCoord4dvARB=(PFNGLMULTITEXCOORD4DVARBPROC)nelglGetProcAddress("glMultiTexCoord4dvARB")))return false;

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

	if(!(nglCompressedTexImage3DARB=(PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)nelglGetProcAddress("glCompressedTexImage3DARB")))return false;
	if(!(nglCompressedTexImage2DARB=(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)nelglGetProcAddress("glCompressedTexImage2DARB")))return false;
	if(!(nglCompressedTexImage1DARB=(PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)nelglGetProcAddress("glCompressedTexImage1DARB")))return false;
	if(!(nglCompressedTexSubImage3DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)nelglGetProcAddress("glCompressedTexSubImage3DARB")))return false;
	if(!(nglCompressedTexSubImage2DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)nelglGetProcAddress("glCompressedTexSubImage2DARB")))return false;
	if(!(nglCompressedTexSubImage1DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)nelglGetProcAddress("glCompressedTexSubImage1DARB")))return false;
	if(!(nglGetCompressedTexImageARB=(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)nelglGetProcAddress("glGetCompressedTexImageARB")))return false;

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
	if(!(nglFlushVertexArrayRangeNV=(PFNGLFLUSHVERTEXARRAYRANGENVPROC)nelglGetProcAddress("glFlushVertexArrayRangeNV")))return false;
	if(!(nglVertexArrayRangeNV=(PFNGLVERTEXARRAYRANGENVPROC)nelglGetProcAddress("glVertexArrayRangeNV")))return false;
#ifdef NL_OS_WINDOWS
	if(!(wglAllocateMemoryNV= (PFNWGLALLOCATEMEMORYNVPROC)nelglGetProcAddress("wglAllocateMemoryNV")))return false;
	if(!(wglFreeMemoryNV= (PFNWGLFREEMEMORYNVPROC)nelglGetProcAddress("wglFreeMemoryNV")))return false;
#endif

	// Get fence adress.
	if(!(nglDeleteFencesNV= (PFNGLDELETEFENCESNVPROC)nelglGetProcAddress("glDeleteFencesNV")))return false;
	if(!(nglGenFencesNV= (PFNGLGENFENCESNVPROC)nelglGetProcAddress("glGenFencesNV")))return false;
	if(!(nglIsFenceNV= (PFNGLISFENCENVPROC)nelglGetProcAddress("glIsFenceNV")))return false;
	if(!(nglTestFenceNV= (PFNGLTESTFENCENVPROC)nelglGetProcAddress("glTestFenceNV")))return false;
	if(!(nglGetFenceivNV= (PFNGLGETFENCEIVNVPROC)nelglGetProcAddress("glGetFenceivNV")))return false;
	if(!(nglFinishFenceNV= (PFNGLFINISHFENCENVPROC)nelglGetProcAddress("glFinishFenceNV")))return false;
	if(!(nglSetFenceNV= (PFNGLSETFENCENVPROC)nelglGetProcAddress("glSetFenceNV")))return false;


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


	if(!(nglVertexWeightfEXT=(PFNGLVERTEXWEIGHTFEXTPROC)nelglGetProcAddress("glVertexWeightfEXT")))return false;
	if(!(nglVertexWeightfvEXT=(PFNGLVERTEXWEIGHTFVEXTPROC)nelglGetProcAddress("glVertexWeightfvEXT")))return false;
	if(!(nglVertexWeightPointerEXT=(PFNGLVERTEXWEIGHTPOINTEREXTPROC)nelglGetProcAddress("glVertexWeightPointerEXT")))return false;

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


	if(!(nglAreProgramsResidentNV= (PFNGLAREPROGRAMSRESIDENTNVPROC)nelglGetProcAddress("glAreProgramsResidentNV"))) return false;
	if(!(nglBindProgramNV= (PFNGLBINDPROGRAMNVPROC)nelglGetProcAddress("glBindProgramNV"))) return false;
	if(!(nglDeleteProgramsNV= (PFNGLDELETEPROGRAMSNVPROC)nelglGetProcAddress("glDeleteProgramsNV"))) return false;
	if(!(nglExecuteProgramNV= (PFNGLEXECUTEPROGRAMNVPROC)nelglGetProcAddress("glExecuteProgramNV"))) return false;
	if(!(nglGenProgramsNV= (PFNGLGENPROGRAMSNVPROC)nelglGetProcAddress("glGenProgramsNV"))) return false;
	if(!(nglGetProgramParameterdvNV= (PFNGLGETPROGRAMPARAMETERDVNVPROC)nelglGetProcAddress("glGetProgramParameterdvNV"))) return false;
	if(!(nglGetProgramParameterfvNV= (PFNGLGETPROGRAMPARAMETERFVNVPROC)nelglGetProcAddress("glGetProgramParameterfvNV"))) return false;
	if(!(nglGetProgramivNV= (PFNGLGETPROGRAMIVNVPROC)nelglGetProcAddress("glGetProgramivNV"))) return false;
	if(!(nglGetProgramStringNV= (PFNGLGETPROGRAMSTRINGNVPROC)nelglGetProcAddress("glGetProgramStringNV"))) return false;
	if(!(nglGetTrackMatrixivNV= (PFNGLGETTRACKMATRIXIVNVPROC)nelglGetProcAddress("glGetTrackMatrixivNV"))) return false;
	if(!(nglGetVertexAttribdvNV= (PFNGLGETVERTEXATTRIBDVNVPROC)nelglGetProcAddress("glGetVertexAttribdvNV"))) return false;
	if(!(nglGetVertexAttribfvNV= (PFNGLGETVERTEXATTRIBFVNVPROC)nelglGetProcAddress("glGetVertexAttribfvNV"))) return false;
	if(!(nglGetVertexAttribivNV= (PFNGLGETVERTEXATTRIBIVNVPROC)nelglGetProcAddress("glGetVertexAttribivNV"))) return false;
	if(!(nglGetVertexAttribPointervNV= (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)nelglGetProcAddress("glGetVertexAttribPointervNV"))) return false;
	if(!(nglIsProgramNV= (PFNGLISPROGRAMNVPROC)nelglGetProcAddress("glIsProgramNV"))) return false;
	if(!(nglLoadProgramNV= (PFNGLLOADPROGRAMNVPROC)nelglGetProcAddress("glLoadProgramNV"))) return false;
	if(!(nglProgramParameter4dNV= (PFNGLPROGRAMPARAMETER4DNVPROC)nelglGetProcAddress("glProgramParameter4dNV"))) return false;
	if(!(nglProgramParameter4dvNV= (PFNGLPROGRAMPARAMETER4DVNVPROC)nelglGetProcAddress("glProgramParameter4dvNV"))) return false;
	if(!(nglProgramParameter4fNV= (PFNGLPROGRAMPARAMETER4FNVPROC)nelglGetProcAddress("glProgramParameter4fNV"))) return false;
	if(!(nglProgramParameter4fvNV= (PFNGLPROGRAMPARAMETER4FVNVPROC)nelglGetProcAddress("glProgramParameter4fvNV"))) return false;
	if(!(nglProgramParameters4dvNV= (PFNGLPROGRAMPARAMETERS4DVNVPROC)nelglGetProcAddress("glProgramParameters4dvNV"))) return false;
	if(!(nglProgramParameters4fvNV= (PFNGLPROGRAMPARAMETERS4FVNVPROC)nelglGetProcAddress("glProgramParameters4fvNV"))) return false;
	if(!(nglRequestResidentProgramsNV= (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)nelglGetProcAddress("glRequestResidentProgramsNV"))) return false;
	if(!(nglTrackMatrixNV= (PFNGLTRACKMATRIXNVPROC)nelglGetProcAddress("glTrackMatrixNV"))) return false;
	if(!(nglVertexAttribPointerNV= (PFNGLVERTEXATTRIBPOINTERNVPROC)nelglGetProcAddress("glVertexAttribPointerNV"))) return false;
	if(!(nglVertexAttrib1dNV= (PFNGLVERTEXATTRIB1DNVPROC)nelglGetProcAddress("glVertexAttrib1dNV"))) return false;
	if(!(nglVertexAttrib1dvNV= (PFNGLVERTEXATTRIB1DVNVPROC)nelglGetProcAddress("glVertexAttrib1dvNV"))) return false;
	if(!(nglVertexAttrib1fNV= (PFNGLVERTEXATTRIB1FNVPROC)nelglGetProcAddress("glVertexAttrib1fNV"))) return false;
	if(!(nglVertexAttrib1fvNV= (PFNGLVERTEXATTRIB1FVNVPROC)nelglGetProcAddress("glVertexAttrib1fvNV"))) return false;
	if(!(nglVertexAttrib1sNV= (PFNGLVERTEXATTRIB1SNVPROC)nelglGetProcAddress("glVertexAttrib1sNV"))) return false;
	if(!(nglVertexAttrib1svNV= (PFNGLVERTEXATTRIB1SVNVPROC)nelglGetProcAddress("glVertexAttrib1svNV"))) return false;
	if(!(nglVertexAttrib2dNV= (PFNGLVERTEXATTRIB2DNVPROC)nelglGetProcAddress("glVertexAttrib2dNV"))) return false;
	if(!(nglVertexAttrib2dvNV= (PFNGLVERTEXATTRIB2DVNVPROC)nelglGetProcAddress("glVertexAttrib2dvNV"))) return false;
	if(!(nglVertexAttrib2fNV= (PFNGLVERTEXATTRIB2FNVPROC)nelglGetProcAddress("glVertexAttrib2fNV"))) return false;
	if(!(nglVertexAttrib2fvNV= (PFNGLVERTEXATTRIB2FVNVPROC)nelglGetProcAddress("glVertexAttrib2fvNV"))) return false;
	if(!(nglVertexAttrib2sNV= (PFNGLVERTEXATTRIB2SNVPROC)nelglGetProcAddress("glVertexAttrib2sNV"))) return false;
	if(!(nglVertexAttrib2svNV= (PFNGLVERTEXATTRIB2SVNVPROC)nelglGetProcAddress("glVertexAttrib2svNV"))) return false;
	if(!(nglVertexAttrib3dNV= (PFNGLVERTEXATTRIB3DNVPROC)nelglGetProcAddress("glVertexAttrib3dNV"))) return false;
	if(!(nglVertexAttrib3dvNV= (PFNGLVERTEXATTRIB3DVNVPROC)nelglGetProcAddress("glVertexAttrib3dvNV"))) return false;
	if(!(nglVertexAttrib3fNV= (PFNGLVERTEXATTRIB3FNVPROC)nelglGetProcAddress("glVertexAttrib3fNV"))) return false;
	if(!(nglVertexAttrib3fvNV= (PFNGLVERTEXATTRIB3FVNVPROC)nelglGetProcAddress("glVertexAttrib3fvNV"))) return false;
	if(!(nglVertexAttrib3sNV= (PFNGLVERTEXATTRIB3SNVPROC)nelglGetProcAddress("glVertexAttrib3sNV"))) return false;
	if(!(nglVertexAttrib3svNV= (PFNGLVERTEXATTRIB3SVNVPROC)nelglGetProcAddress("glVertexAttrib3svNV"))) return false;
	if(!(nglVertexAttrib4dNV= (PFNGLVERTEXATTRIB4DNVPROC)nelglGetProcAddress("glVertexAttrib4dNV"))) return false;
	if(!(nglVertexAttrib4dvNV= (PFNGLVERTEXATTRIB4DVNVPROC)nelglGetProcAddress("glVertexAttrib4dvNV"))) return false;
	if(!(nglVertexAttrib4fNV= (PFNGLVERTEXATTRIB4FNVPROC)nelglGetProcAddress("glVertexAttrib4fNV"))) return false;
	if(!(nglVertexAttrib4fvNV= (PFNGLVERTEXATTRIB4FVNVPROC)nelglGetProcAddress("glVertexAttrib4fvNV"))) return false;
	if(!(nglVertexAttrib4sNV= (PFNGLVERTEXATTRIB4SNVPROC)nelglGetProcAddress("glVertexAttrib4sNV"))) return false;
	if(!(nglVertexAttrib4svNV= (PFNGLVERTEXATTRIB4SVNVPROC)nelglGetProcAddress("glVertexAttrib4svNV"))) return false;
	if(!(nglVertexAttrib4ubvNV= (PFNGLVERTEXATTRIB4UBVNVPROC)nelglGetProcAddress("glVertexAttrib4ubvNV"))) return false;
	if(!(nglVertexAttribs1dvNV= (PFNGLVERTEXATTRIBS1DVNVPROC)nelglGetProcAddress("glVertexAttribs1dvNV"))) return false;
	if(!(nglVertexAttribs1fvNV= (PFNGLVERTEXATTRIBS1FVNVPROC)nelglGetProcAddress("glVertexAttribs1fvNV"))) return false;
	if(!(nglVertexAttribs1svNV= (PFNGLVERTEXATTRIBS1SVNVPROC)nelglGetProcAddress("glVertexAttribs1svNV"))) return false;
	if(!(nglVertexAttribs2dvNV= (PFNGLVERTEXATTRIBS2DVNVPROC)nelglGetProcAddress("glVertexAttribs2dvNV"))) return false;
	if(!(nglVertexAttribs2fvNV= (PFNGLVERTEXATTRIBS2FVNVPROC)nelglGetProcAddress("glVertexAttribs2fvNV"))) return false;
	if(!(nglVertexAttribs2svNV= (PFNGLVERTEXATTRIBS2SVNVPROC)nelglGetProcAddress("glVertexAttribs2svNV"))) return false;
	if(!(nglVertexAttribs3dvNV= (PFNGLVERTEXATTRIBS3DVNVPROC)nelglGetProcAddress("glVertexAttribs3dvNV"))) return false;
	if(!(nglVertexAttribs3fvNV= (PFNGLVERTEXATTRIBS3FVNVPROC)nelglGetProcAddress("glVertexAttribs3fvNV"))) return false;
	if(!(nglVertexAttribs3svNV= (PFNGLVERTEXATTRIBS3SVNVPROC)nelglGetProcAddress("glVertexAttribs3svNV"))) return false;
	if(!(nglVertexAttribs4dvNV= (PFNGLVERTEXATTRIBS4DVNVPROC)nelglGetProcAddress("glVertexAttribs4dvNV"))) return false;
	if(!(nglVertexAttribs4fvNV= (PFNGLVERTEXATTRIBS4FVNVPROC)nelglGetProcAddress("glVertexAttribs4fvNV"))) return false;
	if(!(nglVertexAttribs4svNV= (PFNGLVERTEXATTRIBS4SVNVPROC)nelglGetProcAddress("glVertexAttribs4svNV"))) return false;
	if(!(nglVertexAttribs4ubvNV= (PFNGLVERTEXATTRIBS4UBVNVPROC)nelglGetProcAddress("glVertexAttribs4ubvNV"))) return false;


	return true;
}


// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	if(strstr(glext, "GL_EXT_secondary_color")==NULL)
		return false;


	if(!(nglSecondaryColor3bEXT= (PFNGLSECONDARYCOLOR3BEXTPROC)nelglGetProcAddress("glSecondaryColor3bEXT"))) return false;
	if(!(nglSecondaryColor3bvEXT= (PFNGLSECONDARYCOLOR3BVEXTPROC)nelglGetProcAddress("glSecondaryColor3bvEXT"))) return false;
	if(!(nglSecondaryColor3dEXT= (PFNGLSECONDARYCOLOR3DEXTPROC)nelglGetProcAddress("glSecondaryColor3dEXT"))) return false;
	if(!(nglSecondaryColor3dvEXT= (PFNGLSECONDARYCOLOR3DVEXTPROC)nelglGetProcAddress("glSecondaryColor3dvEXT"))) return false;
	if(!(nglSecondaryColor3fEXT= (PFNGLSECONDARYCOLOR3FEXTPROC)nelglGetProcAddress("glSecondaryColor3fEXT"))) return false;
	if(!(nglSecondaryColor3fvEXT= (PFNGLSECONDARYCOLOR3FVEXTPROC)nelglGetProcAddress("glSecondaryColor3fvEXT"))) return false;
	if(!(nglSecondaryColor3iEXT= (PFNGLSECONDARYCOLOR3IEXTPROC)nelglGetProcAddress("glSecondaryColor3iEXT"))) return false;
	if(!(nglSecondaryColor3ivEXT= (PFNGLSECONDARYCOLOR3IVEXTPROC)nelglGetProcAddress("glSecondaryColor3ivEXT"))) return false;
	if(!(nglSecondaryColor3sEXT= (PFNGLSECONDARYCOLOR3SEXTPROC)nelglGetProcAddress("glSecondaryColor3sEXT"))) return false;
	if(!(nglSecondaryColor3svEXT= (PFNGLSECONDARYCOLOR3SVEXTPROC)nelglGetProcAddress("glSecondaryColor3svEXT"))) return false;
	if(!(nglSecondaryColor3ubEXT= (PFNGLSECONDARYCOLOR3UBEXTPROC)nelglGetProcAddress("glSecondaryColor3ubEXT"))) return false;
	if(!(nglSecondaryColor3ubvEXT= (PFNGLSECONDARYCOLOR3UBVEXTPROC)nelglGetProcAddress("glSecondaryColor3ubvEXT"))) return false;
	if(!(nglSecondaryColor3uiEXT= (PFNGLSECONDARYCOLOR3UIEXTPROC)nelglGetProcAddress("glSecondaryColor3uiEXT"))) return false;
	if(!(nglSecondaryColor3uivEXT= (PFNGLSECONDARYCOLOR3UIVEXTPROC)nelglGetProcAddress("glSecondaryColor3uivEXT"))) return false;
	if(!(nglSecondaryColor3usEXT= (PFNGLSECONDARYCOLOR3USEXTPROC)nelglGetProcAddress("glSecondaryColor3usEXT"))) return false;
	if(!(nglSecondaryColor3usvEXT= (PFNGLSECONDARYCOLOR3USVEXTPROC)nelglGetProcAddress("glSecondaryColor3usvEXT"))) return false;
	if(!(nglSecondaryColorPointerEXT= (PFNGLSECONDARYCOLORPOINTEREXTPROC)nelglGetProcAddress("glSecondaryColorPointerEXT"))) return false;


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




