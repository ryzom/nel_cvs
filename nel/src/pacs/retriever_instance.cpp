/** \file retriever_instance.cpp
 *
 *
 * $Id: retriever_instance.cpp,v 1.17 2001/07/09 10:54:58 legros Exp $
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

#include "pacs/retriever_instance.h"

using namespace std;
using namespace NLMISC;



NLPACS::CRetrieverInstance::CRetrieverInstance()
{
	reset();
}


void	NLPACS::CRetrieverInstance::resetLinks()
{
	uint	i;
	// WARNING !!
	// this is a HARD reset !
	// only the instance i reset, no care about neighbors !!
	for (i=0; i<_Neighbors.size(); ++i)
		_Neighbors[i] = -1;
	_BorderChainLinks.clear();
}

void	NLPACS::CRetrieverInstance::resetLinks(uint32 id)
{
	vector<sint32>::iterator	rit;
	for (rit=_Neighbors.begin(); rit!=_Neighbors.end(); )
		if (*rit == (sint32)id)
			rit = _Neighbors.erase(rit);
		else
			++rit;

	uint	i;
	for (i=0; i<_BorderChainLinks.size(); ++i)
		if (_BorderChainLinks[i].Instance == (sint16)id)
		{
			_BorderChainLinks[i].Instance = -1;
			_BorderChainLinks[i].BorderChainId = -1;
			_BorderChainLinks[i].ChainId = -1;
			_BorderChainLinks[i].SurfaceId = -1;
		}
}

void	NLPACS::CRetrieverInstance::reset()
{
	// WARNING !!
	// this is a HARD reset !
	// only the instance i reset, no care about neighbors !!
	_RetrieveTable.clear();
	_NodesInformation.clear();
	_InstanceId = -1;
	_RetrieverId = -1;
	_Orientation = 0;
	_Origin = CVector::Null;
	resetLinks();
}

void	NLPACS::CRetrieverInstance::init(const CLocalRetriever &retriever)
{
	_RetrieveTable.resize(retriever.getSurfaces().size());
	_NodesInformation.resize(retriever.getSurfaces().size());
	uint	i;
	// Resets _RetrieveTable for later internal use (retrievePosition)
	for (i=0; i<_RetrieveTable.size(); ++i)
		_RetrieveTable[i] = 0;

	// Resets _NodesInformation for later pathfinding graph annotation.
	for (i=0; i<_NodesInformation.size(); ++i)
	{
		CVector	pos = getGlobalPosition(retriever.getSurfaces()[i].getCenter());
		_NodesInformation[i].Position = CVector2f(pos.x, pos.y);
	}

	_BorderChainLinks.resize(retriever.getBorderChains().size());
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

	init(retriever);
}

/* Links the current retriever instance to another instance
 * on the given edge.
 */
void	NLPACS::CRetrieverInstance::link(CRetrieverInstance &neighbor,
										 const vector<CLocalRetriever> &retrievers)
{
	uint	i, j;
	for (i=0; i<_Neighbors.size(); ++i)
		if (_Neighbors[i] == neighbor._InstanceId)
			return;

	const CLocalRetriever						&retriever = retrievers[_RetrieverId];
	const CLocalRetriever						&nRetriever = retrievers[neighbor._RetrieverId];

	const vector<CChain>						&chains = retriever.getChains(),
												&nChains = nRetriever.getChains();
	const vector<uint16>						&borderChains = retriever.getBorderChains(),
												&nBorderChains = nRetriever.getBorderChains();

	vector< pair<CVector,CVector> >				chainTips,
												nChainTips;

	for (i=0; i<borderChains.size(); ++i)
		chainTips.push_back(make_pair(retriever.getTip(chains[borderChains[i]].getStartTip()).Point,
									  retriever.getTip(chains[borderChains[i]].getStopTip()).Point));

	/// \todo Compute real position using _Orientation

	CVector	translation = neighbor._Origin - _Origin;

	for (i=0; i<nBorderChains.size(); ++i)
		nChainTips.push_back(make_pair(nRetriever.getTip(nChains[nBorderChains[i]].getStartTip()).Point+translation,
									   nRetriever.getTip(nChains[nBorderChains[i]].getStopTip()).Point+translation));

	for (i=0; i<borderChains.size(); ++i)
	{
		// if the chain is already linked, just step
		if (_BorderChainLinks[i].Instance != 0xffff || _BorderChainLinks[i].BorderChainId != 0xffff ||
			_BorderChainLinks[i].ChainId != 0xffff || _BorderChainLinks[i].SurfaceId != 0xffff)
			continue;

		float	bestDist = 1.0f;
		sint	best = -1;

		for (j=0; j<nBorderChains.size(); ++j)
		{
			if (neighbor._BorderChainLinks[j].Instance != 0xffff || neighbor._BorderChainLinks[j].BorderChainId != 0xffff ||
				neighbor._BorderChainLinks[j].ChainId != 0xffff || neighbor._BorderChainLinks[j].SurfaceId != 0xffff)
				continue;

			float	d = (chainTips[i].first-nChainTips[j].second).norm()+(chainTips[i].second-nChainTips[j].first).norm();
			if (d < bestDist)
			{
				bestDist = d;
				best = j;
			}
		}

		// if no best match, just don't link
		if (bestDist > 1.0e-1f || best == -1)
			continue;

		_BorderChainLinks[i].Instance = (uint16)neighbor._InstanceId;
		_BorderChainLinks[i].BorderChainId = (uint16)best;
		_BorderChainLinks[i].ChainId = nBorderChains[_BorderChainLinks[i].BorderChainId];
		_BorderChainLinks[i].SurfaceId = (uint16)nChains[_BorderChainLinks[i].ChainId].getLeft();

		neighbor._BorderChainLinks[best].Instance = (uint16)_InstanceId;
		neighbor._BorderChainLinks[best].BorderChainId = (uint16)i;
		neighbor._BorderChainLinks[best].ChainId = borderChains[neighbor._BorderChainLinks[best].BorderChainId];
		neighbor._BorderChainLinks[best].SurfaceId = (uint16)chains[neighbor._BorderChainLinks[best].ChainId].getLeft();
	}

	_Neighbors.push_back(neighbor._InstanceId);
	neighbor._Neighbors.push_back(_InstanceId);
}


void	NLPACS::CRetrieverInstance::unlink(vector<CRetrieverInstance> &instances)
{
	uint	i;

	for (i=0; i<_Neighbors.size(); ++i)
		instances[_Neighbors[i]].resetLinks(_InstanceId);
}




NLPACS::CLocalRetriever::CLocalPosition	NLPACS::CRetrieverInstance::retrievePosition(const NLMISC::CVector &estimated, const CLocalRetriever &retriever) const
{
	CVector							localEstimated;
	CLocalRetriever::CLocalPosition	retrieved;

	// get local coordinates
	localEstimated = getLocalPosition(estimated);
	// Yoyo: must snap vector.
	CRetrieverInstance::snapVector(localEstimated);

	// fills _RetrieveTable by retrievingPosition.
	retriever.retrievePosition(localEstimated, _RetrieveTable);

	uint	surf;
	sint	bestSurf = -1;
	sint	lastSurf = -1;
	float	bestDistance = 1.0e10f;
	float	bestHeight;

	// for each surface in the retriever
	for (surf=0; surf<_RetrieveTable.size(); ++surf)
	{
		// if the surface contains the estimated position.
		if (_RetrieveTable[surf] != 0)
		{
			// at least remembers the last seen surface...
			lastSurf = surf;
			_RetrieveTable[surf] = 0;
			// search in the surface's quad tree for the actual height
			const CQuadLeaf	*leaf = retriever.getSurfaces()[surf].getQuadTree().getLeaf(localEstimated);
			// if there is no acceptable leaf, just give up
			if (leaf == NULL)
				continue;

			// computes the mean height of the leaf, and remembers the surface
			// if it is closer to the estimation than the previous remembered...
			float	meanHeight = leaf->getMaxHeight();
			float	distance = (float)fabs(localEstimated.z-meanHeight);
			if (distance < bestDistance && localEstimated.z > leaf->getMinHeight()-2.0f && localEstimated.z < leaf->getMaxHeight()+2.0f)
			{
				bestDistance = distance;
				bestHeight = meanHeight;
				bestSurf = surf;
			}
		}
	}

	if (bestSurf != -1)
	{
		// if there is a best surface, returns it
		retrieved.Surface = bestSurf;
		retrieved.Estimation = CVector(localEstimated.x, localEstimated.y, bestHeight);
	}
	else
	{
		// else return the last remembered...
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
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
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
}

CVector	NLPACS::CRetrieverInstance::getGlobalPosition(const CVector &localPosition) const
{
	switch (_Orientation)
	{
	default:
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
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
}

CVectorD	NLPACS::CRetrieverInstance::getDoubleGlobalPosition(const CVector &localPosition) const
{
	switch (_Orientation)
	{
	default:
		nlwarning("in NLPACS::CRetrieverInstance::getLocalPosition()");
		nlerror("unexpected orientation value (%d)", _Orientation);
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
}





void	NLPACS::CRetrieverInstance::serial(NLMISC::IStream &f)
{
	uint	i;
	f.serial(_InstanceId, _RetrieverId, _Orientation, _Origin);
	f.serialCont(_Neighbors);
	f.serialCont(_BorderChainLinks);

	// serialises the number of nodes
	uint16	totalNodes = _RetrieveTable.size();
	f.serial(totalNodes);
	if (f.isReading())
	{
		// if the stream is reading, reinits the temps tables...
		_RetrieveTable.resize(totalNodes);
		_NodesInformation.resize(totalNodes);
		for (i=0; i<_RetrieveTable.size(); ++i)
			_RetrieveTable[i] = 0;
	}
}
