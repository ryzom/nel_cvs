/** \file primitive_block.cpp
 * Index buffers.
 *
 * $Id: index_buffer.cpp,v 1.1 2004/03/19 10:11:35 corvazier Exp $
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

#include "3d/index_buffer.h"
#include "3d/driver.h"
#include "nel/misc/stream.h"

using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
// IIBDrvInfos
// ***************************************************************************

IIBDrvInfos::~IIBDrvInfos()
{
	_IndexBuffer->Location = CIndexBuffer::NotResident;
	_IndexBuffer->restoreNonResidentIndexes ();
	_Driver->removeIBDrvInfoPtr(_DriverIterator);
}

// ***************************************************************************
// CIndexBuffer
// ***************************************************************************

CIndexBuffer::CIndexBuffer()
{
	_Capacity = 0;
	_NbIndexes = 0;
	_InternalFlags = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_WriteOnly[RAMPreferred] = false;
	_WriteOnly[AGPPreferred] = true;
	_WriteOnly[VRAMPreferred] = true;
}

// ***************************************************************************

CIndexBuffer::CIndexBuffer(const CIndexBuffer &vb)
{
	_Capacity = 0;
	_NbIndexes = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	Location = NotResident;
	_WriteOnly[RAMPreferred] = false;
	_WriteOnly[AGPPreferred] = true;
	_WriteOnly[VRAMPreferred] = true;
	operator=(vb);
}

// ***************************************************************************

CIndexBuffer::~CIndexBuffer()
{
	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

// ***************************************************************************

CIndexBuffer	&CIndexBuffer::operator=(const CIndexBuffer &vb)
{
	nlassertex (!isLocked(), ("The index buffer is locked."));

	// Single value
	_InternalFlags = vb._InternalFlags;
	_NbIndexes = vb._NbIndexes;
	_Capacity = vb._Capacity;
	_NonResidentIndexes = vb._NonResidentIndexes;
	_PreferredMemory = vb._PreferredMemory;

	// Set touch flags
	_InternalFlags |= TouchedAll;
	Location = NotResident;

	return *this;
}

// ***************************************************************************

void CIndexBuffer::setPreferredMemory (TPreferredMemory preferredMemory)
{
	if (_PreferredMemory != preferredMemory)
	{
		_PreferredMemory = preferredMemory;
		_InternalFlags |= TouchedIndexFormat;
		Location = NotResident;
	}
}

// ***************************************************************************

void CIndexBuffer::setWriteOnly(TPreferredMemory preferredMemory, bool writeOnly)
{
	if ((preferredMemory == _PreferredMemory) && (_WriteOnly[preferredMemory] != writeOnly))
	{
		_WriteOnly[preferredMemory] = writeOnly;
		_InternalFlags |= TouchedIndexFormat;
		Location = NotResident;
	}
}

// ***************************************************************************

void CIndexBuffer::reserve(uint32 n)
{
	nlassert (!isLocked());
	if (_Capacity != n)
	{
		nlassert (!isLocked());
		const uint oldSize = _Capacity;
		_NonResidentIndexes.resize(n);
		_Capacity= n;
		if (_NbIndexes!=std::min (_NbIndexes,_Capacity))
		{
			_NbIndexes=std::min (_NbIndexes,_Capacity);
			_InternalFlags |= TouchedNumIndexes;
		}

		// Download old buffer
		if (isResident())
		{
			// Readable memory ?
			if (!_WriteOnly[Location])
			{
				const uint copySize = std::min ((uint)oldSize, (uint)_Capacity);
				if (copySize)
				{
					nlassert (DrvInfos);
					const uint32 *ptr = DrvInfos->lock (0, copySize, true);
					memcpy (&(_NonResidentIndexes[0]), ptr, copySize*sizeof(uint32));
					DrvInfos->unlock (0, 0);
				}
			}
		}
		_InternalFlags |= TouchedReserve;
		Location = NotResident;
	}
}

// ***************************************************************************

void CIndexBuffer::setNumIndexes(uint32 n)
{
	if(_Capacity<n)
	{
		reserve(n);
	}
	if(_NbIndexes != n)
	{
		_InternalFlags |= TouchedNumIndexes;
		_NbIndexes=n;
	}
}

// ***************************************************************************

void CIndexBuffer::deleteAllIndexes()
{
	if (_Capacity)
	{
		nlassert (!isLocked());
		// free memory.
		contReset(_NonResidentIndexes);
		_Capacity= 0;
		if(_NbIndexes!=0)
		{
			_NbIndexes=0;
			_InternalFlags |= TouchedNumIndexes;
		}

		_InternalFlags |= TouchedReserve;
		Location = NotResident;

		// Delete driver info
		DrvInfos.kill();
	}
}

// ***************************************************************************

void CIndexBuffer::serial(NLMISC::IStream &f)
{
	/** Version 1 : index buffer
	  * Version 0 : primitive block
	  */

	sint ver = f.serialVersion(1);

	// Primitive block?
	if (ver < 1)
	{
		uint32 nb, capacity;
		std::vector<uint32>	indexes;

		// Skip lines
		f.serial(nb, capacity);
		f.serialCont(indexes);

		// Read tri
		f.serial(nb, capacity);
		_NbIndexes = nb*3;
		_Capacity = capacity*3;
		f.serialCont(_NonResidentIndexes);

		// Skip quads
		f.serial(nb, capacity);
		f.serialCont(indexes);
	}

	// Index buffer?
	if (ver >= 1)
	{
		f.serial(_NbIndexes, _Capacity);
		f.serialCont(_NonResidentIndexes);
		f.serialEnum(_PreferredMemory);
		uint i;
		for (i=0; i<PreferredCount; i++)
			f.serial(_WriteOnly[i]);
	}

	// Loaded ?
	if (f.isReading())
	{
		_InternalFlags |= TouchedAll;
		Location = NotResident;
	}
}

// ***************************************************************************

void CIndexBuffer::releaseNonResidentIndexes ()
{
	contReset(_NonResidentIndexes);
}

// ***************************************************************************

void CIndexBuffer::restoreNonResidentIndexes ()
{
	_NonResidentIndexes.resize (_NbIndexes);
}

// ***************************************************************************

}
