/** \file retrievable_surface.h
 * 
 *
 * $Id: retrievable_surface.h,v 1.8.4.1 2003/05/07 09:18:14 legros Exp $
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

#include "pacs/surface_quad.h"

namespace NLPACS
{

/**
 * The various features of the models (precisely, the indexes to the table of features).
 */
enum
{
	NumMaxCreatureModels = 4,
	NumCreatureModels = 2,
	ModelRadius = 0,
	ModelHeight = 1,
	ModelInclineThreshold = 2,
	NumModelCharacteristics = 3
};

/**
 * The features of the models, as floats
 */
extern	float	Models[NumMaxCreatureModels][NumModelCharacteristics];



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
		/// The id of the chain between the 2 neighbors.
		sint32							Chain;

		/// The id of the neighbor.
		sint32							Surface;
	public:
		/// Constructor.
		CSurfaceLink(sint32 chain=0, sint32 surface=0) : Chain(chain), Surface(surface) {}

		/// Serialises this CSurfaceLink.
		void							serial(NLMISC::IStream &f) { f.serial(Chain, Surface); }
	};

	enum
	{
		IsFloorBit = 24,
		IsSlantBit = 25,
		IsCeilingBit = 26,
		IsUnderWaterBit = 27,
		NormalQuantasStartBit = 0,
		NormalQuantasStopBit = 3,
		NormalQuantasBitMask = 0x0000000f,
		CharacterQuantasStartBit = 8,
		CharacterQuantasStopBit = 11,
		CharacterQuantasBitMask = 0x00000f00,
		MaterialQuantasStartBit = 16,
		MaterialQuantasStopBit = 23,
		MaterialQuantasBitMask = 0x00ff0000
	};

	struct TLoop : std::vector<uint16>
	{
		float	Length;
		void	serial(NLMISC::IStream &f);
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
	sint8								_QuantHeight;
	bool								_IsFloor;
	bool								_IsCeiling;
	//@}

	/// Various flags.
	uint32								_Flags;
	float								_WaterHeight;

	/// The links to the neighbor surfaces.
	std::vector<CSurfaceLink>			_Chains;

	/// The loops of chains
	std::vector<TLoop>					_Loops;

	/// A Height QuadTree that allows to easily find the height out for a given 2D point. -- deprecated
	CSurfaceQuadTree					_Quad;

	/// The topologies associated with the surface, for each type of model.
	sint32								_Topologies[NumMaxCreatureModels];

	/// The center of the surface.
	NLMISC::CVector						_Center;

public:
	CRetrievableSurface()
	{
		uint	i;
		for (i=0; i<NumMaxCreatureModels; ++i)
			_Topologies[i] = -1;

		_WaterHeight = 0.0f;
		_QuantHeight = 0;
	}

	uint8								getNormalQuanta() const { return _NormalQuanta; }
	uint8								getOrientationQuanta() const { return _OrientationQuanta; }
	uint8								getMaterial() const { return _Material; }
	uint8								getCharacter() const { return _Character; }
	uint8								getLevel() const { return _Level; }
	bool								isFloor() const { return _IsFloor; }
	bool								isCeiling() const { return _IsCeiling; }
	const CSurfaceQuadTree				&getQuadTree() const { return _Quad; }
	sint32								getTopology(uint model) const { return _Topologies[model]; }
	uint32								getFlags() const { return _Flags; }
	float								getWaterHeight() const { return _WaterHeight; }
	sint8								getQuantHeight() const { return _QuantHeight; }
	float								getMeanHeight() const { return _QuantHeight*2.0f + 1.0f; }

	/// Gets links from this surface to its neighbors through chains...
	const std::vector<CSurfaceLink>		&getChains() const { return _Chains; }
	/// Gets nth link form this surface to its neighbor.
	CSurfaceLink						getChain(uint n) const { return _Chains[n]; }
	/// Get nth loop
	const TLoop							&getLoop(uint n) const { return _Loops[n]; }
	/// Get loops
	const std::vector<TLoop>			&getLoops() const { return _Loops; }

	const NLMISC::CVector				&getCenter() const { return _Center; }

	/// Translates the surface by the translation vector.
	void								translate(const NLMISC::CVector &translation)
	{
		_Center += translation;
		_Quad.translate(translation);
	}
	
	/// Serialises the CRetrievableSurface.
	void								serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_RETRIEVABLE_SURFACE_H

/* End of retrievable_surface.h */
