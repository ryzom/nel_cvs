/** \file vegetablevb_allocator.h
 * <File description>
 *
 * $Id: vegetablevb_allocator.h,v 1.1 2001/10/31 10:19:40 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_VEGETABLEVB_ALLOCATOR_H
#define NL_VEGETABLEVB_ALLOCATOR_H

#include "nel/misc/types_nl.h"
#include "3d/driver.h"


namespace NL3D 
{

class	CVertexProgram;



// ***************************************************************************
// Vegetable VertexProgram: Position of vertices in VertexBuffer.
#define	NL3D_VEGETABLE_VPPOS_POS		(CVertexBuffer::Position)
#define	NL3D_VEGETABLE_VPPOS_NORMAL		(CVertexBuffer::Normal)
#define	NL3D_VEGETABLE_VPPOS_COLOR0		(CVertexBuffer::PrimaryColor)
#define	NL3D_VEGETABLE_VPPOS_COLOR1		(CVertexBuffer::SecondaryColor)
#define	NL3D_VEGETABLE_VPPOS_TEX0		(CVertexBuffer::TexCoord0)
#define	NL3D_VEGETABLE_VPPOS_BENDINFO	(CVertexBuffer::TexCoord1)
#define	NL3D_VEGETABLE_VPPOS_CENTER		(CVertexBuffer::TexCoord2)


// ***************************************************************************
/**
 * A VB allocator (landscape like).
 *	Big difference is that here, we do not really matter about reallocation because both software
 *	and hardware VB are present. Also, VertexProgram MUST be supported by driver here.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableVBAllocator
{
public:

	/// Constructor
	CVegetableVBAllocator();
	~CVegetableVBAllocator();
	/// init the VB allocator, with the good type. must do it first. type must be one of NL3D_VEGETABLE_RDRPASS_*.
	void			init(uint type);


	/** setup driver, and test for possible VBHard reallocation. if reallocation, refill the VBHard
	 *	to do anytime you're not sure of change of the driver/vbHard state.
	 *
	 *	Note: the vertexProgram is created/changed here, according to driver, and TType.
	 *
	 *	\param driver must not be NULL.
	 */
	void			updateDriver(IDriver *driver);


	// delete all VB, and free driver ressources (if RefPtr driver not deleted). clear list too.
	void			clear();


	/// \name Allocation.
	// @{

	/** Allocate free vertices in VB. (RAM and AGP if possible). work with locked or unlocked buffer.
	 *	if VBHard reallocation occurs, VB is unlocked, destroyed, reallocated, and refilled.
	 */
	uint			allocateVertex();
	/// Delete free vertices in VB. (AGP or RAM).
	void			deleteVertex(uint vid);

	// @}


	/// \name Buffer access.
	// @{
	/// get the software VB pointer to the ith index. valid only beetween 2 allocateVertex().
	void			*getVertexPointer(uint i);
	// return soft VB, for info only.
	const CVertexBuffer		&getSoftwareVertexBuffer() const {return _VB;}
	/// If VBHard ok, copy the vertex in AGP. Warning: buffer must be locked!
	void			flushVertex(uint i);

	/// if any, lock the AGP buffer.
	void			lockBuffer();
	/// if any, unlock the AGP buffer.
	void			unlockBuffer();
	bool			bufferLocked() const {return _BufferLocked;}

	/** activate the VB or the VBHard in Driver setuped. nlassert if driver is NULL or if buffer is locked.
	 * activate the vertexProgram too.
	 */
	void			activate();
	// @}


// ******************
private:

	// For Debug.
	struct	CVertexInfo
	{
		bool	Free;
	};

private:
	uint						_Type;
	
	// List of vertices free.
	std::vector<uint>			_VertexFreeMemory;
	std::vector<CVertexInfo>	_VertexInfos;
	uint						_NumVerticesAllocated;


	/// \name VB mgt .
	// @{

	// Our software VB. always here, and always correct.
	CVertexBuffer						_VB;

	// a refPtr on the driver, to delete VBuffer Hard at clear().
	NLMISC::CRefPtr<IDriver>			_Driver;
	// tell if VBHard is possible.
	bool								_VBHardOk;
	NLMISC::CRefPtr<IVertexBufferHard>	_VBHard;
	bool								_BufferLocked;
	uint8								*_AGPBufferPtr;

	/// delete only the Vertexbuffer hard.
	void				deleteVertexBufferHard();
	/* create a VertexBufferSoft, and try to create a vertexBufferHard
		After this call, the vertexBufferHard may be NULL.
		if VBHard allocation, copy from soft.
	*/
	void				allocateVertexBufferAndFillVBHard(uint32 numVertices);
	// @}


	/// \name Vertex Program mgt .
	// @{
	// Vertex Program.
	CVertexProgram		*_VertexProgram;
	// create _VertexProgram and init VB according to type. called in cons() only.
	void				setupVBFormatAndVertexProgram();
	// @}

};


} // NL3D


#endif // NL_VEGETABLEVB_ALLOCATOR_H

/* End of vegetablevb_allocator.h */
