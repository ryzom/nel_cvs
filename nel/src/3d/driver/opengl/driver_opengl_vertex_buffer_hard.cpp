/** \file driver_opengl_vertex_buffer_hard.cpp
 * <File description>
 *
 * $Id: driver_opengl_vertex_buffer_hard.cpp,v 1.15 2004/04/21 12:04:02 vizerie Exp $
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

#include "driver_opengl.h"
#include "driver_opengl_vertex_buffer_hard.h"

#include "3d/vertex_buffer.h"

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
IVertexBufferHardGL::IVertexBufferHardGL(CDriverGL *drv, CVertexBuffer *vb) : VB (vb)
{
	_Driver= drv;
	GPURenderingAfterFence= false;
	VBType = UnknownVB;
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
bool			CVertexArrayRangeNVidia::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	nlassert(_VertexArrayPtr==NULL);

#ifdef	NL_OS_WINDOWS
	// try to allocate AGP or VRAM data.
	switch(vbType)
	{
	case CVertexBuffer::AGPPreferred: 
		_VertexArrayPtr= wglAllocateMemoryNV(size, 0, 0, 0.5f);
		break;
	case CVertexBuffer::StaticPreferred:
		if (_Driver->getStaticMemoryToVRAM())
			_VertexArrayPtr= wglAllocateMemoryNV(size, 0, 0, 1.0f);
		else
			_VertexArrayPtr= wglAllocateMemoryNV(size, 0, 0, 0.5f);
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
			// Yoyo: NVidia Bug ?? must do this strange thing, this ensure that it is correclty setuped.
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
IVertexBufferHardGL		*CVertexArrayRangeNVidia::createVBHardGL(uint size, CVertexBuffer *vb)
{
	// create a NVidia VBHard
	CVertexBufferHardGLNVidia	*newVbHard= new CVertexBufferHardGLNVidia(_Driver, vb);

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
CVertexBufferHardGLNVidia::CVertexBufferHardGLNVidia(CDriverGL *drv, CVertexBuffer *vb) : IVertexBufferHardGL(drv, vb)
{
	_VertexArrayRange= NULL;
	_VertexPtr= NULL;

	GPURenderingAfterFence= false;
	_FenceSet= false;

	// Flag our type
	VBType = NVidiaVB;	

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

	// Lock Profile?
	TTicks	beforeLock;
	if(_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}

	// Ensure the GPU has finished with the current VBHard.
	finishFence();

	// Lock Profile?
	if(_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}

	return _VertexPtr;
}


// ***************************************************************************
void		CVertexBufferHardGLNVidia::unlock()
{
	// no op.
}

// ***************************************************************************
void		CVertexBufferHardGLNVidia::unlock(uint start, uint end)
{
	// no op.
	// NB: start-end only usefull for ATI ext.
}

// ***************************************************************************
void		*CVertexBufferHardGLNVidia::getPointer()
{
	return _VertexPtr;
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

void CVertexBufferHardGLNVidia::setupVBInfos(CVertexBufferInfo &vb)
{
	vb.VBMode = CVertexBufferInfo::HwNVIDIA;	
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
bool					CVertexArrayRangeATI::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	nlassert(!_Allocated);

	// try to allocate AGP (suppose mean ATI dynamic) or VRAM (suppose mean ATI static) data.
	switch(vbType)
	{
	case CVertexBuffer::AGPPreferred: 
		_VertexObjectId= nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
		break;
	case CVertexBuffer::StaticPreferred:
		if (_Driver->getStaticMemoryToVRAM())
			_VertexObjectId= nglNewObjectBufferATI(size, NULL, GL_STATIC_ATI);
		else
			_VertexObjectId= nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
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
IVertexBufferHardGL		*CVertexArrayRangeATI::createVBHardGL(uint size, CVertexBuffer *vb)
{
	// create a ATI VBHard
	CVertexBufferHardGLATI	*newVbHard= new CVertexBufferHardGLATI(_Driver, vb);

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
CVertexBufferHardGLATI::CVertexBufferHardGLATI(CDriverGL *drv, CVertexBuffer *vb) : IVertexBufferHardGL(drv, vb)
{
	_VertexArrayRange= NULL;
	_VertexPtr= NULL;
	_RAMMirrorVertexPtr= NULL;
	_RAMMirrorVertexSize= 0;

	// Flag our type
	VBType = ATIVB;	
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
void		CVertexBufferHardGLATI::unlock(uint start, uint end)
{
	// clamp end.
	if(end>VB->getNumVertices())
		end=VB->getNumVertices()*VB->getVertexSize();

	// verify bound.
	if(start>=end)
		return;

	// Copy a subset of the mirror into the ATI Vertex Object
	uint	size= end-start;
	uint	srcOffStart= start;
	uint	dstOffStart= ((uint)_VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START) + srcOffStart;
	// copy.
	nglUpdateObjectBufferATI(getATIVertexObjectId(), dstOffStart,
		size, (uint8*)_RAMMirrorVertexPtr + srcOffStart, GL_PRESERVE_ATI);
}


// ***************************************************************************
void		*CVertexBufferHardGLATI::getPointer()
{
	return (uint8 *) _VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START;
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

// ***************************************************************************

void CVertexBufferHardGLATI::setupVBInfos(CVertexBufferInfo &vb)
{
	vb.VBMode = CVertexBufferInfo::HwATI;
	vb.VertexObjectId = getATIVertexObjectId();
}

// ***************************************************************************

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
//
// ATI implementation, version 2 using the map object buffer extension
//
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// CVertexArrayRangeMapObjectATI
// ***************************************************************************


// ***************************************************************************
CVertexArrayRangeMapObjectATI::CVertexArrayRangeMapObjectATI(CDriverGL *drv) : IVertexArrayRange(drv),
																			   _VBType(CVertexBuffer::AGPPreferred),
																			   _SizeAllocated(0)
{
}

// ***************************************************************************
bool CVertexArrayRangeMapObjectATI::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	// We don't manage memory ourselves, but test if there's enough room anyway
	GLuint vertexObjectId;
	switch(vbType)
	{	
		case CVertexBuffer::AGPPreferred: 
			vertexObjectId = nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
			break;
		case CVertexBuffer::StaticPreferred:
			if (_Driver->getStaticMemoryToVRAM())
				vertexObjectId = nglNewObjectBufferATI(size, NULL, GL_STATIC_ATI);
			else
				vertexObjectId = nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
			break;
	}
	if (vertexObjectId)
	{	
		// free the object
		nglDeleteObjectBufferATI(vertexObjectId);
		//
		_SizeAllocated = size;
		_VBType = vbType;
		return true;
	}
	return false;
}

// ***************************************************************************
void CVertexArrayRangeMapObjectATI::free()
{
	_SizeAllocated = 0;
}

// ***************************************************************************
IVertexBufferHardGL *CVertexArrayRangeMapObjectATI::createVBHardGL(uint size, CVertexBuffer *vb)
{
	// create a ATI VBHard
	CVertexBufferHardGLMapObjectATI	*newVbHard= new CVertexBufferHardGLMapObjectATI(_Driver, vb);

	uint vertexObjectId;
	// just allocate a new buffer..
	switch(_VBType)
	{
		case CVertexBuffer::AGPPreferred: 
			vertexObjectId = nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
			break;
		case CVertexBuffer::StaticPreferred:
			if (_Driver->getStaticMemoryToVRAM())
				vertexObjectId = nglNewObjectBufferATI(size, NULL, GL_STATIC_ATI);
			else
				vertexObjectId = nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
			break;
	};


	// init the allocator, if success
	if( nglIsObjectBufferATI(vertexObjectId) )
	{
		newVbHard->initGL(this, vertexObjectId);
		return newVbHard;
	}
	else
	{
		delete newVbHard;
		return false;
	}
}

// ***************************************************************************
void CVertexArrayRangeMapObjectATI::enable()
{	
}

// ***************************************************************************
void CVertexArrayRangeMapObjectATI::disable()
{
}



// ***************************************************************************
// CVertexBufferHardGLMapObjectATI
// ***************************************************************************

// ***************************************************************************
CVertexBufferHardGLMapObjectATI::CVertexBufferHardGLMapObjectATI(CDriverGL *drv, CVertexBuffer *vb) :  IVertexBufferHardGL(drv, vb),
																					_VertexObjectId(0),
																					_VertexPtr(NULL),
																					_VertexArrayRange(NULL)
{	
	// Flag our type
	VBType = ATIMapObjectVB;		
}

// ***************************************************************************
CVertexBufferHardGLMapObjectATI::~CVertexBufferHardGLMapObjectATI()
{
	if (_VertexObjectId) nglDeleteObjectBufferATI(_VertexObjectId);
}

// ***************************************************************************
void *CVertexBufferHardGLMapObjectATI::lock()
{
	if (_VertexPtr) return _VertexPtr; // already locked
	if (!_VertexObjectId) return NULL;
	_VertexPtr = nglMapObjectBufferATI(_VertexObjectId);
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::unlock()
{
	if (!_VertexObjectId || !_VertexPtr) return;
	nglUnmapObjectBufferATI(_VertexObjectId);
	_VertexPtr = NULL;
}

// ***************************************************************************
void		*CVertexBufferHardGLMapObjectATI::getPointer()
{
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::unlock(uint start,uint endVert)
{
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::enable()
{
	if(_Driver->_CurrentVertexBufferHard != this)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->enable(); */
		_Driver->_CurrentVertexBufferHard= this;
	}
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::disable()
{
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->disable(); */
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::initGL(CVertexArrayRangeMapObjectATI *var, uint vertexObjectID)
{
	_VertexArrayRange = var;
	_VertexObjectId = vertexObjectID;
}

// ***************************************************************************
void			CVertexBufferHardGLMapObjectATI::lockHintStatic(bool staticLock)
{
	// no op.
}

// ***************************************************************************

void CVertexBufferHardGLMapObjectATI::setupVBInfos(CVertexBufferInfo &vb)
{
	vb.VBMode = CVertexBufferInfo::HwATI;
	vb.VertexObjectId = getATIVertexObjectId();
}

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ARB implementation
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// CVertexArrayRangeARB
// ***************************************************************************


// ***************************************************************************
CVertexArrayRangeARB::CVertexArrayRangeARB(CDriverGL *drv) : IVertexArrayRange(drv),
															 _VBType(CVertexBuffer::AGPPreferred),
															 _SizeAllocated(0)
{
}

// ***************************************************************************
bool CVertexArrayRangeARB::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	nlassert(_SizeAllocated == 0);
	/*
	// We don't manage memory ourselves, but test if there's enough room anyway
	GLuint vertexBufferID;
	glGetError();
	nglGenBuffersARB(1, &vertexBufferID);
	if (glGetError() != GL_NO_ERROR) return false;
		
	switch(vbType)
	{	
		case CVertexBuffer::AGPPreferred: 
			nglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_DYNAMIC_DRAW_ARB);
		break;
		case CVertexBuffer::VRAMPreferred:
			nglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_STATIC_DRAW_ARB);
		break;
		default:
			nlassert(0);
		break;
	}

	if (glGetError() != GL_NO_ERROR)
	{
		nglDeleteBuffersARB(1, &vertexBufferID);
		return false;
	}
	nglDeleteBuffersARB(1, &vertexBufferID);			
	*/
	_SizeAllocated = size;
	_VBType = vbType;	
	return true;	
}

// ***************************************************************************
void CVertexArrayRangeARB::free()
{
	_SizeAllocated = 0;
}

// ***************************************************************************
IVertexBufferHardGL *CVertexArrayRangeARB::createVBHardGL(uint size, CVertexBuffer *vb)
{
	if (!_SizeAllocated) return NULL;
	// create a ARB VBHard
	GLuint vertexBufferID;
	glGetError();
	nglGenBuffersARB(1, &vertexBufferID);
	if (glGetError() != GL_NO_ERROR) return false;		
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);
	switch(_VBType)
	{	
		case CVertexBuffer::AGPPreferred: 
			nglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_DYNAMIC_DRAW_ARB);			
			break;
		case CVertexBuffer::StaticPreferred:
			if (_Driver->getStaticMemoryToVRAM())
				nglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_STATIC_DRAW_ARB);
			else
				nglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_DYNAMIC_DRAW_ARB);				
			break;
		default:
			nlassert(0);
			break;
	}
	if (glGetError() != GL_NO_ERROR)
	{
		nglDeleteBuffersARB(1, &vertexBufferID);
		return false;
	}
	CVertexBufferHardARB *newVbHard= new CVertexBufferHardARB(_Driver, vb);
	newVbHard->initGL(vertexBufferID, _VBType);
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	return newVbHard;
}

// ***************************************************************************
void CVertexArrayRangeARB::enable()
{
	if(_Driver->_CurrentVertexArrayRange != this)
	{				
		_Driver->_CurrentVertexArrayRange= this;
	}
}

// ***************************************************************************
void CVertexArrayRangeARB::disable()
{
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{		
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
// CVertexBufferHardARB
// ***************************************************************************


// ***************************************************************************
CVertexBufferHardARB::CVertexBufferHardARB(CDriverGL *drv, CVertexBuffer *vb) :  IVertexBufferHardGL(drv, vb),
																				_VertexObjectId(0),
																				_VertexPtr(NULL)
																					
{	
	// Flag our type
	VBType = ARBVB;		
}

// ***************************************************************************
CVertexBufferHardARB::~CVertexBufferHardARB()
{
	if (_Driver && _VertexObjectId)
	{
		if (_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == _VertexObjectId)
		{
			_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
		}		
	}
	if (_VertexObjectId) nglDeleteBuffersARB(1, &_VertexObjectId);
	
		
}

// ***************************************************************************
void *CVertexBufferHardARB::lock()
{
	if (_VertexPtr) return _VertexPtr; // already locked
	if (!_VertexObjectId) return NULL;
	TTicks	beforeLock;
	if(_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	_Driver->_DriverGLStates.bindARBVertexBuffer(_VertexObjectId);
	//nlassert(VB->isResident());
	//_VertexPtr = nglMapBufferARB(GL_ARRAY_BUFFER_ARB, VB->isResident() ?	GL_WRITE_ONLY_ARB : GL_READ_WRITE_ARB);		
	_VertexPtr = nglMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	

	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	
		
	// Lock Profile?
	if(_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}		
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHardARB::unlock()
{
	if (!_VertexObjectId || !_VertexPtr) return;
	_Driver->_DriverGLStates.bindARBVertexBuffer(_VertexObjectId);	
	// double start = CTime::ticksToSecond(CTime::getPerformanceTime());
	nglUnmapBufferARB(GL_ARRAY_BUFFER_ARB);	

	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	
		
	/* double end = CTime::ticksToSecond(CTime::getPerformanceTime());
	nlinfo("Unlock = %f ms", (float) ((end - start) * 1000)); */
	_VertexPtr = NULL;	
}

// ***************************************************************************
void		*CVertexBufferHardARB::getPointer()
{
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHardARB::unlock(uint startVert,uint endVert)
{
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************
void CVertexBufferHardARB::enable()
{
	if(_Driver->_CurrentVertexBufferHard != this)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->enable(); */
		_Driver->_CurrentVertexBufferHard= this;
	}
}

// ***************************************************************************
void CVertexBufferHardARB::disable()
{
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->disable(); */
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void CVertexBufferHardARB::initGL(uint vertexObjectID, CVertexBuffer::TPreferredMemory memType)
{	
	_VertexObjectId = vertexObjectID;
	_MemType = memType;
}

// ***************************************************************************
void			CVertexBufferHardARB::lockHintStatic(bool staticLock)
{
	// no op.
}

// ***************************************************************************
void CVertexBufferHardARB::setupVBInfos(CVertexBufferInfo &vb)
{
	vb.VBMode = CVertexBufferInfo::HwARB;
	vb.VertexObjectId = _VertexObjectId;
}


}

