/** \file driver_opengl_vertex_buffer_hard.cpp
 * <File description>
 *
 * $Id: driver_opengl_vertex_buffer_hard.cpp,v 1.8 2003/03/27 17:37:31 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "driver_opengl_vertex_buffer_hard.h"


using	namespace std;
using	namespace NLMISC;

namespace NL3D 
{



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
//
// VBHard interface for both NVidia / ATI extension.
//
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
IVertexArrayRange::IVertexArrayRange(CDriverGL *drv)
{
	_Driver= drv;
}
// ***************************************************************************
IVertexArrayRange::~IVertexArrayRange()
{
}

// ***************************************************************************
IVertexBufferHardGL::IVertexBufferHardGL(CDriverGL *drv)
{
	_Driver= drv;
	NVidiaVertexBufferHard= false;
	ATIVertexBufferHard= false;
	GPURenderingAfterFence= false;
}
// ***************************************************************************
IVertexBufferHardGL::~IVertexBufferHardGL()
{
}



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
//
// NVidia implementation
//
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
// CVertexArrayRangeNVidia
// ***************************************************************************


// ***************************************************************************
CVertexArrayRangeNVidia::CVertexArrayRangeNVidia(CDriverGL *drv) : IVertexArrayRange(drv)
{
	_VertexArrayPtr= NULL;
	_VertexArraySize= 0;
}


// ***************************************************************************
bool			CVertexArrayRangeNVidia::allocate(uint32 size, IDriver::TVBHardType vbType)
{
	nlassert(_VertexArrayPtr==NULL);

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

		// set the size
		_VertexArraySize= size;
	}


	return _VertexArrayPtr!=NULL;
}


// ***************************************************************************
uint			CVertexArrayRangeNVidia::sizeAllocated() const
{
	return _VertexArraySize;
}


// ***************************************************************************
void			CVertexArrayRangeNVidia::free()
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
		_VertexArraySize= 0;
	}
}


// ***************************************************************************
void			CVertexArrayRangeNVidia::enable()
{
	// if not already enabled.
	if(_Driver->_CurrentVertexArrayRange!=this)
	{
		// Setup the ptrs only if differnets from last time (may rarely happens)
		if( _Driver->_NVCurrentVARSize != _VertexArraySize || _Driver->_NVCurrentVARPtr != _VertexArrayPtr)
		{
			// Yoyo: fucking NVidia Bug ?? must do this strange thing, this ensure that it is correclty setuped.
			glFinish();
			nglVertexArrayRangeNV(_VertexArraySize, _VertexArrayPtr);
			glEnableClientState(GL_VERTEX_ARRAY_RANGE_NV);
			glVertexPointer(3,GL_FLOAT, 0, _VertexArrayPtr);
			// cache.
			_Driver->_NVCurrentVARSize= _VertexArraySize;
			_Driver->_NVCurrentVARPtr= _VertexArrayPtr;
		}
		// enable VAR. NB: flush is unesufull, so don't flush if extension is OK
		glEnableClientState(_Driver->_Extensions.NVStateVARWithoutFlush);
		_Driver->_CurrentVertexArrayRange= this;
	}
}


// ***************************************************************************
void			CVertexArrayRangeNVidia::disable()
{
	// if not already disabled.
	if(_Driver->_CurrentVertexArrayRange!=NULL)
	{
		// just disable the state, don't change VAR ptr setup.
		// NB: flush is unesufull, so don't flush if extension is OK
		glDisableClientState(_Driver->_Extensions.NVStateVARWithoutFlush);
		_Driver->_CurrentVertexArrayRange= NULL;
	}
}


// ***************************************************************************
void			*CVertexArrayRangeNVidia::allocateVB(uint32 size)
{
	return _HeapMemory.allocate(size);
}


// ***************************************************************************
void			CVertexArrayRangeNVidia::freeVB(void	*ptr)
{
	_HeapMemory.free(ptr);
}


// ***************************************************************************
IVertexBufferHardGL		*CVertexArrayRangeNVidia::createVBHardGL(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, const uint8 *uvRouting)
{
	// create a NVidia VBHard
	CVertexBufferHardGLNVidia	*newVbHard= new CVertexBufferHardGLNVidia(_Driver);

	// Init the format of the VB.
	newVbHard->initFormat(vertexFormat, typeArray, numVertices, uvRouting);

	// compute size to allocate.
	uint32	size= newVbHard->getVertexSize() * newVbHard->getNumVertices();

	// try to allocate
	void	*vertexPtr;
	if( allocated() )
	{
		 vertexPtr= allocateVB(size);
	}

	// If allocation fails.
	if( !vertexPtr )
	{
		// just destroy this object (no free()).
		delete newVbHard;
		return NULL;
	}
	else
	{
		// Ok, setup, and allocate the fence.
		newVbHard->initGL(this, vertexPtr);
		return newVbHard;
	}
}


// ***************************************************************************
// CVertexBufferHardGLNVidia
// ***************************************************************************


// ***************************************************************************
CVertexBufferHardGLNVidia::CVertexBufferHardGLNVidia(CDriverGL *drv) : IVertexBufferHardGL(drv)
{
	_VertexArrayRange= NULL;
	_VertexPtr= NULL;

	GPURenderingAfterFence= false;
	_FenceSet= false;

	// Flag our type
	NVidiaVertexBufferHard= true;

	// default: dynamic Loick
	_LockHintStatic= false;
}


// ***************************************************************************
CVertexBufferHardGLNVidia::~CVertexBufferHardGLNVidia()
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
void		CVertexBufferHardGLNVidia::initGL(CVertexArrayRangeNVidia *var, void *vertexPtr)
{
	_VertexArrayRange= var;
	_VertexPtr= vertexPtr;
	nglGenFencesNV(1, &_Fence);
}


// ***************************************************************************
void		*CVertexBufferHardGLNVidia::lock()
{
	// sync the 3d card with the system.

	// If the user lock an activated VBHard, after rendering some primitives, we must stall the CPU
	// Also always do this for Static Lock VBHard (because no setFence are inserted after rendering
	if(GPURenderingAfterFence || _LockHintStatic)
	{
		// Set a new fence at the current position in the command stream, replacing the old one
		setFence();
		// And so the GPU has rendered all our primitives.
		GPURenderingAfterFence= false;
	}

	// Ensure the GPU has finished with the current VBHard.
	finishFence();


	return _VertexPtr;
}


// ***************************************************************************
void		CVertexBufferHardGLNVidia::unlock()
{
	// no op.
}

// ***************************************************************************
void		CVertexBufferHardGLNVidia::unlock(uint startVert, uint endVert)
{
	// no op.
	// NB: start-end only usefull for ATI ext.
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::enable()
{
	if(_Driver->_CurrentVertexBufferHard != this)
	{
		nlassert(_VertexArrayRange);
		_VertexArrayRange->enable();
		_Driver->_CurrentVertexBufferHard= this;
	}
}


// ***************************************************************************
void			CVertexBufferHardGLNVidia::disable()
{
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{
		nlassert(_VertexArrayRange);
		_VertexArrayRange->disable();
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::lockHintStatic(bool staticLock)
{
	// Store the flag.
	_LockHintStatic= staticLock;
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::setFence()
{
	// NB: if the Fence was set and not finished, then the new one will KICK the old.
	nglSetFenceNV(_Fence, GL_ALL_COMPLETED_NV);
	_FenceSet= true;
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::finishFence()
{
	if(isFenceSet())
	{
		// Stall CPU while the fence command is not reached in the GPU command stream.
		nglFinishFenceNV(_Fence);
		_FenceSet= false;
	}
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::testFence()
{
	if(isFenceSet())
	{
		// Don't stall the CPU
		GLboolean	b= nglTestFenceNV(_Fence);
		// if completed
		if(b==GL_TRUE)
			_FenceSet= false;
	}
}



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
//
// ATI implementation
//
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// CVertexArrayRangeATI
// ***************************************************************************


// ***************************************************************************
CVertexArrayRangeATI::CVertexArrayRangeATI(CDriverGL *drv) : IVertexArrayRange(drv)
{
	_Allocated= false;
	_VertexObjectId= 0;
	_VertexArraySize= 0;
}
// ***************************************************************************
bool					CVertexArrayRangeATI::allocate(uint32 size, IDriver::TVBHardType vbType)
{
	nlassert(!_Allocated);

	// try to allocate AGP (suppose mean ATI dynamic) or VRAM (suppose mean ATI static) data.
	switch(vbType)
	{
	case IDriver::VBHardAGP: 
		_VertexObjectId= nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
		break;
	case IDriver::VBHardVRAM:
		_VertexObjectId= nglNewObjectBufferATI(size, NULL, GL_STATIC_ATI);
		break;
	};


	// init the allocator, if success
	if( nglIsObjectBufferATI(_VertexObjectId) )
	{
		_Allocated= true;
		/* Init with an alignment of 8. Not sure if it is usefull.
			Init With a FAKE memory starting at NL3D_DRV_ATI_FAKE_MEM_START
		*/
		_HeapMemory.initHeap((void*)NL3D_DRV_ATI_FAKE_MEM_START, size, 8);

		// set the size
		_VertexArraySize= size;

		// Ok!
		return true;
	}
	else
	{
		// failure.
		return false;
	}
}
// ***************************************************************************
uint					CVertexArrayRangeATI::sizeAllocated() const
{
	return _VertexArraySize;
}
// ***************************************************************************
void					CVertexArrayRangeATI::free()
{
	// release the ptr.
	if(_Allocated)
	{
		// reset the allocator.
		_HeapMemory.reset();

		// Free special memory.
		nglDeleteObjectBufferATI(_VertexObjectId);

		_Allocated= false;
		_VertexArraySize= 0;
	}
}
// ***************************************************************************
IVertexBufferHardGL		*CVertexArrayRangeATI::createVBHardGL(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, const uint8 *uvRouting)
{
	// create a ATI VBHard
	CVertexBufferHardGLATI	*newVbHard= new CVertexBufferHardGLATI(_Driver);

	// Init the format of the VB.
	newVbHard->initFormat(vertexFormat, typeArray, numVertices, uvRouting);

	// compute size to allocate.
	uint32	size= newVbHard->getVertexSize() * newVbHard->getNumVertices();

	// try to allocate
	void	*vertexPtr =NULL;
	if( _Allocated )
	{
		 vertexPtr= allocateVB(size);
	}

	// If allocation fails.
	if( !vertexPtr )
	{
		// just destroy this object (no free()).
		delete newVbHard;
		return NULL;
	}
	else
	{
		// Then try to create a RAM mirror in the VB.
		if( newVbHard->createRAMMirror(size) )
		{
			// Ok, setup
			newVbHard->initGL(this, vertexPtr);
			return newVbHard;
		}
		else
		{
			// Discard the VB.
			delete newVbHard;
			return NULL;
		}
	}
}
// ***************************************************************************
void			CVertexArrayRangeATI::enable()
{
	// No-op for ATI !!!
	_Driver->_CurrentVertexArrayRange= this;
}
// ***************************************************************************
void			CVertexArrayRangeATI::disable()
{
	// No-op for ATI !!!
	_Driver->_CurrentVertexArrayRange= NULL;
}
// ***************************************************************************
void			*CVertexArrayRangeATI::allocateVB(uint32 size)
{
	return _HeapMemory.allocate(size);
}
// ***************************************************************************
void			CVertexArrayRangeATI::freeVB(void	*ptr)
{
	_HeapMemory.free(ptr);
}


// ***************************************************************************
// CVertexBufferHardGLATI
// ***************************************************************************


// ***************************************************************************
CVertexBufferHardGLATI::CVertexBufferHardGLATI(CDriverGL *drv) : IVertexBufferHardGL(drv)
{
	_VertexArrayRange= NULL;
	_VertexPtr= NULL;
	_RAMMirrorVertexPtr= NULL;
	_RAMMirrorVertexSize= 0;

	// Flag our type
	ATIVertexBufferHard= true;
}


// ***************************************************************************
CVertexBufferHardGLATI::~CVertexBufferHardGLATI()
{
	if(_VertexArrayRange)
	{
		// free the VAR.
		_VertexArrayRange->freeVB(_VertexPtr);
		_VertexPtr= NULL;
		_VertexArrayRange= NULL;
	}

	if(_RAMMirrorVertexPtr)
	{
		delete [] ((uint8*)_RAMMirrorVertexPtr);
		_RAMMirrorVertexPtr= NULL;
		_RAMMirrorVertexSize= 0;
	}
}


// ***************************************************************************
bool		CVertexBufferHardGLATI::createRAMMirror(uint memSize)
{
	// delete old if any
	if(_RAMMirrorVertexPtr)
	{
		free(_RAMMirrorVertexPtr);
		_RAMMirrorVertexPtr= NULL;
		_RAMMirrorVertexSize= 0;
	}
	// create
	_RAMMirrorVertexPtr = (void*)new uint8[memSize];
	if(_RAMMirrorVertexPtr)
	{
		_RAMMirrorVertexSize= memSize;
		return true;
	}
	else
		return false;
}


// ***************************************************************************
void		CVertexBufferHardGLATI::initGL(CVertexArrayRangeATI *var, void *vertexPtr)
{
	_VertexArrayRange= var;
	_VertexPtr= vertexPtr;
}


// ***************************************************************************
void		*CVertexBufferHardGLATI::lock()
{
	// return directly the mirror one. => client will write into RAM.
	return _RAMMirrorVertexPtr;
}


// ***************************************************************************
void		CVertexBufferHardGLATI::unlock()
{
	// Copy All mirror into the ATI Vertex Object
	nglUpdateObjectBufferATI(getATIVertexObjectId(), (uint)_VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START,
		_RAMMirrorVertexSize, _RAMMirrorVertexPtr, GL_PRESERVE_ATI);
}


// ***************************************************************************
void		CVertexBufferHardGLATI::unlock(uint startVert, uint endVert)
{
	// clamp endVert.
	if(endVert>getNumVertices())
		endVert=getNumVertices();

	// verify bound.
	if(startVert>=endVert)
		return;

	// Copy a subset of the mirror into the ATI Vertex Object
	uint	size= (endVert-startVert)*getVertexSize();
	uint	srcOffStart= startVert*getVertexSize();
	uint	dstOffStart= ((uint)_VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START) + srcOffStart;
	// copy.
	nglUpdateObjectBufferATI(getATIVertexObjectId(), dstOffStart,
		size, (uint8*)_RAMMirrorVertexPtr + srcOffStart, GL_PRESERVE_ATI);
}


// ***************************************************************************
void			CVertexBufferHardGLATI::enable()
{
	if(_Driver->_CurrentVertexBufferHard != this)
	{
		nlassert(_VertexArrayRange);
		_VertexArrayRange->enable();
		_Driver->_CurrentVertexBufferHard= this;
	}
}


// ***************************************************************************
void			CVertexBufferHardGLATI::disable()
{
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{
		nlassert(_VertexArrayRange);
		_VertexArrayRange->disable();
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void			CVertexBufferHardGLATI::lockHintStatic(bool staticLock)
{
	// no op.
}


}

