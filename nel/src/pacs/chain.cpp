/** \file chain.cpp
 *
 *
 * $Id: chain.cpp,v 1.13 2001/06/08 15:38:28 legros Exp $
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

#include "pacs/chain.h"

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


// COrderedChain3f methods implementation

void	NLPACS::COrderedChain3f::serial(IStream &f)
{
	f.serialCont(_Vertices);
	f.serial(_Forward);
	f.serial(_ParentId);
	f.serial(_IndexInParent);
}

// end of COrderedChain3f methods implementation

// COrderedChain methods implementation

// translates the ordered chain by the vector translation
void	NLPACS::COrderedChain::translate(const CVector &translation)
{
	uint	i;
	CVector2s	translat;
	translat.pack(translation);
	for (i=0; i<_Vertices.size(); ++i)
		_Vertices[i] += translat;
}

//
void	NLPACS::COrderedChain::traverse(sint from, sint to, bool forward, vector<NLPACS::CVector2s> &path) const
{
	sint	i;
	if (forward)
	{
		if (from < 0)	from = 0;
		if (to < 0)		to = _Vertices.size()-1;

		for (i=from+1; i<=to; ++i)
			path.push_back(_Vertices[i]);
	}
	else
	{
		if (from < 0)	from = _Vertices.size()-2;
		if (to < 0)		to = -1;

		for (i=from; i>to; --i)
			path.push_back(_Vertices[i]);
	}
}

// serialises the ordered chain
void	NLPACS::COrderedChain::serial(IStream &f)
{
	f.serialCont(_Vertices);
	f.serial(_Forward);
	f.serial(_ParentId);
	f.serial(_IndexInParent);
	f.serial(_Length);
}

// end of COrderedChain methods implementation


// CChain methods implementation

// builds the CChain from a list of vertices and a left and right surfaces id.
// the chains vector is the vector where to store generated ordered chains.
// thisId is the current id of the CChain, and edge is the number of the edge the CChain belongs to (-1
// if none.)
void	NLPACS::CChain::make(const vector<CVector> &vertices, sint32 left, sint32 right, vector<COrderedChain> &chains, uint16 thisId, sint edge,
							 vector<COrderedChain3f> &fullChains)
{
	sint		first = 0, last = 0, i;

	_Left = left;
	_Right = right;
	_Edge = edge;
	_Length = 0.0f;

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
		fullChains.resize(fullChains.size()+1);
		COrderedChain3f	&subchain3f = fullChains.back();
		subchain3f._Vertices.reserve(last-first+1);
		subchain3f._Forward = forward;
		subchain3f._ParentId = thisId;
		subchain3f._IndexInParent = _SubChains.size()-1;

		// and then copies the vertices (sorted, btw!)
		if (forward)
			for (i=first; i<=last; ++i)
				subchain3f._Vertices.push_back(vertices[i]);
		else
			for (i=last; i>=first; --i)
				subchain3f._Vertices.push_back(vertices[i]);

		first = last;

		chains.resize(chains.size()+1);
		COrderedChain	&subchain = chains.back();
		subchain.pack(subchain3f);

		float	length = 0.0f;
		for (i=0; i<(sint)subchain._Vertices.size()-1; ++i)
			length += (subchain._Vertices[i+1]-subchain._Vertices[i]).norm();

		subchain._Length = length;
		_Length += length;
	}
}

// serialises the CChain
void	NLPACS::CChain::serial(IStream &f)
{
	f.serialCont(_SubChains);
	f.serial(_Left, _Right);
	f.serial(_StartTip, _StopTip);
	f.serial(_Edge);
	f.serial(_Length);
	f.serial(_LeftLoop, _LeftLoopIndex);
	f.serial(_RightLoop, _RightLoopIndex);
}

// sets value to the right surface id for later edge link
void	NLPACS::CChain::setIndexOnEdge(uint edge, sint32 index)
{
	// the _Right id should have been previously set to -2.
	if (_Right >= 0)
	{
		nlwarning("in NLPACS::CChain::setIndexOnEdge()");
		nlwarning("Tried to set the right surface of a chain whereas previous value (%d) is greater than -1", _Right);
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

// end of CChain methods implementation

