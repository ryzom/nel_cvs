/** \file driver_opengl_extension.h
 * OpenGL driver extension registry
 *
 * $Id: driver_opengl_extension.h,v 1.31 2002/09/24 14:40:46 vizerie Exp $
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
	bool    EXTVertexShader;
	bool	NVTextureShader;
	// true if NVVertexProgram and if we know that VP is emulated
	bool	NVVertexProgramEmulated;
	bool	EXTSecondaryColor;
	bool	EXTBlendColor;
	// NVVertexArrayRange2. 
	bool	NVVertexArrayRange2;
	// equal to GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV if possible, or GL_VERTEX_ARRAY_RANGE_NV
	uint	NVStateVARWithoutFlush;

	// WGL ARB extensions, true if supported
	bool	WGLARBPBuffer;
	bool	WGLARBPixelFormat;

	// ATI Extensions.
	bool	ATIVertexArrayObject;	
	bool	ATIXTextureEnvCombine3;
	bool    ATIEnvMapBumpMap;

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
		ATIXTextureEnvCombine3= false;
		ARBTextureCubeMap= false;
		NVTextureShader= false;
		NVVertexProgram= false;
		NVVertexProgramEmulated= false;
		EXTSecondaryColor= false;
		WGLARBPBuffer= false;
		WGLARBPixelFormat= false;
		EXTBlendColor= false;
		ATIVertexArrayObject= false;
		ATIEnvMapBumpMap = false;

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
extern NEL_PFNGLACTIVETEXTUREARBPROC nglActiveTextureARB;
extern NEL_PFNGLCLIENTACTIVETEXTUREARBPROC nglClientActiveTextureARB;

extern NEL_PFNGLMULTITEXCOORD1SARBPROC nglMultiTexCoord1sARB;
extern NEL_PFNGLMULTITEXCOORD1IARBPROC nglMultiTexCoord1iARB;
extern NEL_PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern NEL_PFNGLMULTITEXCOORD1FARBPROC nglMultiTexCoord1fARB;
extern NEL_PFNGLMULTITEXCOORD1DARBPROC nglMultiTexCoord1dARB;
extern NEL_PFNGLMULTITEXCOORD2SARBPROC nglMultiTexCoord2sARB;
extern NEL_PFNGLMULTITEXCOORD2IARBPROC nglMultiTexCoord2iARB;
extern NEL_PFNGLMULTITEXCOORD2FARBPROC nglMultiTexCoord2fARB;
extern NEL_PFNGLMULTITEXCOORD2DARBPROC nglMultiTexCoord2dARB;
extern NEL_PFNGLMULTITEXCOORD3SARBPROC nglMultiTexCoord3sARB;
extern NEL_PFNGLMULTITEXCOORD3IARBPROC nglMultiTexCoord3iARB;
extern NEL_PFNGLMULTITEXCOORD3FARBPROC nglMultiTexCoord3fARB;
extern NEL_PFNGLMULTITEXCOORD3DARBPROC nglMultiTexCoord3dARB;
extern NEL_PFNGLMULTITEXCOORD4SARBPROC nglMultiTexCoord4sARB;
extern NEL_PFNGLMULTITEXCOORD4IARBPROC nglMultiTexCoord4iARB;
extern NEL_PFNGLMULTITEXCOORD4FARBPROC nglMultiTexCoord4fARB;
extern NEL_PFNGLMULTITEXCOORD4DARBPROC nglMultiTexCoord4dARB;

extern NEL_PFNGLMULTITEXCOORD1SVARBPROC nglMultiTexCoord1svARB;
extern NEL_PFNGLMULTITEXCOORD1IVARBPROC nglMultiTexCoord1ivARB;
extern NEL_PFNGLMULTITEXCOORD1FVARBPROC nglMultiTexCoord1fvARB;
extern NEL_PFNGLMULTITEXCOORD1DVARBPROC nglMultiTexCoord1dvARB;
extern NEL_PFNGLMULTITEXCOORD2SVARBPROC nglMultiTexCoord2svARB;
extern NEL_PFNGLMULTITEXCOORD2IVARBPROC nglMultiTexCoord2ivARB;
extern NEL_PFNGLMULTITEXCOORD2FVARBPROC nglMultiTexCoord2fvARB;
extern NEL_PFNGLMULTITEXCOORD2DVARBPROC nglMultiTexCoord2dvARB;
extern NEL_PFNGLMULTITEXCOORD3SVARBPROC nglMultiTexCoord3svARB;
extern NEL_PFNGLMULTITEXCOORD3IVARBPROC nglMultiTexCoord3ivARB;
extern NEL_PFNGLMULTITEXCOORD3FVARBPROC nglMultiTexCoord3fvARB;
extern NEL_PFNGLMULTITEXCOORD3DVARBPROC nglMultiTexCoord3dvARB;
extern NEL_PFNGLMULTITEXCOORD4SVARBPROC nglMultiTexCoord4svARB;
extern NEL_PFNGLMULTITEXCOORD4IVARBPROC nglMultiTexCoord4ivARB;
extern NEL_PFNGLMULTITEXCOORD4FVARBPROC nglMultiTexCoord4fvARB;
extern NEL_PFNGLMULTITEXCOORD4DVARBPROC nglMultiTexCoord4dvARB;


// ARB_TextureCompression.
//========================
extern NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC		nglCompressedTexImage3DARB;
extern NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC		nglCompressedTexImage2DARB;
extern NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC		nglCompressedTexImage1DARB;
extern NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC	nglCompressedTexSubImage3DARB;
extern NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC	nglCompressedTexSubImage2DARB;
extern NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC	nglCompressedTexSubImage1DARB;
extern NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC	nglGetCompressedTexImageARB;


// VertexArrayRangeNV.
//====================
extern NEL_PFNGLFLUSHVERTEXARRAYRANGENVPROC		nglFlushVertexArrayRangeNV;
extern NEL_PFNGLVERTEXARRAYRANGENVPROC			nglVertexArrayRangeNV;
#ifdef NL_OS_WINDOWS
extern PFNWGLALLOCATEMEMORYNVPROC			wglAllocateMemoryNV;
extern PFNWGLFREEMEMORYNVPROC				wglFreeMemoryNV;
#endif


// FenceNV.
//====================
extern NEL_PFNGLDELETEFENCESNVPROC				nglDeleteFencesNV;
extern NEL_PFNGLGENFENCESNVPROC					nglGenFencesNV;
extern NEL_PFNGLISFENCENVPROC					nglIsFenceNV;
extern NEL_PFNGLTESTFENCENVPROC					nglTestFenceNV;
extern NEL_PFNGLGETFENCEIVNVPROC				nglGetFenceivNV;
extern NEL_PFNGLFINISHFENCENVPROC				nglFinishFenceNV;
extern NEL_PFNGLSETFENCENVPROC					nglSetFenceNV;


// VertexWeighting.
//==================
extern NEL_PFNGLVERTEXWEIGHTFEXTPROC			nglVertexWeightfEXT;
extern NEL_PFNGLVERTEXWEIGHTFVEXTPROC			nglVertexWeightfvEXT;
extern NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC		nglVertexWeightPointerEXT;


// VertexProgramExtension.
//========================
extern NEL_PFNGLAREPROGRAMSRESIDENTNVPROC		nglAreProgramsResidentNV;
extern NEL_PFNGLBINDPROGRAMNVPROC				nglBindProgramNV;
extern NEL_PFNGLDELETEPROGRAMSNVPROC			nglDeleteProgramsNV;
extern NEL_PFNGLEXECUTEPROGRAMNVPROC			nglExecuteProgramNV;
extern NEL_PFNGLGENPROGRAMSNVPROC				nglGenProgramsNV;
extern NEL_PFNGLGETPROGRAMPARAMETERDVNVPROC		nglGetProgramParameterdvNV;
extern NEL_PFNGLGETPROGRAMPARAMETERFVNVPROC		nglGetProgramParameterfvNV;
extern NEL_PFNGLGETPROGRAMIVNVPROC				nglGetProgramivNV;
extern NEL_PFNGLGETPROGRAMSTRINGNVPROC			nglGetProgramStringNV;
extern NEL_PFNGLGETTRACKMATRIXIVNVPROC			nglGetTrackMatrixivNV;
extern NEL_PFNGLGETVERTEXATTRIBDVNVPROC			nglGetVertexAttribdvNV;
extern NEL_PFNGLGETVERTEXATTRIBFVNVPROC			nglGetVertexAttribfvNV;
extern NEL_PFNGLGETVERTEXATTRIBIVNVPROC			nglGetVertexAttribivNV;
extern NEL_PFNGLGETVERTEXATTRIBPOINTERVNVPROC	nglGetVertexAttribPointervNV;
extern NEL_PFNGLISPROGRAMNVPROC					nglIsProgramNV;
extern NEL_PFNGLLOADPROGRAMNVPROC				nglLoadProgramNV;
extern NEL_PFNGLPROGRAMPARAMETER4DNVPROC		nglProgramParameter4dNV;
extern NEL_PFNGLPROGRAMPARAMETER4DVNVPROC		nglProgramParameter4dvNV;
extern NEL_PFNGLPROGRAMPARAMETER4FNVPROC		nglProgramParameter4fNV;
extern NEL_PFNGLPROGRAMPARAMETER4FVNVPROC		nglProgramParameter4fvNV;
extern NEL_PFNGLPROGRAMPARAMETERS4DVNVPROC		nglProgramParameters4dvNV;
extern NEL_PFNGLPROGRAMPARAMETERS4FVNVPROC		nglProgramParameters4fvNV;
extern NEL_PFNGLREQUESTRESIDENTPROGRAMSNVPROC	nglRequestResidentProgramsNV;
extern NEL_PFNGLTRACKMATRIXNVPROC				nglTrackMatrixNV;
extern NEL_PFNGLVERTEXATTRIBPOINTERNVPROC		nglVertexAttribPointerNV;
extern NEL_PFNGLVERTEXATTRIB1DNVPROC			nglVertexAttrib1dNV;
extern NEL_PFNGLVERTEXATTRIB1DVNVPROC			nglVertexAttrib1dvNV;
extern NEL_PFNGLVERTEXATTRIB1FNVPROC			nglVertexAttrib1fNV;
extern NEL_PFNGLVERTEXATTRIB1FVNVPROC			nglVertexAttrib1fvNV;
extern NEL_PFNGLVERTEXATTRIB1SNVPROC			nglVertexAttrib1sNV;
extern NEL_PFNGLVERTEXATTRIB1SVNVPROC			nglVertexAttrib1svNV;
extern NEL_PFNGLVERTEXATTRIB2DNVPROC			nglVertexAttrib2dNV;
extern NEL_PFNGLVERTEXATTRIB2DVNVPROC			nglVertexAttrib2dvNV;
extern NEL_PFNGLVERTEXATTRIB2FNVPROC			nglVertexAttrib2fNV;
extern NEL_PFNGLVERTEXATTRIB2FVNVPROC			nglVertexAttrib2fvNV;
extern NEL_PFNGLVERTEXATTRIB2SNVPROC			nglVertexAttrib2sNV;
extern NEL_PFNGLVERTEXATTRIB2SVNVPROC			nglVertexAttrib2svNV;
extern NEL_PFNGLVERTEXATTRIB3DNVPROC			nglVertexAttrib3dNV;
extern NEL_PFNGLVERTEXATTRIB3DVNVPROC			nglVertexAttrib3dvNV;
extern NEL_PFNGLVERTEXATTRIB3FNVPROC			nglVertexAttrib3fNV;
extern NEL_PFNGLVERTEXATTRIB3FVNVPROC			nglVertexAttrib3fvNV;
extern NEL_PFNGLVERTEXATTRIB3SNVPROC			nglVertexAttrib3sNV;
extern NEL_PFNGLVERTEXATTRIB3SVNVPROC			nglVertexAttrib3svNV;
extern NEL_PFNGLVERTEXATTRIB4DNVPROC			nglVertexAttrib4dNV;
extern NEL_PFNGLVERTEXATTRIB4DVNVPROC			nglVertexAttrib4dvNV;
extern NEL_PFNGLVERTEXATTRIB4FNVPROC			nglVertexAttrib4fNV;
extern NEL_PFNGLVERTEXATTRIB4FVNVPROC			nglVertexAttrib4fvNV;
extern NEL_PFNGLVERTEXATTRIB4SNVPROC			nglVertexAttrib4sNV;
extern NEL_PFNGLVERTEXATTRIB4SVNVPROC			nglVertexAttrib4svNV;
extern NEL_PFNGLVERTEXATTRIB4UBVNVPROC			nglVertexAttrib4ubvNV;
extern NEL_PFNGLVERTEXATTRIBS1DVNVPROC			nglVertexAttribs1dvNV;
extern NEL_PFNGLVERTEXATTRIBS1FVNVPROC			nglVertexAttribs1fvNV;
extern NEL_PFNGLVERTEXATTRIBS1SVNVPROC			nglVertexAttribs1svNV;
extern NEL_PFNGLVERTEXATTRIBS2DVNVPROC			nglVertexAttribs2dvNV;
extern NEL_PFNGLVERTEXATTRIBS2FVNVPROC			nglVertexAttribs2fvNV;
extern NEL_PFNGLVERTEXATTRIBS2SVNVPROC			nglVertexAttribs2svNV;
extern NEL_PFNGLVERTEXATTRIBS3DVNVPROC			nglVertexAttribs3dvNV;
extern NEL_PFNGLVERTEXATTRIBS3FVNVPROC			nglVertexAttribs3fvNV;
extern NEL_PFNGLVERTEXATTRIBS3SVNVPROC			nglVertexAttribs3svNV;
extern NEL_PFNGLVERTEXATTRIBS4DVNVPROC			nglVertexAttribs4dvNV;
extern NEL_PFNGLVERTEXATTRIBS4FVNVPROC			nglVertexAttribs4fvNV;
extern NEL_PFNGLVERTEXATTRIBS4SVNVPROC			nglVertexAttribs4svNV;
extern NEL_PFNGLVERTEXATTRIBS4UBVNVPROC			nglVertexAttribs4ubvNV;

// VertexShaderExtension.
//========================
extern NEL_PFNGLBEGINVERTEXSHADEREXTPROC		 nglBeginVertexShaderEXT;
extern NEL_PFNGLENDVERTEXSHADEREXTPROC			 nglEndVertexShaderEXT;
extern NEL_PFNGLBINDVERTEXSHADEREXTPROC			 nglBindVertexShaderEXT;
extern NEL_PFNGLGENVERTEXSHADERSEXTPROC			 nglGenVertexShadersEXT;
extern NEL_PFNGLDELETEVERTEXSHADEREXTPROC		 nglDeleteVertexShaderEXT;
extern NEL_PFNGLSHADEROP1EXTPROC				 nglShaderOp1EXT;
extern NEL_PFNGLSHADEROP2EXTPROC				 nglShaderOp2EXT;
extern NEL_PFNGLSHADEROP3EXTPROC				 nglShaderOp3EXT;
extern NEL_PFNGLSWIZZLEEXTPROC					 nglSwizzleEXT;
extern NEL_PFNGLWRITEMASKEXTPROC				 nglWriteMaskEXT;
extern NEL_PFNGLINSERTCOMPONENTEXTPROC			 nglInsertComponentEXT;
extern NEL_PFNGLEXTRACTCOMPONENTEXTPROC			 nglExtractComponentEXT;
extern NEL_PFNGLGENSYMBOLSEXTPROC				 nglGenSymbolsEXT;
extern NEL_PFNGLSETINVARIANTEXTPROC				 nglSetInvariantEXT;
extern NEL_PFNGLSETLOCALCONSTANTEXTPROC			 nglSetLocalConstantEXT;
extern NEL_PFNGLVARIANTPOINTEREXTPROC			 nglVariantPointerEXT;
extern NEL_PFNGLENABLEVARIANTCLIENTSTATEEXTPROC  nglEnableVariantClientStateEXT;
extern NEL_PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC nglDisableVariantClientStateEXT;
extern NEL_PFNGLBINDLIGHTPARAMETEREXTPROC		 nglBindLightParameterEXT;
extern NEL_PFNGLBINDMATERIALPARAMETEREXTPROC	 nglBindMaterialParameterEXT;
extern NEL_PFNGLBINDTEXGENPARAMETEREXTPROC		 nglBindTexGenParameterEXT;
extern NEL_PFNGLBINDTEXTUREUNITPARAMETEREXTPROC  nglBindTextureUnitParameterEXT;
extern NEL_PFNGLBINDPARAMETEREXTPROC			 nglBindParameterEXT;
extern NEL_PFNGLISVARIANTENABLEDEXTPROC			 nglIsVariantEnabledEXT;
extern NEL_PFNGLGETVARIANTBOOLEANVEXTPROC		 nglGetVariantBooleanvEXT;
extern NEL_PFNGLGETVARIANTINTEGERVEXTPROC		 nglGetVariantIntegervEXT;
extern NEL_PFNGLGETVARIANTFLOATVEXTPROC			 nglGetVariantFloatvEXT;
extern NEL_PFNGLGETVARIANTPOINTERVEXTPROC		 nglGetVariantPointervEXT;
extern NEL_PFNGLGETINVARIANTBOOLEANVEXTPROC		 nglGetInvariantBooleanvEXT;
extern NEL_PFNGLGETINVARIANTINTEGERVEXTPROC		 nglGetInvariantIntegervEXT;
extern NEL_PFNGLGETINVARIANTFLOATVEXTPROC		 nglGetInvariantFloatvEXT;
extern NEL_PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC  nglGetLocalConstantBooleanvEXT;
extern NEL_PFNGLGETLOCALCONSTANTINTEGERVEXTPROC  nglGetLocalConstantIntegervEXT;
extern NEL_PFNGLGETLOCALCONSTANTFLOATVEXTPROC    nglGetLocalConstantFloatvEXT;


// ATI_envmap_bumpmap extension

extern  PFNGLTEXBUMPPARAMETERIVATIPROC nglTexBumpParameterivATI;
extern  PFNGLTEXBUMPPARAMETERFVATIPROC nglTexBumpParameterfvATI;
extern  PFNGLGETTEXBUMPPARAMETERIVATIPROC nglGetTexBumpParameterivATI;
extern  PFNGLGETTEXBUMPPARAMETERFVATIPROC nglGetTexBumpParameterfvATI;


// SecondaryColor extension
//========================
extern NEL_PFNGLSECONDARYCOLOR3BEXTPROC			nglSecondaryColor3bEXT;
extern NEL_PFNGLSECONDARYCOLOR3BVEXTPROC		nglSecondaryColor3bvEXT;
extern NEL_PFNGLSECONDARYCOLOR3DEXTPROC			nglSecondaryColor3dEXT;
extern NEL_PFNGLSECONDARYCOLOR3DVEXTPROC		nglSecondaryColor3dvEXT;
extern NEL_PFNGLSECONDARYCOLOR3FEXTPROC			nglSecondaryColor3fEXT;
extern NEL_PFNGLSECONDARYCOLOR3FVEXTPROC		nglSecondaryColor3fvEXT;
extern NEL_PFNGLSECONDARYCOLOR3IEXTPROC			nglSecondaryColor3iEXT;
extern NEL_PFNGLSECONDARYCOLOR3IVEXTPROC		nglSecondaryColor3ivEXT;
extern NEL_PFNGLSECONDARYCOLOR3SEXTPROC			nglSecondaryColor3sEXT;
extern NEL_PFNGLSECONDARYCOLOR3SVEXTPROC		nglSecondaryColor3svEXT;
extern NEL_PFNGLSECONDARYCOLOR3UBEXTPROC		nglSecondaryColor3ubEXT;
extern NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC		nglSecondaryColor3ubvEXT;
extern NEL_PFNGLSECONDARYCOLOR3UIEXTPROC		nglSecondaryColor3uiEXT;
extern NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC		nglSecondaryColor3uivEXT;
extern NEL_PFNGLSECONDARYCOLOR3USEXTPROC		nglSecondaryColor3usEXT;
extern NEL_PFNGLSECONDARYCOLOR3USVEXTPROC		nglSecondaryColor3usvEXT;
extern NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC	nglSecondaryColorPointerEXT;


// BlendColor extension
//========================
extern NEL_PFNGLBLENDCOLOREXTPROC				nglBlendColorEXT;


// GL_ATI_vertex_array_object extension
//========================
extern NEL_PFNGLNEWOBJECTBUFFERATIPROC			nglNewObjectBufferATI;
extern NEL_PFNGLISOBJECTBUFFERATIPROC			nglIsObjectBufferATI;
extern NEL_PFNGLUPDATEOBJECTBUFFERATIPROC		nglUpdateObjectBufferATI;
extern NEL_PFNGLGETOBJECTBUFFERFVATIPROC		nglGetObjectBufferfvATI;
extern NEL_PFNGLGETOBJECTBUFFERIVATIPROC		nglGetObjectBufferivATI;
extern NEL_PFNGLDELETEOBJECTBUFFERATIPROC		nglDeleteObjectBufferATI;
extern NEL_PFNGLARRAYOBJECTATIPROC				nglArrayObjectATI;
extern NEL_PFNGLGETARRAYOBJECTFVATIPROC			nglGetArrayObjectfvATI;
extern NEL_PFNGLGETARRAYOBJECTIVATIPROC			nglGetArrayObjectivATI;
extern NEL_PFNGLVARIANTARRAYOBJECTATIPROC		nglVariantArrayObjectATI;
extern NEL_PFNGLGETVARIANTARRAYOBJECTFVATIPROC	nglGetVariantArrayObjectfvATI;
extern NEL_PFNGLGETVARIANTARRAYOBJECTIVATIPROC	nglGetVariantArrayObjectivATI;



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

