/** \file global_retriever.cpp
 *
 *
 * $Id: global_retriever.cpp,v 1.26 2001/06/08 15:04:04 legros Exp $
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
#include <list>
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

// CGlobalRetriever methods implementation

//
NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::getInstanceFullAccess(const CVector &position)
{
	float				offsetX = position.x - _BBox.getMin().x;
	float				offsetY = _BBox.getMax().y - position.y;
	// please NOTE that offsetY decreases as position.y increases!!
	static const float	zdim = 160.0f;
	return getInstanceFullAccess((uint)(offsetX/zdim), (uint)(offsetY/zdim));
}

const NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::getInstance(const CVector &position) const
{
	float				offsetX = position.x - _BBox.getMin().x;
	float				offsetY = _BBox.getMax().y - position.y;
	// please NOTE that offsetY decreases as position.y increases!!
	static const float	zdim = 160.0f;
	return getInstance((uint)(offsetX/zdim), (uint)(offsetY/zdim));
}

//

void	NLPACS::CGlobalRetriever::serial(NLMISC::IStream &f)
{
	f.serialCont(_Instances);
	f.serial(_Width, _Height);
	f.serial(_BBox);
}

//

void	NLPACS::CGlobalRetriever::makeLinks(uint n)
{
	uint	x, y;
	convertId(n, x, y);

	// links nth instance with its leftmost neighbor.
	if (x > 0 && _Instances[n].getInstanceId() >= 0 && _Instances[n-1].getInstanceId() >= 0)
	{
		try
		{
			_Instances[n].link(_Instances[n-1], 0, _RetrieverBank->getRetrievers());
			_Instances[n-1].link(_Instances[n], 2, _RetrieverBank->getRetrievers());
		}
		catch (Exception &e)
		{
			nlwarning("in NLPACS::CGlobalRetriever::makeLinks()");
			nlwarning("caught an exception: %s", e.what());
		}
	}

	// links nth instance with its downmost neighbor.
	if (y < (uint)(_Height-1) && _Instances[n].getInstanceId() >= 0 && _Instances[n+_Width].getInstanceId() >= 0)
	{
		try
		{
			_Instances[n].link(_Instances[n+_Width], 1, _RetrieverBank->getRetrievers());
			_Instances[n+_Width].link(_Instances[n], 3, _RetrieverBank->getRetrievers());
		}
		catch (Exception &e)
		{
			nlwarning("in NLPACS::CGlobalRetriever::makeLinks()");
			nlwarning("caught an exception: %s", e.what());
		}
	}

	// links nth instance with its rightmost neighbor.
	if (x < (uint)(_Width-1) && _Instances[n].getInstanceId() >= 0 && _Instances[n+1].getInstanceId() >= 0)
	{
		try
		{
			_Instances[n].link(_Instances[n+1], 2, _RetrieverBank->getRetrievers());
			_Instances[n+1].link(_Instances[n], 0, _RetrieverBank->getRetrievers());
		}
		catch (Exception &e)
		{
			nlwarning("in NLPACS::CGlobalRetriever::makeLinks()");
			nlwarning("caught an exception: %s", e.what());
		}
	}

	// links nth instance with its uppermost neighbor.
	if (y > 0 && _Instances[n].getInstanceId() >= 0 && _Instances[n-_Width].getInstanceId() >= 0)
	{
		try
		{
			_Instances[n].link(_Instances[n-_Width], 3, _RetrieverBank->getRetrievers());
			_Instances[n-_Width].link(_Instances[n], 1, _RetrieverBank->getRetrievers());
		}
		catch (Exception &e)
		{
			nlwarning("in NLPACS::CGlobalRetriever::makeLinks()");
			nlwarning("caught an exception: %s", e.what());
		}
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

//

NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::makeInstance(uint x, uint y, uint32 retriever, uint8 orientation, const CVector &origin)
{
	CRetrieverInstance	&inst = getInstanceFullAccess(x, y);
	inst.make(convertId(x, y), retriever, _RetrieverBank->getRetriever(retriever), orientation, origin);
	return inst;
}


NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::makeInstance(uint x, uint y, uint32 retriever, uint8 orientation)
{
	CVector	center = getInstanceCenter(x, y);
	return makeInstance(x, y, retriever, orientation, center);
}

//

NLPACS::CGlobalRetriever::CGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVector &estimated) const
{
	const CRetrieverInstance	&instance = getInstance(estimated);
	if (instance.getRetrieverId() >= 0)
	{
		// if there is an actual instance at this position, retrieve the position
		CLocalRetriever::CLocalPosition	localPosition = instance.retrievePosition(estimated, _RetrieverBank->getRetriever(instance.getRetrieverId()));
		return CGlobalPosition(instance.getInstanceId(), localPosition);
	}
	else
	{
		// if there is no instance there, return a blank position
		return CGlobalPosition(instance.getInstanceId(), CLocalRetriever::CLocalPosition(-1, estimated));
	}
}

CVector		NLPACS::CGlobalRetriever::getGlobalPosition(const NLPACS::CGlobalRetriever::CGlobalPosition &global) const
{
	if (global.InstanceId >= 0)
	{
		return _Instances[global.InstanceId].getGlobalPosition(global.LocalPosition.Estimation);
	}
	else
	{
		// it should be an error here
		return CVector::Null;
	}
}

CVectorD	NLPACS::CGlobalRetriever::getDoubleGlobalPosition(const NLPACS::CGlobalRetriever::CGlobalPosition &global) const
{
	if (global.InstanceId >= 0)
	{
		return _Instances[global.InstanceId].getDoubleGlobalPosition(global.LocalPosition.Estimation);
	}
	else
	{
		// it should be an error here
		return CVectorD::Null;
	}
}

//

CVector		NLPACS::CGlobalRetriever::getInstanceCenter(uint x, uint y) const
{
	const float	zdim = 160.0f;
	CVector	bmin = _BBox.getMin();
	CVector	bmax = _BBox.getMax();
	return CVector(bmin.x+zdim*((float)x+0.5f), bmax.y-zdim*((float)y+0.5f), 0.0f);
}


//

void		NLPACS::CGlobalRetriever::findAStarPath(const NLPACS::CGlobalRetriever::CGlobalPosition &begin,
													const NLPACS::CGlobalRetriever::CGlobalPosition &end,
													vector<NLPACS::CRetrieverInstance::CAStarNodeAccess> &path)
{
	// open and close lists
	// TODO: Use a smart allocator to avoid huge alloc/free and memory fragmentation
	// open is a priority queue (implemented as a stl multimap)
	multimap<float, CRetrieverInstance::CAStarNodeAccess>	open;
	// close is a simple stl vector
	vector<CRetrieverInstance::CAStarNodeAccess>			close;

	// inits start node and info
	CRetrieverInstance::CAStarNodeAccess					beginNode;
	beginNode.InstanceId = begin.InstanceId;
	beginNode.NodeId = (uint16)begin.LocalPosition.Surface;
	CRetrieverInstance::CAStarNodeInfo						&beginInfo = getNode(beginNode);

	// inits end node and info.
	CRetrieverInstance::CAStarNodeAccess					endNode;
	endNode.InstanceId = end.InstanceId;
	endNode.NodeId = (uint16)end.LocalPosition.Surface;
	CRetrieverInstance::CAStarNodeInfo						&endInfo = getNode(endNode);

	// set up first node...
	CRetrieverInstance::CAStarNodeAccess					node = beginNode;
	beginInfo.Parent.InstanceId = -1;
	beginInfo.Parent.NodeId = 0;
	beginInfo.Parent.ThroughChain = 0;
	beginInfo.Cost = 0;
	beginInfo.F = (endInfo.Position-beginInfo.Position).norm();

	// ... and inserts it in the open list.
	open.insert(make_pair(beginInfo.F, node));

	// TO DO: use a CVector2f instead
	CVector													endPosition = getGlobalPosition(end);

	uint	i;

	path.clear();

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
			nlinfo("found a path");
			CRetrieverInstance::CAStarNodeAccess			pathNode = node;
			uint											numNodes = 0;
			while (pathNode.InstanceId != -1)
			{
				++numNodes;
				CRetrieverInstance							&instance = _Instances[pathNode.InstanceId];
				CRetrieverInstance::CAStarNodeInfo			&pathInfo = instance._NodesInformation[pathNode.NodeId];
				nlinfo("pathNode = (InstanceId=%d, NodeId=%d)", pathNode.InstanceId, pathNode.NodeId);
				pathNode = pathInfo.Parent;
			}

			path.resize(numNodes);
			pathNode = node;
			while (pathNode.InstanceId != -1)
			{
				path[--numNodes] = pathNode;
				CRetrieverInstance							&instance = _Instances[pathNode.InstanceId];
				CRetrieverInstance::CAStarNodeInfo			&pathInfo = instance._NodesInformation[pathNode.NodeId];
				pathNode = pathInfo.Parent;
			}

			nlinfo("open.size()=%d", open.size());
			nlinfo("close.size()=%d", close.size());
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
				uint	edge = inst.getInstanceEdge(retriever.getChain(chains[i].Chain).getEdge());
				sint	edgeIndex = CChain::convertEdgeId(nextNodeId);
				nextNode.InstanceId = inst.getNeighbor(edge);

				if (nextNode.InstanceId < 0)
					continue;

				nextInstance = &_Instances[nextNode.InstanceId];
				nextRetriever = &(_RetrieverBank->getRetriever(nextInstance->getRetrieverId()));

				uint	chain = inst.getEdgeChainLink(edge, edgeIndex);
				sint	nodeId = nextRetriever->getChain(chain).getLeft();
				nlassert(nodeId >= 0);
				nextNode.NodeId = (uint16)nodeId;
			}
			else if (nextNodeId >= 0)
			{
				// if the chain points to the same instance
				nextNode.InstanceId = node.InstanceId;
				nextNode.NodeId = (uint16) nextNodeId;
				nextInstance = &inst;
				nextRetriever = &retriever;
			}
			else
			{
				// if the chain cannot be crossed
				continue;
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
			nextInfo.Parent.ThroughChain = i;
			nextInfo.Cost = nextCost;
			nextInfo.F = nextF;

			open.insert(make_pair(nextInfo.F, nextNode));
		}
		close.push_back(node);
	}
}



void	NLPACS::CGlobalRetriever::findPath(const NLPACS::CGlobalRetriever::CGlobalPosition &begin, 
										   const NLPACS::CGlobalRetriever::CGlobalPosition &end, 
										   vector<NLPACS::CVector2s> &waypoints)
{
	vector<CRetrieverInstance::CAStarNodeAccess>	path;
	vector<CLocalPathTips>							surfInfos;

	findAStarPath(begin, end, path);

	surfInfos.reserve(path.size());

	uint	i, j;
	for (i=0; i<path.size(); ++i)
	{
		CLocalPathTips	surf;
		surf.InstanceId = path[i].InstanceId;

		// computes start point
		if (i == 0)
		{
			// if it is the first point, just copy the begin
			surf.Start = begin.LocalPosition;
		}
		else
		{
			// else, take the previous value and convert it in the current instance axis
			// TODO: avoid this if the instances are the same
			CVector	prev = _Instances[surfInfos[i-1].InstanceId].getGlobalPosition(surfInfos[i-1].End.Estimation);
			CVector	current = _Instances[surf.InstanceId].getLocalPosition(prev);
			surf.End.Surface = path[i].NodeId;
			surf.End.Estimation = current;
		}

		// computes end point
		if (i == path.size()-1)
		{
			surf.End = end.LocalPosition;
		}
		else
		{
			// get to the middle of the chain
			// first get the chain between the 2 surfaces
			const CLocalRetriever	&retriever = _RetrieverBank->getRetriever(_Instances[surf.InstanceId].getRetrieverId());
			const CChain			&chain = retriever.getChain(path[i].ThroughChain);
			float					cumulLength = 0.0f, midLength=chain.getLength()*0.5f;
			for (j=0; j<chain.getSubChains().size() && cumulLength<=midLength; ++j)
				cumulLength += retriever.getOrderedChain(chain.getSubChain(j)).getLength();
			--j;
			const COrderedChain		&ochain = retriever.getOrderedChain(chain.getSubChain(j));
			surf.End.Surface = path[i].NodeId;
			surf.End.Estimation = ochain[ochain.getVertices().size()/2].unpack3f();
		}
	}
}


// ***************************************************************************
void	NLPACS::CGlobalRetriever::getInstanceBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const NLMISC::CAABBox &bbox) const
{
	CVector		minP= bbox.getMin() - _BBox.getMin();
	CVector		maxP= bbox.getMax() - _BBox.getMin();

	// A zone is 160x160 meters.
	x0= (sint32)floor(minP.x / 160);
	y0= (sint32)floor(minP.y / 160);
	x1= (sint32) ceil(maxP.x / 160);
	y1= (sint32) ceil(maxP.y / 160);
	x0= max(x0, (sint32)0);
	y0= max(y0, (sint32)0);
	x1= min(x1, (sint32)_Width);
	y1= min(y1, (sint32)_Height);

	// invert y.
	y0= _Height-y0;
	y1= _Height-y1;
	// y0<=y1.
	swap(y0, y1);
}



// ***************************************************************************
// ***************************************************************************
// Collisions part.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
const NLPACS::CRetrievableSurface	*NLPACS::CGlobalRetriever::getSurfaceById(const NLPACS::CSurfaceIdent &surfId)
{
	if(surfId.RetrieverInstanceId>=0 && surfId.SurfaceId>=0)
	{
		sint32	locRetId= this->getInstance(surfId.RetrieverInstanceId).getRetrieverId();
		const CRetrievableSurface	&surf= _RetrieverBank->getRetriever(locRetId).getSurface(surfId.SurfaceId);
		return &surf;
	}
	else
		return NULL;
}



// ***************************************************************************
void	NLPACS::CGlobalRetriever::findCollisionChains(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector &origin) const
{
	sint	i,j;

	// 0. reset.
	//===========
	// reset possible chains.
	cst.CollisionChains.clear();
	cst.resetEdgeCollideNodes();


	// 1. Find Instances which may hit this movement.
	//===========
	cst.CollisionInstances.clear();

	// First, add zones which hit this bbox.
	// Find instances which intersect the bbox.
	sint32	x0, y0, x1, y1;
	CAABBox		bboxMoveGlobal= bboxMove;
	bboxMoveGlobal.setCenter(bboxMoveGlobal.getCenter()+origin);
	this->getInstanceBounds(x0, y0, x1, y1, bboxMoveGlobal);
	// Add them to the list.
	sint32	x,y;
	for(y=y0; y<y1; y++)
	{
		for(x=x0; x<x1; x++)
		{
			cst.CollisionInstances.push_back(this->convertId(x,y));
		}
	}

	// TODO_INTERIOR: add interiors meshes (static/dynamic houses etc...) to this list.


	// 2. Fill CollisionChains.
	//===========
	// For each possible surface mesh, test collision.
	for(i=0 ; i<(sint)cst.CollisionInstances.size(); i++)
	{
		// get retrieverInstance.
		sint32	curInstance= cst.CollisionInstances[i];
		const CRetrieverInstance	&retrieverInstance= getInstance(curInstance);

		// Retrieve the localRetriever of this instance.
		sint32	localRetrieverId= retrieverInstance.getRetrieverId();
		// If invalid one (hole), continue.
		if(localRetrieverId<0)
			continue;
		const CLocalRetriever		&localRetriever= _RetrieverBank->getRetriever(localRetrieverId);

		// get delta between startPos.instance and curInstance.
		CVector		deltaOrigin;
		deltaOrigin= origin - retrieverInstance.getOrigin();

		// compute movement relative to this localRetriever.
		CAABBox		bboxMoveLocal= bboxMove;
		bboxMoveLocal.setCenter(bboxMoveLocal.getCenter()+deltaOrigin);
	
		// add possible collision chains with movement.
		//================
		sint		firstCollisionChain= cst.CollisionChains.size();
		CVector2f	transBase(-deltaOrigin.x, -deltaOrigin.y);
		// Go! fill collision chains that this movement intersect.
		localRetriever.testCollision(cst, bboxMoveLocal, transBase);
		// how many collision chains added?  : nCollisionChain-firstCollisionChain.
		sint		nCollisionChain= cst.CollisionChains.size();


		// For all collision chains added, fill good SurfaceIdent info.
		//================
		for(j=firstCollisionChain; j<nCollisionChain; j++)
		{
			CCollisionChain		&cc= cst.CollisionChains[j];
			// LeftSurface retrieverInstance is always curInstance.
			cc.LeftSurface.RetrieverInstanceId= curInstance;

			// If RightSurface is not an "edgeId" ie a pointer on a neighbor surface on an other retrieverInstance.
			const	CChain		&originalChain= localRetriever.getChain(cc.ChainId);
			if( !originalChain.isEdgeId(cc.RightSurface.SurfaceId) )
			{
				cc.RightSurface.RetrieverInstanceId= curInstance;
			}
			else
			{
				// we must find the surfaceIdent of the neighbor.
				// TODO_INTERIOR: this work only for zone. Must work too for houses.

				// On which edge of the zone is this chain.
				sint	edgeChain= originalChain.getEdge();
				nlassert(edgeChain>=0);
				// get edgeId in global space. (rotate).
				edgeChain= (edgeChain + retrieverInstance.getOrientation())%4;

				// Get the good neighbor instance Id.
				//================
				// get the neighbor instanceId.
				sint	neighborInstanceId= retrieverInstance.getNeighbor(edgeChain);
				// store in the current collisionChain Right.
				cc.RightSurface.RetrieverInstanceId= neighborInstanceId;

				// If no instance near us, this is a WALL.
				if(neighborInstanceId<0)
				{
					// mark as a Wall.
					cc.RightSurface.SurfaceId= -1;
				}
				else
				{
					// Get the good neighbor surfaceId.
					//================
					// get the chainId of the neighborInstance 's localRetriever.
					sint	neighborChainId= retrieverInstance.getEdgeChainLink(edgeChain, 
						CChain::convertEdgeId(cc.RightSurface.SurfaceId));

					// get the chain of the neighborInstance 's localRetriever.
					const CRetrieverInstance	&neighborInstance= getInstance(neighborInstanceId);
					const CChain		&neighborChain= (_RetrieverBank->getRetriever(neighborInstance.getRetrieverId())).getChain(neighborChainId);

					// Now we have this chain, we are sure that chain.Left is our SurfaceId of cc.Right.
					cc.RightSurface.SurfaceId= neighborChain.getLeft();
				}
			}
		}


		// For all collision chains added, look if they are a copy of preceding collsion chain (same Left/Right). Then delete them.
		//================
		// TODO_OPTIMIZE: this is a N² complexity.
		for(j=firstCollisionChain; j<nCollisionChain; j++)
		{
			// test for all collisionChain inserted before.
			for(sint k=0; k<firstCollisionChain; k++)
			{
				// if same collision chain (same surface Ident Left/Right==Left/Right or swapped Left/Right==Right/Left).
				if( cst.CollisionChains[j].sameSurfacesThan(cst.CollisionChains[k]) )
				{
					// remove this jth entry.
					// by swapping with last entry. Only if not already last.
					if(j<nCollisionChain-1)
					{
						swap(cst.CollisionChains[j], cst.CollisionChains[nCollisionChain-1]);
						// NB: some holes remain in cst._EdgeCollideNodes, but do not matters since reseted at 
						// each collision test.
					}

					// pop last entry.
					nCollisionChain--;
					cst.CollisionChains.resize(nCollisionChain);

					// next entry??
					j--;
					break;
				}
			}

		}

	}

}


// ***************************************************************************
void	NLPACS::CGlobalRetriever::testCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface, float radius, const CVector2f bboxStart[4], TCollisionType colType) const
{
	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	uint			nextCollisionSurfaceTested=0;
	sint			i;

	// reset result.
	cst.CollisionDescs.clear();
	// reset all collisionChain to not tested.
	for(i=0; i<(sint)cst.CollisionChains.size(); i++)
	{
		CCollisionChain		&colChain= cst.CollisionChains[i];
		colChain.Tested= false;
	}


	/*
		To manage recovery, we must use such an algorithm, so we are sure to trace the way across all surfaces really 
		collided, and discard any other (such as other floor or ceiling).
	*/
	while(true)
	{
		// run all collisionChain.
		//========================
		for(i=0; i<(sint)cst.CollisionChains.size(); i++)
		{
			CCollisionChain		&colChain= cst.CollisionChains[i];
			// test only currentSurface/X. And don't test chains already tested before.
			if(colChain.hasSurface(currentSurface) && !colChain.Tested)
			{
				// we are testing this chain.
				colChain.Tested= true;


				// test all edges of this chain, and get tmin
				//========================
				float		t, tMin=1;
				CVector2f	normal, normalMin;
				// run list of edge.
				sint32		curEdge= colChain.FirstEdgeCollide;
				while(curEdge!=0xFFFFFFFF)
				{
					// get the edge.
					CEdgeCollideNode	&colEdge= cst.getEdgeCollideNode(curEdge);

					// test collision with this edge.
					if(colType==CGlobalRetriever::Circle)
						t= colEdge.testCircleMove(startCol, deltaCol, radius, normal);
					else if(colType==CGlobalRetriever::BBox)
						t= colEdge.testBBoxMove(startCol, deltaCol, bboxStart, normal);

					// earlier collision??
					if(t<tMin)
					{
						tMin= t;
						normalMin= normal;
					}

					// next edge.
					curEdge= colEdge.Next;
				}


				// If collision with this chain, must insert it in the array of collision.
				//========================
				if(tMin<1)
				{
					CSurfaceIdent	collidedSurface= colChain.getOtherSurface(currentSurface);

					// insert or replace this collision in collisionDescs.
					// NB: yes this looks like a N algorithm (so N²). But not so many collisions may arise, so don't bother.
					sint	indexInsert= cst.CollisionDescs.size();
					sint	colFound= -1;

					// start to search with nextCollisionSurfaceTested, because can't insert before.
					for(sint j= nextCollisionSurfaceTested; j<(sint)cst.CollisionDescs.size(); j++)
					{
						// we must keep time order.
						if(tMin < cst.CollisionDescs[j].ContactTime)
						{
							indexInsert= min(j, indexInsert);
						}
						// Does the collision with this surface already exist??
						if(cst.CollisionDescs[j].ContactSurface==collidedSurface)
						{
							colFound= j;
							// if we have found our old collision, stop, there is no need to search more.
							break;
						}
					}

					// Insert only if the surface was not already collided, or that new collision arise before old.
					if(colFound==-1 || indexInsert<=colFound)
					{
						CCollisionSurfaceDesc	newCol;
						newCol.ContactSurface= collidedSurface;
						newCol.ContactTime= tMin;
						newCol.ContactNormal.set(normalMin.x, normalMin.y, 0);

						// if, by chance, indexInsert==colFound, just replace old collision descriptor.
						if(colFound==indexInsert)
						{
							cst.CollisionDescs[indexInsert]= newCol;
						}
						else
						{
							// if any, erase old collision against this surface. NB: here, colFound>indexInsert.
							if(colFound!=-1)
								cst.CollisionDescs.erase(cst.CollisionDescs.begin() + colFound);

							// must insert the collision.
							cst.CollisionDescs.insert(cst.CollisionDescs.begin() + indexInsert, newCol);
						}
					}
				}
			}
		}

		// Find next surface to test.
		//========================
		// No more?? so this is the end.
		if(nextCollisionSurfaceTested>=cst.CollisionDescs.size())
			break;
		// else next one.
		else
		{
			// NB: with this algorithm, we are sure that no more collisions will arise before currentCollisionSurfaceTested.
			// so just continue with following surface.
			currentSurface= cst.CollisionDescs[nextCollisionSurfaceTested].ContactSurface;

			// Do we touch a wall??
			bool	isWall;
			if(currentSurface.SurfaceId<0)
				isWall= true;
			else
			{
				// test if it is a walkable wall.
				sint32	locRetId= this->getInstance(currentSurface.RetrieverInstanceId).getRetrieverId();
				const CRetrievableSurface	&surf= _RetrieverBank->getRetriever(locRetId).getSurface(currentSurface.SurfaceId);
				isWall= !(surf.isFloor() || surf.isCeiling());
			}

			// If we touch a wall, this is the end of search.
			if(isWall)
			{
				// There can be no more collision after this one.
				cst.CollisionDescs.resize(nextCollisionSurfaceTested+1);
				break;
			}
			else
			{
				// Next time, we will test the following (NB: the array may grow during next pass, or reorder, 
				// but only after nextCollisionSurfaceTested).
				nextCollisionSurfaceTested++;
			}
		}
	}

}


// ***************************************************************************
NLPACS::CSurfaceIdent	NLPACS::CGlobalRetriever::testMovementWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &endCol,
		CSurfaceIdent startSurface) const
{
	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	sint			i;

	// reset result.
	cst.MoveDescs.clear();


	/*
		To manage recovery, we must use such an algorithm, so we are sure to trace the way across all surfaces really 
		collided, and discard any other (such as other floor or ceiling).

		This function is quite different from testCollisionWithCollisionChains() because she must detect all collisions
		with all edges of any chains (and not the minimum collision with a chain).
		This is done in 3 parts:
			- detect collisions with all edges.
			- sort.
			- leave only real collisions.
	*/
	// run all collisionChain.
	//========================
	for(i=0; i<(sint)cst.CollisionChains.size(); i++)
	{
		CCollisionChain		&colChain= cst.CollisionChains[i];


		// test all edges of this chain, and insert if necessary.
		//========================
		CRational64		t;
		// run list of edge.
		sint32		curEdge= colChain.FirstEdgeCollide;
		while(curEdge!=0xFFFFFFFF)
		{
			// get the edge.
			CEdgeCollideNode	&colEdge= cst.getEdgeCollideNode(curEdge);

			// test collision with this edge.
			CEdgeCollide::TPointMoveProblem		pmpb;
			t= colEdge.testPointMove(startCol, endCol, pmpb);
			// manage multiple problems of precision.
			if(t== -1)
			{
				string	errs[CEdgeCollide::PointMoveProblemCount]= {
					"ParallelEdges", "StartOnEdge", "StopOnEdge", "TraverseEndPoint"};
				nlinfo("COL: Precision Problem: %s", errs[pmpb]);
				// return a "Precision Problem" ident. movement is invalid. BUT if startOnEdge, which should never arrive.
				if(pmpb==CEdgeCollide::StartOnEdge)
					return CSurfaceIdent(-1, -1);	// so in this case, block....
				else
					return	CSurfaceIdent(-2, -2);
			}

			// collision??
			if(t<1)
			{
				// insert in list.
				cst.MoveDescs.push_back(CMoveSurfaceDesc(t, colChain.LeftSurface, colChain.RightSurface));
			}

			// next edge.
			curEdge= colEdge.Next;
		}
	}


	// sort.
	//================
	// sort the collisions in ascending time order.
	sort(cst.MoveDescs.begin(), cst.MoveDescs.end());


	// Traverse the array of collisions.
	//========================
	for(i=0;i<(sint)cst.MoveDescs.size();i++)
	{
		// Do we collide with this chain??
		if(cst.MoveDescs[i].hasSurface(currentSurface))
		{
			currentSurface= cst.MoveDescs[i].getOtherSurface(currentSurface);

			// Do we touch a wall?? should not happens, but important for security.
			bool	isWall;
			if(currentSurface.SurfaceId<0)
				isWall= true;
			else
			{
				// test if it is a walkable wall.
				sint32	locRetId= this->getInstance(currentSurface.RetrieverInstanceId).getRetrieverId();
				const CRetrievableSurface	&surf= _RetrieverBank->getRetriever(locRetId).getSurface(currentSurface.SurfaceId);
				isWall= !(surf.isFloor() || surf.isCeiling());
			}

			// If we touch a wall, this is the end of search.
			if(isWall)
			{
				// return a Wall ident. movement is invalid.
				return	CSurfaceIdent(-1, -1);
			}
		}
	}


	return currentSurface;
}



// ***************************************************************************
const	NLPACS::TCollisionSurfaceDescVector	
	&NLPACS::CGlobalRetriever::testCylinderMove(const CGlobalPosition &startPos, const NLMISC::CVector &delta, float radius, CCollisionSurfaceTemp &cst) const
{
	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// 0. reset.
	//===========
	// reset result.
	cst.CollisionDescs.clear();

	// store this request in cst.
	cst.PrecStartSurface= startSurface;
	cst.PrecStartPos= startPos.LocalPosition.Estimation;
	cst.PrecDeltaPos= delta;
	cst.PrecValid= true;

	// 1. Choose a local basis.
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. compute bboxmove.
	//===========
	CAABBox		bboxMove;
	// bounds the movement in a bbox.
	// compute start and end, relative to the retriever instance.
	CVector		start= startPos.LocalPosition.Estimation;
	CVector		end= start+delta;
	// extend the bbox.
	bboxMove.setCenter(start-CVector(radius, radius, 0));
	bboxMove.extend(start+CVector(radius, radius, 0));
	bboxMove.extend(end-CVector(radius, radius, 0));
	bboxMove.extend(end+CVector(radius, radius, 0));


	// 3. find possible collisions in bboxMove+origin. fill cst.CollisionChains.
	//===========
	findCollisionChains(cst, bboxMove, origin);



	// 4. test collisions with CollisionChains.
	//===========
	CVector2f	startCol(start.x, start.y);
	CVector2f	deltaCol(delta.x, delta.y);
	CVector2f	obbDummy[4];	// dummy OBB (not obb here so don't bother)
	testCollisionWithCollisionChains(cst, startCol, deltaCol, startSurface, radius, obbDummy, CGlobalRetriever::Circle);


	// result.
	return cst.CollisionDescs;
}


// ***************************************************************************
const	NLPACS::TCollisionSurfaceDescVector	
	&NLPACS::CGlobalRetriever::testBBoxMove(const CGlobalPosition &startPos, const NLMISC::CVector &delta, 
	const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const
{
	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// 0. reset.
	//===========
	// reset result.
	cst.CollisionDescs.clear();

	// store this request in cst.
	cst.PrecStartSurface= startSurface;
	cst.PrecStartPos= startPos.LocalPosition.Estimation;
	cst.PrecDeltaPos= delta;
	cst.PrecValid= true;


	// 1. Choose a local basis.
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. compute OBB.
	//===========
	CVector2f	obbStart[4];
	// compute start, relative to the retriever instance.
	CVector		start= startPos.LocalPosition.Estimation;
	CVector2f	obbCenter(start.x, start.y);
	CVector2f	locI2d(locI.x, locI.y);
	CVector2f	locJ2d(locJ.x, locJ.y);

	// build points in CCW.
	obbStart[0]= obbCenter - locI2d - locJ2d;
	obbStart[1]= obbCenter + locI2d - locJ2d;
	obbStart[2]= obbCenter + locI2d + locJ2d;
	obbStart[3]= obbCenter - locI2d + locJ2d;

	// 3. compute bboxmove.
	//===========
	CAABBox		bboxMove;
	// extend the bbox.
	bboxMove.setCenter(CVector(obbStart[0].x, obbStart[0].y, 0));
	bboxMove.extend(CVector(obbStart[1].x, obbStart[1].y, 0));
	bboxMove.extend(CVector(obbStart[2].x, obbStart[2].y, 0));
	bboxMove.extend(CVector(obbStart[3].x, obbStart[3].y, 0));
	bboxMove.extend(CVector(obbStart[0].x, obbStart[0].y, 0) + delta);
	bboxMove.extend(CVector(obbStart[1].x, obbStart[1].y, 0) + delta);
	bboxMove.extend(CVector(obbStart[2].x, obbStart[2].y, 0) + delta);
	bboxMove.extend(CVector(obbStart[3].x, obbStart[3].y, 0) + delta);



	// 4. find possible collisions in bboxMove+origin. fill cst.CollisionChains.
	//===========
	findCollisionChains(cst, bboxMove, origin);



	// 5. test collisions with CollisionChains.
	//===========
	CVector2f	startCol(start.x, start.y);
	CVector2f	deltaCol(delta.x, delta.y);
	testCollisionWithCollisionChains(cst, startCol, deltaCol, startSurface, 0, obbStart, CGlobalRetriever::BBox);


	// result.
	return cst.CollisionDescs;
}



// ***************************************************************************
NLPACS::CGlobalRetriever::CGlobalPosition		
	NLPACS::CGlobalRetriever::doMove(const NLPACS::CGlobalRetriever::CGlobalPosition &startPos, const NLMISC::CVector &delta, float t, NLPACS::CCollisionSurfaceTemp &cst, bool rebuildChains) const
{
	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// clamp factor.
	clamp(t, 0.0f, 1.0f);

	// 0. reset.
	//===========
	// reset CollisionDescs.
	cst.CollisionDescs.clear();

	if(!rebuildChains)
	{
		// same move request than prec testMove() ??.
		if( cst.PrecStartSurface != startSurface || 
			cst.PrecStartPos!=startPos.LocalPosition.Estimation || 
			cst.PrecDeltaPos!=delta ||
			!cst.PrecValid)
		{
			// if not, just return start.
			nlstop;
			return startPos;
		}
		// Since we are sure we have same movement than prec testMove(), no need to rebuild cst.CollisionChains.
	}
	else
	{
		// we don't have same movement than prec testMove(), we must rebuild cst.CollisionChains.
		// Prec settings no more valids.
		cst.PrecValid= false;
	}




	// 1. Choose a local basis (same than in testMove()).
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. test collisions with CollisionChains.
	//===========
	CVector		start= startPos.LocalPosition.Estimation;
	// compute end with real delta position.
	CVector		end= start + delta*t;

	// If asked, we must rebuild array of collision chains.
	if(rebuildChains)
	{
		// compute bboxmove.
		CAABBox		bboxMove;
		// must add some extent, to be sure to include snapped CLocalRetriever vertex (2.0f/256 should be sufficient).
		// Nb: this include the precision problem just below (move a little).
		float	radius= 4.0f/256;
		bboxMove.setCenter(start-CVector(radius, radius, 0));
		bboxMove.extend(start+CVector(radius, radius, 0));
		bboxMove.extend(end-CVector(radius, radius, 0));
		bboxMove.extend(end+CVector(radius, radius, 0));

		// find possible collisions in bboxMove+origin. fill cst.CollisionChains.
		findCollisionChains(cst, bboxMove, origin);
	}


	// look where we arrive.
	CSurfaceIdent	endSurface;
	CVector			endRequest= end;
	const sint		maxPbPrec= 32;	// move away from 4 mm at max, in each 8 direction.
	sint			pbPrecNum= 0;

	// must snap the end position.
	CRetrieverInstance::snapVector(endRequest);
	end= endRequest;

	// Normally, just one iteration is made in this loop (but if precision problem (stopOnEdge, startOnEdge....).
	while(true)
	{
		// must snap the end position.
		CRetrieverInstance::snapVector(end);

		CVector2f	startCol(start.x, start.y);
		CVector2f	endCol(end.x, end.y);

		// If same 2D position, just return startPos (suppose no movement)
		if(endCol==startCol)
		{
			CGlobalPosition		res;
			res= startPos;
			// keep good z movement.
			res.LocalPosition.Estimation.z= end.z;
			return res;
		}

		// search destination problem.
		endSurface= testMovementWithCollisionChains(cst, startCol, endCol, startSurface);

		// if no precision problem, Ok, we have found our destination surface (or anormal collide against a wall).
		if(endSurface.SurfaceId!=-2)
			break;
		/* else we are in deep chit, for one on those reason:
			- traverse on point.
			- stop on a edge (dist==0).
			- start on a edge (dist==0).
			- run // on a edge (NB: dist==0 too).
		*/
		else
		{
			// For simplicty, just try to move a little the end position
			if(pbPrecNum<maxPbPrec)
			{
				static struct	{sint x,y;}   dirs[8]= { {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}};
				sint	dir= pbPrecNum%8;
				sint	dist= pbPrecNum/8+1;
				CVector		dta;

				// compute small move.
				dta.x= dirs[dir].x * dist * 1.0f/SnapPrecision;
				dta.y= dirs[dir].y * dist * 1.0f/SnapPrecision;
				dta.z= 0;

				// add it to the original end pos requested.
				end= endRequest + dta;

				pbPrecNum++;
			}
			else
			{
				// do not move at all.
				endSurface= CSurfaceIdent(-1,-1);
				break;
			}
		}
	}

	// 3. return result.
	//===========
	// Problem?? do not move.
	if(endSurface.SurfaceId==-1)
		return startPos;
	else
	{
		// else must return good GlobalPosition.
		CGlobalPosition		res;

		res.InstanceId= endSurface.RetrieverInstanceId;
		res.LocalPosition.Surface= endSurface.SurfaceId;

		// compute newPos, localy to the endSurface.
		// get delta between startPos.instance and curInstance.
		// NB: for float precision, it is important to compute deltaOrigin, and after compute newPos in local.
		CVector		deltaOrigin;
		deltaOrigin= origin - getInstance(res.InstanceId).getOrigin();

		// Because Origin precision is 1 meter, and end precision is 1/1024 meter, we have no precision problem.
		// this is true because we cannot move more than 160 meters in one doMove() (one zone).
		res.LocalPosition.Estimation= end + deltaOrigin;


		// result.
		return res;
	}

}


// ***************************************************************************
const NLPACS::TCollisionSurfaceDescVector	&NLPACS::CGlobalRetriever::testBBoxRot(const CGlobalPosition &startPos, 
	const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const
{
	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// 0. reset.
	//===========
	// reset result.
	cst.CollisionDescs.clear();

	// should not doMove() after a testBBoxRot.
	cst.PrecValid= false;


	// 1. Choose a local basis.
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. compute OBB.
	//===========
	CVector2f	obbStart[4];
	// compute start, relative to the retriever instance.
	CVector		start= startPos.LocalPosition.Estimation;
	CVector2f	obbCenter(start.x, start.y);
	CVector2f	locI2d(locI.x, locI.y);
	CVector2f	locJ2d(locJ.x, locJ.y);

	// build points in CCW.
	obbStart[0]= obbCenter - locI2d - locJ2d;
	obbStart[1]= obbCenter + locI2d - locJ2d;
	obbStart[2]= obbCenter + locI2d + locJ2d;
	obbStart[3]= obbCenter - locI2d + locJ2d;

	// 3. compute bboxmove.
	//===========
	CAABBox		bboxMove;
	// extend the bbox.
	bboxMove.setCenter(CVector(obbStart[0].x, obbStart[0].y, 0));
	bboxMove.extend(CVector(obbStart[1].x, obbStart[1].y, 0));
	bboxMove.extend(CVector(obbStart[2].x, obbStart[2].y, 0));
	bboxMove.extend(CVector(obbStart[3].x, obbStart[3].y, 0));



	// 4. find possible collisions in bboxMove+origin. fill cst.CollisionChains.
	//===========
	findCollisionChains(cst, bboxMove, origin);



	// 5. test Rotcollisions with CollisionChains.
	//===========
	CVector2f	startCol(start.x, start.y);
	testRotCollisionWithCollisionChains(cst, startCol, startSurface, obbStart);


	// result.
	return cst.CollisionDescs;
}


// ***************************************************************************
void	NLPACS::CGlobalRetriever::testRotCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, CSurfaceIdent startSurface, const CVector2f bbox[4]) const
{
	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	sint			i;

	// reset result.
	cst.RotDescs.clear();
	cst.CollisionDescs.clear();


	/*
		Test collisions with all collision chains. Then, to manage recovery, test the graph of surfaces.
	*/
	// run all collisionChain.
	//========================
	for(i=0; i<(sint)cst.CollisionChains.size(); i++)
	{
		CCollisionChain		&colChain= cst.CollisionChains[i];


		// test all edges of this chain, and insert if necessary.
		//========================
		// run list of edge.
		sint32		curEdge= colChain.FirstEdgeCollide;
		while(curEdge!=0xFFFFFFFF)
		{
			// get the edge.
			CEdgeCollideNode	&colEdge= cst.getEdgeCollideNode(curEdge);

			// test collision with this edge.
			if(colEdge.testBBoxCollide(bbox))
			{
				// yes we have a 2D collision with this chain.
				cst.RotDescs.push_back(CRotSurfaceDesc(colChain.LeftSurface, colChain.RightSurface));
				break;
			}

			// next edge.
			curEdge= colEdge.Next;
		}
	}


	// Traverse the array of collisions.
	//========================
	sint	indexCD=0;
	while(true)
	{
		// What surfaces collided do we reach from this currentSurface??
		for(i=0;i<(sint)cst.RotDescs.size();i++)
		{
			// Do we collide with this chain?? chain not tested??
			if(cst.RotDescs[i].hasSurface(currentSurface) && !cst.RotDescs[i].Tested)
			{
				cst.RotDescs[i].Tested= true;

				// insert the collision with the other surface.
				CCollisionSurfaceDesc	col;
				col.ContactTime= 0;
				col.ContactNormal= CVector::Null;
				col.ContactSurface= cst.RotDescs[i].getOtherSurface(currentSurface);
				cst.CollisionDescs.push_back(col);
			}
		}

		// get the next currentSurface from surface collided (traverse the graph of collisions).
		if(indexCD<(sint)cst.CollisionDescs.size())
			currentSurface= cst.CollisionDescs[indexCD++].ContactSurface;
		else
			break;
	}

}

// ***************************************************************************

NLPACS::UGlobalRetriever *NLPACS::UGlobalRetriever::createGlobalRetriever (const char *globalRetriever, const NLPACS::URetrieverBank *retrieverBank)
{
	// Cast
//	nlassert (dynamic_cast<const NLPACS::CRetrieverBank*>(retrieverBank));
	const NLPACS::CRetrieverBank*	bank=static_cast<const NLPACS::CRetrieverBank*>(retrieverBank);

	CIFile	file;
	file.open(globalRetriever);
	CGlobalRetriever	*retriever = new CGlobalRetriever();
	file.serial(*retriever);

	retriever->setRetrieverBank(bank);

	return static_cast<UGlobalRetriever *>(retriever);
}

// ***************************************************************************

void NLPACS::UGlobalRetriever::deleteGlobalRetriever (UGlobalRetriever *retriever)
{
	// Cast
	nlassert (dynamic_cast<NLPACS::CGlobalRetriever*>(retriever));
	NLPACS::CGlobalRetriever* r=static_cast<NLPACS::CGlobalRetriever*>(retriever);

	// Delete
	delete r;
}

// ***************************************************************************

float			NLPACS::CGlobalRetriever::getMeanHeight(const CGlobalPosition &pos)
{
	// get instance/localretriever.
	const CRetrieverInstance	&instance = getInstance(pos.InstanceId);
	const CLocalRetriever		&retriever= _RetrieverBank->getRetriever(instance.getRetrieverId());

	// find quad leaf.
	const CQuadLeaf	*leaf = retriever.getSurfaces()[pos.LocalPosition.Surface].getQuadTree().getLeaf(pos.LocalPosition.Estimation);

	// if there is no acceptable leaf, just give up
	if (leaf == NULL)
	{
		nlinfo("COL: quadtree: don't find the quadLeaf!");
		return pos.LocalPosition.Estimation.z;
	}
	else
	{
		// else return mean height.
		float	meanHeight = (leaf->getMinHeight()+leaf->getMaxHeight())*0.5f;
		return meanHeight;
	}
}

// ***************************************************************************

bool NLPACS::CGlobalRetriever::testRaytrace (const CVectorD &v0, const CVectorD &v1)
{
	// TODO: implement raytrace
	return false;
}

// ***************************************************************************


// end of CGlobalRetriever methods implementation
