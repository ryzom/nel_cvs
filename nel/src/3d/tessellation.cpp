/** \file tessellation.cpp
 * <File description>
 *
 * $Id: tessellation.cpp,v 1.12 2000/11/14 14:54:56 lecroart Exp $
 *
 * \todo YOYO: check split(), and lot of todo in computeTileMaterial().
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

#include "nel/3d/tessellation.h"
#include "nel/3d/patch.h"
#include "nel/3d/zone.h"
#include "nel/misc/common.h"
using namespace NLMISC;
using namespace std;


namespace NL3D {


// ***************************************************************************
// The normal Uvs format.
const	uint8	TileUvFmtNormal1= 0;
const	uint8	TileUvFmtNormal2= 1;
const	uint8	TileUvFmtNormal3= 2;
const	uint8	TileUvFmtNormal4= 3;
const	uint8	TileUvFmtNormal5= 4;
const	uint8	TileUvFmtNormal6= 5;
// The bumped ones.
const	uint8	TileUvFmtBump1= 16+0;
const	uint8	TileUvFmtBump2= 16+1;
const	uint8	TileUvFmtBump3= 16+2;
const	uint8	TileUvFmtBump4= 16+3;
const	uint8	TileUvFmtBump5= 16+4;
const	uint8	TileUvFmtBump6= 16+5;




// ***************************************************************************
// ***************************************************************************
// CPatchRdrPass
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
// Primitives Indices reallocation. must be >16 (see below..)
static sint		GlobalTriListBlockRealloc= 1024;


// ***************************************************************************
sint			CPatchRdrPass::CurGlobalIndex=0;
std::vector<uint32>	CPatchRdrPass::GlobalTriList;

	
// ***************************************************************************
CPatchRdrPass::CPatchRdrPass()
{
	resetTriList();
}

// ***************************************************************************
void			CPatchRdrPass::resetTriList()
{
	NTris=0;
	StartIndex=0;
	CurIndex=0;
	BlockLenIndex=0;
}

// ***************************************************************************
void			CPatchRdrPass::addTri(uint32 idx0, uint32 idx1, uint32 idx2)
{
	// An error may occurs if resetGlobalTriList() called, but not resetTriList().
	nlassert(CurIndex<=CurGlobalIndex);

	// Realloc if necessary.
	// Keep a security of 16 spaces (think that 5/6 only is needed).
	// Nb: 6, because we need one more space for the last JMP of the last block of the last material...
	// Don't bother, 16 is cool....
	if((sint)GlobalTriList.size() < CurGlobalIndex+16)
	{
		GlobalTriList.resize(GlobalTriList.size() + NL3D::GlobalTriListBlockRealloc);
	}

	// First, if current material interleaved, jump.
	if(CurIndex!=CurGlobalIndex)
	{
		// Leave a "jump" space for old material.
		CurGlobalIndex++;
		// This material "jump" to current index.
		if(NTris!=0)	// Only if the list is not empty.
		{
			// Old block (with CurIndex which points to the jump space) must point to cur.
			// Mark this index so we know it is a Jump index.
			GlobalTriList[CurIndex]= CurGlobalIndex | 0x80000000;
			// Start of a new block!!
			BlockLenIndex= CurGlobalIndex++;
			GlobalTriList[BlockLenIndex]=0;
		}
	}

	// insert!!
	if(NTris==0)
	{
		// Start of a new block!!
		StartIndex= BlockLenIndex= CurGlobalIndex++;
		GlobalTriList[BlockLenIndex]=0;
	}
	GlobalTriList[CurGlobalIndex++]= idx0;
	GlobalTriList[CurGlobalIndex++]= idx1;
	GlobalTriList[CurGlobalIndex++]= idx2;

	// CurIndex point to the next global index...
	CurIndex= CurGlobalIndex;
	NTris++;
	GlobalTriList[BlockLenIndex]++;
}

// ***************************************************************************
void			CPatchRdrPass::buildPBlock(CPrimitiveBlock &pb)
{
	sint	idx, n, blocklen;

	pb.setNumTri(NTris);
	uint32	*pi= pb.getTriPointer();

	// Run the list of block.
	n= NTris;
	idx= StartIndex;
	while(n>0)
	{
		// size of block (in tris).
		blocklen= GlobalTriList[idx];
		// Copy the indices (jump the BlockLenIndex).
		memcpy(pi, &GlobalTriList[idx+1], blocklen*3*sizeof(uint32));
		// Jump to the next block!! (not valid if last block, but doesn't matter...)
		idx= GlobalTriList[1+blocklen*3];
		pi+= blocklen*3;
		n-= blocklen;
	}
}

// ***************************************************************************
void			CPatchRdrPass::resetGlobalTriList()
{
	CurGlobalIndex= 0;
}



// ***************************************************************************
// ***************************************************************************
// CTileMaterial
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTileMaterial::CTileMaterial()
{
	for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
	{
		Pass[i]=NULL;
	}
}


// ***************************************************************************
// ***************************************************************************
// CTessFace
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
sint		CTessFace::CurrentDate=0;
CVector		CTessFace::RefineCenter= CVector::Null;
float		CTessFace::RefineThreshold= 0.1f;
float		CTessFace::OORefineThreshold= 1.0f / CTessFace::RefineThreshold;

float		CTessFace::FatherStartComputeLimit= 1.1f;
float		CTessFace::ChildrenStartComputeLimit= 1.9f;
float		CTessFace::SelfEndCompute= 2.1f;

float		CTessFace::TileDistNear= 20;
float		CTessFace::TileDistEndGeom= CTessFace::TileDistNear-10;
float		CTessFace::TileDistFar= CTessFace::TileDistNear+40;
float		CTessFace::TileDistEndGeomSqr= sqr(CTessFace::TileDistEndGeom);
float		CTessFace::TileDistNearSqr= sqr(CTessFace::TileDistNear);
float		CTessFace::TileDistFarSqr= sqr(CTessFace::TileDistFar);
float		CTessFace::OOTileDistDeltaGeomSqr= 1.0f / (CTessFace::TileDistNearSqr - CTessFace::TileDistEndGeomSqr);
float		CTessFace::OOTileDistDeltaSqr= 1.0f / (CTessFace::TileDistFarSqr - CTessFace::TileDistNearSqr);
sint		CTessFace::TileMaxSubdivision=4;

float		CTessFace::Far0Dist= 200;		// 200m.
float		CTessFace::Far1Dist= 400;		// 400m.
float		CTessFace::FarTransition= 10;	// Alpha transition= 10m.
CVertexBuffer	*CTessFace::CurrentVB=NULL;
sint		CTessFace::CurrentVertexIndex=1;


CTessFace	CTessFace::CantMergeFace;


// ***************************************************************************
ITileUv		*CTessFace::allocTileUv(uint8 fmt)
{
	switch (fmt)
	{
		case TileUvFmtNormal1: return new CTileUvNormal1; break;
		case TileUvFmtNormal2: return new CTileUvNormal2; break;
		case TileUvFmtNormal3: return new CTileUvNormal3; break;
		case TileUvFmtNormal4: return new CTileUvNormal4; break;
		case TileUvFmtNormal5: return new CTileUvNormal5; break;
		case TileUvFmtNormal6: return new CTileUvNormal6; break;
		case TileUvFmtBump1: return new CTileUvBump1; break;
		case TileUvFmtBump2: return new CTileUvBump2; break;
		case TileUvFmtBump3: return new CTileUvBump3; break;
		case TileUvFmtBump4: return new CTileUvBump4; break;
		case TileUvFmtBump5: return new CTileUvBump5; break;
		case TileUvFmtBump6: return new CTileUvBump6; break;
		default: nlstop;
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

	TileMaterial= NULL;
	TileUvBase= TileUvLeft= TileUvRight= NULL;
	// TileId and TileType undefined.
}


// ***************************************************************************
CTessFace::~CTessFace()
{
	// Old Code. This is not sufficient to clear the CTessFace.
	// Vertices and Uvs must be correctly cleared too (but difficult because of sharing).
	/*
	delete SonLeft;
	delete SonRight;

	// update neighbors.
	if(FBase)	FBase->changeNeighbor(this, NULL);
	if(FLeft)	FLeft->changeNeighbor(this, NULL);
	if(FRight)	FRight->changeNeighbor(this, NULL);

	FBase=FLeft=FRight= NULL;
	*/
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


	// TileMode Impact.
	//-----------------
	if(Patch->Zone->ComputeTileErrorMetric)
	{
		// TileMode Impact. We must split at least at TileLimitLevel.
		if(Level<Patch->TileLimitLevel)
		{
			// We msut take a more correct errometric here: We must have sons face which have
			// lower projectedsize than father. This is not the case if Center of face is taken (but when not in
			// tile mode this is nearly the case). So take the min dist from 3 points.
			float	s0= (VBase->EndPos-RefineCenter).sqrnorm();
			float	s1= (VLeft->EndPos-RefineCenter).sqrnorm();
			float	s2= (VRight->EndPos-RefineCenter).sqrnorm();
			sqrdist= minof(s0, s1, s2);
			// It is also VERY important to take the min of 3, to ensure the split in TileMode when Far1 vertex begin
			// to blend (si Patch::renderFar1() render).

			if(sqrdist< TileDistFarSqr)
			{
				// General formula for Level, function of Size, treshold etc...:
				// Level= log2(BaseSize / sqrdist / threshold);
				// <=> Level= log2( CurSize*2^CurLevel / sqrdist / threshold).
				// <=> Level= log2( ProjectedSize* 2^CurLevel / threshold).
				float	endgeomLimit;
				// UnOptimised formula: limit= (1<<Patch->TileLimitLevel) * RefineThreshold / (1<<Level);
				endgeomLimit= (1<<Patch->TileLimitLevel) * RefineThreshold * (OO32768*(32768>>Level));
				nlassert(Level<14);
				// If we are not so subdivided.
				if(ProjectedSize<endgeomLimit)
				{
					// To have a better smooth transition, it is only necessary to have the tile splitted, 
					// not the tile fully geomorphed. => *=0.51 to ensure the split (not 0.5 for precision problem).
					float	nearLimit= endgeomLimit*0.51f;
					// But, AFTER the tile transition is made, since we are splitted, it is interressant to geomorph.
					if(sqrdist< TileDistEndGeomSqr)
					{
						ProjectedSize=endgeomLimit;
					}
					else if( sqrdist< TileDistNearSqr)
					{
						// Do the geomorpgh beetween TileDistNearSqr and TileDistEndGeomSqr.
						float	f= (sqrdist- TileDistEndGeomSqr) * OOTileDistDeltaGeomSqr;
						float	ps= endgeomLimit*(1-f) + nearLimit*f;
						ProjectedSize= max(ps, ProjectedSize);
					}
					else
					{
						// Smooth transition to the nearLimit of tesselation.
						float	f= (sqrdist- TileDistNearSqr) * OOTileDistDeltaSqr;
						// This gives better result, by smoothing more the start of transition.
						f= sqr((1-f));
						f= sqr(f);
						ProjectedSize= nearLimit*f + ProjectedSize*(1-f);
					}
				}
			}
		}
	}

	ProjectedSizeDate= CurrentDate;
	
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

		C must be created, but A and B may be created or copied from neighbor.
	*/
	CParamCoord	middle(PVLeft,PVRight);
	sint ts= ((sint)middle.S * (sint)Patch->OrderS) / 0x8000;
	sint tt= ((sint)middle.T * (sint)Patch->OrderT) / 0x8000;
	TileId= tt*Patch->OrderS + ts;
	// TODO: verify the square patch assumption, and verify it can work with rectangular patch (4x8 patchs...)


	// 1. Compute Tile Material.
	//--------------------------
	// if base neighbor is already at TileLimitLevel just ptr-copy, else create the TileMaterial...
	nlassert(!FBase || FBase->Level<=Patch->TileLimitLevel);
	if(FBase && FBase->Level==Patch->TileLimitLevel && FBase->TileUvLeft!=NULL)
	{
		TileMaterial= FBase->TileMaterial;
	}
	else
	{
		// TODO_TEXTURE: work with patch to create the good vertex format (TileMaterial), link Pass to the good materials etc...
		// for test only here....
		TileMaterial= new CTileMaterial;
		TileMaterial->Pass[0]= Patch->getTileRenderPass(TileId, 0);
		// Use NULL in TileMaterial->TilePass to know the format and bind PassToUv.
		sint	uvcount=0;
		for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		{
			if(TileMaterial->Pass[i])
			{
				TileMaterial->PassToUv[i]= uvcount;
				uvcount++;
			}
		}
		// TODO_BUMP: choose beetween bump and normal.
		TileMaterial->TileUvFmt= TileUvFmtNormal1+(uvcount-1);
	}


	// 2. Compute Uvs.
	//----------------
	// Must allocate the base uv.
	TileUvBase= allocTileUv(TileMaterial->TileUvFmt);
	// TODO_TEXTURE: work with Patch, to create the good texcoordinates.
	// for test only here....
	((CTileUvNormal1*)TileUvBase)->UvPasses[0].PUv0.U= PVBase.S<=middle.S? 0.0f: 1.0f;
	((CTileUvNormal1*)TileUvBase)->UvPasses[0].PUv0.V= PVBase.T<=middle.T? 0.0f: 1.0f;

	// if base neighbor is already at TileLimitLevel just ptr-copy, else create the left/right TileUvs...
	nlassert(!FBase || FBase->Level<=Patch->TileLimitLevel);
	if(FBase && FBase->Level==Patch->TileLimitLevel && FBase->TileUvLeft!=NULL)
	{
		// Just cross-copy the pointers.
		TileUvLeft= FBase->TileUvRight;
		TileUvRight= FBase->TileUvLeft;
	}
	else
	{
		TileUvLeft= allocTileUv(TileMaterial->TileUvFmt);
		TileUvRight= allocTileUv(TileMaterial->TileUvFmt);
		// TODO_TEXTURE: work with Patch, to create the good texcoordinates.
		// for test only here....
		((CTileUvNormal1*)TileUvLeft)->UvPasses[0].PUv0.U= PVLeft.S<=middle.S? 0.0f: 1.0f;
		((CTileUvNormal1*)TileUvLeft)->UvPasses[0].PUv0.V= PVLeft.T<=middle.T? 0.0f: 1.0f;
		((CTileUvNormal1*)TileUvRight)->UvPasses[0].PUv0.U= PVRight.S<=middle.S? 0.0f: 1.0f;
		((CTileUvNormal1*)TileUvRight)->UvPasses[0].PUv0.V= PVRight.T<=middle.T? 0.0f: 1.0f;
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
		// Do not release Uvs, since neighbor need it...
		TileUvLeft= NULL;
		TileUvRight= NULL;
		// idem for TileMaterial.
		TileMaterial= NULL;
	}
	else
	{
		delete TileUvLeft;
		delete TileUvRight;
		TileUvLeft= NULL;
		TileUvRight= NULL;
		delete TileMaterial;
		TileMaterial= NULL;
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
	// link neighbor face.
	SonLeft->FBase= FLeft;
	if(FLeft)	FLeft->changeNeighbor(this, SonLeft);
	SonLeft->FLeft= SonRight;
	SonLeft->FRight= FBase;		// Temporary. updated later.
	// link neighbor vertex.
	SonLeft->VLeft= VBase;
	SonLeft->VRight= VLeft;
	// Patch management.
	SonLeft->Patch= Patch;
	SonLeft->PVBase= CParamCoord(PVLeft, PVRight);
	SonLeft->PVLeft= PVBase;
	SonLeft->PVRight= PVLeft;

	// Right Son.
	// subdivision.
	SonRight->Father= this;
	SonRight->Level= Level+1;
	SonRight->Size= Size*0.5f;
	// link neighbor face.
	SonRight->FBase= FRight;
	if(FRight)	FRight->changeNeighbor(this, SonRight);
	SonRight->FLeft= FBase;		// Temporary. updated later.
	SonRight->FRight= SonLeft;
	// link neighbor vertex.
	SonRight->VLeft= VRight;
	SonRight->VRight= VBase;
	// Patch management.
	SonRight->Patch= Patch;
	SonRight->PVBase= CParamCoord(PVLeft, PVRight);
	SonRight->PVLeft= PVRight;
	SonRight->PVRight= PVBase;

	// FBase->FBase==this. Must Doesn't change this. Used and Updated in section 5. ...

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
		SonLeft->TileMaterial= TileMaterial;
		SonLeft->TileUvLeft= TileUvBase;
		SonLeft->TileUvRight= TileUvLeft;

		SonRight->TileId= TileId;
		SonRight->TileMaterial= TileMaterial;
		SonRight->TileUvLeft= TileUvRight;
		SonRight->TileUvRight= TileUvBase;

		// Create, or link to the tileUv.
		ITileUv		*tuv;
		// Try to link to a neighbor TileUv.
		// Can only work iff exist, and iff FBase is same patch, and same TileId.
		if(FBase!=NULL && !FBase->isLeaf() && sameTile(this, FBase) )
		{
			// Ok!! link to the (existing) TileUv.
			// SonLeft!=NULL since FBase->isLeaf()==false.
			tuv= FBase->SonLeft->TileUvBase;
		}
		else
		{
			switch (TileMaterial->TileUvFmt)
			{
				// Create at middle: (TileUvLeft+TileUvRight) /2
				case TileUvFmtNormal1: tuv= new CTileUvNormal1((CTileUvNormal1*)TileUvLeft, (CTileUvNormal1*)TileUvRight); break;
				case TileUvFmtNormal2: tuv= new CTileUvNormal2((CTileUvNormal2*)TileUvLeft, (CTileUvNormal2*)TileUvRight); break;
				case TileUvFmtNormal3: tuv= new CTileUvNormal3((CTileUvNormal3*)TileUvLeft, (CTileUvNormal3*)TileUvRight); break;
				case TileUvFmtNormal4: tuv= new CTileUvNormal4((CTileUvNormal4*)TileUvLeft, (CTileUvNormal4*)TileUvRight); break;
				case TileUvFmtNormal5: tuv= new CTileUvNormal5((CTileUvNormal5*)TileUvLeft, (CTileUvNormal5*)TileUvRight); break;
				case TileUvFmtNormal6: tuv= new CTileUvNormal6((CTileUvNormal6*)TileUvLeft, (CTileUvNormal6*)TileUvRight); break;
				case TileUvFmtBump1: tuv= new CTileUvBump1((CTileUvBump1*)TileUvLeft, (CTileUvBump1*)TileUvRight); break;
				case TileUvFmtBump2: tuv= new CTileUvBump2((CTileUvBump2*)TileUvLeft, (CTileUvBump2*)TileUvRight); break;
				case TileUvFmtBump3: tuv= new CTileUvBump3((CTileUvBump3*)TileUvLeft, (CTileUvBump3*)TileUvRight); break;
				case TileUvFmtBump4: tuv= new CTileUvBump4((CTileUvBump4*)TileUvLeft, (CTileUvBump4*)TileUvRight); break;
				case TileUvFmtBump5: tuv= new CTileUvBump5((CTileUvBump5*)TileUvLeft, (CTileUvBump5*)TileUvRight); break;
				case TileUvFmtBump6: tuv= new CTileUvBump6((CTileUvBump6*)TileUvLeft, (CTileUvBump6*)TileUvRight); break;
				default: nlstop;
			};
		}
		
		SonLeft->TileUvBase= tuv;
		SonRight->TileUvBase= tuv;
	}

	// 3. Update/Create Vertex infos.
	//-------------------------------

	// Must create/link *->VBase.
	if(FBase==NULL || FBase->isLeaf())
	{
		// The base neighbor is a leaf or NULL. So must create the new vertex.
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
		// Else, get from neighbor.
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
		// Just update sons neighbors.
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
	// Test bind/CantMerge config.
	if(face1 == &CantMergeFace)
		return false;
	if(face2 == &CantMergeFace)
		return false;
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
	
	// Change father 's neighbor pointers.
	face1->FLeft= face1->SonLeft->FBase;
	if(face1->FLeft)	face1->FLeft->changeNeighbor(face1->SonLeft, face1);
	face1->FRight= face1->SonRight->FBase;
	if(face1->FRight)	face1->FRight->changeNeighbor(face1->SonRight, face1);
	if(face2!=NULL)
	{
		face2->FLeft= face2->SonLeft->FBase;
		if(face2->FLeft)	face2->FLeft->changeNeighbor(face2->SonLeft, face2);
		face2->FRight= face2->SonRight->FBase;
		if(face2->FRight)	face2->FRight->changeNeighbor(face2->SonRight, face2);
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

	// In Tile/Far Transition zone, force the needCompute.
	if(Patch->Zone->ComputeTileErrorMetric && Level<Patch->TileLimitLevel)
	{
		NeedCompute= true;
	}


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
			// Merge only if neighbor agree.
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

			// get the neighbor, to have the max projectedsize.
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
		// If below a merge(), I may not need to compute.
		if(Father && isLeaf() && ps<ChildrenStartComputeLimit/2)	// 1.9/2
		{
			NeedCompute= false;
		}
		if(!isLeaf() && ps>ChildrenStartComputeLimit)	// 1.9
		{
			// The sons may split soon.
			SonLeft->NeedCompute= true;
			SonRight->NeedCompute= true;

		}
		// If grandfather (may arise in enforced splits problem..., especially near tile subdivision).
		if(!isLeaf() && (!SonLeft->isLeaf() || !SonRight->isLeaf()))
		{
			// The sons must test if they have to merge.
			SonLeft->NeedCompute= true;
			SonRight->NeedCompute= true;
		}
	}
	if(SonLeft)
	{
		SonLeft->refine();
		SonRight->refine();
	}


	// Unstable case: the child say "NeedCompute=false", but his father say "child->NeedCompute=true".
	// OK: At the next refine, the child will be processed again...
}


// ***************************************************************************
void		CTessFace::unbind()
{
	// NB: since CantMergeFace has a NULL patch ptr, it is unbound too.

	// Change Left/Right neighbors.
	if(FLeft && FLeft->Patch!=Patch)
	{
		FLeft->changeNeighbor(this, NULL);
		FLeft= NULL;
	}
	if(FRight && FRight->Patch!=Patch)
	{
		FRight->changeNeighbor(this, NULL);
		FRight= NULL;
	}
	if(FBase && FBase->Patch!=Patch)
	{
		FBase->changeNeighbor(this, NULL);
		FBase= NULL;
		if(!isLeaf())
		{
			// Duplicate the VBase of sons.
			CTessVertex	*old= SonLeft->VBase;
			SonLeft->VBase= new CTessVertex(*old);
			SonRight->VBase= SonLeft->VBase;
		}
	}

	if(!isLeaf())
	{
		// update vertex pointers (since I may have been updated by my father).
		SonLeft->VLeft= VBase;
		SonLeft->VRight= VLeft;
		SonRight->VLeft= VRight;
		SonRight->VRight= VBase;

		// unbind the sons.
		SonLeft->unbind();
		SonRight->unbind();
	}

}
// ***************************************************************************
void		CTessFace::forceMerge()
{
	if(!isLeaf())
	{
		// First, force merge of Sons and neighbor sons, to have a diamond configuration.
		SonLeft->forceMerge();
		SonRight->forceMerge();
		if(FBase && !FBase->isLeaf())
		{
			FBase->SonLeft->forceMerge();
			FBase->SonRight->forceMerge();
		}
		merge();
	}
}


// ***************************************************************************
bool		CTessFace::updateBindEdge(CTessFace	*&edgeFace, bool &splitWanted)
{
	// REturn true, when the bind should be Ok, or if a split has occured.
	// Return false only if pointers are updated, without splits.

	if(edgeFace==NULL)
		return true;

	if(edgeFace->isLeaf())
		return true;

	// If the neighbor is splitted  on ourself, split...
	if(edgeFace->FBase==this)
	{
		splitWanted= true;
		return true;
	}
	else
	{
		// Just update pointers...
		if(edgeFace->FLeft==this)
		{
			CTessFace	*sonLeft= edgeFace->SonLeft;
			sonLeft->FBase= this;
			edgeFace= sonLeft;
		}
		else if(edgeFace->FRight==this)
		{
			CTessFace	*sonRight= edgeFace->SonRight;
			sonRight->FBase= this;
			edgeFace= sonRight;
		}
		else
		{
			// We should aready be splitted.
			nlassert(!isLeaf());
			// The neigbor should link already to one of our son.
			nlassert(SonLeft == edgeFace->FLeft || SonRight == edgeFace->FLeft ||
				SonLeft == edgeFace->FRight || SonRight == edgeFace->FRight);
			return true;
		}
	}

	return false;
}


// ***************************************************************************
void		CTessFace::updateBind()
{
	bool	splitWanted= false;
	while(!updateBindEdge(FBase, splitWanted));
	while(!updateBindEdge(FLeft, splitWanted));
	while(!updateBindEdge(FRight, splitWanted));

	if(splitWanted && isLeaf())
	{
		split();
	}

	// Recurse to sons.
	if(!isLeaf())
	{
		// Force Face pointers (for enforced split face).
		SonLeft->FBase= FLeft;
		if(FLeft)	FLeft->changeNeighbor(this, SonLeft);
		SonRight->FBase= FRight;
		if(FRight)	FRight->changeNeighbor(this, SonRight);

		// Update bind of sons.
		SonLeft->updateBind();
		SonRight->updateBind();
	}
}


// ***************************************************************************
void		CTessFace::appendToRenderList(CTessFace *&root)
{
	if(isLeaf())
	{
		RenderNext= root;
		root= this;
	}
	else
	{
		SonLeft->appendToRenderList(root);
		SonRight->appendToRenderList(root);
	}
}


} // NL3D
