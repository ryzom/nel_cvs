/** \file patch.cpp
 * <File description>
 *
 * $Id: patch.cpp,v 1.4 2000/10/27 14:30:06 berenguier Exp $
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
}
// ***************************************************************************
CPatch::~CPatch()
{
	unbind();
	delete Son0;
	delete Son1;
	// Vertices are smartptr/deleted.
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
	float	bias= Zone->PatchBias, scale= Zone->PatchScale;

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

	// TODO: modulate this value with tangents (roundiness of patch), and with the displacement map.
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
	CVector		bmin= v0, bmax= v0;
	for(i=0;i<4;i++)
	{
		bmin.minof(bmin, p.Vertices[i]);
		bmax.maxof(bmin, p.Vertices[i]);
	}
	for(i=0;i<8;i++)
	{
		bmin.minof(bmin, p.Tangents[i]);
		bmax.maxof(bmin, p.Tangents[i]);
	}
	for(i=0;i<4;i++)
	{
		bmin.minof(bmin, p.Interiors[i]);
		bmax.maxof(bmin, p.Interiors[i]);
	}
	// TODO: modulate with the displacement map.

	CAABBox	ret;
	ret.setMinMax(bmin, bmax);
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
	if(errorSize==0)
		computeDefaultErrorSize();
	else
		ErrorSize= errorSize;

	nlassert(z);
	Zone= z;
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

	// TODO: use UV correction, and use displacement map, to disturb result.
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
void			CPatch::render()
{
	// TODOR.
}


// ***************************************************************************
void			CPatch::unbind()
{
	nlassert(Son0 && Son1);

	Son0->unbind();
	Son1->unbind();
	Son0->forceMerge();
	Son1->forceMerge();
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	delete	Son0;
	delete	Son1;

	// re-build Sons.
	makeRoots();
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
		case 0: Son0->FRight= to;
		case 1: Son0->FLeft= to;
		case 2: Son1->FRight= to;
		case 3: Son1->FLeft= to;
	}
}


// ***************************************************************************
void			CPatch::bind(CBindInfo	Edges[4])
{
	// Clear first...
	unbind();

	// Just recompute base vertices.
	// TODO. bind the noise info before.
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
			this->changeEdgeNeighbor(i, bind.Next0->getRootFaceForEdge(bind.Edge0));
			// Bind Next on me.
			bind.Next0->changeEdgeNeighbor(bind.Edge0, this->getRootFaceForEdge(i));
		}
		// else don't bind!!! :)
	}

	// Propagate the binds to sons.
	Son0->updateBind();
	Son1->updateBind();

}

	}
}

} // NL3D

