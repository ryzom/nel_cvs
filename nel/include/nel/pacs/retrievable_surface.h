/** \file retrievable_surface.h
 * 
 *
 * $Id: retrievable_surface.h,v 1.2 2001/05/09 12:59:24 legros Exp $
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

#ifndef NL_RETRIEVABLE_SURFACE_H
#define NL_RETRIEVABLE_SURFACE_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "nel/pacs/surface_quad.h"

namespace NLPACS
{

/**
 * The various features of the models (precisely, the indexes to the table of features).
 */
enum
{
	NumCreatureModels = 4,
	ModelRadius = 0,
	ModelHeight = 1,
	ModelInclineThreshold = 2,
	NumModelCharacteristics = 3
};

/**
 * The features of the models, as floats
 */
extern	float	Models[NumCreatureModels][NumModelCharacteristics];



/**
 * A retrievable surface (inside a local surface retriever). It is composed of
 * the Ids of the chains composing the border of the surface.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CRetrievableSurface
{
public:
	/**
	 * A link from the current surface to a neighbor surface through a chain.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CSurfaceLink
	{
	public:
		sint32							Chain;
		sint32							Surface;
	public:
		CSurfaceLink(sint32 chain=0, sint32 surface=0) : Chain(chain), Surface(surface) {}
		void							serial(NLMISC::IStream &f) { f.serial(Chain, Surface); }
	};


protected:
	friend class CLocalRetriever;

	/// @name Surface features.
	//@{
	uint8								_NormalQuanta;
	uint8								_OrientationQuanta;
	uint8								_Material;
	uint8								_Character;
	uint8								_Level;
	//@}

	/// The links to the neighbor surfaces.
	std::vector<CSurfaceLink>			_Chains;

	/// A Height QuadTree that allows to easily find the height out for a given 2D point.
	CSurfaceQuadTree					_Quad;

	/// The topologies associated with the surface, for each type of model.
	sint32								_Topologies[NumCreatureModels];

public:
	CRetrievableSurface()
	{
		uint	i;
		for (i=0; i<NumCreatureModels; ++i)
			_Topologies[i] = -1;
	}

	uint8								getNormalQuanta() const { return _NormalQuanta; }
	uint8								getOrientationQuanta() const { return _OrientationQuanta; }
	uint8								getMaterial() const { return _Material; }
	uint8								getCharacter() const { return _Character; }
	uint8								getLevel() const { return _Level; }
	const CSurfaceQuadTree				&getQuadTree() const { return _Quad; }
	sint32								getTopology(uint model) const { return _Topologies[model]; }

	const std::vector<CSurfaceLink>		&getChains() const { return _Chains; }

	void								serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_RETRIEVABLE_SURFACE_H

/* End of retrievable_surface.h */
