/** \file driver_opengl_extension_def.h
 * External OpenGL extension definition.
 *
 * $Id: driver_opengl_extension_def.h,v 1.15 2002/09/24 14:40:50 vizerie Exp $
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

#ifndef NL_OPENGL_EXTENSION_DEF_H
#define NL_OPENGL_EXTENSION_DEF_H


#include "nel/misc/types_nl.h"

#include <GL/gl.h>
#include <GL/glext.h>	// Please download it from http://oss.sgi.com/projects/ogl-sample/ABI/"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef NL_OS_WINDOWS

#ifndef HPBUFFERARB
DECLARE_HANDLE (HPBUFFERARB);
#endif // HPBUFFERARB

/* NV_vertex_array_range WGL extension not defined in glext.h */
typedef void *(APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (GLsizei size, GLfloat readFrequency, GLfloat writeFrequency, GLfloat priority);
typedef void *(APIENTRY * PFNWGLFREEMEMORYNVPROC) (void *pointer);
typedef HPBUFFERARB (APIENTRY * PFNWGLCREATEPBUFFERARBPROC) (HDC hdc, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC			(APIENTRY * PFNWGLGETPUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int			(APIENTRY * PFNWGLRELEASEPUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC);
typedef BOOL		(APIENTRY * PFNWGLDESTROYPUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL		(APIENTRY * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);

typedef BOOL		(APIENTRY * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC, int, int, UINT, const int *, int *);
typedef BOOL		(APIENTRY * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC, int, int, UINT, const int *, FLOAT *);
typedef BOOL		(APIENTRY * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC, const int *, const FLOAT *, UINT, int *, UINT *);

typedef	const char*	(APIENTRY * PFNWGFGETEXTENSIONSSTRINGARB) (HDC);
#endif


/* NV_vertex_program */
#ifndef GL_NV_vertex_program
#define GL_NV_vertex_program              1

#define GL_VERTEX_PROGRAM_NV              0x8620
#define GL_VERTEX_STATE_PROGRAM_NV        0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV           0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV         0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV           0x8625
#define GL_CURRENT_ATTRIB_NV              0x8626
#define GL_PROGRAM_LENGTH_NV              0x8627
#define GL_PROGRAM_STRING_NV              0x8628
#define GL_MODELVIEW_PROJECTION_NV        0x8629
#define GL_IDENTITY_NV                    0x862A
#define GL_INVERSE_NV                     0x862B
#define GL_TRANSPOSE_NV                   0x862C
#define GL_INVERSE_TRANSPOSE_NV           0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV          0x862F
#define GL_MATRIX0_NV                     0x8630
#define GL_MATRIX1_NV                     0x8631
#define GL_MATRIX2_NV                     0x8632
#define GL_MATRIX3_NV                     0x8633
#define GL_MATRIX4_NV                     0x8634
#define GL_MATRIX5_NV                     0x8635
#define GL_MATRIX6_NV                     0x8636
#define GL_MATRIX7_NV                     0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV  0x8640
#define GL_CURRENT_MATRIX_NV              0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV   0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV     0x8643
#define GL_PROGRAM_PARAMETER_NV           0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV        0x8645
#define GL_PROGRAM_TARGET_NV              0x8646
#define GL_PROGRAM_RESIDENT_NV            0x8647
#define GL_TRACK_MATRIX_NV                0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV      0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV      0x864A
#define GL_PROGRAM_ERROR_POSITION_NV      0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV        0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV        0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV        0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV        0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV        0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV        0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV        0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV        0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV        0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV        0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV       0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV       0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV       0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV       0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV       0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV       0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV       0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV       0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV       0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV       0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV       0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV       0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV       0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV       0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV       0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV       0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV      0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV      0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV      0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV      0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV      0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV      0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV       0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV       0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV       0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV       0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV       0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV       0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV       0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV       0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV       0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV       0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV      0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV      0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV      0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV      0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV      0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV      0x867F


#ifdef GL_GLEXT_PROTOTYPES
extern GLboolean APIENTRY glAreProgramsResidentNV(GLsizei n, const GLuint *programs, GLboolean *residences);
extern void APIENTRY glBindProgramNV(GLenum target, GLuint id);
extern void APIENTRY glDeleteProgramsNV(GLsizei n, const GLuint *programs);
extern void APIENTRY glExecuteProgramNV(GLenum target, GLuint id, const GLfloat *params);
extern void APIENTRY glGenProgramsNV(GLsizei n, GLuint *programs);
extern void APIENTRY glGetProgramParameterdvNV(GLenum target, GLuint index, GLenum pname, GLdouble *params);
extern void APIENTRY glGetProgramParameterfvNV(GLenum target, GLuint index, GLenum pname, GLfloat *params);
extern void APIENTRY glGetProgramivNV(GLuint id, GLenum pname, GLint *params);
extern void APIENTRY glGetProgramStringNV(GLuint id, GLenum pname, GLubyte *program);
extern void APIENTRY glGetTrackMatrixivNV(GLenum target, GLuint address, GLenum pname, GLint *params);
extern void APIENTRY glGetVertexAttribdvNV(GLuint index, GLenum pname, GLdouble *params);
extern void APIENTRY glGetVertexAttribfvNV(GLuint index, GLenum pname, GLfloat *params);
extern void APIENTRY glGetVertexAttribivNV(GLuint index, GLenum pname, GLint *params);
extern void APIENTRY glGetVertexAttribPointervNV(GLuint index, GLenum pname, GLvoid* *pointer);
extern GLboolean APIENTRY glIsProgramNV(GLuint id);
extern void APIENTRY glLoadProgramNV(GLenum target, GLuint id, GLsizei len, const GLubyte *program);
extern void APIENTRY glProgramParameter4dNV(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void APIENTRY glProgramParameter4dvNV(GLenum target, GLuint index, const GLdouble *v);
extern void APIENTRY glProgramParameter4fNV(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void APIENTRY glProgramParameter4fvNV(GLenum target, GLuint index, const GLfloat *v);
extern void APIENTRY glProgramParameters4dvNV(GLenum target, GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glProgramParameters4fvNV(GLenum target, GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glRequestResidentProgramsNV(GLsizei n, const GLuint *programs);
extern void APIENTRY glTrackMatrixNV(GLenum target, GLuint address, GLenum matrix, GLenum transform);
extern void APIENTRY glVertexAttribPointerNV(GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
extern void APIENTRY glVertexAttrib1dNV(GLuint index, GLdouble x);
extern void APIENTRY glVertexAttrib1dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib1fNV(GLuint index, GLfloat x);
extern void APIENTRY glVertexAttrib1fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib1sNV(GLuint index, GLshort x);
extern void APIENTRY glVertexAttrib1svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib2dNV(GLuint index, GLdouble x, GLdouble y);
extern void APIENTRY glVertexAttrib2dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib2fNV(GLuint index, GLfloat x, GLfloat y);
extern void APIENTRY glVertexAttrib2fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib2sNV(GLuint index, GLshort x, GLshort y);
extern void APIENTRY glVertexAttrib2svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib3dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z);
extern void APIENTRY glVertexAttrib3dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib3fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void APIENTRY glVertexAttrib3fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib3sNV(GLuint index, GLshort x, GLshort y, GLshort z);
extern void APIENTRY glVertexAttrib3svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib4dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void APIENTRY glVertexAttrib4dvNV(GLuint index, const GLdouble *v);
extern void APIENTRY glVertexAttrib4fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void APIENTRY glVertexAttrib4fvNV(GLuint index, const GLfloat *v);
extern void APIENTRY glVertexAttrib4sNV(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
extern void APIENTRY glVertexAttrib4svNV(GLuint index, const GLshort *v);
extern void APIENTRY glVertexAttrib4ubvNV(GLuint index, const GLubyte *v);
extern void APIENTRY glVertexAttribs1dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs1fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs1svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs2dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs2fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs2svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs3dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs3fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs3svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs4dvNV(GLuint index, GLsizei count, const GLdouble *v);
extern void APIENTRY glVertexAttribs4fvNV(GLuint index, GLsizei count, const GLfloat *v);
extern void APIENTRY glVertexAttribs4svNV(GLuint index, GLsizei count, const GLshort *v);
extern void APIENTRY glVertexAttribs4ubvNV(GLuint index, GLsizei count, const GLubyte *v);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLboolean (APIENTRY * PFNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n, const GLuint *programs, GLboolean *residences);
typedef void (APIENTRY * PFNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
typedef void (APIENTRY * PFNGLDELETEPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id, const GLfloat *params);
typedef void (APIENTRY * PFNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * PFNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * PFNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname, GLubyte *program);
typedef void (APIENTRY * PFNGLGETTRACKMATRIXIVNVPROC) (GLenum target, GLuint address, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef GLboolean (APIENTRY * PFNGLISPROGRAMNVPROC) (GLuint id);
typedef void (APIENTRY * PFNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4DNVPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target, GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4FNVPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target, GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * PFNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVNVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * PFNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index, GLsizei count, const GLubyte *v);
#endif	/* GL_NV_vertex_program */


#ifndef WGL_ARB_pixel_format
#define WGL_ARB_pixel_format 1
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_ARB_pbuffer
#define WGL_ARB_pbuffer 1
#define WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035
#define WGL_PBUFFER_LOST_ARB           0x2036
#endif

/* NV_fence extension */
#ifndef GL_NV_fence
#define GL_NV_fence			1

#define GL_ALL_COMPLETED_NV               0x84F2
#define GL_FENCE_STATUS_NV                0x84F3
#define GL_FENCE_CONDITION_NV             0x84F4

#ifdef GL_GLEXT_PROTOTYPES
extern void APIENTRY glDeleteFencesNV(GLsizei n, const GLuint *fences);
extern void APIENTRY glGenFencesNV(GLsizei n, GLuint *fences);
extern GLboolean APIENTRY glIsFenceNV(GLuint fence);
extern GLboolean APIENTRY glTestFenceNV(GLuint fence);
extern void APIENTRY glGetFenceivNV(GLuint fence, GLenum pname, GLint *params);
extern void APIENTRY glFinishFenceNV(GLuint fence);
extern void APIENTRY glSetFenceNV(GLuint fence, GLenum condition);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRY * PFNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
typedef void (APIENTRY * PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
typedef GLboolean (APIENTRY * PFNGLISFENCENVPROC) (GLuint fence);
typedef GLboolean (APIENTRY * PFNGLTESTFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLFINISHFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);

#endif	/* GL_NV_fence */


/* NV_texture_shader */
#define GL_OFFSET_TEXTURE_RECTANGLE_NV    0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV 0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV 0x864E
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV 0x86D9
#define GL_UNSIGNED_INT_S8_S8_8_8_NV      0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV  0x86DB
#define GL_DSDT_MAG_INTENSITY_NV          0x86DC
#define GL_SHADER_CONSISTENT_NV           0x86DD
#define GL_TEXTURE_SHADER_NV              0x86DE
#define GL_SHADER_OPERATION_NV            0x86DF
#define GL_CULL_MODES_NV                  0x86E0
#define GL_OFFSET_TEXTURE_MATRIX_NV       0x86E1
#define GL_OFFSET_TEXTURE_SCALE_NV        0x86E2
#define GL_OFFSET_TEXTURE_BIAS_NV         0x86E3
#define GL_OFFSET_TEXTURE_2D_MATRIX_NV    GL_OFFSET_TEXTURE_MATRIX_NV
#define GL_OFFSET_TEXTURE_2D_SCALE_NV     GL_OFFSET_TEXTURE_SCALE_NV
#define GL_OFFSET_TEXTURE_2D_BIAS_NV      GL_OFFSET_TEXTURE_BIAS_NV
#define GL_PREVIOUS_TEXTURE_INPUT_NV      0x86E4
#define GL_CONST_EYE_NV                   0x86E5
#define GL_PASS_THROUGH_NV                0x86E6
#define GL_CULL_FRAGMENT_NV               0x86E7
#define GL_OFFSET_TEXTURE_2D_NV           0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV     0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV     0x86EA
#define GL_DOT_PRODUCT_NV                 0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV   0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV      0x86EE
#define GL_DOT_PRODUCT_TEXTURE_3D_NV      0x86EF
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV 0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV 0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV 0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV 0x86F3
#define GL_HILO_NV                        0x86F4
#define GL_DSDT_NV                        0x86F5
#define GL_DSDT_MAG_NV                    0x86F6
#define GL_DSDT_MAG_VIB_NV                0x86F7
#define GL_HILO16_NV                      0x86F8
#define GL_SIGNED_HILO_NV                 0x86F9
#define GL_SIGNED_HILO16_NV               0x86FA
#define GL_SIGNED_RGBA_NV                 0x86FB
#define GL_SIGNED_RGBA8_NV                0x86FC
#define GL_SIGNED_RGB_NV                  0x86FE
#define GL_SIGNED_RGB8_NV                 0x86FF
#define GL_SIGNED_LUMINANCE_NV            0x8701
#define GL_SIGNED_LUMINANCE8_NV           0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV      0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV    0x8704
#define GL_SIGNED_ALPHA_NV                0x8705
#define GL_SIGNED_ALPHA8_NV               0x8706
#define GL_SIGNED_INTENSITY_NV            0x8707
#define GL_SIGNED_INTENSITY8_NV           0x8708
#define GL_DSDT8_NV                       0x8709
#define GL_DSDT8_MAG8_NV                  0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV       0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV   0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV 0x870D
#define GL_HI_SCALE_NV                    0x870E
#define GL_LO_SCALE_NV                    0x870F
#define GL_DS_SCALE_NV                    0x8710
#define GL_DT_SCALE_NV                    0x8711
#define GL_MAGNITUDE_SCALE_NV             0x8712
#define GL_VIBRANCE_SCALE_NV              0x8713
#define GL_HI_BIAS_NV                     0x8714
#define GL_LO_BIAS_NV                     0x8715
#define GL_DS_BIAS_NV                     0x8716
#define GL_DT_BIAS_NV                     0x8717
#define GL_MAGNITUDE_BIAS_NV              0x8718
#define GL_VIBRANCE_BIAS_NV               0x8719
#define GL_TEXTURE_BORDER_VALUES_NV       0x871A
#define GL_TEXTURE_HI_SIZE_NV             0x871B
#define GL_TEXTURE_LO_SIZE_NV             0x871C
#define GL_TEXTURE_DS_SIZE_NV             0x871D
#define GL_TEXTURE_DT_SIZE_NV             0x871E
#define GL_TEXTURE_MAG_SIZE_NV            0x871F


/* GL_EXT_vertex_shader */


#define      GL_VERTEX_SHADER_EXT					 0x8780
#define      GL_VARIANT_VALUE_EXT                   0x87e4
#define      GL_VARIANT_DATATYPE_EXT                0x87e5
#define      GL_VARIANT_ARRAY_STRIDE_EXT            0x87e6
#define      GL_VARIANT_ARRAY_TYPE_EXT              0x87e7
#define      GL_VARIANT_ARRAY_EXT                   0x87e8
#define      GL_VARIANT_ARRAY_POINTER_EXT           0x87e9
#define      GL_INVARIANT_VALUE_EXT                 0x87ea
#define      GL_INVARIANT_DATATYPE_EXT              0x87eb
#define      GL_LOCAL_CONSTANT_VALUE_EXT            0x87ec
#define      GL_LOCAL_CONSTANT_DATATYPE_EXT         0x87ed
#define      GL_OP_INDEX_EXT                        0x8782
#define      GL_OP_NEGATE_EXT                       0x8783
#define      GL_OP_DOT3_EXT                         0x8784
#define      GL_OP_DOT4_EXT                         0x8785
#define      GL_OP_MUL_EXT                          0x8786
#define      GL_OP_ADD_EXT                          0x8787
#define      GL_OP_MADD_EXT                         0x8788
#define      GL_OP_FRAC_EXT                         0x8789
#define      GL_OP_MAX_EXT                          0x878a
#define      GL_OP_MIN_EXT                          0x878b
#define      GL_OP_SET_GE_EXT                       0x878c
#define      GL_OP_SET_LT_EXT                       0x878d
#define      GL_OP_CLAMP_EXT                        0x878e
#define      GL_OP_FLOOR_EXT                        0x878f
#define      GL_OP_ROUND_EXT                        0x8790
#define      GL_OP_EXP_BASE_2_EXT                   0x8791
#define      GL_OP_LOG_BASE_2_EXT                   0x8792
#define      GL_OP_POWER_EXT                        0x8793
#define      GL_OP_RECIP_EXT                        0x8794
#define      GL_OP_RECIP_SQRT_EXT                   0x8795
#define      GL_OP_SUB_EXT                          0x8796
#define      GL_OP_CROSS_PRODUCT_EXT                0x8797
#define      GL_OP_MULTIPLY_MATRIX_EXT              0x8798
#define      GL_OP_MOV_EXT                          0x8799
#define      GL_OUTPUT_VERTEX_EXT                   0x879a
#define      GL_OUTPUT_COLOR0_EXT                   0x879b
#define      GL_OUTPUT_COLOR1_EXT                   0x879c
#define      GL_OUTPUT_TEXTURE_COORD0_EXT           0x879d
#define      GL_OUTPUT_TEXTURE_COORD1_EXT           0x879e
#define      GL_OUTPUT_TEXTURE_COORD2_EXT           0x879f
#define      GL_OUTPUT_TEXTURE_COORD3_EXT           0x87a0
#define      GL_OUTPUT_TEXTURE_COORD4_EXT           0x87a1
#define      GL_OUTPUT_TEXTURE_COORD5_EXT           0x87a2
#define      GL_OUTPUT_TEXTURE_COORD6_EXT           0x87a3
#define      GL_OUTPUT_TEXTURE_COORD7_EXT           0x87a4
#define      GL_OUTPUT_TEXTURE_COORD8_EXT           0x87a5
#define      GL_OUTPUT_TEXTURE_COORD9_EXT           0x87a6
#define      GL_OUTPUT_TEXTURE_COORD10_EXT          0x87a7
#define      GL_OUTPUT_TEXTURE_COORD11_EXT          0x87a8
#define      GL_OUTPUT_TEXTURE_COORD12_EXT          0x87a9
#define      GL_OUTPUT_TEXTURE_COORD13_EXT          0x87aa
#define      GL_OUTPUT_TEXTURE_COORD14_EXT          0x87ab
#define      GL_OUTPUT_TEXTURE_COORD15_EXT          0x87ac
#define      GL_OUTPUT_TEXTURE_COORD16_EXT          0x87ad
#define      GL_OUTPUT_TEXTURE_COORD17_EXT          0x87ae
#define      GL_OUTPUT_TEXTURE_COORD18_EXT          0x87af
#define      GL_OUTPUT_TEXTURE_COORD19_EXT          0x87b0
#define      GL_OUTPUT_TEXTURE_COORD20_EXT          0x87b1
#define      GL_OUTPUT_TEXTURE_COORD21_EXT          0x87b2
#define      GL_OUTPUT_TEXTURE_COORD22_EXT          0x87b3
#define      GL_OUTPUT_TEXTURE_COORD23_EXT          0x87b4
#define      GL_OUTPUT_TEXTURE_COORD24_EXT          0x87b5
#define      GL_OUTPUT_TEXTURE_COORD25_EXT          0x87b6
#define      GL_OUTPUT_TEXTURE_COORD26_EXT          0x87b7
#define      GL_OUTPUT_TEXTURE_COORD27_EXT          0x87b8
#define      GL_OUTPUT_TEXTURE_COORD28_EXT          0x87b9
#define      GL_OUTPUT_TEXTURE_COORD29_EXT          0x87ba
#define      GL_OUTPUT_TEXTURE_COORD30_EXT          0x87bb
#define      GL_OUTPUT_TEXTURE_COORD31_EXT          0x87bc
#define      GL_OUTPUT_FOG_EXT                      0x87bd
#define      GL_SCALAR_EXT                          0x87be
#define      GL_VECTOR_EXT                          0x87bf
#define      GL_MATRIX_EXT                          0x87c0
#define      GL_VARIANT_EXT									  0x87c1
#define      GL_INVARIANT_EXT									  0x87c2
#define      GL_LOCAL_CONSTANT_EXT							      0x87c3
#define      GL_LOCAL_EXT										  0x87c4
#define      GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT               0x87c5
#define      GL_MAX_VERTEX_SHADER_VARIANTS_EXT                   0x87c6
#define      GL_MAX_VERTEX_SHADER_INVARIANTS_EXT                 0x87c7
#define      GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT            0x87c8
#define      GL_MAX_VERTEX_SHADER_LOCALS_EXT                     0x87c9
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT     0x87ca
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT         0x87cb
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT  0x87cc
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT       0x87cd
#define      GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT           0x87ce
#define      GL_VERTEX_SHADER_INSTRUCTIONS_EXT                   0x87cf
#define      GL_VERTEX_SHADER_VARIANTS_EXT                       0x87d0
#define      GL_VERTEX_SHADER_INVARIANTS_EXT                     0x87d1
#define      GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT                0x87d2
#define      GL_VERTEX_SHADER_LOCALS_EXT                         0x87d3
#define      GL_VERTEX_SHADER_BINDING_EXT                        0x8781
#define      GL_VERTEX_SHADER_OPTIMIZED_EXT                     0x87d4
#define      GL_X_EXT                               0x87d5
#define      GL_Y_EXT                               0x87d6
#define      GL_Z_EXT                               0x87d7
#define      GL_W_EXT                               0x87d8
#define      GL_NEGEXTVE_X_EXT                      0x87d9
#define      GL_NEGEXTVE_Y_EXT                      0x87da
#define      GL_NEGEXTVE_Z_EXT                      0x87db
#define      GL_NEGEXTVE_W_EXT                      0x87dc
#define      GL_ZERO_EXT                            0x87dd
#define      GL_ONE_EXT                             0x87de
#define      GL_NEGEXTVE_ONE_EXT                    0x87df
#define      GL_NORMALIZED_RANGE_EXT                0x87e0
#define      GL_FULL_RANGE_EXT                      0x87e1
#define      GL_CURRENT_VERTEX_EXT                  0x87e2
#define      GL_MVP_MATRIX_EXT                      0x87e3



/* NV_vertex_array_range2 */
#ifndef GL_NV_vertex_array_range2
#define GL_NV_vertex_array_range2			1
#define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV 0x8533
#endif


/* GL_ATI_vertex_array_object */
#ifndef GL_ATI_vertex_array_object
#define GL_STATIC_ATI                     0x8760
#define GL_DYNAMIC_ATI                    0x8761
#define GL_PRESERVE_ATI                   0x8762
#define GL_DISCARD_ATI                    0x8763
#define GL_OBJECT_BUFFER_SIZE_ATI         0x8764
#define GL_OBJECT_BUFFER_USAGE_ATI        0x8765
#define GL_ARRAY_OBJECT_BUFFER_ATI        0x8766
#define GL_ARRAY_OBJECT_OFFSET_ATI        0x8767
#endif

#ifndef GL_ATI_vertex_array_object
#define GL_ATI_vertex_array_object 1
#ifdef GL_GLEXT_PROTOTYPES
GLAPI GLuint APIENTRY glNewObjectBufferATI (GLsizei, const GLvoid *, GLenum);
GLAPI GLboolean APIENTRY glIsObjectBufferATI (GLuint);
GLAPI void APIENTRY glUpdateObjectBufferATI (GLuint, GLuint, GLsizei, const GLvoid *, GLenum);
GLAPI void APIENTRY glGetObjectBufferfvATI (GLuint, GLenum, GLfloat *);
GLAPI void APIENTRY glGetObjectBufferivATI (GLuint, GLenum, GLint *);
GLAPI void APIENTRY glDeleteObjectBufferATI (GLuint);
GLAPI void APIENTRY glArrayObjectATI (GLenum, GLint, GLenum, GLsizei, GLuint, GLuint);
GLAPI void APIENTRY glGetArrayObjectfvATI (GLenum, GLenum, GLfloat *);
GLAPI void APIENTRY glGetArrayObjectivATI (GLenum, GLenum, GLint *);
GLAPI void APIENTRY glVariantArrayObjectATI (GLuint, GLenum, GLsizei, GLuint, GLuint);
GLAPI void APIENTRY glGetVariantArrayObjectfvATI (GLuint, GLenum, GLfloat *);
GLAPI void APIENTRY glGetVariantArrayObjectivATI (GLuint, GLenum, GLint *);
#endif /* GL_GLEXT_PROTOTYPES */
typedef GLuint (APIENTRY * PFNGLNEWOBJECTBUFFERATIPROC) (GLsizei size, const GLvoid *pointer, GLenum usage);
typedef GLboolean (APIENTRY * PFNGLISOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * PFNGLUPDATEOBJECTBUFFERATIPROC) (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef void (APIENTRY * PFNGLGETOBJECTBUFFERFVATIPROC) (GLuint buffer, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETOBJECTBUFFERIVATIPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLDELETEOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * PFNGLARRAYOBJECTATIPROC) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * PFNGLGETARRAYOBJECTFVATIPROC) (GLenum array, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETARRAYOBJECTIVATIPROC) (GLenum array, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLVARIANTARRAYOBJECTATIPROC) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * PFNGLGETVARIANTARRAYOBJECTFVATIPROC) (GLuint id, GLenum pname, GLfloat *params);
typedef void (APIENTRY * PFNGLGETVARIANTARRAYOBJECTIVATIPROC) (GLuint id, GLenum pname, GLint *params);
#endif

/* GL_ATIX_texture_env_combine3 */
#ifndef GL_ATIX_texture_env_combine3
#define GL_ATIX_texture_env_combine3

#define GL_MODULATE_ADD_ATIX				0x8744
#define GL_MODULATE_SIGNED_ADD_ATIX			0x8745
#define GL_MODULATE_SUBTRACT_ATIX			0x8746

#endif /* GL_ATIX_texture_env_combine3 */



/* GL_ATI_envmap_bumpmap */

#ifndef GL_ATI_envmap_bumpmap
#define GL_ATI_envmap_bumpmap 1

#define GL_BUMP_ROT_MATRIX_ATI					   0x8775
#define GL_BUMP_ROT_MATRIX_SIZE_ATI				   0x8776
#define GL_BUMP_NUM_TEX_UNITS_ATI				   0x8777
#define GL_BUMP_TEX_UNITS_ATI					   0x8778
#define GL_DUDV_ATI								   0x8779
#define GL_DU8DV8_ATI							   0x877A
#define GL_BUMP_ENVMAP_ATI						   0x877B
#define GL_BUMP_TARGET_ATI						   0x877C

typedef void (APIENTRY * PFNGLTEXBUMPPARAMETERIVATIPROC) (GLenum pname, GLint *param);
typedef void (APIENTRY * PFNGLTEXBUMPPARAMETERFVATIPROC) (GLenum pname, GLfloat *param);
typedef void (APIENTRY * PFNGLGETTEXBUMPPARAMETERIVATIPROC) (GLenum pname, GLint *param);
typedef void (APIENTRY * PFNGLGETTEXBUMPPARAMETERFVATIPROC) (GLenum pname, GLfloat *param);


#endif /* GL_ATI_envmap_bumpmap */




// ***************************************************************************
// ***************************************************************************
// The NEL Functions Typedefs.
// Must do it for compatibilities with futures version of gl.h
// eg: version 1.2 does not define PFNGLACTIVETEXTUREARBPROC. Hence, do it now, with our special name
// ***************************************************************************
// ***************************************************************************


// ARB_multitexture
//=================
typedef void (APIENTRY * NEL_PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * NEL_PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (APIENTRY * NEL_PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);


// ARB_TextureCompression.
//========================
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
typedef void (APIENTRY * NEL_PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, void *img);


// VertexArrayRangeNV.
//====================
typedef void (APIENTRY * NEL_PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (APIENTRY * NEL_PFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid *pointer);


// FenceNV.
//====================
typedef void (APIENTRY * NEL_PFNGLDELETEFENCESNVPROC) (GLsizei n, const GLuint *fences);
typedef void (APIENTRY * NEL_PFNGLGENFENCESNVPROC) (GLsizei n, GLuint *fences);
typedef GLboolean (APIENTRY * NEL_PFNGLISFENCENVPROC) (GLuint fence);
typedef GLboolean (APIENTRY * NEL_PFNGLTESTFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * NEL_PFNGLGETFENCEIVNVPROC) (GLuint fence, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLFINISHFENCENVPROC) (GLuint fence);
typedef void (APIENTRY * NEL_PFNGLSETFENCENVPROC) (GLuint fence, GLenum condition);


// VertexWeighting.
//==================
typedef void (APIENTRY * NEL_PFNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
typedef void (APIENTRY * NEL_PFNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat *weight);
typedef void (APIENTRY * NEL_PFNGLVERTEXWEIGHTPOINTEREXTPROC) (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer);


// VertexProgramExtension.
//========================
typedef GLboolean (APIENTRY * NEL_PFNGLAREPROGRAMSRESIDENTNVPROC) (GLsizei n, const GLuint *programs, GLboolean *residences);
typedef void (APIENTRY * NEL_PFNGLBINDPROGRAMNVPROC) (GLenum target, GLuint id);
typedef void (APIENTRY * NEL_PFNGLDELETEPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * NEL_PFNGLEXECUTEPROGRAMNVPROC) (GLenum target, GLuint id, const GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGENPROGRAMSNVPROC) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMPARAMETERDVNVPROC) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMPARAMETERFVNVPROC) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMIVNVPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETPROGRAMSTRINGNVPROC) (GLuint id, GLenum pname, GLubyte *program);
typedef void (APIENTRY * NEL_PFNGLGETTRACKMATRIXIVNVPROC) (GLenum target, GLuint address, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBDVNVPROC) (GLuint index, GLenum pname, GLdouble *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBFVNVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBIVNVPROC) (GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLGETVERTEXATTRIBPOINTERVNVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
typedef GLboolean (APIENTRY * NEL_PFNGLISPROGRAMNVPROC) (GLuint id);
typedef void (APIENTRY * NEL_PFNGLLOADPROGRAMNVPROC) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4DNVPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4DVNVPROC) (GLenum target, GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4FNVPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETER4FVNVPROC) (GLenum target, GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETERS4DVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLPROGRAMPARAMETERS4FVNVPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLREQUESTRESIDENTPROGRAMSNVPROC) (GLsizei n, const GLuint *programs);
typedef void (APIENTRY * NEL_PFNGLTRACKMATRIXNVPROC) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBPOINTERNVPROC) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1DNVPROC) (GLuint index, GLdouble x);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1FNVPROC) (GLuint index, GLfloat x);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1SNVPROC) (GLuint index, GLshort x);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB1SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2DNVPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2FNVPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2SNVPROC) (GLuint index, GLshort x, GLshort y);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB2SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB3SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4DNVPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4DVNVPROC) (GLuint index, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4FNVPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4FVNVPROC) (GLuint index, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4SNVPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4SVNVPROC) (GLuint index, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIB4UBVNVPROC) (GLuint index, const GLubyte *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS1DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS1FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS1SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS2DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS2FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS2SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS3DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS3FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS3SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4DVNVPROC) (GLuint index, GLsizei count, const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4FVNVPROC) (GLuint index, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4SVNVPROC) (GLuint index, GLsizei count, const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLVERTEXATTRIBS4UBVNVPROC) (GLuint index, GLsizei count, const GLubyte *v);

// VertexShaderExtension (EXT)
//============================
typedef void	(APIENTRY * NEL_PFNGLBEGINVERTEXSHADEREXTPROC) ( void );
typedef void	(APIENTRY * NEL_PFNGLENDVERTEXSHADEREXTPROC) ( void );
typedef void	(APIENTRY * NEL_PFNGLBINDVERTEXSHADEREXTPROC) ( GLuint id );
typedef GLuint	(APIENTRY * NEL_PFNGLGENVERTEXSHADERSEXTPROC) ( GLuint range );
typedef void	(APIENTRY * NEL_PFNGLDELETEVERTEXSHADEREXTPROC) ( GLuint id );
typedef void	(APIENTRY * NEL_PFNGLSHADEROP1EXTPROC) ( GLenum op, GLuint res, GLuint arg1 );
typedef void	(APIENTRY * NEL_PFNGLSHADEROP2EXTPROC) ( GLenum op, GLuint res, GLuint arg1, GLuint arg2 );
typedef void	(APIENTRY * NEL_PFNGLSHADEROP3EXTPROC) ( GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3 );
typedef void	(APIENTRY * NEL_PFNGLSWIZZLEEXTPROC) ( GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW );
typedef void	(APIENTRY * NEL_PFNGLWRITEMASKEXTPROC) ( GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW );
typedef void	(APIENTRY * NEL_PFNGLINSERTCOMPONENTEXTPROC) ( GLuint res, GLuint src, GLuint num );
typedef void	(APIENTRY * NEL_PFNGLEXTRACTCOMPONENTEXTPROC) ( GLuint res, GLuint src, GLuint num );
typedef GLuint	(APIENTRY * NEL_PFNGLGENSYMBOLSEXTPROC) ( GLenum datatype, GLenum storagetype, GLenum range, GLuint components ) ;
typedef void	(APIENTRY * NEL_PFNGLSETINVARIANTEXTPROC) ( GLuint id, GLenum type, void *addr );
typedef void	(APIENTRY * NEL_PFNGLSETLOCALCONSTANTEXTPROC) ( GLuint id, GLenum type, void *addr );
typedef void	(APIENTRY * NEL_PFNGLVARIANTPOINTEREXTPROC) ( GLuint id, GLenum type, GLuint stride, void *addr );
typedef void	(APIENTRY * NEL_PFNGLENABLEVARIANTCLIENTSTATEEXTPROC) ( GLuint id);
typedef void	(APIENTRY * NEL_PFNGLDISABLEVARIANTCLIENTSTATEEXTPROC) ( GLuint id);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDLIGHTPARAMETEREXTPROC) ( GLenum light, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDMATERIALPARAMETEREXTPROC) ( GLenum face, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDTEXGENPARAMETEREXTPROC) ( GLenum unit, GLenum coord, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDTEXTUREUNITPARAMETEREXTPROC) ( GLenum unit, GLenum value);
typedef GLuint	(APIENTRY * NEL_PFNGLBINDPARAMETEREXTPROC) ( GLenum value);
typedef GLboolean (APIENTRY * NEL_PFNGLISVARIANTENABLEDEXTPROC) ( GLuint id, GLenum cap);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTBOOLEANVEXTPROC) ( GLuint id, GLenum value, GLboolean *data);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTINTEGERVEXTPROC) ( GLuint id, GLenum value, GLint *data);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTFLOATVEXTPROC) ( GLuint id, GLenum value, GLfloat *data);
typedef void	(APIENTRY * NEL_PFNGLGETVARIANTPOINTERVEXTPROC) ( GLuint id, GLenum value, void **data);
typedef void	(APIENTRY * NEL_PFNGLGETINVARIANTBOOLEANVEXTPROC) ( GLuint id, GLenum value, GLboolean *data);
typedef void	(APIENTRY * NEL_PFNGLGETINVARIANTINTEGERVEXTPROC) ( GLuint id, GLenum value, GLint *data);
typedef void	(APIENTRY * NEL_PFNGLGETINVARIANTFLOATVEXTPROC) ( GLuint id, GLenum value, GLfloat *data);
typedef void	(APIENTRY * NEL_PFNGLGETLOCALCONSTANTBOOLEANVEXTPROC) ( GLuint id, GLenum value, GLboolean *data);
typedef void	(APIENTRY * NEL_PFNGLGETLOCALCONSTANTINTEGERVEXTPROC) ( GLuint id, GLenum value, GLint *data);
typedef void	(APIENTRY * NEL_PFNGLGETLOCALCONSTANTFLOATVEXTPROC) ( GLuint id, GLenum value, GLfloat *data);


// SecondaryColor extension
//========================
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3BEXTPROC) (GLbyte red, GLbyte green, GLbyte blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3BVEXTPROC) (const GLbyte *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3DEXTPROC) (GLdouble red, GLdouble green, GLdouble blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3DVEXTPROC) (const GLdouble *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3FVEXTPROC) (const GLfloat *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3IEXTPROC) (GLint red, GLint green, GLint blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3IVEXTPROC) (const GLint *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3SEXTPROC) (GLshort red, GLshort green, GLshort blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3SVEXTPROC) (const GLshort *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UBEXTPROC) (GLubyte red, GLubyte green, GLubyte blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UBVEXTPROC) (const GLubyte *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UIEXTPROC) (GLuint red, GLuint green, GLuint blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3UIVEXTPROC) (const GLuint *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3USEXTPROC) (GLushort red, GLushort green, GLushort blue);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLOR3USVEXTPROC) (const GLushort *v);
typedef void (APIENTRY * NEL_PFNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLvoid *pointer);


// BlendColor extension
//========================
typedef void (APIENTRY * NEL_PFNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);


// GL_ATI_vertex_array_object extension
//========================
typedef GLuint (APIENTRY * NEL_PFNGLNEWOBJECTBUFFERATIPROC) (GLsizei size, const GLvoid *pointer, GLenum usage);
typedef GLboolean (APIENTRY * NEL_PFNGLISOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * NEL_PFNGLUPDATEOBJECTBUFFERATIPROC) (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
typedef void (APIENTRY * NEL_PFNGLGETOBJECTBUFFERFVATIPROC) (GLuint buffer, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETOBJECTBUFFERIVATIPROC) (GLuint buffer, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLDELETEOBJECTBUFFERATIPROC) (GLuint buffer);
typedef void (APIENTRY * NEL_PFNGLARRAYOBJECTATIPROC) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * NEL_PFNGLGETARRAYOBJECTFVATIPROC) (GLenum array, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETARRAYOBJECTIVATIPROC) (GLenum array, GLenum pname, GLint *params);
typedef void (APIENTRY * NEL_PFNGLVARIANTARRAYOBJECTATIPROC) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
typedef void (APIENTRY * NEL_PFNGLGETVARIANTARRAYOBJECTFVATIPROC) (GLuint id, GLenum pname, GLfloat *params);
typedef void (APIENTRY * NEL_PFNGLGETVARIANTARRAYOBJECTIVATIPROC) (GLuint id, GLenum pname, GLint *params);


#ifdef __cplusplus
}
#endif

#endif // NL_OPENGL_EXTENSION_DEF_H

