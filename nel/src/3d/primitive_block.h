/** \file primitive_block.h
 * <File description>
 *
 * $Id: primitive_block.h,v 1.3 2001/07/03 08:33:39 corvazier Exp $
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

#ifndef NL_PRIMITIVE_BLOCK_H
#define NL_PRIMITIVE_BLOCK_H

#include "nel/misc/types_nl.h"
#include <vector>

namespace NLMISC
{
	class IStream;
};

namespace NL3D {


/**
 * Class CPrimitiveBlock
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CPrimitiveBlock
{
private:
	// Triangles.
	uint32				_NbTris;
	uint32				_TriCapacity;
	std::vector<uint32>	_Tri;

	// Quads
	uint32				_NbQuads;
	uint32				_QuadCapacity;
	std::vector<uint32>	_Quad;

	// Lines
	uint32				_NbLines;
	uint32				_LineCapacity;
	std::vector<uint32>	_Line;

	/// \todo hulud: support for strips and fans
	// Strip/Fans
	uint32				_StripIdx;
	uint32*				_Strip;
	uint32				_FanIdx;
	uint32*				_Fan;
public:
						CPrimitiveBlock(void) 
						{_TriCapacity=_NbTris= _NbQuads=_QuadCapacity=_NbLines=_LineCapacity= 0;};
						~CPrimitiveBlock(void) {}; 
	
	
	// Lines. A line is 2 uint32.
	
	/// reserve space for nLines Line. You are allowed to write your Line indices on this space.
	void				reserveLine(uint32 n);
	/// Return the number of Line reserved.
	uint32				capacityLine() const {return _LineCapacity;}
	/// Set the number of active Line. It enlarge Line capacity, if needed.
	void				setNumLine(uint32 n);
	/// Get the number of active Lineangles.
	uint32				getNumLine(void) const {return _NbLines;}

	/// Build a Lineangle.
	void				setLine(uint lineIdx, uint32 vidx0, uint32 vidx1);
	/// Apend a line at getNumLine() (then resize +1 the numline).
	void				addLine(uint32 vidx0, uint32 vidx1);

	uint32*				getLinePointer(void);
	const uint32*				getLinePointer(void) const ;



	// Triangles. A triangle is 3 uint32.
	
	/// reserve space for nTris triangles. You are allowed to write your triangles indices on this space.
	void				reserveTri(uint32 n);
	/// Return the number of triangles reserved.
	uint32				capacityTri() {return _TriCapacity;}
	/// Set the number of active triangles. It enlarge Tri capacity, if needed.
	void				setNumTri(uint32 n);
	/// Get the number of active triangles.
	uint32				getNumTri(void) const {return _NbTris;}

	/// Build a triangle.
	void				setTri(uint triIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2);
	/// Apend a triangle at getNumTri() (then resize +1 the numtri).
	void				addTri(uint32 vidx0, uint32 vidx1, uint32 vidx2);

	uint32*				getTriPointer(void);
	const uint32*				getTriPointer(void) const ;




	// Quads (a quad is 4 uint32)

	/**
	 *	reserve space for quads. 
	 */
	void reserveQuad(uint32 n);
	
	/**
	 * Return the number of triangles reserved.
	 */
	uint32 capacityQuad() { return _QuadCapacity; }
	
	/**
	 * Set the number of active quads. It enlarges Quad capacity, if needed.
	 */
	void setNumQuad(uint32 n);
	
	/**
	 * Get the number of active quads.
	 */
	uint32 getNumQuad(void) const { return _NbQuads; }

	/**
	 * Build a quad.
	 */
	void setQuad(uint quadIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3);

	/// Apend a quad at getNumQuad() (then resize +1 the numquad).
	void				addQuad(uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3);

	/**
	 * Return the Quad buffer
	 */
	uint32*	getQuadPointer(void);

	/// return the quad buffer, const version
	const uint32*	getQuadPointer(void) const ;

	/// return total number of triangle in this primitive block
	uint32	getNumTriangles ()
	{
		// Return number of triangles in this primitive block
		return _NbTris+2*_NbQuads+_NbLines;
	}

	void		serial(NLMISC::IStream &f);
};


} // NL3D


#endif // NL_PRIMITIVE_BLOCK_H

/* End of primitive_block.h */
