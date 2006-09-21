/** \file debug_vb.cpp
 *
 * $Id: debug_vb.cpp,v 1.3.30.1 2006/09/21 20:50:31 cado Exp $
 */

/* Copyright, 2000-2004 Nevrax Ltd.
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
#include "debug_vb.h"
#include "vertex_buffer.h"
#include "index_buffer.h"

namespace NL3D
{

static void checkRange(const uint8 *min, const uint8 *max, const uint8 *start, uint length, const std::string &msg)
{
	if (start + length < min || start >= max)
	{
		nlwarning("Buffer start = %x, end = %x, accessed range = (%x, %x), %s", (ptrdiff_t) min, (ptrdiff_t) max, (ptrdiff_t) start, (ptrdiff_t) (start + length), msg.c_str());
		nlassert(0);
	}
}

//************************************************************************************
void nlCheckVertexBuffer(const CVertexBuffer &vb, const uint8 *ptr)
{
	CVertexBufferRead vba; 
	vb.lock (vba);
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vb.getNumVertices() * vb.getVertexSize(), ptr, 0, vb.getName());
}

//************************************************************************************
void nlCheckVBA(CVertexBufferRead &vba, const uint8 *ptr)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptr, 0, vba.getParent()->getName());
}

//************************************************************************************
void nlCheckVBA(CVertexBufferReadWrite &vba, const uint8 *ptr)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptr, 0, vba.getParent()->getName());
}

//************************************************************************************
void nlCheckVBARange(CVertexBufferRead &vba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptStart, length, vba.getParent()->getName());
}

//************************************************************************************
void nlCheckVBARange(CVertexBufferReadWrite &vba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptStart, length, vba.getParent()->getName());
}

//************************************************************************************
void nlCheckIBARange(CIndexBufferReadWrite &iba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, length, iba.getParent()->getName());
}

//************************************************************************************
void nlCheckIBARange(CIndexBufferRead &iba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, length, iba.getParent()->getName());
}

//************************************************************************************
void nlCheckIBA(CIndexBufferReadWrite &iba, const uint8 *ptStart)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, 0, iba.getParent()->getName());
}

//************************************************************************************
void nlCheckIBA(CIndexBufferRead &iba, const uint8 *ptStart)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, 0, iba.getParent()->getName());
}

} // NL3D
