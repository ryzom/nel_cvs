/** \file driver_opengl_extension.cpp
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.cpp,v 1.11 2001/07/06 12:22:55 berenguier Exp $
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


#include "nel/misc/debug.h"

#include "driver_opengl_extension.h"



// ***************************************************************************
// The exported function names for the windows platform.
#ifdef NL_OS_WINDOWS

// ARB_multitexture
//=================
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
PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;


// VertexWeighting.
//====================
PFNGLVERTEXWEIGHTFEXTPROC			glVertexWeightfEXT;
PFNGLVERTEXWEIGHTFVEXTPROC			glVertexWeightfvEXT;
PFNGLVERTEXWEIGHTPOINTEREXTPROC		glVertexWeightPointerEXT;

#endif // NL_OS_WINDOWS




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

#ifdef NL_OS_WINDOWS
	if(!(glActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB")))return false;
	if(!(glClientActiveTextureARB=(PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB")))return false;

	if(!(glMultiTexCoord1sARB=(PFNGLMULTITEXCOORD1SARBPROC)wglGetProcAddress("glMultiTexCoord1sARB")))return false;
	if(!(glMultiTexCoord1iARB=(PFNGLMULTITEXCOORD1IARBPROC)wglGetProcAddress("glMultiTexCoord1iARB")))return false;
	if(!(glMultiTexCoord1fARB=(PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB")))return false;
	if(!(glMultiTexCoord1dARB=(PFNGLMULTITEXCOORD1DARBPROC)wglGetProcAddress("glMultiTexCoord1dARB")))return false;
	if(!(glMultiTexCoord2sARB=(PFNGLMULTITEXCOORD2SARBPROC)wglGetProcAddress("glMultiTexCoord2sARB")))return false;
	if(!(glMultiTexCoord2iARB=(PFNGLMULTITEXCOORD2IARBPROC)wglGetProcAddress("glMultiTexCoord2iARB")))return false;
	if(!(glMultiTexCoord2fARB=(PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB")))return false;
	if(!(glMultiTexCoord2dARB=(PFNGLMULTITEXCOORD2DARBPROC)wglGetProcAddress("glMultiTexCoord2dARB")))return false;
	if(!(glMultiTexCoord3sARB=(PFNGLMULTITEXCOORD3SARBPROC)wglGetProcAddress("glMultiTexCoord3sARB")))return false;
	if(!(glMultiTexCoord3iARB=(PFNGLMULTITEXCOORD3IARBPROC)wglGetProcAddress("glMultiTexCoord3iARB")))return false;
	if(!(glMultiTexCoord3fARB=(PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB")))return false;
	if(!(glMultiTexCoord3dARB=(PFNGLMULTITEXCOORD3DARBPROC)wglGetProcAddress("glMultiTexCoord3dARB")))return false;
	if(!(glMultiTexCoord4sARB=(PFNGLMULTITEXCOORD4SARBPROC)wglGetProcAddress("glMultiTexCoord4sARB")))return false;
	if(!(glMultiTexCoord4iARB=(PFNGLMULTITEXCOORD4IARBPROC)wglGetProcAddress("glMultiTexCoord4iARB")))return false;
	if(!(glMultiTexCoord4fARB=(PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB")))return false;
	if(!(glMultiTexCoord4dARB=(PFNGLMULTITEXCOORD4DARBPROC)wglGetProcAddress("glMultiTexCoord4dARB")))return false;

	if(!(glMultiTexCoord1svARB=(PFNGLMULTITEXCOORD1SVARBPROC)wglGetProcAddress("glMultiTexCoord1svARB")))return false;
	if(!(glMultiTexCoord1ivARB=(PFNGLMULTITEXCOORD1IVARBPROC)wglGetProcAddress("glMultiTexCoord1ivARB")))return false;
	if(!(glMultiTexCoord1fvARB=(PFNGLMULTITEXCOORD1FVARBPROC)wglGetProcAddress("glMultiTexCoord1fvARB")))return false;
	if(!(glMultiTexCoord1dvARB=(PFNGLMULTITEXCOORD1DVARBPROC)wglGetProcAddress("glMultiTexCoord1dvARB")))return false;
	if(!(glMultiTexCoord2svARB=(PFNGLMULTITEXCOORD2SVARBPROC)wglGetProcAddress("glMultiTexCoord2svARB")))return false;
	if(!(glMultiTexCoord2ivARB=(PFNGLMULTITEXCOORD2IVARBPROC)wglGetProcAddress("glMultiTexCoord2ivARB")))return false;
	if(!(glMultiTexCoord2fvARB=(PFNGLMULTITEXCOORD2FVARBPROC)wglGetProcAddress("glMultiTexCoord2fvARB")))return false;
	if(!(glMultiTexCoord2dvARB=(PFNGLMULTITEXCOORD2DVARBPROC)wglGetProcAddress("glMultiTexCoord2dvARB")))return false;
	if(!(glMultiTexCoord3svARB=(PFNGLMULTITEXCOORD3SVARBPROC)wglGetProcAddress("glMultiTexCoord3svARB")))return false;
	if(!(glMultiTexCoord3ivARB=(PFNGLMULTITEXCOORD3IVARBPROC)wglGetProcAddress("glMultiTexCoord3ivARB")))return false;
	if(!(glMultiTexCoord3fvARB=(PFNGLMULTITEXCOORD3FVARBPROC)wglGetProcAddress("glMultiTexCoord3fvARB")))return false;
	if(!(glMultiTexCoord3dvARB=(PFNGLMULTITEXCOORD3DVARBPROC)wglGetProcAddress("glMultiTexCoord3dvARB")))return false;
	if(!(glMultiTexCoord4svARB=(PFNGLMULTITEXCOORD4SVARBPROC)wglGetProcAddress("glMultiTexCoord4svARB")))return false;
	if(!(glMultiTexCoord4ivARB=(PFNGLMULTITEXCOORD4IVARBPROC)wglGetProcAddress("glMultiTexCoord4ivARB")))return false;
	if(!(glMultiTexCoord4fvARB=(PFNGLMULTITEXCOORD4FVARBPROC)wglGetProcAddress("glMultiTexCoord4fvARB")))return false;
	if(!(glMultiTexCoord4dvARB=(PFNGLMULTITEXCOORD4DVARBPROC)wglGetProcAddress("glMultiTexCoord4dvARB")))return false;
#endif

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

#ifdef NL_OS_WINDOWS
	if(!(glCompressedTexImage3DARB=(PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)wglGetProcAddress("glCompressedTexImage3DARB")))return false;
	if(!(glCompressedTexImage2DARB=(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)wglGetProcAddress("glCompressedTexImage2DARB")))return false;
	if(!(glCompressedTexImage1DARB=(PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)wglGetProcAddress("glCompressedTexImage1DARB")))return false;
	if(!(glCompressedTexSubImage3DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)wglGetProcAddress("glCompressedTexSubImage3DARB")))return false;
	if(!(glCompressedTexSubImage2DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)wglGetProcAddress("glCompressedTexSubImage2DARB")))return false;
	if(!(glCompressedTexSubImage1DARB=(PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)wglGetProcAddress("glCompressedTexSubImage1DARB")))return false;
	if(!(glGetCompressedTexImageARB=(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)wglGetProcAddress("glGetCompressedTexImageARB")))return false;
#endif

	return true;
}


// *********************************
static bool	setupNVVertexArrayRange(const char	*glext)
{
	// \todo_yoyo: TODO_LINUX: for now, just return false.
#ifndef NL_OS_WINDOWS
	return false;
#endif


	if(strstr(glext, "GL_NV_vertex_array_range")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(glFlushVertexArrayRangeNV=(PFNGLFLUSHVERTEXARRAYRANGENVPROC)wglGetProcAddress("glFlushVertexArrayRangeNV")))return false;
	if(!(glVertexArrayRangeNV=(PFNGLVERTEXARRAYRANGENVPROC)wglGetProcAddress("glVertexArrayRangeNV")))return false;
	if(!(wglAllocateMemoryNV= (PFNWGLALLOCATEMEMORYNVPROC)wglGetProcAddress("wglAllocateMemoryNV")))return false;
	if(!(wglFreeMemoryNV= (PFNWGLFREEMEMORYNVPROC)wglGetProcAddress("wglFreeMemoryNV")))return false;
#endif

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

#ifdef NL_OS_WINDOWS
	if(!(glVertexWeightfEXT=(PFNGLVERTEXWEIGHTFEXTPROC)wglGetProcAddress("glVertexWeightfEXT")))return false;
	if(!(glVertexWeightfvEXT=(PFNGLVERTEXWEIGHTFVEXTPROC)wglGetProcAddress("glVertexWeightfvEXT")))return false;
	if(!(glVertexWeightPointerEXT=(PFNGLVERTEXWEIGHTPOINTEREXTPROC)wglGetProcAddress("glVertexWeightPointerEXT")))return false;
#endif

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
		ext.NbTextureStages= ntext;
	}

	// Check EXTTextureEnvCombine
	ext.EXTTextureEnvCombine= setupEXTTextureEnvCombine(glext);

	// Check ARBTextureCompression
	ext.ARBTextureCompression= setupARBTextureCompression(glext);

	// Check NVVertexArrayRange
	ext.NVVertexArrayRange= setupNVVertexArrayRange(glext);
	if(ext.NVVertexArrayRange)
	{
		GLint	nverts;
		glGetIntegerv(GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV, &nverts);
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
}


}


