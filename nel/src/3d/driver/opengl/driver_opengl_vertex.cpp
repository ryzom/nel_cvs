/** \file driver_opengl_vertex.cpp
 * OpenGL driver implementation for vertex Buffer / render manipulation.
 *
 * $Id: driver_opengl_vertex.cpp,v 1.23 2002/03/14 18:28:20 vizerie Exp $
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

#include "stdopengl.h"

#include "3d/primitive_block.h"


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


// 500K min.
#define	NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE		(512*1024)



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
		if(!_PaletteSkinHard && ((VB.getVertexFormat() & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag) )
		{
			// Must reallocate post-rendered vertices/normals Flags.
			vbInf->SoftSkinFlags.resize(VB.getNumVertices());
			// Must reallocate post-rendered vertices/normals.
			vbInf->SoftSkinVertices.resize(VB.getNumVertices());
			// Normals onli if nomal enabled.
			if(VB.getVertexFormat() & CVertexBuffer::NormalFlag)
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
	// NB: must duplicate changes in activeVertexBufferHard()

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
	bool	skinning= ( (flags & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag);
	skinning= skinning && (_VertexMode & NL3D_VERTEX_MODE_SKINNING)!=0;
	// NB: this test if palette skin is enabled.
	bool	paletteSkinning= skinning && ( (flags & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag );


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
			if(flags & CVertexBuffer::NormalFlag)
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

	// Fence mgt.
	fenceOnCurVBHardIfNeeded(NULL);

	// For MultiPass Material.
	_LastVB.setupVertexBuffer(VB);

	// Disalbe the current vertexBufferHard if setuped.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->disable();

	// Setup the OpenGL arrays.
	setupGlArrays(_LastVB, vbInf, skinning, paletteSkinning);


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


	// Profiling.
	_PrimitiveProfileIn.NLines+= PB.getNumLine();
	_PrimitiveProfileIn.NTriangles+= PB.getNumTri();
	_PrimitiveProfileIn.NQuads+= PB.getNumQuad();
	_PrimitiveProfileOut.NLines+= PB.getNumLine() * nPass;
	_PrimitiveProfileOut.NTriangles+= PB.getNumTri() * nPass;
	_PrimitiveProfileOut.NQuads+= PB.getNumQuad() * nPass;

	// We have render some prims. inform the VBHard.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->GPURenderingAfterFence= true;

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


	// Profiling.
	_PrimitiveProfileIn.NTriangles+= ntris;
	_PrimitiveProfileOut.NTriangles+= ntris * nPass;

	// We have render some prims. inform the VBHard.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->GPURenderingAfterFence= true;
}


// ***************************************************************************
void	CDriverGL::renderSimpleTriangles(uint32 *tri, uint32 ntris)
{
	// Check user code :)
	nlassert(!_MatrixSetupDirty);
	nlassert(ntris>0);

	// Don't setup any material here.
	
	// test if VB software skinning.
	//==============================
	// NB: still test it...
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
	// NO MULTIPASS HERE!!
	// draw the primitives. (nb: ntrsi>0).
	glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_INT, tri);

	// Profiling.
	_PrimitiveProfileIn.NTriangles+= ntris;
	_PrimitiveProfileOut.NTriangles+= ntris;

	// We have render some prims. inform the VBHard.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->GPURenderingAfterFence= true;
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


	// Profiling.
	_PrimitiveProfileIn.NPoints+= numPoints;
	_PrimitiveProfileOut.NPoints+= numPoints * nPass;

	// We have render some prims. inform the VBHard.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->GPURenderingAfterFence= true;
}




// ***************************************************************************
void	CDriverGL::renderQuads(CMaterial& Mat, uint32 startIndex, uint32 numQuads)
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
		if(numQuads)
			glDrawArrays(GL_QUADS, startIndex << 2, numQuads << 2) ;
	}
	// end multipass.
	endMultiPass(Mat);


	// Profiling.
	_PrimitiveProfileIn.NQuads  += numQuads ;
	_PrimitiveProfileOut.NQuads += numQuads  * nPass;

	// We have render some prims. inform the VBHard.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->GPURenderingAfterFence= true;
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
void		CDriverGL::setupUVPtr(uint stage, CVertexBufferInfo &VB, uint uvId)
{
	_DriverGLStates.clientActiveTextureARB(stage);
	if (VB.VertexFormat & (CVertexBuffer::TexCoord0Flag<<uvId))
	{
		// Check type, if not supported, just ignore
		if (VB.Type[CVertexBuffer::TexCoord0+uvId]==CVertexBuffer::Float2)
		{
			_DriverGLStates.enableTexCoordArray(true);
			glTexCoordPointer(2,GL_FLOAT,VB.VertexSize,VB.ValuePtr[CVertexBuffer::TexCoord0+uvId]);
		}
		else
		{
			_DriverGLStates.enableTexCoordArray(false);
		}
	}
	else
		_DriverGLStates.enableTexCoordArray(false);
}



// ***************************************************************************
// ***************************************************************************
// VertexBufferHard
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool			CDriverGL::supportVertexBufferHard() const
{
	return _Extensions.NVVertexArrayRange;
}


// ***************************************************************************
uint			CDriverGL::getMaxVerticesByVertexBufferHard() const
{
	return _Extensions.NVVertexArrayRangeMaxVertex;
}


// ***************************************************************************
IVertexBufferHard	*CDriverGL::createVertexBufferHard(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, IDriver::TVBHardType vbType)
{
	if(!_Extensions.NVVertexArrayRange)
		return NULL;
	else
	{
		// check max vertex
		if(numVertices > _Extensions.NVVertexArrayRangeMaxVertex)
			return NULL;

		// Create a CVertexBufferHardGL
		CVertexBufferHardGL		*vb;
		vb= new CVertexBufferHardGL();
		// if fails
		if(!vb->init(this, vertexFormat, typeArray, numVertices, vbType))
		{
			// destroy this object.
			delete vb;
			return NULL;
		}
		else
			return _VertexBufferHardSet.insert(vb);
	}
}


// ***************************************************************************
void			CDriverGL::deleteVertexBufferHard(IVertexBufferHard *VB)
{
	// If one _CurrentVertexBufferHard enabled, first disable it.
	// This is very important (disable() flush the vertex data), because after deletion, the space is free. 
	// If the GPU has not finisehd his drawing, and if any allocation occurs on this free space, and if 
	// the user locks to fill this space (feww!) Then some data crash may results....
	if(_CurrentVertexBufferHard)
	{
		_CurrentVertexBufferHard->disable();
	}

	// Then just delete the VBuffer hard from list.
	_VertexBufferHardSet.erase(safe_cast<CVertexBufferHardGL*>(VB));
}



// ***************************************************************************
CVertexBufferHardGL::CVertexBufferHardGL() 
{
	_Driver= NULL;
	_VertexArrayRange= NULL;
	_VertexPtr= NULL;

	GPURenderingAfterFence= false;
	_FenceSet= false;
}


// ***************************************************************************
bool	CVertexBufferHardGL::init(CDriverGL *drv, uint16 vertexFormat, const uint8 *typeArray, 
								  uint32 numVertices, IDriver::TVBHardType vbType)
{
	_Driver= drv;
	_VertexArrayRange= NULL;
	_VertexPtr= NULL;

	// Init the format of the VB.
	IVertexBufferHard::initFormat(vertexFormat, typeArray, numVertices);

	// compute size to allocate.
	uint32	size= getVertexSize() * getNumVertices();

	// choose the VertexArrayRange.
	switch(vbType)
	{
	case IDriver::VBHardAGP: 
		_VertexArrayRange= &_Driver->_AGPVertexArrayRange;
		break;
	case IDriver::VBHardVRAM:
		_VertexArrayRange= &_Driver->_VRAMVertexArrayRange;
		break;
	};

	// try to allocate
	if( _VertexArrayRange && _VertexArrayRange->allocated())
	{
		 _VertexPtr= _VertexArrayRange->allocateVB(size);
	}

	// If allocation fails.
	if( !_VertexPtr )
	{
		// just destroy this object (no free()).
		_VertexArrayRange= NULL;
		return false;
	}
	else
	{
		// Ok, we can allocate the fence.
		nglGenFencesNV(1, &_Fence);
		return true;
	}
}


// ***************************************************************************
CVertexBufferHardGL::~CVertexBufferHardGL()
{
	if(_VertexArrayRange)
	{
		// Destroy Fence.
		// First wait for completion.
		finishFence();
		// then delete.
		nglDeleteFencesNV(1, &_Fence);

		// Then free the VAR.
		_VertexArrayRange->freeVB(_VertexPtr);
		_VertexPtr= NULL;
		_VertexArrayRange= NULL;
	}
}


// ***************************************************************************
void		*CVertexBufferHardGL::lock()
{
	// sync the 3d card with the system.

	// Ensure the GPU has finished with the current VBHard.
	finishFence();
	// If the user lock an activated VBHard, after rendering some primitives, we must stall the CPU
	if(GPURenderingAfterFence)
	{
		// Set a fence at the current position in the command stream.
		setFence();
		// wait for him to finish.
		finishFence();
		// And so the GPU render all our primitives.
		GPURenderingAfterFence= false;
	}


	return _VertexPtr;
}


// ***************************************************************************
void		CVertexBufferHardGL::unlock()
{
	// no op.
}


// ***************************************************************************
void			CVertexBufferHardGL::enable()
{
	if(_Driver->_CurrentVertexBufferHard != this)
	{
		nlassert(_VertexArrayRange);
		_VertexArrayRange->enable();
		_Driver->_CurrentVertexBufferHard= this;
	}
}


// ***************************************************************************
void			CVertexBufferHardGL::disable()
{
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{
		nlassert(_VertexArrayRange);
		_VertexArrayRange->disable();
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}


// ***************************************************************************
void			CVertexBufferHardGL::setFence()
{
	if(!isFenceSet())
	{
		nglSetFenceNV(_Fence, GL_ALL_COMPLETED_NV);
		_FenceSet= true;
	}
}

// ***************************************************************************
void			CVertexBufferHardGL::finishFence()
{
	if(isFenceSet())
	{
		// Stall CPU while the fence command is not reached in the GPU command stream.
		nglFinishFenceNV(_Fence);
		_FenceSet= false;
	}
}


// ***************************************************************************
bool			CVertexArrayRange::allocate(uint32 size, IDriver::TVBHardType vbType)
{
	nlassert(_VertexArrayPtr==NULL);
	_VertexArraySize= size;

#ifdef	NL_OS_WINDOWS
	// try to allocate AGP or VRAM data.
	switch(vbType)
	{
	case IDriver::VBHardAGP: 
		_VertexArrayPtr= wglAllocateMemoryNV(size, 0, 0, 0.5f);
		break;
	case IDriver::VBHardVRAM:
		_VertexArrayPtr= wglAllocateMemoryNV(size, 0, 0, 1.0f);
		break;
	};
#endif	// NL_OS_WINDOWS


	// init the allocator.
	if(_VertexArrayPtr)
	{
		/* Init with an alignment of 8. Not sure it is usefull, but GDC01_Performance.pdf papers talks about
		  "Data arrays must be 8-byte aligned". Don't know what "Data" is.
		*/
		_HeapMemory.initHeap(_VertexArrayPtr, size, 8);
	}


	return _VertexArrayPtr!=NULL;
}


// ***************************************************************************
void			CVertexArrayRange::free()
{
	// release the ptr.
	if(_VertexArrayPtr)
	{
		// reset the allocator.
		_HeapMemory.reset();


		// Free special memory.
#ifdef	NL_OS_WINDOWS
		wglFreeMemoryNV(_VertexArrayPtr);
#endif	// NL_OS_WINDOWS


		_VertexArrayPtr= NULL;
	}
}


// ***************************************************************************
void			CVertexArrayRange::enable()
{
	// if not already enabled.
	if(_Driver->_CurrentVertexArrayRange!=this)
	{
		nglVertexArrayRangeNV(_VertexArraySize, _VertexArrayPtr);
		glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);
		_Driver->_CurrentVertexArrayRange= this;
	}
}


// ***************************************************************************
void			CVertexArrayRange::disable()
{
	// if not already disabled.
	if(_Driver->_CurrentVertexArrayRange!=NULL)
	{
		glDisableClientState(GL_VERTEX_ARRAY_RANGE_NV);
		nglVertexArrayRangeNV(0, 0);
		_Driver->_CurrentVertexArrayRange= NULL;
	}
}


// ***************************************************************************
void			*CVertexArrayRange::allocateVB(uint32 size)
{
	return _HeapMemory.allocate(size);
}


// ***************************************************************************
void			CVertexArrayRange::freeVB(void	*ptr)
{
	_HeapMemory.free(ptr);
}



// ***************************************************************************
void			CDriverGL::activeVertexBufferHard(IVertexBufferHard *iVB)
{
	// NB: must duplicate changes in activeVertexBuffer()

	nlassert(iVB);
	CVertexBufferHardGL		*VB= safe_cast<CVertexBufferHardGL*>(iVB);


	uint32	flags;

	// Just to inform render*() that Matrix mode is OK.
	_MatrixSetupDirty= false;

	if (VB->getNumVertices()==0)
		return;

	// Get VB flags, to setup matrixes and arrays.
	flags=VB->getVertexFormat();


	// Skin mode.
	// NB: this test either if palette skin is enabled, or normal skinning is enabled.
	bool	skinning= ( (flags & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag);
	skinning= skinning && (_VertexMode & NL3D_VERTEX_MODE_SKINNING)!=0;
	// NB: this test if palette skin is enabled.
	bool	paletteSkinning= skinning && ( (flags & CVertexBuffer::PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag );


	// If _PaletteSkinHard is not supported, disable all skinning.
	// No sofwarte skinning, because can't read from AGP mem.
	if(paletteSkinning && !_PaletteSkinHard)
	{
		skinning= false;
		paletteSkinning= false;
	}


	// 0. Setup Matrixes.
	//===================

	// Check if view matrix has been modified
	if (_ViewMatrixSetupDirty)
		// Recompute stuff touched by the view matrix
		cleanViewMatrix ();

	// Check view matrix is good
	nlassert (_ViewMatrixSetupDirty==false);

	// never do software skinning.
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
		// No sofwarte skinning.
	}
	// Non Paletted skinning.
	//==============
	else
	{
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

	// Fence mgt.
	fenceOnCurVBHardIfNeeded(VB);

	// For MultiPass Material.
	_LastVB.setupVertexBufferHard(*VB);

	// Enable the vertexArrayRange of this array.
	VB->enable();

	// Setup the OpenGL arrays.
	setupGlArrays(_LastVB, NULL, skinning, paletteSkinning);

}


// ***************************************************************************
const uint		CDriverGL::NumCoordinatesType[CVertexBuffer::NumType]=
{
	1,	// Double1
	1,	// Float1
	1,	// Short1
	2,	// Double2
	2,	// Float2
	2,	// Short2
	3,	// Double3
	3,	// Float3
	3,	// Short3
	4,	// Double4
	4,	// Float4
	4,	// Short4
	4	// UChar4
};


// ***************************************************************************
const uint		CDriverGL::GLType[CVertexBuffer::NumType]=
{
	GL_DOUBLE,	// Double1
	GL_FLOAT,	// Float1
	GL_SHORT,	// Short1
	GL_DOUBLE,	// Double2
	GL_FLOAT,	// Float2
	GL_SHORT,	// Short2
	GL_DOUBLE,	// Double3
	GL_FLOAT,	// Float3
	GL_SHORT,	// Short3
	GL_DOUBLE,	// Double4
	GL_FLOAT,	// Float4
	GL_SHORT,	// Short4
	GL_UNSIGNED_BYTE	// UChar4
};


// ***************************************************************************
const uint		CDriverGL::GLVertexAttribIndex[CVertexBuffer::NumValue]=
{
	0,	// Position
	2,	// Normal
	8,	// TexCoord0
	9,	// TexCoord1
	10,	// TexCoord2
	11,	// TexCoord3
	12,	// TexCoord4
	13,	// TexCoord5
	14,	// TexCoord6
	15,	// TexCoord7
	3,	// PrimaryColor
	4,	// SecondaryColor
	1,	// Weight
	6,	// Empty (PaletteSkin)
	5,	// Fog
	7,	// Empty
};


// ***************************************************************************
void		CDriverGL::setupGlArrays(CVertexBufferInfo &vb, CVBDrvInfosGL *vbInf, bool skinning, bool paletteSkinning)
{
	uint32	flags= vb.VertexFormat;

	// If change of setup type, must disable olds.
	//=======================

	// If last was a VertexProgram setup, and now it is a standard GL array setup.
	if( _LastSetupGLArrayVertexProgram && !isVertexProgramEnabled () )
	{
		// Disable all VertexAttribs.
		for (uint value=0; value<CVertexBuffer::NumValue; value++)
		{
			// Index
			uint glIndex=GLVertexAttribIndex[value];
			_DriverGLStates.enableVertexAttribArray(glIndex, false);
		}
		_DriverGLStates.enableColorArray(false);
		_DriverGLStates.enableSecondaryColorArray(false);

		// no more a vertex program setup.
		_LastSetupGLArrayVertexProgram= false;
	}

	// If last was a standard GL array setup, and now it is a VertexProgram setup.
	if( !_LastSetupGLArrayVertexProgram && isVertexProgramEnabled () )
	{
		// Disable all standards ptrs.
		_DriverGLStates.enableVertexArray(false);
		_DriverGLStates.enableNormalArray(false);
		if(_Extensions.EXTVertexWeighting)
			_DriverGLStates.enableWeightArray(false);
		_DriverGLStates.enableColorArray(false);
		for(sint i=0; i<getNbTextureStages(); i++)
		{
			_DriverGLStates.clientActiveTextureARB(i);
			_DriverGLStates.enableTexCoordArray(false);
		}


		// now, vertex program setup.
		_LastSetupGLArrayVertexProgram= true;
	}

	// Setup Vertex / Normal.
	//=======================

	// Use a vertex program ?
	if (!isVertexProgramEnabled ())
	{
		// Channel type is Float3 ?
		// if software palette skinning: setup correct Vertex/Normal array.
		if(paletteSkinning && !_PaletteSkinHard)
		{
			/// Must check vbinfo.
			nlassert(vbInf->SoftSkinVertices.size()==vb.NumVertices);

			// Check it is the good type
			nlassert (vb.Type[CVertexBuffer::Position]==CVertexBuffer::Float3);

			// Must point on computed Vertex array.
			_DriverGLStates.enableVertexArray(true);

			// array is compacted.
			glVertexPointer(3,GL_FLOAT,0,&(*vbInf->SoftSkinVertices.begin()));

			// Check for normal param in vertex buffer
			if (flags & CVertexBuffer::NormalFlag)
			{
				// Check type
				nlassert (vb.Type[CVertexBuffer::Normal]==CVertexBuffer::Float3);

				/// Must check vbinfo.
				nlassert(vbInf->SoftSkinNormals.size()==vb.NumVertices);

				// Must point on computed Normal array.
				_DriverGLStates.enableNormalArray(true);

				// array is compacted.
				glNormalPointer(GL_FLOAT,0,&(*vbInf->SoftSkinNormals.begin()));
			}
			else
			{
				_DriverGLStates.enableNormalArray(false);
			}
		}
		else
		{
			// Check type
			nlassert (vb.Type[CVertexBuffer::Position]==CVertexBuffer::Float3);

			_DriverGLStates.enableVertexArray(true);
			glVertexPointer(3,GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Position]);

			// Check for normal param in vertex buffer
			if (flags & CVertexBuffer::NormalFlag)
			{
				// Check type
				nlassert (vb.Type[CVertexBuffer::Normal]==CVertexBuffer::Float3);

				_DriverGLStates.enableNormalArray(true);
				glNormalPointer(GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Normal]);
			}
			else
			{
				_DriverGLStates.enableNormalArray(false);
			}
		}

		// Setup Vertex Weight.
		//=======================
		// disable vertex weight
		bool disableVertexWeight= _Extensions.EXTVertexWeighting;

		// if skinning not paletted...
		if ( skinning && !paletteSkinning)
		{
			// 4 weights ?
			if (vb.NumWeight==4)
			{
				// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 4 matrices mode by hardware (Radeon, NV20).
			}
			// 3 weights ?
			else if (vb.NumWeight==3)
			{
				// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 3 matrices mode by hardware (Radeon, NV20).
			}
			// 2 weights ?
			else
			{
				// Check if vertex weighting extension is available
				if (_Extensions.EXTVertexWeighting)
				{
					// Check type
					nlassert (vb.Type[CVertexBuffer::Weight]==CVertexBuffer::Float1);

					// Active skinning
					glEnable (GL_VERTEX_WEIGHTING_EXT);

					// Don't disable it
					disableVertexWeight=false;

					// Setup
					_DriverGLStates.enableWeightArray(true);
					nglVertexWeightPointerEXT(1, GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Weight]);
				}
				else
				{
					// \todo yoyo: TODO_HARDWARE_SKINNIG: we must implement the 2 matrices mode by hardware (Radeon, NV20).
				}
			}
		}

		// Disable vertex weight
		if (disableVertexWeight)
			_DriverGLStates.enableWeightArray(false);

		// Setup Color / UV.
		//==================
		// Check for color param in vertex buffer
		if (flags & CVertexBuffer::PrimaryColorFlag)
		{
			// Check type
			nlassert (vb.Type[CVertexBuffer::PrimaryColor]==CVertexBuffer::UChar4);

			_DriverGLStates.enableColorArray(true);
			glColorPointer(4,GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[CVertexBuffer::PrimaryColor]);
		}
		else
			_DriverGLStates.enableColorArray(false);

		// Active UVs.
		for(sint i=0; i<getNbTextureStages(); i++)
		{
			// normal behavior: each texture has its own UV.
			setupUVPtr(i, vb, i);
		}
	}
	else
	{
		// Setup vertex program attributes.
		//=================================

		// Extension must exist
		nlassert (_Extensions.NVVertexProgram);

		// For each value
		for (uint value=0; value<CVertexBuffer::NumValue; value++)
		{
			// Flag
			uint16 flag=1<<value;

			// Type
			CVertexBuffer::TType type=vb.Type[value];

			// Index
			uint glIndex=GLVertexAttribIndex[value];

			// Not setuped value and used
			if (flags & flag)
			{
				/* OpenGL Driver Bug with VertexProgram, UChar4 type, and VertexArrayRange.
					Don't work and lead to very poor performance (1/10) (VAR is "disabled").
				*/
				// Test if can use glColorPointer() / glSecondaryColorPointerEXT() instead.
				if( (glIndex==3 || glIndex==4) )
				{
					if( type == CVertexBuffer::UChar4 )
					{
						// Must disable VertexAttrib array.
						_DriverGLStates.enableVertexAttribArray(glIndex, false);

						// Active this value, with standard gl calls
						if(glIndex==3)
						{
							// Primary color
							_DriverGLStates.enableColorArray(true);
							glColorPointer(4,GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[value]);
						}
						else
						{
							// Secondary color
							_DriverGLStates.enableSecondaryColorArray(true);
							nglSecondaryColorPointerEXT(4,GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[value]);
						}
					}
					else
					{
						// Can use normal VertexAttribArray.
						// Disable first standard Color Array.
						if(glIndex==3)
							_DriverGLStates.enableColorArray(false);
						else
							_DriverGLStates.enableSecondaryColorArray(false);

						// Active this value
						_DriverGLStates.enableVertexAttribArray(glIndex, true);
						nglVertexAttribPointerNV (glIndex, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
				}
				// Else normal case, can't do anything for other values with UChar4....
				else
				{
					// Active this value
					_DriverGLStates.enableVertexAttribArray(glIndex, true);
					nglVertexAttribPointerNV (glIndex, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
				}
			}
			else
			{
				_DriverGLStates.enableVertexAttribArray(glIndex, false);
				/* OpenGL Driver Bug with VertexProgram, UChar4 type, and VertexArrayRange.
					Must also disable colorArray in standard gl calls.
				*/
				if(glIndex==3)
					_DriverGLStates.enableColorArray(false);
				else if(glIndex==4)
					_DriverGLStates.enableSecondaryColorArray(false);
			}
		}
	}
}


// ***************************************************************************
void		CVertexBufferInfo::setupVertexBuffer(CVertexBuffer &vb)
{
	sint	i;
	uint	flags= vb.getVertexFormat();
	VertexFormat= flags;
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();
	NumWeight= vb.getNumWeight();

	// Get value pointer
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		// Value used ?
		if (VertexFormat&(1<<i))
		{
			// Get the pointer
			ValuePtr[i]=vb.getValueEx ((CVertexBuffer::TValue)i);

			// Type of the value
			Type[i]=vb.getValueType (i);
		}
	}
}


// ***************************************************************************
void		CVertexBufferInfo::setupVertexBufferHard(CVertexBufferHardGL &vb)
{
	sint	i;
	uint	flags= vb.getVertexFormat();
	VertexFormat= flags;
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();
	NumWeight= vb.getNumWeight();

	// Get value pointer
	for (i=0; i<CVertexBuffer::NumValue; i++)
	{
		// Value used ?
		if (VertexFormat&(1<<i))
		{
			// Get the pointer
			ValuePtr[i]=vb.getValueEx(i);

			// Type of the value
			Type[i]=vb.getValueType (i);
		}
	}
}



// ***************************************************************************
void			CDriverGL::resetVertexArrayRange()
{
	if(_CurrentVertexBufferHard)
	{
		_CurrentVertexBufferHard->disable();
	}
	// Clear any VertexBufferHard created.
	_VertexBufferHardSet.clear();

	// After, Clear the 2 vertexArrayRange, if any.
	_AGPVertexArrayRange.free();
	_VRAMVertexArrayRange.free();
}


// ***************************************************************************
bool			CDriverGL::initVertexArrayRange(uint agpMem, uint vramMem)
{
	if(!supportVertexBufferHard())
		return false;

	// First, reset any VBHard created.
	resetVertexArrayRange();
	bool	ok= true;

	// Try to allocate AGPMemory.
	if(agpMem>0)
	{
		agpMem&= ~15;	// ensure 16-bytes aligned mem count (maybe usefull :) ).
		agpMem= max(agpMem, (uint)NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE);
		while(agpMem>= NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			if(_AGPVertexArrayRange.allocate(agpMem, IDriver::VBHardAGP))
			{
				nlinfo("VAR: %.d vertices supported", _Extensions.NVVertexArrayRangeMaxVertex);
				nlinfo("VAR: Success to allocate %.1f Mo of AGP VAR Ram", agpMem / 1000000.f);
				break;
			}
			else
			{
				agpMem/=2;
				agpMem &=~15;
			}
		}

		if(agpMem< NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			nlinfo("VAR: %.d vertices supported", _Extensions.NVVertexArrayRangeMaxVertex);
			nlinfo("VAR: Failed to allocate %.1f Mo of AGP VAR Ram", NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE / 1000000.f);
			ok= false;
		}
	}


	// Try to allocate VRAMMemory.
	if(vramMem>0)
	{
		vramMem&= ~15;	// ensure 16-bytes aligned mem count (maybe usefull :) ).
		vramMem= max(vramMem, (uint)NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE);
		while(vramMem>= NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			if(_VRAMVertexArrayRange.allocate(vramMem, IDriver::VBHardVRAM))
				break;
			else
			{
				vramMem/=2;
				vramMem &=~15;
			}
		}

		if(vramMem< NL3D_DRV_VERTEXARRAY_MINIMUM_SIZE)
		{
			ok= false;
		}
	}


	return ok;
}


// ***************************************************************************
void				CDriverGL::fenceOnCurVBHardIfNeeded(CVertexBufferHardGL *newVBHard)
{
	// If old VB is a VBHard, and if we do not activate the same (NB: newVBHard==NULL if not a VBHard).
	if(_CurrentVertexBufferHard!=NULL && _CurrentVertexBufferHard!=newVBHard)
	{
		// If some render() have been done with this VB.
		if( _CurrentVertexBufferHard->GPURenderingAfterFence )
		{
			// If an old fence is activated, we wait for him.
			/* NB: performance issue: maybe a wait for nothing in some cases like this one:
				render with VBHard_A	---- end with a fence
				render with VBHard_B	....
				render with VBHard_A	---- end: must finish prec fence before setting a new one.

				This is not a hard issue, if we suppose first A is finished to be rendered during render of VBHard_B.
			*/
			_CurrentVertexBufferHard->finishFence();
			// Since we won't work with this VB for a long time, we set a fence.
			_CurrentVertexBufferHard->setFence();
			_CurrentVertexBufferHard->GPURenderingAfterFence= false;
		}
	}
}


} // NL3D
