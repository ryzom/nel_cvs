/** \file tess_face_priority_list.cpp
 * <File description>
 *
 * $Id: tess_face_priority_list.cpp,v 1.2 2002/02/28 12:59:51 besson Exp $
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

#include "std3d.h"

#include "3d/tess_face_priority_list.h"
#include "nel/misc/debug.h"
#include <math.h>
#include "3d/tessellation.h"


namespace NL3D 
{

// ***************************************************************************
// ***************************************************************************
//	CTessFacePListNode
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTessFacePListNode::CTessFacePListNode()
{
	_PrecTessFaceInPList= NULL;
	_NextTessFaceInPList= NULL;
}

// ***************************************************************************
CTessFacePListNode::~CTessFacePListNode()
{
	// if not done, unlink.
	unlinkInPList();
}

// ***************************************************************************
void		CTessFacePListNode::linkInPList(CTessFacePListNode	&root)
{
	unlinkInPList();

	// if empty list.
	if( !root._NextTessFaceInPList )
	{
		// link me to the list.
		_PrecTessFaceInPList= &root;
		_NextTessFaceInPList= &root;
		// link the list to me. NB: works even if root is empty, because root._NextTessFaceInPList==this.
		root._PrecTessFaceInPList= this;
		root._NextTessFaceInPList= this;
	}
	// else, list no empty.
	else
	{
		// link me to the list.
		_PrecTessFaceInPList= &root;
		_NextTessFaceInPList= root._NextTessFaceInPList;
		// link the list to me.
		root._NextTessFaceInPList->_PrecTessFaceInPList= this;
		root._NextTessFaceInPList= this;
	}
}

// ***************************************************************************
void		CTessFacePListNode::unlinkInPList()
{
	// if linked to a list.
	if( _NextTessFaceInPList )
	{
		// unlink the list from me.
		// to work properly in all case, must bkup before
		CTessFacePListNode		*next= _NextTessFaceInPList;
		CTessFacePListNode		*prec= _PrecTessFaceInPList;
		// test if last 2 elements of the list.
		if(next==prec)
		{
			// prec should be the root. No more linked => empty!
			prec->_NextTessFaceInPList= NULL;
			prec->_PrecTessFaceInPList= NULL;
		}
		else
		{
			prec->_NextTessFaceInPList= _NextTessFaceInPList;
			next->_PrecTessFaceInPList= _PrecTessFaceInPList;
		}
		
		// unlink me from any list.
		_PrecTessFaceInPList= NULL;
		_NextTessFaceInPList= NULL;
	}
}

// ***************************************************************************
void		CTessFacePListNode::appendPList(CTessFacePListNode	&root)
{
	// If list to append is not empty.
	if( root._NextTessFaceInPList )
	{
		// If we are empty.
		if( !_NextTessFaceInPList )
		{
			// link the appendList to the root.
			_PrecTessFaceInPList= root._PrecTessFaceInPList;
			_NextTessFaceInPList= root._NextTessFaceInPList;
			// link the root to the appendList.
			_PrecTessFaceInPList->_NextTessFaceInPList= this;
			_NextTessFaceInPList->_PrecTessFaceInPList= this;
		}
		// else bind first-last in the interval prec-next.
		else
		{
			CTessFacePListNode		*first= root._NextTessFaceInPList;
			CTessFacePListNode		*last= root._PrecTessFaceInPList;
			CTessFacePListNode		*prec= this;
			CTessFacePListNode		*next= _NextTessFaceInPList;
			// insert the appendList in our list.
			next->_PrecTessFaceInPList= last;
			prec->_NextTessFaceInPList= first;
			// insert our list in the appendList.
			last->_NextTessFaceInPList= next;
			first->_PrecTessFaceInPList= prec;
		}

		// clear input list.
		root._PrecTessFaceInPList= NULL;
		root._NextTessFaceInPList= NULL;
	}
}



// ***************************************************************************
// ***************************************************************************
//	CTessFacePriorityList
// ***************************************************************************
// ***************************************************************************

	
// ***************************************************************************
CTessFacePriorityList::CTessFacePriorityList()
{
	_Remainder= 0;
	_OODistStep= 1;
	_NEntries= 0;
	_EntryModStart= 0;
	_EntryStart= 0;
}

// ***************************************************************************
CTessFacePriorityList::~CTessFacePriorityList()
{
	clear();
}

// ***************************************************************************
void		CTessFacePriorityList::init(float distStep, float distMax, float distMaxMod)
{
	// clear the prioriy list before.
	clear();

	// Allocate the Rolling table.
	nlassert(distStep>0);
	nlassert(distMax>0);
	nlassert(distMaxMod<distMax);
	_OODistStep= 1.0f / distStep;
	_NEntries= (uint)ceil(distMax * _OODistStep);
	_EntryModStart= (uint)ceil(distMaxMod * _OODistStep);
	NLMISC::clamp(_EntryModStart, 0U, _NEntries-1);
	_EntryStart= 0;
	_Entries.resize(_NEntries);
}

// ***************************************************************************
void		CTessFacePriorityList::clear()
{
	// just clear all the rolling table.
	clearRollTable();
	// For convenience only (not really usefull).
	_Remainder= 0;
}

// ***************************************************************************
void		CTessFacePriorityList::insert(float distance, CTessFace *value)
{
	// plist must be inited.
	nlassert(_NEntries>0);

	// First, setup in our basis.
	distance*= _OODistStep;

	// Then, look where we must insert it.
	sint	idInsert;
	if(distance<=0)
		idInsert= 0;
	else
		// Must insert so we can't miss it when a shift occurs (=> floor).
		idInsert= (sint)floor(distance + _Remainder);
	idInsert= std::max(0, idInsert);

	// Manage Mod.
	// If the element to insert must be inserted at  distance > distMax.
	if(idInsert>(sint)(_NEntries-1))
	{
		// Compute number of entries to apply the mod.
		uint	nMod= _NEntries - _EntryModStart;
		nlassert(nMod>=1);
		// Of how many entries are we too far.
		idInsert= idInsert - (_NEntries-1);
		// Then loop in the interval [_EntryModStart, _NEntries[.
		idInsert= idInsert % nMod;
		idInsert= _EntryModStart + idInsert;
	}

	// insert in the Roll Table.
	insertInRollTable(idInsert, value);
}


// ***************************************************************************
void		CTessFacePriorityList::shiftEntries(uint entryShift, CTessFacePListNode	&pulledElements)
{
	if(entryShift>0)
	{
		// before shifting the roll Table, fill pulledElements.
		for(uint i=0; i<entryShift; i++)
		{
			// For all elements of the ith entry, pull them and isnert in result list.
			pulledElements.appendPList(getRollTableEntry(i));
		}

		// shift the roll Table. lists are already empty.
		shiftRollTable(entryShift);
	}
}


// ***************************************************************************
void		CTessFacePriorityList::shift(float shiftDistance, CTessFacePListNode	&pulledElements)
{
	// plist must be inited.
	nlassert(_NEntries>0);
	nlassert(shiftDistance>=0);

	// First, setup in our basis.
	shiftDistance*= _OODistStep;

	// at least, fill OUT with elements of entry 0.
	pulledElements.unlinkInPList();
	// For all elements of the entry 0, pull them, and insert in result list.
	pulledElements.appendPList(getRollTableEntry(0));

	// shift.
	_Remainder+= shiftDistance;
	// If Remainder>=1, it means that we must shift the rolling table, and get elements deleted.
	uint	entryShift= (uint)floor(_Remainder);
	_Remainder= _Remainder - entryShift;

	// shift full array??
	if( entryShift >= _NEntries)
	{
		entryShift= _NEntries;
		// The entire array is pulled, _Remainder should get a good value.
		_Remainder= 0;
	}

	// If some real shift, do it.
	shiftEntries(entryShift, pulledElements);
}


// ***************************************************************************
void		CTessFacePriorityList::shiftAll(CTessFacePListNode	&pulledElements)
{
	// plist must be inited.
	nlassert(_NEntries>0);

	pulledElements.unlinkInPList();

	// The entire array is pulled, _Remainder should get a good value.
	uint entryShift= _NEntries;
	_Remainder= 0;

	// shift the entire array.
	shiftEntries(entryShift, pulledElements);
}


// ***************************************************************************
// ***************************************************************************
// Rolling table.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTessFacePriorityList::insertInRollTable(uint entry, CTessFace *value)
{
	CTessFacePListNode	&root= _Entries[ (entry + _EntryStart)%_NEntries ];

	// Insert into list.
	value->linkInPList(root);
}

// ***************************************************************************
CTessFacePListNode	&CTessFacePriorityList::getRollTableEntry(uint entry)
{
	CTessFacePListNode	&root= _Entries[ (entry + _EntryStart)%_NEntries ];
	return root;
}

// ***************************************************************************
void		CTessFacePriorityList::clearRollTableEntry(uint entry)
{
	CTessFacePListNode	&root= _Entries[ (entry + _EntryStart)%_NEntries ];

	// clear all the list.
	while( root.nextInPList() )
	{
		// unlink from list
		CTessFacePListNode	*node= root.nextInPList();
		node->unlinkInPList();
	}
}

// ***************************************************************************
void		CTessFacePriorityList::shiftRollTable(uint shiftEntry)
{
	// delete all elements shifted.
	for(uint i=0; i<shiftEntry; i++)
	{
		clearRollTableEntry(i);
	}
	// shift to right the ptr of entries.
	_EntryStart+= shiftEntry;
	_EntryStart= _EntryStart % _NEntries;
}

// ***************************************************************************
void		CTessFacePriorityList::clearRollTable()
{
	for(uint i=0; i<_NEntries; i++)
	{
		clearRollTableEntry(i);
	}
	_EntryStart= 0;
}



} // NL3D
