/** \file driver_opengl.cpp
 * OpenGL driver implementation for vertex Buffer / render manipulation.
 *
 * $Id: driver_opengl_vertex.cpp,v 1.1 2001/07/03 09:12:34 berenguier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
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

#include "nel/misc/types_nl.h"
#include "3d/primitive_block.h"
#include "driver_opengl.h"


using namespace std;
using namespace NLMISC;



// ***************************************************************************
// Flags for software vertex skinning.
#define	NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE	3
#define	NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE	1
#define	NL3D_DRV_SOFTSKIN_VCOMPUTED		0
// 3 means "vertex may need compute".
// 1 means "Primitive say vertex must be computed".
// 0 means "vertex is computed".



namespace NL3D
{



// ***************************************************************************
bool CDriverGL::setupVertexBuffer(CVertexBuffer& VB)
{
	// 1. Retrieve/Create driver shader.
	//==================================
	if (!VB.DrvInfos)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItVBDrvInfoPtrList	it= _VBDrvInfos.insert(_VBDrvInfos.end());
		// create and set iterator, for future deletion.
		*it= VB.DrvInfos= new CVBDrvInfosGL(this, it);
	}

	// 2. If necessary, do modifications.
	//==================================
	if( VB.getTouchFlags()!=0 )
	{
		CVBDrvInfosGL	*vbInf= static_cast<CVBDrvInfosGL*>((IVBDrvInfos*)(VB.DrvInfos));
		// Software and Skinning: must allocate PostRender Vertices and normals.
		if(!_PaletteSkinHard && (VB.getVertexFormat() & IDRV_VF_PALETTE_SKIN)==IDRV_VF_PALETTE_SKIN )
		{
			// Must reallocate post-rendered vertices/normals Flags.
			vbInf->SoftSkinFlags.resize(VB.getNumVertices());
			// Must reallocate post-rendered vertices/normals.
			vbInf->SoftSkinVertices.resize(VB.getNumVertices());
			// Normals onli if nomal enabled.
			if(VB.getVertexFormat() & IDRV_VF_NORMAL)
				vbInf->SoftSkinNormals.resize(VB.getNumVertices());
		}

		// OK!
		VB.resetTouchFlags();
	}


	return true;
}


// ***************************************************************************
bool CDriverGL::activeVertexBuffer(CVertexBuffer& VB, uint first, uint end)
{
	uint32	flags;

	if (!setupVertexBuffer(VB))
		return false;

	// Just to inform render*() that Matrix mode is OK.
	_MatrixSetupDirty= false;

	if (VB.getNumVertices()==0)
		return true;

	nlassert(end<=VB.getNumVertices());
	nlassert(first<=end);

	// Get VB flags, to setup matrixes and arrays.
	flags=VB.getVertexFormat();
	// Get VB drv infos.
	CVBDrvInfosGL	*vbInf= static_cast<CVBDrvInfosGL*>((IVBDrvInfos*)(VB.DrvInfos));


	// Skin mode.
	// NB: this test either if palette skin is enabled, or normal skinning is enabled.
	bool	skinning= (flags & IDRV_VF_PALETTE_SKIN)!=0;
	skinning= skinning && (_VertexMode & NL3D_VERTEX_MODE_SKINNING)!=0;
	// NB: this test if palette skin is enabled.
	bool	paletteSkinning= skinning && (flags & IDRV_VF_PALETTE_SKIN)==IDRV_VF_PALETTE_SKIN;


	// 0. Setup Matrixes.
	//===================

	// Check if view matrix has been modified
	if (_ViewMatrixSetupDirty)
		// Recompute stuff touched by the view matrix
		cleanViewMatrix ();

	// Check view matrix is good
	nlassert (_ViewMatrixSetupDirty==false);

	// General case: no software skinning.
	_CurrentSoftSkinFlags= NULL;

	// No Skinning at all??. 
	//==============
	if ( !skinning )
	{
		if(_ModelViewMatrixDirty[0])
		{
			_ModelViewMatrixDirty.clear(0);
			// By default, the first model matrix is active
			glLoadMatrixf( _ModelViewMatrix[0].get() );
		}
	}
	// Palette Skinning??
	//==============
	else if ( paletteSkinning )
	{
		if(_PaletteSkinHard)
		{
			// \todo yoyo: TODO_HARDWARE_SKINNIG: setup vertex program.
			// NB: must test _ModelViewMatrixDirtyPaletteSkin...
		}
		else
		{
			// setup identity
			glLoadMatrixf(CMatrix::Identity.get());
			
			// in software, we must test Model View Matrixes flags (_ModelViewMatrixDirtyPaletteSkin), to
			// compute M-1t matrices fro normals, and to compute fast matrix3x4.
			for(sint i=0;i<IDriver::MaxModelMatrix;i++)
			{
				if(_ModelViewMatrixDirtyPaletteSkin[i])
				{
					_ModelViewMatrixDirtyPaletteSkin.clear(i);

					// a. compute correct noraml matrix.
					CMatrix		&mview= _ModelViewMatrix[i];
					CMatrix		&mviewNormal= _ModelViewMatrixNormal[i];
					// copy only the rot matrix.
					mviewNormal.setRot(mview);
					// If matrix has scale...
					if(mviewNormal.hasScalePart())
					{
						// Must compute the transpose of the invert matrix. (10 times slower if not uniform scale!!)
						mviewNormal.invert();
						mviewNormal.transpose3x3();
					}
					// else, no need to do it since transpose==inverse.

					// b. compute fast 3x4 matrixs.
					_ModelViewMatrix3x4[i].set(_ModelViewMatrix[i]);
					_ModelViewMatrixNormal3x4[i].set(_ModelViewMatrixNormal[i]);

				}
			}


			// Compute vertices/normales.
			// NB: in software, skinning compute is done at render, since we don't know, at activeVB(), what
			// vertex will be used.

			// reset flags, only for given vertex range.
			// NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE==3.
			memset(&vbInf->SoftSkinFlags[first], NL3D_DRV_SOFTSKIN_VNEEDCOMPUTE, end-first);

			// For render*() to know how they must skin...
			_CurrentSoftSkinFlags= &vbInf->SoftSkinFlags[0];
			_CurrentSoftSkinSrc= (uint8*) VB.getVertexCoordPointer();
			_CurrentSoftSkinSrcStride= VB.getVertexSize();
			_CurrentSoftSkinFirst= first;
			_CurrentSoftSkinEnd= end;
			_CurrentSoftSkinPaletteSkinOff= VB.getPaletteSkinOff();
			_CurrentSoftSkinWeightOff= VB.getWeightOff(0);
			_CurrentSoftSkinVectorDst= &(vbInf->SoftSkinVertices[0]);
			// Normal ptr.
			if(flags & IDRV_VF_NORMAL)
			{
				_CurrentSoftSkinNormalDst= &(vbInf->SoftSkinNormals[0]);
				_CurrentSoftSkinNormalOff= VB.getNormalOff();
			}
			else
			{
				_CurrentSoftSkinNormalDst= NULL;
			}
		}
	}
	// Non Paletted skinning.
	//==============
	else
	{
		// \todo yoyo: TODO_SOFTWARE_SKINNIG: We must make the skinning by software. (maybe one day :) ).
		// \todo yoyo: TODO_HARDWARE_SKINNIG: we must make the skinning by hardware (Radeon, NV20 vertexprogram).

		// For now, even if weight number is better than 2, do the skinning in EXTVertexWeighting 2 matrix (if possible)

		// Choose an extension to setup a second matrix
		if (_Extensions.EXTVertexWeighting)
		{
			if(_ModelViewMatrixDirty[0])
			{
				_ModelViewMatrixDirty.clear(0);
				// By default, the first model matrix is active
				glLoadMatrixf( _ModelViewMatrix[0].get() );
			}

			if(_ModelViewMatrixDirty[1])
			{
				_ModelViewMatrixDirty.clear(1);
				// Active the second model matrix
				glMatrixMode(GL_MODELVIEW1_EXT);
				// Set it
				glLoadMatrixf( _ModelViewMatrix[1].get() );
				// Active first model matrix
				glMatrixMode(GL_MODELVIEW);
			}
		}
		// else nothing!!!
	}



	// 1. Special Normalize.
	//======================
	// NB: must enable GL_NORMALIZE when skinning is enabled or when ModelView has scale.
	enableGlNormalize( skinning || _ModelViewMatrix[0].hasScalePart() || _ForceNormalize );


	// 2. Setup Arrays.
	//===================


	// Setup Vertex / Normal.
	//=======================
	// if software palette skinning: setup correct Vertex/Normal array.
	if(paletteSkinning && !_PaletteSkinHard)
	{
		/// Must check vbinfo.
		nlassert(vbInf->SoftSkinVertices.size()==VB.getNumVertices());

		// Must point on computed Vertex array.
		glEnable(GL_VERTEX_ARRAY);
		// array is compacted.
		glVertexPointer(3,GL_FLOAT,0,&(*vbInf->SoftSkinVertices.begin()));

		// Check for normal param in vertex buffer
		if (flags & IDRV_VF_NORMAL)
		{
			/// Must check vbinfo.
			nlassert(vbInf->SoftSkinNormals.size()==VB.getNumVertices());
			// Must point on computed Normal array.
			glEnableClientState(GL_NORMAL_ARRAY);
			// array is compacted.
			glNormalPointer(GL_FLOAT,0,&(*vbInf->SoftSkinNormals.begin()));
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);
	}
	else
	{
		glEnable(GL_VERTEX_ARRAY);
		glVertexPointer(3,GL_FLOAT,VB.getVertexSize(),VB.getVertexCoordPointer());

		// Check for normal param in vertex buffer
		if (flags & IDRV_VF_NORMAL)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT,VB.getVertexSize(),VB.getNormalCoordPointer());
		}
		else
			glDisableClientState(GL_NORMAL_ARRAY);
	}


	// Setup Vertex Weight.
	//=======================
	// disable vertex weight
	bool disableVertexWeight=_Extensions.EXTVertexWeighting;

	// if skinning not paletted...
	if ( skinning && !paletteSkinning)
	{
		// 4 weights ?
		if (flags & IDRV_VF_W[3])
		{
			// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 4 matrices mode by hardware (Radeon, NV20).
			// \todo yoyo: TODO_SOFTWARE_SKINNIG: we must implement the 4 matrices mode by software.
		}
		// 3 weights ?
		else if (flags & IDRV_VF_W[2])
		{
			// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 3 matrices mode by hardware (Radeon, NV20).
			// \todo yoyo: TODO_SOFTWARE_SKINNIG: we must implement the 3 matrices mode by software.
		}
		// 2 weights ?
		else
		{
			// Check if vertex weighting extension is available
			if (_Extensions.EXTVertexWeighting)
			{
				// Active skinning
				glEnable (GL_VERTEX_WEIGHTING_EXT);

				// Don't disable it
				disableVertexWeight=false;

				// Setup
				glEnableClientState(GL_VERTEX_WEIGHT_ARRAY_EXT);
				glVertexWeightPointerEXT(1,GL_FLOAT,VB.getVertexSize(),VB.getWeightPointer());
			}
			else
			{
				// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 2 matrices mode by hardware (Radeon, NV20).
				// \todo yoyo: TODO_SOFTWARE_SKINNIG: we must implement the 2 matrices mode by software.
			}
		}
	}

	// Disable vertex weight
	if (disableVertexWeight)
		glDisable (GL_VERTEX_WEIGHTING_EXT);


	// Setup Color / UV.
	//==================
	// Check for color param in vertex buffer
	if (flags & IDRV_VF_COLOR)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,GL_UNSIGNED_BYTE,VB.getVertexSize(),VB.getColorPointer());
	}
	else
		glDisableClientState(GL_COLOR_ARRAY);

	// Active UVs.
	for(sint i=0; i<getNbTextureStages(); i++)
	{
		// normal behavior: each texture has its own UV.
		setupUVPtr(i, VB, i);
	}


	// For MultiPass Material.
	_LastVB= &VB;


	return true;
}


// ***************************************************************************
bool		CDriverGL::activeVertexBuffer(CVertexBuffer& VB)
{
	return activeVertexBuffer(VB, 0, VB.getNumVertices());
}


// ***************************************************************************
bool CDriverGL::render(CPrimitiveBlock& PB, CMaterial& Mat)
{
	// Check user code :)
	nlassert(!_MatrixSetupDirty);

	if ( !setupMaterial(Mat) )
		return false;


	// test if VB software skinning.
	//==============================
	if(_CurrentSoftSkinFlags)
	{
		uint32	*pIndex;
		uint	nIndex;

		// This may be better to flags in 2 pass (first traverse primitives, then test vertices).
		// Better sol for BTB..., because number of tests are divided by 6 (for triangles).

		// First, for all prims, indicate which vertex we must compute.
		// nothing if not already computed (ie 0), because 0&1==0.
		// Lines.
		pIndex= (uint32*)PB.getLinePointer();
		nIndex= PB.getNumLine()*2;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;
		// Tris.
		pIndex= (uint32*)PB.getTriPointer();
		nIndex= PB.getNumTri()*3;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;
		// Quads.
		pIndex= (uint32*)PB.getQuadPointer();
		nIndex= PB.getNumQuad()*4;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;


		// Second, traverse All vertices in range, testing if we must compute those vertices.
		refreshSoftwareSkinning();
	}


	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass(Mat);
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(Mat, pass);
		// draw the primitives.
		if(PB.getNumTri()!=0)
			glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
		if(PB.getNumQuad()!=0)
			glDrawElements(GL_QUADS,4*PB.getNumQuad(),GL_UNSIGNED_INT,PB.getQuadPointer());
		if(PB.getNumLine()!=0)
			glDrawElements(GL_LINES,2*PB.getNumLine(),GL_UNSIGNED_INT,PB.getLinePointer());
	}
	// end multipass.
	endMultiPass(Mat);

	return true;
}


// ***************************************************************************
void	CDriverGL::renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris)
{
	// Check user code :)
	nlassert(!_MatrixSetupDirty);

	if ( !setupMaterial(Mat) )
		return;

	
	// test if VB software skinning.
	//==============================
	if(_CurrentSoftSkinFlags)
	{
		uint32	*pIndex;
		uint	nIndex;

		// see render() for explanation.
		// First, for all prims, indicate which vertex we must compute.
		// nothing if not already computed (ie 0), because 0&1==0.
		// Tris.
		pIndex= tri;
		nIndex= ntris*3;
		for(;nIndex>0;nIndex--, pIndex++)
			_CurrentSoftSkinFlags[*pIndex]&= NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE;

		// Second, traverse All vertices in range, testing if we must compute those vertices.
		refreshSoftwareSkinning();
	}


	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass(Mat);
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(Mat, pass);
		// draw the primitives.
		if(ntris!=0)
			glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_INT, tri);
	}
	// end multipass.
	endMultiPass(Mat);

}


// ***************************************************************************
void	CDriverGL::renderPoints(CMaterial& Mat, uint32 numPoints)
{
	// Check user code :)
	nlassert(!_MatrixSetupDirty);

	if ( !setupMaterial(Mat) )
		return;	


	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass(Mat);
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(Mat, pass);
		// draw the primitives.
		if(numPoints)
			glDrawArrays(GL_POINTS,0, numPoints);
	}
	// end multipass.
	endMultiPass(Mat);
}


// ***************************************************************************
void			CDriverGL::computeSoftwareVertexSkinning(uint8 *pSrc, CVector *pDst)
{
	CMatrix3x4		*pMat;

	// \todo yoyo: TODO_OPTIMIZE: SSE verion...

	CVector			*srcVec= (CVector*)pSrc;
	CPaletteSkin	*srcPal= (CPaletteSkin*)(pSrc + _CurrentSoftSkinPaletteSkinOff);
	float			*srcWgt= (float*)(pSrc + _CurrentSoftSkinWeightOff);

	// checks indices.
	nlassert(srcPal->MatrixId[0]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[1]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[2]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[3]<IDriver::MaxModelMatrix);


	// Sum influences.
	pDst->set(0,0,0);

	// 0th matrix influence.
	pMat= _ModelViewMatrix3x4 + srcPal->MatrixId[0];
	pMat->mulAddPoint(*srcVec, srcWgt[0], *pDst);
	// 1th matrix influence.
	pMat= _ModelViewMatrix3x4 + srcPal->MatrixId[1];
	pMat->mulAddPoint(*srcVec, srcWgt[1], *pDst);
	// 2th matrix influence.
	pMat= _ModelViewMatrix3x4 + srcPal->MatrixId[2];
	pMat->mulAddPoint(*srcVec, srcWgt[2], *pDst);
	// 3th matrix influence.
	pMat= _ModelViewMatrix3x4 + srcPal->MatrixId[3];
	pMat->mulAddPoint(*srcVec, srcWgt[3], *pDst);

}


// ***************************************************************************
void			CDriverGL::computeSoftwareNormalSkinning(uint8 *pSrc, CVector *pDst)
{
	CMatrix3x4		*pMat;

	// \todo yoyo: TODO_OPTIMIZE: SSE verion...

	CVector			*srcNormal= (CVector*)(pSrc + _CurrentSoftSkinNormalOff);
	CPaletteSkin	*srcPal= (CPaletteSkin*)(pSrc + _CurrentSoftSkinPaletteSkinOff);
	float			*srcWgt= (float*)(pSrc + _CurrentSoftSkinWeightOff);

	// checks indices.
	nlassert(srcPal->MatrixId[0]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[1]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[2]<IDriver::MaxModelMatrix);
	nlassert(srcPal->MatrixId[3]<IDriver::MaxModelMatrix);

	
	// Sum influences.
	pDst->set(0,0,0);

	// 0th matrix influence.
	pMat= _ModelViewMatrixNormal3x4 + srcPal->MatrixId[0];
	pMat->mulAddVector(*srcNormal, srcWgt[0], *pDst);
	// 1th matrix influence.
	pMat= _ModelViewMatrixNormal3x4 + srcPal->MatrixId[1];
	pMat->mulAddVector(*srcNormal, srcWgt[1], *pDst);
	// 2th matrix influence.
	pMat= _ModelViewMatrixNormal3x4 + srcPal->MatrixId[2];
	pMat->mulAddVector(*srcNormal, srcWgt[2], *pDst);
	// 3th matrix influence.
	pMat= _ModelViewMatrixNormal3x4 + srcPal->MatrixId[3];
	pMat->mulAddVector(*srcNormal, srcWgt[3], *pDst);
}


// ***************************************************************************
void	CDriverGL::refreshSoftwareSkinning()
{
	// traverse All vertices in range, testing if we must compute those vertices.
	uint8		*pSrc= _CurrentSoftSkinSrc + _CurrentSoftSkinFirst*_CurrentSoftSkinSrcStride;
	uint8		*pFlag= _CurrentSoftSkinFlags + _CurrentSoftSkinFirst;
	CVector		*pVectorDst= _CurrentSoftSkinVectorDst + _CurrentSoftSkinFirst;
	CVector		*pNormalDst= _CurrentSoftSkinNormalDst + _CurrentSoftSkinFirst;
	uint		size= _CurrentSoftSkinEnd - _CurrentSoftSkinFirst;
	for(;size>0;size--)
	{
		// If we must compute this vertex.
		if(*pFlag==NL3D_DRV_SOFTSKIN_VMUSTCOMPUTE)
		{
			// Flag this vertex as computed.
			*pFlag=NL3D_DRV_SOFTSKIN_VCOMPUTED;

			// compute vertex part.
			computeSoftwareVertexSkinning(pSrc, pVectorDst);

			// compute normal part.
			if(_CurrentSoftSkinNormalDst)
			{
				computeSoftwareNormalSkinning(pSrc, pNormalDst);
			}
		}

		pSrc+= _CurrentSoftSkinSrcStride;
		pFlag++;
		pVectorDst++;
		pNormalDst++;
	}
}


// ***************************************************************************
void		CDriverGL::setupUVPtr(uint stage, CVertexBuffer &VB, uint uvId)
{
	glClientActiveTextureARB(GL_TEXTURE0_ARB+stage);
	if (VB.getVertexFormat() & IDRV_VF_UV[uvId])
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,VB.getVertexSize(),VB.getTexCoordPointer(0,uvId));
	}
	else
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}



// ***************************************************************************
IVertexBufferHard	*CDriverGL::createVertexBufferHard(uint32 vertexFormat, uint32 numVertices)
{
	return NULL;
}


// ***************************************************************************
void			CDriverGL::deleteVertexBufferHard(IVertexBufferHard *VB)
{
}


// ***************************************************************************
void			CDriverGL::activeVertexBufferHard(IVertexBufferHard *VB)
{
}



} // NL3D
