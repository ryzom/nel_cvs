/** \file local_retriever.h
 * 
 *
 * $Id: local_retriever.h,v 1.3 2001/05/09 12:59:24 legros Exp $
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

#ifndef NL_LOCAL_RETRIEVER_H
#define NL_LOCAL_RETRIEVER_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "nel/pacs/surface_quad.h"
#include "nel/pacs/chain.h"
#include "nel/pacs/retrievable_surface.h"

namespace NLPACS
{

/**
 * A surface retriever, located by its bounding box.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CLocalRetriever
{
public:
	/**
	 * A tip of several chains. A CTip can contain more than 2 chains.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CTip
	{
	public:
		struct CChainTip
		{
			sint32	Chain;
			bool	Start;
			CChainTip(sint32 chainId = 0, bool start = 0) : Chain(chainId), Start(start) {}
			void	serial(NLMISC::IStream &f) { f.serial(Chain, Start); }
		};

		/// The position of the tip.
		NLMISC::CVector					Point;

		// The chains linked to that tip.
		std::vector<CChainTip>			Chains;

		/// The edges on which is located the tip (marked with a 1 bit)
		uint8							Edges;

	public:
		void	serial(NLMISC::IStream &f)
		{
			f.serial(Point);
			f.serialCont(Chains);
		}
	};


	/**
	 * A topology. It's a set of surfaces which are definning a connex master surface.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CTopology : public std::vector<sint32>
	{
	public:
		void	serial(NLMISC::IStream &f)	{ f.serialCont(*this); }
	};


	/**
	 * An estimation of the position of a point on a specified surface.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CPosition
	{
	public:
		sint32							Surface;
		NLMISC::CVector					Estimation;
	public:
		void							serial(NLMISC::IStream &f) { f.serial(Surface, Estimation); }
	};


protected:
	friend class	CRetrieverInstance;
	
	/// The chains insinde the zone.
	std::vector<COrderedChain>			_OrderedChains;

	/// The chains insinde the zone.
	std::vector<CChain>					_Chains;

	/// The surfaces inside the zone.
	std::vector<CRetrievableSurface>	_Surfaces;

	/// The tips making links between different chains.
	std::vector<CTip>					_Tips;

	/// The axis aligned bounding box of the zone.
	NLMISC::CAABBox						_BBox;

	/// The id of the zone.
	sint32								_ZoneId;

	/// The tips on the edges of the zone.
	std::vector<uint16>					_EdgeTips[4];

	/// The chains on the edges of the zone.
	std::vector<uint16>					_EdgeChains[4];

	/// The topologies within the zone.
	std::vector<CTopology>				_Topologies[NumCreatureModels];

public:
	struct CXPred
	{
		const std::vector<CTip>			*Tips;
		bool							Reverse;
		CXPred(const std::vector<CTip> *tips, bool reverse=false) : Tips(tips), Reverse(reverse) {}
		bool							operator() (uint16 a, uint16 b) const
		{
			return (Reverse) ?
				(*Tips)[a].Point.x > (*Tips)[b].Point.x :
				(*Tips)[a].Point.x < (*Tips)[b].Point.x;
		}
	};

	struct CYPred
	{
		const std::vector<CTip>			*Tips;
		bool							Reverse;
		CYPred(const std::vector<CTip> *tips, bool reverse=false) : Tips(tips), Reverse(reverse) {}
		bool							operator() (uint16 a, uint16 b) const
		{
			return (Reverse) ?
				(*Tips)[a].Point.y > (*Tips)[b].Point.y :
				(*Tips)[a].Point.y < (*Tips)[b].Point.y;
		}
	};

public:
	void								setBBox(const NLMISC::CAABBox &box) { _BBox = box; }
	void								setZoneId (sint32 id) { _ZoneId = id; }
	const NLMISC::CAABBox				&getBBox() const { return _BBox; }
	sint32								getZoneId() const { return _ZoneId; }
	const std::vector<CTip>				&getTips() const { return _Tips; }
	const std::vector<uint16>			&getEdgeTips(sint edge) const { nlassert(0<=edge && edge<4); return _EdgeTips[edge]; }

	const std::vector<COrderedChain>	&getOrderedChains() const { return _OrderedChains; }
	const std::vector<CChain>			&getChains() const { return _Chains; }
	const std::vector<uint16>			&getEdgeChains(sint edge) const { nlassert(0<=edge && edge<4); return _EdgeChains[edge]; }
	const std::vector<CRetrievableSurface>	&getSurfaces() const { return _Surfaces; }

	sint32								addSurface(uint8 normalq, uint8 orientationq,
												   uint8 mat, uint8 charact, uint8 level,
												   const CSurfaceQuadTree &quad);

	sint32								addChain(const std::vector<NLMISC::CVector> &vertices,
												 sint32 left, sint32 right);

	void								computeTopologies();

	void								sortTips();

	void								findEdgeTips();
	void								findEdgeChains();

	void								serial(NLMISC::IStream &f);

protected:
	friend class	CRetrieverInstance;

	void								retrievePosition(NLMISC::CVector estimated, std::vector<uint8> &retrieveTable) const;
	
};

}; // NLPACS

#endif // NL_LOCAL_RETRIEVER_H

/* End of local_retriever.h */
