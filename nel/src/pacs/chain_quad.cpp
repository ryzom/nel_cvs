/** \file chain_quad.cpp
 * a quadgrid of list of edge chain.
 *
 * $Id: chain_quad.cpp,v 1.1 2001/05/14 09:58:51 berenguier Exp $
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


#include "nel/pacs/chain_quad.h"
#include "nel/misc/stream.h"
#include <list>
#include <vector>

using	namespace std;
using	namespace NLMISC;


namespace NLPACS
{


// ***************************************************************************
const	float	CChainQuad::_QuadElementSize= 4;	// = 4 meters.


// ***************************************************************************
CChainQuad::CChainQuad()
{
	_QuadData= NULL;
	_QuadDataLen= 0;
}
// ***************************************************************************
CChainQuad::~CChainQuad()
{
	free(_QuadData);
	_QuadData= NULL;
	_QuadDataLen= 0;
}



// ***************************************************************************
void			CChainQuad::getGridBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const CVector &minP, const CVector &maxP) const
{
	x0= (sint32)floor(minP.x / _QuadElementSize) - _X;
	y0= (sint32)floor(minP.y / _QuadElementSize) - _Y;
	x1= (sint32) ceil(maxP.x / _QuadElementSize) - _X;
	y1= (sint32) ceil(maxP.y / _QuadElementSize) - _Y;
	x0= max(x0, (sint32)0);
	y0= max(y0, (sint32)0);
	x1= min(x1, (sint32)_Width);
	y1= min(y1, (sint32)_Height);
}


// ***************************************************************************
void			CChainQuad::build(const std::vector<COrderedChain> &ochains)
{
	vector< list<CChainQuad::CEdgeChainEntry> >	tempQuad;
	sint	i,j;

	// first, clear any pr-build.
	contReset(_Quad);
	free(_QuadData);
	_QuadData= NULL;
	_QuadDataLen= 0;


	// 0. Find BBox of the grid. Allocate grid.
	//=========================================
	bool		first=true;
	CAABBox		chainquadBBox;
	// run all chains.
	for(i=0;i<(sint)ochains.size();i++)
	{
		const std::vector<NLMISC::CVector>	&vertices= ochains[i].getVertices();

		// run all vertices.
		for(j= 0; j<(sint)vertices.size();j++)
		{
			// enlarge bbox.
			if(first)
				first= false, chainquadBBox.setCenter(vertices[j]);
			else
				chainquadBBox.extend(vertices[j]);
		}
	}

	// compute X,Y,Width, Height.
	_X= (sint32)floor(chainquadBBox.getMin().x / _QuadElementSize);
	_Y= (sint32)floor(chainquadBBox.getMin().y / _QuadElementSize);
	_Width= (sint32)ceil(chainquadBBox.getMax().x / _QuadElementSize) - _X;
	_Height= (sint32)ceil(chainquadBBox.getMax().y / _QuadElementSize) - _Y;

	tempQuad.resize(_Width*_Height);
	_Quad.resize(_Width*_Height, NULL);


	// 1. For each edge, add them to the quadgrid.
	//=========================================
	// run all chains.
	for(i=0;i<(sint)ochains.size();i++)
	{
		const std::vector<NLMISC::CVector>	&vertices= ochains[i].getVertices();

		sint	numEdges= (sint)vertices.size()-1;

		// run all edges.
		for(j= 0; j<numEdges; j++)
		{
			const CVector		&p0= vertices[j];
			const CVector		&p1= vertices[j+1];
			CVector		minP,maxP;
			minP.minof(p0, p1);
			maxP.maxof(p0, p1);

			// get bounding coordinate of this edge in the quadgrid.
			sint32	x0, y0, x1, y1;
			getGridBounds(x0, y0, x1, y1, minP, maxP);

			// add this edge to all the quadnode it touch.
			for(sint y= y0; y<y1; y++)
			{
				for(sint x= x0; x<x1; x++)
				{
					list<CChainQuad::CEdgeChainEntry>	&quadNode= tempQuad[y*_Width+x];

					addEdgeToQuadNode(quadNode, i, j);
				}
			}
		}
	}


	// 2. Mem optimisation: Use only 1 block for ALL quads of the grid.
	//=========================================
	sint	memSize= 0;
	// run all quads.
	for(i=0;i<(sint)tempQuad.size();i++)
	{
		list<CChainQuad::CEdgeChainEntry>	&quadNode= tempQuad[i];

		if(!quadNode.empty())
		{
			// add an entry for Len.
			memSize+= sizeof(uint16);
			// add N entry of CEdgeChainEntry.
			memSize+= quadNode.size()*sizeof(CEdgeChainEntry);
		}
	}

	// allocate.
	_QuadData= (uint8*)malloc(memSize);
	_QuadDataLen= memSize;


	// 3. Fill _QuadData with lists.
	//=========================================
	uint8	*ptr= _QuadData;
	for(i=0;i<(sint)tempQuad.size();i++)
	{
		list<CChainQuad::CEdgeChainEntry>			&srcQuadNode= tempQuad[i];
		list<CChainQuad::CEdgeChainEntry>::iterator	it;

		if(!srcQuadNode.empty())
		{
			_Quad[i]= ptr;

			// write len.
			uint16	len= srcQuadNode.size();
			*((uint16*)ptr)= len;
			ptr+= sizeof(uint16);

			// add entries.
			it= srcQuadNode.begin();
			for(j=0; j<len; j++, it++)
			{
				*((CEdgeChainEntry*)ptr)= *it;
				ptr+= sizeof(CEdgeChainEntry);
			}
		}
	}


	// End.
}


// ***************************************************************************
void			CChainQuad::addEdgeToQuadNode(list<CChainQuad::CEdgeChainEntry> &quadNode, sint ochainId, sint edgeId)
{
	// 0. try to find, insert an edge in an existing CEdgeChainEntry.
	//=========================================
	list<CChainQuad::CEdgeChainEntry>::iterator		it;
	for(it= quadNode.begin(); it!=quadNode.end();it++)
	{
		if(it->OChainId==ochainId)
		{
			// selection is faster if we only manages a single start/end block.
			it->EdgeStart= min(it->EdgeStart, (uint16)edgeId);
			it->EdgeEnd= max(it->EdgeEnd, (uint16)(edgeId+1));
		}
	}


	// 1. else, create new one.
	//=========================================
	CEdgeChainEntry		entry;
	entry.OChainId= ochainId;
	entry.EdgeStart= edgeId;
	entry.EdgeEnd= edgeId+1;
	quadNode.push_back(entry);
}


// ***************************************************************************
sint			CChainQuad::selectEdges(const NLMISC::CAABBox &bbox, CEdgeChainEntry *edgeChainArray, uint edgeChainMax, uint16 ochainLUT[65536]) const
{
	sint	nRes= 0;
	sint	i;

	// get bounding coordinate of this bbox in the quadgrid.
	sint32	x0, y0, x1, y1;
	getGridBounds(x0, y0, x1, y1, bbox.getMin(), bbox.getMax());


	// run all intersected quads.
	for(sint y= y0; y<y1; y++)
	{
		for(sint x= x0; x<x1; x++)
		{
			uint8	*quadNode= _Quad[y*_Width+x];

			// no edgechain entry??
			if(!quadNode)
				continue;

			// get edgechain entries
			sint	numEdgeChainEntries= *((uint16*)quadNode);
			quadNode+= sizeof(uint16);
			CEdgeChainEntry		*ptrEdgeChainEntry= (CEdgeChainEntry*)quadNode;

			// This is an error to not have enough place to store result.
			if( numEdgeChainEntries+nRes > (sint)edgeChainMax)
				throw EChainQuad("EChainQuad: Not enough Mem to select all edge for collision");

			// For each one, add it to the result list.
			for(i=0;i<numEdgeChainEntries;i++)
			{
				uint16	ochainId= ptrEdgeChainEntry[i].OChainId;

				// if ochain not yet inserted.
				if(ochainLUT[ochainId]==0xFFFF)
				{
					// inc the list.
					ochainLUT[ochainId]= nRes;
					edgeChainArray[nRes]= ptrEdgeChainEntry[i];
					nRes++;
				}
				else
				{
					// extend the entry in the list.
					uint16 id= ochainLUT[ochainId];
					edgeChainArray[id].EdgeStart= min(edgeChainArray[id].EdgeStart, ptrEdgeChainEntry[i].EdgeStart);
					edgeChainArray[id].EdgeEnd= max(edgeChainArray[id].EdgeEnd, ptrEdgeChainEntry[i].EdgeEnd);
				}
			}
		}
	}


	// reset LUT to 0xFFFF for all ochains selected.
	for(i=0;i<nRes;i++)
	{
		uint16	ochainId= edgeChainArray[i].OChainId;
		ochainLUT[ochainId]= 0xFFFF;
	}


	return nRes;
}


// ***************************************************************************
void		CChainQuad::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);
	uint	i;

	// serial basics.
	f.serial(_X, _Y, _Width, _Height, _QuadDataLen);


	// serial _QuadData.
	if(f.isReading())
	{
		free(_QuadData);
		_QuadData= (uint8*)malloc(_QuadDataLen);
	}
	// Since we have only uint16 (see CEdgeChainEntry), serial them in a single block.
	uint16	*ptrQData= (uint16*)_QuadData;
	for(i=0;i<_QuadDataLen/2; i++, ptrQData++)
	{
		f.serial(*ptrQData);
	}


	// serial _Quad.
	std::vector<uint32>		offsets;
	uint32		len;
	uint32		val;
	if(f.isReading())
	{
		// len/resize.
		f.serial(len);
		offsets.resize(len);
		contReset(_Quad);
		_Quad.resize(len);

		// read offsets -> ptrs.
		for(i=0; i<len; i++)
		{
			f.serial(val);
			if(val== 0xFFFFFFFF)
				_Quad[i]= NULL;
			else
				_Quad[i]= _QuadData+val;
		}
	}
	else
	{
		// len/resize.
		len= _Quad.size();
		f.serial(len);

		// write offsets.
		for(i=0; i<len; i++)
		{
			uint8	*ptr= _Quad[i];
			if(ptr==NULL)
				val= 0xFFFFFFFF;
			else
				val= (uint32)(ptr-_QuadData);
			f.serial(val);
		}
	}

}



} // NLPACS
