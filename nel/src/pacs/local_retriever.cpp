/** \file local_retriever.cpp
 *
 *
 * $Id: local_retriever.cpp,v 1.32 2001/09/12 10:07:05 legros Exp $
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
#include "nel/misc/vector_2f.h"
#include "nel/misc/plane.h"

#include "nel/misc/debug.h"

#include "pacs/local_retriever.h"
#include "pacs/collision_desc.h"


using namespace std;
using namespace NLMISC;

/// The max distance allowed to merge tips.
const float	NLPACS::CLocalRetriever::_TipThreshold = 0.1f;
const float	NLPACS::CLocalRetriever::_EdgeTipThreshold = 0.1f;

static float	hybrid2dNorm(const CVector &v)
{
	return (float)(sqrt(sqr(v.x)+sqr(v.y))+fabs(v.z)*0.1);
}





NLPACS::CLocalRetriever::CLocalRetriever()
{
	_Type = Landscape;
	_FaceGrid.clear();
}



const CVector	&NLPACS::CLocalRetriever::getStartVector(uint32 chain) const
{
	const COrderedChain3f	&ochain = _FullOrderedChains[_Chains[chain].getSubChains().front()];
	return (ochain.isForward()) ? ochain.getVertices().front() : ochain.getVertices().back();
}

const CVector	&NLPACS::CLocalRetriever::getStopVector(uint32 chain) const
{
	const COrderedChain3f	&ochain = _FullOrderedChains[_Chains[chain].getSubChains().back()];
	return (ochain.isForward()) ? ochain.getVertices().back() : ochain.getVertices().front();
}




const CVector	&NLPACS::CLocalRetriever::getStartVector(uint32 chain, sint32 surface) const
{
	bool					onLeft = _Chains[chain].getLeft() == surface;
	const COrderedChain3f	&ochain = onLeft ? _FullOrderedChains[_Chains[chain].getSubChains().front()] :
											   _FullOrderedChains[_Chains[chain].getSubChains().back()];

	if (ochain.isForward() && onLeft || !ochain.isForward() && !onLeft)
		return ochain.getVertices().front();
	else
		return ochain.getVertices().back();
}

const CVector	&NLPACS::CLocalRetriever::getStopVector(uint32 chain, sint32 surface) const
{
	bool					onLeft = _Chains[chain].getLeft() == surface;
	const COrderedChain3f	&ochain = onLeft ? _FullOrderedChains[_Chains[chain].getSubChains().back()] :
											   _FullOrderedChains[_Chains[chain].getSubChains().front()];

	if (ochain.isForward() && onLeft || !ochain.isForward() && !onLeft)
		return ochain.getVertices().back();
	else
		return ochain.getVertices().front();
}




uint16			NLPACS::CLocalRetriever::getStartTip(uint32 chain, sint32 surface) const
{
	return (_Chains[chain].getLeft() == surface) ? _Chains[chain].getStartTip() : _Chains[chain].getStopTip();
}

uint16			NLPACS::CLocalRetriever::getStopTip(uint32 chain, sint32 surface) const
{
	return (_Chains[chain].getLeft() == surface) ? _Chains[chain].getStopTip() : _Chains[chain].getStartTip();
}





void			NLPACS::CLocalRetriever::setStartTip(uint32 chain, sint32 surface, uint16 startTip)
{
	if (_Chains[chain].getLeft() == surface)
		_Chains[chain]._StartTip = startTip;
	else
		_Chains[chain]._StopTip = startTip;
}

void			NLPACS::CLocalRetriever::setStopTip(uint32 chain, sint32 surface, uint16 stopTip)
{
	if (_Chains[chain].getLeft() == surface)
		_Chains[chain]._StopTip = stopTip;
	else
		_Chains[chain]._StartTip = stopTip;
}




uint32			NLPACS::CLocalRetriever::getPreviousChain(uint32 chain, sint32 surface) const
{
	uint								loop;
	uint								loopIndex;

	if (_Chains[chain].getLeft() == surface)
	{
		loop = _Chains[chain]._LeftLoop;
		loopIndex = _Chains[chain]._LeftLoopIndex;
	}
	else
	{
		loop = _Chains[chain]._RightLoop;
		loopIndex = _Chains[chain]._RightLoopIndex;
	}

	const CRetrievableSurface			&surf = _Surfaces[surface];
	const CRetrievableSurface::TLoop	&sLoop = surf._Loops[loop];
	return surf._Chains[sLoop[(loopIndex+sLoop.size()-1)%sLoop.size()]].Chain;
}

uint32			NLPACS::CLocalRetriever::getNextChain(uint32 chain, sint32 surface) const
{
	uint								loop;
	uint								loopIndex;

	if (_Chains[chain].getLeft() == surface)
	{
		loop = _Chains[chain]._LeftLoop;
		loopIndex = _Chains[chain]._LeftLoopIndex;
	}
	else
	{
		loop = _Chains[chain]._RightLoop;
		loopIndex = _Chains[chain]._RightLoopIndex;
	}

	const CRetrievableSurface			&surf = _Surfaces[surface];
	const CRetrievableSurface::TLoop	&sLoop = surf._Loops[loop];
	return surf._Chains[sLoop[(loopIndex+1)%sLoop.size()]].Chain;
}




void	NLPACS::CLocalRetriever::unify()
{
	uint	i, j;

	for (i=0; i<_Chains.size(); ++i)
		_Chains[i].unify(_OrderedChains);

	for (i=0; i<_Tips.size(); ++i)
	{
		NLPACS::CLocalRetriever::CTip	&tip = _Tips[i];
		CVector2s ptip = tip.Point;

		for (j=0; j<tip.Chains.size(); ++j)
		{
			if (tip.Chains[j].Start)
			{
				if (_Chains[tip.Chains[j].Chain].getStartVector(_OrderedChains) != ptip)
					nlwarning("chain %d is not stuck to tip %d", tip.Chains[j].Chain, i);
				_Chains[tip.Chains[j].Chain].setStartVector(ptip, _OrderedChains);
			}
			else
			{
				if (_Chains[tip.Chains[j].Chain].getStopVector(_OrderedChains) != ptip)
					nlwarning("chain %d is not stuck to tip %d", tip.Chains[j].Chain, i);
				_Chains[tip.Chains[j].Chain].setStopVector(ptip, _OrderedChains);
			}
		}
	}

	_FullOrderedChains.resize(_OrderedChains.size());
	for (i=0; i<_OrderedChains.size(); ++i)
		_FullOrderedChains[i].unpack(_OrderedChains[i]);
}







void	NLPACS::CLocalRetriever::dumpSurface(uint surf, const CVector &vect) const
{
	const CRetrievableSurface	&surface = _Surfaces[surf];

	nlinfo("dump surf %d", surf);
	nlinfo("%d chains, %d loops", surface._Chains.size(), surface._Loops.size());
	
	uint	i, j, k;

	for (i=0; i<surface._Chains.size(); ++i)
	{
		uint			chainId = surface._Chains[i].Chain;
		const CChain	&chain = _Chains[chainId];
		nlinfo("-- chain %d[%d]: %d sub left=%d right=%d", i, chainId, chain.getSubChains().size(), chain.getLeft(), chain.getRight());

		for (j=0; j<chain.getSubChains().size(); ++j)
		{
			const COrderedChain3f	&ochain = _FullOrderedChains[chain.getSubChain(j)];
			nlinfo("     subchain %d[%d]: fwd=%d parent=%d idx=%d", j, chain.getSubChain(j), ochain.isForward(), ochain.getParentId(), ochain.getIndexInParent());
			for (k=0; k<ochain.getVertices().size(); ++k)
				nlinfo("       v[%d]=(%.3f,%.3f,%.3f)", k, ochain.getVertices()[k].x+vect.x, ochain.getVertices()[k].y+vect.y, ochain.getVertices()[k].z+vect.z);
		}

	}

	for (i=0; i<surface._Loops.size(); ++i)
	{
		const CRetrievableSurface::TLoop	&loop = surface._Loops[i];
		nlinfo("-- loop %d: %d chains length=%.2f", i, loop.size(), loop.Length);
		static char	wbuffer[256];
		static char	buffer[10240];
		sprintf(buffer, "    chains:");
		for (j=0; j<loop.size(); ++j)
		{
			sprintf(wbuffer, " %d[%d]", loop[j], surface._Chains[loop[j]].Chain);
			strcat(buffer, wbuffer);
		}
		nlinfo("%s", buffer);
	}
}


float	NLPACS::CLocalRetriever::distanceToBorder(const ULocalPosition &pos) const
{
	const CRetrievableSurface	&surf = _Surfaces[pos.Surface];
	uint						i, j;
	float						minDist = 1.0e10f, dist;

	for (i=0; i<surf._Chains.size(); ++i)
	{
		const CChain	&chain = _Chains[surf._Chains[i].Chain];
		for (j=0; j<chain.getSubChains().size(); ++j)
		{
			dist = _OrderedChains[chain.getSubChain(j)].distance(pos.Estimation);
			if (dist < minDist)
			{
				minDist = dist;
			}
		}
	}

	return minDist;
}





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

	surf._Flags = 0;
	surf._Flags |= (surf._IsFloor) ? (1<<CRetrievableSurface::IsFloorBit) : 0;
	surf._Flags |= (surf._IsCeiling) ? (1<<CRetrievableSurface::IsCeilingBit) : 0;
	surf._Flags |= (!surf._IsFloor && !surf._IsCeiling) ? (1<<CRetrievableSurface::IsSlantBit) : 0;

	surf._Flags |= ((0xffffffff<<(CRetrievableSurface::NormalQuantasStartBit)) & CRetrievableSurface::NormalQuantasBitMask);

	return newId;
}

sint32	NLPACS::CLocalRetriever::addChain(const vector<CVector> &verts,
										  sint32 left, sint32 right)
{
	vector<CVector>	vertices = verts;
	uint		i;

	if (vertices.size() < 2)
	{
		nlwarning("in NLPACS::CLocalRetriever::addChain()");
		nlwarning("The chain has less than 2 vertices");
		return -1;
	}

	// Remove doubled vertices due to CVector2s snapping
	vector<CVector2s>	converts;

	for (i=0; i<vertices.size(); ++i)
		converts.push_back(CVector2s(vertices[i]));

	vector<CVector2s>::iterator	next2s = converts.begin(), it2s, prev2s;
	prev2s = next2s; ++next2s;
	it2s = next2s; ++next2s;

	vector<CVector>::iterator	it3f = vertices.begin();
	CVector						prev3f = *it3f;
	++it3f;


	for (; it2s != converts.end() && next2s != converts.end(); )
	{
		// if the next point is equal to the previous
		if (*it2s == *prev2s || *it2s == *next2s)
		{
			// then remove the next point
			it2s = converts.erase(it2s);
			it3f = vertices.erase(it3f);

			prev2s = it2s;
			--prev2s;
			next2s = it2s;
			++next2s;
		}
		else
		{
			// else remember the next point, and step to the next...
			++prev2s;
			++it2s;
			++next2s;
			++it3f;
			prev3f = *it3f;
		}
	}
	
	if (vertices.size() < 2)
	{
		nlwarning("in NLPACS::CLocalRetriever::addChain()");
		nlwarning("The chain was snapped to a single point");
		return -1;
	}

	sint32		newId = _Chains.size();
	_Chains.resize(newId+1);
	CChain		&chain = _Chains.back();

	if (left>(sint)_Surfaces.size())
		nlerror ("left surface id MUST be id<%d (id=%d)", _Surfaces.size(), left);
	if (right>(sint)_Surfaces.size())
		nlerror ("right surface id MUST be id<%d (id=%d)", _Surfaces.size(), right);

	// checks if we can build the chain.
	if (newId > 65535)
		nlerror("in NLPACS::CLocalRetriever::addChain(): reached the maximum number of chains");

	CRetrievableSurface	*leftSurface = (left>=0) ? &(_Surfaces[left]) : NULL;
	CRetrievableSurface	*rightSurface = (right>=0) ? &(_Surfaces[right]) : NULL;

	// adds the chain and the link to the surface links vector.
	if (leftSurface != NULL)
		leftSurface->_Chains.push_back(CRetrievableSurface::CSurfaceLink(newId, right));
	if (rightSurface != NULL)
		rightSurface->_Chains.push_back(CRetrievableSurface::CSurfaceLink(newId, left));

	chain._StartTip = 0xffff;
	chain._StopTip = 0xffff;

	// make the chain and its subchains.
	chain.make(vertices, left, right, _OrderedChains, (uint16)newId, _FullOrderedChains);

	return newId;
}




void	NLPACS::CLocalRetriever::computeLoopsAndTips()
{
	// for each surface,
	// examine each chain tip to match another tip inside the surface tips
	// if there is no matching tip, then creates a new one

	uint	i, j;

	for (i=0; i<_Surfaces.size(); ++i)
	{
		CRetrievableSurface	&surface = _Surfaces[i];

		vector<bool>		chainFlags;
		chainFlags.resize(surface._Chains.size());
		for (j=0; j<chainFlags.size(); ++j)
			chainFlags[j] = false;

		uint	totalAdded = 0;

		while (true)
		{
			for (j=0; j<chainFlags.size() && chainFlags[j]; ++j)
				;

			if (j == chainFlags.size())
				break;

			uint32						loopId = surface._Loops.size();
			surface._Loops.push_back(CRetrievableSurface::TLoop());
			CRetrievableSurface::TLoop	&loop = surface._Loops.back();

			CVector	loopStart = getStartVector(surface._Chains[j].Chain, i);
			CVector	currentEnd = getStopVector(surface._Chains[j].Chain, i);
			_Chains[surface._Chains[j].Chain].setLoopIndexes(i, loopId, loop.size());
			loop.push_back(j);
			chainFlags[j] = true;

			float	loopCloseDistance;

			while (true)
			{
//				loopCloseDistance = hybrid2dNorm(loopStart-currentEnd);
				loopCloseDistance = (loopStart-currentEnd).norm();

				// choose the best matching start vector
				sint	bestChain = -1;
				float	best = 1.0e10f;
				CVector	thisStart;
				for (j=0; j<chainFlags.size(); ++j)
				{
					if (chainFlags[j])
						continue;
					thisStart = getStartVector(surface._Chains[j].Chain, i);
//					float	d = hybrid2dNorm(thisStart-currentEnd);
					float	d = (thisStart-currentEnd).norm();
					if (d < best)
					{
						best = d;
						bestChain = j;
					}
				}

				if ((bestChain == -1 || best > 3.0e-2f)&& loopCloseDistance > 3.0e-2f)
				{
					nlwarning("in NLPACS::CLocalRetriever::computeTips()");

					dumpSurface(i);

					for (j=0; j<surface._Chains.size(); ++j)
					{
						CVector	start = getStartVector(surface._Chains[j].Chain, i);
						CVector	end = getStopVector(surface._Chains[j].Chain, i);
						nlinfo("surf=%d chain=%d", i, surface._Chains[j].Chain);
						nlinfo("start=(%f,%f,%f)", start.x, start.y, start.z);
						nlinfo("end=(%f,%f,%f)", end.x, end.y, end.z);
					}
					
					nlwarning("bestChain=%d best=%f", bestChain, best);
					nlwarning("loopCloseDistance=%f", loopCloseDistance);
					nlerror("Couldn't close loop on surface=%d", i);
				}
				else if (best > 1.0e0f && loopCloseDistance < 3.0e-2f ||
						 loopCloseDistance < 1.0e-3f)
				{
					break;
				}

				currentEnd = getStopVector(surface._Chains[bestChain].Chain, i);
				_Chains[surface._Chains[bestChain].Chain].setLoopIndexes(i, loopId, loop.size());
				loop.push_back(bestChain);
				chainFlags[bestChain] = true;
				++totalAdded;
			}
		}
	}

	for (i=0; i<_Chains.size(); ++i)
	{
		uint	whichTip;
		// for both tips (start and stop)
		for (whichTip=0; whichTip<=1; ++whichTip)
		{
			// get the tip id
			uint	thisTip = (whichTip) ? _Chains[i].getStopTip() : _Chains[i].getStartTip();

			if (thisTip != 0xffff && thisTip >= _Tips.size())
			{
				nlwarning("in NLPACS::CLocalRetriever::computeLoopsAndTips()");
				nlerror("checked a tip that doesn't exist on chain %d (tipId=%d)", i, thisTip);
			}

			// if it is unaffected yet creates an new tip and affect it to the common chains
			if (thisTip == 0xffff)
			{
				uint	turn;
				uint	tipId = _Tips.size();
				_Tips.resize(tipId+1);
				CTip	&tip = _Tips[tipId];
				tip.Point = (whichTip) ? getStopVector(i) : getStartVector(i);

				for (turn=0; turn<=1; ++turn)
				{
					uint	chain = i;

					//
					if (whichTip)
						_Chains[chain]._StopTip = tipId;
					else
						_Chains[chain]._StartTip = tipId;

					sint32	surf = (!turn && !whichTip || turn && whichTip) ? _Chains[chain].getLeft() : _Chains[chain].getRight();

					while (surf >= 0)
					{
						CChain	&nextChain = (turn) ? _Chains[chain = getNextChain(chain, surf)] : _Chains[chain = getPreviousChain(chain, surf)];
						bool	isForward = (nextChain.getLeft() == surf);	// tells if the left surf is the current surf
						bool	selectTip = isForward && !turn || !isForward && turn;
						uint16	&tipRef = selectTip ? nextChain._StopTip : nextChain._StartTip;
						surf = (isForward) ? nextChain.getRight() : nextChain.getLeft();

						if (tipRef != 0xffff && tipRef != tipId)
						{
							nlwarning("in NLPACS::CLocalRetriever::computeLoopsAndTips()");
							nlerror("Trying to setup a already created tip (tipId=%d, previous=%d)", tipId, tipRef);
						}
						else if (tipRef != 0xffff)
						{
							break;
						}

						tipRef = tipId;
					}
				}
			}
		}
	}

	for (i=0; i<_Chains.size(); ++i)
	{
		uint	startTip = _Chains[i].getStartTip(),
				stopTip = _Chains[i].getStopTip();

/*
		if (_Chains[i].getEdge() >= 0 && startTip == stopTip)
		{
			nlwarning("NLPACS::CLocalRetriever::computeLoopsAndTips(): chain %d on edge %d has same StartTip and StopTip", i, _Chains[i].getEdge(), startTip, stopTip);
		}
*/

		_Tips[startTip].Chains.push_back(CTip::CChainTip(i, true));
		_Tips[stopTip].Chains.push_back(CTip::CChainTip(i, false));
	}

	for (i=0; i<_Surfaces.size(); ++i)
	{
		for (j=0; j<_Surfaces[i]._Loops.size(); ++j)
		{
			_Surfaces[i]._Loops[j].Length = 0.0f;
			uint	k;

			for (k=0; k<_Surfaces[i]._Loops[j].size(); ++k)
				_Surfaces[i]._Loops[j].Length += _Chains[_Surfaces[i]._Chains[_Surfaces[i]._Loops[j][k]].Chain].getLength();
		}
	}


}



// not implemented...
void	NLPACS::CLocalRetriever::sortTips()
{
}




void	NLPACS::CLocalRetriever::findBorderChains()
{
	uint	chain;

	// for each chain, if it belongs to an edge of the
	// local retriever, then adds it to the _BorderChains.
	for (chain=0; chain<_Chains.size(); ++chain)
		if (_Chains[chain].isBorderChain())
		{
			sint32	index = _BorderChains.size();
			_BorderChains.push_back(chain);
			_Chains[chain].setBorderChainIndex(index);
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
}

void	NLPACS::CLocalRetriever::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version (with collision info).
	Version 1:
		- interior vertices and faces, for interior ground snapping
	Version 2:
		- face grid added.
	Version 3:
		- identifier added.
	*/
	sint	ver= f.serialVersion(3);

	uint	i;
	f.serialCont(_Chains);
	f.serialCont(_OrderedChains);
	f.serialCont(_FullOrderedChains);
	f.serialCont(_Surfaces);
	f.serialCont(_Tips);
	f.serialCont(_BorderChains);
	for (i=0; i<NumCreatureModels; ++i)
		f.serialCont(_Topologies[i]);
	f.serial(_ChainQuad);
	f.serial(_BBox);
	f.serialEnum(_Type);
	f.serial(_ExteriorMesh);

	if (ver >= 1)
	{
		f.serialCont(_InteriorVertices);
		f.serialCont(_InteriorFaces);

	}
	if (ver >= 2)
	{
		f.serial(_FaceGrid);
	}
	if (ver >= 3)
	{
		f.serial(_Id);
	}
}










void	NLPACS::CLocalRetriever::retrievePosition(CVector estimated, std::vector<uint8> &retrieveTable, CCollisionSurfaceTemp &cst) const
{
	CAABBox		box;
	box.setMinMax(CVector(estimated.x, _BBox.getMin().y, 0.0f), CVector(estimated.x, _BBox.getMax().y, 0.0f));
	uint	numEdges = _ChainQuad.selectEdges(box, cst);

	uint	ochain, i;
	CVector2s	estim = CVector2s(estimated);

	// WARNING!!
	// retrieveTable is assumed to be 0 filled !!

/*
	// for each ordered chain, checks if the estimated position is between the min and max.
	for (ochain=0; ochain<_OrderedChains.size(); ++ochain)
	{
*/

	// for each ordered chain, checks if the estimated position is between the min and max.
	for (i=0; i<numEdges; ++i)
	{
		ochain = cst.EdgeChainEntries[i].OChainId;

		const COrderedChain	&sub = _OrderedChains[ochain];
		const CVector2s	&min = sub.getMin(),
						&max = sub.getMax();

		// checks the position against the min and max of the chain
		if (estim.x < min.x || estim.x > max.x)
			continue;

		bool	isUpper;

		if (estim.y < min.y)
		{
			isUpper = false;
		}
		else if (estim.y > max.y)
		{
			isUpper = true;
		}
		else
		{
			const vector<CVector2s>	&vertices = sub.getVertices();
			uint					start = 0, stop = vertices.size()-1;

			/// \todo trivial up/down check using bbox.

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

				if (vstart.x == vstop.x)
				{
					// the very rare case the edge is vertical, and the
					// retrieved position is exactly on the edge...
					isUpper = true;
				}
				else
				{
					sint16	intersect = vstart.y + (vstop.y-vstart.y)*(estim.x-vstart.x)/(vstop.x-vstart.x);
					isUpper = estim.y > intersect;
				}
			}
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


void	NLPACS::CLocalRetriever::initFaceGrid()
{
	CFaceGrid::CFaceGridBuild	fgb;
	fgb.init(64, 4.0f);

	uint	i;
	for (i=0; i<_InteriorFaces.size(); ++i)
	{
		CAABBox			box;
		CInteriorFace	&f = _InteriorFaces[i];
		box.setCenter(_InteriorVertices[f.Verts[0]]);
		box.extend(_InteriorVertices[f.Verts[1]]);
		box.extend(_InteriorVertices[f.Verts[2]]);

		fgb.insert(box.getMin(), box.getMax(), i);
	}

	_FaceGrid.create(fgb);
}

void	NLPACS::CLocalRetriever::snapToInteriorGround(NLPACS::ULocalPosition &position, bool &snapped) const
{
	// first preselect faces around the (x, y) position (CQuadGrid ?)
	vector<uint32>	selection;
	_FaceGrid.select(position.Estimation, selection);

	// from the preselect faces, look for the only face that belongs to the surface
	// and that contains the position
	CVector	pos = position.Estimation;
	CVector	posh = pos+CVector(0.0f, 0.0f, 1.0f);
	float	bestDist = 1.0e10f;
	CVector	best;
	vector<uint32>::iterator	it;
	snapped = false;
	for (it=selection.begin(); it!=selection.end(); ++it)
	{
		const CInteriorFace	&f = _InteriorFaces[*it];
		if (f.Surface == (uint32)position.Surface)
		{
			CVector	v[3];
			v[0] = _InteriorVertices[f.Verts[0]];
			v[1] = _InteriorVertices[f.Verts[1]];
			v[2] = _InteriorVertices[f.Verts[2]];

			float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
			// Line p0-p1.
			a = -(v[1].y-v[0].y);
			b =  (v[1].x-v[0].x);
			c = -(v[0].x*a + v[0].y*b);
			if (a*pos.x + b*pos.y + c < 0)	continue;
			// Line p1-p2.
			a = -(v[2].y-v[1].y);
			b =  (v[2].x-v[1].x);
			c = -(v[1].x*a + v[1].y*b);
			if (a*pos.x + b*pos.y + c < 0)	continue;
			//  Line p2-p0.
			a = -(v[0].y-v[2].y);
			b =  (v[0].x-v[2].x);
			c = -(v[2].x*a + v[2].y*b);
			if (a*pos.x + b*pos.y + c < 0)	continue;

			CPlane	p;
			p.make(v[0], v[1], v[2]);

			CVector i = p.intersect(pos, posh);

			float	d = (float)fabs(pos.z-i.z);

			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}
	}

	// and computes the real position on this face
	if (bestDist < 50.0f)
	{
		snapped = true;
		position.Estimation = best;
	}
}

float	NLPACS::CLocalRetriever::getHeight(const NLPACS::ULocalPosition &position) const
{
	if (_Type == Interior)
	{
		// first preselect faces around the (x, y) position (CQuadGrid ?)
		vector<uint32>	selection;
		_FaceGrid.select(position.Estimation, selection);

		// from the preselect faces, look for the only face that belongs to the surface
		// and that contains the position
		CVector	pos = position.Estimation;
		CVector	posh = pos+CVector(0.0f, 0.0f, 1.0f);
		float	bestDist = 1.0e10f;
		CVector	best;
		vector<uint32>::iterator	it;
		for (it=selection.begin(); it!=selection.end(); ++it)
		{
			const CInteriorFace	&f = _InteriorFaces[*it];
			if (f.Surface == (uint32)position.Surface)
			{
				CVector	v[3];
				v[0] = _InteriorVertices[f.Verts[0]];
				v[1] = _InteriorVertices[f.Verts[1]];
				v[2] = _InteriorVertices[f.Verts[2]];

				float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
				// Line p0-p1.
				a = -(v[1].y-v[0].y);
				b =  (v[1].x-v[0].x);
				c = -(v[0].x*a + v[0].y*b);
				if (a*pos.x + b*pos.y + c < 0)	continue;
				// Line p1-p2.
				a = -(v[2].y-v[1].y);
				b =  (v[2].x-v[1].x);
				c = -(v[1].x*a + v[1].y*b);
				if (a*pos.x + b*pos.y + c < 0)	continue;
				//  Line p2-p0.
				a = -(v[0].y-v[2].y);
				b =  (v[0].x-v[2].x);
				c = -(v[2].x*a + v[2].y*b);
				if (a*pos.x + b*pos.y + c < 0)	continue;

				CPlane	p;
				p.make(v[0], v[1], v[2]);

				CVector i = p.intersect(pos, posh);

				float	d = (float)fabs(pos.z-i.z);

				if (d < bestDist)
				{
					bestDist = d;
					best = i;
				}
			}
		}

		// and computes the real position on this face
		return (bestDist < 50.0f) ?	best.z : position.Estimation.z;
	}
	else
	{
		// find quad leaf.
		const CQuadLeaf	*leaf = _Surfaces[position.Surface].getQuadTree().getLeaf(position.Estimation);

		// if there is no acceptable leaf, just give up
		if (leaf == NULL)
		{
			//nlinfo("COL: quadtree: don't find the quadLeaf!");
			return position.Estimation.z;
		}
		else
		{
			// else return mean height.
			float	meanHeight = (leaf->getMinHeight()+leaf->getMaxHeight())*0.5f;
			return meanHeight;
		}
	}
}



void	NLPACS::CLocalRetriever::findPath(const NLPACS::CLocalRetriever::CLocalPosition &A, 
										  const NLPACS::CLocalRetriever::CLocalPosition &B, 
										  std::vector<NLPACS::CVector2s> &path, 
										  NLPACS::CCollisionSurfaceTemp &cst) const
{
	if (A.Surface != B.Surface)
	{
		nlwarning("in NLPACS::CLocalRetriever::findPath()");
		nlerror("Try to find a path between 2 points that are not in the same surface (A=%d, B=%d)", A.Surface, B.Surface);
	}

	CVector		a = A.Estimation,
				b = B.Estimation,
				n = CVector(a.y-b.y, b.x-a.x, 0.0f);

	_ChainQuad.selectEdges(a, b, cst);

	/// \todo Ben use smart allocations here
	vector<CIntersectionMarker>	intersections;

	uint	i, j;
	sint32	surfaceId = A.Surface;
	const CRetrievableSurface	&surface = _Surfaces[surfaceId];

	for (i=0; i<cst.EdgeChainEntries.size(); ++i)
	{
		CEdgeChainEntry		&entry = cst.EdgeChainEntries[i];
		const COrderedChain	&chain = _OrderedChains[entry.OChainId];

		if (_Chains[chain.getParentId()].getLeft() != surfaceId &&
			_Chains[chain.getParentId()].getRight() != surfaceId)
			continue;

		for (j=entry.EdgeStart; j<entry.EdgeEnd; ++j)
		{
			// here the edge collision test

			CVector	p0 = chain[j].unpack3f(),
					p1 = chain[j+1].unpack3f();

			float	vp0 = (p0-a)*n,
					vp1 = (p1-a)*n;

			if (vp0*vp1 <= 0.0f)
			{
				CVector	np = CVector(p0.y-p1.y, p1.x-p0.x, 0.0f);

				float	va = (a-p0)*np,
						vb = (b-p0)*np;

				// here we have an intersection
				if (va*vb <= 0.0f)
				{
					const CChain	&parent = _Chains[chain.getParentId()];
					bool			isIn = (va-vb < 0.0f) ^ (parent.getLeft() == surfaceId) ^ chain.isForward();

					intersections.push_back(CIntersectionMarker(va/(va-vb), entry.OChainId, j, isIn));
				}
			}
		}
	}

	sort(intersections.begin(), intersections.end());

	uint	intersStart = 0;
	uint	intersEnd = intersections.size();

	if (intersEnd > 0)
	{
		while (intersStart < intersections.size() &&
			   intersections[intersStart].In && intersections[intersStart].Position < 1.0e-4f)
			++intersStart;

		while (intersStart < intersEnd &&
			   !intersections[intersEnd-1].In && intersections[intersEnd-1].Position > 1.0f-1.0e-4f)
			--intersEnd;

		// Check intersections have a valid order
		if ((intersEnd-intersStart) & 1)
		{
			nlwarning("in NLPACS::CLocalRetriever::findPath()");
			nlerror("Found an odd (%d) number of intersections", intersections.size());
		}
		
		for (i=intersStart; i<intersEnd; )
		{
			uint	exitLoop, enterLoop;

			const CChain	&exitChain = _Chains[_OrderedChains[intersections[i].OChain].getParentId()];
			exitLoop = (exitChain.getLeft() == surfaceId) ? exitChain.getLeftLoop() : exitChain.getRightLoop();

			if (intersections[i++].In)
			{
				nlwarning("in NLPACS::CLocalRetriever::findPath()");
				nlerror("Entered the surface before exited", intersections.size());
			}

			const CChain	&enterChain = _Chains[_OrderedChains[intersections[i].OChain].getParentId()];
			enterLoop = (enterChain.getLeft() == surfaceId) ? enterChain.getLeftLoop() : enterChain.getRightLoop();

			if (!intersections[i++].In)
			{
				nlwarning("in NLPACS::CLocalRetriever::findPath()");
				nlerror("Exited twice the surface", intersections.size());
			}

			if (exitLoop != enterLoop)
			{
				nlwarning("in NLPACS::CLocalRetriever::findPath()");
				nlerror("Exited and rentered by a different loop");
			}
		}
	}

//	dumpSurface(surfaceId);

	path.push_back(CVector2s(A.Estimation));

	for (i=intersStart; i<intersEnd; )
	{
		uint								exitChainId = _OrderedChains[intersections[i].OChain].getParentId(),
											enterChainId = _OrderedChains[intersections[i+1].OChain].getParentId();
		const CChain						&exitChain = _Chains[exitChainId],
											&enterChain = _Chains[enterChainId];
		uint								loopId, exitLoopIndex, enterLoopIndex;

		if (exitChain.getLeft() == surfaceId)
		{
			loopId = exitChain.getLeftLoop();
			exitLoopIndex = exitChain.getLeftLoopIndex();
		}
		else
		{
			loopId = exitChain.getRightLoop();
			exitLoopIndex = exitChain.getRightLoopIndex();
		}

		const CRetrievableSurface::TLoop	&loop = surface._Loops[loopId];

		if (enterChain.getLeft() == surfaceId)
			enterLoopIndex = enterChain.getLeftLoopIndex();
		else
			enterLoopIndex = enterChain.getRightLoopIndex();

		float			forwardLength = (exitChain.getLength()+enterChain.getLength())*0.5f;

		sint	loopIndex = exitLoopIndex;
		uint	thisChainId = exitChainId;
		bool	thisChainForward = (enterChain.getLeft() == surfaceId);
		uint	thisOChainId = intersections[i].OChain;
		sint	thisOChainIndex = _OrderedChains[thisOChainId].getIndexInParent();
		bool	forward;

		if (exitChainId != enterChainId)
		{
			for (j=(exitLoopIndex+1)%loop.size(); j!=enterLoopIndex; j=(j+1)%loop.size())
				forwardLength += _Chains[surface._Chains[loop[j]].Chain].getLength();
			forward = (forwardLength <= loop.Length-forwardLength);
		}
		else
		{
			forward = !thisChainForward ^ (_OrderedChains[intersections[i].OChain].getIndexInParent() < _OrderedChains[intersections[i+1].OChain].getIndexInParent());
		}

		path.push_back(CVector2s(A.Estimation+intersections[i].Position*(B.Estimation-A.Estimation)));

		while (true)
		{
			sint	from = (thisOChainId == intersections[i].OChain) ? intersections[i].Edge : -1,
					to = (thisOChainId == intersections[i+1].OChain) ? intersections[i+1].Edge : -1;
			bool	oforward = thisChainForward ^ forward ^ _OrderedChains[thisOChainId].isForward();

			if (from != -1 && to != -1)
				oforward = (intersections[i].Edge < intersections[i+1].Edge);

			_OrderedChains[thisOChainId].traverse(from, to, oforward, path);

			if (thisOChainId == intersections[i+1].OChain)
				break;

			thisOChainIndex = (thisChainForward ^ forward) ? thisOChainIndex-1 : thisOChainIndex+1;

			if (thisOChainIndex < 0 || thisOChainIndex >= (sint)_Chains[thisChainId]._SubChains.size())
			{
				if (forward)
				{
					loopIndex++;
					if (loopIndex == (sint)loop.size())
						loopIndex = 0;
				}
				else
				{
					loopIndex--;
					if (loopIndex < 0)
						loopIndex = loop.size()-1;
				}

				thisChainId = surface._Chains[loop[loopIndex]].Chain;
				thisChainForward = (_Chains[thisChainId].getLeft() == surfaceId);
				thisOChainIndex = (thisChainForward && forward || !thisChainForward && !forward) ? 
					0 : _Chains[thisChainId]._SubChains.size()-1;
			}

			thisOChainId = _Chains[thisChainId]._SubChains[thisOChainIndex];
		}

		path.push_back(CVector2s(A.Estimation+intersections[i+1].Position*(B.Estimation-A.Estimation)));
		i += 2;
	}

	path.push_back(CVector2s(B.Estimation));
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
			cst.CollisionChains[ccId].ExteriorEdge = false;
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


