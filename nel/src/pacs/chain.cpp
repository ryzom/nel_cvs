/** \file chain.cpp
 *
 *
 * $Id: chain.cpp,v 1.7 2001/05/18 08:24:06 legros Exp $
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

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "nel/misc/debug.h"

#include "nel/pacs/chain.h"

using namespace std;
using namespace NLMISC;


// Functions for vertices comparison.
// total order relation
static inline bool	isStrictlyLess(const CVector &a, const CVector &b)
{
	if (a.x < b.x)	return true;
	if (a.x > b.x)	return false;
	if (a.y < b.y)	return true;
	if (a.y > b.y)	return false;
	if (a.z < b.y)	return true;
	return false;
}

static inline bool	isStrictlyGreater(const CVector &a, const CVector &b)
{
	if (a.x > b.x)	return true;
	if (a.x < b.x)	return false;
	if (a.y > b.y)	return true;
	if (a.y < b.y)	return false;
	if (a.z > b.y)	return true;
	return false;
}

static inline bool	isEqual(const CVector &a, const CVector &b)
{
	return (a == b);
}


// COrderedChain methods implementation

// translates the ordered chain by the vector translation
void	NLPACS::COrderedChain::translate(const CVector &translation)
{
	uint	i;
	for (i=0; i<_Vertices.size(); ++i)
		_Vertices[i] += translation;
}

// serialises the ordered chain
void	NLPACS::COrderedChain::serial(IStream &f)
{
	f.serialCont(_Vertices);
	f.serial(_Forward);
	f.serial(_ParentId);
}

// sets value to the right surface id for later edge link
void	NLPACS::CChain::setIndexOnEdge(uint edge, sint32 index)
{
	// the _Right id should have been previously set to -2.
	if (_Right != -2)
	{
		nlwarning("in NLPACS::CChain::setIndexOnEdge()");
		nlwarning("Tried to set the right surface of a chain whereas previous value (%d) is different from -2", _Right);
		return;
	}

	// The index must be positive or zero
	if (index < 0)
	{
		nlwarning("in NLPACS::CChain::setIndexOnEdge()");
		nlwarning("Can't set negative index");
		return;
	}

	// sets _Edge and _Right values.
	_Edge = edge;
	_Right = -index-256;
}

// end of COrderedChain methods implementation


// CChain methods implementation

// builds the CChain from a list of vertices and a left and right surfaces id.
// the chains vector is the vector where to store generated ordered chains.
// thisId is the current id of the CChain, and edge is the number of the edge the CChain belongs to (-1
// if none.)
void	NLPACS::CChain::make(const vector<CVector> &vertices, sint32 left, sint32 right, vector<COrderedChain> &chains, uint16 thisId, sint edge)
{
	sint		first = 0, last = 0, i;

	_Left = left;
	_Right = right;
	_Edge = edge;

	// splits the vertices list in ordered sub chains.
	while (first < (sint)vertices.size()-1)
	{
		last = first+1;
		bool	forward = isStrictlyLess(vertices[first], vertices[last]);

		// first checks if the subchain goes forward or backward.
		if (forward)
			for (; last < (sint)vertices.size() && isStrictlyLess(vertices[last-1], vertices[last]); ++last)
				;
		else
			for (; last < (sint)vertices.size() && isStrictlyGreater(vertices[last-1], vertices[last]); ++last)
				;
		--last;

		// inserts the new subchain id within the CChain.
		uint32	subChainId = chains.size();
		if (subChainId > 65535)
			nlerror("in NLPACS::CChain::make(): reached the maximum number of ordered chains");
		_SubChains.push_back((uint16)subChainId);

		// and creates a new COrderedChain
		chains.resize(chains.size()+1);
		COrderedChain	&subchain = chains.back();
		subchain._Vertices.reserve(last-first+1);
		subchain._Forward = forward;
		subchain._ParentId = thisId;

		// and then copies the vertices (sorted, btw!)
		if (forward)
			for (i=first; i<=last; ++i)
				subchain._Vertices.push_back(vertices[i]);
		else
			for (i=last; i>=first; --i)
				subchain._Vertices.push_back(vertices[i]);

		first = last;
	}
}

// serialises the CChain
void	NLPACS::CChain::serial(IStream &f)
{
	f.serialCont(_SubChains);
	f.serial(_Left, _Right);
	f.serial(_StartTip, _StopTip);
	f.serial(_Edge);
}

// end of CChain methods implementation

