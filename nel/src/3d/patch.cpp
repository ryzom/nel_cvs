/** \file patch.cpp
 * <File description>
 *
 * $Id: patch.cpp,v 1.79 2002/03/07 15:39:08 berenguier Exp $
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

#include "std3d.h"


#include "3d/patch.h"
#include "3d/tessellation.h"
#include "3d/bezier_patch.h"
#include "3d/zone.h"
#include "3d/landscape.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"
#include "3d/patchuv_locator.h"
#include "3d/vegetable_manager.h"
#include "3d/fast_floor.h"
#include "3d/light_influence_interpolator.h"
using	namespace	std;
using	namespace	NLMISC;


namespace NL3D 
{


// ***************************************************************************
CBezierPatch	CPatch::CachePatch;
const CPatch	*CPatch::LastPatch= NULL;
uint32			CPatch::_Version=6;


// ***************************************************************************
CPatch::CPatch()
{
	Zone= NULL;
	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	TessBlockRefCount=0;
	Clipped=false;
	RenderClipped= true;
	OldRenderClipped= true;

	// for Pacs process. By default, false.
	ExcludeFromRefineAll= false;

	// Init Passes.
	// DO NOT FILL Patch here, because of operator= problem. do it in compile().
	// By default, RdrPasses are NULL.

	// To force computation of texture info on next preRender().
	Far0= -1;
	Far1= -1;

	// Default: not binded.
	_BindZoneNeighbor[0]= NULL;
	_BindZoneNeighbor[1]= NULL;
	_BindZoneNeighbor[2]= NULL;
	_BindZoneNeighbor[3]= NULL;
	NoiseRotation= 0;
	// No smooth by default.
	_CornerSmoothFlag= 0;

	// MasterBlock never clipped.
	MasterBlock.resetClip();

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
		// First, delete the VB if the zone was removed while the patch is visible.
		if(!RenderClipped)
		{
			// release VertexBuffer.
			deleteVBAndFaceVector();

			// Flag.
			RenderClipped= true;
		}

		// THIS PATCH MSUT BE UNBOUND FIRST!!!!!
		nlassert(Son0 && Son1);
		nlassert(Son0->isLeaf() && Son1->isLeaf());
		nlassert(Son0->FLeft == NULL);
		nlassert(Son0->FRight == NULL);
		nlassert(Son1->FLeft == NULL);
		nlassert(Son1->FRight == NULL);

		// Free renderPass of landscape, and maybe force computation of texture info on next preRender().
		// Must do it here, before deletion of Zone, OrderS/T etc...
		resetRenderFar();

		getLandscape()->deleteTessFace(Son0);
		getLandscape()->deleteTessFace(Son1);
		// Vertices are smartptr/deleted in zone.
	}

	// Flag the fact that this patch can't be rendered.
	Pass0.Patch= NULL;
	Pass1.Patch= NULL;
	OrderS=0;
	OrderT=0;
	Son0=NULL;
	Son1=NULL;
	clearTessBlocks();
	resetMasterBlock();
	Clipped=false;
	RenderClipped= true;
	OldRenderClipped= true;

	// the pathc is uncompiled. must do it after clearTessBlocks(), because may use it 
	// for vegetable manager
	Zone= NULL;
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

	// \todo yoyo: TODO_NOISE: modulate this value with tangents (roundiness of patch), and with the displacement map.
	ErrorSize= ((v1 - v0)^(v2 - v0)).norm();

}



// ***************************************************************************
void			CPatch::buildBBoxFromBezierPatch(const CBezierPatch &p, CAABBox &ret) const
{
	// Because of the structure of CAABBox, extend() is not fast enough for us. first compute bmin, bmax,
	// then compute the bbox.
	CVector		bmin= p.Vertices[0];
	CVector		bmax= bmin; 

	sint			i;
	for(i=0;i<4;i++)
	{
		bmin.minof(bmin, p.Vertices[i]);
		bmax.maxof(bmax, p.Vertices[i]);
	}
	for(i=0;i<8;i++)
	{
		bmin.minof(bmin, p.Tangents[i]);
		bmax.maxof(bmax, p.Tangents[i]);
	}
	for(i=0;i<4;i++)
	{
		bmin.minof(bmin, p.Interiors[i]);
		bmax.maxof(bmax, p.Interiors[i]);
	}

	// Modulate with the maximum displacement map (usefull for patch clipping).
	static	CVector		vectorNoiseMax(NL3D_NOISE_MAX, NL3D_NOISE_MAX, NL3D_NOISE_MAX);
	bmin-= vectorNoiseMax;
	bmax+= vectorNoiseMax;
	// NB: this is not very optimal, since the BBox may be very too big. eg: patch 16mx16m => bbox 18mx18m.
	// But remind that tessblocks do not use this BBox, and are computed with the real geometry.

	ret.setMinMax(bmin, bmax);
}


// ***************************************************************************
CAABBox			CPatch::buildBBox() const
{
	CBezierPatch	&p= *unpackIntoCache();

	// Compute Bounding Box. (easiest way...)
	CAABBox		ret;
	buildBBoxFromBezierPatch(p, ret);

	return ret;
}


// ***************************************************************************
void		CPatch::addTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const
{
	CBezierPatch	&bpatch= *unpackIntoCache();

	// call with the whole root patch.
	addTrianglesInBBoxRecurs(paId, bbox, triangles, tileTessLevel, bpatch, 0, OrderS, 0, OrderT);
}


// ***************************************************************************
void		CPatch::addTrianglesInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel, 
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const
{
	uint8	lenS=s1-s0, lenT=t1-t0;
	nlassert(lenS>0);
	nlassert(lenT>0);

	// compute and compare bbox of the subdivision patch against request bbox.
	//========================
	// NB: this compute includes possible noise.
	CAABBox		paBBox;
	buildBBoxFromBezierPatch(pa, paBBox);
	// if do not intersect, stop here.
	if( !paBBox.intersect(bbox) )
		return;
	// else if at tile level, then just computeTriangles.
	//========================
	else if( lenS==1 && lenT==1 )
	{
		addTileTrianglesInBBox(paId, bbox, triangles, tessLevel, s0, t0);
	}
	// else subdiv and reccurs.
	//========================
	else
	{
		// Subdivide along the bigger side.
		if(lenS>lenT)
		{
			// subdivide.
			CBezierPatch	left, right;
			pa.subdivideS(left, right);
			uint8	sMiddle= (uint8)( ((uint)s0+(uint)s1) /2 );
			// recurs left.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, left, s0, sMiddle, t0, t1);
			// recurs right.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, right, sMiddle, s1, t0, t1);
		}
		else
		{
			// subdivide.
			CBezierPatch	top, bottom;
			pa.subdivideT(top, bottom);
			uint8	tMiddle= (uint8)( ((uint)t0+(uint)t1) /2 );
			// recurs top.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, top, s0, s1, t0, tMiddle);
			// recurs bottom.
			addTrianglesInBBoxRecurs(paId, bbox, triangles, tessLevel, bottom, s0, s1, tMiddle, t1);
		}
	}


}


// ***************************************************************************
void		CPatch::addTileTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel, uint8 s0, uint8 t0) const
{
	nlassert(s0<OrderS);
	nlassert(t0<OrderT);
	nlassert(tessLevel<=2);
	uint	tessLen= 1<<tessLevel;

	// some preca.
	float	startS0= (float)s0 / (float)(OrderS);
	float	startT0= (float)t0 / (float)(OrderT);
	float	ds= 1.0f/(float)(OrderS*tessLen);
	float	dt= 1.0f/(float)(OrderT*tessLen);

	// Parse all quads.
	uint	sl,tl;
	for(tl=0; tl<tessLen; tl++)
	{
		float	fs0, fs1, ft0, ft1;
		// compute t patch coordinates.
		ft0= startT0 + (float)tl * dt ;
		ft1= ft0 + dt;
		for(sl=0; sl<tessLen; sl++)
		{
			// compute s patch coordinates.
			fs0= startS0 + (float)sl * ds ;
			fs1= fs0 + ds;

			// Compute Quad vectors (in CCW).
			CVector		p0, p1, p2, p3;
			CUV			uv0, uv1, uv2, uv3;
			uv0.U= fs0; uv0.V= ft0;
			uv1.U= fs0; uv1.V= ft1;
			uv2.U= fs1; uv2.V= ft1;
			uv3.U= fs1; uv3.V= ft0;
			// evaluate patch (with noise). (NB: because of cache, patch decompression cost nothing).
			p0= computeVertex(uv0.U, uv0.V);
			p1= computeVertex(uv1.U, uv1.V);
			p2= computeVertex(uv2.U, uv2.V);
			p3= computeVertex(uv3.U, uv3.V);

			// build the bbox of this quad, and test with request bbox.
			CAABBox		quadBBox;
			quadBBox.setCenter(p0);
			quadBBox.extend(p1);
			quadBBox.extend(p2);
			quadBBox.extend(p3);

			// insert only if intersect with the bbox.
			if(quadBBox.intersect(bbox))
			{
				// build triangles (in CCW).
				CTrianglePatch	tri;
				tri.PatchId= paId;

				// first tri.
				tri.V0= p0; tri.V1= p1; tri.V2= p2;
				tri.Uv0= uv0; tri.Uv1= uv1; tri.Uv2= uv2;
				triangles.push_back(tri);

				// second tri.
				tri.V0= p2; tri.V1= p3; tri.V2= p0;
				tri.Uv0= uv2; tri.Uv1= uv3; tri.Uv2= uv0;
				triangles.push_back(tri);

				// NB: this is not the same tesselation than in tesselation.cpp.
				// But this looks like Ben's NLPACS::CLocalRetriever tesselation.
			}
		}
	}

}



// ***************************************************************************
void		CPatchQuadBlock::buildTileTriangles(uint8 quadId, CTrianglePatch  triangles[2]) const
{
	// copute coordinate of the tile we want.
	uint	sd0= quadId&(NL_PATCH_BLOCK_MAX_QUAD-1);
	uint	td0= quadId/(NL_PATCH_BLOCK_MAX_QUAD);
	uint	sd1= sd0+1;
	uint	td1= td0+1;
	uint	s= PatchBlockId.S0+sd0;
	uint	t= PatchBlockId.T0+td0;
	nlassert(s<PatchBlockId.S1);
	nlassert(t<PatchBlockId.T1);

	// Compute UV coord.
	float	fs0= (float)s / (float)(PatchBlockId.OrderS);
	float	ft0= (float)t / (float)(PatchBlockId.OrderT);
	float	fs1= (float)(s+1) / (float)(PatchBlockId.OrderS);
	float	ft1= (float)(t+1) / (float)(PatchBlockId.OrderT);
	CUV			uv0, uv1, uv2, uv3;
	uv0.U= fs0; uv0.V= ft0;
	uv1.U= fs0; uv1.V= ft1;
	uv2.U= fs1; uv2.V= ft1;
	uv3.U= fs1; uv3.V= ft0;

	// get vertex coord.
	const CVector	&p0= Vertices[sd0 + td0*NL_PATCH_BLOCK_MAX_VERTEX];
	const CVector	&p1= Vertices[sd0 + td1*NL_PATCH_BLOCK_MAX_VERTEX];
	const CVector	&p2= Vertices[sd1 + td1*NL_PATCH_BLOCK_MAX_VERTEX];
	const CVector	&p3= Vertices[sd1 + td0*NL_PATCH_BLOCK_MAX_VERTEX];

	// build triangles.
	// first tri.
	{
		CTrianglePatch &tri= triangles[0];
		tri.PatchId= PatchBlockId.PatchId;
		tri.V0= p0; tri.V1= p1; tri.V2= p2;
		tri.Uv0= uv0; tri.Uv1= uv1; tri.Uv2= uv2;
	}

	// second tri.
	{
		CTrianglePatch &tri= triangles[1];
		tri.PatchId= PatchBlockId.PatchId;
		tri.V0= p2; tri.V1= p3; tri.V2= p0;
		tri.Uv0= uv2; tri.Uv1= uv3; tri.Uv2= uv0;
	}

}


// ***************************************************************************
void		CPatch::fillPatchQuadBlock(CPatchQuadBlock &quadBlock)  const
{
	CPatchBlockIdent	&pbId= quadBlock.PatchBlockId;
	uint	lenS= pbId.S1-pbId.S0;
	uint	lenT= pbId.T1-pbId.T0;
	nlassert( pbId.OrderS==OrderS );
	nlassert( pbId.OrderT==OrderT );
	nlassert( pbId.S1<=OrderS );
	nlassert( pbId.T1<=OrderT );
	nlassert( pbId.S0<pbId.S1 );
	nlassert( pbId.T0<pbId.T1 );
	nlassert( lenS<=NL_PATCH_BLOCK_MAX_QUAD );
	nlassert( lenT<=NL_PATCH_BLOCK_MAX_QUAD );

	// Fill vertices.
	uint	s0= pbId.S0;
	uint	t0= pbId.T0;
	// some preca.
	float	startS0= (float)s0 / (float)(OrderS);
	float	startT0= (float)t0 / (float)(OrderT);
	float	ds= 1.0f/(float)(OrderS);
	float	dt= 1.0f/(float)(OrderT);

	// Parse all quads vertices corner.
	uint	sl,tl;
	for(tl=0; tl<lenT+1; tl++)
	{
		float	fs, ft;
		// compute t patch coordinates.
		ft= startT0 + (float)tl * dt ;
		for(sl=0; sl<lenS+1; sl++)
		{
			// compute s patch coordinates.
			fs= startS0 + (float)sl * ds ;
			quadBlock.Vertices[sl + tl*NL_PATCH_BLOCK_MAX_VERTEX]= computeVertex(fs, ft);
		}
	}

}


// ***************************************************************************
void		CPatch::addPatchBlocksInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds) const
{
	CBezierPatch	&bpatch= *unpackIntoCache();

	// call with the whole root patch.
	addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, bpatch, 0, OrderS, 0, OrderT);
}


// ***************************************************************************
void		CPatch::addPatchBlocksInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds, 
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const
{
	uint8	lenS=s1-s0, lenT=t1-t0;
	nlassert(lenS>0);
	nlassert(lenT>0);

	// compute and compare bbox of the subdivision patch against request bbox.
	//========================
	// NB: this compute includes possible noise.
	CAABBox		paBBox;
	buildBBoxFromBezierPatch(pa, paBBox);
	// if do not intersect, stop here.
	if( !paBBox.intersect(bbox) )
		return;
	// else if at CPatchQuadBlock tile level, then just add this Id.
	//========================
	else if( lenS<=NL_PATCH_BLOCK_MAX_QUAD && lenT<=NL_PATCH_BLOCK_MAX_QUAD )
	{
		// Add this PatchBlock desctiptor to the list.
		CPatchBlockIdent	pbId;
		// Fill struct from this and result of recursion.
		pbId.PatchId= paId;
		pbId.OrderS= OrderS;
		pbId.OrderT= OrderT;
		pbId.S0= s0;
		pbId.S1= s1;
		pbId.T0= t0;
		pbId.T1= t1;
		// Add to list.
		paBlockIds.push_back(pbId);
	}
	// else subdiv and reccurs.
	//========================
	else
	{
		// Subdivide along the bigger side.
		if(lenS>lenT)
		{
			// subdivide.
			CBezierPatch	left, right;
			pa.subdivideS(left, right);
			uint8	sMiddle= (uint8)( ((uint)s0+(uint)s1) /2 );
			// recurs left.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, left, s0, sMiddle, t0, t1);
			// recurs right.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, right, sMiddle, s1, t0, t1);
		}
		else
		{
			// subdivide.
			CBezierPatch	top, bottom;
			pa.subdivideT(top, bottom);
			uint8	tMiddle= (uint8)( ((uint)t0+(uint)t1) /2 );
			// recurs top.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, top, s0, s1, t0, tMiddle);
			// recurs bottom.
			addPatchBlocksInBBoxRecurs(paId, bbox, paBlockIds, bottom, s0, s1, tMiddle, t1);
		}
	}

}


// ***************************************************************************
CVector		CPatch::getTesselatedPos(CUV uv) const
{
	// clamp values.
	clamp(uv.U, 0, 1);
	clamp(uv.V, 0, 1);
	// recurs down the 2 sons.
	CVector		ret= CVector::Null;
	Son0->getTesselatedPos(uv, true, ret);
	Son1->getTesselatedPos(uv, true, ret);

	return ret;
}


// ***************************************************************************
// ***************************************************************************
// RENDER LIST.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CPatch::addRefTessBlocks()
{
	uint	i;

	TessBlockRefCount++;
	if(TessBlocks.size()==0)
	{
		// Allocate the tessblocks.
		//==========

		nlassert(NL3D_TESSBLOCK_TILESIZE==4);
		// A tessblock is 2*2 tiles.
		sint os= OrderS>>1;
		sint ot= OrderT>>1;
		nlassert(os>0);
		nlassert(ot>0);
		TessBlocks.resize(os*ot);
		// init all tessBlocks with the Patch ptr.
		for(i=0; i<TessBlocks.size(); i++)
			TessBlocks[i].init(this);



		// Vegetable management
		//==========
		CVegetableManager	*vegetableManager= getLandscape()->_VegetableManager;

		// Create ClipBlocks.
		uint	nTbPerCb= NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK;
		uint	wCB= (os + nTbPerCb-1) >> NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK_SHIFT;
		uint	hCB= (ot + nTbPerCb-1) >> NL3D_PATCH_VEGETABLE_NUM_TESSBLOCK_PER_CLIPBLOCK_SHIFT;
		VegetableClipBlocks.resize(wCB * hCB);
		// allocate ClipBlocks
		for(i=0; i<VegetableClipBlocks.size(); i++)
		{
			VegetableClipBlocks[i]= vegetableManager->createClipBlock();
		}

	}
}

// ***************************************************************************
void			CPatch::decRefTessBlocks()
{
	TessBlockRefCount--;
	// If no loinger need the tessblocks, delete them.
	if(TessBlockRefCount==0)
		clearTessBlocks();
	nlassert(TessBlockRefCount>=0);
}


// ***************************************************************************
void			CPatch::clearTessBlocks()
{
	uint	i;

	// Vegetable management
	//==========
	// if compiled.
	if(Zone)
	{
		CVegetableManager	*vegetableManager= getLandscape()->_VegetableManager;

		// delete still existing vegetable Igs.
		deleteAllVegetableIgs();

		// delete ClipBlocks.
		for(i=0; i<VegetableClipBlocks.size(); i++)
		{
			vegetableManager->deleteClipBlock(VegetableClipBlocks[i]);
		}
		contReset(VegetableClipBlocks);
	}


	// Delete TessBlocks
	//==========
	TessBlockRefCount=0;
	contReset(TessBlocks);
}


// ***************************************************************************
void			CPatch::resetMasterBlock()
{
	// We should be not visible so FaceVector no more exist.
	nlassert(RenderClipped);

	MasterBlock.FarVertexList.clear();
	MasterBlock.FarFaceList.clear();
	// no tiles should be here!!
	nlassert(MasterBlock.NearVertexList.size()==0);
}

// ***************************************************************************
uint			CPatch::getNumTessBlock(CTessFace *face)
{
	// To which tessBlocks link the face?
	// compute an approx middle of the face.
	CParamCoord	edgeMid(face->PVLeft, face->PVRight);
	CParamCoord	middle(edgeMid, face->PVBase);
	// Coordinate of the tessblock (2*2 a tile!! so the >>1).
	uint ts= ((uint)middle.S * (uint)(OrderS>>1)) / 0x8000;
	uint tt= ((uint)middle.T * (uint)(OrderT>>1)) / 0x8000;
	uint numtb= tt*(uint)(OrderS>>1) + ts;

	return numtb;
}


// ***************************************************************************
void			CPatch::getNumTessBlock(CParamCoord pc, TFarVertType &type, uint &numtb)
{
	uint	tboS= (uint)(OrderS>>1);
	uint	tboT= (uint)(OrderT>>1);

	// Coordinate of the tessblock (2*2 a tile!! so the >>1).
	uint ts= ((uint)pc.S * tboS) / 0x8000;
	uint tt= ((uint)pc.T * tboT) / 0x8000;
	numtb= tt*tboS + ts;

	bool	edgeS= (ts*0x8000) == ((uint)pc.S * tboS);
	bool	edgeT= (tt*0x8000) == ((uint)pc.T * tboT);

	// Does this vertex lies on a corner of a TessBlock?
	if(edgeS && edgeT)
		type= FVMasterBlock;
	// Does this vertex lies on a edge of a TessBlock?
	else if(edgeS || edgeT)
		type= FVTessBlockEdge;
	// Else it lies exclusively IN a TessBlock.
	else
		type= FVTessBlock;

}



// ***************************************************************************
void			CPatch::extendTessBlockWithEndPos(CTessFace *face)
{
	if(face->Level>=TessBlockLimitLevel)
	{
		// get the tessBlock of the face.
		uint	numtb= getNumTessBlock(face);

		// Must enlarge the BSphere of the tesblock!!
		TessBlocks[numtb].extendSphere(face->VBase->EndPos);
		TessBlocks[numtb].extendSphere(face->VLeft->EndPos);
		TessBlocks[numtb].extendSphere(face->VRight->EndPos);
	}
}


// ***************************************************************************
void			CPatch::dirtTessBlockFaceVector(CTessBlock &tb)
{
	// If patch is visible, block's faceVector should exist, but are no more valid.
	if(!RenderClipped)
	{
		// If this tessBlock not already notified to modification.
		if(!tb.isInModifyList())
		{
			// Then append, and delete all FaceVector.
			// NB: delete FaceVector now, because the TessBlock himself may disapear soon.
			tb.appendToModifyListAndDeleteFaceVector(getLandscape()->_TessBlockModificationRoot, getLandscape()->_FaceVectorManager);
		}
	}
}


// ***************************************************************************
void			CPatch::appendFaceToRenderList(CTessFace *face)
{
	// Update Gnal render.
	//====================
	if(face->Level<TessBlockLimitLevel)
	{
		MasterBlock.FarFaceList.append(face);
		MasterBlock.FaceTileMaterialRefCount++;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(MasterBlock);
	}
	else
	{
		// Alloc if necessary the TessBlocks.
		addRefTessBlocks();

		// link the face to the good tessblock.
		uint	numtb= getNumTessBlock(face);
		TessBlocks[numtb].FarFaceList.append(face);
		TessBlocks[numtb].FaceTileMaterialRefCount++;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(TessBlocks[numtb]);

		// Must enlarge the BSphere of the tesblock!!
		// We must do it on a per-face approach, because of tessblocks 's corners which are outside of tessblocks.
		TessBlocks[numtb].extendSphere(face->VBase->EndPos);
		TessBlocks[numtb].extendSphere(face->VLeft->EndPos);
		TessBlocks[numtb].extendSphere(face->VRight->EndPos);
		// I think this should be done too on StartPos, for geomorph (rare??...) problems.
		// \todo yoyo: is this necessary???
		TessBlocks[numtb].extendSphere(face->VBase->StartPos);
		TessBlocks[numtb].extendSphere(face->VLeft->StartPos);
		TessBlocks[numtb].extendSphere(face->VRight->StartPos);


		// Update Tile render (no need to do it if face not at least at tessblock level).
		appendFaceToTileRenderList(face);
	}
}


// ***************************************************************************
void			CPatch::removeFaceFromRenderList(CTessFace *face)
{
	// Update Gnal render.
	//====================
	if(face->Level<TessBlockLimitLevel)
	{
		MasterBlock.FarFaceList.remove(face);
		MasterBlock.FaceTileMaterialRefCount--;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(MasterBlock);
	}
	else
	{
		// link the face to the good tessblock.
		uint	numtb= getNumTessBlock(face);
		TessBlocks[numtb].FarFaceList.remove(face);
		TessBlocks[numtb].FaceTileMaterialRefCount--;

		// The facelist is modified, so we must update the faceVector, if visible.
		dirtTessBlockFaceVector(TessBlocks[numtb]);

		// Update Tile render (no need to do it if face not at least at tessblock level).
		removeFaceFromTileRenderList(face);

		// Destroy if necessary the TessBlocks.
		decRefTessBlocks();
	}
}


// ***************************************************************************
void			CPatch::appendFaceToTileRenderList(CTessFace *face)
{
	if(face->TileMaterial)
	{
		// For all valid faces, update their links.
		// Do not do this for lightmap, since it use same face from RGB0 pass.
		for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
		{
			CPatchRdrPass	*tilePass= face->TileMaterial->Pass[i].PatchRdrPass;
			// If tile i enabled.
			if(tilePass)
			{
				// a face should have created for this pass.
				nlassert(face->TileFaces[i]);
				face->TileMaterial->TileFaceList[i].append(face->TileFaces[i]);
			}
		}

		// The facelist is modified, so we must update the faceVector, if visible.
		uint	numtb= getNumTessBlock(face);
		dirtTessBlockFaceVector(TessBlocks[numtb]);
	}
}


// ***************************************************************************
void			CPatch::removeFaceFromTileRenderList(CTessFace *face)
{
	if(face->TileMaterial)
	{
		// For all valid faces, update their links.
		// Do not do this for lightmap, since it use same face from RGB0 pass.
		for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
		{
			CPatchRdrPass	*tilePass= face->TileMaterial->Pass[i].PatchRdrPass;
			// If tile i enabled.
			if(tilePass)
			{
				// a face should have created for this pass.
				nlassert(face->TileFaces[i]);
				face->TileMaterial->TileFaceList[i].remove(face->TileFaces[i]);
			}
		}

		// The facelist is modified, so we must update the faceVector, if visible.
		uint	numtb= getNumTessBlock(face);
		dirtTessBlockFaceVector(TessBlocks[numtb]);
	}
}


// ***************************************************************************
void			CPatch::computeTbTm(uint &numtb, uint &numtm, uint ts, uint tt)
{
	sint	is= ts&1;
	sint	it= tt&1;
	ts>>=1;
	tt>>=1;
	
	numtb= tt*(uint)(OrderS>>1) + ts;
	numtm= it*2+is;
}


// ***************************************************************************
void			CPatch::appendTileMaterialToRenderList(CTileMaterial *tm)
{
	nlassert(tm);

	// Alloc if necessary the TessBlocks.
	addRefTessBlocks();

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tm->TileS, tm->TileT);
	TessBlocks[numtb].RdrTileRoot[numtm]= tm;
	TessBlocks[numtb].FaceTileMaterialRefCount++;
	TessBlocks[numtb].TileMaterialRefCount++;

	// if was no tiles before in this tessBlock, create a Vegetable block.
	//==========
	// one Tile <=> was 0 before
	if( TessBlocks[numtb].TileMaterialRefCount == 1 && getLandscape()->isVegetableActive() )
	{
		createVegetableBlock(numtb, tm->TileS, tm->TileT);
	}
}
// ***************************************************************************
void			CPatch::removeTileMaterialFromRenderList(CTileMaterial *tm)
{
	nlassert(tm);

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tm->TileS, tm->TileT);
	TessBlocks[numtb].RdrTileRoot[numtm]= NULL;
	TessBlocks[numtb].FaceTileMaterialRefCount--;
	TessBlocks[numtb].TileMaterialRefCount--;

	// if no more tiles in this tessBlock, delete the vegetable Block.
	//==========
	// if no more tiles in this tessBlock
	if( TessBlocks[numtb].TileMaterialRefCount==0 )
	{
		// release the vegetableBlock (if any)
		releaseVegetableBlock(numtb);
	}

	// Destroy if necessary the TessBlocks.
	decRefTessBlocks();
}


// ***************************************************************************
void			CPatch::appendFarVertexToRenderList(CTessFarVertex *fv)
{
	TFarVertType	type;
	uint			numtb;
	getNumTessBlock(fv->PCoord, type, numtb);
	
	
	if(type==FVMasterBlock || type==FVTessBlockEdge)
	{
		fv->OwnerBlock= &MasterBlock;
		MasterBlock.FarVertexList.append(fv);
	}
	else 
	{
		// Alloc if necessary the TessBlocks.
		addRefTessBlocks();

		fv->OwnerBlock= &TessBlocks[numtb];
		TessBlocks[numtb].FarVertexList.append(fv);
	}
}
// ***************************************************************************
void			CPatch::removeFarVertexFromRenderList(CTessFarVertex *fv)
{
	TFarVertType	type;
	uint			numtb;
	getNumTessBlock(fv->PCoord, type, numtb);
	
	
	if(type==FVMasterBlock || type==FVTessBlockEdge)
	{
		MasterBlock.FarVertexList.remove(fv);
		fv->OwnerBlock= NULL;
	}
	else 
	{
		TessBlocks[numtb].FarVertexList.remove(fv);
		fv->OwnerBlock= NULL;

		// Destroy if necessary the TessBlocks.
		decRefTessBlocks();
	}
}


// ***************************************************************************
void			CPatch::appendNearVertexToRenderList(CTileMaterial *tileMat, CTessNearVertex *nv)
{
	nlassert(tileMat);

	// Alloc if necessary the TessBlocks.
	addRefTessBlocks();

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tileMat->TileS, tileMat->TileT);
	nv->OwnerBlock= &TessBlocks[numtb];
	TessBlocks[numtb].NearVertexList.append(nv);
}
// ***************************************************************************
void			CPatch::removeNearVertexFromRenderList(CTileMaterial *tileMat, CTessNearVertex *nv)
{
	nlassert(tileMat);

	uint	numtb, numtm;
	computeTbTm(numtb, numtm, tileMat->TileS, tileMat->TileT);
	TessBlocks[numtb].NearVertexList.remove(nv);
	nv->OwnerBlock= NULL;

	// Destroy if necessary the TessBlocks.
	decRefTessBlocks();
}



// ***************************************************************************
// ***************************************************************************
// BASIC BUILD.
// ***************************************************************************
// ***************************************************************************




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

	// Init Far vetices.
	CTessFarVertex *fa= &BaseFarVertices[0];
	CTessFarVertex *fb= &BaseFarVertices[1];
	CTessFarVertex *fc= &BaseFarVertices[2];
	CTessFarVertex *fd= &BaseFarVertices[3];
	fa->Src= a;
	fa->PCoord.setST(0,0);
	fb->Src= b;
	fb->PCoord.setST(0,1);
	fc->Src= c;
	fc->PCoord.setST(1,1);
	fd->Src= d;
	fd->PCoord.setST(1,0);

	// We don't have to fill the Far vertices VB here, because this patch is still not visible.
	// NB: we can't because we don't have any driver here.
	nlassert(RenderClipped==true);


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
	Son0= getLandscape()->newTessFace();
	Son1= getLandscape()->newTessFace();

	// Son0.
	Son0->Patch= this;
	Son0->Level= 0;
	if(OrderS>=OrderT)
	{
		Son0->VBase= b;
		Son0->VLeft= c;
		Son0->VRight= a;
		Son0->FVBase= fb;
		Son0->FVLeft= fc;
		Son0->FVRight= fa;
		Son0->PVBase.setST(0, 1);
		Son0->PVLeft.setST(1, 1);
		Son0->PVRight.setST(0, 0);
	}
	else
	{
		Son0->VBase= a;
		Son0->VLeft= b;
		Son0->VRight= d;
		Son0->FVBase= fa;
		Son0->FVLeft= fb;
		Son0->FVRight= fd;
		Son0->PVBase.setST(0, 0);
		Son0->PVLeft.setST(0, 1);
		Son0->PVRight.setST(1, 0);
	}
	Son0->FBase= Son1;
	Son0->FLeft= NULL;
	Son0->FRight= NULL;
	// No tile info.
	Son0->Size= ErrorSize/2;
	Son0->computeSplitPoint();

	// Son1.
	Son1->Patch= this;
	Son1->Level= 0;
	if(OrderS>=OrderT)
	{
		Son1->VBase= d;
		Son1->VLeft= a;
		Son1->VRight= c;
		Son1->FVBase= fd;
		Son1->FVLeft= fa;
		Son1->FVRight= fc;
		Son1->PVBase.setST(1, 0);
		Son1->PVLeft.setST(0, 0);
		Son1->PVRight.setST(1, 1);
	}
	else
	{
		Son1->VBase= c;
		Son1->VLeft= d;
		Son1->VRight= b;
		Son1->FVBase= fc;
		Son1->FVLeft= fd;
		Son1->FVRight= fb;
		Son1->PVBase.setST(1, 1);
		Son1->PVLeft.setST(1, 0);
		Son1->PVRight.setST(0, 1);
	}
	Son1->FBase= Son0;
	Son1->FLeft= NULL;
	Son1->FRight= NULL;
	// No tile info.
	Son1->Size= ErrorSize/2;
	Son1->computeSplitPoint();


	// Prepare the render list...
	clearTessBlocks();
	resetMasterBlock();
	appendFarVertexToRenderList(fa);
	appendFarVertexToRenderList(fb);
	appendFarVertexToRenderList(fc);
	appendFarVertexToRenderList(fd);
	appendFaceToRenderList(Son0);
	appendFaceToRenderList(Son1);

	// Usefull for geomorph: Init 2 root faces MaxNearLimit, and MaxFaceSize
	// NB: since no geomorph is made on endpoints (StartPos==EndPos) of patchs, this is not usefull.
	// but it is important to ensure the VP or software geomorph won't crash with bad float values.
	// Init MaxFaceSize.
	Son0->VBase->MaxFaceSize= 1;
	Son0->VLeft->MaxFaceSize= 1;
	Son0->VRight->MaxFaceSize= 1;
	Son1->VBase->MaxFaceSize= 1;
	Son1->VLeft->MaxFaceSize= 1;
	Son1->VRight->MaxFaceSize= 1;
	// Init MaxNearLimit.
	Son0->VBase->MaxNearLimit= 1;
	Son0->VLeft->MaxNearLimit= 1;
	Son0->VRight->MaxNearLimit= 1;
	Son1->VBase->MaxNearLimit= 1;
	Son1->VLeft->MaxNearLimit= 1;
	Son1->VRight->MaxNearLimit= 1;

}


// ***************************************************************************
void			CPatch::compile(CZone *z, uint patchId, uint8 orderS, uint8 orderT, CTessVertex *baseVertices[4], float errorSize)
{
	nlassert(z);
	Zone= z;

	// Once the patch is inserted and compiled in a zone, it is ready to be rendered.
	// So now fill the Patch info in render pass.
	Pass0.Patch= this;
	Pass1.Patch= this;

	// init also the MasterBlock.
	MasterBlock.init(this);

	// only 65536 patch per zone allowed.
	nlassert(patchId<0x10000);
	PatchId= (uint16)patchId;

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
	// A TessBlock is a 2*2 tile. This simple formula works because patch 1xX are illegal.
	TessBlockLimitLevel= TileLimitLevel-2;
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
	// \todo yoyo: TODO_UVCORRECT: use UV correction.

	if(getLandscape()->getNoiseMode())
	{
		// compute displacement map to disturb result.
		CVector		displace;
		computeNoise(s,t, displace);

		// return patch(s,t) + dispalcement result.
		// unpack. Do it after computeNoise(), because this last may change the cache.
		CBezierPatch	*patch= unpackIntoCache();
		return patch->eval(s,t) + displace;
	}
	else
	{
		// unpack and return patch(s,t).
		CBezierPatch	*patch= unpackIntoCache();
		return patch->eval(s,t);
	}
}


// ***************************************************************************
void			CPatch::refineAll()
{
	// refineAll.
	nlassert(Son0);
	nlassert(Son1);
	Son0->refineAll();
	Son1->refineAll();
}



// ***************************************************************************
void			CPatch::averageTesselationVertices()
{
	nlassert(Son0);
	nlassert(Son1);

	// Recompute the BaseVertices. This is usefull for Pacs.
	// Because CLandscape::averageTesselationVertices() is made on a strict order for patchs (map of zones, then 
	// array of patchs), we are sure to overwrite BaseVertices in this order.
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];
	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);


	// Average the tesselation of sons.
	Son0->averageTesselationVertices();
	Son1->averageTesselationVertices();
}


// ***************************************************************************
void			CPatch::refreshTesselationGeometry()
{
	nlassert(Son0);
	nlassert(Son1);

	// Recompute the BaseVertices.
	CTessVertex *a= BaseVertices[0];
	CTessVertex *b= BaseVertices[1];
	CTessVertex *c= BaseVertices[2];
	CTessVertex *d= BaseVertices[3];
	// Set positions.
	a->Pos= a->StartPos= a->EndPos= computeVertex(0,0);
	b->Pos= b->StartPos= b->EndPos= computeVertex(0,1);
	c->Pos= c->StartPos= c->EndPos= computeVertex(1,1);
	d->Pos= d->StartPos= d->EndPos= computeVertex(1,0);


	// refresh the tesselation of sons.
	Son0->refreshTesselationGeometry();
	Son1->refreshTesselationGeometry();
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

	// A patch clipped is clipped to render too.
	RenderClipped= Clipped;
}


// ***************************************************************************
void			CPatch::resetRenderFar()
{
	if (Pass0.PatchRdrPass)
	{
		// free the render pass.
		Zone->Landscape->freeFarRenderPass (this, Pass0.PatchRdrPass, Far0);
		Pass0.PatchRdrPass= NULL;
	}
	if (Pass1.PatchRdrPass)
	{
		// free the render pass.
		Zone->Landscape->freeFarRenderPass (this, Pass1.PatchRdrPass, Far1);
		Pass1.PatchRdrPass= NULL;
	}

	Far0= -1;
	Far1= -1;
}


// ***************************************************************************
void			CPatch::serial(NLMISC::IStream &f)
{
	/*
	Version 6:
		- default UnderWater flags for tileElements before version 6.
	Version 5:
		- TileLightInfluences serialized.
	Version 4:
		- Smooth flag serialized
	Version 3:
		- NoiseRotation.
		- NoiseSmooth.
	Version 2:
		- Lumels.
	Version 1:
		- Tile color.
	Version 0:
		- base verison.
	*/
	uint	ver= f.serialVersion(_Version);

	// No more compatibility before version 2, because OrderS / OrderT not serialized in preceding version
	// Doens't matter since CZone::serial() do not support it too.
	if (ver<2)
	{
		throw EOlderStream(f);
	}

	f.xmlSerial (Vertices[0], Vertices[1], Vertices[2], Vertices[3], "VERTICIES");

	f.xmlPush ("TANGENTS");
	f.serial (Tangents[0], Tangents[1], Tangents[2], Tangents[3]);
	f.serial (Tangents[4], Tangents[5], Tangents[6], Tangents[7]);
	f.xmlPop ();
	
	f.xmlSerial (Interiors[0], Interiors[1], Interiors[2], Interiors[3], "INTERIORS");

	f.xmlPush ("TILES");
	f.serialCont(Tiles);
	f.xmlPop ();

	if(ver>=1)
		f.xmlPush ("TILE_COLORS");
		f.serialCont(TileColors);
		f.xmlPop ();
	if(ver>=2)
	{
		f.xmlSerial (OrderS, "ORDER_S");
		f.xmlSerial (OrderT, "ORDER_T");

		f.xmlPush ("COMPRESSED_LUMELS");
			f.serialCont(CompressedLumels);
		f.xmlPop ();
	}
	// Else cannot create here the TileColors, because we need the OrderS/OrderT information... Done into CZone serial.
	if(ver>=3)
	{
		f.xmlSerial (NoiseRotation, "NOISE_ROTATION");
		f.xmlSerial (_CornerSmoothFlag, "CORNER_SMOOTH_FLAG");
	}
	else
	{
		// No Rotation / not smooth by default.
		NoiseRotation= 0;
		_CornerSmoothFlag= 0;
	}
	if(ver>=4)
	{
		f.xmlSerial(Flags, "FLAGS");
	}
	else
	{
		Flags=NL_PATCH_SMOOTH_FLAG_MASK;
	}

	// Serialize TileLightInfluences
	if(ver>=5)
	{
		f.xmlPush ("TILE_LIGHT_INFLUENCES");
		f.serialCont(TileLightInfluences);
		f.xmlPop ();
	}
	else
	{
		if(f.isReading())
		{
			// Fill default.
			resetTileLightInfluences();
		}
	}

	// if read a too old version, 
	if(ver<6 && f.isReading())
	{
		// reset tileElements vegetableState to AboveWater.
		for(uint i=0; i<Tiles.size(); i++)
		{
			Tiles[i].setVegetableState(CTileElement::AboveWater);
		}
	}


}



// ***************************************************************************
// ***************************************************************************
// Bind / UnBind.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void			CPatch::unbind()
{
	nlassert(Son0 && Son1);

	Son0->unbind();
	Son1->unbind();
	Son0->forceMerge();
	Son1->forceMerge();
	// forcemerge should have be completed.
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	// unbind should have be completed.
	nlassert(Son0->FLeft == NULL);
	nlassert(Son0->FRight == NULL);
	nlassert(Son1->FLeft == NULL);
	nlassert(Son1->FRight == NULL);


	// unbind Noise.
	_BindZoneNeighbor[0]= NULL;
	_BindZoneNeighbor[1]= NULL;
	_BindZoneNeighbor[2]= NULL;
	_BindZoneNeighbor[3]= NULL;

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
CTessFace		*CPatch::linkTessFaceWithEdge(const CVector2f &uv0, const CVector2f &uv1, CTessFace *linkTo)
{
	nlassert(Son0 && Son1);
	// Try to link with Root Son0
	CTessFace	*face= Son0->linkTessFaceWithEdge(uv0, uv1, linkTo);
	// if Failed Try to link with Root Son1
	if(!face)
		face= Son1->linkTessFaceWithEdge(uv0, uv1, linkTo);
	return face;
}


// ***************************************************************************
void			CPatch::bind(CBindInfo	Edges[4], bool rebind)
{
	// The multiple Patch Face.
	// By default, Patch==NULL, FLeft, FRight and FBase==NULL so ok!
	static	CTessFace	bind1_2[4];
	static	CTessFace	bind1_4[8];


	// THIS PATCH MUST BE UNBOUND FIRST!!!!!
	nlassert(Son0 && Son1);
	nlassert(Son0->isLeaf() && Son1->isLeaf());
	nlassert(Son0->FLeft == NULL);
	nlassert(Son0->FRight == NULL);
	nlassert(Son1->FLeft == NULL);
	nlassert(Son1->FRight == NULL);


	// bind the Realtime bind info here (before any computeVertex, and before bind too).
	sint	i;
	for(i=0;i<4;i++)
	{
		// just Copy zone (if not NULL).
		_BindZoneNeighbor[i]= Edges[i].Zone;
	}


	if(!rebind)
	{
		// Just recompute base vertices.
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
		Son0->computeSplitPoint();
		Son1->computeSplitPoint();
	}
	else
	{
		// Keep old Vertices as computed from neighbors, but reFill the 4 FarVertices.
		// NB: don't do it on NearVertices because suppose that Near is Off when a bind occurs (often far away).
		checkFillVertexVBFar(Son0->FVBase);
		checkFillVertexVBFar(Son0->FVLeft);
		checkFillVertexVBFar(Son0->FVRight);
		checkFillVertexVBFar(Son1->FVBase);
	}


	// Bind the roots.
	for(i=0;i<4;i++)
	{
		CBindInfo	&bind= Edges[i];

		nlassert(bind.NPatchs==0 || bind.NPatchs==1 || bind.NPatchs==2 || bind.NPatchs==4 || bind.NPatchs==5);
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
		else if(bind.NPatchs==5)
		{
			/* I am binded to a bigger patch and this one has already done the binding on me => 
				It must be correclty tesselated. Note also that bind 1/X are only possible on interior of zone.
			*/

			// First, make the link with the face to which I must connect.
			// -----------------

			// Get the coordinate of the current edge of this patch
			CVector2f	uvi0, uvi1;
			switch(i)
			{
			case 0: uvi0.set(0,0); uvi1.set(0,1);  break;
			case 1: uvi0.set(0,1); uvi1.set(1,1);  break;
			case 2: uvi0.set(1,1); uvi1.set(1,0);  break;
			case 3: uvi0.set(1,0); uvi1.set(0,0);  break;
			};
			// mul by OrderS/OrderT for CPatchUVLocator
			uvi0.x*= OrderS;
			uvi0.y*= OrderT;
			uvi1.x*= OrderS;
			uvi1.y*= OrderT;
			// build a CPatchUVLocator to transpose coorindate ot this edge in coordinate on the bigger Neighbor patch.
			CBindInfo	bindInfo;
			getBindNeighbor(i, bindInfo);
			nlassert(bindInfo.Zone!=NULL && bindInfo.NPatchs==1);
			CPatchUVLocator		puvloc;
			puvloc.build(this, i, bindInfo);

			// transpose from this patch coord in neighbor patch coord.
			CVector2f	uvo0, uvo1;
			uint	pid;
			CPatch	*patchNeighbor;
			// Do it for uvi0
			pid= puvloc.selectPatch(uvi0);
			puvloc.locateUV(uvi0, pid, patchNeighbor, uvo0);
			nlassert(patchNeighbor == bindInfo.Next[0]);
			// Do it for uvi1
			pid= puvloc.selectPatch(uvi1);
			puvloc.locateUV(uvi1, pid, patchNeighbor, uvo1);
			nlassert(patchNeighbor == bindInfo.Next[0]);
			// Rescale to have uv in 0,1 basis.
			uvo0.x/= patchNeighbor->OrderS;
			uvo0.y/= patchNeighbor->OrderT;
			uvo1.x/= patchNeighbor->OrderS;
			uvo1.y/= patchNeighbor->OrderT;

			// Now, traverse the tesselation and find the first CTessFace which use this edge.
			CTessFace	*faceNeighbor;
			faceNeighbor= patchNeighbor->linkTessFaceWithEdge(uvo0, uvo1, this->getRootFaceForEdge(i));
			nlassert(faceNeighbor);
			// Bind me on Next.
			this->changeEdgeNeighbor(i, faceNeighbor);
		}

	}

	// Propagate the binds to sons.
	Son0->updateBind();
	Son1->updateBind();

}


// ***************************************************************************
void			CPatch::forceMergeAtTileLevel()
{
	nlassert(Son0 && Son1);

	Son0->forceMergeAtTileLevel();
	Son1->forceMergeAtTileLevel();
}



// ***************************************************************************
// ***************************************************************************
// Texturing.
// ***************************************************************************
// ***************************************************************************


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
	for(sint tb=0; tb<(sint)TessBlocks.size();tb++)
	{
		// Vertices must all be reseted.
		TessBlocks[tb].NearVertexList.clear();
		for(sint i=0;i<NL3D_TESSBLOCK_TILESIZE;i++)
		{
			CTileMaterial	*tm= TessBlocks[tb].RdrTileRoot[i];
			if(tm)
			{
				for(sint pass=0;pass<NL3D_MAX_TILE_FACE;pass++)
				{
					tm->TileFaceList[pass].clear();
				}
			}
		}
	}

	Son0->recreateTileUvs();
	Son1->recreateTileUvs();
}

// ***************************************************************************
// Precalc table used to decompress shadow map
static const uint NL3DDecompressLumelFactor0Case0[8]=
{
	7, 0, 6, 5, 4, 3, 2, 1
};
static const uint NL3DDecompressLumelFactor1Case0[8]=
{
	0, 7, 1, 2, 3, 4, 5, 6
};
static const uint NL3DDecompressLumelFactor0Case1[6]=
{
	5, 0, 4, 3, 2, 1,
};
static const uint NL3DDecompressLumelFactor1Case1[6]=
{
	0, 5, 1, 2, 3, 4,
};
// ***************************************************************************
void		CPatch::unpackLumelBlock (uint8 *dest, const uint8 *src)
{
	// Take the two alpha values
	uint alpha0=src[0];
	uint alpha1=src[1];

	// To read codes
	sint codeBitOffset=7;
	uint firstBlock=src[2];
	uint secondBlock=src[3];
	src+=4;

	// Uncompress 16 codes
	uint codes[4*4];

	// For each code
	uint lumel;
	for (lumel=0; lumel<16; lumel++)
	{
		// Get the code
		if (codeBitOffset>2)
			codes[lumel]=(firstBlock>>(codeBitOffset-2))&0x7;
		else
			codes[lumel]=((firstBlock<<(2-codeBitOffset))&0x7) | ((secondBlock>>(6+codeBitOffset))&0x7);

		// Next bit
		codeBitOffset-=3;
		if (codeBitOffset<0)
		{
			// Crop bit index
			codeBitOffset&=0x7;

			// Second become first
			firstBlock=secondBlock;

			// New second
			if (lumel<13)
			{
				secondBlock=*src;
				src++;
			}
		}
	}

	// Case 0
	if (alpha0>alpha1)
	{
		// For each lumel
		for (lumel=0; lumel<16; lumel++)
		{
			// Decompress the data
			uint code=codes[lumel];
			dest[lumel]=(uint8)((NL3DDecompressLumelFactor0Case0[code]*alpha0+NL3DDecompressLumelFactor1Case0[code]*alpha1)/7);
		}
	}
	// Case 1
	else
	{
		// For each lumel
		for (lumel=0; lumel<16; lumel++)
		{
			// Decompress the data
			uint code=codes[lumel];
			if (code<6)
				dest[lumel]=(uint8)((NL3DDecompressLumelFactor0Case1[code]*alpha0+NL3DDecompressLumelFactor1Case1[code]*alpha1)/5);
			else if (code==6)
				dest[lumel]=0;
			else if (code==7)
				dest[lumel]=255;
		}
	}
}

// ***************************************************************************
inline uint8 getUnpackLumelBlock (const uint8 *src, uint pixel)
{
	// Offset of the bit
	pixel*=3;
	uint offset=(pixel>>3)+2;
	uint bits=pixel&7;

	// Uncompress 16 codes
	uint code;

	// Get the code
	if (bits<=5)
		code=(src[offset]>>(5-bits))&0x7;
	else
		code= ( (src[offset]<<(bits-5)) | (src[offset+1]>>(13-bits)) )&0x7;

	// Case 0
	if (src[0]>src[1])
	{
		// Decompress the data
		return (uint8)((NL3DDecompressLumelFactor0Case0[code]*src[0]+NL3DDecompressLumelFactor1Case0[code]*src[1])/7);
	}
	// Case 1
	else
	{
		// Decompress the data
		if (code<6)
			return (uint8)((NL3DDecompressLumelFactor0Case1[code]*src[0]+NL3DDecompressLumelFactor1Case1[code]*src[1])/5);
		else if (code==6)
			return 0;
		else
			return 255;
	}
}

// ***************************************************************************
void		CPatch::unpackShadowMap (uint8 *pLumelDest)
{
	// Input of compresed data
	uint8 *compressedData=&CompressedLumels[0];

	// Number of lumel by lines
	uint lumelCount=OrderS*NL_LUMEL_BY_TILE;

	// Number of block in a line
	nlassert ((lumelCount&0x3)==0);
	uint numLumelBlock=lumelCount>>2;

	// Number of line
	uint lineCount=OrderT*NL_LUMEL_BY_TILE;

	// Number of block line
	nlassert ((lineCount&0x3)==0);
	uint numLineBlock=lineCount>>2;

	// Destination lumel block size
	uint lumelDestBlockSize=4;

	// Destination lumel line block size
	uint lumelDestLineBlockSize=lumelCount*lumelDestBlockSize;

	// Each line block
	for (uint lineBlock=0; lineBlock<numLineBlock; lineBlock++)
	{
		uint countVx4=16;

		// Block pointer
		uint8 *blockLine=pLumelDest;

		// Each lumel block
		for (uint lumelBlock=0; lumelBlock<numLumelBlock; lumelBlock++)
		{
			// *** Unpack the block
			uint countU=4;

			// Destination lumel
			uint8 *blockDest=blockLine;

			// Temp block
			uint8 block[4*4];

			// Block unpacking...
			unpackLumelBlock (block, compressedData);

			// Copy the lumels
			for (uint v=0; v<countVx4; v+=4)
			{
				for (uint u=0; u<countU; u++)
				{
					// Copy the lumel
					blockDest[u]=block[v+u];
				}

				// Next line
				blockDest+=lumelCount;
			}

			// Next source block
			compressedData+=NL_BLOCK_LUMEL_COMPRESSED_SIZE;

			// Next block on the line
			blockLine+=lumelDestBlockSize;
		}

		// Next line of block
		pLumelDest+=lumelDestLineBlockSize;
	}
}

// ***************************************************************************
uint		CPatch::evalLumelBlock (const uint8 *original, const uint8 *unCompressed, uint width, uint height)
{
	// Sum
	uint sum=0;

	// Eval error for each..
	for (uint v=0; v<height; v++)
	for (uint u=0; u<width; u++)
	{
		sum += abs((sint)original[v*4+u]-(sint)unCompressed[v*4+u]);
	}

	// return the sum
	return sum;
}

// ***************************************************************************
void		CPatch::packLumelBlock (uint8 *dest, const uint8 *source, uint8 alpha0, uint8 alpha1)
{
	// Precalc the height values..
	uint8 value[8];

	// For each value
	uint i;
	for (i=0; i<8; i++)
	{
		// Case 0 or 1 ?
		if (alpha0>alpha1)
			// Case 0
			value[i]=(NL3DDecompressLumelFactor0Case0[i]*alpha0+NL3DDecompressLumelFactor1Case0[i]*alpha1)/7;
		else
		{
			if (i<6)
				value[i]=(NL3DDecompressLumelFactor0Case1[i]*alpha0+NL3DDecompressLumelFactor1Case1[i]*alpha1)/5;
			else if (i==6)
				value[i]=0;
			else
				value[i]=255;
		}
	}

	// Store alpha value
	dest[0]=alpha0;
	dest[1]=alpha1;

	// Clear dest codes
	for (i=0; i<6; i++)
	{
		// Clear the code
		dest[2+i]=0;
	}

	// For each original select the best
	uint codeOffset=2;
	sint codeShift=5;
	for (i=0; i<16; i++)
	{
		// Best dist and code
		uint bestDist=10000;
		uint8 bestCode=0;

		// Calc distance
		for (uint code=0; code<8; code++)
		{
			// Distance from original value
			uint dist=abs ((sint)(source[i])-(sint)(value[code]));

			// The best ?
			if (dist<bestDist)
			{
				// New best
				bestDist=dist;
				bestCode=code;
			}

			// Perfect, stop searching
			if (dist==0)
				break;
		}

		// Store the best
		if (codeShift>=0)
			dest[codeOffset]|=bestCode<<codeShift;
		else
		{
			dest[codeOffset]|=bestCode>>(-codeShift);
			dest[codeOffset+1]|=bestCode<<(8+codeShift);
		}


		// Next shift
		codeShift-=3;
		if (codeShift<=-3)
		{
			codeOffset++;
			codeShift+=8;
		}
	}
}

// ***************************************************************************
void		CPatch::getTileTileColors(uint ts, uint tt, CRGBA corners[4])
{
	for(sint i=0;i<4;i++)
	{
		CTileColor	&tcol= TileColors[ (tt+(i>>1))*(OrderS+1) + (ts+(i&1)) ];
		CRGBA		&col= corners[i];
		col.set565 (tcol.Color565);
	}
}


// ***************************************************************************
// bilinear at center of the pixels. x E [0, 3], y E [0, 3].
inline void		bilinearTileColorAndModulate(CRGBA	corners[4], uint x, uint y, CRGBA &res)
{
	// Fast bilinear and modulate. 
	// \todo yoyo: TODO_OPTIMIZE: should be ASMed later. (MMX...)
	// hardcoded for 4 pixels.
	nlassert(NL_LUMEL_BY_TILE==4);

	// expand to be on center of pixel=> 1,3,5 or 7.
	x= (x<<1)+1;
	y= (y<<1)+1;
	uint	x1= 8-x;
	uint	y1= 8-y;

	// compute weight factors.
	uint	xy=		x*y;
	uint	x1y=	x1*y;
	uint	xy1=	x*y1;
	uint	x1y1=	x1*y1;

	// bilinear
	uint	R,G,B;
	// pix left top.
	R = corners[0].R * x1y1;
	G = corners[0].G * x1y1;
	B = corners[0].B * x1y1;
	// pix right top.
	R+= corners[1].R * xy1;
	G+= corners[1].G * xy1;
	B+= corners[1].B * xy1;
	// pix left bottom.
	R+= corners[2].R * x1y;
	G+= corners[2].G * x1y;
	B+= corners[2].B * x1y;
	// pix right bottom.
	R+= corners[3].R * xy;
	G+= corners[3].G * xy;
	B+= corners[3].B * xy;

	// modulate with input.
	R*= res.R;
	G*= res.G;
	B*= res.B;

	// result.
	res.R= R >> 14;
	res.G= G >> 14;
	res.B= B >> 14;
}


// ***************************************************************************
void		CPatch::modulateTileLightmapWithTileColors(uint ts, uint tt, CRGBA *dest, uint stride)
{
	// Get the tileColors around this tile
	CRGBA	corners[4];
	getTileTileColors(ts, tt, corners);

	// For all lumel, bilinear.
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// compute this pixel, and modulate
			bilinearTileColorAndModulate(corners, x, y, dest[y*stride + x]);
		}
	}
}


// ***************************************************************************
void		CPatch::modulateTileLightmapEdgeWithTileColors(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	// Get the tileColors around this tile
	CRGBA	corners[4];
	getTileTileColors(ts, tt, corners);

	// get coordinate according to edge.
	uint	x,y;
	switch(edge)
	{
	case 0: x= 0; break;
	case 1: y= NL_LUMEL_BY_TILE-1; break;
	case 2: x= NL_LUMEL_BY_TILE-1; break;
	case 3: y= 0; break;
	};

	// For all lumel of the edge, bilinear.
	uint	i;
	for(i=0; i<NL_LUMEL_BY_TILE; i++)
	{
		// if vertical edge
		if( (edge&1)==0 )	y= i;
		// else horizontal edge
		else				x= i;

		// manage inverse.
		uint	where;
		if(inverse)		where= (NL_LUMEL_BY_TILE-1)-i;
		else			where= i;
		// compute this pixel, and modulate
		bilinearTileColorAndModulate(corners, x, y, dest[where*stride]);
	}
}


// ***************************************************************************
void		CPatch::modulateTileLightmapPixelWithTileColors(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	// Get the tileColors around this tile
	CRGBA	corners[4];
	getTileTileColors(ts, tt, corners);

	// compute this pixel, and modulate
	bilinearTileColorAndModulate(corners, s, t, *dest);
}



// ***************************************************************************
void		CPatch::computeTileLightmapAutomatic(uint ts, uint tt, CRGBA *dest, uint stride)
{
	uint	x, y;
	for(y=0; y<NL_LUMEL_BY_TILE; y++)
	{
		for(x=0; x<NL_LUMEL_BY_TILE; x++)
		{
			// compute this pixel.
			computeTileLightmapPixelAutomatic(ts, tt, x, y, dest+ y*stride + x);
		}
	}
}

// ***************************************************************************
void		CPatch::computeTileLightmapEdgeAutomatic(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	// get coordinate according to edge.
	uint	x,y;
	switch(edge)
	{
	case 0: x= 0; break;
	case 1: y= NL_LUMEL_BY_TILE-1; break;
	case 2: x= NL_LUMEL_BY_TILE-1; break;
	case 3: y= 0; break;
	};

	uint	i;
	for(i=0; i<NL_LUMEL_BY_TILE; i++)
	{
		// if vertical edge
		if( (edge&1)==0 )	y= i;
		// else horizontal edge
		else				x= i;

		// manage inverse.
		uint	where;
		if(inverse)		where= (NL_LUMEL_BY_TILE-1)-i;
		else			where= i;
		// compute this pixel.
		computeTileLightmapPixelAutomatic(ts, tt, x, y, dest+ where*stride);
	}
}

// ***************************************************************************
void		CPatch::computeTileLightmapPixelAutomatic(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	float		u,v;
	static const float	lumelSize= 1.f/NL_LUMEL_BY_TILE;

	// use 3 computeVertex to compute a normal. This is slow....
	CVector		p0, p1 ,p2;
	// 1st vert. Top-left of the lumel.
	u= (ts + s*lumelSize )/OrderS;
	v= (tt + t*lumelSize )/OrderT;
	p0= computeVertex(u, v);
	// 2nd vert. Bottom-left of the lumel.
	u= (ts + s*lumelSize )/OrderS;
	v= (tt + (t+1)*lumelSize )/OrderT;
	p1= computeVertex(u, v);
	// 3rd vert. Center-Right of the lumel.
	u= (ts + (s+1)*lumelSize )/OrderS;
	v= (tt + (t+0.5f)*lumelSize )/OrderT;
	p2= computeVertex(u, v);

	// the normal.
	CVector		normal;
	normal= (p1-p0)^(p2-p0);
	normal.normalize();

	// lighting.
	float	c= -normal*getLandscape()->getAutomaticLightDir();
	c= max(c, 0.f);
	sint	ic;

#ifdef NL_OS_WINDOWS
	// FastFloor using fistp. Don't care convention.
	float	fc= c*256;
	_asm
	{
		fld fc
		fistp ic
	}
#else
	ic= (sint)floor(c*256);
#endif
	clamp(ic, 0, 255);

	// ambiant/diffuse lighting.
	*dest= getLandscape()->getStaticLight()[ic];
}


// ***************************************************************************
void		CPatch::getTileLumelmapPrecomputed(uint ts, uint tt, uint8 *dest, uint stride)
{
	// Uncompressed ?
	if (UncompressedLumels.empty())
	{
		// Unpack the lumels
		uint8 buffer[NL_LUMEL_BY_TILE*NL_LUMEL_BY_TILE];
		unpackLumelBlock (buffer, &(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]));

		// Retrun it
		uint	x, y;
		for(y=0; y<NL_LUMEL_BY_TILE; y++)
		{
			for(x=0; x<NL_LUMEL_BY_TILE; x++)
			{
				// lumel
				dest[y*stride + x]= buffer[x+(y<<NL_LUMEL_BY_TILE_SHIFT)];
			}
		}
	}
	else
	{
		// Offset in the lumel
		uint offset=ts+tt*(OrderS<<NL_LUMEL_BY_TILE_SHIFT);

		// For each lumels
		uint	x, y;
		for(y=0; y<NL_LUMEL_BY_TILE; y++)
		{
			for(x=0; x<NL_LUMEL_BY_TILE; x++)
			{
				// lumel
				dest[y*stride + x]= UncompressedLumels[offset + x + (y<<NL_LUMEL_BY_TILE_SHIFT)];
			}
		}
	}
}


// ***************************************************************************
void		CPatch::getTileLumelmapPixelPrecomputed(uint ts, uint tt, uint s, uint t, uint8 &dest) const
{
	// Uncompressed ?
	if (UncompressedLumels.empty())
	{
		// Return the lumel
		dest= getUnpackLumelBlock (&(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]), s+(t<<2));
	}
	else
	{
		// Return the lumel
		dest= UncompressedLumels[ts+tt*(OrderS<<NL_LUMEL_BY_TILE_SHIFT)+s+(t*OrderS<<2)];
	}
}


// ***************************************************************************
void		CPatch::computeTileLightmapPrecomputed(uint ts, uint tt, CRGBA *dest, uint stride)
{
	// Lumel table
	const CRGBA* colorTable=getLandscape ()->getStaticLight ();
	// Uncompressed ?
	if (UncompressedLumels.empty())
	{
		// Unpack the lumels
		uint8 buffer[NL_LUMEL_BY_TILE*NL_LUMEL_BY_TILE];
		unpackLumelBlock (buffer, &(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]));

		// Retrun it
		uint	x, y;
		for(y=0; y<NL_LUMEL_BY_TILE; y++)
		{
			for(x=0; x<NL_LUMEL_BY_TILE; x++)
			{
				// lumel
				dest[y*stride + x]=colorTable[buffer[x+(y<<NL_LUMEL_BY_TILE_SHIFT)]];
			}
		}
	}
	else
	{
		// Offset in the lumel
		uint offset=ts+tt*(OrderS<<NL_LUMEL_BY_TILE_SHIFT);

		// For each lumels
		uint	x, y;
		for(y=0; y<NL_LUMEL_BY_TILE; y++)
		{
			for(x=0; x<NL_LUMEL_BY_TILE; x++)
			{
				// lumel
				dest[y*stride + x]=colorTable[UncompressedLumels[offset + x + (y<<NL_LUMEL_BY_TILE_SHIFT)]];
			}
		}
	}
}

// ***************************************************************************

static uint NL3DPixelStartLumel[4]={0, 4*3, 3, 0};
static uint NL3DDeltaLumel[4]={4, 1, 4, 1};

// ***************************************************************************
void		CPatch::computeTileLightmapEdgePrecomputed(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	// Lumel table
	const CRGBA* colorTable=getLandscape ()->getStaticLight ();

	// Uncompressed ?
	if (UncompressedLumels.empty())
	{
		// Witch corner to start ?
		uint pixel=NL3DPixelStartLumel[edge];
		uint delta=NL3DDeltaLumel[edge];

		// For each lumels
		const uint8 *src=&(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]);
		uint x;
		if (inverse)
		{
			uint inverseStride=stride*(4-1);
			for(x=0; x<4; x++)
			{
				// lumel
				dest[inverseStride-x*stride]=colorTable[getUnpackLumelBlock (src, pixel)];
				pixel+=delta;
			}
		}
		else
		{
			for(x=0; x<4; x++)
			{
				// lumel
				dest[x*stride]=colorTable[getUnpackLumelBlock (src, pixel)];
				pixel+=delta;
			}
		}
	}
	else
	{
		// Offset in the lumel
		uint offset=ts+tt*(OrderS<<NL_LUMEL_BY_TILE_SHIFT);

		// Start and delta
		uint delta;
		uint lumelS=OrderS<<2;
		uint lumelT=OrderT<<2;
		switch (edge)
		{
		case 0:
			delta=lumelS;
			break;
		case 1:
			offset+=lumelS*3;
			delta=1;
			break;
		case 2:
			offset+=3;
			delta=lumelS;
			break;
		case 3:
			delta=1;
			break;
		}

		// For each lumels
		uint x;
		if (inverse)
		{
			for(x=0; x<4; x++)
			{
				uint inverseStride=stride*(4-1);
				// lumel
				dest[inverseStride-x*stride]=colorTable[UncompressedLumels[offset]];
				offset+=delta;
			}
		}
		else
		{
			for(x=0; x<4; x++)
			{
				// lumel
				dest[x*stride]=colorTable[UncompressedLumels[offset]];
				offset+=delta;
			}
		}
	}
}

// ***************************************************************************
void		CPatch::computeTileLightmapPixelPrecomputed(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	// Lumel table
	const CRGBA* colorTable=getLandscape ()->getStaticLight ();

	// Uncompressed ?
	if (UncompressedLumels.empty())
	{
		// Return the lumel
		*dest=colorTable[getUnpackLumelBlock (&(CompressedLumels[(ts + (tt*OrderS))*NL_BLOCK_LUMEL_COMPRESSED_SIZE]), s+(t<<2))];
	}
	else
	{
		// Return the lumel
		*dest=colorTable[UncompressedLumels[ts+tt*(OrderS<<NL_LUMEL_BY_TILE_SHIFT)+s+(t*OrderS<<2)]];
	}
}



// ***************************************************************************
void		CPatch::computeTileLightmap(uint ts, uint tt, CRGBA *dest, uint stride)
{
	if(getLandscape()->getAutomaticLighting())
		computeTileLightmapAutomatic(ts, tt, dest, stride);
	else
		computeTileLightmapPrecomputed(ts, tt, dest, stride);

	// modulate dest with tileColors (at center of lumels).
	modulateTileLightmapWithTileColors(ts, tt, dest, stride);
}
// ***************************************************************************
void		CPatch::computeTileLightmapEdge(uint ts, uint tt, uint edge, CRGBA *dest, uint stride, bool inverse)
{
	if(getLandscape()->getAutomaticLighting())
		computeTileLightmapEdgeAutomatic(ts, tt, edge, dest, stride, inverse);
	else
		computeTileLightmapEdgePrecomputed(ts, tt, edge, dest, stride, inverse);

	// modulate dest with tileColors (at center of lumels).
	modulateTileLightmapEdgeWithTileColors(ts, tt, edge, dest, stride, inverse);
}


// ***************************************************************************
void		CPatch::computeTileLightmapPixel(uint ts, uint tt, uint s, uint t, CRGBA *dest)
{
	if(getLandscape()->getAutomaticLighting())
		computeTileLightmapPixelAutomatic(ts, tt, s, t, dest);
	else
		computeTileLightmapPixelPrecomputed(ts, tt, s, t, dest);

	// modulate dest with tileColors (at center of lumels).
	modulateTileLightmapPixelWithTileColors(ts, tt, s, t, dest);
}


// ***************************************************************************
void		CPatch::computeTileLightmapPixelAroundCorner(const CVector2f &stIn, CRGBA *dest, bool lookAround)
{
	bool	mustLookOnNeighbor= false;

	// Get the Uv, in [0,Order?*NL_LUMEL_BY_TILE] basis (ie lumel basis).
	sint	u, v;
	u= (sint)floor(stIn.x*NL_LUMEL_BY_TILE);
	v= (sint)floor(stIn.y*NL_LUMEL_BY_TILE);

	// if allowed, try to go on neighbor patch.
	if(lookAround)
	{
		// try to know if we must go on a neighbor patch (maybe false with bind X/1).
		if( u<0 || u>=OrderS*NL_LUMEL_BY_TILE || v<0 || v>=OrderT*NL_LUMEL_BY_TILE)
			mustLookOnNeighbor= true;
	}


	// If we must get (if possible) the pixel in the current patch, do it.
	if(!mustLookOnNeighbor)
	{
		// if out this patch, abort.
		if( u<0 || u>=OrderS*NL_LUMEL_BY_TILE || v<0 || v>=OrderT*NL_LUMEL_BY_TILE)
			return;
		else
		{
			// get this pixel.
			computeTileLightmapPixel(u>>NL_LUMEL_BY_TILE_SHIFT, v>>NL_LUMEL_BY_TILE_SHIFT, u&(NL_LUMEL_BY_TILE-1), v&(NL_LUMEL_BY_TILE-1), dest);
		}
	}
	// else get from the best neighbor patch.
	else
	{
		// choose against which edge we must find the pixel.
		uint	edge;
		if(u<0)					edge=0;
		else if(v>=OrderT*NL_LUMEL_BY_TILE)	edge=1;
		else if(u>=OrderS*NL_LUMEL_BY_TILE)	edge=2;
		else if(v<0)			edge=3;

		// retrieve info on neighbor.
		CBindInfo			bindInfo;
		getBindNeighbor(edge, bindInfo);

		// if neighbor present.
		if(bindInfo.Zone)
		{
			CVector2f	stOut;
			CPatch		*patchOut;
			uint		patchId;

			// Ok, search uv on this patch.
			CPatchUVLocator		uvLocator;
			uvLocator.build(this, edge, bindInfo);
			patchId= uvLocator.selectPatch(stIn);
			uvLocator.locateUV(stIn, patchId, patchOut, stOut);

			// retry only one time, so at next call, must find the data IN htis patch (else abort).
			patchOut->computeTileLightmapPixelAroundCorner(stOut, dest, false);
		}
	}
}


// ***************************************************************************
void		CPatch::computeNearBlockLightmap(uint uts, uint utt, CRGBA	*lightText)
{
	sint	ts= uts;
	sint	tt= utt;

	// hardcoded for 10x10.
	nlassert(NL_TILE_LIGHTMAP_SIZE==10);
	CRGBA	*dest;
	uint	edge;
	uint	corner;

	// Compute center of the TessBlock: the 2x2 tiles.
	//=================
	// compute tile 0,0 of the tessBlock. must decal of 1 pixel.
	dest= lightText+NL_TILE_LIGHTMAP_SIZE+1;
	computeTileLightmap(ts, tt, dest, NL_TILE_LIGHTMAP_SIZE);
	// compute tile 1,0 of the tessBlock. must decal of 1 pixel.
	dest= lightText + NL_LUMEL_BY_TILE + NL_TILE_LIGHTMAP_SIZE+1 ;
	computeTileLightmap(ts+1, tt, dest, NL_TILE_LIGHTMAP_SIZE);
	// compute tile 0,1 of the tessBlock. must decal of 1 pixel.
	dest= lightText + NL_LUMEL_BY_TILE*NL_TILE_LIGHTMAP_SIZE + NL_TILE_LIGHTMAP_SIZE+1 ;
	computeTileLightmap(ts, tt+1, dest, NL_TILE_LIGHTMAP_SIZE);
	// compute tile 1,1 of the tessBlock. must decal of 1 pixel.
	dest= lightText + NL_LUMEL_BY_TILE*NL_TILE_LIGHTMAP_SIZE + NL_LUMEL_BY_TILE + NL_TILE_LIGHTMAP_SIZE+1 ;
	computeTileLightmap(ts+1, tt+1, dest, NL_TILE_LIGHTMAP_SIZE);


	// Compute edges of the TessBlock.
	//=================
	bool	edgeBorder[4];
	// where are we on a border of a patch??
	edgeBorder[0]= ( ts==0 );
	edgeBorder[1]= ( tt == OrderT-2 );
	edgeBorder[2]= ( ts == OrderS-2 );
	edgeBorder[3]= ( tt==0 );

	// For all edges.
	for(edge=0; edge<4; edge++)
	{
		// compute dest info.
		//==============
		// Are we on a vertical edge or horizontal edge??
		uint	stride= (edge&1)==0? NL_TILE_LIGHTMAP_SIZE : 1;

		// must compute on which tile we must find info.
		sint	decalS=0;
		sint	decalT=0;
		// and must compute ptr, where we store the result of the edge.
		switch(edge)
		{
		case 0: decalS=-1; dest= lightText + 0 + NL_TILE_LIGHTMAP_SIZE; break;
		case 1: decalT= 2; dest= lightText + 1 + (NL_TILE_LIGHTMAP_SIZE-1)*NL_TILE_LIGHTMAP_SIZE; break;
		case 2: decalS= 2; dest= lightText + (NL_TILE_LIGHTMAP_SIZE-1) + NL_TILE_LIGHTMAP_SIZE; break;
		case 3: decalT=-1; dest= lightText + 1; break;
		};

		// compute the second tile dest info.
		CRGBA	*dest2;
		sint	decalS2;
		sint	decalT2;
		// if vertical edge.
		if((edge&1)==0)
		{
			// Next Y tile.
			dest2= dest + NL_LUMEL_BY_TILE*NL_TILE_LIGHTMAP_SIZE;
			decalS2= decalS;
			decalT2= decalT+1;
		}
		else
		{
			// Next X tile.
			dest2= dest + NL_LUMEL_BY_TILE;
			decalS2= decalS+1;
			decalT2= decalT;
		}


		// If we are not on a border of a patch, just compute on the interior of the patch.
		//==============
		if(!edgeBorder[edge])
		{
			// find the result on the mirrored border of us. First tile.
			computeTileLightmapEdge(ts+decalS, tt+decalT, (edge+2)&3, dest, stride, false);

			// find the result on the mirrored border of us. Second Tile.
			computeTileLightmapEdge(ts+decalS2, tt+decalT2, (edge+2)&3, dest2, stride, false);

		}
		// else, slightly complicated, must find the result on neighbor patch(s).
		//==============
		else
		{
			CPatchUVLocator		uvLocator;
			CBindInfo			bindInfo;
			bindInfo.Zone= NULL;

			// if smmothed edge, search the neighbor.
			if(getSmoothFlag(edge))
			{
				// Build the bindInfo against this edge.
				getBindNeighbor(edge, bindInfo);

				// if ok, build the uv info against this edge.
				if(bindInfo.Zone)
				{
					uvLocator.build(this, edge, bindInfo);
					// if there is not same tile order across the edge, invalidate the smooth.
					// This is rare, so don't bother.
					if(!uvLocator.sameEdgeOrder())
						bindInfo.Zone= NULL;
				}
			}


			// Fast reject: if no neighbor, or if not smoothed, or if edge order pb, just copy from my interior.
			if(!bindInfo.Zone)
			{
				CRGBA	*src;
				switch(edge)
				{
				case 0: src= dest + 1; break;
				case 1: src= dest - NL_TILE_LIGHTMAP_SIZE; break;
				case 2: src= dest - 1; break;
				case 3: src= dest + NL_TILE_LIGHTMAP_SIZE; break;
				};
				
				// fill the NL_LUMEL_BY_TILE*2 (8) pixels.
				for(uint n=NL_LUMEL_BY_TILE*2; n>0; n--, src+=stride, dest+=stride)
					*dest= *src;
			}
			// else, ok, get from neighbor.
			else
			{
				CVector2f	stIn, stOut;
				CPatch		*patchOut;
				uint		patchId;
				uint		edgeOut;
				bool		inverse;

				// First Tile.
				//=========
				// to remove floor pbs, take the center of the wanted tile.
				stIn.set(ts+decalS + 0.5f, tt+decalT + 0.5f);
				patchId= uvLocator.selectPatch(stIn);
				uvLocator.locateUV(stIn, patchId, patchOut, stOut);
				// must find what edge on neighbor to compute, and if we must inverse (swap) result.
				// easy: the edge of the tile is the edge of the patch where we are binded.
				edgeOut= bindInfo.Edge[patchId];
				// edge0 is oriented in T increasing order. edge1 is oriented in S increasing order.
				// edge2 is oriented in T decreasing order. edge3 is oriented in S decreasing order.
				// inverse is true if same sens on both edges (because of mirroring, sens should be different).
				inverse= (edge>>1)==(edgeOut>>1);
				// compute the lightmap on the edge of the neighbor.
				patchOut->computeTileLightmapEdge((sint)floor(stOut.x), (sint)floor(stOut.y), edgeOut, dest, stride, inverse);

				// Second Tile.
				//=========
				// same reasoning.
				stIn.set(ts+decalS2 + 0.5f, tt+decalT2 + 0.5f);
				patchId= uvLocator.selectPatch(stIn);
				uvLocator.locateUV(stIn, patchId, patchOut, stOut);
				edgeOut= bindInfo.Edge[patchId];
				inverse= (edge>>1)==(edgeOut>>1);
				patchOut->computeTileLightmapEdge((sint)floor(stOut.x), (sint)floor(stOut.y), edgeOut, dest2, stride, inverse);
			}

		}
	}


	// Compute corners of the TessBlock.
	//=================
	bool	cornerOnPatchEdge[4];
	bool	cornerOnPatchCorner[4];
	// where are we on a edge border of a patch??
	cornerOnPatchEdge[0]= edgeBorder[3] != edgeBorder[0];
	cornerOnPatchEdge[1]= edgeBorder[0] != edgeBorder[1];
	cornerOnPatchEdge[2]= edgeBorder[1] != edgeBorder[2];
	cornerOnPatchEdge[3]= edgeBorder[2] != edgeBorder[3];
	// where are we on a corner border of a patch??
	cornerOnPatchCorner[0]= edgeBorder[3] && edgeBorder[0];
	cornerOnPatchCorner[1]= edgeBorder[0] && edgeBorder[1];
	cornerOnPatchCorner[2]= edgeBorder[1] && edgeBorder[2];
	cornerOnPatchCorner[3]= edgeBorder[2] && edgeBorder[3];

	// For all corners.
	for(corner=0; corner<4; corner++)
	{
		// compute dest info.
		//==============
		// must compute on which tile we must find info.
		sint	decalS=0;
		sint	decalT=0;
		// and must compute ptr, where we store the result of the corner.
		switch(corner)
		{
		case 0: decalS=-1; decalT=-1; dest= lightText + 0 + 0; break;
		case 1: decalS=-1; decalT= 2; dest= lightText + 0 + (NL_TILE_LIGHTMAP_SIZE-1)*NL_TILE_LIGHTMAP_SIZE; break;
		case 2: decalS= 2; decalT= 2; dest= lightText + (NL_TILE_LIGHTMAP_SIZE-1) + (NL_TILE_LIGHTMAP_SIZE-1)*NL_TILE_LIGHTMAP_SIZE; break;
		case 3: decalS= 2; decalT=-1; dest= lightText + (NL_TILE_LIGHTMAP_SIZE-1) + 0; break;
		};


		// If we are not on a border of a patch, just compute on the interior of the patch.
		//==============
		// if the corner is IN the patch.
		if(!cornerOnPatchCorner[corner] && !cornerOnPatchEdge[corner])
		{
			// what pixel to read.
			uint	subS, subT;
			if(decalS==-1)	subS= NL_LUMEL_BY_TILE-1;
			else			subS= 0;
			if(decalT==-1)	subT= NL_LUMEL_BY_TILE-1;
			else			subT= 0;

			// find the result on the corner of the neighbor tile.
			computeTileLightmapPixel(ts+decalS, tt+decalT, subS, subT, dest);
		}
		else
		{
			// By default, fill the corner with our interior corner. Because other methods may fail.
			CRGBA	*src;
			switch(corner)
			{
			case 0: src= dest + 1 + NL_TILE_LIGHTMAP_SIZE; break;
			case 1: src= dest + 1 - NL_TILE_LIGHTMAP_SIZE; break;
			case 2: src= dest - 1 - NL_TILE_LIGHTMAP_SIZE; break;
			case 3: src= dest - 1 + NL_TILE_LIGHTMAP_SIZE; break;
			};
			
			// fill the pixel.
			*dest= *src;

			// get the coordinate of the corner, in our [0,Order] basis. get it at the center of the pixel.
			CBindInfo			bindInfo;
			CPatchUVLocator		uvLocator;
			CVector2f			stIn, stOut;
			CPatch				*patchOut;
			uint				patchId;
			float				decX, decY;
			static const float	lumelSize= 1.f/NL_LUMEL_BY_TILE;
			static const float	semiLumelSize= 0.5f*lumelSize;

			if(decalS==-1)	decX= -  semiLumelSize;
			else			decX= 2+ semiLumelSize;
			if(decalT==-1)	decY= -  semiLumelSize;
			else			decY= 2+ semiLumelSize;
			stIn.set( ts+decX, tt+decY);


			// if the corner is on One edge only of the patch.
			if(cornerOnPatchEdge[corner])
			{
				// find the edge where to read this corner: hard edge after or before this corner.
				if(edgeBorder[corner])	edge= corner;
				else					edge= (corner+4-1) & 3;

				// if this edge is smoothed, find on neighbor.
				if(getSmoothFlag(edge))
				{
					// retrieve neigbhor info.
					getBindNeighbor(edge, bindInfo);

					// if neighbor present.
					if(bindInfo.Zone)
					{
						// Ok, search uv on this patch.
						uvLocator.build(this, edge, bindInfo);
						patchId= uvLocator.selectPatch(stIn);
						uvLocator.locateUV(stIn, patchId, patchOut, stOut);

						// Get the Uv, in [0,Order?*NL_LUMEL_BY_TILE] basis (ie lumel basis), and get from neighbor patch
						sint	u, v;
						u= (sint)floor(stOut.x*NL_LUMEL_BY_TILE);
						v= (sint)floor(stOut.y*NL_LUMEL_BY_TILE);
						patchOut->computeTileLightmapPixel(u>>NL_LUMEL_BY_TILE_SHIFT, v>>NL_LUMEL_BY_TILE_SHIFT, u&(NL_LUMEL_BY_TILE-1), v&(NL_LUMEL_BY_TILE-1), dest);
					}
				}
				// else we must still smooth with our lumel on this patch, so get it from neighbor on edge.
				else
				{
					// first, clamp to our patch (recenter on the previous pixel)
					if(stIn.x<0)			stIn.x+= lumelSize;
					else if(stIn.x>OrderS)	stIn.x-= lumelSize;
					else if(stIn.y<0)		stIn.y+= lumelSize;
					else if(stIn.y>OrderT)	stIn.y-= lumelSize;

					// Get the Uv, in [0,Order?*NL_LUMEL_BY_TILE] basis (ie lumel basis), and get from this patch
					sint	u, v;
					u= (sint)floor(stIn.x*NL_LUMEL_BY_TILE);
					v= (sint)floor(stIn.y*NL_LUMEL_BY_TILE);
					computeTileLightmapPixel(u>>NL_LUMEL_BY_TILE_SHIFT, v>>NL_LUMEL_BY_TILE_SHIFT, u&(NL_LUMEL_BY_TILE-1), v&(NL_LUMEL_BY_TILE-1), dest);
				}
			}
			// else it is on a corner of the patch.
			else
			{
				// if the corner of the patch (same as tile corner) is smoothed, find on neighbor
				if(getCornerSmoothFlag(corner))
				{
					// retrieve neigbhor info. NB: use edgeId=corner, (corner X is the start of the edge X)it works.
					getBindNeighbor(corner, bindInfo);

					// if neighbor present.
					if(bindInfo.Zone)
					{
						// Ok, search uv on this patch.
						uvLocator.build(this, corner, bindInfo);
						patchId= uvLocator.selectPatch(stIn);
						uvLocator.locateUV(stIn, patchId, patchOut, stOut);

						// same reasoning as in computeDisplaceCornerSmooth(), must find the pixel on the neighbor 
						// of our neighbor. But the current corner may be a corner on a bind X/1. All is managed by doing
						// this way.
						patchOut->computeTileLightmapPixelAroundCorner(stOut, dest, true);
					}
				}
			}
		}

	}


}


// ***************************************************************************
void		CPatch::getTileLightMap(uint ts, uint tt, CPatchRdrPass *&rdrpass)
{
	// TessBlocks must have been allocated.
	nlassert(TessBlocks.size()!=0);

	// get what tessBlock to use.
	uint	numtb, numtm;
	computeTbTm(numtb, numtm, ts, tt);
	CTessBlock	&tessBlock= TessBlocks[numtb];

	// If the lightmap Id has not been computed, compute it.
	if(tessBlock.LightMapRefCount==0)
	{
		// Compute the lightmap texture, with help of TileColors, with neighboring info etc...
		CRGBA	lightText[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE];
		computeNearBlockLightmap(ts&(~1), tt&(~1), lightText);

		// Create a rdrPass with this texture, donlod to Driver etc...
		tessBlock.LightMapId= Zone->Landscape->getTileLightMap(lightText, rdrpass);

		// store this rdrpass ptr.
		tessBlock.LightMapRdrPass= rdrpass;
	}

	// We are using this 2x2 tiles lightmap.
	tessBlock.LightMapRefCount++;


	// get the rdrpass ptr of the tessBlock lightmap
	rdrpass= tessBlock.LightMapRdrPass;
}


// ***************************************************************************
void		CPatch::getTileLightMapUvInfo(uint ts, uint tt, CVector &uvScaleBias)
{
	// TessBlocks must have been allocated.
	nlassert(TessBlocks.size()!=0);

	// get what tessBlock to use.
	uint	numtb, numtm;
	computeTbTm(numtb, numtm, ts, tt);
	CTessBlock	&tessBlock= TessBlocks[numtb];

	// Get the uvScaleBias for the tile 0,0  of the block.
	Zone->Landscape->getTileLightMapUvInfo(tessBlock.LightMapId, uvScaleBias);

	// Must increment the bias, for the good tile in the 2x2 block Lightmap.
	uint	tsDec= ts & 1;
	uint	ttDec= tt & 1;
	uvScaleBias.x+= tsDec * uvScaleBias.z;
	uvScaleBias.y+= ttDec * uvScaleBias.z;
}


// ***************************************************************************
void		CPatch::releaseTileLightMap(uint ts, uint tt)
{
	// TessBlocks must have been allocated.
	nlassert(TessBlocks.size()!=0);

	// get what tessBlock to use.
	uint	numtb, numtm;
	computeTbTm(numtb, numtm, ts, tt);
	CTessBlock	&tessBlock= TessBlocks[numtb];

	// If no more tileMaterial use this lightmap, release it.
	nlassert(tessBlock.LightMapRefCount>0);
	tessBlock.LightMapRefCount--;
	if(tessBlock.LightMapRefCount==0)
	{
		Zone->Landscape->releaseTileLightMap(tessBlock.LightMapId);
	}
}

// ***************************************************************************
void		CPatch::packShadowMap (const uint8 *pLumelSrc)
{
	// Number of lumel by lines
	uint lumelCount=OrderS*NL_LUMEL_BY_TILE;

	// Number of block in a line
	nlassert ((lumelCount&0x3)==0);
	uint numLumelBlock=lumelCount>>2;

	// Number of line
	uint lineCount=OrderT*NL_LUMEL_BY_TILE;

	// Number of block line
	nlassert ((lineCount&0x3)==0);
	uint numLineBlock=lineCount>>2;

	// Resize the compressed buffer
	CompressedLumels.resize (numLumelBlock*numLineBlock*NL_BLOCK_LUMEL_COMPRESSED_SIZE);

	// Input of compresed data
	uint8 *compressedData=&CompressedLumels[0];

	// Each line block
	for (uint lineBlock=0; lineBlock<numLineBlock; lineBlock++)
	{
		// Block pointer
		const uint8 *blockLine=pLumelSrc;

		// Each lumel block
		for (uint lumelBlock=0; lumelBlock<numLumelBlock; lumelBlock++)
		{
			// *** Unpack the block
			uint countU;

			// Last block ?
			if (lumelBlock==numLumelBlock-1)
				countU=lumelCount&3;
			else
				countU=4;

			// Destination lumel
			const uint8 *blockSrc=blockLine;

			// Temp block
			uint8 originalBlock[4*4];

			// Copy the lumels in the bloc
			for (uint v=0; v<NL_LUMEL_BY_TILE; v++)
			{
				for (uint u=0; u<NL_LUMEL_BY_TILE; u++)
				{
					// Copy the lumel
					originalBlock[(v<<2)+u]=blockSrc[u];
				}

				// Next line
				blockSrc+=lumelCount;
			}

			// Get min and max alpha
			uint8 alphaMin=255;
			uint8 alphaMax=0;

			// Scan
			for (uint i=0; i<16; i++)
			{
				// Min ?
				if (originalBlock[i]<alphaMin)
					alphaMin=originalBlock[i];
				if (originalBlock[i]>alphaMax)
					alphaMax=originalBlock[i];
			}

			// *** Try to compress by 2 methods

			// Blcok uncompressed
			uint8 uncompressedBlock[4*4];
	
			// Pack the block
			packLumelBlock (compressedData, originalBlock, alphaMin, alphaMax);

			// Unpack the block
			unpackLumelBlock (uncompressedBlock, compressedData);

			// Eval error
			uint firstMethod=evalLumelBlock (originalBlock, uncompressedBlock, NL_LUMEL_BY_TILE, NL_LUMEL_BY_TILE);

			// second compression
			uint8 secondCompressedBlock[NL_BLOCK_LUMEL_COMPRESSED_SIZE];
			packLumelBlock (secondCompressedBlock, originalBlock, alphaMax, alphaMin);

			// Unpack the block
			unpackLumelBlock (uncompressedBlock, secondCompressedBlock);

			// Eval error
			uint secondMethod=evalLumelBlock (originalBlock, uncompressedBlock, NL_LUMEL_BY_TILE, NL_LUMEL_BY_TILE);

			// Second best ?
			if (secondMethod<firstMethod)
			{
				// Copy compressed data
				memcpy (compressedData, secondCompressedBlock, NL_BLOCK_LUMEL_COMPRESSED_SIZE);
			}

			// Next source block
			compressedData+=NL_BLOCK_LUMEL_COMPRESSED_SIZE;

			// Next block on the line
			blockLine+=4;
		}

		// Next line of block
		pLumelSrc+=lumelCount*4;
	}
}

// ***************************************************************************
void		CPatch::resetCompressedLumels ()
{
	// Number of lumel by lines
	uint lumelCount=OrderS*NL_LUMEL_BY_TILE;

	// Number of block in a line
	nlassert ((lumelCount&0x3)==0);
	uint numLumelBlock=lumelCount>>2;

	// Number of line
	uint lineCount=OrderT*NL_LUMEL_BY_TILE;

	// Number of block line
	nlassert ((lineCount&0x3)==0);
	uint numLineBlock=lineCount>>2;

	// Size of the lumel array
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

}

// ***************************************************************************
void		CPatch::appendTessellationLeaves(std::vector<const CTessFace*>  &leaves) const
{
	nlassert(Son0);
	nlassert(Son1);
	Son0->appendTessellationLeaves(leaves);
	Son1->appendTessellationLeaves(leaves);
}


// ***************************************************************************
CLandscape		*CPatch::getLandscape () const
{
	return Zone->getLandscape();
}





// ***************************************************************************
uint8			CPatch::getOrderForEdge(sint8 edge) const
{
	uint	e= ((sint)edge + 256)&3;
	// If an horizontal edge.
	if( e&1 )	return OrderS;
	else		return OrderT;
}


// ***************************************************************************
void		CPatch::resetTileLightInfluences()
{
	// Fill default.
	TileLightInfluences.resize((OrderS/2 +1) * (OrderT/2 +1));
	// Disable All light influence on all points
	for(uint i=0;i <TileLightInfluences.size(); i++)
	{
		// Disable all light influence on this point.
		TileLightInfluences[i].Light[0]= 0xFF;
	}
}


// ***************************************************************************
// ***************************************************************************
// Realtime Bind info.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CPatch::getBindNeighbor(uint edge, CBindInfo &neighborEdge) const
{
	nlassert(edge<4);

	if(_BindZoneNeighbor[edge]!=NULL)
	{
		getZone()->buildBindInfo(PatchId, edge, _BindZoneNeighbor[edge], neighborEdge);
	}
	else
	{
		neighborEdge.Zone= NULL;
		neighborEdge.NPatchs= 0;
		neighborEdge.MultipleBindNum= 0;
	}
}

// ***************************************************************************
/// debug coloring
void CPatch::setupColorsFromTileFlags(const NLMISC::CRGBA colors[4])
{
	for (uint s = 0; s <= OrderS; ++s)
	{
		for (uint t = 0; t <= OrderT; ++t)
		{
			uint index = std::min(t, (uint) (OrderT - 1)) * OrderS 
				         + std::min(s, (uint) (OrderS - 1));
			TileColors[s + t * (OrderS + 1)].Color565 = colors[(uint) (Tiles[index].getVegetableState())].get565();					
		}
	}
}

// ***************************************************************************
void CPatch::copyTileFlagsFromPatch(const CPatch *src)
{
	nlassert(OrderS == src->OrderS
			 && OrderT == src->OrderT);

	for (uint k = 0; k  < Tiles.size(); ++k)
	{
		Tiles[k].copyFlagsFromOther(src->Tiles[k]);
	}
}


// ***************************************************************************
// ***************************************************************************
// Lightmap get interface.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
uint8		CPatch::getLumel(const CUV &uv) const
{
	// compute tile coord and lumel coord.
	sint	ts, tt;
	// get in lumel coord.
	sint	w= (OrderS<<NL_LUMEL_BY_TILE_SHIFT);
	sint	h= (OrderT<<NL_LUMEL_BY_TILE_SHIFT);
	// fastFloor: use a precision of 256 to avoid doing OptFastFloorBegin.
	// add 128, to round and get cneter of lumel.
	ts= OptFastFloor(uv.U* (w<<8) + 128);	ts>>=8;
	tt= OptFastFloor(uv.V* (h<<8) + 128);	tt>>=8;
	clamp(ts, 0, w-1);
	clamp(tt, 0, h-1);
	// get the lumel
	uint8	ret;
	getTileLumelmapPixelPrecomputed(ts>>NL_LUMEL_BY_TILE_SHIFT, tt>>NL_LUMEL_BY_TILE_SHIFT, 
		ts&(NL_LUMEL_BY_TILE-1), tt&(NL_LUMEL_BY_TILE-1), ret);

	return ret;
}

// ***************************************************************************
void		CPatch::appendTileLightInfluences(const CUV &uv, 
	std::vector<CPointLightInfluence> &pointLightList) const
{
	// Compute TLI coord for BiLinear.
	sint	x,y;
	// There is (OrderS/2+1) * (OrderT/2+1) tileLightInfluences (TLI).
	sint	w= (OrderS>>1);
	sint	h= (OrderT>>1);
	sint	wTLI= w+1;
	sint	hTLI= h+1;
	// fastFloor: use a precision of 256 to avoid doing OptFastFloorBegin.
	x= OptFastFloor(uv.U * (w<<8));
	y= OptFastFloor(uv.V * (h<<8));
	clamp(x, 0, w<<8);
	clamp(y, 0, h<<8);
	// compute the TLI coord, and the subCoord for bilinear.
	sint	xTLI,yTLI, xSub, ySub;
	xTLI= x>>8; clamp(xTLI, 0, w-1);
	yTLI= y>>8; clamp(yTLI, 0, h-1);
	// Hence, xSub and ySub range is [0, 256].
	xSub= x - (xTLI<<8);
	ySub= y - (yTLI<<8);


	// Use a CLightInfluenceInterpolator to biLinear light influence
	CLightInfluenceInterpolator		interp;
	// Must support only 2 light per TLI.
	nlassert(CTileLightInfluence::NumLightPerCorner==2);
	nlassert(CLightInfluenceInterpolator::NumLightPerCorner==2);
	// Get ref on array of PointLightNamed.
	CPointLightNamed	*zonePointLights= NULL;
	if( getZone()->_PointLightArray.getPointLights().size() >0 )
	{
		// const_cast, because will only change _IdInfluence, and 
		// also because CLightingManager will call appendLightedModel()
		zonePointLights= const_cast<CPointLightNamed*>(&(getZone()->_PointLightArray.getPointLights()[0]));
	}
	// For 4 corners.
	for(y=0;y<2;y++)
	{
		for(x=0;x<2;x++)
		{
			// get ref on TLI, and on corner.
			const CTileLightInfluence				&tli= TileLightInfluences[ (yTLI+y)*wTLI + xTLI+x ];
			CLightInfluenceInterpolator::CCorner	&corner= interp.Corners[y*2 + x];
			// For all lights
			uint lid;
			for(lid= 0; lid<CTileLightInfluence::NumLightPerCorner; lid++)
			{
				// get the id of the light in the zone
				uint	tliLightId= tli.Light[lid];
				// If empty id, stop
				if(tliLightId==0xFF)
					break;
				else
				{
					// Set pointer of the light in the corner
					corner.Lights[lid]= zonePointLights + tliLightId;
				}
			}
			// Reset Empty slots.
			for(; lid<CTileLightInfluence::NumLightPerCorner; lid++)
			{
				// set to NULL
				corner.Lights[lid]= NULL;
			}
		}
	}
	// interpolate.
	interp.interpolate(pointLightList, xSub/256.f, ySub/256.f);
}



} // NL3D

