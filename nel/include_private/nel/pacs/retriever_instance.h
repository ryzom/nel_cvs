/** \file retriever_instance.h
 * 
 *
 * $Id: retriever_instance.h,v 1.5 2001/06/08 15:04:29 legros Exp $
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


/// Precision of Snap. 1/1024 meter. If you change this, CEdgeCollide::testPointMove() won't work.
const	float	SnapPrecision= 1024;


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

protected:
	friend class CGlobalRetriever;

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
		uint16	ThroughChain;

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
	mutable std::vector<uint8>			_RetrieveTable;

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
//	NLMISC::CAABBox						_BBox;

public:
	/// Default constructor.
	CRetrieverInstance();

	/// Resets the instance. This doesn't affect any neighboring instances...
	void								reset();
	/// Resets links of the instance. This doesn't affect any neighboring instances...
	void								resetLinks();
	/// Resets links of the instance on the given edge. This doesn't affect any neighboring instances...
	void								resetLinks(uint edge);

	/// Returns the id of this instance.
	sint32								getInstanceId() const { return _InstanceId; }
	/// Returns the id of the retriever associated to this instance.
	sint32								getRetrieverId() const { return _RetrieverId; }
	/// Returns the orientation of the instance in relation to the retriever.
	uint8								getOrientation() const { return _Orientation; }
	/// Returns the origin translation of this instance.
	NLMISC::CVector						getOrigin() const { return _Origin; }


	/// Gets the id of the neighbor on the edge.
	sint32								getNeighbor(uint edge) const { return _Neighbors[edge]; }

	/// Gets the ids of the neighbor tips on the given edge.
	const std::vector<uint16>			&getEdgeTipLinks(uint edge) const { return _EdgeTipLinks[edge]; }
	/// Gets the id of the nth neighbor tip on the given edge.
	uint16								getEdgeTipLink(uint edge, uint n) const { return _EdgeTipLinks[edge][n]; }

	/// Gets the ids of the neighbor chains on the given edge.
	const std::vector<uint16>			&getEdgeChainLinks(uint edge) const { return _EdgeChainLinks[edge]; }
	/// Gets the id of the nth neighbor chain on the given edge.
	uint16								getEdgeChainLink(uint edge, uint n) const { return _EdgeChainLinks[edge][n]; }

	/// Returns the number of the edge on the instance corresponding to the edge on the retriever.
	uint8								getInstanceEdge(uint8 retrieverEdge) const { return (retrieverEdge+_Orientation)%4; }
	/// Returns the number of the edge on the retriever corresponding to the edge on the instance.
	uint8								getRetrieverEdge(uint8 instanceEdge) const { return (instanceEdge+4-_Orientation)%4; }


	/// Initializes the instance.
	void								make(sint32 instanceId, sint32 retrieverId, const CLocalRetriever &retriever,
											 uint8 orientation, const NLMISC::CVector &origin);

	/// Links the instance to a given neighbor on the given edge.
	void								link(const CRetrieverInstance &neighbor, uint8 edge,
											 const std::vector<CLocalRetriever> &retrievers);

	/// Unlinks the given edge. The neighbor instance is AFFECTED.
	void								unlink(std::vector<CRetrieverInstance> &instances);


	/**
	 * Retrieves the position in the instance from an estimated position.
	 * WARNING: the estimated position is a GLOBAL position, and the returned position
	 * is a LOCAL position (to the retriever).
	 */
	CLocalRetriever::CLocalPosition		retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever) const;

	/// Serialises this CRetrieverInstance.
	void								serial(NLMISC::IStream &f);

	/// Computes the position in the local axis of the retriever from a global position.
	NLMISC::CVector						getLocalPosition(const NLMISC::CVector &globalPosition) const;
	/// Computes the position in the global axis from a local position (in the retriever axis).
	NLMISC::CVector						getGlobalPosition(const NLMISC::CVector &localPosition) const;
	/// Computes the position (as double) in the global axis from a local position (in the retriever axis).
	NLMISC::CVectorD					getDoubleGlobalPosition(const NLMISC::CVector &localPosition) const;

	/// Returns the bbox of the instance.
//	NLMISC::CAABBox						getBBox() { return _BBox; }


	/// \name Vector Snapping.
	// @{
	/** Snap a vector at 1mm (1/1024). v must be a local position (ie range from -80 to +240).
	 * Doing this, we are sure we have precision of 9+10 bits, which is enough for 24 bits float precision.
	 * NB: z is not snapped.
	 */
	static void	snapVector(CVector &v)
	{
		v.x= (float)floor(v.x*SnapPrecision)/SnapPrecision;
		v.y= (float)floor(v.y*SnapPrecision)/SnapPrecision;
	}
	/** Snap a vector at 1mm (1/1024). v must be a local position (ie range from -80 to +240).
	 * Doing this, we are sure we have precision of 9+10 bits, which is enough for 24 bits float precision.
	 */
	static void	snapVector(CVector2f &v)
	{
		v.x= (float)floor(v.x*SnapPrecision)/SnapPrecision;
		v.y= (float)floor(v.y*SnapPrecision)/SnapPrecision;
	}
	// @}

};

}; // NLPACS

#endif // NL_RETRIEVER_INSTANCE_H

/* End of retriever_instance.h */
