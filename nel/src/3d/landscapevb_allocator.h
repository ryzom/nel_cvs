/** \file landscapevb_allocator.h
 * <File description>
 *
 * $Id: landscapevb_allocator.h,v 1.1 2001/09/10 10:06:56 berenguier Exp $
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

#ifndef NL_LANDSCAPEVB_ALLOCATOR_H
#define NL_LANDSCAPEVB_ALLOCATOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "3d/tessellation.h"
#include "3d/vertex_buffer.h"


namespace NL3D 
{


class	IDriver;



// ***************************************************************************
/**
 * A class to easily allocate vertices for Landscape Far / Tile faces.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeVBAllocator
{
public:

	enum	TType	{Far0, Far1, Tile};

	/// Constructor
	CLandscapeVBAllocator(TType type);
	~CLandscapeVBAllocator();

	/** setup driver, and test for possible VBHard reallocation.
	 *  if the VBhard/Driver has been deleted externally, Vertices are lost.
	 *	The vertex buffer is reallocated and reallocationOccurs() return true (see reallocationOccurs()).
	 *	to do anytime you're not sure of change of the driver/vbHard state.
	 *	\param driver must not be NULL.
	 */
	void			updateDriver(IDriver *driver);

	// delete all VB, and free driver ressources (if RefPtr driver not deleted). clear list too.
	void			clear();


	/// \name Allocation.
	// @{

	// true if reallocationOccurs during a allocateVertex() or a setDriver().
	// a buildVBInfo() should be done and ALL data are lost, so all VB must be rewrited.
	bool			reallocationOccurs() const {return _ReallocationOccur;}
	void			resetReallocation();

	// Allocate free vertices in VB. (AGP or RAM). work with locked or unlocked buffer.
	// NB: if reallocationOccurs(), then ALL data are lost.
	uint			allocateVertex();
	// Delete free vertices in VB. (AGP or RAM).
	void			deleteVertex(uint vid);
	// @}


	/// \name Buffer access.
	// @{
	/** lock buffers Hard (if any). "slow call", so batch them. nlassert good TType. return is the VB info.
	 *	NB: if the buffer is locked while a reallocation occurs, then the buffer is unlocked.
	 */
	void			lockBuffer(CFarVertexBufferInfo &farVB);
	void			lockBuffer(CNearVertexBufferInfo &tileVB);
	void			unlockBuffer();
	bool			bufferLocked() const {return _BufferLocked;}

	/// activate the VB or the VBHard in Driver setuped. nlassert if driver is NULL or if buffer is locked.
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
	TType						_Type;
	
	bool						_ReallocationOccur;
	// List of vertices free.
	std::vector<uint>			_VertexFreeMemory;
	std::vector<CVertexInfo>	_VertexInfos;
	uint						_NumVerticesAllocated;


	/// \name VB mgt .
	// @{

	// a refPtr on the driver, to delete VBuffer Hard at clear().
	NLMISC::CRefPtr<IDriver>			_Driver;
	// tell if VBHard is possible.
	bool								_VBHardOk;
	CVertexBuffer						_VB;
	NLMISC::CRefPtr<IVertexBufferHard>	_VBHard;
	bool								_BufferLocked;

	/* try to create a vertexBufferHard or a vbSoft if not possible.
		After this call, the vertexBufferHard may be NULL.
	*/
	void				deleteVertexBuffer();
	void				allocateVertexBuffer(uint32 numVertices);
	// @}


};


} // NL3D


#endif // NL_LANDSCAPEVB_ALLOCATOR_H

/* End of landscapevb_allocator.h */
