/** \file ordering_table.h
 * Generic Ordering Table
 *
 * $Id: ordering_table.h,v 1.1 2001/07/02 12:00:10 besson Exp $
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

#ifndef NL_ORDERING_TABLE_H
#define NL_ORDERING_TABLE_H

#include "nel/misc/pool_memory.h"

namespace NL3D 
{

using NLMISC::CPoolMemory;

// ***************************************************************************
/**
 * 
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2000
 */
template<class T> class COrderingTable
{

public:

	COrderingTable();
	~COrderingTable();

	/**
	 * Initialization.
	 * The ordering table has a range from 0 to nNbEntries-1
	 */
	void init( uint32 nNbEntries );

	/**
	 * Put the ordering table to empty
	 */
	void reset();

	/**
	 * Insert an element in the ordering table
	 */
	void insert( uint32 nEntryPos, T *pValue );

	/**
	 * Traversing operations
	 * 
	 *	OrderingTable<Face> ot;
	 *	ot.begin();
	 *	while( ot.get() != NULL )
	 *	{
	 *		Face *pF = ot.get();
	 *		// Do the treatment you want here
	 *		ot.next();
	 *	}
	 */
	void begin();

	/**
	 * Get the currently selected element.
	 */
	T* get();

	/**
	 * Move selection pointer to the next element
	 */
	void next();

// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:

	struct CNode
	{
		T *val;
		CNode *next;

		CNode()
		{ 
			val = NULL;
			next = NULL;
		}
	};

	
	CPoolMemory<CNode> _Allocator;

	uint32 _nNbElt;
	CNode* _Array;
	CNode* _SelNode;

};

// ***************************************************************************
template<class T> COrderingTable<T>::COrderingTable() : _Allocator( 128 )
{
	_nNbElt = 0;
	_Array = NULL;
	_SelNode = NULL;
}

// ***************************************************************************
template<class T> COrderingTable<T>::~COrderingTable()
{
	if( _Array != NULL )
		delete [] _Array;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::init( uint32 nNbEntries )
{
	if( _Array != NULL )
	{
		reset();
		delete [] _Array;
	}
	_nNbElt = nNbEntries;
	_Array = new CNode[_nNbElt];
	reset();
}

// ***************************************************************************
template<class T> void COrderingTable<T>::reset()
{
	_Allocator.free();

	for( uint32 i = 0; i < _nNbElt-1; ++i )
	{
		_Array[i].next = &_Array[i+1];
	}
	_Array[_nNbElt-1].next = NULL;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::insert( uint32 nEntryPos, T *pValue )
{
	if( nEntryPos >= _nNbElt )
		nEntryPos = _nNbElt-1;
	CNode *SelNode = &_Array[nEntryPos];
	while( SelNode->val != NULL )
		SelNode = SelNode->next;
	SelNode->val = pValue;
	CNode *nextNode = _Allocator.allocate();
	nextNode->next = SelNode->next;
	SelNode->next = nextNode;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::begin()
{
	_SelNode = &_Array[0];
	if( _SelNode->val == NULL )
		next();
}

// ***************************************************************************
template<class T> T* COrderingTable<T>::get()
{
	if( _SelNode != NULL )
		return _SelNode->val;
	else
		return NULL;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::next()
{
	_SelNode = _SelNode->next;
	while( ( _SelNode != NULL )&&( _SelNode->val == NULL ) )
		_SelNode = _SelNode->next;
}

} // NL3D


#endif // NL_ORDERING_TABLE_H

/* End of ordering_table.h */
