/** \file vegetable_sort_block.cpp
 * <File description>
 *
 * $Id: vegetable_sort_block.cpp,v 1.5 2002/02/28 12:59:52 besson Exp $
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

#include "std3d.h"

#include "3d/vegetable_sort_block.h"
#include "3d/radix_sort.h"
#include "3d/fast_floor.h"


using namespace std;
using namespace NLMISC;

namespace NL3D 
{


// sort triangles with 2 mm of precision. It leaves 64 m with an uint16, which is really sufficient :)
#define	NL3D_VEGETABLE_TRI_RADIX_KEY_PRECISION	512


// ***************************************************************************
CVegetableSortBlock::CVegetableSortBlock()
{
	ZSortHardMode= true;
	_NTriangles= 0;
	_NIndices= 0;
	_Dirty= false;
	_UnderWater= false;
}


// ***************************************************************************
// the struct to sort a triangle.
struct	CSortTri
{
	// index of the triangle.
	uint16	TriIndex;

	// QSort only.
	// distance.
	float	Dist;
	bool	operator<(const CSortTri &o) const
	{
		return Dist>o.Dist;
	}
};


// ***************************************************************************
void			CVegetableSortBlock::updateSortBlock(CVegetableManager &vegetManager)
{
	// if nothing to update (ie instance added/deleted do not impact me).
	if(!_Dirty)
	{
		// nothing to do.
		return;
	}
	else
	{
		// Ok clean me now.
		_Dirty= false;
	}


	// compute number of triangles.
	_NTriangles= 0;
	CVegetableInstanceGroup		*ptrIg= _InstanceGroupList.begin();
	while(ptrIg)
	{
		// add only zsort rdrPass triangles.
		_NTriangles+= ptrIg->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT].NTriangles;

		// next Ig in the SortBlock
		ptrIg= (CVegetableInstanceGroup*)(ptrIg->Next);
	}
	// compute number of indices
	_NIndices= _NTriangles*3;


	// if no triangles, clear and go
	if(_NTriangles == 0)
	{
		// reset the array of indices.
		_SortedTriangleArray.clear();
		// bye
		return;
	}
	else
	{
		// else, re-allocate the array
		_SortedTriangleArray.resize(_NIndices * NL3D_VEGETABLE_NUM_QUADRANT);
	}


	// resize an array for sorting.
	nlassert(_NTriangles < 65536);
	static	std::vector<CSortTri>	triSort;
	static	std::vector<uint32>		triIndices;
	triSort.clear();
	triSort.resize(_NTriangles);
	triIndices.resize(_NIndices);


	// for all quadrants
	for(uint quadrant=0; quadrant<NL3D_VEGETABLE_NUM_QUADRANT; quadrant++)
	{
		// fill triSort with all ig info.
		//-------------
		CSortTri	*triPtr= &triSort[0];
		uint32		*triIdxPtr= &triIndices[0];
		uint		triId= 0;
		// for all igs in the sortBlock.
		ptrIg= _InstanceGroupList.begin();
		while(ptrIg)
		{
			CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ptrIg->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT];

			// add only zsort rdrPass triangles.
			for(uint i=0; i<vegetRdrPass.NTriangles; i++, triPtr++, triId++)
			{
				// QSort.
				triPtr->Dist = ptrIg->_TriangleQuadrantOrders[quadrant][i];

				// copy tri info
				triPtr->TriIndex= triId;
					
				// fill the triangle indices.
				*(triIdxPtr++)= vegetRdrPass.TriangleIndices[i*3 + 0];
				*(triIdxPtr++)= vegetRdrPass.TriangleIndices[i*3 + 1];
				*(triIdxPtr++)= vegetRdrPass.TriangleIndices[i*3 + 2];
			}

			// next Ig in the SortBlock
			ptrIg= (CVegetableInstanceGroup*)(ptrIg->Next);
		}

		// sort the array according to distance
		//-------------
		// QSort.
		sort(triSort.begin(), triSort.end());


		// Fill result.
		//-------------
		// init quadrant ptr.
		_SortedTriangleIndices[quadrant]= _SortedTriangleArray.getPtr() + quadrant * _NIndices;

		// fill the indices.
		uint32	*pIdx= _SortedTriangleIndices[quadrant];
		for(uint i=0; i<_NTriangles; i++)
		{
			uint32	idTriIdx= triSort[i].TriIndex * 3;
			*(pIdx++)= triIndices[idTriIdx+0];
			*(pIdx++)= triIndices[idTriIdx+1];
			*(pIdx++)= triIndices[idTriIdx+2];
		}
	}
}




} // NL3D
