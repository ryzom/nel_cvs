/** \file chain.cpp
 *
 *
 * $Id: chain.cpp,v 1.3 2001/05/09 12:59:06 legros Exp $
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


void	NLPACS::COrderedChain::serial(IStream &f)
{
	f.serialCont(_Vertices);
	f.serial(_Forward);
	f.serial(_ParentId);
}


void	NLPACS::CChain::make(const vector<CVector> &vertices, sint32 left, sint32 right, vector<COrderedChain> &chains, uint16 thisId)
{
	sint		first = 0, last = 0, i;

	_Left = left;
	_Right = right;
	_Edges = 0;

	while (first < (sint)vertices.size()-1)
	{
		last = first+1;
		bool	forward = isStrictlyLess(vertices[first], vertices[last]);

		if (forward)
			for (; last < (sint)vertices.size() && isStrictlyLess(vertices[last-1], vertices[last]); ++last)
				;
		else
			for (; last < (sint)vertices.size() && isStrictlyGreater(vertices[last-1], vertices[last]); ++last)
				;
		--last;

		uint32	subChainId = chains.size();
		if (subChainId > 65535)
			nlerror("in NLPACS::CChain::make(): reached the maximum number of ordered chains");
		_SubChains.push_back((uint16)subChainId);

		chains.resize(chains.size()+1);
		COrderedChain	&subchain = chains.back();
		subchain._Vertices.reserve(last-first+1);
		subchain._Forward = forward;
		subchain._ParentId = thisId;

		if (forward)
			for (i=first; i<=last; ++i)
				subchain._Vertices.push_back(vertices[i]);
		else
			for (i=last; i>=first; --i)
				subchain._Vertices.push_back(vertices[i]);

		first = last;
	}
}

void	NLPACS::CChain::serial(IStream &f)
{
	f.serialCont(_SubChains);
	f.serial(_Left, _Right);
	f.serial(_StartTip, _StopTip);
	f.serial(_Edges);
}


