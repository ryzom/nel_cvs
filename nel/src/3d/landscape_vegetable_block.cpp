/** \file landscape_vegetable_block.cpp
 * <File description>
 *
 * $Id: landscape_vegetable_block.cpp,v 1.1 2001/11/05 16:26:44 berenguier Exp $
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

#include "3d/landscape_vegetable_block.h"
#include "3d/vegetable_manager.h"
#include "3d/vegetable_clip_block.h"
#include "3d/vegetable_instance_group.h"
#include "3d/patch.h"
#include "3d/bezier_patch.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
/*
	Distances type squared.
*/
class	CLVBSqrDistLUT
{
public:
	static	float		Array[NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST+1];

	CLVBSqrDistLUT()
	{
		// 0, 10, 20, 30, 40, 50
		for(uint i=0;i<NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST+1;i++)
		{
			Array[i]= i*NL3D_LANDSCAPE_VEGETABLE_BLOCK_ELTDIST;
			Array[i]= sqr(Array[i]);
		}
	}

};


CLVBSqrDistLUT	NL3D_InitSqrDistLUT;
float		CLVBSqrDistLUT::Array[NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST+1];


// ***************************************************************************
CLandscapeVegetableBlock::CLandscapeVegetableBlock()
{
	_VegetableClipBlock= NULL;
	_CurDistType= NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST;

	for(uint i=0;i<NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST;i++)
	{
		_VegetableIG[i]= NULL;
	}
}



// ***************************************************************************
void			CLandscapeVegetableBlock::init(const CVector &center, CVegetableClipBlock *vegetableClipBlock, CPatch *patch, uint ts, uint tt,
	CTessList<CLandscapeVegetableBlock> &vblist)
{
	nlassert(patch);
	_Center= center;
	_VegetableClipBlock= vegetableClipBlock;
	_Patch= patch;
	_Ts= ts;
	_Tt= tt;

	// append to list.
	vblist.append(this);
}


// ***************************************************************************
void			CLandscapeVegetableBlock::release(CVegetableManager *vegeManager, CTessList<CLandscapeVegetableBlock> &vblist)
{
	// release all Igs.
	for(uint i=0;i<NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST;i++)
	{
		if(_VegetableIG[i])
		{
			vegeManager->deleteIg(_VegetableIG[i]);
			_VegetableIG[i]= NULL;
		}
	}

	// reset state.
	_CurDistType= NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST;

	// remove from list.
	vblist.remove(this);
}

// ***************************************************************************
void			CLandscapeVegetableBlock::update(const CVector &viewCenter, CVegetableManager *vegeManager)
{
	float	sqrDist= (viewCenter-_Center).sqrnorm();

	// compute new distance type. Incremental mode.
	uint	newDistType= _CurDistType;
	while(sqrDist<CLVBSqrDistLUT::Array[newDistType])
	{
		newDistType--;
	}
	while(newDistType<NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST && sqrDist>CLVBSqrDistLUT::Array[newDistType+1])
	{
		newDistType++;
	}
	/*
		NB: to test but may be better than 
			newDistType= floor()(delta.norm() / NL3D_LANDSCAPE_VEGETABLE_BLOCK_ELTDIST);
	*/


	// Change of distance type??
	if(newDistType!=_CurDistType)
	{
		// Erase or create IGs.
		if(newDistType>_CurDistType)
		{
			// Erase no more needed Igs.
			for(uint i=_CurDistType; i<newDistType; i++)
			{
				if(_VegetableIG[i])
				{
					vegeManager->deleteIg(_VegetableIG[i]);
					_VegetableIG[i]= NULL;
				}
			}
		}
		else
		{
			// Create a context for creation.
			CLandscapeVegetableBlockCreateContext	ctx;
			ctx.init(_Patch, _Ts, _Tt);

			// create new Igs.
			for(uint i=newDistType; i<_CurDistType; i++)
			{
				createVegetableIGForDistType(i, vegeManager, ctx);
			}
		}

		// copy new dist type.
		_CurDistType= newDistType;
	}


}


// ***************************************************************************
void			CLandscapeVegetableBlock::createVegetableIGForDistType(uint i, CVegetableManager *vegeManager,
	CLandscapeVegetableBlockCreateContext &vbCreateCtx)
{
	// create the instance group
	CVegetableInstanceGroup		*vegetIg= vegeManager->createIg(_VegetableClipBlock);

	// set
	nlassert(_VegetableIG[i]==NULL);
	_VegetableIG[i]= vegetIg;

	// Add instance in this instance group.
	// Create vegetables instances per tile_material.
	sint	tms,tmt;
	// for all tiles
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);
	for(tmt= _Tt; tmt<_Tt+2; tmt++)
	{
		for(tms= _Ts; tms<_Ts+2; tms++)
		{
			// generate 
			_Patch->generateTileVegetable(vegetIg, i, tms, tmt, vbCreateCtx);
		}
	}

}


// ***************************************************************************
// ***************************************************************************
// CLandscapeVegetableIGCreateContext
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLandscapeVegetableBlockCreateContext::CLandscapeVegetableBlockCreateContext()
{
	_Patch= NULL;
}


// ***************************************************************************
void			CLandscapeVegetableBlockCreateContext::init(CPatch *patch, uint ts, uint tt)
{
	nlassert(patch);
	_Empty= true;
	_Patch= patch;
	_Ts= ts;
	_Tt= tt;
}


// ***************************************************************************
void			CLandscapeVegetableBlockCreateContext::eval(uint ts, uint tt, float x, float y, CVector &pos)
{
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);

	// If never created, do it now
	// ==================
	if(_Empty)
	{
		// Eval Position and normals for the 9 vertices (around the 2x2 tiles)
		for(uint j=0; j<3;j++)
		{
			float	t= (float)(_Tt+j)/_Patch->getOrderT();
			for(uint i=0; i<3;i++)
			{
				float	s= (float)(_Ts+i)/_Patch->getOrderS();
				// eval position.
				_Pos[j*3+i]= _Patch->computeVertex(s, t);
			}
		}

		// Ok, it's done
		_Empty= false;
	}


	// Eval, with simple bilinear
	// ==================
	nlassert(ts==_Ts || ts==_Ts+1);
	nlassert(tt==_Tt || tt==_Tt+1);
	uint	ds= ts-_Ts;
	uint	dt= tt-_Tt;
	// Indices.
	uint	v00= dt*3 + ds;
	uint	v10= v00 + 1;
	uint	v01= v00 + 3;
	uint	v11= v00 + 4;
	// BiLinearFactor.
	float	dxdy= (1-x)*(1-y);
	float	dx2dy= x*(1-y);
	float	dxdy2= (1-x)*y;
	float	dx2dy2= x*y;

	// Compute Pos.
	pos = _Pos[v00] * dxdy;
	pos+= _Pos[v10] * dx2dy;
	pos+= _Pos[v01] * dxdy2;
	pos+= _Pos[v11] * dx2dy2;
}


} // NL3D
