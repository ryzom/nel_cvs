/** \file chain.h
 * 
 *
 * $Id: chain.h,v 1.6 2001/05/16 15:57:40 legros Exp $
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

#ifndef NL_CHAIN_H
#define NL_CHAIN_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

namespace NLPACS
{

/**
 * A list of ordered vertices, partially delimiting 2 different surfaces.
 * In the vertex list, we consider the following order
 *    v1 < v2 iff  v1.x < v2.x  ||  v1.x == v2.x && v1.y < v2.y  ||  v1.x == v2.x && v1.y == v2.y && v1.z < v2.z
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class COrderedChain
{
protected:
	friend class CChain;
	friend class CChainCycle;
	friend class CRetrievableSurface;

	/// The vertices of the chain, ordered following x growth.
	std::vector<NLMISC::CVector>		_Vertices;

	/// Set if the chain should be read forward within the parent CChain (for sequential access to vertices.)
	bool								_Forward;

	/// The parent chain Id.
	uint16								_ParentId;

public:
	/// Returns the vertices of the chain
	const std::vector<NLMISC::CVector>	&getVertices() const { return _Vertices; }

	/// Returns true if the chain should be accessed forward within the parent CChain (see _Forward.)
	bool								isForward() const { return _Forward; }

	/// Returns the parent chain Id of this ordered chain.
	uint16								getParentId() const { return _ParentId; }

	void								translate(const NLMISC::CVector &translation);

	void								serial(NLMISC::IStream &f);
};


/**
 * A list of ordered chains of vertices, delimiting 2 surfaces.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CChain
{
protected:
	friend class CRetrievableSurface;
	friend class CLocalRetriever;

	/// The list of ordered chains that compose the chain.
	std::vector<uint16>					_SubChains;

	/// The surface on the left of the chain.
	sint32								_Left;

	/// The surface on the right of the chain.
	sint32								_Right;

	uint16								_StartTip;
	uint16								_StopTip;

	/// The edge on which the chain is stuck (-1 if not stuck on any edge)
	sint8								_Edge;

protected:
	friend class CRetrievableSurface;
	friend class CLocalRetriever;

	/// Build the whole surface from a vector of CVector and the left and right surfaces.
	void								make(const std::vector<NLMISC::CVector> &vertices, sint32 left, sint32 right, std::vector<COrderedChain> &chains, uint16 thisId, sint edges);

	void								setIndexOnEdge(uint edge, sint32 index);

public:
	/// Returns a vector of ordered chains that compose the entire chain
	const std::vector<uint16>			&getSubChains() const { return _SubChains; }
	uint16								getSubChain(uint n) const { return _SubChains[n]; }

	/// Returns the left surface id.
	sint32								getLeft() const { return _Left; }

	/// Returns the right surface id.
	sint32								getRight() const { return _Right; }

	/// Gets the index of the chain on the given edge (in the local retriever object.)
	sint32								getIndexOnEdge(sint edge) const
	{
		return (_Edge == edge && isEdgeId(_Right)) ? convertEdgeId(_Right) : -1;
	}

	static bool							isEdgeId(sint32 id) { return id <= -256; }
	static sint32						convertEdgeId(sint32 id) { return -(id+256); }
	
	uint16								getStartTip() const { return _StartTip; }

	uint16								getStopTip() const { return _StopTip; }

	sint8								getEdge() const { return _Edge; }

	void								serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_CHAIN_H

/* End of chain.h */
