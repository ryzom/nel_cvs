/** \file vertex_buffer_hard.cpp
 * <File description>
 *
 * $Id: vertex_buffer_hard.cpp,v 1.2 2001/07/05 08:33:04 berenguier Exp $
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

#include "3d/vertex_buffer_hard.h"


namespace NL3D 
{



void	IVertexBufferHard::initFormat(uint32 flags, uint32 numVertices)
{
	// _NbVerts.
	_NbVerts= numVertices;

	uint	i;
	uint	offset;

	// Compute format: flags / offsets, for each component.
	_VertexSize=0;
	offset=0;
	_Flags=0;
	if (flags & IDRV_VF_XYZ)
	{
		_Flags|=IDRV_VF_XYZ;
		_VertexSize+=3*sizeof(float);
	}
	if (flags & IDRV_VF_NORMAL)
	{
		_Flags|=IDRV_VF_NORMAL;
		_NormalOff=_VertexSize;
		_VertexSize+=3*sizeof(float);
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
	for(i=0 ; i<IDRV_VF_MAXW ; i++)
	{
		if (flags & IDRV_VF_W[i])
		{
			_Flags|=IDRV_VF_W[i];
			_WOff[i]=_VertexSize;
			_VertexSize+=sizeof(float);			
		}
	}
	if ( (flags & IDRV_VF_PALETTE_SKIN) == IDRV_VF_PALETTE_SKIN)
	{
		_Flags|=IDRV_VF_PALETTE_SKIN;
		_PaletteSkinOff=_VertexSize;
		_VertexSize+=sizeof(CPaletteSkin);
	}
}



} // NL3D
