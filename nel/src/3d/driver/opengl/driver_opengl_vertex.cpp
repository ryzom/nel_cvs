/** \file driver_opengl_vertex.cpp
 * OpenGL driver implementation for vertex Buffer / render manipulation.
 *
 * $Id: driver_opengl_vertex.cpp,v 1.36 2003/03/17 17:32:02 berenguier Exp $
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
#include "driver_opengl_vertex_buffer_hard.h"


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
		// nop
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

	if (VB.getNumVertices()==0)
		return true;

	nlassert(end<=VB.getNumVertices());
	nlassert(first<=end);

	// Get VB flags, to setup matrixes and arrays.
	flags=VB.getVertexFormat();


	// 2. Setup Arrays.
	//===================

	// Fence mgt.
	fenceOnCurVBHardIfNeeded(NULL);

	// For MultiPass Material.
	_LastVB.setupVertexBuffer(VB);

	// Disable the current vertexBufferHard if setuped.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->disable();

	// Setup the OpenGL arrays.
	setupGlArrays(_LastVB);


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
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(Mat) )
		return false;


	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(pass);		
		// draw the primitives.
		if(PB.getNumTri()!=0)
			glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
		if(PB.getNumQuad()!=0)
			glDrawElements(GL_QUADS,4*PB.getNumQuad(),GL_UNSIGNED_INT,PB.getQuadPointer());
		if(PB.getNumLine()!=0)
			glDrawElements(GL_LINES,2*PB.getNumLine(),GL_UNSIGNED_INT,PB.getLinePointer());
	}
	// end multipass.
	endMultiPass();


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
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(Mat) )
		return;

	
	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(pass);
		// draw the primitives.
		if(ntris!=0)
			glDrawElements(GL_TRIANGLES,3*ntris,GL_UNSIGNED_INT, tri);
	}
	// end multipass.
	endMultiPass();


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
	nlassert(ntris>0);

	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();


	// Don't setup any material here.
	
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
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(Mat) )
		return;	


	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(pass);
		// draw the primitives.
		if(numPoints)
			glDrawArrays(GL_POINTS,0, numPoints);
	}
	// end multipass.
	endMultiPass();


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
	// update matrix and Light in OpenGL if needed
	refreshRenderSetup();

	// setup material
	if ( !setupMaterial(Mat) )
		return;	


	// render primitives.
	//==============================
	// start multipass.
	uint	nPass;
	nPass= beginMultiPass();
	// draw all passes.
	for(uint pass=0;pass<nPass; pass++)
	{
		// setup the pass.
		setupPass(pass);
		// draw the primitives.
		if(numQuads)
			glDrawArrays(GL_QUADS, startIndex << 2, numQuads << 2) ;
	}
	// end multipass.
	endMultiPass();


	// Profiling.
	_PrimitiveProfileIn.NQuads  += numQuads ;
	_PrimitiveProfileOut.NQuads += numQuads  * nPass;

	// We have render some prims. inform the VBHard.
	if(_CurrentVertexBufferHard)
		_CurrentVertexBufferHard->GPURenderingAfterFence= true;
}


// ***************************************************************************
void		CDriverGL::setupUVPtr(uint stage, CVertexBufferInfo &VB, uint uvId)
{
	// sould not be called with vertex program Array setuped.
	nlassert(!_LastSetupGLArrayVertexProgram);

	_DriverGLStates.clientActiveTextureARB(stage);
	if (VB.VertexFormat & (CVertexBuffer::TexCoord0Flag<<uvId))
	{
		// Check type, if not supported, just ignore
		if (VB.Type[CVertexBuffer::TexCoord0+uvId]==CVertexBuffer::Float2)
		{
			_DriverGLStates.enableTexCoordArray(true);
			// Setup ATI VBHard or std ptr.
			if(VB.ATIVBHardMode)
				nglArrayObjectATI(GL_TEXTURE_COORD_ARRAY, 2, GL_FLOAT, VB.VertexSize, 
					VB.ATIVertexObjectId, VB.ATIValueOffset[CVertexBuffer::TexCoord0+uvId]);
			else
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
void		CDriverGL::mapTextureStageToUV(uint stage, uint uv)
{
	// Just call it for last VertexBuffer setuped.
	setupUVPtr(stage, _LastVB, uv);
}



// ***************************************************************************
// ***************************************************************************
// VertexBufferHard
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool			CDriverGL::supportVertexBufferHard() const
{
	return _SupportVBHard;
}


// ***************************************************************************
bool			CDriverGL::slowUnlockVertexBufferHard() const
{
	return _SlowUnlockVBHard;
}


// ***************************************************************************
uint			CDriverGL::getMaxVerticesByVertexBufferHard() const
{
	return _MaxVerticesByVBHard;
}


// ***************************************************************************
IVertexBufferHard	*CDriverGL::createVertexBufferHard(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, IDriver::TVBHardType vbType, const uint8 *uvRouting)
{
	// choose the VertexArrayRange of good type
	IVertexArrayRange	*vertexArrayRange= NULL;
	switch(vbType)
	{
	case IDriver::VBHardAGP: 
		vertexArrayRange= _AGPVertexArrayRange;
		break;
	case IDriver::VBHardVRAM:
		vertexArrayRange= _VRAMVertexArrayRange;
		break;
	};

	// If this one at least created (an extension support it).
	if( !vertexArrayRange )
		return NULL;
	else
	{
		// check max vertex
		if(numVertices > _MaxVerticesByVBHard)
			return NULL;

		// Create a CVertexBufferHardGL
		IVertexBufferHardGL		*vb;
		// let the VAR create the vbhard.
		vb= vertexArrayRange->createVBHardGL(vertexFormat, typeArray, numVertices, uvRouting);
		// if fails
		if(!vb)
		{
			return NULL;
		}
		else
		{
			// insert in list.
			return _VertexBufferHardSet.insert(vb);
		}
	}
}


// ***************************************************************************
void			CDriverGL::deleteVertexBufferHard(IVertexBufferHard *VB)
{
	// If one _CurrentVertexBufferHard enabled, first End its drawning, and disable it.
	// This is very important, because after deletion, the space is free. 
	// If the GPU has not finisehd his drawing, and if any allocation occurs on this free space, and if 
	// the user locks to fill this space (feww!) Then some data crash may results....
	if(_CurrentVertexBufferHard)
	{
		// Must ensure it has ended any drawing
		_CurrentVertexBufferHard->lock();
		_CurrentVertexBufferHard->unlock();
		// disable the VBHard.
		_CurrentVertexBufferHard->disable();
	}

	// Then just delete the VBuffer hard from list.
	_VertexBufferHardSet.erase(safe_cast<IVertexBufferHardGL*>(VB));
}



// ***************************************************************************
void			CDriverGL::activeVertexBufferHard(IVertexBufferHard *iVB)
{
	// NB: must duplicate changes in activeVertexBuffer()

	nlassert(iVB);
	IVertexBufferHardGL		*VB= safe_cast<IVertexBufferHardGL*>(iVB);

	uint32	flags;

	if (VB->getNumVertices()==0)
		return;

	// Get VB flags, to setup matrixes and arrays.
	flags=VB->getVertexFormat();


	// 2. Setup Arrays.
	//===================

	// Fence mgt.
	fenceOnCurVBHardIfNeeded(VB);

	// For MultiPass Material.
	_LastVB.setupVertexBufferHard(*VB);

	// Enable the vertexArrayRange of this array.
	VB->enable();

	// Setup the OpenGL arrays.
	setupGlArrays(_LastVB);

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
void		CDriverGL::setupGlArraysStd(CVertexBufferInfo &vb)
{
	uint32	flags= vb.VertexFormat;
	// setup vertex ptr.
	//-----------
	uint numVertexCoord = CVertexBuffer::NumComponentsType[vb.Type[CVertexBuffer::Position]];
	nlassert (numVertexCoord >= 2);

	_DriverGLStates.enableVertexArray(true);
	// Setup ATI VBHard or std ptr.
	if(vb.ATIVBHardMode)
		nglArrayObjectATI(GL_VERTEX_ARRAY, numVertexCoord, GL_FLOAT, vb.VertexSize, 
			vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::Position]);
	else
		glVertexPointer(numVertexCoord, GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Position]);


	// setup normal ptr.
	//-----------
	// Check for normal param in vertex buffer
	if (flags & CVertexBuffer::NormalFlag)
	{
		// Check type
		nlassert (vb.Type[CVertexBuffer::Normal]==CVertexBuffer::Float3);

		_DriverGLStates.enableNormalArray(true);
		// Setup ATI VBHard or std ptr.
		if(vb.ATIVBHardMode)
			nglArrayObjectATI(GL_NORMAL_ARRAY, 3, GL_FLOAT, vb.VertexSize, 
				vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::Normal]);
		else
			glNormalPointer(GL_FLOAT, vb.VertexSize, vb.ValuePtr[CVertexBuffer::Normal]);
	}
	else
	{
		_DriverGLStates.enableNormalArray(false);
	}


	// Setup Color
	//-----------
	// Check for color param in vertex buffer
	if (flags & CVertexBuffer::PrimaryColorFlag)
	{
		// Check type
		nlassert (vb.Type[CVertexBuffer::PrimaryColor]==CVertexBuffer::UChar4);

		_DriverGLStates.enableColorArray(true);
		// Setup ATI VBHard or std ptr.
		if(vb.ATIVBHardMode)
			nglArrayObjectATI(GL_COLOR_ARRAY, 4, GL_UNSIGNED_BYTE, vb.VertexSize, 
				vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::PrimaryColor]);
		else
			glColorPointer(4,GL_UNSIGNED_BYTE, vb.VertexSize, vb.ValuePtr[CVertexBuffer::PrimaryColor]);
	}
	else
		_DriverGLStates.enableColorArray(false);


	// Setup Uvs
	//-----------
	// Get the routing
	for(sint i=0; i<inlGetNumTextStages(); i++)
	{
		// normal behavior: each texture has its own UV.
		setupUVPtr(i, vb, vb.UVRouting[i]);
	}
}



// ***************************************************************************
void		CDriverGL::toggleGlArraysForNVVertexProgram()
{
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
		_DriverGLStates.enableColorArray(false);
		for(sint i=0; i<inlGetNumTextStages(); i++)
		{
			_DriverGLStates.clientActiveTextureARB(i);
			_DriverGLStates.enableTexCoordArray(false);
		}


		// now, vertex program setup.
		_LastSetupGLArrayVertexProgram= true;
	}
}


// ***************************************************************************
void		CDriverGL::toggleGlArraysForEXTVertexShader()
{
	// If change of setup type, must disable olds.
	//=======================


	// If last was a VertexProgram setup, and now it is a standard GL array setup.
	if( _LastSetupGLArrayVertexProgram && !isVertexProgramEnabled () )
	{
		CVertexProgram *vp = _LastSetuppedVP;
		if (vp)
		{		
			CVertexProgamDrvInfosGL *drvInfo = NLMISC::safe_cast<CVertexProgamDrvInfosGL *>((IVertexProgramDrvInfos *) vp->_DrvInfo);
			if (drvInfo)
			{
				// Disable all VertexAttribs.
				for (uint value=0; value<CVertexBuffer::NumValue; value++)
				{
					_DriverGLStates.enableVertexAttribArrayForEXTVertexShader(value, false, drvInfo->Variants);
				}
			}										
		}
		// no more a vertex program setup.
		_LastSetupGLArrayVertexProgram= false;
	}

	// If last was a standard GL array setup, and now it is a VertexProgram setup.
	if( !_LastSetupGLArrayVertexProgram && isVertexProgramEnabled () )
	{
		// Disable all standards ptrs.
		_DriverGLStates.enableVertexArray(false);
		_DriverGLStates.enableNormalArray(false);
		_DriverGLStates.enableColorArray(false);
		for(sint i=0; i<inlGetNumTextStages(); i++)
		{
			_DriverGLStates.clientActiveTextureARB(i);
			_DriverGLStates.enableTexCoordArray(false);
		}


		// now, vertex program setup.
		_LastSetupGLArrayVertexProgram= true;
	}
}

// ***************************************************************************
void		CDriverGL::setupGlArraysForNVVertexProgram(CVertexBufferInfo &vb)
{			
	uint32	flags= vb.VertexFormat;
	
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


// ***************************************************************************
void		CDriverGL::setupGlArraysForEXTVertexShader(CVertexBufferInfo &vb)
{

	CVertexProgram *vp = _LastSetuppedVP;
	if (!vp) return;		
	CVertexProgamDrvInfosGL *drvInfo = NLMISC::safe_cast<CVertexProgamDrvInfosGL *>((IVertexProgramDrvInfos *) vp->_DrvInfo);
	if (!drvInfo) return;
	
	uint32	flags= vb.VertexFormat;

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
		if (flags & flag & drvInfo->UsedVertexComponents)
		{						
			_DriverGLStates.enableVertexAttribArrayForEXTVertexShader(glIndex, true, drvInfo->Variants);
			// use variant or open gl standard array
			switch(value)
			{
					case CVertexBuffer::Position: // position 
					{					
						nlassert(NumCoordinatesType[type] >= 2);
						if(vb.ATIVBHardMode) nglArrayObjectATI(GL_VERTEX_ARRAY, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::Position]);
					    else                 glVertexPointer(NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Weight: // skin weight
					{
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						if(vb.ATIVBHardMode) nglVariantArrayObjectATI(drvInfo->Variants[CDriverGL::EVSSkinWeightVariant], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::Weight]);
					    else                 nglVariantPointerEXT(drvInfo->Variants[CDriverGL::EVSSkinWeightVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Normal: // normal
					{
						nlassert(NumCoordinatesType[type] == 3); // must have 3 components for normals
						if(vb.ATIVBHardMode) nglArrayObjectATI(GL_NORMAL_ARRAY, 3, GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[value]);
					    else                 glNormalPointer(GLType[type], vb.VertexSize, vb.ValuePtr[CVertexBuffer::Normal]);						
					}
					break;
					case CVertexBuffer::PrimaryColor: // color
					{
						nlassert(NumCoordinatesType[type] >= 3); // must have 3 or 4 components for primary color
						if(vb.ATIVBHardMode) nglArrayObjectATI(GL_COLOR_ARRAY, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::PrimaryColor]);
					    else                 glColorPointer(NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}						
					break;
					case CVertexBuffer::SecondaryColor: // secondary color
					{								
						// implemented using a variant, as not available with EXTVertexShader
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						if(vb.ATIVBHardMode) nglVariantArrayObjectATI(drvInfo->Variants[CDriverGL::EVSSecondaryColorVariant], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::SecondaryColor]);
					    else                 nglVariantPointerEXT(drvInfo->Variants[CDriverGL::EVSSecondaryColorVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);													
					}						
					break;
					case CVertexBuffer::Fog: // fog coordinate
					{
						// implemented using a variant
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						if(vb.ATIVBHardMode) nglVariantArrayObjectATI(drvInfo->Variants[CDriverGL::EVSFogCoordsVariant], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::Fog]);
						else                 nglVariantPointerEXT(drvInfo->Variants[CDriverGL::EVSFogCoordsVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}						
					break;
					case CVertexBuffer::PaletteSkin: // palette skin
					{					
						// implemented using a variant
						nlassert(NumCoordinatesType[type] == 4); // variant, only 4 component supported
						if(vb.ATIVBHardMode) nglVariantArrayObjectATI(drvInfo->Variants[CDriverGL::EVSPaletteSkinVariant], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[CVertexBuffer::PaletteSkin]);
						else                 nglVariantPointerEXT(drvInfo->Variants[CDriverGL::EVSPaletteSkinVariant], GLType[type], vb.VertexSize, vb.ValuePtr[value]);
					}
					break;
					case CVertexBuffer::Empty: // empty
						nlstop
					break;
					case CVertexBuffer::TexCoord0:
					case CVertexBuffer::TexCoord1:
					case CVertexBuffer::TexCoord2:
					case CVertexBuffer::TexCoord3:
					case CVertexBuffer::TexCoord4:
					case CVertexBuffer::TexCoord5:
					case CVertexBuffer::TexCoord6:
					case CVertexBuffer::TexCoord7:
					{			
						_DriverGLStates.clientActiveTextureARB(value - CVertexBuffer::TexCoord0);
						if(vb.ATIVBHardMode) nglArrayObjectATI(GL_TEXTURE_COORD_ARRAY, NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ATIVertexObjectId, vb.ATIValueOffset[value]);
						else glTexCoordPointer(NumCoordinatesType[type], GLType[type], vb.VertexSize, vb.ValuePtr[value ]);
					}
					break;
					default:
						nlstop; // invalid value
					break;					
			}			
		}
		else
		{
			_DriverGLStates.enableVertexAttribArrayForEXTVertexShader(glIndex, false, drvInfo->Variants);
		}
	}	
}



// ***************************************************************************
void		CDriverGL::setupGlArrays(CVertexBufferInfo &vb)
{
	uint32	flags= vb.VertexFormat;

	/** \todo yoyo, or nico: this code should change with ATI VertexProgram.
	 *	For now, ATI VBHard is only coded for non-VertexProgram case.
	 */

	// Standard case (NVVertexProgram or no vertex program case)
	if (_Extensions.NVVertexProgram || !_Extensions.EXTVertexShader)
	{	
		toggleGlArraysForNVVertexProgram();
		// Use a vertex program ?
		if (!isVertexProgramEnabled ())
		{
			setupGlArraysStd(vb);
		}
		else
		{		
			setupGlArraysForNVVertexProgram(vb);
		}
	}	
	else // EXTVertexShader case
	{
		toggleGlArraysForEXTVertexShader();
		// Use a vertex program ?
		if (!isVertexProgramEnabled ())
		{
			setupGlArraysStd(vb);
		}
		else
		{		
			setupGlArraysForEXTVertexShader(vb);
		}		
	}
	
	// Reset specials flags.
	_LastVertexSetupIsLightMap= false;
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

	// No VBhard.
	ATIVBHardMode= false;

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

	// Copy the UVRouting table
	const uint8 *uvRouting = vb.getUVRouting();
	for (i=0; i<CVertexBuffer::MaxStage; i++)
	{
		UVRouting[i] = uvRouting[i];
	}
}


// ***************************************************************************
void		CVertexBufferInfo::setupVertexBufferHard(IVertexBufferHardGL &vb)
{
	sint	i;
	uint	flags= vb.getVertexFormat();
	VertexFormat= flags;
	VertexSize= vb.getVertexSize();
	NumVertices= vb.getNumVertices();
	NumWeight= vb.getNumWeight();

	// Not ATI VBHard by default
	ATIVBHardMode= false;

	// Setup differs from ATI or NVidia VBHard.
	if(vb.NVidiaVertexBufferHard)
	{
		CVertexBufferHardGLNVidia	&vbHardNV= static_cast<CVertexBufferHardGLNVidia&>(vb);

		// Get value pointer
		for (i=0; i<CVertexBuffer::NumValue; i++)
		{
			// Value used ?
			if (VertexFormat&(1<<i))
			{
				// Get the pointer
				ValuePtr[i]= vbHardNV.getNVidiaValueEx(i);

				// Type of the value
				Type[i]= vbHardNV.getValueType (i);
			}
		}
	}
	else
	{
		nlassert(vb.ATIVertexBufferHard);
		CVertexBufferHardGLATI	&vbHardATI= static_cast<CVertexBufferHardGLATI&>(vb);

		// special setup in setupGlArrays()...
		ATIVBHardMode= true;

		// store the VertexObject Id.
		ATIVertexObjectId= vbHardATI.getATIVertexObjectId();

		// Get value offset
		for (i=0; i<CVertexBuffer::NumValue; i++)
		{
			// Value used ?
			if (VertexFormat&(1<<i))
			{
				// Get the pointer
				ATIValueOffset[i]= vbHardATI.getATIValueOffset(i);

				// Type of the value
				Type[i]= vbHardATI.getValueType (i);
			}
		}
	}

	// Copy the UVRouting table
	const uint8 *uvRouting = vb.getUVRouting ();
	for (i=0; i<CVertexBuffer::MaxStage; i++)
	{
		UVRouting[i] = uvRouting[i];
	}
}



// ***************************************************************************
void			CDriverGL::resetVertexArrayRange()
{
	if(_CurrentVertexBufferHard)
	{
		// Must ensure it has ended any drawing
		_CurrentVertexBufferHard->lock();
		_CurrentVertexBufferHard->unlock();
		// disable it
		_CurrentVertexBufferHard->disable();
	}
	// Clear any VertexBufferHard created.
	_VertexBufferHardSet.clear();

	// After, Clear the 2 vertexArrayRange, if any.
	if(_AGPVertexArrayRange)
		_AGPVertexArrayRange->free();
	if(_VRAMVertexArrayRange)
		_VRAMVertexArrayRange->free();
}


// ***************************************************************************
bool			CDriverGL::initVertexArrayRange(uint agpMem, uint vramMem)
{
	if(!supportVertexBufferHard())
		return false;

	// must be supported
	if(!_AGPVertexArrayRange || !_VRAMVertexArrayRange)
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
			if(_AGPVertexArrayRange->allocate(agpMem, IDriver::VBHardAGP))
			{
				nlinfo("VAR: %.d vertices supported", _MaxVerticesByVBHard);
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
			nlinfo("VAR: %.d vertices supported", _MaxVerticesByVBHard);
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
			if(_VRAMVertexArrayRange->allocate(vramMem, IDriver::VBHardVRAM))
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
uint32				CDriverGL::getAvailableVertexAGPMemory ()
{
	if (_AGPVertexArrayRange)
		return _AGPVertexArrayRange->sizeAllocated();
	else
		return 0;
}


// ***************************************************************************
uint32				CDriverGL::getAvailableVertexVRAMMemory ()
{
	if (_VRAMVertexArrayRange)
		return _VRAMVertexArrayRange->sizeAllocated();
	else
		return 0;
}


// ***************************************************************************
void				CDriverGL::fenceOnCurVBHardIfNeeded(IVertexBufferHardGL *newVBHard)
{
	// If old is not a VBHard, or if not a NVidia VBHard, no-op.
	if( _CurrentVertexBufferHard==NULL || !_CurrentVertexBufferHard->NVidiaVertexBufferHard )
		return;

	// if we do not activate the same (NB: newVBHard==NULL if not a VBHard).
	if(_CurrentVertexBufferHard!=newVBHard)
	{
		// get NVidia interface
		CVertexBufferHardGLNVidia	*vbHardNV= static_cast<CVertexBufferHardGLNVidia*>(_CurrentVertexBufferHard);

		// If some render() have been done with this VB.
		if( vbHardNV->GPURenderingAfterFence )
		{
			/*
				Since we won't work with this VB for a long time, we set a fence.

				NB: if the fence was previously set. NV_Fence Specification says that the new ONE replaces it.
				This is EXACTLY what we wants, since the old one is no more interesting.

				NB: never insert a fence for said "Static Lock" VBHard. Those VBHard are said to be "static"
				therefore, user should never modify them (else lock() is much slower...)
			*/
			if( !vbHardNV->getLockHintStatic() )
				vbHardNV->setFence();
			// Since we have set a new Fence, we won't need to do it at next vbHardNV->lock()
			vbHardNV->GPURenderingAfterFence= false;
		}
	}
}


} // NL3D
