/** \file driver_opengl_extension.cpp
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.cpp,v 1.28 2002/02/07 19:32:56 berenguier Exp $
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
#ifndef NEL_GL_VERSION_1_2_IN_GL_H

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

PFNGLMULTITEXCOORD1SARBPROC glMultiTexCoord1sARB;
PFNGLMULTITEXCOORD1IARBPROC glMultiTexCoord1iARB;
PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARB;
PFNGLMULTITEXCOORD1DARBPROC glMultiTexCoord1dARB;
PFNGLMULTITEXCOORD2SARBPROC glMultiTexCoord2sARB;
PFNGLMULTITEXCOORD2IARBPROC glMultiTexCoord2iARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD2DARBPROC glMultiTexCoord2dARB;
PFNGLMULTITEXCOORD3SARBPROC glMultiTexCoord3sARB;
PFNGLMULTITEXCOORD3IARBPROC glMultiTexCoord3iARB;
PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3DARBPROC glMultiTexCoord3dARB;
PFNGLMULTITEXCOORD4SARBPROC glMultiTexCoord4sARB;
PFNGLMULTITEXCOORD4IARBPROC glMultiTexCoord4iARB;
PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;
PFNGLMULTITEXCOORD4DARBPROC glMultiTexCoord4dARB;

PFNGLMULTITEXCOORD1SVARBPROC glMultiTexCoord1svARB;
PFNGLMULTITEXCOORD1IVARBPROC glMultiTexCoord1ivARB;
PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fvARB;
PFNGLMULTITEXCOORD1DVARBPROC glMultiTexCoord1dvARB;
PFNGLMULTITEXCOORD2SVARBPROC glMultiTexCoord2svARB;
PFNGLMULTITEXCOORD2IVARBPROC glMultiTexCoord2ivARB;
PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
PFNGLMULTITEXCOORD2DVARBPROC glMultiTexCoord2dvARB;
PFNGLMULTITEXCOORD3SVARBPROC glMultiTexCoord3svARB;
PFNGLMULTITEXCOORD3IVARBPROC glMultiTexCoord3ivARB;
PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARB;
PFNGLMULTITEXCOORD3DVARBPROC glMultiTexCoord3dvARB;
PFNGLMULTITEXCOORD4SVARBPROC glMultiTexCoord4svARB;
PFNGLMULTITEXCOORD4IVARBPROC glMultiTexCoord4ivARB;
PFNGLMULTITEXCOORD4FVARBPROC glMultiTexCoord4fvARB;
PFNGLMULTITEXCOORD4DVARBPROC glMultiTexCoord4dvARB;

#endif // GL_VERSION_1_2


// ARB_TextureCompression.
//========================
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC	glCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC	glCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC	glCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	glCompressedTexSubImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	glCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	glCompressedTexSubImage1DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	glGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
PFNGLFLUSHVERTEXARRAYRANGENVPROC	glFlushVertexArrayRangeNV;
PFNGLVERTEXARRAYRANGENVPROC			glVertexArrayRangeNV;
#ifdef NL_OS_WINDOWS
PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;
#endif


// FenceNV.
//====================
PFNGLDELETEFENCESNVPROC				glDeleteFencesNV;
PFNGLGENFENCESNVPROC				glGenFencesNV;
PFNGLISFENCENVPROC					glIsFenceNV;
PFNGLTESTFENCENVPROC				glTestFenceNV;
PFNGLGETFENCEIVNVPROC				glGetFenceivNV;
PFNGLFINISHFENCENVPROC				glFinishFenceNV;
PFNGLSETFENCENVPROC					glSetFenceNV;


// VertexWeighting.
//====================
PFNGLVERTEXWEIGHTFEXTPROC			glVertexWeightfEXT;
PFNGLVERTEXWEIGHTFVEXTPROC			glVertexWeightfvEXT;
PFNGLVERTEXWEIGHTPOINTEREXTPROC		glVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
PFNGLAREPROGRAMSRESIDENTNVPROC		glAreProgramsResidentNV;
PFNGLBINDPROGRAMNVPROC				glBindProgramNV;
PFNGLDELETEPROGRAMSNVPROC			glDeleteProgramsNV;
PFNGLEXECUTEPROGRAMNVPROC			glExecuteProgramNV;
PFNGLGENPROGRAMSNVPROC				glGenProgramsNV;
PFNGLGETPROGRAMPARAMETERDVNVPROC	glGetProgramParameterdvNV;
PFNGLGETPROGRAMPARAMETERFVNVPROC	glGetProgramParameterfvNV;
PFNGLGETPROGRAMIVNVPROC				glGetProgramivNV;
PFNGLGETPROGRAMSTRINGNVPROC			glGetProgramStringNV;
PFNGLGETTRACKMATRIXIVNVPROC			glGetTrackMatrixivNV;
PFNGLGETVERTEXATTRIBDVNVPROC		glGetVertexAttribdvNV;
PFNGLGETVERTEXATTRIBFVNVPROC		glGetVertexAttribfvNV;
PFNGLGETVERTEXATTRIBIVNVPROC		glGetVertexAttribivNV;
PFNGLGETVERTEXATTRIBPOINTERVNVPROC	glGetVertexAttribPointervNV;
PFNGLISPROGRAMNVPROC				glIsProgramNV;
PFNGLLOADPROGRAMNVPROC				glLoadProgramNV;
PFNGLPROGRAMPARAMETER4DNVPROC		glProgramParameter4dNV;
PFNGLPROGRAMPARAMETER4DVNVPROC		glProgramParameter4dvNV;
PFNGLPROGRAMPARAMETER4FNVPROC		glProgramParameter4fNV;
PFNGLPROGRAMPARAMETER4FVNVPROC		glProgramParameter4fvNV;
PFNGLPROGRAMPARAMETERS4DVNVPROC		glProgramParameters4dvNV;
PFNGLPROGRAMPARAMETERS4FVNVPROC		glProgramParameters4fvNV;
PFNGLREQUESTRESIDENTPROGRAMSNVPROC	glRequestResidentProgramsNV;
PFNGLTRACKMATRIXNVPROC				glTrackMatrixNV;
PFNGLVERTEXATTRIBPOINTERNVPROC		glVertexAttribPointerNV;
PFNGLVERTEXATTRIB1DNVPROC			glVertexAttrib1dNV;
PFNGLVERTEXATTRIB1DVNVPROC			glVertexAttrib1dvNV;
PFNGLVERTEXATTRIB1FNVPROC			glVertexAttrib1fNV;
PFNGLVERTEXATTRIB1FVNVPROC			glVertexAttrib1fvNV;
PFNGLVERTEXATTRIB1SNVPROC			glVertexAttrib1sNV;
PFNGLVERTEXATTRIB1SVNVPROC			glVertexAttrib1svNV;
PFNGLVERTEXATTRIB2DNVPROC			glVertexAttrib2dNV;
PFNGLVERTEXATTRIB2DVNVPROC			glVertexAttrib2dvNV;
PFNGLVERTEXATTRIB2FNVPROC			glVertexAttrib2fNV;
PFNGLVERTEXATTRIB2FVNVPROC			glVertexAttrib2fvNV;
PFNGLVERTEXATTRIB2SNVPROC			glVertexAttrib2sNV;
PFNGLVERTEXATTRIB2SVNVPROC			glVertexAttrib2svNV;
PFNGLVERTEXATTRIB3DNVPROC			glVertexAttrib3dNV;
PFNGLVERTEXATTRIB3DVNVPROC			glVertexAttrib3dvNV;
PFNGLVERTEXATTRIB3FNVPROC			glVertexAttrib3fNV;
PFNGLVERTEXATTRIB3FVNVPROC			glVertexAttrib3fvNV;
PFNGLVERTEXATTRIB3SNVPROC			glVertexAttrib3sNV;
PFNGLVERTEXATTRIB3SVNVPROC			glVertexAttrib3svNV;
PFNGLVERTEXATTRIB4DNVPROC			glVertexAttrib4dNV;
PFNGLVERTEXATTRIB4DVNVPROC			glVertexAttrib4dvNV;
PFNGLVERTEXATTRIB4FNVPROC			glVertexAttrib4fNV;
PFNGLVERTEXATTRIB4FVNVPROC			glVertexAttrib4fvNV;
PFNGLVERTEXATTRIB4SNVPROC			glVertexAttrib4sNV;
PFNGLVERTEXATTRIB4SVNVPROC			glVertexAttrib4svNV;
PFNGLVERTEXATTRIB4UBVNVPROC			glVertexAttrib4ubvNV;
PFNGLVERTEXATTRIBS1DVNVPROC			glVertexAttribs1dvNV;
PFNGLVERTEXATTRIBS1FVNVPROC			glVertexAttribs1fvNV;
PFNGLVERTEXATTRIBS1SVNVPROC			glVertexAttribs1svNV;
PFNGLVERTEXATTRIBS2DVNVPROC			glVertexAttribs2dvNV;
PFNGLVERTEXATTRIBS2FVNVPROC			glVertexAttribs2fvNV;
PFNGLVERTEXATTRIBS2SVNVPROC			glVertexAttribs2svNV;
PFNGLVERTEXATTRIBS3DVNVPROC			glVertexAttribs3dvNV;
PFNGLVERTEXATTRIBS3FVNVPROC			glVertexAttribs3fvNV;
PFNGLVERTEXATTRIBS3SVNVPROC			glVertexAttribs3svNV;
PFNGLVERTEXATTRIBS4DVNVPROC			glVertexAttribs4dvNV;
PFNGLVERTEXATTRIBS4FVNVPROC			glVertexAttribs4fvNV;
PFNGLVERTEXATTRIBS4SVNVPROC			glVertexAttribs4svNV;
PFNGLVERTEXATTRIBS4UBVNVPROC		glVertexAttribs4ubvNV;


// SecondaryColor extension
//========================
PFNGLSECONDARYCOLOR3BEXTPROC		glSecondaryColor3bEXT;
PFNGLSECONDARYCOLOR3BVEXTPROC		glSecondaryColor3bvEXT;
PFNGLSECONDARYCOLOR3DEXTPROC		glSecondaryColor3dEXT;
PFNGLSECONDARYCOLOR3DVEXTPROC		glSecondaryColor3dvEXT;
PFNGLSECONDARYCOLOR3FEXTPROC		glSecondaryColor3fEXT;
PFNGLSECONDARYCOLOR3FVEXTPROC		glSecondaryColor3fvEXT;
PFNGLSECONDARYCOLOR3IEXTPROC		glSecondaryColor3iEXT;
PFNGLSECONDARYCOLOR3IVEXTPROC		glSecondaryColor3ivEXT;
PFNGLSECONDARYCOLOR3SEXTPROC		glSecondaryColor3sEXT;
PFNGLSECONDARYCOLOR3SVEXTPROC		glSecondaryColor3svEXT;
PFNGLSECONDARYCOLOR3UBEXTPROC		glSecondaryColor3ubEXT;
PFNGLSECONDARYCOLOR3UBVEXTPROC		glSecondaryColor3ubvEXT;
PFNGLSECONDARYCOLOR3UIEXTPROC		glSecondaryColor3uiEXT;
PFNGLSECONDARYCOLOR3UIVEXTPROC		glSecondaryColor3uivEXT;
PFNGLSECONDARYCOLOR3USEXTPROC		glSecondaryColor3usEXT;
PFNGLSECONDARYCOLOR3USVEXTPROC		glSecondaryColor3usvEXT;
PFNGLSECONDARYCOLORPOINTEREXTPROC	glSecondaryColorPointerEXT;


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

#ifndef NEL_GL_VERSION_1_2_IN_GL_H
	if(!(glActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC)nelglGetProcAddress("glActiveTextureARB")))return false;
	if(!(glClientActiveTextureARB=(PFNGLCLIENTACTIVETEXTUREARBPROC)nelglGetProcAddress("glClientActiveTextureARB")))return false;

	if(!(glMultiTexCoord1sARB=(PFNGLMULTITEXCOORD1SARBPROC)nelglGetProcAddress("glMultiTexCoord1sARB")))return false;
	if(!(glMultiTexCoord1iARB=(PFNGLMULTITEXCOORD1IARBPROC)nelglGetProcAddress("glMultiTexCoord1iARB")))return false;
	if(!(glMultiTexCoord1fARB=(PFNGLMULTITEXCOORD1FARBPROC)nelglGetProcAddress("glMultiTexCoord1fARB")))return false;
	if(!(glMultiTexCoord1dARB=(PFNGLMULTITEXCOORD1DARBPROC)nelglGetProcAddress("glMultiTexCoord1dARB")))return false;
	if(!(glMultiTexCoord2sARB=(PFNGLMULTITEXCOORD2SARBPROC)nelglGetProcAddress("glMultiTexCoord2sARB")))return false;
	if(!(glMultiTexCoord2iARB=(PFNGLMULTITEXCOORD2IARBPROC)nelglGetProcAddress("glMultiTexCoord2iARB")))return false;
	if(!(glMultiTexCoord2fARB=(PFNGLMULTITEXCOORD2FARBPROC)nelglGetProcAddress("glMultiTexCoord2fARB")))return false;
	if(!(glMultiTexCoord2dARB=(PFNGLMULTITEXCOORD2DARBPROC)nelglGetProcAddress("glMultiTexCoord2dARB")))return false;
	if(!(glMultiTexCoord3sARB=(PFNGLMULTITEXCOORD3SARBPROC)nelglGetProcAddress("glMultiTexCoord3sARB")))return false;
	if(!(glMultiTexCoord3iARB=(PFNGLMULTITEXCOORD3IARBPROC)nelglGetProcAddress("glMultiTexCoord3iARB")))return false;
	if(!(glMultiTexCoord3fARB=(PFNGLMULTITEXCOORD3FARBPROC)nelglGetProcAddress("glMultiTexCoord3fARB")))return false;
	if(!(glMultiTexCoord3dARB=(PFNGLMULTITEXCOORD3DARBPROC)nelglGetProcAddress("glMultiTexCoord3dARB")))return false;
	if(!(glMultiTexCoord4sARB=(PFNGLMULTITEXCOORD4SARBPROC)nelglGetProcAddress("glMultiTexCoord4sARB")))return false;
	if(!(glMultiTexCoord4iARB=(PFNGLMULTITEXCOORD4IARBPROC)nelglGetProcAddress("glMultiTexCoord4iARB")))return false;
	if(!(glMultiTexCoord4fARB=(PFNGLMULTITEXCOORD4FARBPROC)nelglGetProcAddress("glMultiTexCoord4fARB")))return false;
	if(!(glMultiTexCoord4dARB=(PFNGLMULTITEXCOORD4DARBPROC)nelglGetProcAddress("glMultiTexCoord4dARB")))return false;

	if(!(glMultiTexCoord1svARB=(PFNGLMULTITEXCOORD1SVARBPROC)nelglGetProcAddress("glMultiTexCoord1svARB")))return false;
	if(!(glMultiTexCoord1ivARB=(PFNGLMULTITEXCOORD1IVARBPROC)nelglGetProcAddress("glMultiTexCoord1ivARB")))return false;
	if(!(glMultiTexCoord1fvARB=(PFNGLMULTITEXCOORD1FVARBPROC)nelglGetProcAddress("glMultiTexCoord1fvARB")))return false;
	if(!(glMultiTexCoord1dvARB=(PFNGLMULTITEXCOORD1DVARBPROC)nelglGetProcAddress("glMultiTexCoord1dvARB")))return false;
	if(!(glMultiTexCoord2svARB=(PFNGLMULTITEXCOORD2SVARBPROC)nelglGetProcAddress("glMultiTexCoord2svARB")))return false;
	if(!(glMultiTexCoord2ivARB=(PFNGLMULTITEXCOORD2IVARBPROC)nelglGetProcAddress("glMultiTexCoord2ivARB")))return false;
	if(!(glMultiTexCoord2fvARB=(PFNGLMULTITEXCOORD2FVARBPROC)nelglGetProcAddress("glMultiTexCoord2fvARB")))return false;
	if(!(glMultiTexCoord2dvARB=(PFNGLMULTITEXCOORD2DVARBPROC)nelglGetProcAddress("glMultiTexCoord2dvARB")))return false;
	if(!(glMultiTexCoord3svARB=(PFNGLMULTITEXCOORD3SVARBPROC)nelglGetProcAddress("glMultiTexCoord3svARB")))return false;
	if(!(glMultiTexCoord3ivARB=(PFNGLMULTITEXCOORD3IVARBPROC)nelglGetProcAddress("glMultiTexCoord3ivARB")))return false;
	if(!(glMultiTexCoord3fvARB=(PFNGLMULTITEXCOORD3FVARBPROC)nelglGetProcAddress("glMultiTexCoord3fvARB")))return false;
	if(!(glMultiTexCoord3dvARB=(PFNGLMULTITEXCOORD3DVARBPROC)nelglGetProcAddress("glMultiTexCoord3dvARB")))return false;
	if(!(glMultiTexCoord4svARB=(PFNGLMULTITEXCOORD4SVARBPROC)nelglGetProcAddress("glMultiTexCoord4svARB")))return false;
	if(!(glMultiTexCoord4ivARB=(PFNGLMULTITEXCOORD4IVARBPROC)nelglGetProcAddress("glMultiTexCoord4ivARB")))return false;
	if(!(glMultiTexCoord4fvARB=(PFNGLMULTITEXCOORD4FVARBPROC)nelglGetProcAddress("glMultiTexCoord4fvARB")))return false;
	if(!(glMultiTexCoord4dvARB=(PFNGLMULTITEXCOORD4DVARBPROC)nelglGetProcAddress("glMultiTexCoord4dvARB")))return false;
#endif // GL_VERSION_1_2

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

	if(!(glCompressedTexImage3DARB=(PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)nelglGetProcAddress("glCompressedTexImage3DARB")))return false;
	if(!(glCompressedTexImage2DARB=(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)nelglGetProcAddress("glCompressedTexImage2DARB")))return false;
	if(!(glCompressedTexImage1DARB=(PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)nelglGetProcAddress("glCompressedTexImage1DARB")))return false;
	if(!(glCompressedTexSubImage3DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)nelglGetProcAddress("glCompressedTexSubImage3DARB")))return false;
	if(!(glCompressedTexSubImage2DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)nelglGetProcAddress("glCompressedTexSubImage2DARB")))return false;
	if(!(glCompressedTexSubImage1DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)nelglGetProcAddress("glCompressedTexSubImage1DARB")))return false;
	if(!(glGetCompressedTexImageARB=(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)nelglGetProcAddress("glGetCompressedTexImageARB")))return false;

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
	if(!(glFlushVertexArrayRangeNV=(PFNGLFLUSHVERTEXARRAYRANGENVPROC)nelglGetProcAddress("glFlushVertexArrayRangeNV")))return false;
	if(!(glVertexArrayRangeNV=(PFNGLVERTEXARRAYRANGENVPROC)nelglGetProcAddress("glVertexArrayRangeNV")))return false;
#ifdef NL_OS_WINDOWS
	if(!(wglAllocateMemoryNV= (PFNWGLALLOCATEMEMORYNVPROC)nelglGetProcAddress("wglAllocateMemoryNV")))return false;
	if(!(wglFreeMemoryNV= (PFNWGLFREEMEMORYNVPROC)nelglGetProcAddress("wglFreeMemoryNV")))return false;
#endif

	// Get fence adress.
	if(!(glDeleteFencesNV= (PFNGLDELETEFENCESNVPROC)nelglGetProcAddress("glDeleteFencesNV")))return false;
	if(!(glGenFencesNV= (PFNGLGENFENCESNVPROC)nelglGetProcAddress("glGenFencesNV")))return false;
	if(!(glIsFenceNV= (PFNGLISFENCENVPROC)nelglGetProcAddress("glIsFenceNV")))return false;
	if(!(glTestFenceNV= (PFNGLTESTFENCENVPROC)nelglGetProcAddress("glTestFenceNV")))return false;
	if(!(glGetFenceivNV= (PFNGLGETFENCEIVNVPROC)nelglGetProcAddress("glGetFenceivNV")))return false;
	if(!(glFinishFenceNV= (PFNGLFINISHFENCENVPROC)nelglGetProcAddress("glFinishFenceNV")))return false;
	if(!(glSetFenceNV= (PFNGLSETFENCENVPROC)nelglGetProcAddress("glSetFenceNV")))return false;


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


	if(!(glVertexWeightfEXT=(PFNGLVERTEXWEIGHTFEXTPROC)nelglGetProcAddress("glVertexWeightfEXT")))return false;
	if(!(glVertexWeightfvEXT=(PFNGLVERTEXWEIGHTFVEXTPROC)nelglGetProcAddress("glVertexWeightfvEXT")))return false;
	if(!(glVertexWeightPointerEXT=(PFNGLVERTEXWEIGHTPOINTEREXTPROC)nelglGetProcAddress("glVertexWeightPointerEXT")))return false;

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


	if(!(glAreProgramsResidentNV= (PFNGLAREPROGRAMSRESIDENTNVPROC)nelglGetProcAddress("glAreProgramsResidentNV"))) return false;
	if(!(glBindProgramNV= (PFNGLBINDPROGRAMNVPROC)nelglGetProcAddress("glBindProgramNV"))) return false;
	if(!(glDeleteProgramsNV= (PFNGLDELETEPROGRAMSNVPROC)nelglGetProcAddress("glDeleteProgramsNV"))) return false;
	if(!(glExecuteProgramNV= (PFNGLEXECUTEPROGRAMNVPROC)nelglGetProcAddress("glExecuteProgramNV"))) return false;
	if(!(glGenProgramsNV= (PFNGLGENPROGRAMSNVPROC)nelglGetProcAddress("glGenProgramsNV"))) return false;
	if(!(glGetProgramParameterdvNV= (PFNGLGETPROGRAMPARAMETERDVNVPROC)nelglGetProcAddress("glGetProgramParameterdvNV"))) return false;
	if(!(glGetProgramParameterfvNV= (PFNGLGETPROGRAMPARAMETERFVNVPROC)nelglGetProcAddress("glGetProgramParameterfvNV"))) return false;
	if(!(glGetProgramivNV= (PFNGLGETPROGRAMIVNVPROC)nelglGetProcAddress("glGetProgramivNV"))) return false;
	if(!(glGetProgramStringNV= (PFNGLGETPROGRAMSTRINGNVPROC)nelglGetProcAddress("glGetProgramStringNV"))) return false;
	if(!(glGetTrackMatrixivNV= (PFNGLGETTRACKMATRIXIVNVPROC)nelglGetProcAddress("glGetTrackMatrixivNV"))) return false;
	if(!(glGetVertexAttribdvNV= (PFNGLGETVERTEXATTRIBDVNVPROC)nelglGetProcAddress("glGetVertexAttribdvNV"))) return false;
	if(!(glGetVertexAttribfvNV= (PFNGLGETVERTEXATTRIBFVNVPROC)nelglGetProcAddress("glGetVertexAttribfvNV"))) return false;
	if(!(glGetVertexAttribivNV= (PFNGLGETVERTEXATTRIBIVNVPROC)nelglGetProcAddress("glGetVertexAttribivNV"))) return false;
	if(!(glGetVertexAttribPointervNV= (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)nelglGetProcAddress("glGetVertexAttribPointervNV"))) return false;
	if(!(glIsProgramNV= (PFNGLISPROGRAMNVPROC)nelglGetProcAddress("glIsProgramNV"))) return false;
	if(!(glLoadProgramNV= (PFNGLLOADPROGRAMNVPROC)nelglGetProcAddress("glLoadProgramNV"))) return false;
	if(!(glProgramParameter4dNV= (PFNGLPROGRAMPARAMETER4DNVPROC)nelglGetProcAddress("glProgramParameter4dNV"))) return false;
	if(!(glProgramParameter4dvNV= (PFNGLPROGRAMPARAMETER4DVNVPROC)nelglGetProcAddress("glProgramParameter4dvNV"))) return false;
	if(!(glProgramParameter4fNV= (PFNGLPROGRAMPARAMETER4FNVPROC)nelglGetProcAddress("glProgramParameter4fNV"))) return false;
	if(!(glProgramParameter4fvNV= (PFNGLPROGRAMPARAMETER4FVNVPROC)nelglGetProcAddress("glProgramParameter4fvNV"))) return false;
	if(!(glProgramParameters4dvNV= (PFNGLPROGRAMPARAMETERS4DVNVPROC)nelglGetProcAddress("glProgramParameters4dvNV"))) return false;
	if(!(glProgramParameters4fvNV= (PFNGLPROGRAMPARAMETERS4FVNVPROC)nelglGetProcAddress("glProgramParameters4fvNV"))) return false;
	if(!(glRequestResidentProgramsNV= (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)nelglGetProcAddress("glRequestResidentProgramsNV"))) return false;
	if(!(glTrackMatrixNV= (PFNGLTRACKMATRIXNVPROC)nelglGetProcAddress("glTrackMatrixNV"))) return false;
	if(!(glVertexAttribPointerNV= (PFNGLVERTEXATTRIBPOINTERNVPROC)nelglGetProcAddress("glVertexAttribPointerNV"))) return false;
	if(!(glVertexAttrib1dNV= (PFNGLVERTEXATTRIB1DNVPROC)nelglGetProcAddress("glVertexAttrib1dNV"))) return false;
	if(!(glVertexAttrib1dvNV= (PFNGLVERTEXATTRIB1DVNVPROC)nelglGetProcAddress("glVertexAttrib1dvNV"))) return false;
	if(!(glVertexAttrib1fNV= (PFNGLVERTEXATTRIB1FNVPROC)nelglGetProcAddress("glVertexAttrib1fNV"))) return false;
	if(!(glVertexAttrib1fvNV= (PFNGLVERTEXATTRIB1FVNVPROC)nelglGetProcAddress("glVertexAttrib1fvNV"))) return false;
	if(!(glVertexAttrib1sNV= (PFNGLVERTEXATTRIB1SNVPROC)nelglGetProcAddress("glVertexAttrib1sNV"))) return false;
	if(!(glVertexAttrib1svNV= (PFNGLVERTEXATTRIB1SVNVPROC)nelglGetProcAddress("glVertexAttrib1svNV"))) return false;
	if(!(glVertexAttrib2dNV= (PFNGLVERTEXATTRIB2DNVPROC)nelglGetProcAddress("glVertexAttrib2dNV"))) return false;
	if(!(glVertexAttrib2dvNV= (PFNGLVERTEXATTRIB2DVNVPROC)nelglGetProcAddress("glVertexAttrib2dvNV"))) return false;
	if(!(glVertexAttrib2fNV= (PFNGLVERTEXATTRIB2FNVPROC)nelglGetProcAddress("glVertexAttrib2fNV"))) return false;
	if(!(glVertexAttrib2fvNV= (PFNGLVERTEXATTRIB2FVNVPROC)nelglGetProcAddress("glVertexAttrib2fvNV"))) return false;
	if(!(glVertexAttrib2sNV= (PFNGLVERTEXATTRIB2SNVPROC)nelglGetProcAddress("glVertexAttrib2sNV"))) return false;
	if(!(glVertexAttrib2svNV= (PFNGLVERTEXATTRIB2SVNVPROC)nelglGetProcAddress("glVertexAttrib2svNV"))) return false;
	if(!(glVertexAttrib3dNV= (PFNGLVERTEXATTRIB3DNVPROC)nelglGetProcAddress("glVertexAttrib3dNV"))) return false;
	if(!(glVertexAttrib3dvNV= (PFNGLVERTEXATTRIB3DVNVPROC)nelglGetProcAddress("glVertexAttrib3dvNV"))) return false;
	if(!(glVertexAttrib3fNV= (PFNGLVERTEXATTRIB3FNVPROC)nelglGetProcAddress("glVertexAttrib3fNV"))) return false;
	if(!(glVertexAttrib3fvNV= (PFNGLVERTEXATTRIB3FVNVPROC)nelglGetProcAddress("glVertexAttrib3fvNV"))) return false;
	if(!(glVertexAttrib3sNV= (PFNGLVERTEXATTRIB3SNVPROC)nelglGetProcAddress("glVertexAttrib3sNV"))) return false;
	if(!(glVertexAttrib3svNV= (PFNGLVERTEXATTRIB3SVNVPROC)nelglGetProcAddress("glVertexAttrib3svNV"))) return false;
	if(!(glVertexAttrib4dNV= (PFNGLVERTEXATTRIB4DNVPROC)nelglGetProcAddress("glVertexAttrib4dNV"))) return false;
	if(!(glVertexAttrib4dvNV= (PFNGLVERTEXATTRIB4DVNVPROC)nelglGetProcAddress("glVertexAttrib4dvNV"))) return false;
	if(!(glVertexAttrib4fNV= (PFNGLVERTEXATTRIB4FNVPROC)nelglGetProcAddress("glVertexAttrib4fNV"))) return false;
	if(!(glVertexAttrib4fvNV= (PFNGLVERTEXATTRIB4FVNVPROC)nelglGetProcAddress("glVertexAttrib4fvNV"))) return false;
	if(!(glVertexAttrib4sNV= (PFNGLVERTEXATTRIB4SNVPROC)nelglGetProcAddress("glVertexAttrib4sNV"))) return false;
	if(!(glVertexAttrib4svNV= (PFNGLVERTEXATTRIB4SVNVPROC)nelglGetProcAddress("glVertexAttrib4svNV"))) return false;
	if(!(glVertexAttrib4ubvNV= (PFNGLVERTEXATTRIB4UBVNVPROC)nelglGetProcAddress("glVertexAttrib4ubvNV"))) return false;
	if(!(glVertexAttribs1dvNV= (PFNGLVERTEXATTRIBS1DVNVPROC)nelglGetProcAddress("glVertexAttribs1dvNV"))) return false;
	if(!(glVertexAttribs1fvNV= (PFNGLVERTEXATTRIBS1FVNVPROC)nelglGetProcAddress("glVertexAttribs1fvNV"))) return false;
	if(!(glVertexAttribs1svNV= (PFNGLVERTEXATTRIBS1SVNVPROC)nelglGetProcAddress("glVertexAttribs1svNV"))) return false;
	if(!(glVertexAttribs2dvNV= (PFNGLVERTEXATTRIBS2DVNVPROC)nelglGetProcAddress("glVertexAttribs2dvNV"))) return false;
	if(!(glVertexAttribs2fvNV= (PFNGLVERTEXATTRIBS2FVNVPROC)nelglGetProcAddress("glVertexAttribs2fvNV"))) return false;
	if(!(glVertexAttribs2svNV= (PFNGLVERTEXATTRIBS2SVNVPROC)nelglGetProcAddress("glVertexAttribs2svNV"))) return false;
	if(!(glVertexAttribs3dvNV= (PFNGLVERTEXATTRIBS3DVNVPROC)nelglGetProcAddress("glVertexAttribs3dvNV"))) return false;
	if(!(glVertexAttribs3fvNV= (PFNGLVERTEXATTRIBS3FVNVPROC)nelglGetProcAddress("glVertexAttribs3fvNV"))) return false;
	if(!(glVertexAttribs3svNV= (PFNGLVERTEXATTRIBS3SVNVPROC)nelglGetProcAddress("glVertexAttribs3svNV"))) return false;
	if(!(glVertexAttribs4dvNV= (PFNGLVERTEXATTRIBS4DVNVPROC)nelglGetProcAddress("glVertexAttribs4dvNV"))) return false;
	if(!(glVertexAttribs4fvNV= (PFNGLVERTEXATTRIBS4FVNVPROC)nelglGetProcAddress("glVertexAttribs4fvNV"))) return false;
	if(!(glVertexAttribs4svNV= (PFNGLVERTEXATTRIBS4SVNVPROC)nelglGetProcAddress("glVertexAttribs4svNV"))) return false;
	if(!(glVertexAttribs4ubvNV= (PFNGLVERTEXATTRIBS4UBVNVPROC)nelglGetProcAddress("glVertexAttribs4ubvNV"))) return false;


	return true;
}


// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	if(strstr(glext, "GL_EXT_secondary_color")==NULL)
		return false;


	if(!(glSecondaryColor3bEXT= (PFNGLSECONDARYCOLOR3BEXTPROC)nelglGetProcAddress("glSecondaryColor3bEXT"))) return false;
	if(!(glSecondaryColor3bvEXT= (PFNGLSECONDARYCOLOR3BVEXTPROC)nelglGetProcAddress("glSecondaryColor3bvEXT"))) return false;
	if(!(glSecondaryColor3dEXT= (PFNGLSECONDARYCOLOR3DEXTPROC)nelglGetProcAddress("glSecondaryColor3dEXT"))) return false;
	if(!(glSecondaryColor3dvEXT= (PFNGLSECONDARYCOLOR3DVEXTPROC)nelglGetProcAddress("glSecondaryColor3dvEXT"))) return false;
	if(!(glSecondaryColor3fEXT= (PFNGLSECONDARYCOLOR3FEXTPROC)nelglGetProcAddress("glSecondaryColor3fEXT"))) return false;
	if(!(glSecondaryColor3fvEXT= (PFNGLSECONDARYCOLOR3FVEXTPROC)nelglGetProcAddress("glSecondaryColor3fvEXT"))) return false;
	if(!(glSecondaryColor3iEXT= (PFNGLSECONDARYCOLOR3IEXTPROC)nelglGetProcAddress("glSecondaryColor3iEXT"))) return false;
	if(!(glSecondaryColor3ivEXT= (PFNGLSECONDARYCOLOR3IVEXTPROC)nelglGetProcAddress("glSecondaryColor3ivEXT"))) return false;
	if(!(glSecondaryColor3sEXT= (PFNGLSECONDARYCOLOR3SEXTPROC)nelglGetProcAddress("glSecondaryColor3sEXT"))) return false;
	if(!(glSecondaryColor3svEXT= (PFNGLSECONDARYCOLOR3SVEXTPROC)nelglGetProcAddress("glSecondaryColor3svEXT"))) return false;
	if(!(glSecondaryColor3ubEXT= (PFNGLSECONDARYCOLOR3UBEXTPROC)nelglGetProcAddress("glSecondaryColor3ubEXT"))) return false;
	if(!(glSecondaryColor3ubvEXT= (PFNGLSECONDARYCOLOR3UBVEXTPROC)nelglGetProcAddress("glSecondaryColor3ubvEXT"))) return false;
	if(!(glSecondaryColor3uiEXT= (PFNGLSECONDARYCOLOR3UIEXTPROC)nelglGetProcAddress("glSecondaryColor3uiEXT"))) return false;
	if(!(glSecondaryColor3uivEXT= (PFNGLSECONDARYCOLOR3UIVEXTPROC)nelglGetProcAddress("glSecondaryColor3uivEXT"))) return false;
	if(!(glSecondaryColor3usEXT= (PFNGLSECONDARYCOLOR3USEXTPROC)nelglGetProcAddress("glSecondaryColor3usEXT"))) return false;
	if(!(glSecondaryColor3usvEXT= (PFNGLSECONDARYCOLOR3USVEXTPROC)nelglGetProcAddress("glSecondaryColor3usvEXT"))) return false;
	if(!(glSecondaryColorPointerEXT= (PFNGLSECONDARYCOLORPOINTEREXTPROC)nelglGetProcAddress("glSecondaryColorPointerEXT"))) return false;


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




