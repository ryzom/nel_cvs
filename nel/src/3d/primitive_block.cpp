/** \file primitive_block.cpp
 * Primitive Block implementation
 *
 * $Id: primitive_block.cpp,v 1.1 2000/10/26 13:10:32 viau Exp $
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

bool CPrimitiveBlock::setNumTri(uint16 n)
{
	_TriIdx=0;
	_Tri.resize(n*3);
	return(false);
}

bool CPrimitiveBlock::addTri(uint16 idx1, uint16 idx2, uint16 idx3)
{
	uint32*	ptr;

	ptr=(uint32*)(&_Tri[_TriIdx*3]);
	*ptr=idx1;
	ptr++;
	*ptr=idx2;
	ptr++;
	*ptr=idx3;
	_TriIdx++;
	return(true);
}

uint16 CPrimitiveBlock::getNumTri(void)
{
	return(_TriIdx);
}

void* CPrimitiveBlock::getTriPointer(void)
{
	return((void*)&(*_Tri.begin()));
}

// --------------------------------------------------

}
