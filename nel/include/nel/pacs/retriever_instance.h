/** \file retriever_instance.h
 * 
 *
 * $Id: retriever_instance.h,v 1.4 2001/05/15 08:03:09 legros Exp $
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

#ifndef NL_RETRIEVER_INSTANCE_H
#define NL_RETRIEVER_INSTANCE_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "nel/pacs/local_retriever.h"

namespace NLPACS
{

/**
 * An instance of a local retriever.
 * It defines a physical/geographic zone of landscape.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CRetrieverInstance
{
public:
	class CSurfaceEdge
	{
	public:
		sint32							From;
		sint32							To;
	public:
		CSurfaceEdge(sint32 from=0, sint32 to=0) : From(from), To(to) {}
	};

//protected:
public:
	//friend class ...;

	/**
	 * The link to another node
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	struct CAStarNodeAccess
	{
		sint32	InstanceId;
		uint16	NodeId;

		bool	operator == (const CAStarNodeAccess &node) { return InstanceId == node.InstanceId && NodeId == node.NodeId; }
		bool	operator != (const CAStarNodeAccess &node) { return InstanceId != node.InstanceId || NodeId != node.NodeId; }
	};

	/**
	 * The information bound to the surfaces nodes.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	struct CAStarNodeInfo
	{
		/// The position of this node.
		NLMISC::CVector2f	Position;

		/// The cost to this node.
		float				Cost;
		float				F;

		/// The parent link.
		CAStarNodeAccess	Parent;
	};

	///
	std::vector<CAStarNodeInfo>			_NodesInformation;
	
	/// Used to retrieve the surface. Internal use only, to avoid large amount of new/delete
	std::vector<uint8>					_RetrieveTable;

protected:
	/// The id of this instance.
	sint32								_InstanceId;

	/// The id of the retrievable surface pattern.
	sint32								_RetrieverId;

	/// @name Instance displacement.
	//@{
	uint8								_Orientation;
	NLMISC::CVector						_Origin;
	//@}

	/// The instance ids of the neighbors.
	sint32								_Neighbors[4];

	/// The neighbor tips on each edge (in the corresponding neighbor.)
	std::vector<uint16>					_EdgeTipLinks[4];

	/// The neighbor  chains on each edge (cf tips.)
	std::vector<uint16>					_EdgeChainLinks[4];

	/// The BBox.
	NLMISC::CAABBox						_BBox;

public:
	CRetrieverInstance();

	void								reset();
	void								resetLinks();
	void								resetLinks(uint edge);

	sint32								getInstanceId() const { return _InstanceId; }
	sint32								getRetrieverId() const { return _RetrieverId; }
	uint8								getOrientation() const { return _Orientation; }
	NLMISC::CVector						getOrigin() const { return _Origin; }

	sint32								getNeighbor(uint edge) const { return _Neighbors[edge]; }
	const std::vector<uint16>			&getEdgeTipLinks(uint edge) const { return _EdgeTipLinks[edge]; }
	uint16								getEdgeTipLink(uint edge, uint n) const { return _EdgeTipLinks[edge][n]; }
	const std::vector<uint16>			&getEdgeChainLinks(uint edge) const { return _EdgeChainLinks[edge]; }
	uint16								getEdgeChainLink(uint edge, uint n) const { return _EdgeChainLinks[edge][n]; }

	void								make(sint32 instanceId, sint32 retrieverId, const CLocalRetriever &retriever,
											 uint8 orientation, const NLMISC::CVector &origin);

	void								link(const CRetrieverInstance &neighbor, uint8 edge,
											 const std::vector<CLocalRetriever> &retrievers);

	void								unlink(std::vector<CRetrieverInstance> &instances);

	CLocalRetriever::CLocalPosition		retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever);
	
	void								serial(NLMISC::IStream &f);

	NLMISC::CVector						getLocalPosition(const NLMISC::CVector &globalPosition) const;
	NLMISC::CVector						getGlobalPosition(const NLMISC::CVector &localPosition) const;
	NLMISC::CVectorD					getDoubleGlobalPosition(const NLMISC::CVector &localPosition) const;

	NLMISC::CAABBox						getBBox() { return _BBox; }
};

}; // NLPACS

#endif // NL_RETRIEVER_INSTANCE_H

/* End of retriever_instance.h */
