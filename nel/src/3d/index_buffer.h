/** \file index_buffer.h
 * Index buffers.
 *
 * $Id: index_buffer.h,v 1.1 2004/03/19 10:11:35 corvazier Exp $
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

#ifndef NL_INDEX_BUFFER_H
#define NL_INDEX_BUFFER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include <vector>

namespace NLMISC
{
	class IStream;
};

namespace NL3D {

using NLMISC::CRefCount;
using NLMISC::CRefPtr;

// List typedef.
class	IIBDrvInfos;
class IDriver;
typedef	std::list<IIBDrvInfos*>			TIBDrvInfoPtrList;
typedef	TIBDrvInfoPtrList::iterator		ItIBDrvInfoPtrList;

/**
  * Index buffer driver informations.
  */
// *** IMPORTANT ********************
// *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
// **********************************
class IIBDrvInfos : public CRefCount
{
private:
	IDriver				*_Driver;
	ItIBDrvInfoPtrList	_DriverIterator;
	class CIndexBuffer	*_IndexBuffer;

public:
	IIBDrvInfos(IDriver	*drv, ItIBDrvInfoPtrList it, CIndexBuffer *ib) {_Driver= drv; _DriverIterator= it; _IndexBuffer = ib;}

	/** Lock method.
	  * \param first is the first index to be accessed. Put 0 to select all the indexes. What ever is this index, 
	  * the indexices in the index buffer remain the same.
	  * \param last is the last index to be accessed + 1. Put 0 to select all the indexes.
	  */
	virtual uint32	*lock (uint first, uint last, bool readOnly) =0;

	/** Unlock method.
	  * \param first is the index of the first indexes to update. 0 to update all the indexes.
	  * \param last is the index of the last indexes to update + 1. 0 to update all the indexes.
	  */
	virtual void	unlock (uint first, uint last) =0;

	// The virtual dtor is important.
	virtual ~IIBDrvInfos();
};

/**
 * An index buffer to work with the driver
 *
 * Before the index buffer is resident (IDriver::activeIndexBuffer), it is in system memory.
 * Once the index buffer is resident, the driver creates its proprietary index buffer and release the internal index buffer.
 * At this moment the index buffer can be in VRAM, AGP or system memory.
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// All these flags are similar to DX8
class CIndexBuffer : public CRefCount
{
public:
	friend class CIndexBufferReadWrite;
	friend class CIndexBufferRead;

	/**
	  * Type of preferred memory
	  */
	enum TPreferredMemory
	{
		RAMPreferred = 0,
		AGPPreferred,
		VRAMPreferred,
		PreferredCount,
	};

	/**
	  * Type of index buffer location
	  */
	enum TLocation
	{
		RAMResident = 0,
		AGPResident,
		VRAMResident,
		NotResident,
		LocationCount,
	};

	/**
	  * Internal flags
	  */
	enum
	{
		/// Vertex format touched
		TouchedIndexFormat		= 1,

		/// Num vertices touched
		TouchedNumIndexes		= 2,

		/// Reserve touched
		TouchedReserve			= 4,

		/// All touhched
		TouchedAll				= 0xFFFF
	};

public:

	// \name Private. For Driver only.
	// @{
	TLocation				Location;		// The driver implementation must set the location after activeIndexBuffer.
	CRefPtr<IIBDrvInfos>	DrvInfos;
	uint					getTouchFlags() const { return _InternalFlags&TouchedAll; }
	void					resetTouchFlags() {_InternalFlags &= ~TouchedAll;}
	void					releaseNonResidentIndexes ();
	void					restoreNonResidentIndexes ();
	// @}

public:

	/**
	  * Default constructor. Make an empty index buffer. No value, no index. Index color format is set to TRGBA.
	  */
	CIndexBuffer(void);

	/**
	  * Copy constructor.
	  *  Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  */
	CIndexBuffer(const CIndexBuffer &vb);

	/**
	  * Destructor.
	  */
	~CIndexBuffer(void);

	/**
	  * Copy operator.
	  * Do not copy DrvInfos, copy all infos and set IDRV_VF_TOUCHED_ALL.
	  * All the destination index buffer is invalidated. Data are lost.
	  * The index buffer must not be locked.
	  */
	CIndexBuffer			&operator=(const CIndexBuffer &vb);

	/**
	  * Set the index buffer preferred memory. Default preferred memory is RAM.
	  *
	  * If VRAM memory allocation failed, the driver will try with AGP and then with RAM.
	  * If AGP memory allocation failed, the driver will try with RAM.
	  * RAM allocation should never failed
	  *
 	  *	Performance note:
	  *	 - for RAM CIndexBuffer, you can read / write as you like.
	  *	 - for AGP CIndexBuffer, you should write sequentially to take full advantage of the write combiners.
	  *	 - for VRAM CIndexBuffer, you should write only one time, to init.
	  *
	  * If the index buffer is resident, the data are lost and the index buffer is no more resident.
	  * The index buffer is invalidated. Data are lost.
	  * The index buffer must not be locked.
	  */
	void setPreferredMemory (TPreferredMemory preferredMemory);

	/**
	  * Get the index buffer preferred memory.
	  */
	TPreferredMemory getPreferredMemory () const { return _PreferredMemory; }

	/**
	  * Get the index buffer current location.
	  */
	TLocation getLocation () const { return Location; }

	/**
	  * Set the write only memory flag for each memory type. If set, the user must not read the memory of this index buffer.
	  * Default is false for RAMPreferred and true for AGPPreferred and VRAMPreferred.
	  *
	  * If the index buffer is resident in the memory type modified, the data are lost and the index buffer is no more resident,
	  * the index buffer is invalidated. Data are lost and the index buffer must not be locked.
	  */ 
	void setWriteOnly(TPreferredMemory preferredMemory, bool writeOnly);

	/**
	  * Get write only memory flag.
	  */ 
	bool getWriteOnly(TPreferredMemory preferredMemory) const {return _WriteOnly[preferredMemory];}

	/**
	  * Returns if the index buffer is driver resident or not.
	  * The index buffer is resident after a call to IDriver::activeIndexBuffer().
	  */
	bool isResident () const { return (Location != NotResident) && DrvInfos; }

	/** 
	  * Set the number of active indexes. It enlarge capacity, if needed.
	  * If the new size is bigger than capacity, reserve() will be called. see reserve().
	  * If the new size is smaller than capacity, the data are keeped, the index buffer stay resident if it is resident.
	  */
	void					setNumIndexes(uint32 n);
	
	/** 
	  * Get the number of active indexes.
	  */
	uint32					getNumIndexes(void) const  { return(_NbIndexes); }

	/**
	  * Reset all the indexes from memory (contReset()), so that capacity() == getNumIndexes() == 0.
	  *
	  * If the index buffer is resident, the data are lost and the index buffer is no more resident.
	  * The index buffer is invalidated. Data are lost.
	  * The index buffer must not be locked.
	  */
	void					deleteAllIndexes();

	/**
	  * Reserve space for nIndexes indexes. You are allowed to write your indexes on this space.
	  * If the index buffer is resident in a write only memory, the data are lost and the index buffer is no more resident.
	  * If the index buffer is not resident or resident in a readable memory, the data are keeped.
	  *
	  * The index buffer is invalidated. Data are lost.
	  * The index buffer must not be locked.
	  */
	void					reserve(uint32 nIndexes);
	
	/**
	  * Return the number of indexes reserved.
	  */
	uint32					capacity() { return _Capacity; }

	/**
	  * If the index buffer is resident, the data are lost and the index buffer is no more resident.
	  * The index buffer is invalidated.
	  * The index buffer must not be locked.
	  */
	void					serial(NLMISC::IStream &f);

	/**
	  * Access indexes. Multi lock is possible only if no regions are used. Each lock need an accessor to be unlocked.
	  *
	  * Lock the index buffer and return and fill an accessor object. Once the object is destroyed, the buffer in unlocked.
	  *
	  * \param accessor is the accessor object to fill
	  * \param first is the first index to be accessed. Put 0 to select all the indexes. What ever is this index, 
	  * the indexices in the index buffer remain the same.
	  * \param last is the last index to be accessed + 1. Put 0 to select all the indexes.
	  */
	inline void	lock (CIndexBufferReadWrite &accessor, uint first=0, uint last=0);

	/**
	  * Read only indexes access. Multi lock is possible only if no regions are used. Each lock need an accessor to be unlocked.
	  *
	  * Lock the index buffer and return and fill an accessor object. Once the object is destroyed, the buffer in unlocked.
	  *
	  * \param accessor is the accessor object to fill
	  * \param first is the first index to be accessed. Put 0 to select all the indexes. What ever is this index, 
	  * the indexices in the index buffer remain the same.
	  * \param last is the last index to be accessed + 1. Put 0 to select all the indexes.
	  */
	inline void	lock (CIndexBufferRead &accessor, uint first=0, uint last=0) const;

	// Return true if the vetx buffer is locked
	bool		isLocked () const {return _LockCounter!=0;}

private:

	// Check locked buffers
	bool checkLockedBuffer () const { return _LockedBuffer || (!isResident() && _NonResidentIndexes.empty()); }

	/**
	  * Unlock the index buffer. Called by CIndexBufferReadWrite.
	  * \param first the first index that as been modified.
	  * \param last the last index that as been modified + 1.
	  */
	inline void	unlock (uint first, uint last);

	/**
	  * Unlock the index buffer. Called by CIndexBufferRead.
	  * \param first the first index that as been modified.
	  * \param last the last index that as been modified + 1.
	  */
	inline void	unlock () const;
	
private:

	// Internal flags
	uint16					_InternalFlags;		// Offset 18 : aligned

	// Index count in the buffer
	uint32					_NbIndexes;			// Offset 20 : aligned

	// Capacity of the buffer
	uint32					_Capacity;

	// Index array
	std::vector<uint32>		_NonResidentIndexes;

	// The locked index buffer
	mutable uint32*			_LockedBuffer;

	// The index buffer is locked n times
	mutable uint			_LockCounter;

	// Prefered memory
	TPreferredMemory		_PreferredMemory;

	// Write only
	bool					_WriteOnly[PreferredCount];
};

/**
 * The index buffer accessor read / write
 */
class CIndexBufferReadWrite
{
public:
	friend class CIndexBuffer;

	CIndexBufferReadWrite()
	{
		_Parent = NULL;
	}
	~CIndexBufferReadWrite()
	{
		unlock();
	}

	/**
	  * Unlock the index buffer.
	  * After this call, the accessor should not be used before a new lock.
	  */
	void unlock()
	{
		if (_Parent)
		{
			_Parent->unlock(_First, _Last);
			_Parent = NULL;
		}
	}

	// Set a line index at index idx.
	inline void				setLine(uint idx, uint32 i0, uint32 i1)
	{
		nlassert (_Parent->checkLockedBuffer());
		_Parent->_LockedBuffer[idx] = i0;
		_Parent->_LockedBuffer[idx+1] = i1;
	}

	// Set a line index at index idx.
	inline void				setTri(uint idx, uint32 i0, uint32 i1, uint32 i2)
	{
		nlassert (_Parent->checkLockedBuffer());
		_Parent->_LockedBuffer[idx] = i0;
		_Parent->_LockedBuffer[idx+1] = i1;
		_Parent->_LockedBuffer[idx+2] = i2;
	}

	// Get a pointer on an index. 
	uint32*					getPtr (uint idx=0)
	{
		nlassert (_Parent->checkLockedBuffer());
		return &(_Parent->_LockedBuffer[0]);
	}

	/** Touch the updated indexes. If the method is not call, the accessor update all the indexes.
	  * \param first is the index of the first indexes to update.
	  * \param last is the index of the last indexes to update + 1.
	  */
	void					touchIndexes (uint first, uint last);

private:

	// No copy operators available
	void		operator=(const CIndexBufferReadWrite& other) {};
	CIndexBufferReadWrite(const CIndexBufferReadWrite& other) {};

	CIndexBuffer		*_Parent;
	uint				_First, _Last;
};

/**
 * The index buffer read accessor 
 */
class CIndexBufferRead
{
public:
	friend class CIndexBuffer;

	CIndexBufferRead()
	{
		_Parent = NULL;
	}
	~CIndexBufferRead()
	{
		unlock();
	}

	/**
	  * Unlock the index buffer.
	  * After this call, the accessor should not be used before a new lock.
	  */
	void unlock()
	{
		if (_Parent)
		{
			_Parent->unlock();
			_Parent = NULL;
		}
	}

	// Get a pointer on an index. 
	const uint32*		getPtr (uint idx=0) const
	{
		nlassert (_Parent->checkLockedBuffer());
		return &(_Parent->_LockedBuffer[0]);
	}

private:
	
	// No copy operators available
	void		operator=(const CIndexBufferRead& other) {};
	CIndexBufferRead(const CIndexBufferRead& other) {};

	const CIndexBuffer		*_Parent;
};

// ***************************************************************************

inline void CIndexBuffer::lock (CIndexBufferReadWrite &accessor, uint first, uint last)
{
	accessor.unlock();
	accessor._Parent = this;
	accessor._First = 0;
	accessor._Last = 0;

	// Already locked ?
	if (_LockCounter == 0)
	{
		nlassert (_LockedBuffer == NULL);

		// No
		if (isResident())
			_LockedBuffer = DrvInfos->lock (first, last, false);
		else
		{
			if (_NonResidentIndexes.empty())
				_LockedBuffer = NULL;
			else
				_LockedBuffer = &(_NonResidentIndexes[0]);
		}
	}
	else
		nlassert ((first==0)&&(last==0));
		
	_LockCounter++;
}

// ***************************************************************************

inline void CIndexBuffer::lock (CIndexBufferRead &accessor, uint first, uint last) const
{
	accessor.unlock();
	accessor._Parent = this;

	// Already locked ?
	if (_LockCounter == 0)
	{
		nlassert (_LockedBuffer == NULL);

		// No
		if (isResident())
		{
			// Can read it ?
			nlassertex (!_WriteOnly[Location], ("Try to read a write only index buffer"));
			_LockedBuffer = DrvInfos->lock (first, last, true);
		}
		else
		{
			if (_NonResidentIndexes.empty())
				_LockedBuffer = NULL;
			else
				_LockedBuffer = const_cast<uint32*>(&(_NonResidentIndexes[0]));
		}
	}
	else
		nlassert ((first==0)&&(last==0));
		
	_LockCounter++;
}

// ***************************************************************************

inline void CIndexBuffer::unlock (uint first, uint end)
{
	nlassertex (_LockCounter!=0, ("Index buffer not locked"));
	nlassert (_LockedBuffer || (!isResident() && _NonResidentIndexes.empty()));

	if (_LockCounter)
		_LockCounter--;

	if (_LockCounter == 0)
	{
		if (isResident())
			DrvInfos->unlock (0, 0);

		_LockedBuffer = NULL;
	}
}

// ***************************************************************************

inline void CIndexBuffer::unlock () const
{
	nlassertex (_LockCounter!=0, ("Index buffer not locked"));
	nlassert (_LockedBuffer || (!isResident() && _NonResidentIndexes.empty()));

	if (_LockCounter)
		_LockCounter--;

	if (_LockCounter == 0)
	{
		if (isResident())
			DrvInfos->unlock (0, 0);

		_LockedBuffer = NULL;
	}
}

// ***************************************************************************










//todo hulud remove
#if 0
/**
 * Class CIndexBuffer
 *
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CIndexBuffer
{
private:
	// Triangles.
	uint32				_NbTris;
	uint32				_TriCapacity;
	std::vector<uint32>	_Indexes;

	// Quads
	uint32				_NbQuads;
	uint32				_QuadCapacity;
	std::vector<uint32>	_Quad;

	// Lines
	uint32				_NbLines;
	uint32				_LineCapacity;
	std::vector<uint32>	_Line;

	/// \todo hulud: support for strips and fans
	// Strip/Fans
	uint32				_StripIdx;
	uint32*				_Strip;
	uint32				_FanIdx;
	uint32*				_Fan;
public:
						CIndexBuffer(void) 
						{_TriCapacity=_NbTris= _NbQuads=_QuadCapacity=_NbLines=_LineCapacity= 0;};
						~CIndexBuffer(void) {}; 
	
	
	// Lines. A line is 2 uint32.
	
	/// reserve space for nLines Line. You are allowed to write your Line indices on this space.
	void				reserveLine(uint32 n);
	/// Return the number of Line reserved.
	uint32				capacityLine() const {return _LineCapacity;}
	/// Set the number of active Line. It enlarge Line capacity, if needed.
	void				setNumLine(uint32 n);
	/// Get the number of active Lineangles.
	uint32				getNumLine(void) const {return _NbLines;}

	/// Build a Lineangle.
	void				setLine(uint lineIdx, uint32 vidx0, uint32 vidx1);
	/// Apend a line at getNumLine() (then resize +1 the numline).
	void				addLine(uint32 vidx0, uint32 vidx1);

	uint32*				getLinePointer(void);
	const uint32*				getLinePointer(void) const ;



	// Triangles. A triangle is 3 uint32.
	
	/// reserve space for nTris triangles. You are allowed to write your triangles indices on this space.
	void				reserveTri(uint32 n);
	/// Return the number of triangles reserved.
	uint32				capacityTri() {return _TriCapacity;}
	/// Set the number of active triangles. It enlarge Tri capacity, if needed.
	void				setNumTri(uint32 n);
	/// Get the number of active triangles.
	uint32				getNumTri(void) const {return _NbTris;}

	/// Build a triangle.
	void				setTri(uint triIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2);

	uint32*				getPtr(void);
	const uint32*				getPtr(void) const ;




	// Quads (a quad is 4 uint32)

	/**
	 *	reserve space for quads. 
	 */
	void reserveQuad(uint32 n);
	
	/**
	 * Return the number of triangles reserved.
	 */
	uint32 capacityQuad() { return _QuadCapacity; }
	
	/**
	 * Set the number of active quads. It enlarges Quad capacity, if needed.
	 */
	void setNumQuad(uint32 n);
	
	/**
	 * Get the number of active quads.
	 */
	uint32 getNumQuad(void) const { return _NbQuads; }

	/**
	 * Build a quad.
	 */
	void setQuad(uint quadIdx, uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3);

	/// Apend a quad at getNumQuad() (then resize +1 the numquad).
	void				addQuad(uint32 vidx0, uint32 vidx1, uint32 vidx2, uint32 vidx3);

	/**
	 * Return the Quad buffer
	 */
	uint32*	getQuadPointer(void);

	/// return the quad buffer, const version
	const uint32*	getQuadPointer(void) const ;

	/// return total number of triangle in this primitive block
	uint32	getNumTriangles ()
	{
		// Return number of triangles in this primitive block
		return _NbTris+2*_NbQuads+_NbLines;
	}

	void		serial(NLMISC::IStream &f);
};
#endif 

} // NL3D


#endif // NL_INDEX_BUFFER_H

/* End of index_buffer.h */
