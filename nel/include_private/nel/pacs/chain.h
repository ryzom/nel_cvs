/** \file chain.h
 * 
 *
 * $Id: chain.h,v 1.5 2001/06/07 08:23:36 legros Exp $
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
#include "nel/pacs/vector_2s.h"
#include "nel/misc/file.h"

namespace NLPACS
{

/**
 * A list of ordered vertices, partially delimiting 2 different surfaces.
 * In the vertex list, we consider the following order
 *    v1 < v2 iff  v1.x < v2.x  ||  v1.x == v2.x && v1.y < v2.y  ||  v1.x == v2.x && v1.y == v2.y && v1.z < v2.z
 * The vertices composing the chain are actual CVector (12 bytes per vertex.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class COrderedChain3f
{
protected:
	friend class CChain;
	friend class CRetrievableSurface;

	/// The vertices of the chain, ordered following x growth.
	std::vector<NLMISC::CVector>		_Vertices;

	/// Set if the chain should be read forward within the parent CChain (for sequential access to vertices.)
	bool								_Forward;

	/// The parent chain Id.
	uint16								_ParentId;

	/// The index of the ochain within the parent chain
	uint16								_IndexInParent;

public:
	/// Returns the vertices of the chain
	const std::vector<NLMISC::CVector>	&getVertices() const { return _Vertices; }

	/// Returns true if the chain should be accessed forward within the parent CChain (see _Forward.)
	bool								isForward() const { return _Forward; }

	/// Returns the parent chain Id of this ordered chain.
	uint16								getParentId() const { return _ParentId; }

	/// Returns the index of the ochain within the parent chain.
	uint16								getIndexInParent() const { return _IndexInParent; }

	///
	const NLMISC::CVector				&operator[] (uint n) const { return _Vertices[n]; }

	void								translate(const NLMISC::CVector &translation)
	{
		uint	i;
		for (i=0; i<_Vertices.size(); ++i)
			_Vertices[i] += translation;
	}

	void								serial(NLMISC::IStream &f);
};

/**
 * A list of ordered vertices, partially delimiting 2 different surfaces.
 * In the vertex list, we consider the following order
 *    v1 < v2 iff  v1.x < v2.x  ||  v1.x == v2.x && v1.y < v2.y
 * The vertices composing the chain are only 2 coordinates (x, y) wide, packed on 16 bits each
 * (which is 4 bytes per vertex.) This is the packed form of the COrderedChain3f.
 * (4 bytes per vertex.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class COrderedChain
{
protected:
	friend class CChain;
	friend class CRetrievableSurface;

	/// The vertices of the chain, ordered following x growth.
	std::vector<CVector2s>				_Vertices;

	/// Set if the chain should be read forward within the parent CChain (for sequential access to vertices.)
	bool								_Forward;

	/// The parent chain Id.
	uint16								_ParentId;

	/// The index of the ochain within the parent chain
	uint16								_IndexInParent;

	/// The length of the chain.
	float								_Length;

public:
	/// Returns the vertices of the chain
	const std::vector<CVector2s>		&getVertices() const { return _Vertices; }

	/// Returns true if the chain should be accessed forward within the parent CChain (see _Forward.)
	bool								isForward() const { return _Forward; }

	/// Returns the parent chain Id of this ordered chain.
	uint16								getParentId() const { return _ParentId; }

	/// Returns the index of the ochain within the parent chain.
	uint16								getIndexInParent() const { return _IndexInParent; }

	///
	const CVector2s						&operator[] (uint n) const { return _Vertices[n]; }

	void								translate(const NLMISC::CVector &translation);

	void								pack(const COrderedChain3f &chain)
	{
		uint	i;
		const std::vector<NLMISC::CVector>	&vertices = chain.getVertices();
		_Vertices.resize(vertices.size());
		_Forward = chain.isForward();
		_ParentId = chain.getParentId();
		for (i=0; i<vertices.size(); ++i)
			_Vertices[i] = CVector2s(vertices[i]);
	}

	void								traverse(sint from, sint to, bool forward, std::vector<CVector2s> path) const;


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

	/// The tips indexes in the retriever object.
	uint16								_StartTip;
	uint16								_StopTip;

	/// The edge on which the chain is stuck (-1 if not stuck on any edge)
	sint8								_Edge;

	/// The length of the whole chain.
	float								_Length;

	uint8								_LeftLoop, _LeftLoopIndex;
	uint8								_RightLoop, _RightLoopIndex;

protected:
	friend class CRetrievableSurface;
	friend class CLocalRetriever;

	/// Build the whole surface from a vector of CVector and the left and right surfaces.
	void								make(const std::vector<NLMISC::CVector> &vertices, sint32 left, sint32 right, std::vector<COrderedChain> &chains, uint16 thisId, sint edges,
											 std::vector<COrderedChain3f> &fullChains);

	void								setIndexOnEdge(uint edge, sint32 index);

	void								setLoopIndexes(sint32 surface, uint loop, uint loopIndex)
	{
		if (_Left == surface)
		{
			_LeftLoop = loop;
			_LeftLoopIndex = loopIndex;
		}
		else
		{
			_RightLoop = loop;
			_RightLoopIndex = loopIndex;
		}
	}

public:

	/// Constructor.
	CChain() :	_Left(-1), _Right(-1), _StartTip(0xffff), _StopTip(0xffff), _Edge(-1), _Length(0.0f),
				_LeftLoop(0), _LeftLoopIndex(0), _RightLoop(0), _RightLoopIndex(0) {}

	/// Returns a vector of ordered chain ids that compose the entire chain.
	const std::vector<uint16>			&getSubChains() const { return _SubChains; }

	/// Returns the id of the nth ordered chain that composes the chain.
	uint16								getSubChain(uint n) const { return _SubChains[n]; }

	/// Returns the left surface id.
	sint32								getLeft() const { return _Left; }
	uint8								getLeftLoop() const { return _LeftLoop; }
	uint8								getLeftLoopIndex() const { return _LeftLoopIndex; }

	/// Returns the right surface id.
	sint32								getRight() const { return _Right; }
	uint8								getRightLoop() const { return _RightLoop; }
	uint8								getRightLoopIndex() const { return _RightLoopIndex; }

	/// returns the legnth of the whole chain.
	float								getLength() const { return _Length; }

	/// Gets the index of the chain on the given edge (in the local retriever object.)
	sint32								getIndexOnEdge(sint edge) const
	{
		return (_Edge == edge && isEdgeId(_Right)) ? convertEdgeId(_Right) : -1;
	}

	/// Returns true iff  the given id corresponds to a link on an edge.
	static bool							isEdgeId(sint32 id) { return id <= -256; }

	/// Converts the edge id into the real index to the link (in the EdgeChainLinks of the CRetrieverInstance.)
	static sint32						convertEdgeId(sint32 id) { return -(id+256); }

	/// Returns the id of the start tip of the chain.
	uint16								getStartTip() const { return _StartTip; }

	/// Returns the id of the end tip of the chain.
	uint16								getStopTip() const { return _StopTip; }

	/// Returns the number of the retriever's edge on which the chain is stuck (-1 if none.)
	sint8								getEdge() const { return _Edge; }

	/// Serialises the CChain object.
	void								serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_CHAIN_H

/* End of chain.h */
