/** \file vertex_buffer_hard.h
 * <File description>
 *
 * $Id: vertex_buffer_hard.h,v 1.2 2001/07/05 08:33:04 berenguier Exp $
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
	uint32					getVertexFormat(void) const  { return(_Flags); }
	uint8					getVertexSize(void) const {return(_VertexSize);}
	uint32					getNumVertices(void) const  {return(_NbVerts);}
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
	sint					getNormalOff() const {nlassert(_Flags & IDRV_VF_NORMAL); return _NormalOff;}
	sint					getTexCoordOff(uint8 stage=0) const  {nlassert(_Flags & IDRV_VF_UV[stage]); return _UVOff[stage];}
  	sint					getColorOff() const {nlassert(_Flags & IDRV_VF_COLOR); return _RGBAOff;}
	sint					getSpecularOff() const {nlassert(_Flags & IDRV_VF_SPECULAR); return _SpecularOff;}
	/// NB: it is ensured that   WeightOff(i)==WeightOff(0)+i*sizeof(float).
	sint					getWeightOff(sint wgt) const {nlassert(_Flags & IDRV_VF_W[wgt]); return _WOff[wgt];}
	sint					getPaletteSkinOff() const {nlassert(_Flags & IDRV_VF_PALETTE_SKIN); return _PaletteSkinOff;}
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
	// @}


// *************************
protected:

	uint32					_Flags;
	uint8					_VertexSize;
	uint32					_NbVerts;

	uint					_WOff[IDRV_VF_MAXW];
	uint					_NormalOff;
	uint					_RGBAOff;
	uint					_SpecularOff;
	uint					_UVOff[IDRV_VF_MAXSTAGES];
	uint					_PaletteSkinOff;

	/// Constructor: build good offfsets / size.
	IVertexBufferHard() {}
	virtual ~IVertexBufferHard() {}


	void					initFormat(uint32 vertexFormat, uint32 numVertices);

};


} // NL3D


#endif // NL_VERTEX_BUFFER_HARD_H

/* End of vertex_buffer_hard.h */
