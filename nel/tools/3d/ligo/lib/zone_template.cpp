/** \file zone_template.cpp
 * Ligo zone template implementation
 *
 * $Id: zone_template.cpp,v 1.1 2001/10/12 13:26:01 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "zone_template.h"
#include "ligo_error.h"
#include "ligo_config.h"

#include "nel/misc/stream.h"

using namespace std;
using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************

inline void CZoneTemplate::snap (float& value, float snap)
{
	// Snap it
	value  = snap * (float) floor ( (value / snap) + 0.5f );
}

// ***************************************************************************

inline bool CZoneTemplate::snapOnGrid (float& value, float resolution, float snap)
{
	// Calc the floor
	float _floor = (float) ( resolution * floor (value / resolution) );
	nlassert (_floor<=value);

	// Calc the remainder
	float remainder = value - _floor;
	nlassert ( (remainder>=0) && (remainder<resolution) );

	// Check the snape
	if ( remainder <= snap )
	{
		// Flag it
		value = _floor;

		// Floor is good
		return true;
	}
	else if ( (resolution - remainder) <= snap )
	{
		// Flag it
		value = _floor + resolution;

		// Floor + resolution is good
		return true;
	}
	return false;
}

// ***************************************************************************

inline bool CZoneTemplate::isSnapedOnGrid (float value, float resolution, float snap)
{
	// Snapped
	float snapped = value;
	return snapOnGrid (snapped, resolution, snap);
}

// ***************************************************************************

inline sint32 CZoneTemplate::getSnappedIndex (float value, float resolution, float snap)
{
	// Snapped
	float snapped = value;

	// This value must be snapped
	nlverify (snapOnGrid (snapped, resolution, snap));

	// Return the index
	return (sint32) floor ( (snapped / resolution) + 0.5f );
}

// ***************************************************************************

bool CZoneTemplate::build (const std::vector<NLMISC::CVector> &vertices, const std::vector< std::pair<uint, uint> > &indexes, const CLigoConfig &config, CLigoError &errors)
{
	// Clear the error message
	errors.clear ();

	// Make an boundary flag array
	vector<uint>		boundaryFlags;

	// Vertices count
	uint vertexCount = vertices.size();

	// Resize the array
	boundaryFlags.resize (vertexCount, 0);

	// *** Build the flag array and the snapped vertex array

	// For each vertices
	uint vertex;
	for (vertex = 0; vertex < vertexCount; vertex++)
	{
		// Snap the point on the X grid
		if (isSnapedOnGrid (vertices[vertex].x, config.CellSize, config.Snap))
			// Flag on X
			boundaryFlags[vertex]|=CZoneTemplateVertex::SnappedXFlag;

		// Snap the point on the Y grid
		if (isSnapedOnGrid (vertices[vertex].y, config.CellSize, config.Snap))
			// Flag on Y
			boundaryFlags[vertex]|=CZoneTemplateVertex::SnappedYFlag;
	}

	// *** Build the edge set
	multimap<uint, uint>	edgePair;
	multimap<uint, uint>	edgePairReverse;

	// Index count
	uint edgeCount = indexes.size();

	// For each vertices
	uint edge;
	for (edge = 0; edge < edgeCount; edge++)
	{
		// Ref on the pair
		const pair<uint, uint> &theEdge = indexes[edge];

		// Vertex snapped ?
		if ( boundaryFlags[theEdge.first] && boundaryFlags[theEdge.second] )
		{
			// Common coordinates
			uint common = boundaryFlags[theEdge.first] & boundaryFlags[theEdge.second];

			// Snapped on the same kind of coordinates ?
			if ( common )
			{
				// Keep this edge ?
				bool keep = false;

				// Snapped both on X ?
				if ( common & CZoneTemplateVertex::SnappedXFlag )
				{
					// Same indexes on x ?
					/*sint32 firstX = getSnappedIndex (vertices[theEdge.first].x, config.CellSize, config.Snap);
					sint32 secondX = getSnappedIndex (vertices[theEdge.second].x, config.CellSize, config.Snap);
					if ( firstX == secondX )*/
					{
						// Keep it
						keep = true;
					}
				}

				// Snapped both on X ?
				if ( common & CZoneTemplateVertex::SnappedYFlag )
				{
					// Same indexes on y ?
					/*sint32 firstY = getSnappedIndex (vertices[theEdge.first].y, config.CellSize, config.Snap);
					sint32 secondY = getSnappedIndex (vertices[theEdge.second].y, config.CellSize, config.Snap);
					if ( firstY == secondY )*/
					{
						// Keep it
						keep = true;
					}
				}

				// Keep this edge ?
				if (keep)
				{
					// Add to the map
					edgePair.insert (map<uint, uint>::value_type(theEdge.first, theEdge.second));
					edgePairReverse.insert (map<uint, uint>::value_type(theEdge.second, theEdge.first));
				}
			}
		}
	}

	// *** Build the list of non included vertices

	// For each vertices
	for (uint i=0; i<vertexCount; i++)
	{
		// Vertex is inserted ?
		if (edgePair.find (i) == edgePair.end())
		{
			// No, add an error message
			errors.pushVertexError (CLigoError::NotInserted, i, 0);
		}
	}

	// *** Build the linked list

	// No vertices found ?
	if (edgePair.begin() == edgePair.end())
	{
		// Error message
		errors.MainError = CLigoError::NoEdgeVertices;
		return false;
	}

	// Build the linked segments
	list<list<uint> >	segmentList;
	multimap<uint, uint>::iterator currentVert = edgePair.begin();

	// For each remaining segment
	while (currentVert != edgePair.end())
	{
		// Get next vert
		uint first = currentVert->first;
		uint next = currentVert->second;

		// New list
		segmentList.push_front (list<uint>());
		list<uint> &listVert = *segmentList.begin();

		// Put the first vertices of the edge list
		listVert.push_back (first);
		listVert.push_back (next);

		// Erase it and
		edgePair.erase (currentVert);

		// Erase the reverse one
		currentVert = edgePairReverse.find (next);
		nlassert (currentVert != edgePairReverse.end());
		edgePairReverse.erase (currentVert);

		// Look forward
		currentVert = edgePair.find (next);
		while (currentVert != edgePair.end())
		{
			// Backup
			uint current = currentVert->first;
			next = currentVert->second;

			// Push the next vertex
			listVert.push_back (next);

			// Erase it and
			edgePair.erase (currentVert);

			// Erase the reverse one
			currentVert = edgePairReverse.find (next);
			nlassert (currentVert != edgePairReverse.end());
			edgePairReverse.erase (currentVert);

			// Look forward
			currentVert = edgePair.find (next);
		}

		// Edgelist ok ?
		if (next != first)
		{
			// No, look backward
			currentVert = edgePairReverse.find (first);
			while (currentVert != edgePairReverse.end())
			{
				// Backup
				uint current = currentVert->second;
				next = currentVert->first;

				// Push the next vertex
				listVert.push_front (current);

				// Erase it
				edgePairReverse.erase (currentVert);

				// Erase the reverse one
				currentVert = edgePair.find (current);
				nlassert (currentVert != edgePair.end());
				edgePair.erase (currentVert);

				// Look forward
				currentVert = edgePairReverse.find (current);
			}
		}

		// Next edge list
		currentVert = edgePair.begin();
	}

	// ** Error traitment

	// Ok
	bool ok = true;

	// Edge index
	uint edgeIndex = 0;

	// List ok ?
	list<list<uint> >::iterator iteList = segmentList.begin ();
	while (iteList != segmentList.end())
	{
		// Only one list
		list<uint> &listVert = *iteList;

		// First and last edge
		uint first = *listVert.begin();
		uint last = *(--listVert.end());

		// Opened edge ?
		if ( first != last )
		{
			// Opened edge
			errors.pushVertexError (CLigoError::OpenedEdge, first, edgeIndex);
			errors.pushVertexError (CLigoError::OpenedEdge, last, edgeIndex);
	
			// Main error
			errors.MainError = CLigoError::OpenedEdge;

			// Not ko
			ok = false;
		}

		// Next edge list
		edgeIndex++;
		iteList++;
	}

	if (segmentList.size () > 1)
	{
		// Main error
		errors.MainError = CLigoError::MultipleEdge;

		// Not ok
		ok = false;
	}

	// Ok ?
	if (ok)
	{
		// Only one list
		list<uint> &listVert = *segmentList.begin ();

		// Test vertex enchainement
		list<uint>::iterator vertIte = listVert.begin();

		// Current vertex id
		uint previous = *(--listVert.end());
		vertIte++;

		// Error vertex set
		set<uint> errored;

		// For each vertices
		while (vertIte != listVert.end ())
		{
			// Vertex id
			uint next = *vertIte;

			// Common flags
			uint commonFlags = boundaryFlags[previous]&boundaryFlags[next];

			// The both on X ?
			if ( commonFlags & CZoneTemplateVertex::SnappedXFlag )
			{
				// Get x index
				sint32 prevIndex = getSnappedIndex (vertices[previous].x, config.CellSize, config.Snap);
				sint32 nextIndex = getSnappedIndex (vertices[next].x, config.CellSize, config.Snap);

				// Not the same ?
				if (prevIndex != nextIndex)
				{
					// Vertex list error
					if (errored.insert (previous).second)
						errors.pushVertexError (CLigoError::VertexList, previous, 0);
					if (errored.insert (next).second)
						errors.pushVertexError (CLigoError::VertexList, next, 0);
			
					// Main error
					errors.MainError = CLigoError::VertexList;
				}
			}

			// Next vertex
			previous = next;
			vertIte++;
		}
		
		// No error ?
		if (errored.empty())
		{
			// Only one list
			nlassert (segmentList.size()==1);

			// First of the list
			vertIte = listVert.begin();

			// Remove first
			listVert.erase (vertIte);
			vertIte = listVert.begin();

			// Resize the vertex array
			_Vertices.resize (listVert.size());

			// Vert index
			uint index=0;

			// For each vertices
			while (vertIte != listVert.end ())
			{
				// Vertices 
				_Vertices[index] = CZoneTemplateVertex (vertices[*vertIte], boundaryFlags[*vertIte]);

				// Next vertices
				vertIte++;
				index++;
			}

			// Return ok
			return true;
		}
	}
	
	// Errors.
	return false;
}

// ***************************************************************************

void CZoneTemplate::serial (NLMISC::IStream& s)
{
	// open an XML node
	s.xmlPush ("LIGO ZONE TEMPLATE");

		// An header file
		s.serialCheck (string ("LIGO ZONE TEMPLATE") );

		// Node for the boundaries
		s.xmlPush ("BOUNDARIES");

			// Serial the Vertices
			s.serialCont (_Vertices);

		// Node for the boundaries
		s.xmlPop ();

	// Close the node
	s.xmlPop ();
}

// ***************************************************************************

}