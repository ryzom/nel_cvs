/** \file driver_opengl_extension.cpp
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.cpp,v 1.22 2001/12/05 09:54:38 corvazier Exp $
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
PFNWGLCREATEPBUFFERARBPROC			wglCreatePbufferARB;
PFNWGLGETPUFFERDCARBPROC			wglGetPbufferDCARB;
PFNWGLRELEASEPUFFERDCARBPROC		wglReleasePbufferDCARB;
PFNWGLDESTROYPUFFERARBPROC			wglDestroyPbufferARB;
PFNWGLQUERYPBUFFERARBPROC			wglQueryPbufferARB;


// Get Pixel format extension
//===========================
PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribivARB;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfvARB;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB;


// WGL_ARB_extensions_string
PFNWGFGETEXTENSIONSSTRINGARB		wglGetExtensionsStringARB;


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


	// Test if VAR is present.
	if(strstr(glext, "GL_NV_vertex_array_range")==NULL)
		return false;
	// Tess Fence too.
	if( strstr(glext, "GL_NV_fence")==NULL )
		return false;

#ifdef NL_OS_WINDOWS
	// Get VAR adress.
	if(!(glFlushVertexArrayRangeNV=(PFNGLFLUSHVERTEXARRAYRANGENVPROC)wglGetProcAddress("glFlushVertexArrayRangeNV")))return false;
	if(!(glVertexArrayRangeNV=(PFNGLVERTEXARRAYRANGENVPROC)wglGetProcAddress("glVertexArrayRangeNV")))return false;
	if(!(wglAllocateMemoryNV= (PFNWGLALLOCATEMEMORYNVPROC)wglGetProcAddress("wglAllocateMemoryNV")))return false;
	if(!(wglFreeMemoryNV= (PFNWGLFREEMEMORYNVPROC)wglGetProcAddress("wglFreeMemoryNV")))return false;

	// Get fence adress.
	if(!(glDeleteFencesNV= (PFNGLDELETEFENCESNVPROC)wglGetProcAddress("glDeleteFencesNV")))return false;
	if(!(glGenFencesNV= (PFNGLGENFENCESNVPROC)wglGetProcAddress("glGenFencesNV")))return false;
	if(!(glIsFenceNV= (PFNGLISFENCENVPROC)wglGetProcAddress("glIsFenceNV")))return false;
	if(!(glTestFenceNV= (PFNGLTESTFENCENVPROC)wglGetProcAddress("glTestFenceNV")))return false;
	if(!(glGetFenceivNV= (PFNGLGETFENCEIVNVPROC)wglGetProcAddress("glGetFenceivNV")))return false;
	if(!(glFinishFenceNV= (PFNGLFINISHFENCENVPROC)wglGetProcAddress("glFinishFenceNV")))return false;
	if(!(glSetFenceNV= (PFNGLSETFENCENVPROC)wglGetProcAddress("glSetFenceNV")))return false;

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


// *********************************
static bool	setupNVVertexProgram(const char	*glext)
{
	if(strstr(glext, "GL_NV_vertex_program")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(glAreProgramsResidentNV= (PFNGLAREPROGRAMSRESIDENTNVPROC)wglGetProcAddress("glAreProgramsResidentNV"))) return false;
	if(!(glBindProgramNV= (PFNGLBINDPROGRAMNVPROC)wglGetProcAddress("glBindProgramNV"))) return false;
	if(!(glDeleteProgramsNV= (PFNGLDELETEPROGRAMSNVPROC)wglGetProcAddress("glDeleteProgramsNV"))) return false;
	if(!(glExecuteProgramNV= (PFNGLEXECUTEPROGRAMNVPROC)wglGetProcAddress("glExecuteProgramNV"))) return false;
	if(!(glGenProgramsNV= (PFNGLGENPROGRAMSNVPROC)wglGetProcAddress("glGenProgramsNV"))) return false;
	if(!(glGetProgramParameterdvNV= (PFNGLGETPROGRAMPARAMETERDVNVPROC)wglGetProcAddress("glGetProgramParameterdvNV"))) return false;
	if(!(glGetProgramParameterfvNV= (PFNGLGETPROGRAMPARAMETERFVNVPROC)wglGetProcAddress("glGetProgramParameterfvNV"))) return false;
	if(!(glGetProgramivNV= (PFNGLGETPROGRAMIVNVPROC)wglGetProcAddress("glGetProgramivNV"))) return false;
	if(!(glGetProgramStringNV= (PFNGLGETPROGRAMSTRINGNVPROC)wglGetProcAddress("glGetProgramStringNV"))) return false;
	if(!(glGetTrackMatrixivNV= (PFNGLGETTRACKMATRIXIVNVPROC)wglGetProcAddress("glGetTrackMatrixivNV"))) return false;
	if(!(glGetVertexAttribdvNV= (PFNGLGETVERTEXATTRIBDVNVPROC)wglGetProcAddress("glGetVertexAttribdvNV"))) return false;
	if(!(glGetVertexAttribfvNV= (PFNGLGETVERTEXATTRIBFVNVPROC)wglGetProcAddress("glGetVertexAttribfvNV"))) return false;
	if(!(glGetVertexAttribivNV= (PFNGLGETVERTEXATTRIBIVNVPROC)wglGetProcAddress("glGetVertexAttribivNV"))) return false;
	if(!(glGetVertexAttribPointervNV= (PFNGLGETVERTEXATTRIBPOINTERVNVPROC)wglGetProcAddress("glGetVertexAttribPointervNV"))) return false;
	if(!(glIsProgramNV= (PFNGLISPROGRAMNVPROC)wglGetProcAddress("glIsProgramNV"))) return false;
	if(!(glLoadProgramNV= (PFNGLLOADPROGRAMNVPROC)wglGetProcAddress("glLoadProgramNV"))) return false;
	if(!(glProgramParameter4dNV= (PFNGLPROGRAMPARAMETER4DNVPROC)wglGetProcAddress("glProgramParameter4dNV"))) return false;
	if(!(glProgramParameter4dvNV= (PFNGLPROGRAMPARAMETER4DVNVPROC)wglGetProcAddress("glProgramParameter4dvNV"))) return false;
	if(!(glProgramParameter4fNV= (PFNGLPROGRAMPARAMETER4FNVPROC)wglGetProcAddress("glProgramParameter4fNV"))) return false;
	if(!(glProgramParameter4fvNV= (PFNGLPROGRAMPARAMETER4FVNVPROC)wglGetProcAddress("glProgramParameter4fvNV"))) return false;
	if(!(glProgramParameters4dvNV= (PFNGLPROGRAMPARAMETERS4DVNVPROC)wglGetProcAddress("glProgramParameters4dvNV"))) return false;
	if(!(glProgramParameters4fvNV= (PFNGLPROGRAMPARAMETERS4FVNVPROC)wglGetProcAddress("glProgramParameters4fvNV"))) return false;
	if(!(glRequestResidentProgramsNV= (PFNGLREQUESTRESIDENTPROGRAMSNVPROC)wglGetProcAddress("glRequestResidentProgramsNV"))) return false;
	if(!(glTrackMatrixNV= (PFNGLTRACKMATRIXNVPROC)wglGetProcAddress("glTrackMatrixNV"))) return false;
	if(!(glVertexAttribPointerNV= (PFNGLVERTEXATTRIBPOINTERNVPROC)wglGetProcAddress("glVertexAttribPointerNV"))) return false;
	if(!(glVertexAttrib1dNV= (PFNGLVERTEXATTRIB1DNVPROC)wglGetProcAddress("glVertexAttrib1dNV"))) return false;
	if(!(glVertexAttrib1dvNV= (PFNGLVERTEXATTRIB1DVNVPROC)wglGetProcAddress("glVertexAttrib1dvNV"))) return false;
	if(!(glVertexAttrib1fNV= (PFNGLVERTEXATTRIB1FNVPROC)wglGetProcAddress("glVertexAttrib1fNV"))) return false;
	if(!(glVertexAttrib1fvNV= (PFNGLVERTEXATTRIB1FVNVPROC)wglGetProcAddress("glVertexAttrib1fvNV"))) return false;
	if(!(glVertexAttrib1sNV= (PFNGLVERTEXATTRIB1SNVPROC)wglGetProcAddress("glVertexAttrib1sNV"))) return false;
	if(!(glVertexAttrib1svNV= (PFNGLVERTEXATTRIB1SVNVPROC)wglGetProcAddress("glVertexAttrib1svNV"))) return false;
	if(!(glVertexAttrib2dNV= (PFNGLVERTEXATTRIB2DNVPROC)wglGetProcAddress("glVertexAttrib2dNV"))) return false;
	if(!(glVertexAttrib2dvNV= (PFNGLVERTEXATTRIB2DVNVPROC)wglGetProcAddress("glVertexAttrib2dvNV"))) return false;
	if(!(glVertexAttrib2fNV= (PFNGLVERTEXATTRIB2FNVPROC)wglGetProcAddress("glVertexAttrib2fNV"))) return false;
	if(!(glVertexAttrib2fvNV= (PFNGLVERTEXATTRIB2FVNVPROC)wglGetProcAddress("glVertexAttrib2fvNV"))) return false;
	if(!(glVertexAttrib2sNV= (PFNGLVERTEXATTRIB2SNVPROC)wglGetProcAddress("glVertexAttrib2sNV"))) return false;
	if(!(glVertexAttrib2svNV= (PFNGLVERTEXATTRIB2SVNVPROC)wglGetProcAddress("glVertexAttrib2svNV"))) return false;
	if(!(glVertexAttrib3dNV= (PFNGLVERTEXATTRIB3DNVPROC)wglGetProcAddress("glVertexAttrib3dNV"))) return false;
	if(!(glVertexAttrib3dvNV= (PFNGLVERTEXATTRIB3DVNVPROC)wglGetProcAddress("glVertexAttrib3dvNV"))) return false;
	if(!(glVertexAttrib3fNV= (PFNGLVERTEXATTRIB3FNVPROC)wglGetProcAddress("glVertexAttrib3fNV"))) return false;
	if(!(glVertexAttrib3fvNV= (PFNGLVERTEXATTRIB3FVNVPROC)wglGetProcAddress("glVertexAttrib3fvNV"))) return false;
	if(!(glVertexAttrib3sNV= (PFNGLVERTEXATTRIB3SNVPROC)wglGetProcAddress("glVertexAttrib3sNV"))) return false;
	if(!(glVertexAttrib3svNV= (PFNGLVERTEXATTRIB3SVNVPROC)wglGetProcAddress("glVertexAttrib3svNV"))) return false;
	if(!(glVertexAttrib4dNV= (PFNGLVERTEXATTRIB4DNVPROC)wglGetProcAddress("glVertexAttrib4dNV"))) return false;
	if(!(glVertexAttrib4dvNV= (PFNGLVERTEXATTRIB4DVNVPROC)wglGetProcAddress("glVertexAttrib4dvNV"))) return false;
	if(!(glVertexAttrib4fNV= (PFNGLVERTEXATTRIB4FNVPROC)wglGetProcAddress("glVertexAttrib4fNV"))) return false;
	if(!(glVertexAttrib4fvNV= (PFNGLVERTEXATTRIB4FVNVPROC)wglGetProcAddress("glVertexAttrib4fvNV"))) return false;
	if(!(glVertexAttrib4sNV= (PFNGLVERTEXATTRIB4SNVPROC)wglGetProcAddress("glVertexAttrib4sNV"))) return false;
	if(!(glVertexAttrib4svNV= (PFNGLVERTEXATTRIB4SVNVPROC)wglGetProcAddress("glVertexAttrib4svNV"))) return false;
	if(!(glVertexAttrib4ubvNV= (PFNGLVERTEXATTRIB4UBVNVPROC)wglGetProcAddress("glVertexAttrib4ubvNV"))) return false;
	if(!(glVertexAttribs1dvNV= (PFNGLVERTEXATTRIBS1DVNVPROC)wglGetProcAddress("glVertexAttribs1dvNV"))) return false;
	if(!(glVertexAttribs1fvNV= (PFNGLVERTEXATTRIBS1FVNVPROC)wglGetProcAddress("glVertexAttribs1fvNV"))) return false;
	if(!(glVertexAttribs1svNV= (PFNGLVERTEXATTRIBS1SVNVPROC)wglGetProcAddress("glVertexAttribs1svNV"))) return false;
	if(!(glVertexAttribs2dvNV= (PFNGLVERTEXATTRIBS2DVNVPROC)wglGetProcAddress("glVertexAttribs2dvNV"))) return false;
	if(!(glVertexAttribs2fvNV= (PFNGLVERTEXATTRIBS2FVNVPROC)wglGetProcAddress("glVertexAttribs2fvNV"))) return false;
	if(!(glVertexAttribs2svNV= (PFNGLVERTEXATTRIBS2SVNVPROC)wglGetProcAddress("glVertexAttribs2svNV"))) return false;
	if(!(glVertexAttribs3dvNV= (PFNGLVERTEXATTRIBS3DVNVPROC)wglGetProcAddress("glVertexAttribs3dvNV"))) return false;
	if(!(glVertexAttribs3fvNV= (PFNGLVERTEXATTRIBS3FVNVPROC)wglGetProcAddress("glVertexAttribs3fvNV"))) return false;
	if(!(glVertexAttribs3svNV= (PFNGLVERTEXATTRIBS3SVNVPROC)wglGetProcAddress("glVertexAttribs3svNV"))) return false;
	if(!(glVertexAttribs4dvNV= (PFNGLVERTEXATTRIBS4DVNVPROC)wglGetProcAddress("glVertexAttribs4dvNV"))) return false;
	if(!(glVertexAttribs4fvNV= (PFNGLVERTEXATTRIBS4FVNVPROC)wglGetProcAddress("glVertexAttribs4fvNV"))) return false;
	if(!(glVertexAttribs4svNV= (PFNGLVERTEXATTRIBS4SVNVPROC)wglGetProcAddress("glVertexAttribs4svNV"))) return false;
	if(!(glVertexAttribs4ubvNV= (PFNGLVERTEXATTRIBS4UBVNVPROC)wglGetProcAddress("glVertexAttribs4ubvNV"))) return false;

#endif

	return true;
}


// *********************************
static bool	setupEXTSecondaryColor(const char	*glext)
{
	if(strstr(glext, "GL_EXT_secondary_color")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(glSecondaryColor3bEXT= (PFNGLSECONDARYCOLOR3BEXTPROC)wglGetProcAddress("glSecondaryColor3bEXT"))) return false;
	if(!(glSecondaryColor3bvEXT= (PFNGLSECONDARYCOLOR3BVEXTPROC)wglGetProcAddress("glSecondaryColor3bvEXT"))) return false;
	if(!(glSecondaryColor3dEXT= (PFNGLSECONDARYCOLOR3DEXTPROC)wglGetProcAddress("glSecondaryColor3dEXT"))) return false;
	if(!(glSecondaryColor3dvEXT= (PFNGLSECONDARYCOLOR3DVEXTPROC)wglGetProcAddress("glSecondaryColor3dvEXT"))) return false;
	if(!(glSecondaryColor3fEXT= (PFNGLSECONDARYCOLOR3FEXTPROC)wglGetProcAddress("glSecondaryColor3fEXT"))) return false;
	if(!(glSecondaryColor3fvEXT= (PFNGLSECONDARYCOLOR3FVEXTPROC)wglGetProcAddress("glSecondaryColor3fvEXT"))) return false;
	if(!(glSecondaryColor3iEXT= (PFNGLSECONDARYCOLOR3IEXTPROC)wglGetProcAddress("glSecondaryColor3iEXT"))) return false;
	if(!(glSecondaryColor3ivEXT= (PFNGLSECONDARYCOLOR3IVEXTPROC)wglGetProcAddress("glSecondaryColor3ivEXT"))) return false;
	if(!(glSecondaryColor3sEXT= (PFNGLSECONDARYCOLOR3SEXTPROC)wglGetProcAddress("glSecondaryColor3sEXT"))) return false;
	if(!(glSecondaryColor3svEXT= (PFNGLSECONDARYCOLOR3SVEXTPROC)wglGetProcAddress("glSecondaryColor3svEXT"))) return false;
	if(!(glSecondaryColor3ubEXT= (PFNGLSECONDARYCOLOR3UBEXTPROC)wglGetProcAddress("glSecondaryColor3ubEXT"))) return false;
	if(!(glSecondaryColor3ubvEXT= (PFNGLSECONDARYCOLOR3UBVEXTPROC)wglGetProcAddress("glSecondaryColor3ubvEXT"))) return false;
	if(!(glSecondaryColor3uiEXT= (PFNGLSECONDARYCOLOR3UIEXTPROC)wglGetProcAddress("glSecondaryColor3uiEXT"))) return false;
	if(!(glSecondaryColor3uivEXT= (PFNGLSECONDARYCOLOR3UIVEXTPROC)wglGetProcAddress("glSecondaryColor3uivEXT"))) return false;
	if(!(glSecondaryColor3usEXT= (PFNGLSECONDARYCOLOR3USEXTPROC)wglGetProcAddress("glSecondaryColor3usEXT"))) return false;
	if(!(glSecondaryColor3usvEXT= (PFNGLSECONDARYCOLOR3USVEXTPROC)wglGetProcAddress("glSecondaryColor3usvEXT"))) return false;
	if(!(glSecondaryColorPointerEXT= (PFNGLSECONDARYCOLORPOINTEREXTPROC)wglGetProcAddress("glSecondaryColorPointerEXT"))) return false;

#endif

	return true;
}

// *********************************
static bool	setupWGLARBPBuffer(const char	*glext)
{
	if(strstr(glext, "WGL_ARB_pbuffer")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(wglCreatePbufferARB= (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB"))) return false;
	if(!(wglGetPbufferDCARB= (PFNWGLGETPUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB"))) return false;
	if(!(wglReleasePbufferDCARB= (PFNWGLRELEASEPUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB"))) return false;
	if(!(wglDestroyPbufferARB= (PFNWGLDESTROYPUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB"))) return false;
	if(!(wglQueryPbufferARB= (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB"))) return false;
#endif

	return true;
}

// *********************************
static bool	setupWGLARBPixelFormat (const char	*glext)
{
	if(strstr(glext, "WGL_ARB_pixel_format")==NULL)
		return false;

#ifdef NL_OS_WINDOWS
	if(!(wglGetPixelFormatAttribivARB= (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB"))) return false;
	if(!(wglGetPixelFormatAttribfvARB= (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB"))) return false;
	if(!(wglChoosePixelFormatARB= (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB"))) return false;
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
	ext.NVVertexProgram = setupNVVertexProgram(glext);

	// Check texture shaders
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
	if(!(wglGetExtensionsStringARB=(PFNWGFGETEXTENSIONSSTRINGARB)wglGetProcAddress("wglGetExtensionsStringARB")))
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




