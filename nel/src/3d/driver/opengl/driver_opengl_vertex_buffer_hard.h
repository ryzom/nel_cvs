/** \file driver_opengl_vertex_buffer_hard.h
 * <File description>
 *
 * $Id: driver_opengl_vertex_buffer_hard.h,v 1.1 2002/08/30 11:58:02 berenguier Exp $
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

#ifndef NL_DRIVER_OPENGL_VERTEX_BUFFER_HARD_H
#define NL_DRIVER_OPENGL_VERTEX_BUFFER_HARD_H

#include "nel/misc/types_nl.h"
#include "3d/vertex_buffer_hard.h"


namespace NL3D 
{

	
class	CDriverGL;
class	IVertexBufferHardGL;


// ***************************************************************************
// ***************************************************************************
// VBHard interface for both NVidia / ATI extension.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/** Interface to a Big block of AGP memory either throurgh NVVertexArrayRange or ATIVertexObject
 */
class IVertexArrayRange
{
public:
	IVertexArrayRange(CDriverGL *drv);
	virtual	~IVertexArrayRange();

	/// allocate a vertex array space. false if error. client must free before re-allocate.
	virtual	bool					allocate(uint32 size, IDriver::TVBHardType vbType)= 0;
	/// free this space.
	virtual	void					free()= 0;
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices) =0;
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const =0;

protected:
	CDriverGL	*_Driver;
};



// ***************************************************************************
/** Common interface for both NVidia and ATI extenstion
 *
 */
class IVertexBufferHardGL : public IVertexBufferHard
{
public:

	IVertexBufferHardGL(CDriverGL *drv);
	virtual	~IVertexBufferHardGL();


	// ATI and NVidia have their own methods.
	//virtual	void		*lock();
	//virtual	void		unlock();
	//virtual void			unlock(uint startVert, uint endVert);

	virtual	void			enable() =0;
	virtual	void			disable() =0;


	// true if NVidia vertex buffer hard.
	bool				NVidiaVertexBufferHard;
	// true if ATI vertex buffer hard.
	bool				ATIVertexBufferHard;
	// For Fence access. Ignored for ATI.
	bool				GPURenderingAfterFence;


public:
	// shortcut to IVertexBufferHard::initFormat()
	void					initFormat (uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices)
	{
		IVertexBufferHard::initFormat(vertexFormat, typeArray, numVertices);
	}


protected:
	CDriverGL			*_Driver;
};



// ***************************************************************************
// ***************************************************************************
// NVidia implementation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/** Work only if ARRAY_RANGE_NV is enabled. else, only call to ctor/dtor/free() is valid.
 *	any call to allocateVB() will return NULL.
 */
class CVertexArrayRangeNVidia : public IVertexArrayRange
{
public:
	CVertexArrayRangeNVidia(CDriverGL *drv);


	/// \name Implementation
	// @{
	/// allocate a vertex array sapce. false if error. must free before re-allocate.
	virtual	bool					allocate(uint32 size, IDriver::TVBHardType vbType);
	/// free this space.
	virtual	void					free();
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices);
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const;
	// @}


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/// active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	void			enable();
	/// disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	void			disable();

	/// free a VB allocated with allocateVB. No-op if NULL.
	void			freeVB(void	*ptr);


// *************************
private:
	void		*_VertexArrayPtr;
	uint32		_VertexArraySize;

	// Allocator.
	NLMISC::CHeapMemory		_HeapMemory;

	/// true if allocated.
	bool			allocated() const {return _VertexArrayPtr!=NULL;}
	/// Allocate a small subset of the memory. NULL if not enough mem.
	void			*allocateVB(uint32 size);

};



// ***************************************************************************
/// Work only if ARRAY_RANGE_NV is enabled.
class CVertexBufferHardGLNVidia : public IVertexBufferHardGL
{
public:

	CVertexBufferHardGLNVidia(CDriverGL *drv);
	virtual	~CVertexBufferHardGLNVidia();


	/// \name Implementation
	// @{
	virtual	void		*lock();
	virtual	void		unlock();
	virtual void		unlock(uint startVert, uint endVert);
	virtual	void		enable();
	virtual	void		disable();
	// @}


	// setup ptrs allocated by createVBHard(), and init Fence.
	void					initGL(CVertexArrayRangeNVidia *var, void *vertexPtr);


	// true if a setFence() has been done, without a finishFence().
	bool			isFenceSet() const {return _FenceSet;}
	// if(!isFenceSet()), set the fence, else no-op
	void			setFence();
	// if(isFenceSet()), finish the fence, else no-op
	void			finishFence();


public:
	void				*getNVidiaValueEx (uint value)
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getValueOff (value);
	}


// *************************
private:
	CVertexArrayRangeNVidia		*_VertexArrayRange;
	void						*_VertexPtr;

	// The fence inserted in command stream
	GLuint				_Fence;
	// True if a setFence() has been done, without a finishFence().
	bool				_FenceSet;

};



// ***************************************************************************
// ***************************************************************************
// ATI implementation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// This is a fake value to simulate allocation on CHeapMemory for ATI mgt.
// NB: can set 0, since won't work for multiple reasons....
#define	NL3D_DRV_ATI_FAKE_MEM_START	0x10000


// ***************************************************************************
/** Work only if ATIVertexArrayObject is enabled. else, only call to ctor/dtor/free() is valid.
 *	any call to allocateVB() will return NULL.
 */
class CVertexArrayRangeATI : public IVertexArrayRange
{
public:
	CVertexArrayRangeATI(CDriverGL *drv);


	/// \name Implementation
	// @{
	/// allocate a vertex array sapce. false if error. must free before re-allocate.
	virtual	bool					allocate(uint32 size, IDriver::TVBHardType vbType);
	/// free this space.
	virtual	void					free();
	/// create a IVertexBufferHardGL
	virtual	IVertexBufferHardGL		*createVBHardGL(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices);
	/// return the size allocated. 0 if not allocated or failure
	virtual	uint					sizeAllocated() const;
	// @}


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/** active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	 *	NB: no-op for ATI, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			enable();
	/** disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	 *	NB: no-op for ATI, but ensure correct _Driver->_CurrentVertexArrayRange value.
	 */
	void			disable();

	/// free a VB allocated with allocateVB. No-op if NULL.
	void			freeVB(void	*ptr);


	/// get Handle of the ATI buffer.
	uint			getATIVertexObjectId() const {return _VertexObjectId;}

// *************************
private:
	uint		_VertexObjectId;
	bool		_Allocated;
	uint32		_VertexArraySize;

	// Allocator. NB: We don't have any Mem Ptr For ATI extension, so use a Fake Ptr: NL3D_DRV_ATI_FAKE_MEM_START
	NLMISC::CHeapMemory		_HeapMemory;

	/// Allocate a small subset of the memory. NULL if not enough mem.
	void			*allocateVB(uint32 size);

};



// ***************************************************************************
/// Work only if ARRAY_RANGE_NV is enabled.
class CVertexBufferHardGLATI : public IVertexBufferHardGL
{
public:

	CVertexBufferHardGLATI(CDriverGL *drv);
	virtual	~CVertexBufferHardGLATI();


	/// \name Implementation
	// @{
	virtual	void		*lock();
	virtual	void		unlock();
	virtual void		unlock(uint startVert, uint endVert);
	virtual	void		enable();
	virtual	void		disable();
	// @}


	/// try to create a RAM mirror that 'll contain a copy of the VB.
	bool					createRAMMirror(uint memSize);

	/**	setup ptrs allocated by createVBHard()
	 */
	void					initGL(CVertexArrayRangeATI *var, void *vertexPtr);


public:
	uint					getATIValueOffset (uint value)
	{
		nlassert(_VertexPtr);
		// To get the correct offset, remove the dummy MEM offset.
		return ((uint)_VertexPtr + getValueOff (value)) - NL3D_DRV_ATI_FAKE_MEM_START;
	}

	/// get Handle of the ATI buffer.
	uint					getATIVertexObjectId() const {return _VertexArrayRange->getATIVertexObjectId();}


// *************************
private:
	CVertexArrayRangeATI		*_VertexArrayRange;
	void						*_VertexPtr;
	void						*_RAMMirrorVertexPtr;
	uint						_RAMMirrorVertexSize;

};



} // NL3D


#endif // NL_DRIVER_OPENGL_VERTEX_BUFFER_HARD_H

/* End of driver_opengl_vertex_buffer_hard.h */
