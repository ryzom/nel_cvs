/** \file zone.cpp
 * <File description>
 *
 * $Id: zone.cpp,v 1.14 2000/11/30 10:54:58 berenguier Exp $
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

#include "nel/3d/zone.h"
#include "nel/misc/common.h"


using namespace NLMISC;
using namespace std;


namespace NL3D {


const sint	ClipIn= 0;
const sint	ClipOut= 1;
const sint	ClipSide= 2;


// ***************************************************************************
CZone::CZone()
{
	ComputeTileErrorMetric= false;
	ZoneId= 0;
	Compiled= false;
	Landscape= NULL;
}
// ***************************************************************************
CZone::~CZone()
{
	// release() must have been called.
	nlassert(!Compiled);
}


// ***************************************************************************
void			CZone::build(uint16 zoneId, const std::vector<CPatchInfo> &patchs, const std::vector<CBorderVertex> &borderVertices)
{
	sint	i,j;
	nlassert(!Compiled);

	ZoneId= zoneId;
	BorderVertices= borderVertices;

	// Compute the bbox and the bias/scale.
	//=====================================
	CAABBox		bb;
	if(patchs.size())
		bb.setCenter(patchs[0].Patch.Vertices[0]);
	bb.setHalfSize(CVector::Null);
	for(j=0;j<(sint)patchs.size();j++)
	{
		const CBezierPatch	&p= patchs[j].Patch;
		for(i=0;i<4;i++)
			bb.extend(p.Vertices[i]);
		for(i=0;i<8;i++)
			bb.extend(p.Tangents[i]);
		for(i=0;i<4;i++)
			bb.extend(p.Interiors[i]);
	}
	ZoneBB= bb;
	// Take a security for noise. (usefull for zone clipping).
	ZoneBB.setHalfSize(ZoneBB.getHalfSize()+CVector(NL3D_NOISE_MAX, NL3D_NOISE_MAX, NL3D_NOISE_MAX));
	CVector	hs= ZoneBB.getHalfSize();
	float	rmax= maxof(hs.x, hs.y, hs.z);
	PatchScale= rmax / 32760;		// Prevent from float imprecision by taking 32760 and not 32767.
	PatchBias= ZoneBB.getCenter();


	// Compute/compress Patchs.
	//=========================
	Patchs.resize(patchs.size());
	PatchConnects.resize(patchs.size());
	sint	maxVertex=-1;
	for(j=0;j<(sint)patchs.size();j++)
	{
		const CPatchInfo	&pi= patchs[j];
		const CBezierPatch	&p= pi.Patch;
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		// Build the patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].pack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].pack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].pack(p.Interiors[i], PatchBias, PatchScale);
		pa.Tiles= pi.Tiles;
		nlassert(pa.Tiles.size()== (uint)pi.OrderS*pi.OrderT);



		// Build the patchConnect.
		pc.OrderS= pi.OrderS;
		pc.OrderT= pi.OrderT;
		pc.ErrorSize= pi.ErrorSize;
		for(i=0;i<4;i++)
		{
			pc.BaseVertices[i]= pi.BaseVertices[i];
			maxVertex= max((sint)pc.BaseVertices[i], maxVertex);
		}
		for(i=0;i<4;i++)
			pc.BindEdges[i]= pi.BindEdges[i];
	}

	NumVertices= maxVertex+1;
}


// ***************************************************************************
void			CZone::build(const CZone &zone)
{
	nlassert(!Compiled);

	ZoneId= zone.ZoneId;
	BorderVertices= zone.BorderVertices;

	// Compute the bbox and the bias/scale.
	//=====================================
	ZoneBB= zone.ZoneBB;
	PatchScale= zone.PatchScale;
	PatchBias= zone.PatchBias;


	// Compute/compress Patchs.
	//=========================
	Patchs= zone.Patchs;
	PatchConnects= zone.PatchConnects;


	NumVertices= zone.NumVertices;
}



// ***************************************************************************
void			CBorderVertex::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);
	f.serial(CurrentVertex, NeighborZoneId, NeighborVertex);
}
void			CZone::CPatchConnect::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);
	f.serial(OrderS, OrderT, ErrorSize);
	f.serial(BaseVertices[0], BaseVertices[1], BaseVertices[2], BaseVertices[3]);
	f.serial(BindEdges[0], BindEdges[1], BindEdges[2], BindEdges[3]);
}
void			CPatchInfo::CBindInfo::serial(NLMISC::IStream &f)
{
	int		i;
	uint	ver= f.serialVersion(0);
	f.serial(NPatchs);
	f.serial(ZoneId);
	for(i=0;i<4;i++)
		f.serial(Next[i]);
	for(i=0;i<4;i++)
		f.serial(Edge[i]);
}

// ***************************************************************************
void			CZone::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);

	f.serial(ZoneId, ZoneBB, PatchBias, PatchScale, NumVertices);
	f.serialCont(BorderVertices);
	f.serialCont(Patchs);
	f.serialCont(PatchConnects);
}


// ***************************************************************************
void			CZone::compile(CLandscape *landscape, TZoneMap &loadedZones)
{
	sint	i,j;
	TZoneMap		neighborZones;

	// Can't compile if compiled.
	nlassert(!Compiled);
	Landscape= landscape;

	// Attach this to loadedZones.
	//============================
	nlassert(loadedZones.find(ZoneId)==loadedZones.end());
	loadedZones[ZoneId]= this;
	
	// Create/link the base vertices according to present neigbor zones.
	//============================
	BaseVertices.clear();
	BaseVertices.resize(NumVertices);
	// First try to link vertices to other.
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		sint	cur= BorderVertices[i].CurrentVertex;
		sint	vertto= BorderVertices[i].NeighborVertex;
		sint	zoneto= BorderVertices[i].NeighborZoneId;
		nlassert(cur<NumVertices);

		if(loadedZones.find(zoneto)!=loadedZones.end())
		{
			CZone	*zone;
			zone= (*loadedZones.find(zoneto)).second;
			nlassert(zone!=this);
			// insert the zone in the neigborood (if not done...).
			neighborZones[zoneto]= zone;
			// Doesn't matter if BaseVertices is already linked to an other zone... 
			// This should be the same pointer in this case...
			BaseVertices[cur]=  zone->getBaseVertex(vertto);
		}
	}
	// Else, create unbounded vertices.
	for(i=0;i<(sint)BaseVertices.size();i++)
	{
		if(BaseVertices[i]==NULL)
		{
			BaseVertices[i]=  new CTessBaseVertex;
		}
	}


	// compile() the patchs.
	//======================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];
		CTessVertex			*baseVertices[4];

		baseVertices[0]= &(BaseVertices[pc.BaseVertices[0]]->Vert);
		baseVertices[1]= &(BaseVertices[pc.BaseVertices[1]]->Vert);
		baseVertices[2]= &(BaseVertices[pc.BaseVertices[2]]->Vert);
		baseVertices[3]= &(BaseVertices[pc.BaseVertices[3]]->Vert);
		pa.compile(this, pc.OrderS, pc.OrderT, baseVertices, pc.ErrorSize);
	};

	// bind() the patchs. (after all compiled).
	//===================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		bindPatch(loadedZones, pa, pc);
	}
	
	
	// rebindBorder() on neighbor zones.
	//==================================
	ItZoneMap		zoneIt;
	// Traverse the neighborood.
	for(zoneIt= neighborZones.begin(); zoneIt!=neighborZones.end(); zoneIt++)
	{
		(*zoneIt).second->rebindBorder(loadedZones);
	}

	// End!!
	Compiled= true;
}

// ***************************************************************************
void			CZone::release(TZoneMap &loadedZones)
{
	sint	i,j;

	if(!Compiled)
		return;

	// detach this zone to loadedZones.
	//=================================
	nlassert(loadedZones.find(ZoneId)!=loadedZones.end());
	loadedZones.erase(ZoneId);
	// It doesn't server to unbindPatch(), since patch is not binded to neigbors.


	// unbind() the patchs.
	//=====================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];
		unbindPatch(loadedZones, pa, pc);
		/* 
			This patch may be unbinded with exceptions (multiple patch case), but there is no problem, since in this case,
			his neighbor will (or have precedently) unbind.
			Since only Bind 1/1 are permitted on zone neighborood, there should be no problem:
			patch are unbinded with no exceptions.
		*/
	}


	// rebindBorder() on neighbor zones.
	//==================================
	// Build the nieghborood.
	TZoneMap		neighborZones;
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		sint	cur= BorderVertices[i].CurrentVertex;
		sint	zoneto= BorderVertices[i].NeighborZoneId;
		nlassert(cur<NumVertices);

		if(loadedZones.find(zoneto)!=loadedZones.end())
		{
			CZone	*zone;
			zone= (*loadedZones.find(zoneto)).second;
			nlassert(zone!=this);
			// insert the zone in the neigborood (if not done...).
			neighborZones[zoneto]= zone;
		}
	}
	// rebind borders.
	ItZoneMap		zoneIt;
	// Traverse the neighborood.
	for(zoneIt= neighborZones.begin(); zoneIt!=neighborZones.end(); zoneIt++)
	{
		// Since 
		(*zoneIt).second->rebindBorder(loadedZones);
	}


	// release() the patchs.
	//======================
	// unbind() need compiled neigbor patchs, so do the release after all unbind (so after rebindBorder() too...).
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		pa.release();
	}


	// destroy/unlink the base vertices (internal..), according to present neigbor zones.
	//=================================
	// Just release the smartptrs (easy!!). Do it after patchs released...
	BaseVertices.clear();
	

	// End!!
	Compiled= false;
	Landscape= NULL;
}


// ***************************************************************************
// ***************************************************************************
// Private part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::rebindBorder(TZoneMap &loadedZones)
{
	sint	j;

	// rebind patchs which are on border.
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		if(patchOnBorder(pc))
			bindPatch(loadedZones, pa, pc);
	}
}

// ***************************************************************************
CPatch		*CZone::getZonePatch(TZoneMap &loadedZones, sint zoneId, sint patch)
{
	if(loadedZones.find(zoneId)==loadedZones.end())
		return NULL;
	else
		return (loadedZones[zoneId])->getPatch(patch);
}


// ***************************************************************************
void		CZone::unbindAndMakeBindInfo(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc, CPatch::CBindInfo	edges[4])
{
	CPatch	*exceptions[4]= {NULL, NULL, NULL, NULL};

	/*
		Remind: the old version with CPatch::unbindFrom*() doesn't work because of CZone::release(). This function
		first erase the zone from loadedZones...
		Not matter here. We use CPatch::unbind() which should do all the good job correctly (unbind pa from ohters
		, and unbind others from pa at same time).
	*/

	// Fill all edges.
	for(sint i=0;i<4;i++)
	{
		CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[i];
		CPatch::CBindInfo		&paBind= edges[i];

		nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);
		paBind.NPatchs= pcBind.NPatchs;

		// Special case of a small patch connected to a bigger.
		if(paBind.NPatchs==5)
		{
			// In this case, neither the unbind must not be done, nor the bind.
			exceptions[i]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[0]);
			// This code prevent the bind in CPatch::bind() to be done!!
			// The bind must not be done, since exceptions[] prevent the unbind!
			paBind.NPatchs=0;
			continue;
		}


		if(paBind.NPatchs>=1)
		{
			paBind.Edge[0]= pcBind.Edge[0];
			paBind.Next[0]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[0]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[0])
				paBind.NPatchs=0;
		}
		if(paBind.NPatchs>=2)
		{
			paBind.Edge[1]= pcBind.Edge[1];
			paBind.Next[1]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[1]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[1])
				paBind.NPatchs=0;
		}
		if(paBind.NPatchs>=4)
		{
			paBind.Edge[2]= pcBind.Edge[2];
			paBind.Edge[3]= pcBind.Edge[3];
			paBind.Next[2]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[2]);
			paBind.Next[3]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[3]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[2] || !paBind.Next[3])
				paBind.NPatchs=0;
		}
	}

	pa.unbind(exceptions);
}

// ***************************************************************************
void		CZone::bindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc)
{
	CPatch::CBindInfo	edges[4];

	unbindAndMakeBindInfo(loadedZones, pa, pc, edges);

	pa.bind(edges);
}


// ***************************************************************************
void		CZone::unbindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc)
{
	CPatch::CBindInfo	edges[4];

	unbindAndMakeBindInfo(loadedZones,  pa, pc, edges);

	// Don't rebind.
}


// ***************************************************************************
bool			CZone::patchOnBorder(const CPatchConnect &pc) const
{
	// If only one of neighbor patch is not of this zone, we are on a border.

	// Test all edges.
	for(sint i=0;i<4;i++)
	{
		const CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[i];

		nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);
		if(pcBind.NPatchs>=1)
		{
			if(pcBind.ZoneId != ZoneId)
				return true;
		}
	}

	return false;
}


// ***************************************************************************
// ***************************************************************************
// Render part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::clip(const std::vector<CPlane>	&pyramid)
{
	nlassert(Compiled);

	// Store current pyramid.
	CurrentPyramid= pyramid;

	// Fill ClipResult.
	ClipResult= ClipIn;
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// If entirely out.
		if(!ZoneBB.clipBackUnitPlane(pyramid[i]))
		{
			ClipResult= ClipOut;
			// If out of only one plane, out of all.
			break;
		}
		// If partially IN (ie not entirely out, and not entirely IN)
		else if(ZoneBB.clipFrontUnitPlane(pyramid[i]))
		{
			// Force ClipResult to be ClipSide, and not ClipIn.
			ClipResult=ClipSide;
		}
	}

	// Fill computeTileErrorMetric.
	CBSphere		zonesphere(ZoneBB.getCenter(), ZoneBB.getRadius());
	if(zonesphere.intersect(CBSphere(CTessFace::RefineCenter, CTessFace::TileDistFar)))
		ComputeTileErrorMetric= true;
	else
		ComputeTileErrorMetric= false;

	// Easy Clip  :)
	if(Patchs.size()==0)
	{
		ClipResult= ClipOut;
	}

}


// ***************************************************************************
// Code for Debug test Only..
/*
static	void	cleanTess(CTessFace *face)
{
	if(!face->isLeaf())
	{
		cleanTess(face->SonLeft);
		cleanTess(face->SonRight);
	}
	// If has father, clean it.
	if(face->Father)
	{
		CTessFace	*face1=face->Father;
		CTessFace	*face2=face->Father->FBase;
		face1->FLeft= face1->SonLeft->FBase;
		face1->FRight= face1->SonRight->FBase;
		if(face2!=NULL)
		{
			face2->FLeft= face2->SonLeft->FBase;
			face2->FRight= face2->SonRight->FBase;
		}
	}
}
static	void	testTess(CTessFace *face)
{
	if(!face->isLeaf())
	{
		testTess(face->SonLeft);
		testTess(face->SonRight);
	}
	// Test validity.
	nlassert(!face->FBase || face->FBase->Patch!=(CPatch*)0xdddddddd);
	nlassert(!face->FLeft || face->FLeft->Patch!=(CPatch*)0xdddddddd);
	nlassert(!face->FRight || face->FRight->Patch!=(CPatch*)0xdddddddd);
}
static	void	checkTess()
{
	// This test should be inserted at begin of CZone::refine().
	// And it needs hacking public/private.
	CPatch		*pPatch;
	sint		n;
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		cleanTess(pPatch->Son0);
		cleanTess(pPatch->Son1);
	}
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		testTess(pPatch->Son0);
		testTess(pPatch->Son1);
	}
}
*/


// ***************************************************************************
// TempYoyo.
//volatile sint pipo1;
//volatile sint pipo2;
void			CZone::refine()
{
	nlassert(Compiled);

	// TempYoyo.
	// For the monkey bind test.
	/*extern sint numFrames;
	pipo1=(rand()>>12)&1;
	pipo2=(rand()>>12)&1;
	//if(pipo1 && numFrames>1360)
	if(true)
	{
		TZoneMap	pipoMap;
		pipoMap[ZoneId]= this;
		bindPatch(pipoMap, Patchs[0], PatchConnects[0]);
	}*/

	
	// Force refine of invisible zones only every 8 times.
	if(ClipResult==ClipOut && (CTessFace::CurrentDate&7)!=(ZoneId&7))
		return;
	// Fuck stlport....
	if(Patchs.size()==0)
		return;

	// Else refine ALL patchs (even those which may be invisible).
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->refine();
	}
}
// ***************************************************************************
void			CZone::preRender()
{
	nlassert(Compiled);

	// Clip Pass.
	if(ClipResult==ClipOut)
		return;
	else if(ClipResult==ClipIn)
	{
		CPatch		*pPatch= &(*Patchs.begin());
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			pPatch->forceNoClip();
		}
	}
	else
	{
		CPatch		*pPatch= &(*Patchs.begin());
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			pPatch->clip(CurrentPyramid);
		}
	}

	// PreRender Pass.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->preRender();
	}
}
// ***************************************************************************
void			CZone::renderFar0()
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// RenderFar0.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->renderFar0();
	}
}
// ***************************************************************************
void			CZone::renderFar1()
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// RenderFar1.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->renderFar1();
	}
}
// ***************************************************************************
void			CZone::renderTile(sint pass)
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// RenderTile.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->renderTile(pass);
	}
}


// ***************************************************************************
// ***************************************************************************
// Misc part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::changePatchTexture(TZoneMap &loadedZones, sint numPatch, const std::vector<CTileElement> &tiles)
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());
	// unbind => forceMerge() the patch. Hence, tiles are reseted.
	unbindPatch(loadedZones, Patchs[numPatch], PatchConnects[numPatch]);

	// Update the patch texture.
	nlassert(Patchs[numPatch].Tiles.size() ==tiles.size() );
	Patchs[numPatch].Tiles= tiles;

	// rebind. At next refine(), tesselation will be updated and tiles created.
	bindPatch(loadedZones, Patchs[numPatch], PatchConnects[numPatch]);
}


} // NL3D
