/** \file tess_face_priority_list.h
 * <File description>
 *
 * $Id: tess_face_priority_list.h,v 1.1 2001/10/10 15:48:39 berenguier Exp $
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

#ifndef NL_TESS_FACE_PRIORITY_LIST_H
#define NL_TESS_FACE_PRIORITY_LIST_H

#include "nel/misc/types_nl.h"
#include <vector>


namespace NL3D 
{


class	CTessFace;


// ***************************************************************************
/** A chain link node for PriorityList. NB: It is a circular list. But (NULL,NULL) if list is empty, or node is allone
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTessFacePListNode
{
public:

	// init to empty list.
	CTessFacePListNode();
	~CTessFacePListNode();

	/// unlinkInPList, then link this node to the root of a list.
	void		linkInPList(CTessFacePListNode	&root);
	/// if linked, unlink this node from his list.
	void		unlinkInPList();

	/** append a list just after this node. root is the root of the list. It is not inserted in the result. 
	 *	After this, the list pointed by "root" is empty.
	 */
	void		appendPList(CTessFacePListNode	&root);

	/// get next ptr. next==this if list empty.
	CTessFacePListNode		*precInPList() const {return _PrecTessFaceInPList;}
	CTessFacePListNode		*nextInPList() const {return _NextTessFaceInPList;}

private:
	CTessFacePListNode		*_PrecTessFaceInPList;
	CTessFacePListNode		*_NextTessFaceInPList;

};


// ***************************************************************************
/** This class manage a Priority list of elements, inserted with a "distance". The priority list can be shifted, so
 *	elements with new distance <=0 are pulled from the priority list.
 *
 *	NB: it works quite well if you have (as example) a distStep of 1, and you shift only with value of 0.01 etc..,
 *	because it manages a "Remainder system", which do the good stuff.
 *	How does it works? in essence, it is a Rolling table, with 1+ shift back when necessary (maybe not at each shift()).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTessFacePriorityList
{
public:

	/// Constructor
	CTessFacePriorityList();
	~CTessFacePriorityList();

	/** Clear and Init the priority list. It reserve a Rolling table of ceil(distMax/distStep) entries.
	 *
	 *	distMaxMod is important for performance and should be < distMax. eg: distMaxMod= 0.8*distMax.
	 *	It is a trick to avoid the "Too Far Priority problem". Imagine you have set distMax= 1000,
	 *	and you insert(1100, an element). If we clamp to the max, it may be a bad thing, because ALL elements
	 *	inserted after 1000 will be poped in one shift(), after 1000 shift(1) for example.
	 *	To avoid this, if distMaxMod==800, then insert(1050) will really insert at 850, so elements will be poped
	 *	not as the same time (for the extra cost of some elements get poped too early...).
	 */
	void		init(float distStep, float distMax, float distMaxMod);
	/** Clear the priority list. All elements are removed. NB: for convenience, the remainder is reset.
	 */
	void		clear();

	/** Insert an element at a given distance.
	 *	Insert at the closest step. eg insert(1.2, elt) will insert elt at entry 1 (assuming distStep==1 here).
	 *	Special case: if distance<=0, it is ensured that at each shift(), the element will be pulled.
	 *	NB: manage correctly the entry where it is inserted, according to the Remainder system.
	 */
	void		insert(float distance, CTessFace *value);

	/** Shift the Priority list of shiftDistance. NB: even if shiftDistance==0, all elements in the entry 0 are pulled out.
	 *	Out: pulledElements is the root of the list which will contains elements pulled from the list.
	 */
	void		shift(float shiftDistance, CTessFacePListNode	&pulledElements);

	/** Same as shift(), but shift all the array.
	 */
	void		shiftAll(CTessFacePListNode	&pulledElements);


// **************************
private:

	/*
		NB: Remainder E [0,1[.
		Meaning: value to substract to entries, to get their actual value.
		eg: the entry 1 means "1 unit" (in the internal basis, ie independent of distStep). If Remainder==0.1,
			then, entry 1 means "0.9 unit".
			Then, if, as example, an element must be inserted at dist=0.95, it will be inserted in entry 1, and not entry 0!
		NB: the "meaning" of entry 0 is always <=0.
	*/
	float	_Remainder;
	float	_OODistStep;


	/// fot shift() and shiftAll()
	void		shiftEntries(uint entryShift, CTessFacePListNode	&pulledElements);


	/// \name The rolling table <=> HTable.
	// @{
	std::vector<CTessFacePListNode>		_Entries;
	uint					_NEntries;
	uint					_EntryModStart;
	// where is the entry 0 in _Entries.
	uint					_EntryStart;


	// entry is relative to _EntryStart.
	void		insertInRollTable(uint entry, CTessFace *value);

	// get the root of the list at entry. entry is relative to _EntryStart.
	CTessFacePListNode		&getRollTableEntry(uint entry);

	// clear all element of an entry of the roll table. entry is relative to _EntryStart.
	void		clearRollTableEntry(uint entry);

	// shift the rollingTable of shiftEntry. elements in entries shifted are deleted. no-op if 0.
	void		shiftRollTable(uint shiftEntry);

	// clear all elements of the roll table.
	void		clearRollTable();

	// @}


};



} // NL3D


#endif // NL_TESS_FACE_PRIORITY_LIST_H

/* End of tess_face_priority_list.h */
