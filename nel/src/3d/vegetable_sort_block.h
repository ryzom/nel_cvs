/** \file vegetable_sort_block.h
 * <File description>
 *
 * $Id: vegetable_sort_block.h,v 1.1 2001/11/30 13:17:54 berenguier Exp $
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

#ifndef NL_VEGETABLE_SORT_BLOCK_H
#define NL_VEGETABLE_SORT_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "3d/tess_list.h"
#include "3d/vegetable_instance_group.h"


namespace NL3D 
{


using NLMISC::CVector;

class	CSortVSB;

// ***************************************************************************
/**
 *	A block of vegetable instance groups.
 *	CVegetableSortBlock are sorted in Z order.
 *	NB: for speed and convenience, only the RdrPass NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT is sorted.
 *	A block have a number of quadrants (8). Each quadrant has an array of triangles to render.
 *	Internal to VegetableManager. Just an Handle for public.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableSortBlock : public CTessNodeList
{
public:

	/// Constructor
	CVegetableSortBlock();

	// get the center of the sort Block.
	const CVector	&getCenter() const {return _Center;}

	/** After adding some instance to any instance group of a sorted block, you must recall this method
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 */
	void			updateSortBlock(CVegetableManager &vegetManager);


// ***************
private:
	friend class	CVegetableManager;
	friend class	CSortVSB;
	friend class	CVegetableClipBlock;


	// Who owns us.
	CVegetableClipBlock		*_Owner;

	/// \name Fast sorting.
	// @{
	/// center of the sort block.
	CVector			_Center;
	/// approximate Radius of the sort block.
	float			_Radius;

	/// Positive value used for sort. (square of distance to viewer + threshold). temp computed at each render()
	float			_SortKey;
	///	current quadrant used. computed at each render.
	uint			_QuadrantId;

	/// Quadrants.
	std::vector<uint32>		SortedTriangleIndices[NL3D_VEGETABLE_NUM_QUADRANT];
	uint			_NTriangles;

	// @}

	// List of Igs.
	CTessList<CVegetableInstanceGroup>		_InstanceGroupList;

	// ZSort rdrPass Igs must all be in same hardMode. This is the state.
	// NB: this restriction does not apply to other rdrPass
	bool					ZSortHardMode;

};


} // NL3D


#endif // NL_VEGETABLE_SORT_BLOCK_H

/* End of vegetable_sort_block.h */
