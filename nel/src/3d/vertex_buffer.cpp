/** \file vertex_buffer.cpp
 * Vertex Buffer implementation
 *
 * $Id: vertex_buffer.cpp,v 1.33 2002/03/14 18:19:48 vizerie Exp $
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

#include "std3d.h"

#include "3d/vertex_buffer.h"
#include "nel/misc/vector.h"
#include "3d/driver.h"
using namespace NLMISC;


namespace NL3D
{

// --------------------------------------------------

const uint CVertexBuffer::SizeType[NumType]=
{
	1*sizeof(double),
	1*sizeof(float),
	1*sizeof(short),
	2*sizeof(double),
	2*sizeof(float),
	2*sizeof(short),
	3*sizeof(double),
	3*sizeof(float),
	3*sizeof(short),
	4*sizeof(double),
	4*sizeof(float),
	4*sizeof(short),
	4*sizeof(char),
};

// --------------------------------------------------

const CVertexBuffer::TType CVertexBuffer::DefaultValueType[NumValue]=
{
	Float3,		// Position
	Float3,		// Normal
	Float2,		// TexCoord0
	Float2,		// TexCoord1
	Float2,		// TexCoord2
	Float2,		// TexCoord3
	Float2,		// TexCoord4
	Float2,		// TexCoord5
	Float2,		// TexCoord6
	Float2,		// TexCoord7
	UChar4,		// Primary color
	UChar4,		// Secondary color
	Float4,		// 4 Weights
	UChar4,		// PaletteSkin
	Float1,		// Fog
	Float1,		// Empty
};

// --------------------------------------------------

CVertexBuffer::CVertexBuffer()
{
	_Flags = 0;
	_Capacity = 0;
	_NbVerts = 0;
	_InternalFlags = 0;
	_VertexSize = 0;
}

// --------------------------------------------------

CVertexBuffer::CVertexBuffer(const CVertexBuffer &vb)
{
	_Flags = 0;
	_Capacity = 0;
	_NbVerts = 0;
	_VertexSize = 0;
	operator=(vb);
}

// --------------------------------------------------

CVertexBuffer::~CVertexBuffer()
{
	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

// --------------------------------------------------

CVertexBuffer	&CVertexBuffer::operator=(const CVertexBuffer &vb)
{
	// Single value
	_VertexSize = vb._VertexSize;
	_Flags = vb._Flags;
	_InternalFlags = vb._InternalFlags;
	_NbVerts = vb._NbVerts;
	_Capacity = vb._Capacity;
	_Verts = vb._Verts;

	// Arraies
	for (uint value=0; value<NumValue; value++)
	{
		_Offset[value]= vb._Offset[value];
		_Type[value]= vb._Type[value];
	}

	// Set touch flags
	_InternalFlags |= TouchedAll;

	return *this;
}

// --------------------------------------------------

bool CVertexBuffer::setVertexFormat(uint32 flags)
{
	uint	i;

	// Clear extended values 
	clearValueEx ();

	// Position ?
	if (flags & PositionFlag)
	{
		// Add a standard position value
		addValueEx (Position, Float3);
	}

	// Normal ?
	if (flags & NormalFlag)
	{
		// Add a standard normal value
		addValueEx (Normal, Float3);
	}

	// For each uv values
	for(i=0 ; i<MaxStage ; i++)
	{
		// UV ?
		if (flags & (TexCoord0Flag<<i))
		{
			// Add a standard uv value
			addValueEx ((TValue)(TexCoord0+i), Float2);
		}
	}

	// Fog ?
	if (flags & FogFlag)
	{
		// Add a standard primary color value
		addValueEx (Fog, Float1);
	}

	// Primary color ?
	if (flags & PrimaryColorFlag)
	{
		// Add a standard primary color value
		addValueEx (PrimaryColor, UChar4);
	}

	// Secondary color ?
	if (flags & SecondaryColorFlag)
	{
		// Add a standard primary color value
		addValueEx (SecondaryColor, UChar4);
	}

	// Weight ?
	if (flags & WeightFlag)
	{
		// Add a standard primary color value
		addValueEx (Weight, Float4);
	}

	// Palette skin ?
	if ((flags & PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag)
	{
		// Add a standard primary color value
		addValueEx (PaletteSkin, UChar4);
	}

	// Compute the vertex buffer
	initEx ();

	return (true);
}

// --------------------------------------------------

CVertexBuffer::TValue CVertexBuffer::getValueIdByNumberEx (uint valueNumber)
{
	// See NV_vertex_program spec, or driver_opengl_vertex.cpp:: GLVertexAttribIndex.
	static	TValue	lut[16]= {
		Position,
		Weight,
		Normal,
		PrimaryColor,
		SecondaryColor,
		Fog,
		PaletteSkin,
		Empty,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
	};

	return lut[valueNumber];
}

// --------------------------------------------------

void CVertexBuffer::clearValueEx ()
{
	// Reset format flags
	_Flags=0;
}

// --------------------------------------------------

void CVertexBuffer::addValueEx (TValue valueId, TType type)
{
	// Reset format flags
	_Flags |= 1<<valueId;

	// Set the type
	_Type[valueId]=(uint8)type;
}

// --------------------------------------------------

bool CVertexBuffer::hasValueEx(TValue valueId) const
{
	return (_Flags & (1 << valueId)) != 0;
}

// --------------------------------------------------

void CVertexBuffer::initEx ()
{
	// Reset internal flag
	_InternalFlags=TouchedAll;

	// Calc vertex size and set value's offset
	_VertexSize=0;
	for (uint value=0; value<NumValue; value++)
	{
		// Value used ?
		if (_Flags&(1<<value))
		{
			// Set offset
			_Offset[value]=_VertexSize;

			// New size
			_VertexSize+=SizeType[_Type[value]];
		}
	}

	// Reset number of vertices
	_NbVerts=0;

	// Compute new capacity
	if (_VertexSize)
		_Capacity = _Verts.size()/_VertexSize;
	else
		_Capacity = 0;
}

// --------------------------------------------------

void CVertexBuffer::reserve(uint32 n)
{
	_Verts.resize(n*_VertexSize);
	_Capacity= n;
}

// --------------------------------------------------

void CVertexBuffer::setNumVertices(uint32 n)
{
	if(_Capacity<n)
	{
		reserve(n);
	}
	if(_NbVerts != n)
	{
		_InternalFlags |= TouchedNumVertices;
		_NbVerts=n;
	}
}

// --------------------------------------------------

void	CVertexBuffer::deleteAllVertices()
{
	// free memory.
	contReset(_Verts);
	_Capacity= 0;
	if(_NbVerts!=0)
	{
		_NbVerts=0;
		_InternalFlags |= TouchedNumVertices;
	}
}

// --------------------------------------------------

void* CVertexBuffer::getVertexCoordPointer(uint idx)
{
	uint8*	ptr;

	ptr=&(*_Verts.begin());
	ptr+=(idx*_VertexSize);
	return((void*)ptr);
}

// --------------------------------------------------

void* CVertexBuffer::getNormalCoordPointer(uint idx)
{
	uint8*	ptr;

	if ( !(_Flags & NormalFlag) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[Normal];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

void* CVertexBuffer::getColorPointer(uint idx)
{
	uint8*	ptr;

	if ( !(_Flags & PrimaryColorFlag) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[PrimaryColor];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

void* CVertexBuffer::getSpecularPointer(uint idx)
{
	uint8*	ptr;

	if ( !(_Flags & SecondaryColorFlag) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[SecondaryColor];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

void* CVertexBuffer::getTexCoordPointer(uint idx, uint8 stage)
{
	uint8*	ptr;

	if ( !(_Flags & (TexCoord0Flag<<stage)) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[TexCoord0+stage];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

void* CVertexBuffer::getWeightPointer(uint idx, uint8 wgt)
{
	uint8*	ptr;

	nlassert(wgt<MaxWeight);
	if( !(_Flags & WeightFlag))
		return NULL;

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[Weight]+wgt*sizeof(float);

	return ptr;
}

// --------------------------------------------------

void* CVertexBuffer::getPaletteSkinPointer(uint idx)
{
	uint8*	ptr;

	if ( (_Flags & PaletteSkinFlag) != CVertexBuffer::PaletteSkinFlag )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[PaletteSkin];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------


////////////////////
// const versions //
////////////////////


// --------------------------------------------------

const void* CVertexBuffer::getVertexCoordPointer(uint idx) const 
{
	const uint8*	ptr;

	ptr=&(*_Verts.begin());
	ptr+=(idx*_VertexSize);
	return((void*)ptr);
}

// --------------------------------------------------

const void* CVertexBuffer::getNormalCoordPointer(uint idx) const
{
	const uint8*	ptr;

	if ( !(_Flags & NormalFlag) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[Normal];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

const void* CVertexBuffer::getColorPointer(uint idx) const
{
	const uint8*	ptr;

	if ( !(_Flags & PrimaryColorFlag) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[PrimaryColor];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

const void* CVertexBuffer::getSpecularPointer(uint idx) const
{
	const uint8*	ptr;

	if ( !(_Flags & SecondaryColorFlag) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[SecondaryColor];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

const void* CVertexBuffer::getTexCoordPointer(uint idx, uint8 stage) const
{
	const uint8*	ptr;

	if ( !(_Flags & (TexCoord0Flag<<stage)) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[TexCoord0+stage];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

const void* CVertexBuffer::getWeightPointer(uint idx, uint8 wgt) const
{
	const uint8*	ptr;

	nlassert(wgt<MaxWeight);
	if( !(_Flags & WeightFlag))
		return NULL;

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_Offset[Weight]+wgt*sizeof(float);

	return ptr;
}

// --------------------------------------------------

const void* CVertexBuffer::getPaletteSkinPointer(uint idx) const
{
	const uint8*	ptr;

	if ( (_Flags & PaletteSkinFlag) != CVertexBuffer::PaletteSkinFlag )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_Offset[PaletteSkin];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

// --------------------------------------------------

uint16		CVertexBuffer::remapV2Flags (uint32 oldFlags, uint& weightCount)
{
	// Old flags
	const uint32 OLD_IDRV_VF_XYZ = 0x00000001;
	const uint32 OLD_IDRV_VF_W0 = 0x00000002;
	const uint32 OLD_IDRV_VF_W1 = 0x00000004;
	const uint32 OLD_IDRV_VF_W2 = 0x00000008;
	const uint32 OLD_IDRV_VF_W3 = 0x00000010;
	const uint32 OLD_IDRV_VF_NORMAL = 0x00000020;
	const uint32 OLD_IDRV_VF_COLOR = 0x00000040;
	const uint32 OLD_IDRV_VF_SPECULAR = 0x00000080;
	const uint32 OLD_IDRV_VF_UV0 = 0x00000100;
	const uint32 OLD_IDRV_VF_UV1 = 0x00000200;
	const uint32 OLD_IDRV_VF_UV2 = 0x00000400;
	const uint32 OLD_IDRV_VF_UV3 = 0x00000800;
	const uint32 OLD_IDRV_VF_UV4 = 0x00001000;
	const uint32 OLD_IDRV_VF_UV5 = 0x00002000;
	const uint32 OLD_IDRV_VF_UV6 = 0x00004000;
	const uint32 OLD_IDRV_VF_UV7 = 0x00008000;
	const uint32 OLD_IDRV_VF_PALETTE_SKIN = 0x00010000 | OLD_IDRV_VF_W0 | OLD_IDRV_VF_W1 | OLD_IDRV_VF_W2 | OLD_IDRV_VF_W3;

	// Old Flags
	uint16 newFlags=0;

	// Number of weight values
	weightCount=0;

	// Remap the flags
	if (oldFlags&OLD_IDRV_VF_XYZ)
		newFlags|=PositionFlag;
	if (oldFlags&OLD_IDRV_VF_NORMAL)
		newFlags|=NormalFlag;
	if (oldFlags&OLD_IDRV_VF_COLOR)
		newFlags|=PrimaryColorFlag;
	if (oldFlags&OLD_IDRV_VF_SPECULAR)
		newFlags|=SecondaryColorFlag;
	if (oldFlags&OLD_IDRV_VF_UV0)
		newFlags|=TexCoord0Flag;
	if (oldFlags&OLD_IDRV_VF_UV1)
		newFlags|=TexCoord1Flag;
	if (oldFlags&OLD_IDRV_VF_UV2)
		newFlags|=TexCoord2Flag;
	if (oldFlags&OLD_IDRV_VF_UV3)
		newFlags|=TexCoord3Flag;
	if (oldFlags&OLD_IDRV_VF_UV4)
		newFlags|=TexCoord4Flag;
	if (oldFlags&OLD_IDRV_VF_UV5)
		newFlags|=TexCoord5Flag;
	if (oldFlags&OLD_IDRV_VF_UV6)
		newFlags|=TexCoord6Flag;
	if (oldFlags&OLD_IDRV_VF_UV7)
		newFlags|=TexCoord7Flag;
	if (oldFlags&OLD_IDRV_VF_W0)
	{
		weightCount=1;
		newFlags|=WeightFlag;
	}
	if (oldFlags&OLD_IDRV_VF_W1)
	{
		weightCount=2;
		newFlags|=WeightFlag;
	}
	if (oldFlags&OLD_IDRV_VF_W2)
	{
		weightCount=3;
		newFlags|=WeightFlag;
	}
	if (oldFlags&OLD_IDRV_VF_W3)
	{
		weightCount=4;
		newFlags|=WeightFlag;
	}
	if (oldFlags&(OLD_IDRV_VF_PALETTE_SKIN))
		newFlags|=PaletteSkinFlag;

	// Return the new flags
	return newFlags;
}

// --------------------------------------------------

void		CVertexBuffer::serialOldV1Minus(NLMISC::IStream &f, sint ver)
{
	/*
	Version 1:
		- PaletteSkin version.
	Version 0:
		- base verison.
	*/

	// old Flags
	uint32 oldFlags;

	// Serial VBuffers format/size.
	//=============================
	f.serial(oldFlags);

	// Remap the flags
	uint weightCount;
	uint16 newFlags=remapV2Flags (oldFlags, weightCount);

	// Must be reading
	nlassert (f.isReading());

	// Set default value type
	uint i;
	for (i=0; i<NumValue; i++)
		_Type[i]=DefaultValueType[i];

	uint32 nbVert;	// Read only
	f.serial(nbVert);
	reserve(0);
	setVertexFormat(newFlags);
	setNumVertices(nbVert);
	// All other infos (but _Verts) are computed by setVertexFormat() and setNumVertices().

	// Weight count ?
	switch (weightCount)
	{
	case 1:
		_Type[Weight]=Float1;
		break;
	case 2:
		_Type[Weight]=Float2;
		break;
	case 3:
		_Type[Weight]=Float3;
		break;
	case 4:
		_Type[Weight]=Float4;
		break;
	}

	// Serial VBuffers components.
	//============================
	for(sint id=0;id<(sint)_NbVerts;id++)
	{
		// XYZ.
		if(_Flags & PositionFlag)
		{
			CVector		&vert= *(CVector*)getVertexCoordPointer(id);
			f.serial(vert);
		}
		// Normal
		if(_Flags & NormalFlag)
		{
			CVector		&norm= *(CVector*)getNormalCoordPointer(id);
			f.serial(norm);
		}
		// Uvs.
		for(i=0;i<MaxStage;i++)
		{
			if(_Flags & (TexCoord0Flag<<i))
			{
				CUV		&uv= *(CUV*)getTexCoordPointer(id, i);
				f.serial(uv);
			}
		}
		// Color.
		if(_Flags & PrimaryColorFlag)
		{
			CRGBA		&col= *(CRGBA*)getColorPointer(id);
			f.serial(col);
		}
		// Specular.
		if(_Flags & SecondaryColorFlag)
		{
			CRGBA		&col= *(CRGBA*)getSpecularPointer(id);
			f.serial(col);
		}
		// Weights
		for(i=0;i<weightCount;i++)
		{
			// Weight channel available ?
			float	&w= *(float*)getWeightPointer(id, i);
			f.serial(w);
		}
		// CPaletteSkin (version 1+ only).
		if((ver>=1) && ((_Flags & PaletteSkinFlag) == CVertexBuffer::PaletteSkinFlag) )
		{
			CPaletteSkin	&ps= *(CPaletteSkin*)getPaletteSkinPointer(id);
			f.serial(ps);
		}

	}

	// Set touch flags
	_InternalFlags = 0;
	if(f.isReading())
		_InternalFlags |= TouchedAll;
}

// --------------------------------------------------

void		CVertexBuffer::serial(NLMISC::IStream &f)
{
	/*
	Version 2:
		- cut to use serialHeader() serialSubset().
	Version 1:
		- PaletteSkin version.
	Version 0:
		- base verison.
	*/
	sint	ver= f.serialVersion(2);

	if (ver<2)
	{
		// old serial method
		serialOldV1Minus(f, ver);
	}
	else
	{
		// read write the header of the VBuffer.
		serialHeader(f);

		// read write the entire subset.
		serialSubset(f, 0, _NbVerts);
	}
}

// --------------------------------------------------

void		CVertexBuffer::serialHeader(NLMISC::IStream &f)
{
	/*
	Version 1:
		- Extended vertex format management.
	Version 0:
		- base verison of the header serialisation.
	*/
	sint	ver= f.serialVersion(1);

	// Serial VBuffers format/size.
	//=============================

	// Flags
	uint16 flags=_Flags;

	if (ver<1)
	{
		// Must be reading
		nlassert (f.isReading());

		// Serial old flags
		uint32 oldFlags;
		f.serial(oldFlags);

		// Remap flags
		uint weightCount;
		flags=remapV2Flags (oldFlags, weightCount);

		// Set default value type
		for (uint i=0; i<NumValue; i++)
			_Type[i]=DefaultValueType[i];

		// weight count ?
		switch (weightCount)
		{
		case 1:
			_Type[Weight]=Float1;
			break;
		case 2:
			_Type[Weight]=Float2;
			break;
		case 3:
			_Type[Weight]=Float3;
			break;
		case 4:
			_Type[Weight]=Float4;
			break;
		}
	}
	else
	{
		// Serial new vertex flags
		f.serial(flags);

		// Serial type of values
		for (uint i=0; i<NumValue; i++)
			f.serial (_Type[i]);
	}

	// Serial nb vertices
	uint32 nbVerts=_NbVerts;
	f.serial(nbVerts);

	if(f.isReading())
	{
		reserve(0);

		// Init vertex format setup
		clearValueEx ();

		// Init vertex format
		for (uint i=0; i<NumValue; i++)
		{
			// Setup this value ?
			if (flags&(1<<i))
			{
				// Add a value
				addValueEx ((TValue)i, (TType)_Type[i]);
			}
		}

		// Build final vertex format
		initEx ();

		// Set num of vertices
		setNumVertices(nbVerts);
	}
	// All other infos (but _Verts) are computed by initEx() and setNumVertices().
}


// --------------------------------------------------
uint	CVertexBuffer::	getNumTexCoordUsed() const
{	
	for (sint k = (MaxStage - 1); k >= 0; --k)
	{
		if (_Flags & (TexCoord0Flag << k))  return (uint) (k + 1);
	}
	return 0;
}

// --------------------------------------------------

uint8		CVertexBuffer::getNumWeight () const
{
	// Num weight
	switch (_Type[Weight])
	{
	case Float1:
		return 1;
	case Float2:
		return 2;
	case Float3:
		return 3;
	case Float4:
		return 4;
	}

	// No weight
	return 0;
}

// --------------------------------------------------

void		CVertexBuffer::serialSubset(NLMISC::IStream &f, uint vertexStart, uint vertexEnd)
{
	/*
	Version 1:
		- weight is 4 float in standard format.
	Version 0:
		- base verison of a vbuffer subset serialisation.
	*/
	sint	ver= f.serialVersion(1);


	// Serial VBuffers components.
	//============================
	nlassert(vertexStart<_NbVerts);
	nlassert(vertexEnd<=_NbVerts);
	for(uint id=vertexStart; id<vertexEnd; id++)
	{
		// For each value
		for (uint value=0; value<NumValue; value++)
		{
			// Value used ?
			if (_Flags&(1<<value))
			{
				// Get the pointer on it
				void *ptr=getValueEx ((TValue)value, id);
				f.serialBuffer ((uint8*)ptr, SizeType[_Type[value]]);
			}
		}
	}

	// Set touch flags
	if(f.isReading())
		_InternalFlags |= TouchedAll;
}

// --------------------------------------------------

// CPaletteSkin serial (no version chek).
void	CPaletteSkin::serial(NLMISC::IStream &f)
{
	f.serial(MatrixId[0], MatrixId[1], MatrixId[2], MatrixId[3]);
}

// --------------------------------------------------

IVBDrvInfos::~IVBDrvInfos()
{
	_Driver->removeVBDrvInfoPtr(_DriverIterator);
}

// --------------------------------------------------

}
