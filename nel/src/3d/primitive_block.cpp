/** \file primitive_block.cpp
 * Primitive Block implementation
 *
 * $Id: primitive_block.cpp,v 1.3 2000/11/17 14:57:33 coutelas Exp $
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

namespace NL3D
{

// --------------------------------------------------


uint32* CPrimitiveBlock::getTriPointer(void)
{
	if(_Tri.begin()==_Tri.end())
		return NULL;
	else
		return(&(*_Tri.begin()));
}

// --------------------------------------------------


void				CPrimitiveBlock::reserveTri(uint32 n)
{
	_Tri.resize(n*3);
	_TriCapacity= n;
}
void				CPrimitiveBlock::setNumTri(uint32 n)
{
	if(_TriCapacity<n)
	{
		reserveTri(n);
	}
	_NbTris= n;
}
void				CPrimitiveBlock::setTri(uint triIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2)
{
	uint32*	ptr;

	ptr=(uint32*)(&_Tri[triIdx*3]);
	*ptr=vidx0;
	ptr++;
	*ptr=vidx1;
	ptr++;
	*ptr=vidx2;
}





/*---------------------------------------------------------------------------*\
							getQuadPointer()
\*---------------------------------------------------------------------------*/
uint32* CPrimitiveBlock::getQuadPointer(void)
{
	if(_Quad.begin()==_Quad.end())
		return NULL;
	else
		return(&(*_Quad.begin()));
}


/*---------------------------------------------------------------------------*\
							reserveQuad()
\*---------------------------------------------------------------------------*/
void CPrimitiveBlock::reserveQuad(uint32 n)
{
	_Quad.resize(n*4);
	_QuadCapacity = n;
}


/*---------------------------------------------------------------------------*\
							setNumQuad()
\*---------------------------------------------------------------------------*/
void CPrimitiveBlock::setNumQuad(uint32 n)
{
	if(_QuadCapacity<n)
	{
		reserveQuad(n);
	}
	_NbQuads = n;
}


/*---------------------------------------------------------------------------*\
							setQuad()
\*---------------------------------------------------------------------------*/
void CPrimitiveBlock::setQuad(uint quadIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3)
{
	uint32*	ptr;

	ptr = (uint32*)(&_Quad[quadIdx*3]);
	*ptr = vidx0;
	ptr++;
	*ptr = vidx1;
	ptr++;
	*ptr = vidx2;
	ptr++;
	*ptr = vidx3;
}



}
