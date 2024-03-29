/** \file vertex_neighborhood.cpp
 * TODO: File description
 *
 * $Id: vertex_neighborhood.cpp,v 1.2 2004/11/15 10:25:11 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stdafx.h"
#include "vertex_neighborhood.h"

#define AVERAGE_NUM_POINT 1000

// **********************************************************************

CVertexNeighborhood vertexNeighborhoodGlobal;

// **********************************************************************

CVertexNeighborhood::CVertexNeighborhood()
{
	// Reserve a table for 1000 points
	_VectorIndex.reserve (2*AVERAGE_NUM_POINT+AVERAGE_NUM_POINT*5);
}

// **********************************************************************

void CVertexNeighborhood::build (const PatchMesh& patch)
{
	// Resize the table for the index entry
	_VectorIndex.resize (patch.numVerts*2, 0);

	// Count number of neighbor by vertex
	int i;
	for (i=0; i<patch.numEdges; i++)
	{
		if (patch.edges[i].v1!=-1)
			getNeighborCountRef (patch.edges[i].v1)++;
		if (patch.edges[i].v2!=-1)
			getNeighborCountRef (patch.edges[i].v2)++;
	}
	
	// Commpute the offset for each vertices
	uint finalSize=2*patch.numVerts;
	for (i=0; i<patch.numVerts; i++)
	{
		// Set the offset
		getNeighborIndexRef (i)=finalSize;

		// Increment this offset
		finalSize+=getNeighborCountRef (i);

		// Set size to 0
		getNeighborCountRef (i)=0;
	}

	// Resize the table for final size without erasing offsets
	_VectorIndex.resize (finalSize);

	// Fill the neighborhood info for each vertex
	for (i=0; i<patch.numEdges; i++)
	{
		if (patch.edges[i].v1!=-1)
		{
			// Get the vertex id
			uint vertexId=patch.edges[i].v1;

			// Add the edge to the list
			_VectorIndex[getNeighborIndexRef (vertexId)+getNeighborCountRef (vertexId)]=i;

			// Add a vertex in the list
			getNeighborCountRef (vertexId)++;
		}
		if (patch.edges[i].v2!=-1)
		{
			// Get the vertex id
			uint vertexId=patch.edges[i].v2;

			// Add the edge to the list
			_VectorIndex[getNeighborIndexRef (vertexId)+getNeighborCountRef (vertexId)]=i;

			// Add a vertex in the list
			getNeighborCountRef (vertexId)++;
		}
	}
}
