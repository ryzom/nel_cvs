/** \file global_retriever.cpp
 *
 *
 * $Id: global_retriever.cpp,v 1.4 2001/05/16 15:58:14 legros Exp $
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

NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::getInstanceFullAccess(const CVector &position)
{
	CVector				offset = position-_BBox.getMin();
	static const float	zdim = 160.0f;
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

	if (x > 0 && _Instances[n].getInstanceId() >= 0 && _Instances[n-1].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n-1], 0, _RetrieverBank->getRetrievers());
		_Instances[n-1].link(_Instances[n], 2, _RetrieverBank->getRetrievers());
	}

	if (y < (uint)(_Height-1) && _Instances[n].getInstanceId() >= 0 && _Instances[n+_Width].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n+_Width], 1, _RetrieverBank->getRetrievers());
		_Instances[n+_Width].link(_Instances[n], 3, _RetrieverBank->getRetrievers());
	}

	if (x < (uint)(_Width-1) && _Instances[n].getInstanceId() >= 0 && _Instances[n+1].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n+1], 2, _RetrieverBank->getRetrievers());
		_Instances[n+1].link(_Instances[n], 0, _RetrieverBank->getRetrievers());
	}

	if (y > 0 && _Instances[n].getInstanceId() >= 0 && _Instances[n-_Width].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n-_Width], 3, _RetrieverBank->getRetrievers());
		_Instances[n-_Width].link(_Instances[n], 1, _RetrieverBank->getRetrievers());
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

NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::makeInstance(uint x, uint y, uint32 retriever, uint8 orientation)
{
	CVector	center = getInstanceCenter(x, y);
	return makeInstance(x, y, retriever, orientation, center);
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
													const NLPACS::CGlobalRetriever::CGlobalPosition &end,
													list<CRetrieverInstance::CAStarNodeAccess> &path)
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
			while (pathNode.InstanceId != -1)
			{
				path.push_front(pathNode);
				CRetrieverInstance							&instance = _Instances[pathNode.InstanceId];
				CRetrieverInstance::CAStarNodeInfo			&pathInfo = instance._NodesInformation[pathNode.NodeId];
				nlinfo("pathNode = (InstanceId=%d, NodeId=%d)", pathNode.InstanceId, pathNode.NodeId);
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
				uint	edge = retriever.getChain(chains[i].Chain).getEdge();
				sint	edgeIndex = CChain::convertEdgeId(nextNodeId);
				nextNode.InstanceId = inst.getNeighbor((edge+inst.getOrientation())%4);

				if (nextNode.InstanceId < 0)
					continue;

				nextInstance = &_Instances[nextNode.InstanceId];
				nextRetriever = &(_RetrieverBank->getRetriever(nextInstance->getRetrieverId()));

				uint	chain = inst.getEdgeChainLink(edge, edgeIndex);
				sint	nodeId = nextRetriever->getChain(chain).getLeft();
				nlassert(nodeId >= 0);
				nextNode.NodeId = (uint16)nodeId;
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
		close.push_back(node);
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
}



// ***************************************************************************
// ***************************************************************************
// Collisions part.
// ***************************************************************************
// ***************************************************************************




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
	for(i=0 ; cst.CollisionInstances.size(); i++)
	{
		// get retrieverInstance.
		sint32	curInstance= cst.CollisionInstances[i];
		const CRetrieverInstance	&retrieverInstance= getInstance(curInstance);

		// get delta between startPos.instance and curInstance.
		CVector		deltaOrigin;
		deltaOrigin= retrieverInstance.getOrigin()-origin;

		// Retrieve the localRetriever of this instance.
		const CLocalRetriever		&localRetriever= _RetrieverBank->getRetriever(retrieverInstance.getRetrieverId());

		// compute movement relative to this localRetriever.
		CAABBox		bboxMoveLocal= bboxMove;
		bboxMoveLocal.setCenter(bboxMoveLocal.getCenter()+deltaOrigin);
	
		// add possible collision chains with movement.
		//================
		sint		firstCollisionChain= cst.CollisionChains.size();
		CVector2f	transBase(deltaOrigin.x, deltaOrigin.y);
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
				// TODODO: mgt "NULL" neighbors.


				// Get the good neighbor surfaceId.
				//================
				// get the neighbor instance.
				const CRetrieverInstance	&neighborInstance= getInstance(neighborInstanceId);
				// Get the mirror edge chain id. (1<->3,  0<->2).
				sint	neighborEdgeChain= (edgeChain+2)%4;

				// get the chainId of the neighborInstance 's localRetriever.
				sint	neighborChainId= neighborInstance.getEdgeChainLink(neighborEdgeChain, 
					CChain::convertEdgeId(cc.RightSurface.SurfaceId));
				// get the chain of the neighborInstance 's localRetriever.
				const	CChain		&neighborChain= (_RetrieverBank->getRetriever(neighborInstance.getRetrieverId())).getChain(neighborChainId);

				// Now we have this chain, we are sure that Chain.Left is our SurfaceId of cc.Right.
				cc.RightSurface.SurfaceId= neighborChain.getRight();
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
		CSurfaceIdent startSurface, float radius, TCollisionType colType) const
{
	// start currentSurface with surface start.
	CSurfaceIdent	currentSurface= startSurface;
	uint			nextCollisionSurfaceTested=0;
	sint			i;

	// reset result.
	cst.CollisionDescs.clear();

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
						t= colEdge.testCircle(startCol, deltaCol, radius, normal);
					else if(colType==CGlobalRetriever::Point)
						t= colEdge.testPoint(startCol, deltaCol);

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

			// If we touch a wall, this is the end of search.
			if(currentSurface.SurfaceId<0)
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
const	std::vector<NLPACS::CCollisionSurfaceDesc>	
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
	testCollisionWithCollisionChains(cst, startCol, deltaCol, startSurface, radius, CGlobalRetriever::Circle);


	// result.
	return cst.CollisionDescs;
}



// ***************************************************************************
NLPACS::CGlobalRetriever::CGlobalPosition		
	NLPACS::CGlobalRetriever::doMove(const NLPACS::CGlobalRetriever::CGlobalPosition &startPos, const NLMISC::CVector &delta, float t, NLPACS::CCollisionSurfaceTemp &cst) const
{
	CSurfaceIdent	startSurface(startPos.InstanceId, startPos.LocalPosition.Surface);

	// clamp factor.
	clamp(t, 0.0f, 1.0f);

	// 0. reset.
	//===========
	// reset CollisionDescs.
	cst.CollisionDescs.clear();

	// same move request than prec testMove() ??.
	if( cst.PrecStartSurface != startSurface || 
		cst.PrecStartPos!=startPos.LocalPosition.Estimation || 
		cst.PrecDeltaPos!=delta)
		// if not, just return start.
		return startPos;
	

	// Since we are sure we have same movement than prec testMove(), no need to rebuild cst.CollisionChains.


	// 1. Choose a local basis (same than in testMove()).
	//===========
	// Take the retrieverInstance of startPos as a local basis.
	CVector		origin;
	origin= getInstance(startPos.InstanceId).getOrigin();


	// 2. test collisions with CollisionChains.
	//===========
	CVector		start= startPos.LocalPosition.Estimation;
	CVector2f	startCol(start.x, start.y);
	CVector2f	deltaCol(delta.x, delta.y);
	testCollisionWithCollisionChains(cst, startCol, deltaCol, startSurface, 0, CGlobalRetriever::Point);


	// 3. run all possible collisions to get the new surface where we are now.
	//===========
	CGlobalPosition		res;
	sint				idCol;
	// search on which surface we are now.
	for(idCol=0; idCol<(sint)cst.CollisionDescs.size(); idCol++)
	{
		// test with the next surface.
		if( t<cst.CollisionDescs[idCol].ContactTime )
		{
			break;
		}
	}
	idCol --;

	// particular case: the user ask an impossible movement against a wall. clamp t so that it does not cross the wall.
	if(idCol>=0 && cst.CollisionDescs[idCol].ContactSurface.SurfaceId<0)	// SurfaceId<0 <=> Wall.
	{
		clamp(t, 0, cst.CollisionDescs[idCol].ContactTime);
		// so we are on precedent surface.
		idCol--;
	}

	// if we are still on the same surface.
	if(idCol<0)
	{
		res= startPos;
		res.LocalPosition.Estimation+= delta*t;
	}
	else
	{
		// compute newPos, localy to the startSurface.
		CVector		newPos= startPos.LocalPosition.Estimation+ delta*t;

		res.InstanceId= cst.CollisionDescs[idCol].ContactSurface.RetrieverInstanceId;
		res.LocalPosition.Surface= cst.CollisionDescs[idCol].ContactSurface.SurfaceId;

		// compute newPos, localy to the endSurface.
		// get delta between startPos.instance and curInstance.
		CVector		deltaOrigin;
		deltaOrigin= getInstance(res.InstanceId).getOrigin()-origin;

		// NB: for float precision, it is important to compute deltaOrigin, and after compute newPos in local.
		res.LocalPosition.Estimation= newPos - deltaOrigin;
	}


	// result.
	return res;
}


