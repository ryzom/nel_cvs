/** \file vertex_buffer_hard.h
 * <File description>
 *
 * $Id: vertex_buffer_hard.h,v 1.5 2003/03/13 13:40:59 corvazier Exp $
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

#ifndef NL_VERTEX_BUFFER_HARD_H
#define NL_VERTEX_BUFFER_HARD_H

#include "nel/misc/types_nl.h"
#include "3d/vertex_buffer.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * This a vertex buffer created by Driver. His pointer may be allocated on AGP Ram or VRAM.
 * see CVertexBuffer, because it use the same flags for vertex format.
 * IVertexBufferHard are created / deleted by IDriver.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class IVertexBufferHard : public CRefCount
{
public:

	/// \name Misc Get.
	// @{
	uint16					getVertexFormat(void) const  { return(_Flags); }
	uint16					getVertexSize(void) const {return(_VertexSize);}
	uint32					getNumVertices(void) const  {return(_NbVerts);}
	CVertexBuffer::TType	getValueType (uint value) const { nlassert (value<CVertexBuffer::NumValue); return((CVertexBuffer::TType)_Type[value]); }
	uint8					getNumWeight () const;
	const uint8				*getUVRouting () const { return _UVRouting; }
	// @}


	/// \name Get component offsets.
	/** NB: the order of those methods follow the order in memory of the elements:
	 *	- VertexCoord
	 *	- NormalCoord
	 *	- TexCoord
	 *	- Color
	 *	- Specular
	 *	- Weight
	 *	- PaletteSkin
	 *	NB: same order as in CVertexBuffer.
	 */
	// @{
	// It is an error (assert) to query a vertex offset of a vertex component not setuped VertexFormat.
	// NB: The Vertex offset is always 0.
	/*sint					getNormalOff() const {nlassert(_Flags & CVertexBuffer::NormalFlag); return _Offset[CVertexBuffer::Normal];}
	sint					getTexCoordOff(uint8 stage=0) const  {nlassert(_Flags & (CVertexBuffer::TexCoord0Flag<<stage)); return _Offset[CVertexBuffer::TexCoord0+stage];}
  	sint					getColorOff() const {nlassert(_Flags & CVertexBuffer::PrimaryColorFlag); return _Offset[CVertexBuffer::PrimaryColor];}
	sint					getSpecularOff() const {nlassert(_Flags & CVertexBuffer::SecondaryColorFlag); return _Offset[CVertexBuffer::SecondaryColor];}
	/// NB: it is ensured that   WeightOff(i)==WeightOff(0)+i*sizeof(float).
	sint					getWeightOff(sint wgt) const {nlassert(_Flags & CVertexBuffer::WeightFlag); return _Offset[CVertexBuffer::Weight]+sizeof(float)*wgt;}
	sint					getPaletteSkinOff() const {nlassert(_Flags & CVertexBuffer::PaletteSkinFlag); return _Offset[CVertexBuffer::PaletteSkin];}*/
	sint					getValueOff(uint value) const {nlassert(_Flags & (1<<value)); return _Offset[value];}
	// @}


	/// \name Vertex access.
	// @{
	/** Lock the entire VertexBuffer. The returned buffer is a write only buffer. (undefined result if you read it).
	 *	Write note:
	 *	- for VRAM VertexBufferHard, you should write only one time, to init.
	 *	- for AGP VertexBufferHard, you should write sequentially to take full advantage of the write combiners.
	 *	\return the adress of the vertex 0. Use getVertexSize(), get*Off() ... to access any part of this VertexBuffer.
	 */
	virtual	void			*lock() =0;
	/** UnLock the VertexBuffer so the Gfx card can now use it.
	 */
	virtual void			unlock() =0;
	/** Same as unlock() but give hints to driver on vertices that have changed. Used by ATI-OpenGL drivers.
	 *	NB: driver may still ignore this information, and so take into acount all vertices
	 *	\param	startVert the fisrt vertex to update
	 *	\param	endVert the last vertex (not included) to update => numVertices= vertEnd-vertBegin.
	 *	Hence unlock() as the same effect as unlock(0, getNumVertices());
	 */
	virtual void			unlock(uint startVert, uint endVert) =0;
	// @}


// *************************
protected:

	// Type of data stored in each value
	uint8					_Type[CVertexBuffer::NumValue];	// Offset 0 : aligned
	uint8					_Pad;				// Offset 13 : aligned

	// Size of the vertex (sum of the size of each value
	uint16					_VertexSize;		// Offset 14 : aligned

	// Flags: bit #n is 1 if the value #n is used
	uint16					_Flags;				// Offset 16 : aligned

	// Internal flags
	uint16					_InternalFlags;		// Offset 18 : aligned

	// Vertex count in the buffer
	uint32					_NbVerts;			// Offset 20 : aligned

	// Offset of each value
	uint16					_Offset[CVertexBuffer::NumValue];

	/// Constructor: build good offfsets / size.
	IVertexBufferHard() {}
	virtual ~IVertexBufferHard() {}

	// The routing
	uint8					_UVRouting[CVertexBuffer::MaxStage];

	void					initFormat (uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, const uint8 *uvRouting);

};


} // NL3D


#endif // NL_VERTEX_BUFFER_HARD_H

/* End of vertex_buffer_hard.h */
