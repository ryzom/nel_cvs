/** \file local_retriever.cpp
 *
 *
 * $Id: local_retriever.cpp,v 1.11 2001/05/23 11:57:02 legros Exp $
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

#include "nel/pacs/local_retriever.h"

#include "nel/pacs/collision_desc.h"


using namespace std;
using namespace NLMISC;

/// The max distance allowed to merge tips.
const float	NLPACS::CLocalRetriever::_TipThreshold = 0.1f;


sint32	NLPACS::CLocalRetriever::addSurface(uint8 normalq, uint8 orientationq,
											uint8 mat, uint8 charact, uint8 level,
											const CVector &center,
											const NLPACS::CSurfaceQuadTree &quad)
{
	// creates a new surface...
	sint32	newId = _Surfaces.size();
	_Surfaces.resize(newId+1);
	CRetrievableSurface	&surf = _Surfaces.back();

	// ... and fills it
	surf._NormalQuanta = normalq;
	surf._OrientationQuanta = orientationq;
	surf._Material = mat;
	surf._Character = charact;
	surf._Level = level;
	surf._Quad = quad;
	surf._Center = center;

	// WARNING!! MODIFY THESE IF QUANTAS VALUES CHANGE !!
	surf._IsFloor = (surf._NormalQuanta <= 1);
	surf._IsCeiling = (surf._NormalQuanta >= 3);

	return newId;
}

sint32	NLPACS::CLocalRetriever::addChain(const std::vector<NLMISC::CVector> &vertices,
										  sint32 left, sint32 right, sint edge)
{
	sint32		newId = _Chains.size();
	_Chains.resize(newId+1);
	CChain		&chain = _Chains.back();
/*
	if (left<-2 || left>(sint)_Surfaces.size())
		nlerror ("left surface id MUST be -2<=id<%d (id=%d)", _Surfaces.size(), left);
	if (right<-2 || right>(sint)_Surfaces.size())
		nlerror ("right surface id MUST be -2<=id<%d (id=%d)", _Surfaces.size(), right);
*/
	if (left>(sint)_Surfaces.size())
		nlerror ("left surface id MUST be id<%d (id=%d)", _Surfaces.size(), left);
	if (right>(sint)_Surfaces.size())
		nlerror ("right surface id MUST be id<%d (id=%d)", _Surfaces.size(), right);

	// checks if we can build the chain.
	if (newId > 65535)
		nlerror("in NLPACS::CLocalRetriever::addChain(): reached the maximum number of chains");

	// make the chain and its subchains.
	chain.make(vertices, left, right, _OrderedChains, (uint16)newId, edge);

	CRetrievableSurface	*leftSurface = (left>=0) ? &(_Surfaces[left]) : NULL;
	CRetrievableSurface	*rightSurface = (right>=0) ? &(_Surfaces[right]) : NULL;

	// adds the chain and the link to the surface links vector.
	if (leftSurface != NULL)
		leftSurface->_Chains.push_back(CRetrievableSurface::CSurfaceLink(newId, right));
	if (rightSurface != NULL)
		rightSurface->_Chains.push_back(CRetrievableSurface::CSurfaceLink(newId, left));

	// For each chain find closest start and stop tips
	uint	j;

	// match closest start
	float	closestStartDistance = 1e10f;
	sint	closestStart = -1;
	for (j=0; j<_Tips.size(); ++j)
	{
		float	d = (vertices.front()-_Tips[j].Point).norm();
		if (d < closestStartDistance)
		{
			closestStartDistance = d;
			closestStart = j;
		}
	}

	// if can't find a matching tip, create a new one
	if (closestStart < 0 || closestStartDistance > _TipThreshold)
	{
		_Tips.resize(_Tips.size()+1);
		_Tips.back().Point = vertices.front();
		closestStart = _Tips.size()-1;
	}

	_Tips[closestStart].Chains.push_back(CTip::CChainTip(newId, true));
	chain._StartTip = closestStart;

	// match closest stop
	float	closestStopDistance = 1e10f;
	sint	closestStop = -1;
	for (j=0; j<_Tips.size(); ++j)
	{
		float	d = (vertices.back()-_Tips[j].Point).norm();
		if (d < closestStopDistance)
		{
			closestStopDistance = d;
			closestStop = j;
		}
	}

	if (closestStop < 0 || closestStopDistance > _TipThreshold)
	{
		_Tips.resize(_Tips.size()+1);
		_Tips.back().Point = vertices.back();
		closestStop= _Tips.size()-1;
	}

	_Tips[closestStop].Chains.push_back(CTip::CChainTip(newId, false));
	chain._StopTip = closestStop;

	return newId;
}

// not implemented...
void	NLPACS::CLocalRetriever::sortTips()
{
}


void	NLPACS::CLocalRetriever::findEdgeTips()
{
	uint	i;

/*
	CVector	bmin = _BBox.getMin(),
			bmax = _BBox.getMax();
*/

	// prepares some flags...
	for (i=0; i<_Tips.size(); ++i)
		_Tips[i].Edges &= 0xF;

	// for each chain, checks on which edge it is located
	// then mark the tips of the chain, and adds them to
	// the _EdgeTips tables...
	for (i=0; i<_Chains.size(); ++i)
	{
		CChain	&chain = _Chains[i];

		// it the chain belongs to an edge
		if (chain._Edge >= 0)
		{
			uint	edge = 1<<chain._Edge;
			// extracts start and stop tips.
			uint	start = chain._StartTip,
					stop = chain._StopTip;

			// and marks them (if they were not yet)
			if (!(_Tips[start].Edges & (edge<<4)))
			{
				_EdgeTips[chain._Edge].push_back(start);
				_Tips[start].Edges |= (edge<<4);
			}

			if (!(_Tips[stop].Edges & (edge<<4)))
			{
				_EdgeTips[chain._Edge].push_back(stop);
				_Tips[stop].Edges |= (edge<<4);
			}
		}
	}

	// clears uses flags
	for (i=0; i<_Tips.size(); ++i)
		_Tips[i].Edges &= 0xF;

	// sorts the tips on each edge so we can directly match them at instance linking.
	sort(_EdgeTips[0].begin(), _EdgeTips[0].end(), CYPred(&_Tips, false));
	sort(_EdgeTips[1].begin(), _EdgeTips[1].end(), CXPred(&_Tips, false));
	sort(_EdgeTips[2].begin(), _EdgeTips[2].end(), CYPred(&_Tips, true));
	sort(_EdgeTips[3].begin(), _EdgeTips[3].end(), CXPred(&_Tips, true));
}

void	NLPACS::CLocalRetriever::findEdgeChains()
{
	uint	chain;

	// for each chain, if it belongs to an edge of the
	// local retriever, then adds it to the _EdgeChains tables.
	for (chain=0; chain<_Chains.size(); ++chain)
	{
		sint	edge = _Chains[chain]._Edge;

		if (edge >= 0)
		{
			sint32	numOnEdge = _EdgeChains[edge].size();
			_EdgeChains[edge].push_back(chain);
			// sets index so we can easily retrieve the neighbor after linking.
			_Chains[chain].setIndexOnEdge(edge, numOnEdge);
		}
	}
}

void	NLPACS::CLocalRetriever::updateChainIds()
{
	uint	surf, link;

	for (surf=0; surf<_Surfaces.size(); ++surf)
	{
		CRetrievableSurface	&surface = _Surfaces[surf];

		for (link=0; link<surface._Chains.size(); ++link)
		{
			sint32	chain = surface._Chains[link].Chain;

			if (_Chains[chain]._Left == (sint32)surf)
				surface._Chains[link].Surface = _Chains[chain]._Right;
			else if (_Chains[chain]._Right == (sint32)surf)
				surface._Chains[link].Surface = _Chains[chain]._Left;
			else
			{
				nlwarning("in NLPACS::CLocalRetriever::updateEdgesOnSurfaces()");
				nlerror("Can't find back point to surface %d on chain %d", surf, chain);
			}
		}
	}
}

void	NLPACS::CLocalRetriever::computeTopologies()
{
	nlinfo("compute topologies");

	// Find topologies out...
	uint		character;
	for (character=0; character<NumCreatureModels; ++character)
	{
		// for each type of creature, flood fill surfaces...
		sint32	surface;
		uint	topology = 0;

		for (surface=0; surface<(sint)_Surfaces.size(); ++surface)
		{
			if (_Surfaces[surface]._Topologies[character] == -1 &&
				_Surfaces[surface]._Character == character)
			{
				vector<sint32>	surfacesStack;
				surfacesStack.push_back(surface);

				while (!surfacesStack.empty())
				{
					CRetrievableSurface	&current = _Surfaces[surfacesStack.back()];
					surfacesStack.pop_back();
					current._Topologies[character] = topology;

					uint	i;
					for (i=0; i<current._Chains.size(); ++i)
					{
						CChain	&chain = _Chains[current._Chains[i].Chain];
						sint32	link = (chain.getLeft() == surface) ? chain.getRight() : chain.getLeft();
						if (link>=0 && link<(sint)_Surfaces.size() &&
							_Surfaces[link]._Topologies[character] == -1 &&
							_Surfaces[link]._Character >= character)
						{
							surfacesStack.push_back(link);
							_Surfaces[link]._Topologies[character] = topology;
						}
					}
				}

				++topology;
			}
		}

		_Topologies[character].resize(topology);
		nlinfo("generated %d topologies for character %d", topology, character);
	}

	uint		surface;
	for (surface=0; surface<_Surfaces.size(); ++surface)
	{
		CRetrievableSurface	&current = _Surfaces[surface];

		for (character=0; character<NumCreatureModels; ++character)
			if (current._Topologies[character] >= 0)
				_Topologies[character][current._Topologies[character]].push_back(surface);
	}
}

void	NLPACS::CLocalRetriever::translate(const NLMISC::CVector &translation)
{
	uint	i;
	for (i=0; i<_OrderedChains.size(); ++i)
		_OrderedChains[i].translate(translation);
	for (i=0; i<_Surfaces.size(); ++i)
		_Surfaces[i].translate(translation);
	for (i=0; i<_Tips.size(); ++i)
		_Tips[i].translate(translation);

//	_BBox.setCenter(_BBox.getCenter()+translation);
}

void	NLPACS::CLocalRetriever::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base verison (with collision info).
	*/
	sint	ver= f.serialVersion(0);

	uint	i;
	f.serialCont(_Chains);
	f.serialCont(_OrderedChains);
	f.serialCont(_Surfaces);
	f.serialCont(_Tips);
//	f.serial(_BBox);
//	f.serial(_ZoneId);
	for (i=0; i<4; ++i)
		f.serialCont(_EdgeTips[i]);
	for (i=0; i<4; ++i)
		f.serialCont(_EdgeChains[i]);
	for (i=0; i<NumCreatureModels; ++i)
		f.serialCont(_Topologies[i]);
	f.serial(_ChainQuad);
}




void	NLPACS::CLocalRetriever::retrievePosition(CVector estimated, std::vector<uint8> &retrieveTable) const
{
	uint	ochain;

	CVector2s	estim = CVector2s(estimated);

	// WARNING!!
	// retrieveTable is assumed to be 0 filled !!

	// for each ordered chain, checks if the estimated position is between the min and max.
	for (ochain=0; ochain<_OrderedChains.size(); ++ochain)
	{
		const COrderedChain	&sub = _OrderedChains[ochain];

		// checks the position against the min and max of the chain
		if (estim.x < sub.getVertices().front().x || estim.x > sub.getVertices().back().x)
			continue;

		const vector<CVector2s>	&vertices = sub.getVertices();
		uint					start = 0, stop = vertices.size()-1;

		// TODO: trivial up/down check using bbox.

		// then finds the smallest segment of the chain that includes the estimated position.
		while (stop-start > 1)
		{
			uint	mid = (start+stop)/2;

			if (vertices[mid].x > estim.x)
				stop = mid;
			else
				start = mid;
		}

		// and then checks if the estimated position is up or down the chain.
		bool	isUpper;
		
		// first trivial case (up both tips)
		if (estim.y > vertices[start].y && estim.y > vertices[stop].y)
		{
			isUpper = true;
		}
		// second trivial case (down both tips)
		else if (estim.y < vertices[start].y && estim.y < vertices[stop].y)
		{
			isUpper = false;
		}
		// full test...
		else
		{
			const CVector2s	&vstart = vertices[start],
							&vstop = vertices[stop];
			sint16	intersect = vstart.y + (vstop.y-vstart.y)*(estim.x-vstart.x)/(vstop.y-vstart.y);

			isUpper = estim.y > intersect;
		}

		sint32	left = _Chains[sub.getParentId()].getLeft(),
				right = _Chains[sub.getParentId()].getRight();

		// Depending on the chain is forward, up the position, increase/decrease the surface table...
		if (sub.isForward())
		{
			if (isUpper)
			{
				if (left >= 0)	++retrieveTable[left];
				if (right >= 0)	--retrieveTable[right];
			}
			else
			{
				if (left >= 0)	--retrieveTable[left];
				if (right >= 0)	++retrieveTable[right];
			}
		}
		else
		{
			if (isUpper)
			{
				if (left >= 0)	--retrieveTable[left];
				if (right >= 0)	++retrieveTable[right];
			}
			else
			{
				if (left >= 0)	++retrieveTable[left];
				if (right >= 0)	--retrieveTable[right];
			}
		}
	}
}



// ***************************************************************************
// ***************************************************************************
// Collisions part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	NLPACS::CLocalRetriever::computeCollisionChainQuad()
{
	_ChainQuad.build(_OrderedChains);
}


// ***************************************************************************
void	NLPACS::CLocalRetriever::testCollision(CCollisionSurfaceTemp &cst, const CAABBox &bboxMove, const CVector2f &transBase) const
{
	sint	i;

	// 0. select ordered chains in the chainquad.
	//=====================================
	sint	nEce= _ChainQuad.selectEdges(bboxMove, cst);
	// NB: cst.OChainLUT is assured to be full of 0xFFFF after this call (if was right before).


	// 1. regroup them in chains. build cst.CollisionChains
	//=====================================
	// NB: use cst.OChainLUT to look if a Chain has been inserted before.
	uint16	*chainLUT= cst.OChainLUT;

	// bkup where we begin to add chains.
	uint	firstChainAdded= cst.CollisionChains.size();

	// For all edgechain entry.
	for(i=0;i<nEce;i++)
	{
		CEdgeChainEntry		&ece= cst.EdgeChainEntries[i];
		// this is the ordered chain in the retriever.
		const	COrderedChain	&oChain= this->getOrderedChains()[ece.OChainId];
		// this is the id of the chain is the local retriever.
		uint16				chainId= oChain.getParentId();


		// add/retrieve the id in cst.CollisionChains.
		//=================================
		uint				ccId;
		// if never added.
		if(chainLUT[chainId]==0xFFFF)
		{
			// add a new CCollisionChain.
			ccId= cst.CollisionChains.size();
			cst.CollisionChains.push_back();
			// Fill it with default.
			cst.CollisionChains[ccId].Tested= false;
			cst.CollisionChains[ccId].FirstEdgeCollide= 0xFFFFFFFF;
			cst.CollisionChains[ccId].ChainId= chainId;
			// Fill Left right info.
			cst.CollisionChains[ccId].LeftSurface.SurfaceId= this->getChains()[chainId].getLeft();
			cst.CollisionChains[ccId].RightSurface.SurfaceId= this->getChains()[chainId].getRight();
			// NB: cst.CollisionChains[ccId].*Surface.RetrieverInstanceId is not filled here because we don't have
			// this info at this level.

			// store this Id in the LUT of chains.
			chainLUT[chainId]= ccId;
		}
		else
		{
			// get the id of this collision chain.
			ccId= chainLUT[chainId];
		}

		// add edge collide to the list.
		//=================================
		CCollisionChain			&colChain= cst.CollisionChains[ccId];
		const std::vector<CVector2s>	&oChainVertices= oChain.getVertices();
		for(sint edge=ece.EdgeStart; edge<ece.EdgeEnd; edge++)
		{
			CVector2f	p0= oChainVertices[edge].unpack();
			CVector2f	p1= oChainVertices[edge+1].unpack();

			// alloc a new edgeCollide.
			uint32	ecnId= cst.allocEdgeCollideNode();
			CEdgeCollideNode	&ecn= cst.getEdgeCollideNode(ecnId);

			// append to the front of the list.
			ecn.Next= colChain.FirstEdgeCollide;
			colChain.FirstEdgeCollide= ecnId;

			// build this edge.
			p0+= transBase;
			p1+= transBase;
			ecn.make(p0, p1);
		}
	}



	// 2. Reset LUT to 0xFFFF.
	//=====================================

	// for all collisions chains inserted (starting from firstChainAdded), reset LUT.
	for(i=firstChainAdded; i<(sint)cst.CollisionChains.size(); i++)
	{
		uint	ccId= cst.CollisionChains[i].ChainId;
		chainLUT[ccId]= 0xFFFF;
	}

}


