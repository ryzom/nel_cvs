/** \file primitive_block.cpp
 * Primitive Block implementation
 *
 * $Id: primitive_block.cpp,v 1.16 2003/06/30 18:59:21 lecroart Exp $
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

#include "3d/primitive_block.h"
#include "nel/misc/stream.h"

namespace NL3D
{



// --------------------------------------------------

uint32* CPrimitiveBlock::getLinePointer(void)
{
	if(_Line.begin()==_Line.end())
		return NULL;
	else
		return(&(*_Line.begin()));
}

const uint32* CPrimitiveBlock::getLinePointer(void) const
{
	if(_Line.begin()==_Line.end())
		return NULL;
	else
		return(&(*_Line.begin()));
}



void				CPrimitiveBlock::reserveLine(uint32 n)
{
	_Line.resize(n*2);
	_LineCapacity= n;
}
void				CPrimitiveBlock::setNumLine(uint32 n)
{
	if(_LineCapacity<n)
	{
		reserveLine(n);
	}
	_NbLines= n;
}
void				CPrimitiveBlock::setLine(uint lineIdx, uint32 vidx0, uint32 vidx1)
{
	uint32*	ptr;

	ptr=(uint32*)(&_Line[lineIdx*2]);
	*ptr=vidx0;
	ptr++;
	*ptr=vidx1;
}


void				CPrimitiveBlock::addLine(uint32 vidx0, uint32 vidx1)
{
	setNumLine(getNumLine()+1);
	setLine(getNumLine()-1, vidx0, vidx1);
}


	
// --------------------------------------------------


uint32* CPrimitiveBlock::getTriPointer(void)
{
	if(_Tri.begin()==_Tri.end())
		return NULL;
	else
		return(&(*_Tri.begin()));
}

const uint32* CPrimitiveBlock::getTriPointer(void) const
{
	if(_Tri.begin()==_Tri.end())
		return NULL;
	else
		return(&(*_Tri.begin()));
}



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
	#ifdef NL_DEBUG
		nlassert((sint) triIdx <= ((sint) _Tri.size() - 3));
	#endif
	uint32*	ptr;

	ptr=(uint32*)(&_Tri[triIdx*3]);
	*ptr=vidx0;
	ptr++;
	*ptr=vidx1;
	ptr++;
	*ptr=vidx2;
}


void				CPrimitiveBlock::addTri(uint32 vidx0, uint32 vidx1, uint32 vidx2)
{
	setNumTri(getNumTri()+1);
	setTri(getNumTri()-1, vidx0, vidx1, vidx2);
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

const uint32* CPrimitiveBlock::getQuadPointer(void) const
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

	ptr = (uint32*)(&_Quad[quadIdx*4]);
	*ptr = vidx0;
	ptr++;
	*ptr = vidx1;
	ptr++;
	*ptr = vidx2;
	ptr++;
	*ptr = vidx3;
}


/*---------------------------------------------------------------------------*\
							addQuad()
\*---------------------------------------------------------------------------*/
void CPrimitiveBlock::addQuad(uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3)
{
	setNumQuad(getNumQuad()+1);
	setQuad(getNumQuad()-1, vidx0, vidx1, vidx2, vidx3);
}

// ***************************************************************************

void		CPrimitiveBlock::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(_NbLines, _LineCapacity);
	f.serialCont(_Line);
	f.serial(_NbTris, _TriCapacity);
	f.serialCont(_Tri);
	f.serial(_NbQuads, _QuadCapacity);
	f.serialCont(_Quad);

}

// ***************************************************************************

}
