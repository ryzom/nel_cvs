/** \file retriever_instance.cpp
 *
 *
 * $Id: retriever_instance.cpp,v 1.6 2001/05/17 09:05:10 legros Exp $
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
	reset();
}

void	NLPACS::CRetrieverInstance::resetLinks()
{
	uint	i;
	for (i=0; i<4; ++i)
	{
		_Neighbors[i] = -1;
		_EdgeTipLinks[i].clear();
		_EdgeChainLinks[i].clear();
	}
}

void	NLPACS::CRetrieverInstance::resetLinks(uint edge)
{
	_Neighbors[edge] = -1;
	_EdgeTipLinks[edge].clear();
	_EdgeChainLinks[edge].clear();
}

void	NLPACS::CRetrieverInstance::reset()
{
	_RetrieveTable.clear();
	_InstanceId = -1;
	_RetrieverId = -1;
	_Orientation = 0;
	_Origin = CVector::Null;
	_BBox.setHalfSize(CVector::Null);
	_BBox.setCenter(CVector::Null);
	resetLinks();
}

void	NLPACS::CRetrieverInstance::make(sint32 instanceId, sint32 retrieverId, const CLocalRetriever &retriever,
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
	_RetrieveTable.resize(retriever.getSurfaces().size());
	_NodesInformation.resize(retriever.getSurfaces().size());
	uint	i;
	for (i=0; i<_RetrieveTable.size(); ++i)
		_RetrieveTable[i] = 0;

	for (i=0; i<_NodesInformation.size(); ++i)
	{
		CVector	pos = getGlobalPosition(retriever.getSurfaces()[i].getCenter());
		_NodesInformation[i].Position = CVector2f(pos.x, pos.y);
	}

	_BBox = retriever.getBBox();
	_BBox.setCenter(getGlobalPosition(_BBox.getCenter()));
}

/* Links the current retriever instance to another instance
 * on the given edge.
 */
void	NLPACS::CRetrieverInstance::link(const CRetrieverInstance &neighbor, uint8 edge,
										 const vector<CLocalRetriever> &retrievers)
{
	uint	nEdge = (edge+2)%4;

	// First check if there is no previous link
	if (_Neighbors[edge] != -1 && _Neighbors[edge] != neighbor._InstanceId)
	{
		nlwarning("in call to NLPACS::CRetrieverInstance::link");
		nlwarning("_InstanceId=%d _RetrieverId=%d _Neighbors[%d]=%d", _InstanceId, _RetrieverId, edge, _Neighbors[edge]);
		nlwarning("neighbor._InstanceId=%d", neighbor._InstanceId);
		nlerror("Neighbor %d has already been set to %d in instance %d", edge, _Neighbors[edge], _InstanceId);
	}

	if (_Neighbors[edge] == neighbor._InstanceId)
		return;

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
		CVector	point = neighbor.getLocalPosition(getGlobalPosition(tips[edgeTips[i]].Point));
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

		_EdgeChainLinks[edge].push_back(nEdgeChains[j]);
	}
}

void	NLPACS::CRetrieverInstance::unlink(vector<CRetrieverInstance> &instances)
{
	uint	edge;

	for (edge=0; edge<4; ++edge)
	{
		if (_Neighbors[edge] != -1)
		{
			CRetrieverInstance	&neighbor = instances[_Neighbors[edge]];
			neighbor.resetLinks((edge+2)%4);
		}
		resetLinks(edge);
	}
}


NLPACS::CLocalRetriever::CLocalPosition	NLPACS::CRetrieverInstance::retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever)
{
	CVector							localEstimated;
	CLocalRetriever::CLocalPosition	retrieved;

	// !!!!!! MODIFY THIS !!!!!!
	localEstimated = getLocalPosition(estimated);

	retriever.retrievePosition(localEstimated, _RetrieveTable);

	uint	surf;
	sint	bestSurf = -1;
	sint	lastSurf = -1;
	float	bestDistance = 1.0e10f;
	float	bestMeanHeight;

	for (surf=0; surf<_RetrieveTable.size(); ++surf)
	{
		if (_RetrieveTable[surf] != 0)
		{
			lastSurf = surf;
			_RetrieveTable[surf] = 0;
			const CQuadLeaf	*leaf = retriever.getSurfaces()[surf].getQuadTree().getLeaf(localEstimated);
			if (leaf == NULL)
				continue;

			float	meanHeight = (leaf->getMinHeight()+leaf->getMaxHeight())*0.5f;
			float	distance = (float)fabs(localEstimated.z-meanHeight);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestMeanHeight = meanHeight;
				bestSurf = surf;
			}
		}
	}

	if (bestSurf != -1)
	{
		retrieved.Surface = bestSurf;
		retrieved.Estimation = CVector(localEstimated.x, localEstimated.y, bestMeanHeight);
	}
	else
	{
		retrieved.Surface = lastSurf;
		retrieved.Estimation = localEstimated;
	}

	return retrieved;
}

CVector	NLPACS::CRetrieverInstance::getLocalPosition(const CVector &globalPosition) const
{
	switch (_Orientation)
	{
	default:
	case 0:
		return CVector(+globalPosition.x-_Origin.x, +globalPosition.y-_Origin.y, globalPosition.z-_Origin.z);
		break;
	case 1:
		return CVector(+globalPosition.y-_Origin.y, -globalPosition.x+_Origin.x, globalPosition.z-_Origin.z);
		break;
	case 2:
		return CVector(-globalPosition.x+_Origin.x, -globalPosition.y+_Origin.y, globalPosition.z-_Origin.z);
		break;
	case 3:
		return CVector(-globalPosition.y+_Origin.y, +globalPosition.x-_Origin.x, globalPosition.z-_Origin.z);
		break;
	}
/*
	static const float	cosTable[4] = { 1.0f, 0.0f, -1.0f, 0.0f };
	static const float	sinTable[4] = { 0.0f, 1.0f, 0.0f, -1.0f };

	float	s = sinTable[_Orientation],
			c = cosTable[_Orientation];

	return CVector(	+c*(globalPosition.x-_Origin.x)+s*(globalPosition.y-_Origin.y),
					-s*(globalPosition.x-_Origin.x)+c*(globalPosition.y-_Origin.y),
					+globalPosition.z-_Origin.z );
*/
}

CVector	NLPACS::CRetrieverInstance::getGlobalPosition(const CVector &localPosition) const
{
	switch (_Orientation)
	{
	default:
	case 0:
		return CVector(+localPosition.x+_Origin.x, +localPosition.y+_Origin.y, localPosition.z+_Origin.z );
		break;
	case 1:
		return CVector(-localPosition.y+_Origin.x, +localPosition.x+_Origin.y, localPosition.z+_Origin.z );
		break;
	case 2:
		return CVector(-localPosition.x+_Origin.x, -localPosition.y+_Origin.y, localPosition.z+_Origin.z );
		break;
	case 3:
		return CVector(+localPosition.y+_Origin.x, -localPosition.x+_Origin.y, localPosition.z+_Origin.z );
		break;
	}
/*
	static const float	cosTable[4] = { 1.0f, 0.0f, -1.0f, 0.0f };
	static const float	sinTable[4] = { 0.0f, 1.0f, 0.0f, -1.0f };

	float	s = sinTable[_Orientation],
			c = cosTable[_Orientation];

	return CVector(	+c*localPosition.x-s*localPosition.y+_Origin.x,
					+s*localPosition.x+c*localPosition.y+_Origin.y,
					localPosition.z+_Origin.z );
*/
}

CVectorD	NLPACS::CRetrieverInstance::getDoubleGlobalPosition(const CVector &localPosition) const
{
	switch (_Orientation)
	{
	default:
	case 0:
		return CVectorD(+(double)localPosition.x+(double)_Origin.x, +(double)localPosition.y+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	case 1:
		return CVectorD(-(double)localPosition.y+(double)_Origin.x, +(double)localPosition.x+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	case 2:
		return CVectorD(-(double)localPosition.x+(double)_Origin.x, -(double)localPosition.y+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	case 3:
		return CVectorD(+(double)localPosition.y+(double)_Origin.x, -(double)localPosition.x+(double)_Origin.y, (double)localPosition.z+(double)_Origin.z );
		break;
	}
/*
	static const float	sinTable[4] = { 0.0f, 1.0f, 0.0f, -1.0f };
	static const float	cosTable[4] = { 1.0f, 0.0f, -1.0f, 0.0f };

	double	s = sinTable[_Orientation],
			c = cosTable[_Orientation];

	return CVectorD( +c*localPosition.x-s*localPosition.y+_Origin.x,
					 +s*localPosition.x+c*localPosition.y+_Origin.y,
					 (double)(localPosition.z)+(double)(_Origin.z) );
*/
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

	uint16	totalNodes = _RetrieveTable.size();
	f.serial(totalNodes);
	if (f.isReading())
	{
		_RetrieveTable.resize(totalNodes);
		_NodesInformation.resize(totalNodes);
		for (i=0; i<_RetrieveTable.size(); ++i)
			_RetrieveTable[i] = 0;
	}
}
