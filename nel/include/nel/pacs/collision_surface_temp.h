/** \file collision_surface_temp.h
 * Temp collision data used during resolution of collision within surfaces.
 *
 * $Id: collision_surface_temp.h,v 1.1 2001/05/15 13:36:58 berenguier Exp $
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

#ifndef NL_COLLISION_SURFACE_TEMP_H
#define NL_COLLISION_SURFACE_TEMP_H

#include "nel/misc/types_nl.h"
#include "nel/pacs/edge_collide.h"
#include "nel/pacs/collision_desc.h"


namespace NLPACS 
{


// ***************************************************************************
/**
 * Temp collision data used during tryMove().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CEdgeCollideNode : public CEdgeCollide
{
public:
	/// Next edgeCollideNode in the CCollisionSurfaceTemp allocator. 0xFFFFFFFF if none.
	uint32			Next;

public:
	CEdgeCollideNode()
	{
		Next= 0xFFFFFFFF;
	}
};


// ***************************************************************************
/**
 * Temp collision data used during tryMove().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionChain
{
public:
	/// First edgeCollideNode in the CCollisionSurfaceTemp allocator. 0xFFFFFFFF if none. This is a List of edgeCollide.
	uint32			FirstEdgeCollide;
	/// The Left/Right surface next this chain.
	CSurfaceIdent	LeftSurface, RightSurface;
	/// the id in the local retriever which generate this chain (temp).
	uint16			ChainId;
	/// In the algorithm, this chain has been tested???
	bool			Tested;

public:
	CCollisionChain()
	{
		FirstEdgeCollide= 0xFFFFFFFF;
		Tested= false;
	}
};


// ***************************************************************************
/**
 * Temp collision data used during tryMove().
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CEdgeChainEntry
{
public:
	/// The id of the ordered chain.
	uint16		OChainId;
	/// the first edge of the ordered chain, found in this quad.
	uint16		EdgeStart;
	/// the end edge of the ordered chain, found in this quad. "end edge" is lastEdge+1: numEdges= end-start.
	uint16		EdgeEnd;
};



// ***************************************************************************
/**
 * Temp collision data used during resolution of collision within surfaces. There should be one CCollisionSurfaceTemp
 * per thread. This is a private class, in essence.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionSurfaceTemp
{
public:
	/// For CChainQuad::selectEdges().
	uint16							OChainLUT[65536];
	std::vector<CEdgeChainEntry>	EdgeChainEntries;


	/// Array of near Collision Chains.
	std::vector<CCollisionChain>	CollisionChains;


	/// Result of collision tryMove().
	std::vector<CCollisionSurfaceDesc>	CollisionDescs;


public:

	/// Constructor
	CCollisionSurfaceTemp();

	/// \name Access to EdgeCollideNode
	// @{
	void				resetEdgeCollideNodes();
	/// return first Id.
	uint32				allocEdgeCollideNode(uint32 size=1);
	CEdgeCollideNode	&getEdgeCollideNode(uint32 id);
	// @}


private:
	/// Allocator of EdgeCollideNode.
	std::vector<CEdgeCollideNode>		_EdgeCollideNodes;

};


} // NLPACS


#endif // NL_COLLISION_SURFACE_TEMP_H

/* End of collision_surface_temp.h */
