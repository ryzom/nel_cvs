/** \file primitive_block.cpp
 * Index buffers.
 *
 * $Id: index_buffer.cpp,v 1.3 2004/04/08 09:05:45 corvazier Exp $
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
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;
}

// ***************************************************************************

CIndexBuffer::CIndexBuffer(const CIndexBuffer &vb)
{
	_Capacity = 0;
	_NbIndexes = 0;
	_LockCounter = 0;
	_LockedBuffer = NULL;
	_PreferredMemory = RAMPreferred;
	_Location = NotResident;
	_ResidentSize = 0;
	_KeepLocalMemory = false;
	operator=(vb);
}

// ***************************************************************************

CIndexBuffer::~CIndexBuffer()
{
	if (DrvInfos)
		DrvInfos->IndexBufferPtr = NULL;	// Tell the driver info to not restaure memory when it will die

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
	_KeepLocalMemory = vb._KeepLocalMemory;

	// Set touch flags
	_InternalFlags |= TouchedAll;
	_Location = NotResident;
	_ResidentSize = 0;

	return *this;
}

// ***************************************************************************

void CIndexBuffer::setPreferredMemory (TPreferredMemory preferredMemory, bool keepLocalMemory)
{
	if ((_PreferredMemory != preferredMemory) || (_KeepLocalMemory != keepLocalMemory))
	{
		_PreferredMemory = preferredMemory;
		_KeepLocalMemory = keepLocalMemory;

		// Force non resident
		restaureNonResidentMemory();
	}
}

// ***************************************************************************

void CIndexBuffer::reserve(uint32 n)
{
	nlassert (!isLocked());
	if (_Capacity != n)
	{
		const uint oldSize = _Capacity;
		_Capacity= n;
		_NbIndexes=std::min (_NbIndexes,_Capacity);

		// Force non resident
		restaureNonResidentMemory();
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

		// Force non resident
		restaureNonResidentMemory();

		// Delete driver info
		nlassert (DrvInfos == NULL);
	}
}

// ***************************************************************************

void CIndexBuffer::setLocation (TLocation newLocation)
{
	// Upload ?
	if (newLocation != NotResident)
	{
		// The driver must have setuped the driver info
		nlassert (DrvInfos);

		// Current size of the buffer
		const uint size = ((_PreferredMemory==RAMVolatile)||(_PreferredMemory==AGPVolatile))?_NbIndexes:_Capacity;

		// The buffer must not be resident
		if (_Location != NotResident)
			setLocation (NotResident);

		// Copy the buffer containt
		uint32 *dest = DrvInfos->lock (0, size, false);
		nlassert (_NonResidentIndexes.size() == _Capacity);	// Internal buffer must have the good size
		if (_Capacity != 0)
			memcpy (dest, &(_NonResidentIndexes[0]), size*sizeof(uint32));
		DrvInfos->unlock(0, 0);

		// Reset the non resident container if not a static preferred memory and not put in RAM
		if ((_PreferredMemory != StaticPreferred) && (_Location != RAMResident) && !_KeepLocalMemory)
			contReset(_NonResidentIndexes);

		// Clear touched flags
		resetTouchFlags ();

		_Location =	newLocation;
		_ResidentSize = _Capacity;
	}
	else
	{
		// Resize the non resident buffer
		_NonResidentIndexes.resize (_Capacity);

		// If resident in RAM, backup the data in non resident memory
		if ((_Location == RAMResident) && (_PreferredMemory != RAMVolatile) && (_PreferredMemory != AGPVolatile) && !_KeepLocalMemory)
		{
			// The driver must have setuped the driver info
			nlassert (DrvInfos);

			// Copy the old buffer data
			const uint32 *src = DrvInfos->lock (0, _ResidentSize, true);
			uint size = std::min ((uint)(_Capacity*sizeof(uint32)), (uint)(_ResidentSize*sizeof(uint32)));
			if (size)
				memcpy (&(_NonResidentIndexes[0]), src, size);
			DrvInfos->unlock(0, 0);
		}

		_Location = NotResident;
		_ResidentSize = 0;

		// Touch the buffer
		_InternalFlags |= TouchedAll;
	}
}

// ***************************************************************************

void CIndexBuffer::restaureNonResidentMemory()
{
	setLocation (NotResident);
	
	if (DrvInfos)
		DrvInfos->IndexBufferPtr = NULL;	// Tell the driver info to not restaure memory when it will die

	// Must kill the drv mirror of this VB.
	DrvInfos.kill();
}

// ***************************************************************************

void CIndexBuffer::serial(NLMISC::IStream &f)
{
	/** Version 2 : no more write only flags
	  * Version 1 : index buffer
	  * Version 0 : primitive block
	  */

	sint ver = f.serialVersion(2);

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

		// Read the old format
		if (ver == 1)
		{
			uint i;
			bool temp;
			for (i=0; i<PreferredCount; i++)
				f.serial(temp);
		}
	}

	// Loaded ?
	if (f.isReading())
	{
		// Force non resident
		restaureNonResidentMemory();
	}
}

// ***************************************************************************

void CIndexBuffer::fillBuffer ()
{
	if (DrvInfos && _KeepLocalMemory)
	{
		// Copy the local memory in local memory
		nlassert (_NbIndexes<=_NonResidentIndexes.size());
		uint32 *dest = DrvInfos->lock (0, _NbIndexes, false);
		memcpy (dest, &(_NonResidentIndexes[0]), _NbIndexes*sizeof(uint32));
		DrvInfos->unlock(0, _NbIndexes);
	}
}

// ***************************************************************************

} // namespace NL3D