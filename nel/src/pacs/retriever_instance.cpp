/** \file retriever_instance.cpp
 *
 *
 * $Id: retriever_instance.cpp,v 1.1 2001/05/04 14:51:21 legros Exp $
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

#include "nel/pacs/retriever_instance.h"

using namespace std;
using namespace NLMISC;



NLPACS::CRetrieverInstance::CRetrieverInstance()
{
	uint	i;
	_InstanceId = -1;
	_RetrieverId = -1;
	_Orientation = 0;
	_Origin = CVector::Null;
	for (i=0; i<4; ++i)
		_Neighbors[i] = -1;
}

void	NLPACS::CRetrieverInstance::make(sint32 instanceId, sint32 retrieverId,
										 uint8 orientation, const CVector &origin)
{
	if (_InstanceId != -1 || _RetrieverId != -1)
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::make");
		nlwarning("_InstanceId=%d _RetrieverId=%d _Orientation=%d", _InstanceId, _RetrieverId, _Orientation);
		nlwarning("instanceId=%d retrieverId=%d orientation=%d", instanceId, retrieverId, orientation);
		nlerror("Retriever instance %d has already been set", _InstanceId);
	}
	
	_InstanceId = instanceId;
	_RetrieverId = retrieverId;
	_Orientation = (orientation%4);
	_Origin = origin;
}

/* Links the current retriever instance to another instance
 * on the given edge.
 */
void	NLPACS::CRetrieverInstance::link(const CRetrieverInstance &neighbor, uint8 edge,
										 const vector<CLocalRetriever> &retrievers)
{
	uint	nEdge = (edge+2)%4;

	// First check if there is no previous link
	if (_Neighbors[edge] != -1)
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::link");
		nlwarning("_InstanceId=%d _RetrieverId=%d _Neighbors[%d]=%d", _InstanceId, _RetrieverId, edge, _Neighbors[edge]);
		nlwarning("neighbor._InstanceId=%d", neighbor._InstanceId);
		nlerror("Neighbor %d has already been set to %d in instance %d", edge, _Neighbors[edge], _InstanceId);
	}

	if (neighbor._Neighbors[nEdge] != -1 && neighbor._Neighbors[nEdge] != _InstanceId)
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::link");
		nlerror("Neighbor %d (instance %d) has already a neighbor (instance %d) on edge %d", edge, neighbor._InstanceId, neighbor._Neighbors[nEdge], nEdge);
	}

	_Neighbors[edge] = neighbor._InstanceId;

	const CLocalRetriever	&retriever = retrievers[_RetrieverId];
	const CLocalRetriever	&nRetriever = retrievers[neighbor._RetrieverId];

	uint	retrieverEdge = (edge+4-_Orientation)%4;
	uint	nRetrieverEdge = (nEdge+4-neighbor._Orientation)%4;

	const vector<CLocalRetriever::CTip>			&tips = retriever.getTips(),
												&nTips = nRetriever.getTips();
	const vector<uint16>						&edgeTips = retriever.getEdgeTips(retrieverEdge),
												&nEdgeTips = nRetriever.getEdgeTips(nRetrieverEdge);

	if (edgeTips.size() != nEdgeTips.size())
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::link");
		nlerror("Instance %d and instance %d have different number of tips on common edge", _InstanceId, neighbor._InstanceId);
	}

	uint	i, j;

	/* WARNING !!!!   --- TO DO ---
	 * TRANSFORM THE VERTICES FROM THE LOCAL AXIS TO THE GLOBAL AXIS !!!
	 */

	for (i=0; i<edgeTips.size(); ++i)
	{
		CVector	point = tips[edgeTips[i]].Point;
		sint	bestTip = -1;
		float	bestDistance = 1.0e10f;

		for (j=0; j<nEdgeTips.size(); ++j)
		{
			float	d = (nTips[nEdgeTips[j]].Point-point).norm();
			if (d < bestDistance)
			{
				bestDistance = d;
				bestTip = nEdgeTips[j];
			}
		}

		if (bestDistance > 1.0f)
		{
			nlwarning("in call to NLPACS::CRetrieverInstance::link");
			nlerror("Impossible to match tip");
		}

		_EdgeTipLinks[edge].push_back(bestTip);
	}

	const vector<CChain>						&chains = retriever.getChains(),
												&nChains = nRetriever.getChains();
	const vector<uint16>						&edgeChains = retriever.getEdgeChains(retrieverEdge),
												&nEdgeChains = nRetriever.getEdgeChains(nRetrieverEdge);

	if (edgeChains.size() != nEdgeChains.size())
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::link");
		nlerror("Instance %d and instance %d have different number of chains on common edge", _InstanceId, neighbor._InstanceId);
	}

	for (i=0; i<edgeChains.size(); ++i)
	{
		uint16	tip0 = chains[edgeChains[i]].getStartTip(),
				tip1 = chains[edgeChains[i]].getStopTip();

		for (j=0; j<edgeTips.size() && edgeTips[j]!=tip0; ++j)	;
		tip0 = _EdgeTipLinks[edge][j];
		for (j=0; j<edgeTips.size() && edgeTips[j]!=tip1; ++j)	;
		tip1 = _EdgeTipLinks[edge][j];

		bool	found = false;

		for (j=0; j<nEdgeChains.size(); ++j)
		{
			uint16	nTip0 = nChains[nEdgeChains[j]].getStartTip(),
					nTip1 = nChains[nEdgeChains[j]].getStopTip();
			
			if ((nTip0 == tip0 && nTip1 == tip1) || (nTip0 == tip1 && nTip1 == tip0))
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			nlwarning("in call to NLPACS::CRetrieverInstance::link");
			nlerror("Instance %d and instance %d have different number of tips on common edge", _InstanceId, neighbor._InstanceId);
		}

		_EdgeChainLinks[edge].push_back(j);
	}
}

void	NLPACS::CRetrieverInstance::serial(NLMISC::IStream &f)
{
	uint	i;
	f.serial(_InstanceId, _RetrieverId, _Orientation, _Origin);
	for (i=0; i<4; ++i)
	{
		f.serial(_Neighbors[i]);
		f.serialCont(_EdgeTipLinks[i]);
		f.serialCont(_EdgeChainLinks[i]);
	}
}
