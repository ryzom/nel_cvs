/** \file vertex_buffer.h
 * <File description>
 *
 * $Id: vertex_buffer.h,v 1.9 2002/09/24 15:03:00 vizerie Exp $
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
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/debug.h"
#include "nel/misc/uv.h"
#include <vector>
#include <list>

namespace NLMISC 
{
	class CUV;
}


namespace NL3D 
{

using NLMISC::CRefCount;
using NLMISC::CRefPtr;
using NLMISC::CRGBA;
using NLMISC::CVector;
using NLMISC::CVectorD;
using NLMISC::CUV;


class	IDriver;

// --------------------------------------------------


// List typedef.
class	IVBDrvInfos;
typedef	std::list<IVBDrvInfos*>			TVBDrvInfoPtrList;
typedef	TVBDrvInfoPtrList::iterator		ItVBDrvInfoPtrList;


// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IVBDrvInfos : public CRefCount
{
private:
	IDriver				*_Driver;
	ItVBDrvInfoPtrList	_DriverIterator;

public:
	IVBDrvInfos(IDriver	*drv, ItVBDrvInfoPtrList it) {_Driver= drv; _DriverIterator= it;}
	// The virtual dtor is important.
	virtual ~IVBDrvInfos();
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

/**
 * A vertex buffer to work with the driver
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// All these flags are similar to DX8
class CVertexBuffer : public CRefCount
{
public:

	/**
	  * Value ID, there is 16 value id
	  */
	enum TValue
	{ 
		Position		=0, 
		Normal			=1, 
		TexCoord0		=2,
		TexCoord1		=3, 
		TexCoord2		=4,
		TexCoord3		=5, 
		TexCoord4		=6, 
		TexCoord5		=7, 
		TexCoord6		=8, 
		TexCoord7		=9,
		PrimaryColor	=10, 
		SecondaryColor	=11, 
		Weight			=12,
		PaletteSkin		=13,
		Fog				=14, 
		Empty			=15,
		NumValue		=16
	};

	/** 
	  * Misc infos
	  */
	enum
	{
		FirstTexCoordValue = TexCoord0,
		LastTexCoordValue  = TexCoord7,		
	};

	/**
	  * Value flags
	  */
	enum 
	{
		PositionFlag		=	1<<Position, 
		NormalFlag			=	1<<Normal, 
		TexCoord0Flag		=	1<<TexCoord0, 
		TexCoord1Flag		=	1<<TexCoord1, 
		TexCoord2Flag		=	1<<TexCoord2, 
		TexCoord3Flag		=	1<<TexCoord3, 
		TexCoord4Flag		=	1<<TexCoord4, 
		TexCoord5Flag		=	1<<TexCoord5, 
		TexCoord6Flag		=	1<<TexCoord6, 
		TexCoord7Flag		=	1<<TexCoord7,
		PrimaryColorFlag	=	1<<PrimaryColor, 
		SecondaryColorFlag	=	1<<SecondaryColor, 
		WeightFlag			=	1<<Weight,
		PaletteSkinFlag		=	(1<<PaletteSkin)|(1<<Weight),
		FogFlag				=	1<<Fog, 
		EmptyFlag			=	1<<Empty,
	};

	

	/**
	  * Value type, there is 13 kind of value type as in DirectX8 and gl_vertex_program used in exteneded mode
	  */
	enum TType
	{ 
		Double1=0, 
		Float1, 
		Short1, 
		Double2, 
		Float2, 
		Short2, 
		Double3, 
		Float3, 
		Short3,
		Double4, 
		Float4, 
		Short4,
		UChar4,
		NumType
	};

	/**
	  * Some constants
	  */
	enum
	{
		// Max count of texture coordinates
		MaxStage = 8,

		// Max count of skinning weight
		MaxWeight = 4,
	};

	/**
	  * Static array with the size in byte of each value type
	  */
	static const uint SizeType[NumType];

	/**
	  * Static array with the standard type of each value
	  */
	static const TType DefaultValueType[NumValue];

	/**
	  * Static array with the number of component of each value type
	  */
	static const uint  NumComponentsType[NumType];

private:

	/**
	  * Internal flags
	  */
	enum
	{
		/// Vertex format touched
		TouchedVertexFormat		= 1,

		/// Num vertices touched
		TouchedNumVertices		= 2,

		/// All touhched
		TouchedAll				= 0xFFFF
	};

	// Type of data stored in each value
	uint8					_Type[NumValue];	// Offset 0 : aligned
	uint8					_Pad;				// Offset 13 : aligned

	// Size of the vertex (sum of the size of each value
	uint16					_VertexSize;		// Offset 14 : aligned

	// Flags: bit #n is 1 if the value #n is used
	uint16					_Flags;				// Offset 16 : aligned

	// Internal flags
	uint16					_InternalFlags;		// Offset 18 : aligned

	// Vertex count in the buffer
	uint32					_NbVerts;			// Offset 20 : aligned

	// Capacity of the buffer
	uint32					_Capacity;

	// Vertex array
	std::vector<uint8>		_Verts;

	// Offset of each value
	uint16					_Offset[NumValue];

public:
	// \name Private. For Driver only.
	// @{
	CRefPtr<IVBDrvInfos>	DrvInfos;
	uint					getTouchFlags() const { return _InternalFlags&TouchedAll; }
	void					resetTouchFlags() {_InternalFlags &= ~TouchedAll;}
	// @}

public:

	/**
	  * Default constructor. Make an empty vertex buffer. No value, no vertex.
	  */
	CVertexBuffer(void);

	/**
	  * Copy constructor.
	  *  Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  */
	CVertexBuffer(const CVertexBuffer &vb);

	/**
	  * Destructor.
	  */
	~CVertexBuffer(void);

	/**
	  * Copy operator.
	  * Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  */
	CVertexBuffer			&operator=(const CVertexBuffer &vb);

	/**
	  * \name Standard values vertex buffer mgt.
	  * \name Thoses methods manage the vertex buffer (position, normal, colors and uv) with standard value.
	  */
	// @{

		/**
		  * Setup the vertex format using standard values. Do it before any other standard methods.
		  *
		  * Use one or several flag between : PositionFlag, WeightFlag, NormalFlag, PrimaryColorFlag, 
		  * SecondaryColorFlag, FogFlag, TexCoord0Flag, TexCoord1Flag, TexCoord2Flag, 
		  * TexCoord3Flag, TexCoord4Flag, TexCoord5Flag, TexCoord6Flag, TexCoord7Flag, PaletteSkinFlag
		  *
		  * If WeightFlag is specified, 4 float are used to setup the skinning value on 4 bones.
		  */
		bool					setVertexFormat (uint32 Flags);

		/**
		  * Return the vertex format used by the vertex buffer.
		  *
		  * Return one or several flags between : PositionFlag, WeightFlag, NormalFlag, PrimaryColorFlag, 
		  * SecondaryColorFlag, FogFlag, TexCoord0Flag, TexCoord1Flag, TexCoord2Flag, 
		  * TexCoord3Flag, TexCoord4Flag, TexCoord5Flag, TexCoord6Flag, TexCoord7Flag, PaletteSkinFlag
		  *
		  * If WeightFlag is specified, 4 float are used to setup the skinning value on 4 bones.
		  */
		uint16					getVertexFormat (void) const  { return(_Flags); };


		/// Returns the number of texture coordinate stages used by this vertex buffer
		uint					getNumTexCoordUsed() const;

		// It is an error (assert) to set a vertex component if not setuped in setVertexFormat().
		inline void				setVertexCoord(uint idx, float x, float y, float z);
		inline void				setVertexCoord(uint idx, const CVector &v);
		inline void				setNormalCoord(uint idx, const CVector &v);
		inline void				setTexCoord(uint idx, uint8 stage, float u, float v);
		inline void				setTexCoord(uint idx, uint8 stage, const CUV &uv);
		inline void				setColor(uint idx, CRGBA rgba);
		inline void				setSpecular(uint idx, CRGBA rgba);
		inline void				setWeight(uint idx, uint8 wgt, float w);
		inline void				setPaletteSkin(uint idx, CPaletteSkin ps);

		// It is an error (assert) to query a vertex offset of a vertex component not setuped in setVertexFormat().
		// NB: The Vertex offset is always 0.
		sint					getNormalOff() const {nlassert(_Flags & NormalFlag); return _Offset[Normal];}
		sint					getTexCoordOff(uint8 stage=0) const  {nlassert(_Flags & (TexCoord0Flag<<stage)); return _Offset[TexCoord0+stage]; }
  		sint					getColorOff() const {nlassert(_Flags & PrimaryColorFlag); return _Offset[PrimaryColor];}
		sint					getSpecularOff() const {nlassert(_Flags & SecondaryColorFlag); return _Offset[SecondaryColor];}
		/// NB: it is ensured that   WeightOff(i)==WeightOff(0)+i*sizeof(float).
		sint					getWeightOff(sint wgt) const {nlassert(_Flags & WeightFlag); return _Offset[Weight]+(wgt*sizeof(float));}
		sint					getPaletteSkinOff() const {nlassert(_Flags & PaletteSkin); return _Offset[PaletteSkin];}

		/** NB: the order of those methods follow the order in memory of the elements:
		 *	- VertexCoord
		 *	- NormalCoord
		 *	- TexCoord
		 *	- Color
		 *	- Specular
		 *	- Weight
		 *	- PaletteSkin
		 */
		void*					getVertexCoordPointer(uint idx=0);
		void*					getNormalCoordPointer(uint idx=0);
		void*					getTexCoordPointer(uint idx=0, uint8 stage=0);
		void*					getColorPointer(uint idx=0);
		void*					getSpecularPointer(uint idx=0);
		void*					getWeightPointer(uint idx=0, uint8 wgt=0);
		void*					getPaletteSkinPointer(uint idx=0);

		const void*				getVertexCoordPointer(uint idx=0) const ;
		const void*				getNormalCoordPointer(uint idx=0) const ;
		const void*				getTexCoordPointer(uint idx=0, uint8 stage=0) const ;
		const void*				getColorPointer(uint idx=0) const ;
		const void*				getSpecularPointer(uint idx=0) const ;
		const void*				getWeightPointer(uint idx=0, uint8 wgt=0) const ;
		const void*				getPaletteSkinPointer(uint idx=0) const ;
	// @}

	/**
	  * \name Extended values vertex buffer mgt.
	  * \name Thoses methods manage the vertex buffer with extended value.
	  * \name This is usable only with OpenGL vertex_program or DX8 vertex shaders.
	  */
	// @{

		/**
		  * get the corresponding TValue according to the number of vertex attribute wanted (v[0], v[1] ...)
		  *	Warning!: (TValue)valueNumber != getValueIdByNumberEx(valueNumber).
		  *
		  * \param valueNumber is the value index (0..15) you want to know the valueId.
		  */
		static TValue		getValueIdByNumberEx (uint valueNumber);

		/**
		  * Clear all value in the vertex buffer. After this call, call addValue for each value you want in your vertex
		  * buffer then call initEx() to init the vertex buffer.
		  */
		void				clearValueEx ();		

		/**
		  * Add a value in the vertex buffer. After this call, call initEx() to init the vertex buffer.
		  *
		  * \param valueId is the value id to setup.
		  * \param type is the type used for this value.
		  */
		void				addValueEx (TValue valueId, TType type);

		/// Test if the given value is present in the vertex buffer
		bool				hasValueEx(TValue valueId) const;		

		/**
		  * Init the vertex buffer in extended mode.
		  */
		void				initEx ();

		/**
		  * Setup values. nlassert are raised if wrong value type is setuped.
		  */
		inline void			setValueDouble1Ex (TValue valueId, uint idx, double value);
		inline void			setValueDouble2Ex (TValue valueId, uint idx, double x, double y);
		inline void			setValueDouble3Ex (TValue valueId, uint idx, double x, double y, double z);
		inline void			setValueDouble3Ex (TValue valueId, uint idx, const CVectorD& vector);
		inline void			setValueDouble4Ex (TValue valueId, uint idx, double x, double y, double z, double w);
		inline void			setValueFloat1Ex (TValue valueId, uint idx, float value);
		inline void			setValueFloat2Ex (TValue valueId, uint idx, float x, float y);
		inline void			setValueFloat3Ex (TValue valueId, uint idx, float x, float y, float z);
		inline void			setValueFloat3Ex (TValue valueId, uint idx, const CVector& vector);
		inline void			setValueFloat4Ex (TValue valueId, uint idx, float x, float y, float z, float w);
		inline void			setValueShort1Ex (TValue valueId, uint idx, uint16 value);
		inline void			setValueShort2Ex (TValue valueId, uint idx, uint16 x, uint16 y);
		inline void			setValueShort3Ex (TValue valueId, uint idx, uint16 x, uint16 y, uint16 z);
		inline void			setValueShort4Ex (TValue valueId, uint idx, uint16 x, uint16 y, uint16 z, uint16 w);
		inline void			setValueUChar4Ex (TValue valueId, uint idx, CRGBA rgba);

		/**
		  * Get writable value pointer.
		  */
		void*				getValueEx (TValue valueId, uint idx=0) { nlassert (_Flags & (1<<valueId));	return (void*)((&(*_Verts.begin()))+idx*_VertexSize+getValueOffEx (valueId)); }

		/**
		  * Get readable value pointer.
		  */
		const void*			getValueEx (TValue valueId, uint idx=0) const { nlassert (_Flags & (1<<valueId));	return (void*)((&(*_Verts.begin()))+idx*_VertexSize+getValueOffEx (valueId)); };

		/**
		  * Get value offset.
		  */
		sint				getValueOffEx (TValue valueId) const { nlassert(_Flags & (1<<valueId)); return _Offset[valueId]; };

	// @}

	/** 
	  * Set the number of active vertices. It enlarge capacity, if needed.
	  */
	void					setNumVertices(uint32 n);
	
	/** 
	  * Get the number of active vertices.
	  */
	uint32					getNumVertices(void) const  { return(_NbVerts); }

	/**
	  * Reset all the vertices from memory (contReset()), so that capacity() == getNumVertices() == 0.
	  */
	void					deleteAllVertices();

	/**
	  * Reserve space for nVerts vertices. You are allowed to write your vertices on this space.
	  */
	void					reserve(uint32 nVerts);
	
	/**
	  * Return the number of vertices reserved.
	  */
	uint32					capacity() { return _Capacity; }

	/**
	  * Return the size of a vertex
	  */
	uint16					getVertexSize (void) const { return(_VertexSize); }

	/**
	  * Return the type of a value
	  */
	TType					getValueType (uint value) const { nlassert (value<NumValue); return((TType)_Type[value]); }

	/**
	  * Return the type array
	  */
	const uint8				*getValueTypePointer () const { return _Type; }

	/**
	  * Return number of weight value in vertices
	  */
	uint8					getNumWeight () const;

	void		serial(NLMISC::IStream &f);


	/// \name Lod VB serialisation.
	// @{
	/// just read/write the VertexFormat of the VB, number of vertices .... If read, VB is resized to numVertices.
	void		serialHeader(NLMISC::IStream &f);
	/** just read/write a subset of the vertex buffer. NB: because a version is serialised by subset, you must read 
	 * same subset of vertices you have saved. (can't write 0-10, 10-20, and after load 0-5, 5-20).
	 */
	void		serialSubset(NLMISC::IStream &f, uint vertexStart, uint vertexEnd);
	// @}


private:
	/// Old version serialisation. V0 and V1.
	void		serialOldV1Minus(NLMISC::IStream &f, sint ver);

	/// Translate old flags
	uint16		remapV2Flags (uint32 oldFlags, uint& weightCount);
};


//////////////////////////////////////
// implementation of inline methods //
//////////////////////////////////////
// --------------------------------------------------

inline void CVertexBuffer::setVertexCoord(uint idx, float x, float y, float z)
{
	float*	ptr;

	nlassert (_Flags & PositionFlag);
	nlassert (_Type[Position]==Float3);

	ptr=(float*)(&_Verts[idx*_VertexSize]);
	*ptr=x;
	ptr++;
	*ptr=y;
	ptr++;
	*ptr=z;
}

// --------------------------------------------------

inline void CVertexBuffer::setVertexCoord(uint idx, const CVector &v)
{
	uint8*	ptr;

	nlassert (_Flags & PositionFlag);
	nlassert (_Type[Position]==Float3);

	ptr=&_Verts[idx*_VertexSize];
	memcpy(ptr, &(v.x), 3*sizeof(float));
}

// --------------------------------------------------

inline void CVertexBuffer::setNormalCoord(uint idx, const CVector &v)
{
	uint8*	ptr;

	nlassert (_Flags & NormalFlag);
	nlassert (_Type[Normal]==Float3);

	ptr=&_Verts[idx*_VertexSize];
	ptr+=_Offset[Normal];
	memcpy(ptr, &(v.x), 3*sizeof(float));
}

// --------------------------------------------------

inline void CVertexBuffer::setColor(uint idx, CRGBA rgba)
{
	uint8*	ptr;
	CRGBA	*pCol;

	nlassert(_Flags & PrimaryColorFlag);
	nlassert (_Type[PrimaryColor]==UChar4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[PrimaryColor];
	pCol= (CRGBA*)ptr;
	*pCol= rgba;
}

// --------------------------------------------------

inline void CVertexBuffer::setSpecular(uint idx, CRGBA rgba)
{
	uint8*	ptr;
	CRGBA	*pCol;

	nlassert(_Flags & SecondaryColorFlag);
	nlassert (_Type[SecondaryColor]==UChar4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[SecondaryColor];
	pCol= (CRGBA*)ptr;
	*pCol= rgba;
}

// --------------------------------------------------

inline void CVertexBuffer::setTexCoord(uint idx, uint8 stage, float u, float v)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(stage<MaxStage);
	nlassert(_Flags & (TexCoord0Flag<<stage));
	nlassert (_Type[TexCoord0+stage]==Float2);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[TexCoord0+stage];
	ptrf=(float*)ptr;
	*ptrf=u;
	ptrf++;
	*ptrf=v;
}

// --------------------------------------------------

inline void	CVertexBuffer::setTexCoord(uint idx, uint8 stage, const CUV &uv)
{
	uint8*	ptr;
	CUV*	ptruv;

	nlassert(stage<MaxStage);
	nlassert(_Flags & (TexCoord0Flag<<stage));
	nlassert (_Type[TexCoord0+stage]==Float2);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[TexCoord0+stage];
	ptruv=(CUV*)ptr;
	*ptruv=uv;
}


// --------------------------------------------------

inline void CVertexBuffer::setWeight(uint idx, uint8 wgt, float w)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(wgt<MaxWeight);
	nlassert(_Flags & (WeightFlag));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[Weight]+sizeof(float)*wgt;
	ptrf=(float*)ptr;
	*ptrf=w;
}

// --------------------------------------------------

inline void	CVertexBuffer::setPaletteSkin(uint idx, CPaletteSkin ps)
{
	uint8*	ptr;
	CPaletteSkin	*pPalSkin;

	nlassert ( (_Flags & PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag);
	nlassert (_Type[PaletteSkin]==UChar4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[PaletteSkin];
	pPalSkin= (CPaletteSkin*)ptr;
	*pPalSkin= ps;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueDouble1Ex (TValue valueId, uint idx, double value)
{
	uint8*	ptr;
	double*	ptrd;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Double4)||(_Type[valueId]==Double3)||(_Type[valueId]==Double2)||(_Type[valueId]==Double1));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrd=(double*)ptr;
	*ptrd=value;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueDouble2Ex (TValue valueId, uint idx, double x, double y)
{
	uint8*	ptr;
	double*	ptrd;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Double4)||(_Type[valueId]==Double3)||(_Type[valueId]==Double2));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrd=(double*)ptr;
	ptrd[0]=x;
	ptrd[1]=y;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueDouble3Ex (TValue valueId, uint idx, double x, double y, double z)
{
	uint8*	ptr;
	double*	ptrd;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Double4)||(_Type[valueId]==Double3));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrd=(double*)ptr;
	ptrd[0]=x;
	ptrd[1]=y;
	ptrd[2]=z;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueDouble3Ex (TValue valueId, uint idx, const NLMISC::CVectorD& vector)
{
	uint8*	ptr;
	double*	ptrd;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Double4)||(_Type[valueId]==Double3));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrd=(double*)ptr;
	memcpy (ptrd, &vector, sizeof(double)*3);
}

// --------------------------------------------------

inline void CVertexBuffer::setValueDouble4Ex (TValue valueId, uint idx, double x, double y, double z, double w)
{
	uint8*	ptr;
	double*	ptrd;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert(_Type[valueId]==Double4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrd=(double*)ptr;
	ptrd[0]=x;
	ptrd[1]=y;
	ptrd[2]=z;
	ptrd[3]=w;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueFloat1Ex (TValue valueId, uint idx, float value)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Float4)||(_Type[valueId]==Float3)||(_Type[valueId]==Float2)||(_Type[valueId]==Float1));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrf=(float*)ptr;
	*ptrf=value;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueFloat2Ex (TValue valueId, uint idx, float x, float y)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Float4)||(_Type[valueId]==Float3)||(_Type[valueId]==Float2));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrf=(float*)ptr;
	ptrf[0]=x;
	ptrf[1]=y;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueFloat3Ex (TValue valueId, uint idx, float x, float y, float z)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Float4)||(_Type[valueId]==Float3));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrf=(float*)ptr;
	ptrf[0]=x;
	ptrf[1]=y;
	ptrf[2]=z;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueFloat3Ex (TValue valueId, uint idx, const NLMISC::CVector& vector)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Float4)||(_Type[valueId]==Float3));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrf=(float*)ptr;
	memcpy (ptrf, &vector, sizeof(float)*3);
}

// --------------------------------------------------

inline void CVertexBuffer::setValueFloat4Ex (TValue valueId, uint idx, float x, float y, float z, float w)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert(_Type[valueId]==Float4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrf=(float*)ptr;
	ptrf[0]=x;
	ptrf[1]=y;
	ptrf[2]=z;
	ptrf[3]=w;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueShort1Ex (TValue valueId, uint idx, uint16 value)
{
	uint8*	ptr;
	uint16*	ptri;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Short4)||(_Type[valueId]==Short3)||(_Type[valueId]==Short2)||(_Type[valueId]==Short1));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptri=(uint16*)ptr;
	*ptri=value;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueShort2Ex (TValue valueId, uint idx, uint16 x, uint16 y)
{
	uint8*	ptr;
	uint16*	ptri;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Short4)||(_Type[valueId]==Short3)||(_Type[valueId]==Short2));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptri=(uint16*)ptr;
	ptri[0]=x;
	ptri[1]=y;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueShort3Ex (TValue valueId, uint idx, uint16 x, uint16 y, uint16 z)
{
	uint8*	ptr;
	uint16*	ptri;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert((_Type[valueId]==Short4)||(_Type[valueId]==Short3));

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptri=(uint16*)ptr;
	ptri[0]=x;
	ptri[1]=y;
	ptri[2]=z;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueShort4Ex (TValue valueId, uint idx, uint16 x, uint16 y, uint16 z, uint16 w)
{
	uint8*	ptr;
	uint16*	ptri;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert(_Type[valueId]==Short4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptri=(uint16*)ptr;
	ptri[0]=x;
	ptri[1]=y;
	ptri[2]=z;
	ptri[3]=w;
}

// --------------------------------------------------

inline void CVertexBuffer::setValueUChar4Ex (TValue valueId, uint idx, CRGBA rgba)
{
	uint8*	ptr;
	CRGBA*	ptrr;

	nlassert(valueId<NumValue);
	nlassert(_Flags & (1<<(uint)valueId));
	nlassert(_Type[valueId]==UChar4);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[valueId];
	ptrr=(CRGBA*)ptr;
	*ptrr=rgba;
}

// --------------------------------------------------

} // NL3D


#endif // NL_VERTEX_BUFFER_H

/* End of vertex_buffer.h */
