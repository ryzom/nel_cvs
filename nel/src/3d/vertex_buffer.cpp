/** \file vertex_buffer.cpp
 * Vertex Buffer implementation
 *
 * $Id: vertex_buffer.cpp,v 1.5 2000/11/21 18:13:33 valignat Exp $
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

#include "nel/3d/driver.h"
#include "nel/misc/debug.h"
using namespace NLMISC;


namespace NL3D
{

// --------------------------------------------------

CVertexBuffer::CVertexBuffer()
{
	_Flags=0;
	_Capacity=0;
	_NbVerts=0;
}

CVertexBuffer::~CVertexBuffer()
{
}

// --------------------------------------------------

bool CVertexBuffer::setVertexFormat(uint32 flags)
{
	uint	i;
	uint	offset;

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
			_VertexSize+=3*sizeof(float);			
		}
	}
	if (flags & IDRV_VF_NORMAL)
	{
		_Flags|=IDRV_VF_NORMAL;
		_NormalOff=_VertexSize;
		_VertexSize+=3*sizeof(float);
	}
	if (flags & IDRV_VF_RGBA)
	{
		_Flags|=IDRV_VF_RGBA;
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
	_NbVerts=n;
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

	if ( !(_Flags & IDRV_VF_RGBA) )
	{
		return(NULL);
	}
	ptr=&(*_Verts.begin());
	ptr+=_RGBAOff;
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

// --------------------------------------------------

void CVertexBuffer::setVertexCoord(uint idx, float x, float y, float z)
{
	float*	ptr;

	nlassert(_Flags & IDRV_VF_XYZ);

	ptr=(float*)(&_Verts[idx*_VertexSize]);
	*ptr=x;
	ptr++;
	*ptr=y;
	ptr++;
	*ptr=z;
}

// --------------------------------------------------

void CVertexBuffer::setVertexCoord(uint idx, const CVector &v)
{
	uint8*	ptr;

	nlassert(_Flags & IDRV_VF_XYZ);
	ptr=&_Verts[idx*_VertexSize];
	memcpy(ptr, &(v.x), 3*sizeof(float));
}

// --------------------------------------------------

void CVertexBuffer::setNormalCoord(uint idx, const CVector &v)
{
	uint8*	ptr;

	nlassert(_Flags & IDRV_VF_NORMAL);

	ptr=&_Verts[idx*_VertexSize];
	ptr+=_NormalOff;
	memcpy(ptr, &(v.x), 3*sizeof(float));
}

// --------------------------------------------------

void CVertexBuffer::setRGBA(uint idx, CRGBA rgba)
{
	uint8*	ptr;

	nlassert(_Flags & IDRV_VF_RGBA);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_RGBAOff;
	*ptr=rgba.R;
	ptr++;
	*ptr=rgba.G;
	ptr++;
	*ptr=rgba.B;
	ptr++;
	*ptr=rgba.A;
}

// --------------------------------------------------

void CVertexBuffer::setTexCoord(uint idx, uint8 stage, float u, float v)
{
	uint8*	ptr;
	float*	ptrf;

	nlassert(stage<IDRV_VF_MAXSTAGES);
	nlassert(_Flags & IDRV_VF_UV[stage]);

	ptr=(uint8*)(&_Verts[idx*_VertexSize]);
	ptr+=_UVOff[stage];
	ptrf=(float*)ptr;
	*ptrf=u;
	ptrf++;
	*ptrf=v;
}

// --------------------------------------------------

}
