/** \file patch.cpp
 * <File description>
 *
 * $Id: patch.cpp,v 1.12 2000/11/20 13:40:00 berenguier Exp $
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


// ***************************************************************************
CPatch::CPatch()
{
	Zone= NULL;
	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	Clipped=false;

	// To force computation on next render().
	Far0= -1;
	Far1= -1;
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
		nlassert(Son0->FRight==NULL && Son0->FLeft==NULL);
		nlassert(Son1->FRight==NULL && Son1->FLeft==NULL);

		delete Son0;
		delete Son1;
		// Vertices are smartptr/deleted in zone.
		Zone= NULL;
	}

	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	Clipped=false;

	// To force computation on next render().
	Far0= -1;
	Far1= -1;
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
		Tesselation layout.

		A-------D
		|\ Son1 |
		|  \    |
		|    \  |
		| Son0 \|
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
	Son0->VBase= b;
	Son0->VLeft= c;
	Son0->VRight= a;
	Son0->PVBase.setST(0, 1);
	Son0->PVLeft.setST(1, 1);
	Son0->PVRight.setST(0, 0);
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
	Son1->VBase= d;
	Son1->VLeft= a;
	Son1->VRight= c;
	Son1->PVBase.setST(1, 0);
	Son1->PVLeft.setST(0, 0);
	Son1->PVRight.setST(1, 1);
	Son1->FBase= Son0;
	Son1->FLeft= NULL;
	Son1->FRight= NULL;
	// No tile info.
	Son1->Size= ErrorSize/2;
	Son1->Center= (Son1->VBase->EndPos + Son1->VLeft->EndPos + Son1->VRight->EndPos)/3;
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
	OrderS= orderS;
	OrderT= orderT;

	// Compile additional infos.
	sint	ps= getPowerOf2(orderS) , pt= getPowerOf2(orderT);
	sint	pmin= min(ps,pt);
	sint	pmax= max(ps,pt);
	// Rectangular patch OK.
	// Work, since patch 1xX are illegal. => The TileLimitLevel is at least 2 level distant from the time where
	// the rectangular patch is said "un-rectangular-ed" (tesselation looks like square). Hence, there is no problem
	// with rectangular UV geomorph.
	TileLimitLevel= pmin*2 + pmax-pmin;

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
CVector			CPatch::computeVertex(float s, float t)
{
	// First, unpack...
	CBezierPatch	*patch= unpackIntoCache();

	// TODO_NOISE/TODO_UVCORRECT: use UV correction, and use displacement map, to disturb result.
	return patch->eval(s,t);
}
// ***************************************************************************
void			CPatch::refine()
{
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
		if(!BSphere.clipBackUnitPlane(pyramid[i]))
		{
			Clipped= true;
			break;
		}
	}

}


// ***************************************************************************
sint			CPatch::resetTileIndices(CTessFace *pFace)
{
	sint	ret=0;
	for(;pFace; pFace=pFace->RenderNext)
	{
		// If tile level reached for this face.
		if(pFace->TileMaterial)
		{
			pFace->TileUvBase->TileIndex= 0;
			pFace->TileUvLeft->TileIndex= 0;
			pFace->TileUvRight->TileIndex= 0;
			ret++;
		}
	}

	return ret;
}
// ***************************************************************************
sint			CPatch::resetFarIndices(CTessFace *pFace)
{
	sint	ret=0;
	for(;pFace; pFace=pFace->RenderNext)
	{
		pFace->VBase->FarIndex= 0;
		pFace->VLeft->FarIndex= 0;
		pFace->VRight->FarIndex= 0;
		ret++;
	}
	return ret;
}

// ***************************************************************************
void			CPatch::preRender()
{
	// Don't do anything if clipped.
	if(Clipped)
		return;

	// Classify the patch.
	//====================
	sint	far0,far1;
	float	r= (CTessFace::RefineCenter-BSphere.Center).norm() - BSphere.Radius;
	float	rr;
	if(r<CTessFace::TileDistNear)
		rr= r-CTessFace::TileDistNear, far0= 0;
	else if(r<CTessFace::Far0Dist)
		rr= r-CTessFace::Far0Dist, far0= 1;
	else if(r<CTessFace::Far1Dist)
		rr= r-CTessFace::Far1Dist, far0= 2;
	else
		far0= 3;
	// Transition with the next level.
	far1=0;
	if(far0<3 && rr>-(CTessFace::FarTransition+2*BSphere.Radius))
	{
		far1= far0+1;
	}


	// Update Texture Info.
	//=====================
	if(far0!=Far0 || far1!=Far1)
	{
		Far0= far0;
		Far1= far1;
		// Compute / get the texture Far.
		if(Far0>=1)
			Pass0= getFarRenderPass(Far0, Far0UVScale, Far0UBias, Far0VBias);
		else
			Pass0= NULL;
		if(Far1>=1)
		{
			Pass1= getFarRenderPass(Far1, Far1UVScale, Far1UBias, Far1VBias);
			// Compute info for transition.
			float	farDist;
			switch(Far1)
			{
				case 1: farDist= CTessFace::TileDistNear; break;
				case 2: farDist= CTessFace::Far0Dist; break;
				case 3: farDist= CTessFace::Far1Dist; break;
				default: nlstop;
			};
			TransitionSqrMin= sqr(farDist-CTessFace::FarTransition);
			OOTransitionSqrDelta= 1.0f/(sqr(farDist)-TransitionSqrMin);
		}
		else
			Pass1= NULL;
	}


	// Make render list.
	//==================
	RdrRoot= NULL;
	Son0->appendToRenderList(RdrRoot);
	Son1->appendToRenderList(RdrRoot);
}


// ***************************************************************************
sint			CPatch::getFarIndex0(CTessVertex *vert, CTessFace::CParamCoord  pc)
{
	if(vert->FarIndex==0)
	{
		// Compute/build the new vertex.
		vert->FarIndex= CTessFace::CurrentVertexIndex++;

		// Set Pos.
		CTessFace::CurrentVB->setVertexCoord(vert->FarIndex, vert->Pos.x, vert->Pos.y, vert->Pos.z);

		// Set Uvs.
		float	u,v;
		u= pc.getS()* Far0UVScale + Far0UBias;
		v= pc.getT()* Far0UVScale + Far0VBias;
		CTessFace::CurrentVB->setTexCoord(vert->FarIndex, 0, u, v);

		// Compute color.
		CRGBA	col(255,255,255,255);
		// TODO_CLOUD: use normal information, for cloud attenuation, in RGB fields.
		// For Far0, alpha is un-usefull.
		CTessFace::CurrentVB->setRGBA(vert->FarIndex, col);
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
		CTessFace::CurrentVB->setVertexCoord(vert->FarIndex, vert->Pos.x, vert->Pos.y, vert->Pos.z);

		// Set Uvs.
		float	u,v;
		u= pc.getS()* Far1UVScale + Far1UBias;
		v= pc.getT()* Far1UVScale + Far1VBias;
		CTessFace::CurrentVB->setTexCoord(vert->FarIndex, 0, u, v);

		// Compute color.
		CRGBA	col(255,255,255,255);
		// TODO_CLOUD: use normal information, for cloud attenuation, in RGB fields.
		// For Far1, use alpha fro transition.
		float	f= (vert->Pos - CTessFace::RefineCenter).sqrnorm();
		f= (f-TransitionSqrMin) * OOTransitionSqrDelta;
		clamp(f,0,1);
		col.A= (uint8)(f*255);
		CTessFace::CurrentVB->setRGBA(vert->FarIndex, col);
		// END!!
	}
	
	return vert->FarIndex;
}


// ***************************************************************************
sint			CPatch::getTileIndex(CTessVertex *vert, ITileUv *uv, sint idUv)
{
	if(uv->TileIndex==0)
	{
		// Compute/build the new vertex.
		uv->TileIndex= CTessFace::CurrentVertexIndex++;

		// Set Pos.
		CTessFace::CurrentVB->setVertexCoord(uv->TileIndex, vert->Pos.x, vert->Pos.y, vert->Pos.z);

		// Set Uvs.
		// TODO_BUMP: chose beetween bump and normal uvs. Always normal here.
		ITileUvNormal	*uvn= (ITileUvNormal*)uv;
		CPassUvNormal	&uvpass= uvn->UvPasses[idUv];
		CTessFace::CurrentVB->setTexCoord(uv->TileIndex, 0, uvpass.PUv0.U, uvpass.PUv0.V);
		CTessFace::CurrentVB->setTexCoord(uv->TileIndex, 1, uvpass.PUv1.U, uvpass.PUv1.V);

		// Compute color.
		CRGBA	col(255,255,255,255);
		// TODO_CLOUD/TODO_ADDITIVE: use cloud color information for RGB. And use alpha global for global apparition of 
		// additives tiles only.
		CTessFace::CurrentVB->setRGBA(uv->TileIndex, col);
		// END!!
	}

	return uv->TileIndex;
}


// ***************************************************************************
void			CPatch::renderFar0()
{
	if(Pass0 && !Clipped)
	{
		sint	nTris= resetFarIndices(RdrRoot);
		// Realloc if necessary the VertexBuffer.
		if((sint)CTessFace::CurrentVB->capacity() < CTessFace::CurrentVertexIndex+3*nTris)
			CTessFace::CurrentVB->reserve(CTessFace::CurrentVertexIndex+3*nTris);
		// Add tris.
		CTessFace	*pFace= RdrRoot;
		for(;pFace; pFace=pFace->RenderNext)
		{
			Pass0->addTri(getFarIndex0(pFace->VBase, pFace->PVBase),
						getFarIndex0(pFace->VLeft, pFace->PVLeft),
						getFarIndex0(pFace->VRight, pFace->PVRight));
		}
	}
}


// ***************************************************************************
void			CPatch::renderFar1()
{
	if(Pass1 && !Clipped)
	{
		sint	nTris= resetFarIndices(RdrRoot);
		// Realloc if necessary the VertexBuffer.
		if((sint)CTessFace::CurrentVB->capacity() < CTessFace::CurrentVertexIndex+3*nTris)
			CTessFace::CurrentVB->reserve(CTessFace::CurrentVertexIndex+3*nTris);
		// Add tris.
		CTessFace	*pFace= RdrRoot;
		for(;pFace; pFace=pFace->RenderNext)
		{
			Pass1->addTri(getFarIndex1(pFace->VBase, pFace->PVBase),
						getFarIndex1(pFace->VLeft, pFace->PVLeft),
						getFarIndex1(pFace->VRight, pFace->PVRight));
		}
	}
}
// ***************************************************************************
void			CPatch::renderTile(sint pass)
{
	// If tile mode.
	if(Far0==0 && !Clipped)
	{
		sint	nTris= resetTileIndices(RdrRoot);
		// Realloc if necessary the VertexBuffer (at max possible).
		if((sint)CTessFace::CurrentVB->capacity() < CTessFace::CurrentVertexIndex+3*nTris)
			CTessFace::CurrentVB->reserve(CTessFace::CurrentVertexIndex+3*nTris);
		// Add tris.
		CTessFace	*pFace= RdrRoot;
		for(;pFace; pFace=pFace->RenderNext)
		{
			// If tile level reached for this face.
			if(pFace->TileMaterial)
			{
				CPatchRdrPass	*tilePass= pFace->TileMaterial->Pass[pass];
				// If tile pass enabled.
				if(tilePass)
				{
					sint	idUv=pFace->TileMaterial->PassToUv[pass];
					tilePass->addTri(getTileIndex(pFace->VBase, pFace->TileUvBase, idUv),
							getTileIndex(pFace->VLeft, pFace->TileUvLeft, idUv),
							getTileIndex(pFace->VRight, pFace->TileUvRight, idUv));
				}
			}
		}
	}
}


// ***************************************************************************
void			CPatch::unbindFromAll()
{
	nlassert(Son0 && Son1);

	Son0->unbindFromAll();
	Son1->unbindFromAll();
	Son0->forceMerge();
	Son1->forceMerge();
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	delete	Son0;
	delete	Son1;
	Son0= NULL;
	Son1= NULL;

	// re-build Sons.
	makeRoots();
}


// ***************************************************************************
void			CPatch::unbindFrom(CPatch *other)
{
	nlassert(Son0 && Son1);

	Son0->unbindFrom(other);
	Son1->unbindFrom(other);
}


// ***************************************************************************
CTessFace		*CPatch::getRootFaceForEdge(sint edge) const
{
	nlassert(edge>=0 && edge<=3);

	// See tessellation rules.
	if(edge==0 || edge==1)
		return Son0;
	else
		return Son1;
}


// ***************************************************************************
void			CPatch::changeEdgeNeighbor(sint edge, CTessFace *to)
{
	nlassert(edge>=0 && edge<=3);

	switch(edge)
	{
		case 0: Son0->FRight= to; break;
		case 1: Son0->FLeft= to; break;
		case 2: Son1->FRight= to; break;
		case 3: Son1->FLeft= to; break;
	}
}


// ***************************************************************************
void			CPatch::bind(CBindInfo	Edges[4])
{
	// THIS PATCH MSUT BE UNBOUND FIRST!!!!!
	nlassert(Son0 && Son1);
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	nlassert(Son0->FRight==NULL && Son0->FLeft==NULL);
	nlassert(Son1->FRight==NULL && Son1->FLeft==NULL);

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
	// NB: no propagation problem, since the patch has root only (since unbound!!!)

	// Bind the roots.
	for(sint i=0;i<4;i++)
	{
		CBindInfo	&bind= Edges[i];

		// Well, take it easy for now....
		nlassert(bind.NPatchs== 0 || bind.NPatchs== 1);
		if(bind.NPatchs==1)
		{
			// Bind me on Next.
			this->changeEdgeNeighbor(i, bind.Next[0]->getRootFaceForEdge(bind.Edge[0]));
			// Bind Next on me.
			bind.Next[0]->changeEdgeNeighbor(bind.Edge[0], this->getRootFaceForEdge(i));
		}
		// else don't bind!!! :)
	}

	// Propagate the binds to sons.
	Son0->updateBind();
	Son1->updateBind();

}

// ***************************************************************************
void			CPatch::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);

	f.serial(Vertices[0], Vertices[1], Vertices[2], Vertices[3]);
	f.serial(Tangents[0], Tangents[1], Tangents[2], Tangents[3]);
	f.serial(Tangents[4], Tangents[5], Tangents[6], Tangents[7]);
	f.serial(Interiors[0], Interiors[1], Interiors[2], Interiors[3]);
}


// ***************************************************************************
CPatchRdrPass	*CPatch::getFarRenderPass(sint farLevel, float &farUVScale, float &farUBias, float &farVBias)
{
	// TODO_TEXTURE.
	// dummy texture.
	farUBias= 0;
	farVBias= 0;
	farUVScale= 1.0f/farLevel;
	return Zone->Landscape->getFarRenderPass();
}


// ***************************************************************************
CPatchRdrPass	*CPatch::getTileRenderPass(sint tileId, sint pass)
{
	// TODO_TEXTURE.
	// dummy texture.
	if(pass==0)
		return Zone->Landscape->getTileRenderPass();
	else
		return NULL;
}

	}
}

} // NL3D

