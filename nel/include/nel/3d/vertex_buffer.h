/** \file vertex_buffer.h
 * <File description>
 *
 * $Id: vertex_buffer.h,v 1.6 2001/04/03 13:02:56 berenguier Exp $
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

#ifndef NL_VERTEX_BUFFER_H
#define NL_VERTEX_BUFFER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/debug.h"
#include <vector>

namespace NLMISC 
{
	class CVector;
	class CUV;
}


namespace NL3D 
{

using NLMISC::CRefCount;
using NLMISC::CRefPtr;
using NLMISC::CRGBA;
using NLMISC::CVector;
using NLMISC::CUV;


// --------------------------------------------------

// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IVBDrvInfos : public CRefCount
{
private:
public:
	// The virtual dtor is important.
	virtual ~IVBDrvInfos() {};
};


// ***************************************************************************
/** Describe index for palette skinning.
 * Id must be in range [0, IDriver::MaxModelMatrix [
 */
struct	CPaletteSkin
{
	uint8	MatrixId[4];

	void	serial(NLMISC::IStream &f);
};


// ***************************************************************************
// Vertex Format
const uint32	IDRV_VF_MAXW		=	4;
const uint32	IDRV_VF_MAXSTAGES	=	8;
const uint32	IDRV_VF_XYZ			=	0x00000001;
const uint32	IDRV_VF_W[IDRV_VF_MAXW]			= { 0x00000002,0x00000004,0x00000008,0x00000010 };
const uint32	IDRV_VF_NORMAL		=	0x00000020;
const uint32	IDRV_VF_COLOR		=	0x00000040;
const uint32	IDRV_VF_SPECULAR	=	0x00000080;
const uint32	IDRV_VF_UV[IDRV_VF_MAXSTAGES]	= { 0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000 };
const uint32	IDRV_VF_PALETTE_SKIN=  0x00010000 | IDRV_VF_W[0] | IDRV_VF_W[1] | IDRV_VF_W[2] | IDRV_VF_W[3];

// Touch Flags.
const uint32	IDRV_VF_TOUCHED_VERTEX_FORMAT	=	0x00000001;
const uint32	IDRV_VF_TOUCHED_NUM_VERTICES	=	0x00000002;


/**
 * <Class description>
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// All these flags are similar to DX8
class CVertexBuffer : public CRefCount
{
private:
	uint32					_Flags;
	uint8					_VertexSize;
	uint32					_NbVerts;
	uint32					_Capacity;
	std::vector<uint8>		_Verts;

	uint					_WOff[IDRV_VF_MAXW];
	uint					_NormalOff;
	uint					_RGBAOff;
	uint					_SpecularOff;
	uint					_UVOff[IDRV_VF_MAXSTAGES];
	uint					_PaletteSkinOff;

	uint					_Touch;

public:
	// \name Private. For Driver only.
	// @{
	CRefPtr<IVBDrvInfos>	DrvInfos;
	uint					getTouchFlags() const {return _Touch;}
	void					resetTouchFlags() {_Touch= 0;}
	// @}

public:
	CVertexBuffer(void);
	// see operator=.
	CVertexBuffer(const CVertexBuffer &vb);
	~CVertexBuffer(void);
	// Do not copy DrvInfos, copy all infos and set IDRV_TOUCHED_ALL.
	CVertexBuffer			&operator=(const CVertexBuffer &vb);


	/// Setup the vertex format. Do it before any other method.
	bool					setVertexFormat(uint32 Flags);
	uint32					getVertexFormat(void) { return(_Flags); };
	uint8					getVertexSize(void) {return(_VertexSize);}

	/// reserve space for nVerts vertices. You are allowed to write your vertices on this space.
	void					reserve(uint32 nVerts);
	/// Return the number of vertices reserved.
	uint32					capacity() {return _Capacity;}
	/// Set the number of active vertices. It enlarge capacity, if needed.
	void					setNumVertices(uint32 n);
	/// Get the number of active vertices.
	uint32					getNumVertices(void) {return(_NbVerts);}


	// It is an error (assert) to set a vertex component if not setuped in setVertexFormat().
	void					setVertexCoord(uint idx, float x, float y, float z);
	void					setVertexCoord(uint idx, const CVector &v);
	void					setNormalCoord(uint idx, const CVector &v);
	void					setTexCoord(uint idx, uint8 stage, float u, float v);
	void					setTexCoord(uint idx, uint8 stage, const CUV &uv);
	void					setColor(uint idx, CRGBA rgba);
	void					setSpecular(uint idx, CRGBA rgba);
	void					setWeight(uint idx, uint8 wgt, float w);
	void					setPaletteSkin(uint idx, CPaletteSkin ps);


	// It is an error (assert) to query a vertex offset of a vertex component not setuped in setVertexFormat().
	// NB: The Vertex offset is always 0.
	sint					getNormalOff() const {nlassert(_Flags & IDRV_VF_NORMAL); return _NormalOff;}
	sint					getTexCoordOff(uint8 stage=0) const  {nlassert(_Flags & IDRV_VF_UV[stage]); return _UVOff[stage];}
  	sint					getColorOff() const {nlassert(_Flags & IDRV_VF_COLOR); return _RGBAOff;}
	sint					getSpecularOff() const {nlassert(_Flags & IDRV_VF_SPECULAR); return _SpecularOff;}
	/// NB: it is ensured that   WeightOff(i)==WeightOff(0)+i*sizeof(float).
	sint					getWeightOff(sint wgt) const {nlassert(_Flags & IDRV_VF_W[wgt]); return _WOff[wgt];}
	sint					getPaletteSkinOff() const {nlassert(_Flags & IDRV_VF_PALETTE_SKIN); return _PaletteSkinOff;}


	void*					getVertexCoordPointer(uint idx=0);
	void*					getNormalCoordPointer(uint idx=0);
	void*					getTexCoordPointer(uint idx=0, uint8 stage=0);
	void*					getColorPointer(uint idx=0);
	void*					getSpecularPointer(uint idx=0);
	void*					getWeightPointer(uint idx=0, uint8 wgt=0);
	void*					getPaletteSkinPointer(uint idx=0);


	void		serial(NLMISC::IStream &f);
};


} // NL3D


#endif // NL_VERTEX_BUFFER_H

/* End of vertex_buffer.h */
