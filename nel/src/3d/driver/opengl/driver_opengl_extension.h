/** \file driver_opengl_extension.h
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.h,v 1.5 2001/01/16 14:46:56 berenguier Exp $
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

#ifndef NL_OPENGL_EXTENSION_H
#define NL_OPENGL_EXTENSION_H


#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <GL/glx.h>
#endif // NL_OS_WINDOWS


#include <GL/gl.h>
#ifdef NL_OS_UNIX
#define	GL_GLEXT_PROTOTYPES
#endif
#include <GL/glext.h>	// Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"

#ifdef GL_GLEXT_VERSION_EXT			// Old glext.h define GL_GLEXT_VERSION_EXT not GL_GLEXT_VERSION
  #if GL_GLEXT_VERSION_EXT < 7
    #error "I need a newer <GL/glext.h>. Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"
  #endif // GL_GLEXT_VERSION_EXT < 7
#else // GL_GLEXT_VERSION_EXT
  #if GL_GLEXT_VERSION < 7
    #error "I need a newer <GL/glext.h>. Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"
  #endif // GL_GLEXT_VERSION < 7
#endif 

namespace	NL3D
{


// ***************************************************************************
/// The extensions used by NL3D.
struct	CGlExtensions
{
	// Required Extensions.
	bool	ARBMultiTexture;
	sint	NbTextureStages;
	bool	EXTTextureEnvCombine;

	// Optionnal Extensions.
	bool	ARBTextureCompression;
	bool	NVVertexArrayRange;
	bool	EXTTextureCompressionS3TC;


public:
	CGlExtensions()
	{
		// Fill all false by default.
		ARBMultiTexture= false;
		NbTextureStages= 1;
		EXTTextureEnvCombine= false;
		ARBTextureCompression= false;
		NVVertexArrayRange= false;
		EXTTextureCompressionS3TC= false;
	};
};


// ***************************************************************************
/// This function test and register the extensions for the current GL context.
void	registerGlExtensions(CGlExtensions &ext);


}



// ***************************************************************************
// The exported function names for the windows platform.
#ifdef NL_OS_WINDOWS


// ARB_multitexture
//=================
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

extern PFNGLMULTITEXCOORD1SARBPROC glMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1IARBPROC glMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1DARBPROC glMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD2SARBPROC glMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2IARBPROC glMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2DARBPROC glMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD3SARBPROC glMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3IARBPROC glMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3DARBPROC glMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD4SARBPROC glMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4IARBPROC glMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4DARBPROC glMultiTexCoord4dARB;

extern PFNGLMULTITEXCOORD1SVARBPROC glMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD1IVARBPROC glMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1DVARBPROC glMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD2SVARBPROC glMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD2IVARBPROC glMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2DVARBPROC glMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD3SVARBPROC glMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD3IVARBPROC glMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3DVARBPROC glMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD4SVARBPROC glMultiTexCoord4svARB;
extern PFNGLMULTITEXCOORD4IVARBPROC glMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4FVARBPROC glMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4DVARBPROC glMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC		glCompressedTexImage3DARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		glCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC		glCompressedTexImage1DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	glCompressedTexSubImage3DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	glCompressedTexSubImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	glCompressedTexSubImage1DARB;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	glGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
typedef void *(APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (GLsizei size, GLfloat readFrequency, GLfloat writeFrequency, GLfloat priority);
extern PFNGLFLUSHVERTEXARRAYRANGENVPROC	glFlushVertexArrayRangeNV;
extern PFNGLVERTEXARRAYRANGENVPROC		glVertexArrayRangeNV;
extern PFNWGLALLOCATEMEMORYNVPROC		wglAllocateMemoryNV;


#endif


#endif // NL_OPENGL_EXTENSION_H

