/** \file vegetable_instance_group.h
 * <File description>
 *
 * $Id: vegetable_instance_group.h,v 1.5 2001/12/05 11:03:50 berenguier Exp $
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

#ifndef NL_VEGETABLE_INSTANCE_GROUP_H
#define NL_VEGETABLE_INSTANCE_GROUP_H

#include "nel/misc/types_nl.h"
#include "3d/tess_list.h"
#include "3d/vegetable_instance_group.h"
#include "3d/vegetable_def.h"
#include <vector>


namespace NL3D 
{


class	CVegetableManager;
class	CVegetableClipBlock;
class	CVegetableVBAllocator;


// ***************************************************************************
/**
 *	A block of vegetable instances. Instaces are added to an IG, and deleted when the ig is deleted.
 *	Internal to VegetableManager. Just an Handle for public.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableInstanceGroup : public CTessNodeList
{
public:

	/// Constructor
	CVegetableInstanceGroup();

	/// tells if the instanceGroup has no faces at all.
	bool			isEmpty() const;


// ***************
private:
	friend class	CVegetableManager;
	friend class	CVegetableClipBlock;
	friend class	CVegetableSortBlock;


	// Who owns us.
	CVegetableClipBlock		*_ClipOwner;
	CVegetableSortBlock		*_SortOwner;


	struct	CVegetableRdrPass
	{
		// vertices are in VBSoft or VBHard ??
		bool					HardMode;
		// List of vertices used (used for deletion of the ig, and also for change of HardMode).
		std::vector<uint32>		Vertices;
		// List of faces indices to render. They points to vertex in VBuffer.
		std::vector<uint32>		TriangleIndices;
		// List of faces indices to render. They points to Vertices in this.
		std::vector<uint32>		TriangleLocalIndices;
		// the number of triangles, ie _TriangleIndices.size()/3.
		uint32					NTriangles;

		CVegetableRdrPass()
		{
			HardMode= true;
			NTriangles= 0;
		}
	};


	// For now, there is only 5 render pass: Lighted and Unlit combined with 2Sided or not + the ZSort one.
	CVegetableRdrPass			_RdrPass[NL3D_VEGETABLE_NRDRPASS];

	// list of triangles order, for quadrant ZSorting. only for NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT rdrpass.
	// this is why this don't appear in CVegetableRdrPass
	std::vector<float>			_TriangleQuadrantOrderArray;
	uint						_TriangleQuadrantOrderNumTriangles;
	float						*_TriangleQuadrantOrders[NL3D_VEGETABLE_NUM_QUADRANT];
	// If the Igs contains some instance in NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT rdrpass, this flag is true.
	bool						_HasZSortPassInstances;

};


// ***************************************************************************
/**
 *	Mirror struct of CVegetableInstanceGroup, for reserveIg system in CVegetableManager.
 *	Internal to VegetableManager. 
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableInstanceGroupReserve
{
public:

	/// Constructor
	CVegetableInstanceGroupReserve();


// ********************
private:
	friend class	CVegetableManager;


	// For each rdrPass, the number of Vertices and Triangles to reserve.
	struct	CVegetableRdrPass
	{
		uint		NVertices;
		uint		NTriangles;

		CVegetableRdrPass()
		{
			NVertices= 0;
			NTriangles= 0;
		}
	};

	// space to be reserved for all rdrPass.
	CVegetableRdrPass			_RdrPass[NL3D_VEGETABLE_NRDRPASS];

};


} // NL3D


#endif // NL_VEGETABLE_INSTANCE_GROUP_H

/* End of vegetable_instance_group.h */
