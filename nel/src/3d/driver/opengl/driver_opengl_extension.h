/** \file driver_opengl_extension.h
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.h,v 1.26 2002/02/11 10:01:34 berenguier Exp $
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
	// NB: Fence extension is not here, because NVVertexArrayRange is false if GL_NV_fence is not here.
	bool	NVVertexArrayRange;
	uint	NVVertexArrayRangeMaxVertex;
	bool	EXTTextureCompressionS3TC;
	bool	EXTVertexWeighting;
	bool	EXTSeparateSpecularColor;
	bool	NVTextureEnvCombine4;
	bool	ARBTextureCubeMap;
	bool	NVVertexProgram;
	bool	NVTextureShader;
	// true if NVVertexProgram and if we know that VP is emulated
	bool	NVVertexProgramEmulated;
	bool	EXTSecondaryColor;

	// WGL ARB extensions, true if supported
	bool	WGLARBPBuffer;
	bool	WGLARBPixelFormat;


public:

	/// \name Disable Hardware feature. False by default. setuped by IDriver
	// @{
	bool				DisableHardwareVertexProgram;
	bool				DisableHardwareVertexArrayAGP;
	bool				DisableHardwareTextureShader;
	// @}


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
		NVTextureShader= false;
		NVVertexProgram= false;
		NVVertexProgramEmulated= false;
		EXTSecondaryColor= false;
		WGLARBPBuffer= false;
		WGLARBPixelFormat= false;

		/// \name Disable Hardware feature. False by default. setuped by IDriver
		DisableHardwareVertexProgram= false;
		DisableHardwareVertexArrayAGP= false;
		DisableHardwareTextureShader= false;
	};
};


// ***************************************************************************

#ifdef NL_OS_WINDOWS
/// This function will test and register WGL functions before than the gl context is created
void	registerWGlExtensions(CGlExtensions &ext, HDC hDC);
#endif // NL_OS_WINDOWS

/// This function test and register the extensions for the current GL context.
void	registerGlExtensions(CGlExtensions &ext);


}



// ***************************************************************************
// The exported function names
/* NB: We named all like nglActiveTextureARB (n for NEL :) )
	to avoid compilation conflict with future version of gl.h
	eg: gl.h Version 1.2 define glActiveTextureARB so we can't use it.

	NB: we do it for all (EXT, NV, ARB extension) even it should be usefull only for ARB ones.
*/


// ARB_multitexture
//=================
extern PFNGLACTIVETEXTUREARBPROC nglActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC nglClientActiveTextureARB;

extern PFNGLMULTITEXCOORD1SARBPROC nglMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1IARBPROC nglMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1DARBPROC nglMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD2SARBPROC nglMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2IARBPROC nglMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2FARBPROC nglMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2DARBPROC nglMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD3SARBPROC nglMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3IARBPROC nglMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3FARBPROC nglMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3DARBPROC nglMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD4SARBPROC nglMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4IARBPROC nglMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4FARBPROC nglMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4DARBPROC nglMultiTexCoord4dARB;

extern PFNGLMULTITEXCOORD1SVARBPROC nglMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD1IVARBPROC nglMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1FVARBPROC nglMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1DVARBPROC nglMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD2SVARBPROC nglMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD2IVARBPROC nglMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2FVARBPROC nglMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2DVARBPROC nglMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD3SVARBPROC nglMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD3IVARBPROC nglMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3FVARBPROC nglMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3DVARBPROC nglMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD4SVARBPROC nglMultiTexCoord4svARB;
extern PFNGLMULTITEXCOORD4IVARBPROC nglMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4FVARBPROC nglMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4DVARBPROC nglMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC		nglCompressedTexImage3DARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		nglCompressedTexImage2DARB;
extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC		nglCompressedTexImage1DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	nglCompressedTexSubImage3DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	nglCompressedTexSubImage2DARB;
extern PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	nglCompressedTexSubImage1DARB;
extern PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	nglGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
extern PFNGLFLUSHVERTEXARRAYRANGENVPROC		nglFlushVertexArrayRangeNV;
extern PFNGLVERTEXARRAYRANGENVPROC			nglVertexArrayRangeNV;
#ifdef NL_OS_WINDOWS
extern PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
extern PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;
#endif


// FenceNV.
//====================
extern PFNGLDELETEFENCESNVPROC				nglDeleteFencesNV;
extern PFNGLGENFENCESNVPROC					nglGenFencesNV;
extern PFNGLISFENCENVPROC					nglIsFenceNV;
extern PFNGLTESTFENCENVPROC					nglTestFenceNV;
extern PFNGLGETFENCEIVNVPROC				nglGetFenceivNV;
extern PFNGLFINISHFENCENVPROC				nglFinishFenceNV;
extern PFNGLSETFENCENVPROC					nglSetFenceNV;


// VertexWeighting.
//==================
extern PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
extern PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
extern PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
extern PFNGLAREPROGRAMSRESIDENTNVPROC		nglAreProgramsResidentNV;
extern PFNGLBINDPROGRAMNVPROC				nglBindProgramNV;
extern PFNGLDELETEPROGRAMSNVPROC			nglDeleteProgramsNV;
extern PFNGLEXECUTEPROGRAMNVPROC			nglExecuteProgramNV;
extern PFNGLGENPROGRAMSNVPROC				nglGenProgramsNV;
extern PFNGLGETPROGRAMPARAMETERDVNVPROC		nglGetProgramParameterdvNV;
extern PFNGLGETPROGRAMPARAMETERFVNVPROC		nglGetProgramParameterfvNV;
extern PFNGLGETPROGRAMIVNVPROC				nglGetProgramivNV;
extern PFNGLGETPROGRAMSTRINGNVPROC			nglGetProgramStringNV;
extern PFNGLGETTRACKMATRIXIVNVPROC			nglGetTrackMatrixivNV;
extern PFNGLGETVERTEXATTRIBDVNVPROC			nglGetVertexAttribdvNV;
extern PFNGLGETVERTEXATTRIBFVNVPROC			nglGetVertexAttribfvNV;
extern PFNGLGETVERTEXATTRIBIVNVPROC			nglGetVertexAttribivNV;
extern PFNGLGETVERTEXATTRIBPOINTERVNVPROC	nglGetVertexAttribPointervNV;
extern PFNGLISPROGRAMNVPROC					nglIsProgramNV;
extern PFNGLLOADPROGRAMNVPROC				nglLoadProgramNV;
extern PFNGLPROGRAMPARAMETER4DNVPROC		nglProgramParameter4dNV;
extern PFNGLPROGRAMPARAMETER4DVNVPROC		nglProgramParameter4dvNV;
extern PFNGLPROGRAMPARAMETER4FNVPROC		nglProgramParameter4fNV;
extern PFNGLPROGRAMPARAMETER4FVNVPROC		nglProgramParameter4fvNV;
extern PFNGLPROGRAMPARAMETERS4DVNVPROC		nglProgramParameters4dvNV;
extern PFNGLPROGRAMPARAMETERS4FVNVPROC		nglProgramParameters4fvNV;
extern PFNGLREQUESTRESIDENTPROGRAMSNVPROC	nglRequestResidentProgramsNV;
extern PFNGLTRACKMATRIXNVPROC				nglTrackMatrixNV;
extern PFNGLVERTEXATTRIBPOINTERNVPROC		nglVertexAttribPointerNV;
extern PFNGLVERTEXATTRIB1DNVPROC			nglVertexAttrib1dNV;
extern PFNGLVERTEXATTRIB1DVNVPROC			nglVertexAttrib1dvNV;
extern PFNGLVERTEXATTRIB1FNVPROC			nglVertexAttrib1fNV;
extern PFNGLVERTEXATTRIB1FVNVPROC			nglVertexAttrib1fvNV;
extern PFNGLVERTEXATTRIB1SNVPROC			nglVertexAttrib1sNV;
extern PFNGLVERTEXATTRIB1SVNVPROC			nglVertexAttrib1svNV;
extern PFNGLVERTEXATTRIB2DNVPROC			nglVertexAttrib2dNV;
extern PFNGLVERTEXATTRIB2DVNVPROC			nglVertexAttrib2dvNV;
extern PFNGLVERTEXATTRIB2FNVPROC			nglVertexAttrib2fNV;
extern PFNGLVERTEXATTRIB2FVNVPROC			nglVertexAttrib2fvNV;
extern PFNGLVERTEXATTRIB2SNVPROC			nglVertexAttrib2sNV;
extern PFNGLVERTEXATTRIB2SVNVPROC			nglVertexAttrib2svNV;
extern PFNGLVERTEXATTRIB3DNVPROC			nglVertexAttrib3dNV;
extern PFNGLVERTEXATTRIB3DVNVPROC			nglVertexAttrib3dvNV;
extern PFNGLVERTEXATTRIB3FNVPROC			nglVertexAttrib3fNV;
extern PFNGLVERTEXATTRIB3FVNVPROC			nglVertexAttrib3fvNV;
extern PFNGLVERTEXATTRIB3SNVPROC			nglVertexAttrib3sNV;
extern PFNGLVERTEXATTRIB3SVNVPROC			nglVertexAttrib3svNV;
extern PFNGLVERTEXATTRIB4DNVPROC			nglVertexAttrib4dNV;
extern PFNGLVERTEXATTRIB4DVNVPROC			nglVertexAttrib4dvNV;
extern PFNGLVERTEXATTRIB4FNVPROC			nglVertexAttrib4fNV;
extern PFNGLVERTEXATTRIB4FVNVPROC			nglVertexAttrib4fvNV;
extern PFNGLVERTEXATTRIB4SNVPROC			nglVertexAttrib4sNV;
extern PFNGLVERTEXATTRIB4SVNVPROC			nglVertexAttrib4svNV;
extern PFNGLVERTEXATTRIB4UBVNVPROC			nglVertexAttrib4ubvNV;
extern PFNGLVERTEXATTRIBS1DVNVPROC			nglVertexAttribs1dvNV;
extern PFNGLVERTEXATTRIBS1FVNVPROC			nglVertexAttribs1fvNV;
extern PFNGLVERTEXATTRIBS1SVNVPROC			nglVertexAttribs1svNV;
extern PFNGLVERTEXATTRIBS2DVNVPROC			nglVertexAttribs2dvNV;
extern PFNGLVERTEXATTRIBS2FVNVPROC			nglVertexAttribs2fvNV;
extern PFNGLVERTEXATTRIBS2SVNVPROC			nglVertexAttribs2svNV;
extern PFNGLVERTEXATTRIBS3DVNVPROC			nglVertexAttribs3dvNV;
extern PFNGLVERTEXATTRIBS3FVNVPROC			nglVertexAttribs3fvNV;
extern PFNGLVERTEXATTRIBS3SVNVPROC			nglVertexAttribs3svNV;
extern PFNGLVERTEXATTRIBS4DVNVPROC			nglVertexAttribs4dvNV;
extern PFNGLVERTEXATTRIBS4FVNVPROC			nglVertexAttribs4fvNV;
extern PFNGLVERTEXATTRIBS4SVNVPROC			nglVertexAttribs4svNV;
extern PFNGLVERTEXATTRIBS4UBVNVPROC			nglVertexAttribs4ubvNV;


// SecondaryColor extension
//========================
extern PFNGLSECONDARYCOLOR3BEXTPROC			nglSecondaryColor3bEXT;
extern PFNGLSECONDARYCOLOR3BVEXTPROC		nglSecondaryColor3bvEXT;
extern PFNGLSECONDARYCOLOR3DEXTPROC			nglSecondaryColor3dEXT;
extern PFNGLSECONDARYCOLOR3DVEXTPROC		nglSecondaryColor3dvEXT;
extern PFNGLSECONDARYCOLOR3FEXTPROC			nglSecondaryColor3fEXT;
extern PFNGLSECONDARYCOLOR3FVEXTPROC		nglSecondaryColor3fvEXT;
extern PFNGLSECONDARYCOLOR3IEXTPROC			nglSecondaryColor3iEXT;
extern PFNGLSECONDARYCOLOR3IVEXTPROC		nglSecondaryColor3ivEXT;
extern PFNGLSECONDARYCOLOR3SEXTPROC			nglSecondaryColor3sEXT;
extern PFNGLSECONDARYCOLOR3SVEXTPROC		nglSecondaryColor3svEXT;
extern PFNGLSECONDARYCOLOR3UBEXTPROC		nglSecondaryColor3ubEXT;
extern PFNGLSECONDARYCOLOR3UBVEXTPROC		nglSecondaryColor3ubvEXT;
extern PFNGLSECONDARYCOLOR3UIEXTPROC		nglSecondaryColor3uiEXT;
extern PFNGLSECONDARYCOLOR3UIVEXTPROC		nglSecondaryColor3uivEXT;
extern PFNGLSECONDARYCOLOR3USEXTPROC		nglSecondaryColor3usEXT;
extern PFNGLSECONDARYCOLOR3USVEXTPROC		nglSecondaryColor3usvEXT;
extern PFNGLSECONDARYCOLORPOINTEREXTPROC	nglSecondaryColorPointerEXT;


#ifdef NL_OS_WINDOWS

// Pbuffer extension
//==================
extern PFNWGLCREATEPBUFFERARBPROC			wglCreatePbufferARB;
extern PFNWGLGETPUFFERDCARBPROC				wglGetPbufferDCARB;
extern PFNWGLRELEASEPUFFERDCARBPROC			wglReleasePbufferDCARB;
extern PFNWGLDESTROYPUFFERARBPROC			wglDestroyPbufferARB;
extern PFNWGLQUERYPBUFFERARBPROC			wglQueryPbufferARB;


// Get Pixel format extension
//===========================
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribivARB;
extern PFNWGLGETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfvARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB;


// WGL_ARB_extensions_string
extern PFNWGFGETEXTENSIONSSTRINGARB			wglGetExtensionsStringARB;

#endif


#endif // NL_OPENGL_EXTENSION_H

