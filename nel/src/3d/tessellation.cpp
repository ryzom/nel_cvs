/** \file tessellation.cpp
 * <File description>
 *
 * $Id: tessellation.cpp,v 1.2 2000/10/23 14:08:30 berenguier Exp $
 *
 * \todo YOYO: check split(), and lot of todo in computeTileMaterial().
 */

/** Copyright, 2000 Nevrax Ltd.
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

#include "nel/3d/tessellation.h"
#include "nel/3d/patch.h"
#include "nel/misc/common.h"
using namespace NLMISC;
using namespace std;


namespace NL3D {


// ***************************************************************************
// maybe Same as an enum... but may be faster...
const	uint8	TileUvFmt1Tf2= 0;
const	uint8	TileUvFmt2Tf2= 1;
const	uint8	TileUvFmt3Tf2= 2;
const	uint8	TileUvFmt1Tf4= 3;
const	uint8	TileUvFmt2Tf4= 4;
const	uint8	TileUvFmt3Tf4= 5;
// maybe Same as an enum... but may be faster...
const	uint8	TileMatNormal= 0;
const	uint8	TileMatAlpha= 1;
const	uint8	TileMatSpec= 2;
const	uint8	TileMatAlphaSpec= 3;
const	uint8	TileMatSpecAlpha= 4;


// ***************************************************************************
// ***************************************************************************
// CTessFace
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
sint		CTessFace::CurrentDate=0;
CVector		CTessFace::RefineCenter= CVector::Null;
float		CTessFace::RefineThreshold= 0.5;
float		CTessFace::OORefineThreshold= 1.0f / CTessFace::RefineThreshold;

float		CTessFace::FatherStartComputeLimit= 1.1f;
float		CTessFace::SelfEndComputeMin= 0.9f;
float		CTessFace::ChildrenStartComputeLimit= 1.9f;
float		CTessFace::SelfEndComputeMax= 2.1f;

float		CTessFace::TileDistNear= 50;
float		CTessFace::TileDistFar= 75;
float		CTessFace::OOTileDistDelta= 1.0f / (CTessFace::TileDistFar - CTessFace::TileDistNear);
sint		CTessFace::TileMaxSubdivision=4;


// ***************************************************************************
ITileUv		*CTessFace::allocTileUv(uint8 fmt)
{
	switch (fmt)
	{
		case TileUvFmt1Tf2: return new CTileUv1Tf2; break;
		case TileUvFmt2Tf2: return new CTileUv2Tf2; break;
		case TileUvFmt3Tf2: return new CTileUv3Tf2; break;
		case TileUvFmt1Tf4: return new CTileUv1Tf4; break;
		case TileUvFmt2Tf4: return new CTileUv2Tf4; break;
		case TileUvFmt3Tf4: return new CTileUv3Tf4; break;
		default: nlassert(false);
	}
}

	
// ***************************************************************************
CTessFace::CTessFace()
{
	Patch= NULL;
	VBase=VLeft=VRight= NULL;
	FBase=FLeft=FRight= NULL;
	Father=SonLeft=SonRight= NULL;
	Level=0;
	ProjectedSizeDate= 0;
	// Size, Center, paramcoord undetermined.

	// By default, a new face (freshly splitted) don't compute em.
	// Exception: the root faces...
	NeedCompute= false;	

	TilePass0= TilePass1= TilePass2= NULL;
	TileUvBase= TileUvLeft= TileUvRight= NULL;
	// TileId and TileType undefined.
}


// ***************************************************************************
float		CTessFace::updateErrorMetric()
{
	// If already updated for this pass...
	if(ProjectedSizeDate>= CurrentDate)
		return ProjectedSize;

	CVector	viewdir= Center-RefineCenter;
	float	sqrdist= viewdir.sqrnorm();

	// trivial formula.
	//-----------------
	ProjectedSize= Size/ sqrdist;


	// Hoppe97 formula:  k²= a² * ("v-e"² - ((v-e).n)²) / "v-e"^4.
	//-----------------
	// Here, we have:
	// Size==a², where a is the surface error term.  (correct???)
	// k²==em (correct???).
	// v-e==viewdir.
	// "v-e"²== sqrdist.
	// n==Normal.
	/*float	viewscal= viewdir*Normal;
	em= Size * (sqrdist - viewscal*viewscal) / (sqrdist*sqrdist);
	*/
	// Results: Lot of pops!! because this is not uniform, and sons may have a greater error than father.
	// And to complicate updateErrorMetric, may not be a good idea.
	// think to it...


	// Yoyo test. Same as Hoppe97, but with increased impact of perpendicular factor.
	//----------------------------
	/*
	float	perpfact= (sqrdist - sqr(viewscal)) / sqrdist;
	// perpfact E [0,1].  (since "Normal"==1.).
	perpfact= pow(perpfact,2);			// rise it to N.
	em= Size * perpfact / sqrdist;		// remark the similarity with trivial formula...
	*/


	if(Patch->ComputeTileErrorMetric)
	{
		// TileMode Impact. We must split at least at TileLimitLevel.
		if(Level<Patch->TileLimitLevel && sqrdist< sqr(TileDistFar))
		{
			// General formula for Level, function of Size, treshold etc...:
			// Level= log2(BaseSize / sqrdist / threshold);
			// <=> Level= log2( CurSize*2^CurLevel / sqrdist / threshold).
			// <=> Level= log2( ProjectedSize* 2^CurLevel / threshold).
			float	limit;
			limit= (1<<Patch->TileLimitLevel) * RefineThreshold * (OO16384*(16384>>Level));
			nlassert(Level<14);
			// UnOptimised formula: limit= (1<<TileLimitLevel) * RefineThreshold / (1<<Level);
			// If we are not so subdivided.
			if(ProjectedSize<limit)
			{
				if(sqrdist< sqr(TileDistNear))
						ProjectedSize=limit;
				else
				{
					// Smooth transition of tesselation.
					float	a= (float)sqrt(sqrdist);
					float	f= (a- TileDistNear) * OOTileDistDelta;
					ProjectedSize= limit*(1-f) + ProjectedSize*f;
				}
			}
		}
	}

	return ProjectedSize;
}


// ***************************************************************************
void		CTessFace::computeTileMaterial()
{
	// 0. Compute TileId.
	//-------------------
	// Square Order Patch assumption: assume that when a CTessFace become a tile, his base can ONLY be diagonal...
	/* a Patch:
		A ________
		|\      /|
		| \    / |
	   C|__\B_/  |
		|  /  \  |
		| /    \ |
		|/______\|

		Here, if OrderS*OrderT=2*2, ABC is a new CTessFace of a Tile, and AB is the diagonal of the tile.
		Hence the middle of the tile is the middle of AB.

		C must be created, but A and B may be created or copied from neighboor.
	*/
	CParamCoord	middle= (PVLeft+PVRight).shifted();
	sint ts= ((sint)middle.S * (sint)Patch->OrderS) / 0x8000;
	sint tt= ((sint)middle.T * (sint)Patch->OrderT) / 0x8000;
	TileId= tt*Patch->OrderS + ts;
	// TODO: verify the square patch assumption, and verify it can work with rectangular patch (4x8 patchs...)


	// 1. Compute Tile Material.
	//--------------------------
	// TODO: work with patch to create the good vertex format (TileFmt/TileMat), link Pass to the good materials etc...
	// for test only here....
	TileFmt= TileUvFmt1Tf2;
	TileMat= TileMatNormal;


	// 2. Compute Uvs.
	//----------------
	// Must allocate the base uv.
	TileUvBase= allocTileUv(TileFmt);
	// TODO: work with Patch, to create the good texcoordinates.
	// for test only here....
	((CTileUv1Tf2*)TileUvLeft)->P0Uv0.U= PVLeft.S<=middle.S? 0.0f: 1.0f;
	((CTileUv1Tf2*)TileUvLeft)->P0Uv0.V= PVLeft.T<=middle.T? 0.0f: 1.0f;

	// if base neighboor is already at TileLimitLevel just ptr-copy, else create the left/right TileUvs...
	nlassert(!FBase || FBase->Level<=Patch->TileLimitLevel);
	if(FBase && FBase->Level==Patch->TileLimitLevel && FBase->TileUvLeft!=NULL)
	{
		// Just cross-copy the pointers.
		TileUvLeft= FBase->TileUvRight;
		TileUvRight= FBase->TileUvLeft;
	}
	else
	{
		TileUvLeft= allocTileUv(TileFmt);
		TileUvRight= allocTileUv(TileFmt);
		// TODO: work with Patch, to create the good texcoordinates.
		// for test only here....
		((CTileUv1Tf2*)TileUvLeft)->P0Uv0.U= PVLeft.S<=middle.S? 0.0f: 1.0f;
		((CTileUv1Tf2*)TileUvLeft)->P0Uv0.V= PVLeft.T<=middle.T? 0.0f: 1.0f;
		((CTileUv1Tf2*)TileUvRight)->P0Uv0.U= PVRight.S<=middle.S? 0.0f: 1.0f;
		((CTileUv1Tf2*)TileUvRight)->P0Uv0.V= PVRight.T<=middle.T? 0.0f: 1.0f;
	}

}
// ***************************************************************************
void	CTessFace::releaseTileMaterial()
{
	// Hence, must release the tile. TileUvBase is differnet for each of leaves.
	delete TileUvBase;
	TileUvBase= NULL;

	nlassert(!FBase || FBase->Level<=Patch->TileLimitLevel);
	if(FBase && FBase->Level==Patch->TileLimitLevel && FBase->TileUvLeft!=NULL)
	{
		// Do not release Uvs, since neighboor need it...
		TileUvLeft= NULL;
		TileUvRight= NULL;
	}
	else
	{
		delete TileUvLeft;
		delete TileUvRight;
		TileUvLeft= NULL;
		TileUvRight= NULL;
	}
}



// ***************************************************************************
void		CTessFace::split()
{

	// 0. Some easy ending.
	//---------------------
	// Already splitted??
	if(!isLeaf())
		return;
	// Don't do this!!!
	//if(Level>=LS_MAXLEVEL)
	//	return;
	// since split() may reach LS_MAXLEVEL, but enforce splits which outpass this stage!!


	// 1. Create sons, and update links.
	//----------------------------------

	// create and bind Sons.
	SonLeft= new CTessFace;
	SonRight= new CTessFace;

	// Left Son.
	// subdivision.
	SonLeft->Father= this;
	SonLeft->Level= Level+1;
	SonLeft->Size= Size*0.5f;
	// link neighboor face.
	SonLeft->FBase= FLeft;
	if(FLeft)	FLeft->changeNeighboor(this, SonLeft);
	SonLeft->FLeft= SonRight;
	SonLeft->FRight= FBase;		// Temporary. updated later.
	// link neighboor vertex.
	SonLeft->VLeft= VBase;
	SonLeft->VRight= VLeft;
	// Patch management.
	SonLeft->Patch= Patch;
	SonLeft->PVBase= (PVLeft+PVRight).shifted();
	SonLeft->PVLeft= PVBase;
	SonLeft->PVRight= PVLeft;

	// Right Son.
	// subdivision.
	SonRight->Father= this;
	SonRight->Level= Level+1;
	SonRight->Size= Size*0.5f;
	// link neighboor face.
	SonRight->FBase= FRight;
	if(FRight)	FRight->changeNeighboor(this, SonRight);
	SonRight->FLeft= FBase;		// Temporary. updated later.
	SonRight->FRight= SonLeft;
	// link neighboor vertex.
	SonRight->VLeft= VRight;
	SonRight->VRight= VBase;
	// Patch management.
	SonRight->Patch= Patch;
	SonRight->PVBase= (PVLeft+PVRight).shifted();
	SonRight->PVLeft= PVRight;
	SonRight->PVRight= PVBase;

	// FBase->FBase==this. Must Doesn't change this.

	// 2. Update Tile infos.
	//----------------------
	// If new tile ....
	if(SonLeft->Level==Patch->TileLimitLevel)
	{
		SonLeft->computeTileMaterial();
		SonRight->computeTileMaterial();
	}
	// else Tile herit.
	else if(SonLeft->Level > Patch->TileLimitLevel)
	{
		SonLeft->TileId= TileId;
		SonLeft->TileFmt= TileFmt;
		SonLeft->TileMat= TileMat;
		SonLeft->TilePass0= TilePass0;
		SonLeft->TilePass1= TilePass1;
		SonLeft->TilePass2= TilePass2;
		SonLeft->TileUvLeft= TileUvBase;
		SonLeft->TileUvRight= TileUvLeft;

		SonRight->TileId= TileId;
		SonRight->TileFmt= TileFmt;
		SonRight->TileMat= TileMat;
		SonRight->TilePass0= TilePass0;
		SonRight->TilePass1= TilePass1;
		SonRight->TilePass2= TilePass2;
		SonRight->TileUvLeft= TileUvRight;
		SonRight->TileUvRight= TileUvBase;

		// Create, or link to the tileUv.
		ITileUv		*tuv;
		// Try to link to a neighboor TileUv.
		// Can only work iff exist, and iff FBase is same patch, and same TileId.
		if(FBase!=NULL && !FBase->isLeaf() && sameTile(this, FBase) )
		{
			// Ok!! link to the (existing) TileUv.
			// SonLeft!=NULL since FBase->isLeaf()==false.
			tuv= FBase->SonLeft->TileUvBase;
		}
		else
		{
			switch (TileFmt)
			{
				// Create at middle: (TileUvLeft+TileUvRight) /2
				case TileUvFmt1Tf2: tuv= new CTileUv1Tf2( (CTileUv1Tf2*)TileUvLeft, (CTileUv1Tf2*)TileUvRight ); break;
				case TileUvFmt2Tf2: tuv= new CTileUv2Tf2( (CTileUv2Tf2*)TileUvLeft, (CTileUv2Tf2*)TileUvRight ); break;
				case TileUvFmt3Tf2: tuv= new CTileUv3Tf2( (CTileUv3Tf2*)TileUvLeft, (CTileUv3Tf2*)TileUvRight ); break;
				case TileUvFmt1Tf4: tuv= new CTileUv1Tf4( (CTileUv1Tf4*)TileUvLeft, (CTileUv1Tf4*)TileUvRight ); break;
				case TileUvFmt2Tf4: tuv= new CTileUv2Tf4( (CTileUv2Tf4*)TileUvLeft, (CTileUv2Tf4*)TileUvRight ); break;
				case TileUvFmt3Tf4: tuv= new CTileUv3Tf4( (CTileUv3Tf4*)TileUvLeft, (CTileUv3Tf4*)TileUvRight ); break;
				default: nlassert(false);
			};
		}
		
		SonLeft->TileUvBase= tuv;
		SonRight->TileUvBase= tuv;
	}

	// 3. Update/Create Vertex infos.
	//-------------------------------

	// Il reste *->VBase à créer/linker. Let's do It.
	if(FBase==NULL || FBase->isLeaf())
	{
		// The base neighboor is a leaf or NULL. So must create the new vertex.
		CTessVertex	*newVertex= new CTessVertex;
		SonRight->VBase= newVertex;
		SonLeft->VBase= newVertex;

		// Compute pos.
		newVertex->StartPos= (VLeft->EndPos + VRight->EndPos)/2;
		newVertex->EndPos= Patch->computeVertex(SonLeft->PVBase.getS(), SonLeft->PVBase.getT());

		// Init Pos= InitialPos. Important in the case of enforced split.
		newVertex->Pos= newVertex->StartPos;
	}
	else
	{
		// Else, get from neighboor.
		// NB: since *FBase is not a leaf, FBase->SonLeft!=NULL...
		SonRight->VBase= FBase->SonLeft->VBase;
		SonLeft->VBase= FBase->SonLeft->VBase;
	}



	// 4. Compute centers.
	//--------------------
	SonRight->Center= (SonRight->VBase->EndPos + SonRight->VLeft->EndPos + SonRight->VRight->EndPos)/3;
	SonLeft->Center= (SonLeft->VBase->EndPos + SonLeft->VLeft->EndPos + SonLeft->VRight->EndPos)/3;


	// 5. Propagate, or link sons of base.
	//------------------------------------
	// TODO : verify if work.
	// If current face and FBase has sons, just links.
	if(FBase==NULL)
	{
		// Just update sons neighboors.
		SonLeft->FRight= NULL;
		SonRight->FLeft= NULL;
	}
	else if(!FBase->isLeaf())
	{
		// Cross connection of sons.
		SonLeft->FRight= FBase->SonRight;
		FBase->SonRight->FLeft= SonLeft;

		SonRight->FLeft= FBase->SonLeft;
		FBase->SonLeft->FRight= SonRight;
	}
	else
	{
		// Warning: at each iteration, the pointer of FBase may change (because of split() which can change the neighbor 
		// and so "this").
		while(FBase->isLeaf())
			FBase->split();
	}


}
// ***************************************************************************
bool		CTessFace::merge()
{
	CTessFace	*face1,*face2;

	face1= this;
	face2= FBase;


	// 0. Verify if merge is posible.
	//----------------------------
	// Test diamond config (sons must be leaves).
	if(face1->isLeaf())
		return false;
	if(!face1->SonLeft->isLeaf())
		return false;
	if(!face1->SonRight->isLeaf())
		return false;
	if(face2!=NULL)
	{
		nlassert(!face2->isLeaf());
		if(!face2->SonLeft->isLeaf())
			return false;
		if(!face2->SonRight->isLeaf())
			return false;
	}

	// 1. Let's merge.
	//----------------
	// Delete vertex.
	delete face1->SonLeft->VBase;

	// Delete Uv.
	if(face1->SonLeft->Level==Patch->TileLimitLevel)
	{
		// Square patch assumption: the 4 sons are not of the same TileId/Patch.
		nlassert(!sameTile(face1->SonLeft, face1->SonRight));
		if(face2)
		{
			nlassert(!sameTile(face2->SonLeft, face2->SonRight));
			nlassert(!sameTile(face1->SonLeft, face2->SonLeft));
			nlassert(!sameTile(face1->SonRight, face2->SonRight));
			nlassert(!sameTile(face1->SonLeft, face2->SonRight));
			nlassert(!sameTile(face2->SonLeft, face1->SonRight));
		}
		// release tiles.
		face1->SonLeft->releaseTileMaterial();
		face1->SonRight->releaseTileMaterial();
		if(face2)
		{
			face2->SonLeft->releaseTileMaterial();
			face2->SonRight->releaseTileMaterial();
		}
	}
	else if(face1->SonLeft->Level > Patch->TileLimitLevel)
	{
		// If Uv not shared, delete the two Uvs.
		if(face2 && !sameTile(face1, face2))
			delete face2->SonLeft->TileUvBase;
		delete face1->SonLeft->TileUvBase;
	}
	
	// Change father 's neighboor pointers.
	face1->FLeft= face1->SonLeft->FBase;
	if(face1->FLeft)	face1->FLeft->changeNeighboor(face1->SonLeft, face1);
	face1->FRight= face1->SonRight->FBase;
	if(face1->FRight)	face1->FRight->changeNeighboor(face1->SonRight, face1);
	if(face2!=NULL)
	{
		face2->FLeft= face2->SonLeft->FBase;
		if(face2->FLeft)	face2->FLeft->changeNeighboor(face2->SonLeft, face2);
		face2->FRight= face2->SonRight->FBase;
		if(face2->FRight)	face2->FRight->changeNeighboor(face2->SonRight, face2);
	}
	// delete sons.
	delete face1->SonLeft;
	delete face1->SonRight;
	face1->SonLeft=NULL;
	face1->SonRight=NULL;
	if(face2!=NULL)
	{
		delete face2->SonLeft;
		delete face2->SonRight;
		face2->SonLeft=NULL;
		face2->SonRight=NULL;
	}

	return true;
}
// ***************************************************************************
void		CTessFace::refine()
{
	/*
		if(ps<RefineThreshold), the face must be merged (ie have no leaves).
		if(ps E [RefineThreshold, RefineTreshold*2]), the face must be splitted (ave leaves), and is geomorphed.
		if(ps>RefineThreshold*2), the face is fully splitted/geomoprhed (tests reported on sons...).
	*/

	if(NeedCompute)
	{
		float	ps=updateErrorMetric();
		ps*= OORefineThreshold;
		// 1.0f is the point of split().
		// 2.0f is the end of geomorph.


		// 0. Test split/merge.
		//---------------------
		// If wanted, not already done, and limit not reached, split().
		if(ps>1.0f && isLeaf() && Level<= (Patch->TileLimitLevel+TileMaxSubdivision) )
		{
			split();
		}
		// Else, if splitted, must merge (if not already the case).
		else if(ps<1.0f && !isLeaf())
		{
			// Merge only if neighboor agree.
			if(FBase)
			{
				float	ps2= FBase->updateErrorMetric();
				ps2*= OORefineThreshold;
				if(ps2<1.0f)
					merge();
			}
			else
			{
				merge();
			}
		}


		// 1. Geomorph.
		//-------------
		// NB: the geomoprh may be done twice here (this, and FBase).
		// But errorMetric are not computed twice.
		if(ps>1.0f && !isLeaf())
		{
			CTessVertex		*morph=SonLeft->VBase;
			float	pgeom= ps;

			// get the neighboor, to have the max projectedsize.
			// => geo-split() at the maximum level of the two faces.
			if(FBase)
			{
				float	ps2= FBase->updateErrorMetric();
				ps2*= OORefineThreshold;
				pgeom= max(pgeom, ps2);
			}

			// Interpolate beetween trheshold and treshold*2 => beetween 1.0 and 2.0
			float		delta= pgeom - 1.0f;
			clamp(delta, 0.0f,1.0f);
			morph->Pos= morph->StartPos *(1-delta) + morph->EndPos * delta;
		}



		// 2. Update NeedCompute.
		//-----------------------
		// If near a merge()/split(), parents and children should compute themselves.
		if(Father && ps<FatherStartComputeLimit)		// 1.1
		{
			// The father may merge soon.
			Father->NeedCompute= true;

			// Should I compute myself again??
			if(ps<SelfEndComputeMin)					// 0.9
				NeedCompute= false;
		}
		if(!isLeaf() && ps>ChildrenStartComputeLimit)	// 1.9
		{
			// The sons may split soon.
			SonLeft->NeedCompute= true;
			SonRight->NeedCompute= true;

			// Should I compute myself again?? 
			// (after geomorph ended).
			if(ps>SelfEndComputeMax)					// 2.1
				NeedCompute= false;
		}
	}
	if(SonLeft)
	{
		SonLeft->refine();
		SonRight->refine();
	}


	// Unstable case: the child tell to his father "NeedCompute=true", but the father previously said "NeedCompute=false".
	// It is a normal case, at the next father->refine(), he will compute himslef.
}



} // NL3D
