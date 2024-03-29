/** \file driver_opengl_vertex_buffer_hard.cpp
 * TODO: File description
 *
 * $Id: driver_opengl_vertex_buffer_hard.cpp,v 1.22 2007/03/09 09:49:30 boucher Exp $
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

#include "../../vertex_buffer.h"

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
	H_AUTO_OGL(IVertexArrayRange_IVertexArrayRange)
	_Driver= drv;
}
// ***************************************************************************
IVertexArrayRange::~IVertexArrayRange()
{
	H_AUTO_OGL(IVertexArrayRange_IVertexArrayRangeDtor)
}

// ***************************************************************************
IVertexBufferHardGL::IVertexBufferHardGL(CDriverGL *drv, CVertexBuffer *vb) : VB (vb)
{
	H_AUTO_OGL(IVertexBufferHardGL_IVertexBufferHardGL)
	_Driver= drv;
	GPURenderingAfterFence= false;
	VBType = UnknownVB;
	_Invalid = false;	
}
// ***************************************************************************
IVertexBufferHardGL::~IVertexBufferHardGL()
{
	H_AUTO_OGL(IVertexBufferHardGL_IVertexBufferHardGLDtor)
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
	H_AUTO_OGL(CVertexArrayRangeNVidia_CVertexArrayRangeNVidia)
	_VertexArrayPtr= NULL;
	_VertexArraySize= 0;
}


// ***************************************************************************
bool			CVertexArrayRangeNVidia::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	H_AUTO_OGL(CVertexArrayRangeNVidia_allocate)
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
	H_AUTO_OGL(CVertexArrayRangeNVidia_sizeAllocated)
	return _VertexArraySize;
}


// ***************************************************************************
void			CVertexArrayRangeNVidia::free()
{
	H_AUTO_OGL(CVertexArrayRangeNVidia_free)
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
	H_AUTO_OGL(CVertexArrayRangeNVidia_enable)
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
	H_AUTO_OGL(CVertexArrayRangeNVidia_disable)
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
	H_AUTO_OGL(CVertexArrayRangeNVidia_allocateVB)
	return _HeapMemory.allocate(size);
}


// ***************************************************************************
void			CVertexArrayRangeNVidia::freeVB(void	*ptr)
{
	H_AUTO_OGL(CVertexArrayRangeNVidia_freeVB)
	_HeapMemory.free(ptr);
}


// ***************************************************************************
IVertexBufferHardGL		*CVertexArrayRangeNVidia::createVBHardGL(uint size, CVertexBuffer *vb)
{
	H_AUTO_OGL(CVertexArrayRangeNVidia_createVBHardGL)
	// create a NVidia VBHard
	CVertexBufferHardGLNVidia	*newVbHard= new CVertexBufferHardGLNVidia(_Driver, vb);

	// try to allocate
	void	*vertexPtr = 0;
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
	H_AUTO_OGL(CVertexBufferHardGLNVidia_CVertexBufferHardGLNVidia)
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
	H_AUTO_OGL(CVertexBufferHardGLNVidia_CVertexBufferHardGLNVidiaDtor)
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
	H_AUTO_OGL(CVertexBufferHardGLNVidia_initGL)
	_VertexArrayRange= var;
	_VertexPtr= vertexPtr;
	nglGenFencesNV(1, &_Fence);
}


// ***************************************************************************
void		*CVertexBufferHardGLNVidia::lock()
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_lock)
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
	H_AUTO_OGL(CVertexBufferHardGLNVidia_unlock)
	// no op.
}

// ***************************************************************************
void		CVertexBufferHardGLNVidia::unlock(uint start, uint end)
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_unlock_start_end)
	// no op.
	// NB: start-end only usefull for ATI ext.
}

// ***************************************************************************
void		*CVertexBufferHardGLNVidia::getPointer()
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_getPointer)
	return _VertexPtr;
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::enable()
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_enable)
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
	H_AUTO_OGL(CVertexBufferHardGLNVidia_disbale)
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
	H_AUTO_OGL(CVertexBufferHardGLNVidia_lockHintStatic)
	// Store the flag.
	_LockHintStatic= staticLock;
}

// ***************************************************************************

void CVertexBufferHardGLNVidia::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_setupVBInfos)
	vb.VBMode = CVertexBufferInfo::HwNVIDIA;	
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::setFence()
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_setFence)
	// NB: if the Fence was set and not finished, then the new one will KICK the old.
	nglSetFenceNV(_Fence, GL_ALL_COMPLETED_NV);
	_FenceSet= true;
}

// ***************************************************************************
void			CVertexBufferHardGLNVidia::finishFence()
{
	H_AUTO_OGL(CVertexBufferHardGLNVidia_finishFence)
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

	H_AUTO_OGL(CVertexBufferHardGLNVidia_testFence)
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
	H_AUTO_OGL(CVertexArrayRangeATI_CVertexArrayRangeATI)
	_Allocated= false;
	_VertexObjectId= 0;
	_VertexArraySize= 0;
}
// ***************************************************************************
bool					CVertexArrayRangeATI::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	H_AUTO_OGL(CVertexArrayRangeATI_allocate)
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
    default:
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
	H_AUTO_OGL(CVertexArrayRangeATI_sizeAllocated)
	return _VertexArraySize;
}
// ***************************************************************************
void					CVertexArrayRangeATI::free()
{
	H_AUTO_OGL(CVertexArrayRangeATI_free)
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
	H_AUTO_OGL(CVertexArrayRangeATI_createVBHardGL)
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
	H_AUTO_OGL(CVertexArrayRangeATI_enable)
	// No-op for ATI !!!
	_Driver->_CurrentVertexArrayRange= this;
}
// ***************************************************************************
void			CVertexArrayRangeATI::disable()
{
	H_AUTO_OGL(CVertexArrayRangeATI_disable)
	// No-op for ATI !!!
	_Driver->_CurrentVertexArrayRange= NULL;
}
// ***************************************************************************
void			*CVertexArrayRangeATI::allocateVB(uint32 size)
{
	H_AUTO_OGL(CVertexArrayRangeATI_allocateVB)
	return _HeapMemory.allocate(size);
}
// ***************************************************************************
void			CVertexArrayRangeATI::freeVB(void	*ptr)
{
	H_AUTO_OGL(CVertexArrayRangeATI_freeVB)
	_HeapMemory.free(ptr);
}


// ***************************************************************************
// CVertexBufferHardGLATI
// ***************************************************************************


// ***************************************************************************
CVertexBufferHardGLATI::CVertexBufferHardGLATI(CDriverGL *drv, CVertexBuffer *vb) : IVertexBufferHardGL(drv, vb)
{
	H_AUTO_OGL(CVertexBufferHardGLATI_CVertexBufferHardGLATI)
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
	H_AUTO_OGL(CVertexBufferHardGLATI_CVertexBufferHardGLATIDtor)
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
	H_AUTO_OGL(CVertexBufferHardGLATI_createRAMMirror)
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
	H_AUTO_OGL(CVertexBufferHardGLATI_initGL)
	_VertexArrayRange= var;
	_VertexPtr= vertexPtr;
}


// ***************************************************************************
void		*CVertexBufferHardGLATI::lock()
{
	H_AUTO_OGL(CVertexBufferHardGLATI_lock)
	// return directly the mirror one. => client will write into RAM.
	return _RAMMirrorVertexPtr;
}


// ***************************************************************************
void		CVertexBufferHardGLATI::unlock()
{
	H_AUTO_OGL(CVertexBufferHardGLATI_unlock)
	// Copy All mirror into the ATI Vertex Object
	nglUpdateObjectBufferATI(getATIVertexObjectId(), (ptrdiff_t)_VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START,
		_RAMMirrorVertexSize, _RAMMirrorVertexPtr, GL_PRESERVE_ATI);
}


// ***************************************************************************
void		CVertexBufferHardGLATI::unlock(uint start, uint end)
{
	H_AUTO_OGL(CVertexBufferHardGLATI_unlosk_start_end)
	unlock();
	// clamp end.	
	if(end>VB->getNumVertices()*VB->getVertexSize())
		end=VB->getNumVertices()*VB->getVertexSize();

	// verify bound.
	//if(start>=end)
	//	return;
	if (start == 0 && end == 0)
	{
		end = VB->getVertexSize() * VB->getNumVertices();
	}

	// Copy a subset of the mirror into the ATI Vertex Object
	uint	size= end-start;
	uint	srcOffStart= start;
	uint	dstOffStart= ((ptrdiff_t)_VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START) + srcOffStart;
	// copy.
	nglUpdateObjectBufferATI(getATIVertexObjectId(), dstOffStart,
		size, (uint8*)_RAMMirrorVertexPtr + srcOffStart, GL_PRESERVE_ATI);		
}


// ***************************************************************************
void		*CVertexBufferHardGLATI::getPointer()
{
	H_AUTO_OGL(CVertexBufferHardGLATI_getPointer)
	return (uint8 *) _VertexPtr - NL3D_DRV_ATI_FAKE_MEM_START;
}

// ***************************************************************************
void			CVertexBufferHardGLATI::enable()
{
	H_AUTO_OGL(CVertexBufferHardGLATI_enable)
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
	H_AUTO_OGL(CVertexBufferHardGLATI_disbale)
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
	H_AUTO_OGL(CVertexBufferHardGLATI_lockHintStatic)
	// no op.
}

// ***************************************************************************

void CVertexBufferHardGLATI::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferHardGLATI_setupVBInfos)
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
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_CVertexArrayRangeMapObjectATI)
}

// ***************************************************************************
bool CVertexArrayRangeMapObjectATI::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_allocate)
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
        default:
            vertexObjectId = 0;
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
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_free)
	_SizeAllocated = 0;
}

// ***************************************************************************
IVertexBufferHardGL *CVertexArrayRangeMapObjectATI::createVBHardGL(uint size, CVertexBuffer *vb)
{
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_createVBHardGL)
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
        default:
            vertexObjectId = 0;
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
		return NULL;
	}	
}

// ***************************************************************************
void CVertexArrayRangeMapObjectATI::enable()
{	
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_enable)
}

// ***************************************************************************
void CVertexArrayRangeMapObjectATI::disable()
{
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_disbale)
}



// ***************************************************************************
// CVertexBufferHardGLMapObjectATI
// ***************************************************************************

// ***************************************************************************
CVertexBufferHardGLMapObjectATI::CVertexBufferHardGLMapObjectATI(CDriverGL *drv, CVertexBuffer *vb) :  IVertexBufferHardGL(drv, vb),
																					_VertexPtr(NULL),
																					_VertexArrayRange(NULL),
																					_VertexObjectId(0)
{	
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_CVertexBufferHardGLMapObjectATI)
	// Flag our type
	VBType = ATIMapObjectVB;		
}

// ***************************************************************************
CVertexBufferHardGLMapObjectATI::~CVertexBufferHardGLMapObjectATI()
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_CVertexBufferHardGLMapObjectATIDtor)
	if (_VertexObjectId) nglDeleteObjectBufferATI(_VertexObjectId);
	#ifdef NL_DEBUG
		if (_VertexPtr)
		{
			_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
		}
	#endif
	if (_VertexArrayRange)
	{		
		if (_Invalid)
		{
			if (VB->getLocation() != CVertexBuffer::NotResident)
			{
				// when the vb is put in tthe NotResident state, it is removed from that list
				_VertexArrayRange->_LostVBList.erase(_IteratorInLostVBList);
			}
		}
	}
}

// ***************************************************************************
void *CVertexBufferHardGLMapObjectATI::lock()
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_lock)
	if (_VertexPtr) return _VertexPtr; // already locked
	if (_Invalid)
	{				
		if (VB->getLocation() != CVertexBuffer::NotResident)
		{			
			nlassert(!_DummyVB.empty());
			return &_DummyVB[0];
		}		
		// recreate a vb		
		const uint size = VB->getNumVertices() * VB->getVertexSize();		
		nlassert(!_VertexObjectId);
		switch(_VertexArrayRange->getVBType())
		{
			case CVertexBuffer::AGPPreferred: 
				_VertexObjectId = nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
				break;
			case CVertexBuffer::StaticPreferred:
				if (_Driver->getStaticMemoryToVRAM())
					_VertexObjectId = nglNewObjectBufferATI(size, NULL, GL_STATIC_ATI);
				else
					_VertexObjectId = nglNewObjectBufferATI(size, NULL, GL_DYNAMIC_ATI);
				break;
            default:
                break;
		};				
		if (!_VertexObjectId)
		{			
			_Driver->incrementResetCounter();
			nlassert(!_DummyVB.empty());
			return &_DummyVB[0];
		}		
		NLMISC::contReset(_DummyVB); // free vector memory for real		
		nlassert(_VertexObjectId);
		_Invalid = false;
		_VertexArrayRange->_LostVBList.erase(_IteratorInLostVBList);
		// continue to standard mapping code below ..		
	}
	if (!_VertexObjectId) return NULL;
	_VertexPtr = nglMapObjectBufferATI(_VertexObjectId);		
	if (!_VertexPtr)
	{			
		nglUnmapObjectBufferATI(_VertexObjectId);
		nlassert(nglIsObjectBufferATI(_VertexObjectId));
		invalidate();				
		return &_DummyVB[0];		
	}	
	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.push_front(this);
		_IteratorInMappedVBList = _VertexArrayRange->_MappedVBList.begin();
	#endif
	return _VertexPtr;


}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::invalidate()
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_invalidate)
	nlassert(!_Invalid);	
	// Buffer is lost (maybe there was a alt-tab or fullscrren / windowed change)
	// Buffer is deleted at end of frame only
	_Invalid = true;		
	_Driver->incrementResetCounter();	
	_DummyVB.resize(VB->getNumVertices() * VB->getVertexSize(), 0);	
	// insert in lost vb list	
	_VertexArrayRange->_LostVBList.push_front(this);
	_IteratorInLostVBList = _VertexArrayRange->_LostVBList.begin();	
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::unlock()
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_unlock)
	#ifdef NL_DEBUG
		if (_VertexPtr)
		{
			_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
		}
	#endif
	_VertexPtr = NULL;
	if (_Invalid) return;
	if (!_VertexObjectId) return;
	#ifdef NL_DEBUG
		_Unmapping = true;
	#endif
	nglUnmapObjectBufferATI(_VertexObjectId);
	#ifdef NL_DEBUG
		_Unmapping = false;
	#endif
}

// ***************************************************************************
void		*CVertexBufferHardGLMapObjectATI::getPointer()
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_getPointer)
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::unlock(uint start,uint endVert)
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_unlock)
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::enable()
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_enable)
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
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_disable)
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
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_initGL)
	_VertexArrayRange = var;
	_VertexObjectId = vertexObjectID;
}

// ***************************************************************************
void			CVertexBufferHardGLMapObjectATI::lockHintStatic(bool staticLock)
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_lockHintStatix)
	// no op.
}

// ***************************************************************************
void CVertexBufferHardGLMapObjectATI::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferHardGLMapObjectATI_setupVBInfos)
	vb.VBMode = CVertexBufferInfo::HwATI;
	vb.VertexObjectId = getATIVertexObjectId();
}

// ***************************************************************************
void CVertexArrayRangeMapObjectATI::updateLostBuffers()
{
	H_AUTO_OGL(CVertexArrayRangeMapObjectATI_updateLostBuffers)
	// Put all vb that have been lost in the NotResident state so that they will be recomputed
	// We do this only if the app is active, because if vb were lost, it is likely that there are no resources available.
	nlassert(_Driver);
	if (_Driver->isWndActive())
	{
		for(std::list<CVertexBufferHardGLMapObjectATI *>::iterator it = _LostVBList.begin(); it != _LostVBList.end(); ++it)
		{
			nlassert((*it)->_VertexObjectId);
			nlassert(nglIsObjectBufferATI((*it)->_VertexObjectId));			
			nglDeleteObjectBufferATI((*it)->_VertexObjectId);			
			(*it)->_VertexObjectId = 0;
			(*it)->VB->setLocation(CVertexBuffer::NotResident);
		}
		_LostVBList.clear();
	}	
}
// ***************************************************************************
#ifdef NL_DEBUG
	void CVertexArrayRangeMapObjectATI::dumpMappedBuffers()
	{
		nlwarning("*****************************************************");
		nlwarning("Mapped buffers :");
		for(std::list<CVertexBufferHardGLMapObjectATI *>::iterator it = _MappedVBList.begin(); it != _MappedVBList.end(); ++it)
		{
			CVertexBufferHardGLMapObjectATI &vbati = **it;
			nlwarning("Buffer id = %d, size = %d, address = 0x%x", (int) vbati._VertexObjectId, (int) (vbati.VB->getVertexSize() * vbati.VB->getNumVertices()), (int) vbati.getPointer());
		}
	}
#endif




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
	H_AUTO_OGL(CVertexArrayRangeARB_CVertexArrayRangeARB)
}

// ***************************************************************************
bool CVertexArrayRangeARB::allocate(uint32 size, CVertexBuffer::TPreferredMemory vbType)
{
	H_AUTO_OGL(CVertexArrayRangeARB_allocate)
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
	H_AUTO_OGL(CVertexArrayRangeARB_free)
	_SizeAllocated = 0;
}

// ***************************************************************************
IVertexBufferHardGL *CVertexArrayRangeARB::createVBHardGL(uint size, CVertexBuffer *vb)
{
	H_AUTO_OGL(CVertexArrayRangeARB_createVBHardGL)
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
	newVbHard->initGL(vertexBufferID, this, _VBType);
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
	return newVbHard;
}

// ***************************************************************************
void CVertexArrayRangeARB::enable()
{
	H_AUTO_OGL(CVertexArrayRangeARB_enable)
	if(_Driver->_CurrentVertexArrayRange != this)
	{				
		_Driver->_CurrentVertexArrayRange= this;
	}
}

// ***************************************************************************
void CVertexArrayRangeARB::disable()
{
	H_AUTO_OGL(CVertexArrayRangeARB_disbale)
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{			
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void CVertexArrayRangeARB::updateLostBuffers()
{
	H_AUTO_OGL(CVertexArrayRangeARB_updateLostBuffers)
	// Put all vb that have been lost in the NotResident state so that they will be recomputed
	// We do this only if the app is active, because if vb were lost, it is likely that there are no resources available.
	nlassert(_Driver);
	if (_Driver->isWndActive())
	{
		for(std::list<CVertexBufferHardARB *>::iterator it = _LostVBList.begin(); it != _LostVBList.end(); ++it)
		{
			nlassert((*it)->_VertexObjectId);	
			GLuint id = (GLuint) (*it)->_VertexObjectId;
			nlassert(nglIsBufferARB(id));
			nglDeleteBuffersARB(1, &id);
			(*it)->_VertexObjectId = 0;
			(*it)->VB->setLocation(CVertexBuffer::NotResident);
		}
		_LostVBList.clear();
	}	
}


// ***************************************************************************
// CVertexBufferHardARB
// ***************************************************************************


// ***************************************************************************
CVertexBufferHardARB::CVertexBufferHardARB(CDriverGL *drv, CVertexBuffer *vb) :  IVertexBufferHardGL(drv, vb),
                                                                                 _VertexPtr(NULL),
																				_VertexObjectId(0)
																					
{	
	H_AUTO_OGL(CVertexBufferHardARB_CVertexBufferHardARB)
	// Flag our type
	VBType = ARBVB;	
	_VertexArrayRange = NULL;
	#ifdef NL_DEBUG
		_Unmapping = false;
	#endif
}

// ***************************************************************************
CVertexBufferHardARB::~CVertexBufferHardARB()
{
	H_AUTO_OGL(CVertexBufferHardARB_CVertexBufferHardARBDtor)
	if (_Driver && _VertexObjectId)
	{
		if (_Driver->_DriverGLStates.getCurrBoundARBVertexBuffer() == _VertexObjectId)
		{
			_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);
		}		
	}
	if (_VertexObjectId)
	{
		GLuint id = (GLuint) _VertexObjectId;
		nlassert(nglIsBufferARB(id));
		nglDeleteBuffersARB(1, &id);
	}
	if (_VertexArrayRange)
	{		
		if (_Invalid)
		{
			if (VB->getLocation() != CVertexBuffer::NotResident)
			{
				// when the vb is put in tthe NotResident state, it is removed from that list
				_VertexArrayRange->_LostVBList.erase(_IteratorInLostVBList);
			}
		}
	}	
	#ifdef NL_DEBUG
		if (_VertexPtr)
		{
			_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
		}
	#endif
}

// ***************************************************************************
void *CVertexBufferHardARB::lock()
{
	H_AUTO_OGL(CVertexBufferHardARB_lock)
	if (_VertexPtr) return _VertexPtr; // already locked
	if (_Invalid)
	{				
		if (VB->getLocation() != CVertexBuffer::NotResident)
		{			
			nlassert(!_DummyVB.empty());
			return &_DummyVB[0];
		}		
		// recreate a vb		
		GLuint vertexBufferID;
		nglGenBuffersARB(1, &vertexBufferID);
		if (glGetError() != GL_NO_ERROR)
		{
			_Driver->incrementResetCounter();
			return &_DummyVB[0];
		}
		const uint size = VB->getNumVertices() * VB->getVertexSize();
		_Driver->_DriverGLStates.forceBindARBVertexBuffer(vertexBufferID);	
		switch(_MemType)
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
			_Driver->incrementResetCounter();
			nglDeleteBuffersARB(1, &vertexBufferID);
			return &_DummyVB[0];;
		}	
		_VertexObjectId = vertexBufferID;
		NLMISC::contReset(_DummyVB); // free vector memory for real		
		nlassert(_VertexObjectId);
		_Invalid = false;
		_VertexArrayRange->_LostVBList.erase(_IteratorInLostVBList);
		// continue to standard mapping code below ..		
	}	
	TTicks	beforeLock;
	if(_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	_Driver->_DriverGLStates.bindARBVertexBuffer(_VertexObjectId);
	_VertexPtr = nglMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	if (!_VertexPtr)
	{
		nglUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		nlassert(nglIsBufferARB((GLuint) _VertexObjectId));
		invalidate();
		return &_DummyVB[0];
	}
	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.push_front(this);
		_IteratorInMappedVBList = _VertexArrayRange->_MappedVBList.begin();
	#endif
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
	H_AUTO_OGL(CVertexBufferHardARB_unlock)
	_VertexPtr = NULL;
	if (_Invalid) return;
	if (!_VertexObjectId) return;
	TTicks	beforeLock = 0;
	if(_Driver->_VBHardProfiling)
	{
		beforeLock= CTime::getPerformanceTime();
	}
	_Driver->_DriverGLStates.bindARBVertexBuffer(_VertexObjectId);	
	// double start = CTime::ticksToSecond(CTime::getPerformanceTime());	
	#ifdef NL_DEBUG
		_Unmapping = true;
	#endif
	GLboolean unmapOk = nglUnmapBufferARB(GL_ARRAY_BUFFER_ARB);	
	#ifdef NL_DEBUG
		_Unmapping = false;
	#endif
	// Lock Profile?
	if(_Driver->_VBHardProfiling)
	{
		TTicks	afterLock;
		afterLock= CTime::getPerformanceTime();
		_Driver->appendVBHardLockProfile(afterLock-beforeLock, VB);
	}
	#ifdef NL_DEBUG
		_VertexArrayRange->_MappedVBList.erase(_IteratorInMappedVBList);
	#endif
	_Driver->_DriverGLStates.forceBindARBVertexBuffer(0);			
	if (!unmapOk)
	{
		invalidate();
	}
	/* double end = CTime::ticksToSecond(CTime::getPerformanceTime());
	nlinfo("Unlock = %f ms", (float) ((end - start) * 1000)); */	
}

// ***************************************************************************
void		*CVertexBufferHardARB::getPointer()
{
	H_AUTO_OGL(CVertexBufferHardARB_getPointer)
	return _VertexPtr;
}

// ***************************************************************************
void CVertexBufferHardARB::unlock(uint startVert,uint endVert)
{
	H_AUTO_OGL(CVertexBufferHardARB_unlock)
	unlock(); // can't do a lock on a range of the vb..
}

// ***************************************************************************
void CVertexBufferHardARB::enable()
{
	H_AUTO_OGL(CVertexBufferHardARB_enable)
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
	H_AUTO_OGL(CVertexBufferHardARB_disable)
	if(_Driver->_CurrentVertexBufferHard != NULL)
	{
		/* nlassert(_VertexArrayRange);
		_VertexArrayRange->disable(); */
		_Driver->_CurrentVertexBufferHard= NULL;
	}
}

// ***************************************************************************
void CVertexBufferHardARB::initGL(uint vertexObjectID, CVertexArrayRangeARB *var, CVertexBuffer::TPreferredMemory memType)
{	
	H_AUTO_OGL(CVertexBufferHardARB_initGL)
	_VertexObjectId = vertexObjectID;
	_MemType = memType;
	_VertexArrayRange = var;
}

// ***************************************************************************
void			CVertexBufferHardARB::lockHintStatic(bool staticLock)
{
	H_AUTO_OGL(CVertexBufferHardARB_lockHintStatic)
	// no op.
}

// ***************************************************************************
void CVertexBufferHardARB::setupVBInfos(CVertexBufferInfo &vb)
{
	H_AUTO_OGL(CVertexBufferHardARB_setupVBInfos)
	vb.VBMode = CVertexBufferInfo::HwARB;
	vb.VertexObjectId = _VertexObjectId;
}

// ***************************************************************************
void CVertexBufferHardARB::invalidate()
{	
	H_AUTO_OGL(CVertexBufferHardARB_invalidate)
	nlassert(!_Invalid);	
	// Buffer is lost (maybe there was a alt-tab or fullscrren / windowed change)
	// Buffer is deleted at end of frame only
	_Invalid = true;		
	_Driver->incrementResetCounter();	
	_DummyVB.resize(VB->getNumVertices() * VB->getVertexSize(), 0);	
	// insert in lost vb list	
	_VertexArrayRange->_LostVBList.push_front(this);
	_IteratorInLostVBList = _VertexArrayRange->_LostVBList.begin();		
}

// ***************************************************************************
#ifdef NL_DEBUG
	void CVertexArrayRangeARB::dumpMappedBuffers()
	{
		nlwarning("*****************************************************");
		nlwarning("Mapped buffers :");
		for(std::list<CVertexBufferHardARB *>::iterator it = _MappedVBList.begin(); it != _MappedVBList.end(); ++it)
		{
			CVertexBufferHardARB &vbarb = **it;
			nlwarning("Buffer id = %d, size = %d, address = 0x%x", (int) vbarb._VertexObjectId, (int) (vbarb.VB->getVertexSize() * vbarb.VB->getNumVertices()), (int) vbarb.getPointer());
		}
	}
#endif


}

