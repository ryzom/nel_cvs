/** \file global_retriever.cpp
 *
 *
 * $Id: global_retriever.cpp,v 1.2 2001/05/15 08:02:55 legros Exp $
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
#include <map>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/vector_2f.h"

#include "nel/misc/debug.h"

#include "nel/pacs/global_retriever.h"
#include "nel/pacs/retriever_bank.h"

using namespace std;
using namespace NLMISC;

NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::getInstanceFullAccess(const CVector &position)
{
	CVector			offset = _BBox.getMin()-position;
	const float		zdim = 160.0f;
	return getInstanceFullAccess((uint)(offset.x/zdim), (uint)(offset.y/zdim));
}

void	NLPACS::CGlobalRetriever::serial(NLMISC::IStream &f)
{
	f.serialCont(_Instances);
	f.serial(_Width, _Height);
	f.serial(_BBox);
}

void	NLPACS::CGlobalRetriever::makeLinks(uint n)
{
	uint	x, y;
	convertId(n, x, y);

	if (x > 0 && _Instances[n-1].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n-1], 0, _RetrieverBank->getRetrievers());
		_Instances[n-1].link(_Instances[n], 2, _RetrieverBank->getRetrievers());
	}

	if (y < (uint)(_Height-1) && _Instances[n-_Width].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n-_Width], 1, _RetrieverBank->getRetrievers());
		_Instances[n-_Width].link(_Instances[n], 3, _RetrieverBank->getRetrievers());
	}

	if (x < (uint)(_Width-1) && _Instances[n+1].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n+1], 2, _RetrieverBank->getRetrievers());
		_Instances[n+1].link(_Instances[n], 0, _RetrieverBank->getRetrievers());
	}

	if (y > 0 && _Instances[n+_Width].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n+_Width], 3, _RetrieverBank->getRetrievers());
		_Instances[n+_Width].link(_Instances[n], 1, _RetrieverBank->getRetrievers());
	}
}

void	NLPACS::CGlobalRetriever::resetAllLinks()
{
	uint	n;
	for (n=0; n<_Instances.size(); ++n)
		_Instances[n].unlink(_Instances);
}

void	NLPACS::CGlobalRetriever::makeAllLinks()
{
	resetAllLinks();

	uint	n;
	for (n=0; n<_Instances.size(); ++n)
		makeLinks(n);
}

NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::makeInstance(uint x, uint y, uint32 retriever, uint8 orientation, const CVector &origin)
{
	CRetrieverInstance	&inst = getInstanceFullAccess(x, y);
	inst.make(convertId(x, y), retriever, _RetrieverBank->getRetriever(retriever), orientation, origin);
	return inst;
}



NLPACS::CGlobalRetriever::CGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVector &estimated)
{
	CRetrieverInstance	&instance = getInstanceFullAccess(estimated);
	CLocalRetriever::CLocalPosition	localPosition = instance.retrievePosition(estimated, _RetrieverBank->getRetriever(instance.getRetrieverId()));
	return CGlobalPosition(instance.getInstanceId(), localPosition);
}

CVector		NLPACS::CGlobalRetriever::getGlobalPosition(const NLPACS::CGlobalRetriever::CGlobalPosition &global) const
{
	return _Instances[global.InstanceId].getGlobalPosition(global.LocalPosition.Estimation);
}

CVectorD	NLPACS::CGlobalRetriever::getDoubleGlobalPosition(const NLPACS::CGlobalRetriever::CGlobalPosition &global) const
{
	return _Instances[global.InstanceId].getDoubleGlobalPosition(global.LocalPosition.Estimation);
}

CVector		NLPACS::CGlobalRetriever::getInstanceCenter(uint x, uint y) const
{
	const float	zdim = 160.0f;
	CVector	bmin = _BBox.getMin();
	return CVector(bmin.x+zdim*((float)x+0.5f), bmin.y+zdim*((float)y+0.5f), 0.0f);
}



void		NLPACS::CGlobalRetriever::findAStarPath(const NLPACS::CGlobalRetriever::CGlobalPosition &begin,
													const NLPACS::CGlobalRetriever::CGlobalPosition &end)
{
	multimap<float, CRetrieverInstance::CAStarNodeAccess>	open;
	vector<CRetrieverInstance::CAStarNodeAccess>			close;

	CRetrieverInstance::CAStarNodeAccess					beginNode;
	beginNode.InstanceId = begin.InstanceId;
	beginNode.NodeId = (uint16)begin.LocalPosition.Surface;
	CRetrieverInstance::CAStarNodeInfo						&beginInfo = getNode(beginNode);

	CRetrieverInstance::CAStarNodeAccess					endNode;
	endNode.InstanceId = end.InstanceId;
	endNode.NodeId = (uint16)end.LocalPosition.Surface;
	CRetrieverInstance::CAStarNodeInfo						&endInfo = getNode(endNode);

	CRetrieverInstance::CAStarNodeAccess					node = beginNode;
	beginInfo.Parent.InstanceId = -1;
	beginInfo.Parent.NodeId = 0;
	beginInfo.Cost = 0;
	beginInfo.F = (endInfo.Position-beginInfo.Position).norm();

	open.insert(make_pair(beginInfo.F, node));

	CVector													endPosition = getGlobalPosition(end);

	uint	i;

	while (true)
	{
		if (open.empty())
		{
			// couldn't find a path
			return;
		}

		multimap<float, CRetrieverInstance::CAStarNodeAccess>::iterator	it;

		it = open.begin();
		node = it->second;
		open.erase(it);

		if (node == endNode)
		{
			// found a path
			return;
		}

		// push successors of the current node
		CRetrieverInstance								&inst = _Instances[node.InstanceId];
		const CLocalRetriever							&retriever = _RetrieverBank->getRetriever(inst.getRetrieverId());
		const CRetrievableSurface						&surf = retriever.getSurface(node.NodeId);
		const vector<CRetrievableSurface::CSurfaceLink>	&chains = surf.getChains();

		CRetrieverInstance								*nextInstance;
		const CLocalRetriever							*nextRetriever;
		const CRetrievableSurface						*nextSurface;

		for (i=0; i<chains.size(); ++i)
		{
			sint32	nextNodeId = chains[i].Surface;
			CRetrieverInstance::CAStarNodeAccess		nextNode;

			if (CChain::isEdgeId(nextNodeId))
			{
				// if the chain points to another retriever

				// first get the edge on the retriever
				uint	edge = retriever.getChain(chains[i].Chain).getEdge();
				sint	edgeIndex = CChain::convertEdgeId(nextNodeId);
				nextNode.InstanceId = inst.getNeighbor((edge+inst.getOrientation())%4);

				if (nextNode.InstanceId < 0)
					continue;

				nextNode.NodeId = inst.getEdgeChainLink(edge, edgeIndex);
				nextInstance = &_Instances[nextNode.InstanceId];
				nextRetriever = &(_RetrieverBank->getRetriever(nextInstance->getRetrieverId()));
			}
			else
			{
				nextNode.InstanceId = node.InstanceId;
				nextNode.NodeId = (uint16) nextNodeId;
				nextInstance = &inst;
				nextRetriever = &retriever;
			}

			nextSurface = &(nextRetriever->getSurface(nextNode.NodeId));

			// compute new node value (heuristic and cost)

			float	stepCost = (surf.getCenter()-nextSurface->getCenter()).norm();
			float	nextCost = inst._NodesInformation[node.NodeId].Cost+stepCost;
			float	nextHeuristic = (nextSurface->getCenter()-endPosition).norm();
			float	nextF = nextCost+nextHeuristic;
			CRetrieverInstance::CAStarNodeInfo	&nextInfo = nextInstance->_NodesInformation[nextNode.NodeId];

			vector<CRetrieverInstance::CAStarNodeAccess>::iterator			closeIt;
			for (closeIt=close.begin(); closeIt!=close.end() && *closeIt!=nextNode; ++closeIt)
				;

			if (closeIt != close.end() && nextInfo.F < nextF)
				continue;
			
			multimap<float, CRetrieverInstance::CAStarNodeAccess>::iterator	openIt;
			for (openIt=open.begin(); openIt!=open.end() && openIt->second!=nextNode; ++openIt)
				;

			if (openIt != open.end() && nextInfo.F < nextF)
				continue;

			if (openIt != open.end())
				open.erase(openIt);

			if (closeIt != close.end())
				close.erase(closeIt);

			nextInfo.Parent = node;
			nextInfo.Cost = nextCost;
			nextInfo.F = nextF;

			open.insert(make_pair(nextInfo.F, nextNode));
		}
	}
}
