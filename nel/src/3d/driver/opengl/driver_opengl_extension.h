/** \file driver_opengl_extension.h
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.h,v 1.14 2001/09/06 07:25:38 corvazier Exp $
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

#else // NL_OS_UNIX

#define	GL_GLEXT_PROTOTYPES
#include <GL/glx.h>

#endif // NL_OS_UNIX


#include <GL/gl.h>
#include <GL/glext.h>	// Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"

#ifndef GL_GLEXT_VERSION
#error "I need a newer <GL/glext.h>. Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"
#endif // GL_GLEXT_VERSION

#if GL_GLEXT_VERSION < 7
#error "I need a newer <GL/glext.h>. Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"
#endif // GL_GLEXT_VERSION < 7

#include "driver_opengl_extension_def.h"

namespace	NL3D
{


// ***************************************************************************
/// The extensions used by NL3D.
struct	CGlExtensions
{
	// Is this driver a correct OpenGL 1.2 driver?
	bool	Version1_2;

	// Required Extensions.
	bool	ARBMultiTexture;
	sint	NbTextureStages;
	bool	EXTTextureEnvCombine;

	// Optionnal Extensions.
	bool	ARBTextureCompression;
	bool	NVVertexArrayRange;
	uint	NVVertexArrayRangeMaxVertex;
	bool	EXTTextureCompressionS3TC;
	bool	EXTVertexWeighting;
	bool	EXTSeparateSpecularColor;
	bool	NVTextureEnvCombine4;
	bool	ARBTextureCubeMap;
	bool	NVVertexProgram;

public:
	CGlExtensions()
	{
		// Fill all false by default.
		Version1_2= false;
		ARBMultiTexture= false;
		NbTextureStages= 1;
		EXTTextureEnvCombine= false;
		ARBTextureCompression= false;
		NVVertexArrayRange= false;
		NVVertexArrayRangeMaxVertex= 0;
		EXTTextureCompressionS3TC= false;
		EXTVertexWeighting= false;
		EXTSeparateSpecularColor= false;
		NVTextureEnvCombine4= false;
		ARBTextureCubeMap= false;
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
extern PFNGLFLUSHVERTEXARRAYRANGENVPROC		glFlushVertexArrayRangeNV;
extern PFNGLVERTEXARRAYRANGENVPROC			glVertexArrayRangeNV;
extern PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
extern PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;


// VertexWeighting.
//==================
extern PFNGLVERTEXWEIGHTFEXTPROC			glVertexWeightfEXT;
extern PFNGLVERTEXWEIGHTFVEXTPROC			glVertexWeightfvEXT;
extern PFNGLVERTEXWEIGHTPOINTEREXTPROC		glVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
extern PFNGLAREPROGRAMSRESIDENTNVPROC		glAreProgramsResidentNV;
extern PFNGLBINDPROGRAMNVPROC				glBindProgramNV;
extern PFNGLDELETEPROGRAMSNVPROC			glDeleteProgramsNV;
extern PFNGLEXECUTEPROGRAMNVPROC			glExecuteProgramNV;
extern PFNGLGENPROGRAMSNVPROC				glGenProgramsNV;
extern PFNGLGETPROGRAMPARAMETERDVNVPROC		glGetProgramParameterdvNV;
extern PFNGLGETPROGRAMPARAMETERFVNVPROC		glGetProgramParameterfvNV;
extern PFNGLGETPROGRAMIVNVPROC				glGetProgramivNV;
extern PFNGLGETPROGRAMSTRINGNVPROC			glGetProgramStringNV;
extern PFNGLGETTRACKMATRIXIVNVPROC			glGetTrackMatrixivNV;
extern PFNGLGETVERTEXATTRIBDVNVPROC			glGetVertexAttribdvNV;
extern PFNGLGETVERTEXATTRIBFVNVPROC			glGetVertexAttribfvNV;
extern PFNGLGETVERTEXATTRIBIVNVPROC			glGetVertexAttribivNV;
extern PFNGLGETVERTEXATTRIBPOINTERVNVPROC	glGetVertexAttribPointervNV;
extern PFNGLISPROGRAMNVPROC					glIsProgramNV;
extern PFNGLLOADPROGRAMNVPROC				glLoadProgramNV;
extern PFNGLPROGRAMPARAMETER4DNVPROC		glProgramParameter4dNV;
extern PFNGLPROGRAMPARAMETER4DVNVPROC		glProgramParameter4dvNV;
extern PFNGLPROGRAMPARAMETER4FNVPROC		glProgramParameter4fNV;
extern PFNGLPROGRAMPARAMETER4FVNVPROC		glProgramParameter4fvNV;
extern PFNGLPROGRAMPARAMETERS4DVNVPROC		glProgramParameters4dvNV;
extern PFNGLPROGRAMPARAMETERS4FVNVPROC		glProgramParameters4fvNV;
extern PFNGLREQUESTRESIDENTPROGRAMSNVPROC	glRequestResidentProgramsNV;
extern PFNGLTRACKMATRIXNVPROC				glTrackMatrixNV;
extern PFNGLVERTEXATTRIBPOINTERNVPROC		glVertexAttribPointerNV;
extern PFNGLVERTEXATTRIB1DNVPROC			glVertexAttrib1dNV;
extern PFNGLVERTEXATTRIB1DVNVPROC			glVertexAttrib1dvNV;
extern PFNGLVERTEXATTRIB1FNVPROC			glVertexAttrib1fNV;
extern PFNGLVERTEXATTRIB1FVNVPROC			glVertexAttrib1fvNV;
extern PFNGLVERTEXATTRIB1SNVPROC			glVertexAttrib1sNV;
extern PFNGLVERTEXATTRIB1SVNVPROC			glVertexAttrib1svNV;
extern PFNGLVERTEXATTRIB2DNVPROC			glVertexAttrib2dNV;
extern PFNGLVERTEXATTRIB2DVNVPROC			glVertexAttrib2dvNV;
extern PFNGLVERTEXATTRIB2FNVPROC			glVertexAttrib2fNV;
extern PFNGLVERTEXATTRIB2FVNVPROC			glVertexAttrib2fvNV;
extern PFNGLVERTEXATTRIB2SNVPROC			glVertexAttrib2sNV;
extern PFNGLVERTEXATTRIB2SVNVPROC			glVertexAttrib2svNV;
extern PFNGLVERTEXATTRIB3DNVPROC			glVertexAttrib3dNV;
extern PFNGLVERTEXATTRIB3DVNVPROC			glVertexAttrib3dvNV;
extern PFNGLVERTEXATTRIB3FNVPROC			glVertexAttrib3fNV;
extern PFNGLVERTEXATTRIB3FVNVPROC			glVertexAttrib3fvNV;
extern PFNGLVERTEXATTRIB3SNVPROC			glVertexAttrib3sNV;
extern PFNGLVERTEXATTRIB3SVNVPROC			glVertexAttrib3svNV;
extern PFNGLVERTEXATTRIB4DNVPROC			glVertexAttrib4dNV;
extern PFNGLVERTEXATTRIB4DVNVPROC			glVertexAttrib4dvNV;
extern PFNGLVERTEXATTRIB4FNVPROC			glVertexAttrib4fNV;
extern PFNGLVERTEXATTRIB4FVNVPROC			glVertexAttrib4fvNV;
extern PFNGLVERTEXATTRIB4SNVPROC			glVertexAttrib4sNV;
extern PFNGLVERTEXATTRIB4SVNVPROC			glVertexAttrib4svNV;
extern PFNGLVERTEXATTRIB4UBVNVPROC			glVertexAttrib4ubvNV;
extern PFNGLVERTEXATTRIBS1DVNVPROC			glVertexAttribs1dvNV;
extern PFNGLVERTEXATTRIBS1FVNVPROC			glVertexAttribs1fvNV;
extern PFNGLVERTEXATTRIBS1SVNVPROC			glVertexAttribs1svNV;
extern PFNGLVERTEXATTRIBS2DVNVPROC			glVertexAttribs2dvNV;
extern PFNGLVERTEXATTRIBS2FVNVPROC			glVertexAttribs2fvNV;
extern PFNGLVERTEXATTRIBS2SVNVPROC			glVertexAttribs2svNV;
extern PFNGLVERTEXATTRIBS3DVNVPROC			glVertexAttribs3dvNV;
extern PFNGLVERTEXATTRIBS3FVNVPROC			glVertexAttribs3fvNV;
extern PFNGLVERTEXATTRIBS3SVNVPROC			glVertexAttribs3svNV;
extern PFNGLVERTEXATTRIBS4DVNVPROC			glVertexAttribs4dvNV;
extern PFNGLVERTEXATTRIBS4FVNVPROC			glVertexAttribs4fvNV;
extern PFNGLVERTEXATTRIBS4SVNVPROC			glVertexAttribs4svNV;
extern PFNGLVERTEXATTRIBS4UBVNVPROC			glVertexAttribs4ubvNV;


#endif


#endif // NL_OPENGL_EXTENSION_H

