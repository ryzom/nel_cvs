/** \file exterior_mesh.h
 * 
 *
 * $Id: exterior_mesh.h,v 1.1 2001/07/24 08:44:19 legros Exp $
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

#ifndef NL_EXTERIOR_MESH_H
#define NL_EXTERIOR_MESH_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "pacs/vector_2s.h"
#include "pacs/surface_quad.h"
#include "pacs/chain.h"
#include "pacs/retrievable_surface.h"
#include "pacs/chain_quad.h"

#include "nel/pacs/u_global_position.h"



namespace NLPACS
{

/**
 * The external mesh of a interior local retriever.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CExteriorMesh
{
public:
	struct CEdge
	{
		NLMISC::CVector					Start;
		sint32							Link;
	};

	struct COrderedEdges
	{
		uint32							Start, End;
		bool							Forward;
	};

protected:
	std::vector<CEdge>					_Edges;
	std::vector<COrderedEdges>			_OrderedEdges;

	NLMISC::CAABBox						_BBox;


public:
	/// @name Constructors
	// @{

	CExteriorMesh();

	// @}


	/// @name Selectors
	// @{

	/// Get the set of edges that forms the whole mesh
	const std::vector<CEdge>			&getEdges() const { return _Edges; }

	/// Get the nth edge of the mesh
	const CEdge							getEdge(uint n) const { return _Edges[n]; }


	/// Get the ordered edges
	const std::vector<COrderedEdges>	&getOrderedEdges() const { return _OrderedEdges; }

	/// Get the nth set of ordered edges
	const COrderedEdges					&getOrderedEdges(uint n) const { return _OrderedEdges[n]; }


	/// Get the bbox of the mesh
	const NLMISC::CAABBox				&getBBox() const { return _BBox; }

	// @}


	/// @name Mutators/initialisation
	// @{

	void								setEdges(const std::vector<CEdge> &edges);

	// @}
};

}; // NLPACS

#endif // NL_EXTERIOR_MESH_H

/* End of exterior_mesh.h */
