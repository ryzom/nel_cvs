/** \file vertex_buffer.cpp
 * Vertex Buffer implementation
 *
 * $Id: vertex_buffer.cpp,v 1.19 2001/06/12 08:21:22 vizerie Exp $
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

#include "nel/3d/vertex_buffer.h"
#include "nel/misc/vector.h"
#include "nel/3d/driver.h"
using namespace NLMISC;


namespace NL3D
{

// --------------------------------------------------

CVertexBuffer::CVertexBuffer()
{
	_Flags=0;
	_Capacity=0;
	_NbVerts=0;

	_Touch= 0;
}


CVertexBuffer::CVertexBuffer(const CVertexBuffer &vb)
{
	_Flags=0;
	_Capacity=0;
	_NbVerts=0;
	operator=(vb);
}


CVertexBuffer::~CVertexBuffer()
{
	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

CVertexBuffer	&CVertexBuffer::operator=(const CVertexBuffer &vb)
{
	_Flags= vb._Flags;
	_VertexSize= vb._VertexSize;
	_NbVerts= vb._NbVerts;
	_Capacity= vb._Capacity;
	_Verts= vb._Verts;

	_NormalOff= vb._NormalOff;
	_RGBAOff= vb._RGBAOff;
	_SpecularOff= vb._SpecularOff;
	_PaletteSkinOff= vb._PaletteSkinOff;

	sint i;
	for(i=0;i<IDRV_VF_MAXW;i++)
		_WOff[i]= vb._WOff[i];
	for(i=0;i<IDRV_VF_MAXSTAGES;i++)
		_UVOff[i]= vb._UVOff[i];

	// Set touch flags
	_Touch= IDRV_VF_TOUCHED_ALL;

	return *this;
}


// --------------------------------------------------

bool CVertexBuffer::setVertexFormat(uint32 flags)
{
	uint	i;
	uint	offset;

	_Touch|= IDRV_VF_TOUCHED_VERTEX_FORMAT;

	uint8 oldVertexSize = _VertexSize ;
	_VertexSize=0;
	offset=0;
	_Flags=0;
	if (flags & IDRV_VF_XYZ)
	{
		_Flags|=IDRV_VF_XYZ;
		_VertexSize+=3*sizeof(float);
	}
	for(i=0 ; i<IDRV_VF_MAXW ; i++)
	{
		if (flags & IDRV_VF_W[i])
		{
			_Flags|=IDRV_VF_W[i];
			_WOff[i]=_VertexSize;
			_VertexSize+=sizeof(float);			
		}
	}
	if (flags & IDRV_VF_NORMAL)
	{
		_Flags|=IDRV_VF_NORMAL;
		_NormalOff=_VertexSize;
		_VertexSize+=3*sizeof(float);
	}
	if (flags & IDRV_VF_COLOR)
	{
		_Flags|=IDRV_VF_COLOR;
		_RGBAOff=_VertexSize;
		_VertexSize+=4*sizeof(uint8);
	}
	if (flags & IDRV_VF_SPECULAR)
	{
		_Flags|=IDRV_VF_SPECULAR;
		_SpecularOff=_VertexSize;
		_VertexSize+=3*sizeof(uint8);
	}
	for(i=0 ; i<IDRV_VF_MAXSTAGES ; i++)
	{
		if (flags & IDRV_VF_UV[i])
		{
			_Flags|=IDRV_VF_UV[i];
			_UVOff[i]=_VertexSize;
			_VertexSize+=2*sizeof(float);
		}
	}
	if ( (flags & IDRV_VF_PALETTE_SKIN) == IDRV_VF_PALETTE_SKIN)
	{
		_Flags|=IDRV_VF_PALETTE_SKIN;
		_PaletteSkinOff=_VertexSize;
		_VertexSize+=sizeof(CPaletteSkin);
	}

	// compute new capacity
	_Capacity = floorf(_Capacity * (float(oldVertexSize) / _VertexSize)) ;

	return(true);
}

// --------------------------------------------------

void CVertexBuffer::reserve(uint32 n)
{
	_Verts.resize(n*_VertexSize);
	_Capacity= n;
}

void CVertexBuffer::setNumVertices(uint32 n)
{
	if(_Capacity<n)
	{
		reserve(n);
	}
	if(_NbVerts != n)
	{
		_Touch|= IDRV_VF_TOUCHED_NUM_VERTICES;
		_NbVerts=n;
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

void* CVertexBuffer::getNormalCoordPointer(uint idx)
{
	uint8*	ptr;

	if ( !(_Flags & IDRV_VF_NORMAL) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_NormalOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

void* CVertexBuffer::getColorPointer(uint idx)
{
	uint8*	ptr;

	if ( !(_Flags & IDRV_VF_COLOR) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_RGBAOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

void* CVertexBuffer::getSpecularPointer(uint idx)
{
	uint8*	ptr;

	if ( !(_Flags & IDRV_VF_SPECULAR) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_SpecularOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

void* CVertexBuffer::getTexCoordPointer(uint idx, uint8 stage)
{
	uint8*	ptr;

	if ( !(_Flags & IDRV_VF_UV[stage]) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_UVOff[stage];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}


void* CVertexBuffer::getWeightPointer(uint idx, uint8 wgt)
{
	uint8*	ptr;

	nlassert(wgt<IDRV_VF_MAXW);
	if( !(_Flags & IDRV_VF_W[wgt]))
		return NULL;

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_WOff[wgt];

	return ptr;
}


void* CVertexBuffer::getPaletteSkinPointer(uint idx)
{
	uint8*	ptr;

	if ( (_Flags & IDRV_VF_PALETTE_SKIN) != IDRV_VF_PALETTE_SKIN )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_PaletteSkinOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}


////////////////////
// const versions //
////////////////////


const void* CVertexBuffer::getVertexCoordPointer(uint idx) const 
{
	const uint8*	ptr;

	ptr=&(*_Verts.begin());
	ptr+=(idx*_VertexSize);
	return((void*)ptr);
}

const void* CVertexBuffer::getNormalCoordPointer(uint idx) const
{
	const uint8*	ptr;

	if ( !(_Flags & IDRV_VF_NORMAL) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_NormalOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

const void* CVertexBuffer::getColorPointer(uint idx) const
{
	const uint8*	ptr;

	if ( !(_Flags & IDRV_VF_COLOR) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_RGBAOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

const void* CVertexBuffer::getSpecularPointer(uint idx) const
{
	const uint8*	ptr;

	if ( !(_Flags & IDRV_VF_SPECULAR) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_SpecularOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}

const void* CVertexBuffer::getTexCoordPointer(uint idx, uint8 stage) const
{
	const uint8*	ptr;

	if ( !(_Flags & IDRV_VF_UV[stage]) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_UVOff[stage];
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}


const void* CVertexBuffer::getWeightPointer(uint idx, uint8 wgt) const
{
	const uint8*	ptr;

	nlassert(wgt<IDRV_VF_MAXW);
	if( !(_Flags & IDRV_VF_W[wgt]))
		return NULL;

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_WOff[wgt];

	return ptr;
}


const void* CVertexBuffer::getPaletteSkinPointer(uint idx) const
{
	const uint8*	ptr;

	if ( (_Flags & IDRV_VF_PALETTE_SKIN) != IDRV_VF_PALETTE_SKIN )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_PaletteSkinOff;
	ptr+=idx*_VertexSize;
	return((void*)ptr);
}





//****************************************************************************
void		CVertexBuffer::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- PaletteSkin version.
	Version 0:
		- base verison.
	*/
	sint	ver= f.serialVersion(1);

	// Serial VBuffers format/size.
	//=============================
	f.serial(_Flags);
	f.serial(_NbVerts);
	if(f.isReading())
	{
		reserve(0);
		setVertexFormat(_Flags);
		setNumVertices(_NbVerts);
	}
	// All other infos (but _Verts) are computed by setVertexFormat() and setNumVertices().


	// Serial VBuffers components.
	//============================
	sint	i;
	for(sint id=0;id<(sint)_NbVerts;id++)
	{
		// XYZ.
		if(_Flags & IDRV_VF_XYZ)
		{
			CVector		&vert= *(CVector*)getVertexCoordPointer(id);
			f.serial(vert);
		}
		// Normal
		if(_Flags & IDRV_VF_NORMAL)
		{
			CVector		&norm= *(CVector*)getNormalCoordPointer(id);
			f.serial(norm);
		}
		// Uvs.
		for(i=0;i<IDRV_VF_MAXSTAGES;i++)
		{
			if(_Flags & IDRV_VF_UV[i])
			{
				CUV		&uv= *(CUV*)getTexCoordPointer(id, i);
				f.serial(uv);
			}
		}
		// Color.
		if(_Flags & IDRV_VF_COLOR)
		{
			CRGBA		&col= *(CRGBA*)getColorPointer(id);
			f.serial(col);
		}
		// Specular.
		if(_Flags & IDRV_VF_SPECULAR)
		{
			CRGBA		&col= *(CRGBA*)getSpecularPointer(id);
			f.serial(col);
		}
		// Weights
		for(i=0;i<IDRV_VF_MAXW;i++)
		{
			if(_Flags & IDRV_VF_W[i])
			{
				float	&w= *(float*)getWeightPointer(id, i);
				f.serial(w);
			}
		}
		// CPaletteSkin (version 1+ only).
		if(ver>=1 && (_Flags & IDRV_VF_PALETTE_SKIN) == IDRV_VF_PALETTE_SKIN )
		{
			CPaletteSkin	&ps= *(CPaletteSkin*)getPaletteSkinPointer(id);
			f.serial(ps);
		}

	}

	// Set touch flags
	if(f.isReading())
		_Touch= IDRV_VF_TOUCHED_ALL;

}

// CPaletteSkin serial (no version chek).
void	CPaletteSkin::serial(NLMISC::IStream &f)
{
	f.serial(MatrixId[0], MatrixId[1], MatrixId[2], MatrixId[3]);
}



//****************************************************************************
IVBDrvInfos::~IVBDrvInfos()
{
	_Driver->removeVBDrvInfoPtr(_DriverIterator);
}



}
