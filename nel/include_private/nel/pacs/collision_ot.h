/** \file collision_ot.h
 * Collision descriptor for time odered table
 *
 * $Id: collision_ot.h,v 1.2 2001/05/31 13:36:42 corvazier Exp $
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

#ifndef NL_COLLISION_OT_H
#define NL_COLLISION_OT_H

#include "nel/misc/types_nl.h"
#include "nel/pacs/collision_desc.h"


namespace NLPACS 
{

class CMovePrimitive;
class CCollisionOTInfo;
	
/**
 * Odered table element
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionOT
{
public:
	CCollisionOT ()
	{
		_Info=false;
		clear ();
	}

	/// Clear the element
	void			clear ()
	{
		_Next=NULL;
		_Previous=NULL;
	}

	/// Link in the ot
	void			link (CCollisionOT* newElement)
	{
		if (newElement)
		{
			newElement->_Previous=this;
			newElement->_Next=_Next;
		}
		if (_Next)
			_Next->_Previous=newElement;
		_Next=newElement;
	}

	/// Remove from OT
	void			unlink ()
	{
		if (_Previous)
			_Previous->_Next=_Next;
		if (_Next)
			_Next->_Previous=_Previous;
		_Next=NULL;
		_Previous=NULL;
	}

	/// Return true if it an info collision else false;
	bool			isInfo () const
	{
		return _Info;
	}

	/// Get next element in the ot
	CCollisionOT		*getPrevious () const
	{
		return _Previous;
	}

	/// Get next element in the ot
	CCollisionOT		*getNext () const
	{
		return _Next;
	}

	/// Get next element in the ot
	CCollisionOTInfo	*getNextInfo () const;

private:
	// The next cell
	CCollisionOT	*_Next;

	// The previous cell
	CCollisionOT	*_Previous;

protected:
	// Is a CCollisionOTInfo
	bool			_Info;
};


/**
 * Collision descriptor for time odered table
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionOTInfo : public CCollisionOT
{
public:
	CCollisionOTInfo ()
	{
		_Info=true;
	}

	// Get the first primitive in the collision
	CMovePrimitive		*getFirstPrimitive () const
	{
		return _FirstPrimitive;
	}

	// Get the second primitive in the collision
	CMovePrimitive		*getSecondPrimitive () const
	{
		return _SecondPrimitive;
	}

	// Get the second primitive in the collision
	const CCollisionDesc	&getCollisionDesc () const
	{
		return _Desc;
	}

	// Init the info
	void				init (CMovePrimitive *firstPrimitive, CMovePrimitive *secondPrimitive, const CCollisionDesc& desc)
	{
		_FirstPrimitive=firstPrimitive;
		_SecondPrimitive=secondPrimitive;
		_Desc=desc;
	}

	// Link in the primitive
	void				primitiveLink (CMovePrimitive *primitive, CCollisionOTInfo *other)
	{
		// First primitive ?
		if (primitive==_FirstPrimitive)
		{
			// Check
			nlassert  (primitive!=_SecondPrimitive);

			// Link
			_FirstNext=other;
		}
		else // second
		{
			// Check
			nlassert  (primitive==_SecondPrimitive);

			// Link
			_SecondNext=other;
		}
	}

	// Link in the primitive
	CCollisionOTInfo	*getNext (CMovePrimitive *primitive) const
	{
		// First primitive ?
		if (primitive==_FirstPrimitive)
		{
			// Check
			nlassert  (primitive!=_SecondPrimitive);

			// return next
			return _FirstNext;
		}
		else // second
		{
			// Check
			nlassert  (primitive==_SecondPrimitive);

			// Link
			return _SecondNext;
		}
	}

private:

	// The first primitive
	CMovePrimitive		*_FirstPrimitive;

	// The second primitive
	CMovePrimitive		*_SecondPrimitive;

	// Descriptor
	CCollisionDesc		_Desc;

	// First primitive linked list
	CCollisionOTInfo	*_FirstNext;

	// Second primitive linked list
	CCollisionOTInfo	*_SecondNext;
};

// ***************************************************************************

inline CCollisionOTInfo	*CCollisionOT::getNextInfo () const
{
	// Get next
	CCollisionOT *next=_Next;

	// Is an info ?
	while ( next && (!next->isInfo ()) )
		next=next->getNextInfo ();

	// Return an info
	return (CCollisionOTInfo*)next;
}



} // NLPACS


#endif // NL_COLLISION_OT_H

/* End of collision_ot.h */
