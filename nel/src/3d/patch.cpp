/** \file patch.cpp
 * <File description>
 *
 * $Id: patch.cpp,v 1.40 2001/02/01 16:32:30 berenguier Exp $
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


#include "nel/3d/patch.h"
#include "nel/3d/tessellation.h"
#include "nel/3d/bezier_patch.h"
#include "nel/3d/zone.h"
#include "nel/3d/landscape.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"
using	namespace	std;
using	namespace	NLMISC;


namespace NL3D 
{


// ***************************************************************************
CBezierPatch	CPatch::CachePatch;
const CPatch	*CPatch::LastPatch= NULL;
uint8			CPatch::_ShadingBuffer[(NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1)*(NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1)];
uint32			CPatch::_Version=2;

// ***************************************************************************
CPatch::CPatch()
{
	Zone= NULL;
	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	RdrRoot= NULL;
	for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		RdrTileRoot[i]=NULL;
	NCurrentFaces= 0;
	Clipped=false;

	// To force computation of texture info on next preRender().
	Far0= -1;
	Far1= -1;

	// Null the two passes
	Pass0=NULL;
	Pass1=NULL;
}
// ***************************************************************************
CPatch::~CPatch()
{
	release();
}

// ***************************************************************************
void			CPatch::release()
{
	if(Zone)
	{
		// THIS PATCH MSUT BE UNBOUND FIRST!!!!!
		nlassert(Son0 && Son1);
		nlassert(Son0->isLeaf() && Son1->isLeaf());
		// Can't test if OK, since bind 2/1 or 4/1 are not unbound.

		// Free renderPass of landscape, and maybe force computation of texture info on next preRender().
		// Must do it here, before deletion of Zone, OrderS/T etc...
		resetRenderFar();

		delete Son0;
		delete Son1;
		// Vertices are smartptr/deleted in zone.
		Zone= NULL;
	}

	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	RdrRoot= NULL;
	for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		RdrTileRoot[i]=NULL;
	NCurrentFaces= 0;
	Clipped=false;

}


// ***************************************************************************
CBezierPatch	*CPatch::unpackIntoCache() const
{
	if(LastPatch!=this)
	{
		unpack(CachePatch);
		LastPatch=this;
	}
	return &CachePatch;
}
// ***************************************************************************
void			CPatch::unpack(CBezierPatch	&p) const
{
	sint	i;
	const	CVector	&bias= Zone->getPatchBias();
	float	scale= Zone->getPatchScale();

	for(i=0;i<4;i++)
		Vertices[i].unpack(p.Vertices[i], bias, scale);
	for(i=0;i<8;i++)
		Tangents[i].unpack(p.Tangents[i], bias, scale);
	for(i=0;i<4;i++)
		Interiors[i].unpack(p.Interiors[i], bias, scale);
}
// ***************************************************************************
void			CPatch::computeDefaultErrorSize()
{
	CBezierPatch	&p= *unpackIntoCache();
	CVector			&v0= p.Vertices[0];
	CVector			&v1= p.Vertices[1];
	CVector			&v2= p.Vertices[2];

	// TODO_NOISE: modulate this value with tangents (roundiness of patch), and with the displacement map.
	ErrorSize= ((v1 - v0)^(v2 - v0)).norm();

}


// ***************************************************************************
CAABBox			CPatch::buildBBox() const
{
	sint			i;
	CBezierPatch	&p= *unpackIntoCache();
	CVector			&v0= p.Vertices[0];
	CVector			&v1= p.Vertices[1];
	CVector			&v2= p.Vertices[2];

	// Compute Bounding Box. (easiest way...)
	CAABBox		ret;
	ret.setCenter(v0);
	for(i=0;i<4;i++)
		ret.extend(p.Vertices[i]);
	for(i=0;i<8;i++)
		ret.extend(p.Tangents[i]);
	for(i=0;i<4;i++)
		ret.extend(p.Interiors[i]);
	// TODO_NOISE: modulate with the displacement map.

	return ret;
}


// ***************************************************************************
void			CPatch::appendFaceToRenderList(CTessFace *face)
{
	NCurrentFaces++;

	// Update Gnal render.
	//====================
	// update Next.
	face->RenderNext= RdrRoot;
	if(RdrRoot)
		RdrRoot->RenderPrec= face;
	// update Prec.
	face->RenderPrec= NULL;
	RdrRoot= face;

	// Update Tile render.
	//====================
	appendFaceToTileRdrList(face);
}


// ***************************************************************************
void			CPatch::appendFaceToTileRdrList(CTessFace *face)
{
	// Update Tile render.
	//====================
	if(face->TileRdrPtr)
	{
		// For all valid pass, update their links.
		for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		{
			CPatchRdrPass	*tilePass= face->TileMaterial->Pass[i];
			// If tile i enabled.
			if(tilePass)
			{
				// Yes, it is hard to understand.... :)
				// update Next.
				face->TileRdrPtr->RenderNext[i]= RdrTileRoot[i];
				if(RdrTileRoot[i])
					RdrTileRoot[i]->TileRdrPtr->RenderPrec[i]= face;
				// update Prec.
				face->TileRdrPtr->RenderPrec[i]= NULL;
				RdrTileRoot[i]= face;
			}
		}
	}
}

// ***************************************************************************
void			CPatch::removeFaceFromRenderList(CTessFace *face)
{
	NCurrentFaces--;

	// Update Gnal render.
	//====================
	// update Prec.
	if(!face->RenderPrec)
	{
		nlassert(RdrRoot==face);
		RdrRoot= face->RenderNext;
	}
	else
	{
		face->RenderPrec->RenderNext= face->RenderNext;
	}
	// update Next.
	if(face->RenderNext)
		face->RenderNext->RenderPrec= face->RenderPrec;
	face->RenderNext= NULL;
	face->RenderPrec= NULL;


	// Update Tile render.
	//====================
	if(face->TileMaterial)
	{
		for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		{
			CPatchRdrPass	*tilePass= face->TileMaterial->Pass[i];
			// If tile pass enabled.
			if(tilePass)
			{
				// Yes, it is hard to understand.... :)
				// update Prec.
				if(!face->TileRdrPtr->RenderPrec[i])
				{
					nlassert(RdrTileRoot[i]==face);
					RdrTileRoot[i]= face->TileRdrPtr->RenderNext[i];
				}
				else
				{
					face->TileRdrPtr->RenderPrec[i]->TileRdrPtr->RenderNext[i]= face->TileRdrPtr->RenderNext[i];
				}
				// update Next.
				if(face->TileRdrPtr->RenderNext[i])
					face->TileRdrPtr->RenderNext[i]->TileRdrPtr->RenderPrec[i]= face->TileRdrPtr->RenderPrec[i];
				face->TileRdrPtr->RenderNext[i]= NULL;
				face->TileRdrPtr->RenderPrec[i]= NULL;
			}
		}
	}
}


// ***************************************************************************
void			CPatch::makeRoots()
{
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];

	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);
	

	// Make Roots.
	/*
		Tesselation layout. For Square Face, and if OrderS>=OrderT.

		A-------D
		|\ Son1 |
		|  \    |
		|    \  |
		| Son0 \|
		B-------C

		For rectangles whith OrderT>OrderS. It is VERY IMPORTANT, for splitRectangular() reasons.

		A-------D
		| Son0 /|
		|    /  |
		|  /    |
		|/ Son1 |
		B-------C

	*/
	nlassert(Son0==NULL);
	nlassert(Son1==NULL);
	Son0= new CTessFace;
	Son1= new CTessFace;

	// Son0.
	Son0->Patch= this;
	Son0->Level= 0;
	// Roots always need to be computed, unless if their sons say "NO!"
	Son0->NeedCompute= true;
	if(OrderS>=OrderT)
	{
		Son0->VBase= b;
		Son0->VLeft= c;
		Son0->VRight= a;
		Son0->PVBase.setST(0, 1);
		Son0->PVLeft.setST(1, 1);
		Son0->PVRight.setST(0, 0);
	}
	else
	{
		Son0->VBase= a;
		Son0->VLeft= b;
		Son0->VRight= d;
		Son0->PVBase.setST(0, 0);
		Son0->PVLeft.setST(0, 1);
		Son0->PVRight.setST(1, 0);
	}
	Son0->FBase= Son1;
	Son0->FLeft= NULL;
	Son0->FRight= NULL;
	// No tile info.
	Son0->Size= ErrorSize/2;
	Son0->Center= (Son0->VBase->EndPos + Son0->VLeft->EndPos + Son0->VRight->EndPos)/3;

	// Son1.
	Son1->Patch= this;
	Son1->Level= 0;
	// Roots always need to be computed, unless if their sons say "NO!"
	Son1->NeedCompute= true;
	if(OrderS>=OrderT)
	{
		Son1->VBase= d;
		Son1->VLeft= a;
		Son1->VRight= c;
		Son1->PVBase.setST(1, 0);
		Son1->PVLeft.setST(0, 0);
		Son1->PVRight.setST(1, 1);
	}
	else
	{
		Son1->VBase= c;
		Son1->VLeft= d;
		Son1->VRight= b;
		Son1->PVBase.setST(1, 1);
		Son1->PVLeft.setST(1, 0);
		Son1->PVRight.setST(0, 1);
	}
	Son1->FBase= Son0;
	Son1->FLeft= NULL;
	Son1->FRight= NULL;
	// No tile info.
	Son1->Size= ErrorSize/2;
	Son1->Center= (Son1->VBase->EndPos + Son1->VLeft->EndPos + Son1->VRight->EndPos)/3;


	// Prepare the render list...
	RdrRoot= NULL;
	for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		RdrTileRoot[i]=NULL;
	NCurrentFaces= 0;
	appendFaceToRenderList(Son0);
	appendFaceToRenderList(Son1);

}


// ***************************************************************************
void			CPatch::compile(CZone *z, uint8 orderS, uint8 orderT, CTessVertex *baseVertices[4], float errorSize)
{
	nlassert(z);
	Zone= z;

	if(errorSize==0)
		computeDefaultErrorSize();
	else
		ErrorSize= errorSize;

	nlassert(orderS==2 || orderS==4 || orderS==8 || orderS==16);
	nlassert(orderT==2 || orderT==4 || orderT==8 || orderT==16);
	nlassert (OrderS==orderS);
	nlassert (OrderT==orderT);

	// Compile additional infos.
	sint	ps= getPowerOf2(orderS) , pt= getPowerOf2(orderT);
	sint	pmin= min(ps,pt);
	sint	pmax= max(ps,pt);
	// Rectangular patch OK.
	// Work, since patch 1xX are illegal. => The TileLimitLevel is at least 2 level distant from the time where
	// the rectangular patch is said "un-rectangular-ed" (tesselation looks like square). Hence, there is no problem
	// with rectangular UV geomorph (well don't bother me, make a draw :) ).
	TileLimitLevel= pmin*2 + pmax-pmin;
	// This tell us when the tess face is "un-rectangular-ed" (to say a square). Before, it is a "rectangular" face, 
	// which has a strange fxxxxxg split.
	// If patch is square, then SquareLimitLevel=0 (ok!!).
	SquareLimitLevel= pmax-pmin;

	// Buil the BSPhere.
	CAABBox	bb= buildBBox();
	BSphere.Center= bb.getCenter();
	BSphere.Radius= bb.getRadius();

	// Bind vertices, to zone base vertices.
	BaseVertices[0]= baseVertices[0];
	BaseVertices[1]= baseVertices[1];
	BaseVertices[2]= baseVertices[2];
	BaseVertices[3]= baseVertices[3];

	// build Sons.
	makeRoots();
}
// ***************************************************************************
CVector			CPatch::computeVertex(float s, float t) const
{
	// First, unpack...
	CBezierPatch	*patch= unpackIntoCache();

	// TODO_NOISE/TODO_UVCORRECT: use UV correction, and use displacement map, to disturb result.
	return patch->eval(s,t);
}
// ***************************************************************************
void			CPatch::refine()
{
	if(Zone->ComputeTileErrorMetric)
	{
		// Must test more precisely...
		if(BSphere.intersect(CTessFace::TileFarSphere))
			ComputeTileErrorMetric= true;
		else
			ComputeTileErrorMetric= false;
		// If true, must test if we are in transition, or totaly IN.
		if(ComputeTileErrorMetric)
		{
			// Do the zone include ALL the patch???
			if(CTessFace::TileNearSphere.include(BSphere))
				TileFarTransition= false;
			else
				TileFarTransition= true;
		}
	}
	else
		ComputeTileErrorMetric= false;
	nlassert(Son0);
	nlassert(Son1);
	Son0->refine();
	Son1->refine();
}


// ***************************************************************************
void			CPatch::clip(const std::vector<CPlane>	&pyramid)
{
	Clipped= false;
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// If entirely out.
		if(!BSphere.clipBack(pyramid[i]))
		{
			Clipped= true;
			break;
		}
	}

}


// ***************************************************************************
void			CPatch::resetFarIndices(CTessFace *pFace)
{
	for(;pFace; pFace=pFace->RenderNext)
	{
		pFace->VBase->FarIndex= 0;
		pFace->VLeft->FarIndex= 0;
		pFace->VRight->FarIndex= 0;
	}
}


// ***************************************************************************
void			CPatch::resetRenderFar()
{
	if (Pass0)
		Zone->Landscape->freeFarRenderPass (this, Pass0, Far0);
	if (Pass1)
		Zone->Landscape->freeFarRenderPass (this, Pass1, Far1);

	Pass0= NULL;
	Pass1= NULL;
	Far0= -1;
	Far1= -1;
}


// ***************************************************************************
void			CPatch::preRender()
{
	// Don't do anything if clipped.
	if(Clipped)
		return;

	// Classify the patch.
	//====================
	sint	newFar0,newFar1;
	float	r= (CTessFace::RefineCenter-BSphere.Center).norm() - BSphere.Radius;
	float	rr;
	if(r<CTessFace::TileDistNear)
		rr= r-CTessFace::TileDistNear, newFar0= 0;
	else if(r<CTessFace::Far0Dist)
		rr= r-CTessFace::Far0Dist, newFar0= 1;
	else if(r<CTessFace::Far1Dist)
		rr= r-CTessFace::Far1Dist, newFar0= 2;
	else
		newFar0= 3;
	// Transition with the next level.
	newFar1=0;
	if(newFar0<3 && rr>-(CTessFace::FarTransition+2*BSphere.Radius))
	{
		newFar1= newFar0+1;
	}


	// Update Texture Info.
	//=====================
	if(newFar0!=Far0 || newFar1!=Far1)
	{
		// Backup old pass0
		CPatchRdrPass	*oldPass0=Pass0;
		CPatchRdrPass	*oldPass1=Pass1;
		float oldFar0UScale=Far0UScale;
		float oldFar0VScale=Far0VScale;
		float oldFar0UBias=Far0UBias;
		float oldFar0VBias=Far0VBias;
		uint8 oldFlags=FarRotated;

		// Don't delete the pass0 if the new newFar1 will use it
		if ((newFar1==Far0)&&(Far0>0))
			Pass0=NULL;

		// Don't delete the pass1 if the new newFar0 will use it
		if ((newFar0==Far1)&&(Far1>0))
			Pass1=NULL;

		// Pass0 have changed ?
		if (newFar0!=Far0)
		{
			// Compute / get the texture Far.
			if(newFar0>0)
			{
				// Free the old pass, don't used any more
				if (Pass0)
					Zone->Landscape->freeFarRenderPass (this, Pass0, Far0);

				// Can we use the old pass1 ?
				if (newFar0==Far1)
				{
					// Yes, recycle it!
					Pass0=oldPass1;

					// Copy uv coordinates
					Far0UScale=Far1UScale;
					Far0VScale=Far1VScale;
					Far0UBias=Far1UBias;
					Far0VBias=Far1VBias;

					// Copy rotation flag
					FarRotated&=~NL_PATCH_FAR0_ROTATED;			// erase it
					if (FarRotated&NL_PATCH_FAR1_ROTATED)
						FarRotated|=NL_PATCH_FAR0_ROTATED;			// copy it
				}
				else	// get a new render pass
				{
					// Rotation boolean
					bool bRot;
					Pass0=Zone->Landscape->getFarRenderPass(this, newFar0, Far0UScale, Far0VScale, Far0UBias, Far0VBias, bRot);

					// Flags is set if the far texture is rotated of 90° to the left
					if (bRot)
						FarRotated|=NL_PATCH_FAR0_ROTATED;
					else
						FarRotated&=~NL_PATCH_FAR0_ROTATED;
				}
			}
			else	// no more far pass0
			{
				if (Pass0)
				{
					Zone->Landscape->freeFarRenderPass (this, Pass0, Far0);
					Pass0=NULL;
				}
			}
		}

		// Pass1 have changed ?
		if (newFar1!=Far1)
		{
			// Now let's go with pass1
			if(newFar1>0)
			{
				// Delete the pass1 if not used any more
				if (Pass1)
					Zone->Landscape->freeFarRenderPass (this, Pass1, Far1);

				// Can we use the old pass1 ?
				if (newFar1==Far0)
				{
					// Yes, recycle it!
					Pass1=oldPass0;

					// Copy uv coordinates
					Far1UScale=oldFar0UScale;
					Far1VScale=oldFar0VScale;
					Far1UBias=oldFar0UBias;
					Far1VBias=oldFar0VBias;

					// Copy rotation flag
					FarRotated&=~NL_PATCH_FAR1_ROTATED;			// erase it
					if (oldFlags&NL_PATCH_FAR0_ROTATED)
						FarRotated|=NL_PATCH_FAR1_ROTATED;			// copy it
				}
				else	// get a new render pass
				{
					// Rotation boolean
					bool bRot;
					Pass1=Zone->Landscape->getFarRenderPass(this, newFar1, Far1UScale, Far1VScale, Far1UBias, Far1VBias, bRot);

					// Flags is set if the far texture is rotated of 90° to the left
					if (bRot)
						FarRotated|=NL_PATCH_FAR1_ROTATED;
					else
						FarRotated&=~NL_PATCH_FAR1_ROTATED;
				}

				// Compute info for transition.
				float	farDist;
				switch(newFar1)
				{
					case 1: farDist= CTessFace::TileDistNear; break;
					case 2: farDist= CTessFace::Far0Dist; break;
					case 3: farDist= CTessFace::Far1Dist; break;
					default: nlstop;
				};
				TransitionSqrMin= sqr(farDist-CTessFace::FarTransition);
				OOTransitionSqrDelta= 1.0f/(sqr(farDist)-TransitionSqrMin);
			}
			else	// no more far pass1
			{
				if (Pass1)
				{
					Zone->Landscape->freeFarRenderPass (this, Pass1, Far1);
					Pass1=NULL;
				}
			}
		}
	}
	// Set new far values
	Far0= newFar0;
	Far1= newFar1;
}


// ***************************************************************************
sint			CPatch::getFarIndex0(CTessVertex *vert, CTessFace::CParamCoord  pc)
{
	if(vert->FarIndex==0)
	{
		// Compute/build the new vertex.
		vert->FarIndex= CTessFace::CurrentVertexIndex++;

		// Set Pos.
		CTessFace::CurrentVB->setVertexCoord(vert->FarIndex, vert->Pos);

		// Set Uvs.
		static CUV		uv;
		if (FarRotated&NL_PATCH_FAR0_ROTATED)
		{
			uv.U= pc.getT()* Far0UScale + Far0UBias;
			uv.V= (1.f-pc.getS())* Far0VScale + Far0VBias;
		}
		else
		{
			uv.U= pc.getS()* Far0UScale + Far0UBias;
			uv.V= pc.getT()* Far0VScale + Far0VBias;
		}
		CTessFace::CurrentVB->setTexCoord(vert->FarIndex, 0, uv);

		// Compute color.
		static CRGBA	col(255,255,255,255);
		// TODO_CLOUD: use normal information, for cloud attenuation, in RGB fields.
		// For Far0, alpha is un-usefull.
		CTessFace::CurrentVB->setColor(vert->FarIndex, col);
		// END!!
	}
	
	return vert->FarIndex;
}
// ***************************************************************************
sint			CPatch::getFarIndex1(CTessVertex *vert, CTessFace::CParamCoord  pc)
{
	if(vert->FarIndex==0)
	{
		// Compute/build the new vertex.
		vert->FarIndex= CTessFace::CurrentVertexIndex++;

		// Set Pos.
		CTessFace::CurrentVB->setVertexCoord(vert->FarIndex, vert->Pos);

		// Set Uvs.
		static CUV		uv;
		if (FarRotated&NL_PATCH_FAR1_ROTATED)
		{
			uv.U= pc.getT()* Far1UScale + Far1UBias;
			uv.V= (1.f-pc.getS())* Far1VScale + Far1VBias;
		}
		else
		{
			uv.U= pc.getS()* Far1UScale + Far1UBias;
			uv.V= pc.getT()* Far1VScale + Far1VBias;
		}
		CTessFace::CurrentVB->setTexCoord(vert->FarIndex, 0, uv);

		// Compute color.
		static CRGBA	col(255,255,255,255);
		// TODO_CLOUD: use normal information, for cloud attenuation, in RGB fields.
		// For Far1, use alpha fro transition.
		float	f= (vert->Pos - CTessFace::RefineCenter).sqrnorm();
		f= (f-TransitionSqrMin) * OOTransitionSqrDelta;

		clamp(f,0,1);
		col.A= (uint8)(f*255);
		CTessFace::CurrentVB->setColor(vert->FarIndex, col);
		// END!!
	}
	
	return vert->FarIndex;
}


// ***************************************************************************
// Local for computeTileVertex only. Setuped in CPatch::renderTiles().
static	uint8	*CurVBPtr;
static	sint	CurVertexSize;
static	sint	CurUV0Off;
static	sint	CurUV1Off;
void			CPatch::computeTileVertex(CTessVertex *vert, ITileUv *uv, sint idUv)
{
	// Compute/build the new vertex.
	uv->TileDate=CTessFace::CurrentRenderDate;
	uv->TileIndex= CTessFace::CurrentVertexIndex++;

	// Set Pos.
	//CTessFace::CurrentVB->setVertexCoord(uv->TileIndex, vert->Pos);
	*(CVector*)CurVBPtr= vert->Pos;

	// Set Uvs.
	ITileUvNormal	*uvn= (ITileUvNormal*)uv;
	CPassUvNormal	&uvpass= uvn->UvPasses[idUv];
	//CTessFace::CurrentVB->setTexCoord(uv->TileIndex, 0, uvpass.PUv0);
	//CTessFace::CurrentVB->setTexCoord(uv->TileIndex, 1, uvpass.PUv1);
	*(CUV*)(CurVBPtr+CurUV0Off)= uvpass.PUv0;
	*(CUV*)(CurVBPtr+CurUV1Off)= uvpass.PUv1;

	// Inc the ptr.
	CurVBPtr+= CurVertexSize;
}


// ***************************************************************************
void			CPatch::renderFar0()
{
	if(Pass0 && !Clipped)
	{
		resetFarIndices(RdrRoot);
		// Realloc if necessary the VertexBuffer.
		if((sint)CTessFace::CurrentVB->capacity() < CTessFace::CurrentVertexIndex+3*NCurrentFaces)
			CTessFace::CurrentVB->reserve(CTessFace::CurrentVertexIndex+3*NCurrentFaces);
		// Add tris.
		CTessFace	*pFace= RdrRoot;
		for(;pFace; pFace=pFace->RenderNext)
		{
			Pass0->addTri(getFarIndex0(pFace->VBase, pFace->PVBase),
						getFarIndex0(pFace->VLeft, pFace->PVLeft),
						getFarIndex0(pFace->VRight, pFace->PVRight));
		}

		// Check the pass is in the set
#ifdef NL_DEBUG
		if (Pass0)
		{
			nlassert (Zone->Landscape->_FarRdrPassSet.find (Pass0)!=Zone->Landscape->_FarRdrPassSet.end());
			if (Zone->Landscape->_FarRdrPassSet.find (Pass0)==Zone->Landscape->_FarRdrPassSet.end())
			{
				bool bFound=false;
				{
					for (sint t=0; t<(sint)Zone->Landscape->_FarRdrPassSetVectorFree.size(); t++)
					{
						if (Zone->Landscape->_FarRdrPassSetVectorFree[t].find (Pass0)!=Zone->Landscape->_FarRdrPassSetVectorFree[t].end())
						{
							bFound=true;
							break;
						}
					}
				}
				nlassert (bFound);
			}
		}
#endif // NL_DEBUG
	}
}


// ***************************************************************************
void			CPatch::renderFar1()
{
	if(Pass1 && !Clipped)
	{
		resetFarIndices(RdrRoot);
		// Realloc if necessary the VertexBuffer.
		if((sint)CTessFace::CurrentVB->capacity() < CTessFace::CurrentVertexIndex+3*NCurrentFaces)
			CTessFace::CurrentVB->reserve(CTessFace::CurrentVertexIndex+3*NCurrentFaces);
		// Add tris.
		CTessFace	*pFace= RdrRoot;
		for(;pFace; pFace=pFace->RenderNext)
		{
			Pass1->addTri(getFarIndex1(pFace->VBase, pFace->PVBase),
						getFarIndex1(pFace->VLeft, pFace->PVLeft),
						getFarIndex1(pFace->VRight, pFace->PVRight));
		}

		// Check the pass is in the set
#ifdef NL_DEBUG
		if (Pass1)
		{
			nlassert (Zone->Landscape->_FarRdrPassSet.find (Pass1)!=Zone->Landscape->_FarRdrPassSet.end());
			if (Zone->Landscape->_FarRdrPassSet.find (Pass1)==Zone->Landscape->_FarRdrPassSet.end())
			{
				bool bFound=false;
				{
					for (sint t=0; t<(sint)Zone->Landscape->_FarRdrPassSetVectorFree.size(); t++)
					{
						if (Zone->Landscape->_FarRdrPassSetVectorFree[t].find (Pass1)!=Zone->Landscape->_FarRdrPassSetVectorFree[t].end())
						{
							bFound=true;
							break;
						}
					}
				}
				nlassert (bFound);
			}
		}
#endif // NL_DEBUG
	}
}
// ***************************************************************************
void			CPatch::renderTile(sint pass)
{
	// If tile mode.
	if(Far0==0 && !Clipped)
	{
		// Realloc if necessary the VertexBuffer (at max possible).
		if((sint)CTessFace::CurrentVB->capacity() < CTessFace::CurrentVertexIndex+3*NCurrentFaces)
			CTessFace::CurrentVB->reserve(CTessFace::CurrentVertexIndex+3*NCurrentFaces);

		// Setup local VB infos, for getTileIndex().
		CurVBPtr= (uint8*)(CTessFace::CurrentVB->getVertexCoordPointer(CTessFace::CurrentVertexIndex));
		CurVertexSize= CTessFace::CurrentVB->getVertexSize();
		CurUV0Off= CTessFace::CurrentVB->getTexCoordOff(0);
		CurUV1Off= CTessFace::CurrentVB->getTexCoordOff(1);

		// Add tris.
		CTessFace	*pFace= RdrTileRoot[pass];
		for(;pFace; pFace=pFace->TileRdrPtr->RenderNext[pass])
		{
			CPatchRdrPass	*tilePass= pFace->TileMaterial->Pass[pass];
			nlassert(tilePass);
			sint	idUv=pFace->TileMaterial->PassToUv[pass];

			// The 3 vertices.
			ITileUv	*tileUvBase=pFace->TileUvBase;
			ITileUv	*tileUvLeft=pFace->TileUvLeft;
			ITileUv	*tileUvRight=pFace->TileUvRight;

			// If not computed, compute them.
			if(tileUvBase->TileDate!=CTessFace::CurrentRenderDate)
				computeTileVertex(pFace->VBase, tileUvBase, idUv);
			if(tileUvLeft->TileDate!=CTessFace::CurrentRenderDate)
				computeTileVertex(pFace->VLeft, tileUvLeft, idUv);
			if(tileUvRight->TileDate!=CTessFace::CurrentRenderDate)
				computeTileVertex(pFace->VRight, tileUvRight, idUv);

			// addTri to render pass.
			tilePass->addTri(tileUvBase->TileIndex, tileUvLeft->TileIndex, tileUvRight->TileIndex);
		}
	}
}


// ***************************************************************************
void			CPatch::unbind(CPatch *except[4])
{
	nlassert(Son0 && Son1);

	Son0->unbind(except);
	Son1->unbind(except);
	Son0->forceMerge();
	Son1->forceMerge();
	// Even if I am still binded to a "bigger" neigbhor patch, forcemerge should have be completed.
	// It is because only bind 1/2 and 1/4 could be done... (make a draw to understand).
	nlassert(Son0->isLeaf() && Son1->isLeaf());
}


// ***************************************************************************
CTessFace		*CPatch::getRootFaceForEdge(sint edge) const
{
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(OrderS>=OrderT)
	{
		if(edge==0 || edge==1)
			return Son0;
		else
			return Son1;
	}
	else
	{
		if(edge==0 || edge==3)
			return Son0;
		else
			return Son1;
	}
}

// ***************************************************************************
CTessVertex		*CPatch::getRootVertexForEdge(sint edge) const
{
	// Return the vertex which is the start of edge.
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(OrderS>=OrderT)
	{
		switch(edge)
		{
			case 0: return Son0->VRight;
			case 1: return Son0->VBase;
			case 2: return Son0->VLeft;
			case 3: return Son1->VBase;
			default: return NULL;
		}
	}
	else
	{
		switch(edge)
		{
			case 0: return Son0->VBase;
			case 1: return Son0->VLeft;
			case 2: return Son1->VBase;
			case 3: return Son0->VRight;
			default: return NULL;
		}
	}
}


// ***************************************************************************
void			CPatch::changeEdgeNeighbor(sint edge, CTessFace *to)
{
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(OrderS>=OrderT)
	{
		switch(edge)
		{
			case 0: Son0->FRight= to; break;
			case 1: Son0->FLeft= to; break;
			case 2: Son1->FRight= to; break;
			case 3: Son1->FLeft= to; break;
		}
	}
	else
	{
		switch(edge)
		{
			case 0: Son0->FLeft= to; break;
			case 1: Son1->FRight= to; break;
			case 2: Son1->FLeft= to; break;
			case 3: Son0->FRight= to; break;
		}
	}
}


// ***************************************************************************
void			CPatch::bind(CBindInfo	Edges[4])
{
	// The multiple Patch Face.
	// By default, Patch==NULL, FLeft, FRight and FBase==NULL so ok!
	static	CTessFace	bind1_2[4];
	static	CTessFace	bind1_4[8];


	// THIS PATCH MUST BE UNBOUND FIRST!!!!!
	nlassert(Son0 && Son1);
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	// Can't test if OK, since bind 2/1 or 4/1 are not unbound.

	// Just recompute base vertices.
	// TODO_NOISE. bind the noise info before.
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];
	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);
	// NB: no propagation problem, since the patch has root only (since has to be unbound!!!)
	// Recompute centers.
	Son0->Center= (Son0->VBase->EndPos + Son0->VLeft->EndPos + Son0->VRight->EndPos)/3;
	Son1->Center= (Son1->VBase->EndPos + Son1->VLeft->EndPos + Son1->VRight->EndPos)/3;


	// Bind the roots.
	for(sint i=0;i<4;i++)
	{
		CBindInfo	&bind= Edges[i];

		nlassert(bind.NPatchs==0 || bind.NPatchs==1 || bind.NPatchs==2 || bind.NPatchs==4);
		if(bind.NPatchs==1)
		{
			// Bind me on Next.
			this->changeEdgeNeighbor(i, bind.Next[0]->getRootFaceForEdge(bind.Edge[0]));
			// Bind Next on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], this->getRootFaceForEdge(i));
		}
		else if(bind.NPatchs==2)
		{
			// Setup multiple bind.
			this->changeEdgeNeighbor(i, bind1_2+i);
			bind1_2[i].FBase= this->getRootFaceForEdge(i);
			// Setup the multiple face.
			// Follow the conventions! Make a draw for understand. Small Patchs are numbered in CCW.
			bind1_2[i].SonRight= bind.Next[0]->getRootFaceForEdge(bind.Edge[0]);
			bind1_2[i].SonLeft= bind.Next[1]->getRootFaceForEdge(bind.Edge[1]);
			bind1_2[i].VBase= bind.Next[0]->getRootVertexForEdge(bind.Edge[0]);
			// Set a "flag" to neighbors, so they know what edge is to be bind on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], &CTessFace::MultipleBindFace);
			bind.Next[1]->changeEdgeNeighbor(bind.Edge[1], &CTessFace::MultipleBindFace);
		}
		else if(bind.NPatchs==4)
		{
			// Setup multiple bind level 0.
			this->changeEdgeNeighbor(i, bind1_2+i);
			bind1_2[i].FBase= this->getRootFaceForEdge(i);

			// Setup multiple bind level 1.
			// Follow the conventions! Make a draw for understand. Small Patchs are numbered in CCW.
			bind1_2[i].SonRight= bind1_4 + 2*i+0;
			bind1_2[i].SonLeft= bind1_4 + 2*i+1;
			bind1_2[i].VBase= bind.Next[1]->getRootVertexForEdge(bind.Edge[1]);
			// Make first multiple face bind level1.
			bind1_4[2*i+0].FBase= &CTessFace::MultipleBindFace;	// to link correctly when the root face will be splitted.
			bind1_4[2*i+0].SonRight= bind.Next[0]->getRootFaceForEdge(bind.Edge[0]);
			bind1_4[2*i+0].SonLeft= bind.Next[1]->getRootFaceForEdge(bind.Edge[1]);
			bind1_4[2*i+0].VBase= bind.Next[0]->getRootVertexForEdge(bind.Edge[0]);
			// Make second multiple face bind level1.
			bind1_4[2*i+1].FBase= &CTessFace::MultipleBindFace;	// to link correctly when the root face will be splitted.
			bind1_4[2*i+1].SonRight= bind.Next[2]->getRootFaceForEdge(bind.Edge[2]);
			bind1_4[2*i+1].SonLeft= bind.Next[3]->getRootFaceForEdge(bind.Edge[3]);
			bind1_4[2*i+1].VBase= bind.Next[2]->getRootVertexForEdge(bind.Edge[2]);

			// Set a "flag" to neighbors, so they know what edge is to be bind on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], &CTessFace::MultipleBindFace);
			bind.Next[1]->changeEdgeNeighbor(bind.Edge[1], &CTessFace::MultipleBindFace);
			bind.Next[2]->changeEdgeNeighbor(bind.Edge[2], &CTessFace::MultipleBindFace);
			bind.Next[3]->changeEdgeNeighbor(bind.Edge[3], &CTessFace::MultipleBindFace);
		}
	}

	// Propagate the binds to sons.
	Son0->updateBind();
	Son1->updateBind();

}

// ***************************************************************************
void			CPatch::serial(NLMISC::IStream &f)
{
	/*
	Version 2:
		- Lumels.
	Version 1:
		- Tile color.
	Version 0:
		- base verison.
	*/
	uint	ver= f.serialVersion(_Version);

	f.serial(Vertices[0], Vertices[1], Vertices[2], Vertices[3]);
	f.serial(Tangents[0], Tangents[1], Tangents[2], Tangents[3]);
	f.serial(Tangents[4], Tangents[5], Tangents[6], Tangents[7]);
	f.serial(Interiors[0], Interiors[1], Interiors[2], Interiors[3]);
	f.serialCont(Tiles);
	if(ver>=1)
		f.serialCont(TileColors);
	if(ver>=2)
	{
		f.serial (OrderS);
		f.serial (OrderT);
		f.serialCont(CompressedLumels);
	}
	// Else cannot create here the TileColors, because we need the OrderS/OrderT information... Done into CZone serial.
}


// ***************************************************************************
CPatchRdrPass	*CPatch::getTileRenderPass(sint tileId, sint pass)
{
	// All but lightmap.
	nlassert(pass==NL3D_TILE_PASS_RGB0 || pass==NL3D_TILE_PASS_RGB1 || pass==NL3D_TILE_PASS_RGB2 || 
		pass==NL3D_TILE_PASS_ADD);

	bool	additive= (pass==NL3D_TILE_PASS_ADD);
	sint	passNum= pass-NL3D_TILE_PASS_RGB0;
	// If additive, take the additve tile of pass0.
	if(additive)
		passNum= 0;

	sint	tileNumber= Tiles[tileId].Tile[passNum];
	if(tileNumber==0xFFFF)
	{
		// Display a "fake" only if pass 0.
		if(pass==NL3D_TILE_PASS_RGB0)
			return Zone->Landscape->getTileRenderPass(0xFFFF, false);
		// Else, this tile do not have such a pass (not a transition).
		return NULL;
	}
	else
	{
		// return still may be NULL, in additive case.
		return Zone->Landscape->getTileRenderPass(tileNumber, additive);
	}
}

// ***************************************************************************
void			CPatch::getTileUvInfo(sint tileId, sint pass, bool alpha, uint8 &orient, CVector &uvScaleBias, bool &is256x256, uint8 &uvOff)
{
	// All but lightmap.
	nlassert(pass==NL3D_TILE_PASS_RGB0 || pass==NL3D_TILE_PASS_RGB1 || pass==NL3D_TILE_PASS_RGB2 || 
		pass==NL3D_TILE_PASS_ADD);

	bool	additive= (pass==NL3D_TILE_PASS_ADD);
	sint	passNum= pass-NL3D_TILE_PASS_RGB0;
	// If additive, take the additve tile of pass0.
	if(additive)
		passNum= 0;

	sint	tileNumber= Tiles[tileId].Tile[passNum];
	if(tileNumber==0xFFFF)
	{
		// dummy... Should not be called here.
		orient= 0;
		uvScaleBias.x=0;
		uvScaleBias.y=0;
		uvScaleBias.z=1;
		is256x256=false;
		uvOff=0;
	}
	else
	{
		orient= Tiles[tileId].getTileOrient(passNum);
		Tiles[tileId].getTile256Info(is256x256, uvOff);
		CTile::TBitmap type;
		if(additive)
			type= CTile::additive;
		else
		{
			if(alpha)
				type= CTile::alpha;
			else
				type= CTile::diffuse;
		}

		uint8	rotalpha;
		Zone->Landscape->getTileUvScaleBiasRot(tileNumber, type, uvScaleBias, rotalpha);

		// Add the special rotation of alpha.
		if(alpha)
			orient= (orient+rotalpha)&3;
	}

}


// ***************************************************************************
void			CPatch::deleteTileUvs()
{
	Son0->deleteTileUvs();
	Son1->deleteTileUvs();
}


// ***************************************************************************
void			CPatch::recreateTileUvs()
{
	// Reset the Tile rdr list.
	for(sint i=0;i<NL3D_MAX_TILE_PASS;i++)
		RdrTileRoot[i]=NULL;
	Son0->recreateTileUvs();
	Son1->recreateTileUvs();
}

	}
}
extern "C" void	NL3D_bilinearTileLightMap(CRGBA *tex);
uint		CPatch::getTileLightMap(sint ts, sint tt, CPatchRdrPass *&rdrpass)
// ***************************************************************************
	// Compute the lightmap texture, with help of TileColors.
	CRGBA	lightText[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE];
	
	// Following code assume size of 5...
	nlassert (NL_TILE_LIGHTMAP_SIZE==5);
	float	c= -normal*getLandscape()->getAutomaticLightDir();
	// Warning: Uncompressed data should be filled now. Rebuild the shadow map ?
	if (UncompressedLumels.begin()==UncompressedLumels.end())
	// FastFloor using fistp. Don't care convention.
		// Resize the lumel array
		UncompressedLumels.resize ((OrderS*4+1)*(OrderT*4+1));
	{
		// Unpack the far shadow map
		unpackShadowMap (&UncompressedLumels[0]);
				dest[y*stride + x]=colorTable[UncompressedLumels[offset + x + (y<<NL_LUMEL_BY_TILE_SHIFT)]];
		}
	// Get the static lighting array
	const CRGBA* pStaticLight=Zone->getLandscape()->getStaticLight();
{
	// Get the tilecolors at the corners.
	for(sint i=0;i<4;i++)

		static	uint	lut[4]= {0, 4, 20, 24};
		CTileColor	&tcol= TileColors[ (tt+(i>>1))*(OrderS+1) + (ts+(i&1)) ];
	modulateTileLightmapPixelWithTileColors(ts, tt, s, t, dest);
		// Blend the color.
		CRGBA	col;
		col.set565 (tcol.Color565);
		col.A=255;
void		CPatch::computeTileLightmapPixelAroundCorner(const CVector2f &stIn, CRGBA *dest, bool lookAround)
		// TestYoyo: test with a dummy light.
		/*CBezierPatch	*pa= unpackIntoCache();
		float	t= (float)(tt+(i>>1))/OrderT;
		float	s= (float)(ts+(i&1))/OrderS;
		CVector	norm, light(0, 1, -0.75);
		norm= pa->evalNormal(s,t);
		light.normalize();
		float	f= -norm*light;
		clamp(f, 0, 1);
		col.R= 0;
		col.G= f*255;
		col.B= 0;*/
	sint	u, v;
		lightText[lut[i]]= col;
		// try to know if we must go on a neighbor patch (maybe false with bind X/1).
		if( u<0 || u>=OrderS*NL_LUMEL_BY_TILE || v<0 || v>=OrderT*NL_LUMEL_BY_TILE)
	// Bilinear!!!
	NL3D_bilinearTileLightMap(lightText);
			mustLookOnNeighbor= true;
	// Add shadow
	CRGBA *texel=lightText;
	CTileLumel *lumels=&UncompressedLumels[ 4*tt*(OrderS*4+1) + 4*ts ];
	for (int y=0; y<NL_TILE_LIGHTMAP_SIZE; y++)
	}
		for (int x=0; x<NL_TILE_LIGHTMAP_SIZE; x++)
			computeTileLightmapPixel(ts+decalS, tt+decalT, subS, subT, dest);
			// Modulate with the color of the shadow
			texel->modulateFromColor (*texel, pStaticLight[lumels[x].Shaded]);
			texel++;
					}
				}
		// Next line
		lumels+=OrderS*4+1;
			}
		}
	return Zone->Landscape->getTileLightMap(lightText, rdrpass);

}
void		CPatch::getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias)

	Zone->Landscape->getTileLightMapUvInfo(tileLightMapId, uvScaleBias);
	uint	ttDec= tt & 1;
}
void		CPatch::releaseTileLightMap(uint tileLightMapId)

	Zone->Landscape->releaseTileLightMap(tileLightMapId);
	{
	}
extern "C" void	NL3D_bilinearShadingLightMap4x (uint8 *tex, uint lineSize);
void		CPatch::expandShading (uint8 * Shading, uint ratio)

	// Check args
	nlassert ((ratio==1)||(ratio==2));		// only ratio 1:1 or 1:2
	// Number of block in a line
	// Check the size of the tile color array. Must be (OrderS+1) * (OrderT+1)
	nlassert (TileColors.size()==(uint)((OrderS+1)*(OrderT+1)));

	switch (ratio)
	{
	case 1:
	{
		// Unpack shading first
		uint s, t;
		for (t=0; t<OrderT; t++)
		for (s=0; s<OrderS; s++)
		{
			// Start of dist block
			uint8 *pStartBlock=Shading+t*(OrderS*NL_LUMEL_BY_TILE+1)*NL_LUMEL_BY_TILE+s*NL_LUMEL_BY_TILE;
			const CTileColor *pStartBlockSrc=&TileColors[0]+t*(OrderS+1)+s;

			// Place the 4 values
			pStartBlock[0]=pStartBlockSrc[0].Shade;
			pStartBlock[NL_LUMEL_BY_TILE]=pStartBlockSrc[1].Shade;
			pStartBlock[(OrderS*NL_LUMEL_BY_TILE+1)*NL_LUMEL_BY_TILE]=pStartBlockSrc[OrderS+1].Shade;
			pStartBlock[(OrderS*NL_LUMEL_BY_TILE+1)*NL_LUMEL_BY_TILE+NL_LUMEL_BY_TILE]=pStartBlockSrc[OrderS+2].Shade;

			// Bili it!
			NL3D_bilinearShadingLightMap4x (pStartBlock, OrderS*NL_LUMEL_BY_TILE+1);
		}
	}
	break;
	case 2:

		// Assume that NL_LUMEL_BY_TILE==4
		nlassert (NL_LUMEL_BY_TILE==4);
	{
		// Unpack shading first
		uint s, t;
		for (t=0; t<OrderT; t++)
		for (s=0; s<OrderS; s++)

			// Start of dist block
			uint8 *pStartBlock=Shading+t*(OrderS*2+1)*2+s*2;
			const CTileColor *pStartBlockSrc=&TileColors[0]+t*(OrderS+1)+s;
		{
			// Bili it!
			uint corner0=pStartBlockSrc[0].Shade;
			pStartBlock[0]=corner0;
			uint corner1=pStartBlockSrc[1].Shade;
			pStartBlock[2]=corner1;
			uint corner3=pStartBlockSrc[OrderS+1].Shade;
			pStartBlock[2*(OrderS*2+1)]=corner3;
			uint corner4=pStartBlockSrc[OrderS+2].Shade;
			pStartBlock[2*(OrderS*2+1)+2]=corner4;
				countU=lumelCount&3;
			pStartBlock[1]=(uint8)(((uint)corner0+(uint)corner1)>>1);
			pStartBlock[OrderS*2+1]=(uint8)(((uint)corner0+(uint)corner3)>>1);
			pStartBlock[2*(OrderS*2+1)+1]=(uint8)(((uint)corner3+(uint)corner4)>>1);
			pStartBlock[OrderS*2+3]=(uint8)(((uint)corner4+(uint)corner1)>>1);
			pStartBlock[OrderS*2+2]=(uint8)(((uint)pStartBlock[OrderS*2+1]+(uint)pStartBlock[OrderS*2+3])>>1);
		}
	}
	break;
	}
}
// ***************************************************************************
void		CPatch::unpackShadowMap (CTileLumel *pLumel, uint ratio)
{
	// Check args
	nlassert ((ratio==1)||(ratio==2));		// only ratio 1:1 or 1:2

	// Expand the shading
	expandShading (_ShadingBuffer, ratio);
	
	// *** Now create lumel

	// Create a bit stream for shadows
	CTileLumel::CStreamBit stream;

	// Init the stream on a buffer
	stream.setPtr (&CompressedLumels);

	// Ptr on the bilinear value
	uint8 *pInterpolated=_ShadingBuffer;

	// Unpack by line
	int nLine;
	int nLineCount=OrderT*NL_LUMEL_BY_TILE+1;
	uint lumelCount=(OrderS*NL_LUMEL_BY_TILE+1);
	for (nLine=0; nLine<nLineCount; nLine++)
	{
		// Skip on over two lumel line when ratio==2
		if ((ratio!=2)||((nLine&1)==0))
		{
			// Read the first bit
			if (stream.popBackBool())

				// Ok, on this line, some lumels are shadowed
				uint lumel;
				for (lumel=0; lumel<lumelCount; lumel++)
			for (uint v=0; v<NL_LUMEL_BY_TILE; v++)
					// Skip on over two lumel when ratio==2
					if ((ratio!=2)||((lumel&1)==0))
					{
						// Unpack the lumel
						pLumel->unpack (stream, *pInterpolated);
					// Copy the lumel
						// Next shading value
						pLumel++;
						pInterpolated++;
					}
					else
						CTileLumel::skip (stream);
				}

			else
			{
				// No lumels shadowed on this line
				uint lumel;
				for (lumel=0; lumel<lumelCount; lumel++)
				{
					if ((ratio!=2)||((lumel&1)==0))
					{
						// Create it
						pLumel->createUncompressed (*pInterpolated, *pInterpolated);
				// Next line
						// Next shading value
						pLumel++;
						pInterpolated++;
					}
				}
			}
		}
		else
		// skip the whole line
		{
			// Read the first bit
			if (stream.popBackBool())

				// Ok, on this line, some lumels are shadowed
				uint lumel;
				for (lumel=0; lumel<lumelCount; lumel++)
				{
					// Skip all lumel
					CTileLumel::skip (stream);
				}
					alphaMin=originalBlock[i];
		}
	}
}
				if (originalBlock[i]>alphaMax)
// ***************************************************************************
void		CPatch::packShadowMap (const CTileLumel *pLumel)
{
	// Create a bit stream for shadows
	CTileLumel::CStreamBit stream;

	// Init the stream on a buffer
	stream.setPtr (&CompressedLumels);
			// second compression
	// Ptr on the bilinear value
	uint8 *pInterpolated=_ShadingBuffer;

	// Pack by line
	int nLine;
	int nLineCount=OrderT*NL_LUMEL_BY_TILE+1;
	uint lumelCount=(OrderS*NL_LUMEL_BY_TILE+1);
	for (nLine=0; nLine<nLineCount; nLine++)
	{
		// Check if some lumels are shadowed
		// Ok, on this line, some lumels are shadowed
		uint lumel;
		bool bShadowed=false;
		for (lumel=0; lumel<lumelCount; lumel++)
		{
			// Found one?
			if (pLumel[lumel].isShadowed ())

				bShadowed=true;
				break;
			{
		}
				// Copy compressed data
		// Put true if the line is shadowed
		stream.pushBackBool (bShadowed);

		// Ok, now pack the line if some lumel are shadowed
		if (bShadowed)
		{
			// Ok, on this line, some lumels are shadowed
			for (lumel=0; lumel<lumelCount; lumel++)
			{
				// Pack the lumel
				pLumel[lumel].pack (stream);
			}

			// Next block on the line
		// Increment pointers
		pLumel+=lumelCount;

		// Next line of block
		pLumelSrc+=lumelCount*4;
	}
}

	nlassert ((lineCount&0x3)==0);
	uint size=((OrderS*NL_LUMEL_BY_TILE+1)*(OrderT*NL_LUMEL_BY_TILE+1))/8+1;

	// On bit per lumel
	uint size=numLineBlock*numLumelBlock*8;

	// 4 bits per lumel
	CompressedLumels.resize (size);

	// No line have shadows.
	memset (&CompressedLumels[0], 0, size);
}

// ***************************************************************************
void		CPatch::clearUncompressedLumels ()
{
	// Erase the uncompressed array
	if (UncompressedLumels.begin()!=UncompressedLumels.end())
		contReset (UncompressedLumels);
}

// ***************************************************************************
// ***************************************************************************
// Functions (C/ASM).
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
#define		a00	tex[0]
#define		a10	tex[1]
#define		a20	tex[2]
#define		a30	tex[3]
#define		a40	tex[4]

#define		a01	tex[5]
#define		a11	tex[6]
#define		a21	tex[7]
#define		a31	tex[8]
#define		a41	tex[9]

#define		a02	tex[10]
#define		a12	tex[11]
#define		a22	tex[12]
#define		a32	tex[13]
#define		a42	tex[14]

#define		a03	tex[15]
#define		a13	tex[16]
#define		a23	tex[17]
#define		a33	tex[18]
#define		a43	tex[19]

#define		a04	tex[20]
#define		a14	tex[21]
#define		a24	tex[22]
#define		a34	tex[23]
#define		a44	tex[24]

void	NL3D_bilinearTileLightMap(CRGBA *tex)
{
	// Fast bilinear of a 5x5 tile.
	// Corners must be set.
	// Later: pass it to ASM.

	// Fill first column 0 and column 4.
	a02.avg2(a00, a04);
	a01.avg2(a00, a02);
	a03.avg2(a02, a04);
	a42.avg2(a40, a44);
	a41.avg2(a40, a42);
	a43.avg2(a42, a44);

	// Fill Line 0.
	a20.avg2(a00, a40);
	a10.avg2(a00, a20);
	a30.avg2(a20, a40);

	// Fill Line 1.
	a21.avg2(a01, a41);
	a11.avg2(a01, a21);
	a31.avg2(a21, a41);

	// Fill Line 2. 
	a22.avg2(a02, a42);
	a12.avg2(a02, a22);
	a32.avg2(a22, a42);

	// Fill Line 3. 
	a23.avg2(a03, a43);
	a13.avg2(a03, a23);
	a33.avg2(a23, a43);

	// Fill Line 4. 
	a24.avg2(a04, a44);
	a14.avg2(a04, a24);
	a34.avg2(a24, a44);

#define		b00	tex[0]
#define		b10	tex[1]
#define		b20	tex[2]
#define		b30	tex[3]
#define		b40	tex[4]

#define		b01	tex[lineSize+0]
#define		b11	tex[lineSize+1]
#define		b21	tex[lineSize+2]
#define		b31	tex[lineSize+3]
#define		b41	tex[lineSize+4]

#define		b02	tex[lineSize*2+0]
#define		b12	tex[lineSize*2+1]
#define		b22	tex[lineSize*2+2]
#define		b32	tex[lineSize*2+3]
#define		b42	tex[lineSize*2+4]

#define		b03	tex[lineSize*3+0]
#define		b13	tex[lineSize*3+1]
#define		b23	tex[lineSize*3+2]
#define		b33	tex[lineSize*3+3]
#define		b43	tex[lineSize*3+4]

#define		b04	tex[lineSize*4+0]
#define		b14	tex[lineSize*4+1]
#define		b24	tex[lineSize*4+2]
#define		b34	tex[lineSize*4+3]
#define		b44	tex[lineSize*4+4]

void	NL3D_bilinearShadingLightMap4x(uint8 *tex, uint lineSize)
{
	// Fast bilinear of a 5x5 tile.
	// Corners must be set.
	// Later: pass it to ASM.

#ifndef NL_OS_WINDOWS	// This code doesn't work under VC++ 6.0 in Release. It should. Compilator bug.
	// Fill first column 0 and column 4.
	b02=(uint8)(((uint)b00+(uint)b04)>>1);
	b01=(uint8)(((uint)b00+(uint)b02)>>1);
	b03=(uint8)(((uint)b02+(uint)b04)>>1);
	b42=(uint8)(((uint)b40+(uint)b44)>>1);
	b41=(uint8)(((uint)b40+(uint)b42)>>1);
	b43=(uint8)(((uint)b42+(uint)b44)>>1);
	
	// Fill Line 0.
	b20=(uint8)(((uint)b00+(uint)b40)>>1);
	b10=(uint8)(((uint)b00+(uint)b20)>>1);
	b30=(uint8)(((uint)b20+(uint)b40)>>1);

	// Fill Line 1.
	b21=(uint8)(((uint)b01+(uint)b41)>>1);
	b11=(uint8)(((uint)b01+(uint)b21)>>1);
	b31=(uint8)(((uint)b21+(uint)b41)>>1);

	// Fill Line 2. 
	b22=(uint8)(((uint)b02+(uint)b42)>>1);
	b12=(uint8)(((uint)b02+(uint)b22)>>1);
	b32=(uint8)(((uint)b22+(uint)b42)>>1);

	// Fill Line 3. 
	b23=(uint8)(((uint)b03+(uint)b43)>>1);
	b13=(uint8)(((uint)b03+(uint)b23)>>1);
	b33=(uint8)(((uint)b23+(uint)b43)>>1);

	// Fill Line 4. 
	b24=(uint8)(((uint)b04+(uint)b44)>>1);
	b14=(uint8)(((uint)b04+(uint)b24)>>1);
	b34=(uint8)(((uint)b24+(uint)b44)>>1);
#else // NL_OS_WINDOWS
	uint s, t;
	for (t=0; t<5; t++)
	for (s=0; s<5; s++)
	{
		tex[t*lineSize+s]= 
			(uint8)((((uint)b00*(4-s) + (uint)b40*s)*(4-t) +
			((uint)b04*(4-s) + (uint)b44*s)*t)>>4);
	}
#endif // NL_OS_WINDOWS
	nlassert(Son1);
	Son0->appendTessellationLeaves(leaves);
		neighborEdge.MultipleBindNum= 0;
	}
}

} // NL3D

