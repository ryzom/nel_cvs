/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.109 2002/04/16 12:36:27 berenguier Exp $
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


#include "3d/landscape.h"
#include "nel/misc/bsphere.h"
#include "3d/texture_file.h"
#include "3d/texture_far.h"
#include "3d/landscape_profile.h"
#include "nel/3d/height_map.h"
#include "3d/tile_noise_map.h"
#include "3d/vegetable_manager.h"
#include "3d/vegetable.h"
#include "3d/landscape_vegetable_block.h"
#include "3d/fast_floor.h"
#include "3d/tile_vegetable_desc.h"
#include "3d/texture_dlm.h"
#include "3d/patchdlm_context.h"


#include "3d/vertex_program.h"



using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
/* 
	Target is 20K faces  in frustum.
	So 80K faces at same time
	So 160K elements (bin tree).
	A good BlockSize (in my opinion) is EstimatedMaxSize / 10, to have less memory leak as possible,
	and to make not so many system allocation.

	NL3D_TESSRDR_ALLOC_BLOCKSIZE is 2 times less, because elements are in Far zone or in Near zone only
	(approx same size...)
*/
#define	NL3D_TESS_ALLOC_BLOCKSIZE		16000
#define	NL3D_TESSRDR_ALLOC_BLOCKSIZE	8000


// ***************************************************************************
// This value is important for the precision of the priority list
#define	NL3D_REFINE_PLIST_DIST_STEP		0.0625
/* This value is important, because faces will be inserted at maximum at this entry in the priority list.
	If not so big (eg 500 meters), a big bunch of faces may be inserted in this entry, which may cause slow down
	sometimes, when all this bunch comes to 0 in the priority list.
	To avoid such a thing, see CTessFacePriorityList::init(), and use of NL3D_REFINE_PLIST_DIST_MAX_MOD.
*/
#define	NL3D_REFINE_PLIST_DIST_MAX		1000
#define	NL3D_REFINE_PLIST_DIST_MAX_MOD	0.7*NL3D_REFINE_PLIST_DIST_MAX

/*
	OverHead size of the priority list is 8 * (NL3D_REFINE_PLIST_DIST_MAX / NL3D_REFINE_PLIST_DIST_STEP).
	So here, it is "only" 128K.
*/


// ***************************************************************************
// Size (in cases) of the quadgrid. must be e power of 2.
const uint			CLandscape::_PatchQuadGridSize= 128;
// Size of a case of the quadgrid.
const float			CLandscape::_PatchQuadGridEltSize= 16;


// ***************************************************************************

// Bitmap Cross

class CTextureCross : public ITexture
{
public:
	/** 
	 * Generate the texture
	 * \author Stephane Coutelas
	 * \date 2000
	 */	
	virtual void doGenerate()
	{
		// Resize
		resize (16, 16);

		// Cross
		static const uint32 cross[16*16]=
		{
			//  0			1			2			3			4			5			6			7			8			9			10			11			12			13			14			15
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
		};

		// Null
		memcpy (&_Data[0][0], cross, 16*16*4);
	}

	// Dummy serial...
	virtual void	serial(NLMISC::IStream &f)  throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureCross);
};


// ***************************************************************************
const char	*EBadBind::what() const throw()
{
	sint			numErr= 0;
	const	sint	NErrByLines= 4;

	_Output= "Landscape Bind Error in (3DSMax indices!! (+1) ): ";

	std::list<CBindError>::const_iterator		it;
	for(it= BindErrors.begin();it!=BindErrors.end(); it++, numErr++)
	{
		char	tmp[256];
		sint	x= it->ZoneId & 255;
		sint	y= it->ZoneId >> 8;
		sprintf(tmp, "zone%3d_%c%c.patch%3d;   ", y+1, (char)('A'+(x/26)), (char)('A'+(x%26)), it->PatchId+1);
		if( (numErr%NErrByLines) == 0)
			_Output+= "\n";
		_Output+= tmp;
	}
	return _Output.c_str(); 
}


// ***************************************************************************
// Init BlockAllcoator with standard BlockMemory.
CLandscape::CLandscape() : 
	TessFaceAllocator(NL3D_TESS_ALLOC_BLOCKSIZE), 
	TessVertexAllocator(NL3D_TESS_ALLOC_BLOCKSIZE), 
	TessFarVertexAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE), 
	TessNearVertexAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE), 
	TileMaterialAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE), 
	TileFaceAllocator(NL3D_TESSRDR_ALLOC_BLOCKSIZE),
	_Far0VB(CLandscapeVBAllocator::Far0), _Far1VB(CLandscapeVBAllocator::Far1), _TileVB(CLandscapeVBAllocator::Tile)
{
	TileInfos.resize(NL3D::NbTilesMax);

	// Resize the vectors of sert of render pass for the far texture
	_FarRdrPassSetVectorFree.resize (getRdrPassIndexWithSize (NL_MAX_SIZE_OF_TEXTURE_EDGE, NL_MAX_SIZE_OF_TEXTURE_EDGE)+1);

	// Far texture not initialized till initTileBanks is not called
	_FarInitialized=false;
	
	// Init far lighting with White/black
	setupStaticLight (CRGBA(255,255,255), CRGBA(0,0,0), 1.f);

	fill(TileInfos.begin(), TileInfos.end(), (CTileInfo*)NULL);

	_FarTransition= 10;		// 10 meters.
	_TileDistNear=100.f;
	_Threshold= 0.001f;
	_RefineMode=true;

	_TileMaxSubdivision= 0;

	_NFreeLightMaps= 0;

	// By default Automatic light comes from up.
	_AutomaticLighting = false;
	_AutomaticLightDir= -CVector::K;

	// By default, noise is enabled.
	_NoiseEnabled= true;

	// By default, we compute Geomorph and Alpha in software.
	_VertexShaderOk= false;
	_VPThresholdChange= false;

	_RenderMustRefillVB= false;

	// priority list.
	_OldRefineCenterSetuped= false;
	_SplitPriorityList.init(NL3D_REFINE_PLIST_DIST_STEP, NL3D_REFINE_PLIST_DIST_MAX, NL3D_REFINE_PLIST_DIST_MAX_MOD);
	_MergePriorityList.init(NL3D_REFINE_PLIST_DIST_STEP, NL3D_REFINE_PLIST_DIST_MAX, NL3D_REFINE_PLIST_DIST_MAX_MOD);
	// just for getTesselatedPos to work properly.
	_OldRefineCenter= CVector::Null;

	// create / Init the vegetable manager.
	_VegetableManager= new CVegetableManager(NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_UNLIT, NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_LIGHTED);

	// Init vegetable  setup.
	_VegetableManagerEnabled= false;
	_DriverOkForVegetable= false;

	_PZBModelPosition= CVector::Null;


	// Default: no updateLighting.
	_ULFrequency= 0;
	_ULPrecTimeInit= false;
	// Default: no textureFar created.
	_ULTotalFarPixels= 0;
	_ULFarPixelsToUpdate= 0;
	_ULRootTextureFar= NULL;
	_ULFarCurrentPatchId= 0;
	// Default: no patch created
	_ULTotalNearPixels= 0;
	_ULNearPixelsToUpdate= 0;
	_ULRootNearPatch= NULL;
	_ULNearCurrentTessBlockId= 0;


	// Dynamic Lighting.
	_TextureDLM= new CTextureDLM(NL3D_LANDSCAPE_DLM_WIDTH, NL3D_LANDSCAPE_DLM_HEIGHT);
	_PatchDLMContextList= new CPatchDLMContextList;
	_DLMMaxAttEnd= 30.f;

}
// ***************************************************************************
CLandscape::~CLandscape()
{
	clear();

	// release the VegetableManager.
	delete _VegetableManager;
	_VegetableManager= NULL;

	// Dynamic Lighting.
	// smartPtr delete
	_TextureDLM= NULL;
	delete _PatchDLMContextList;
	_PatchDLMContextList= NULL;
}


// ***************************************************************************
void			CLandscape::init()
{
	// Fill Far mat.
	// Must init his BlendFunction here!!! becaus it switch between blend on/off during rendering.
	FarMaterial.initUnlit();
	FarMaterial.setSrcBlend(CMaterial::srcalpha);
	FarMaterial.setDstBlend(CMaterial::invsrcalpha);

	// Init material for tile.
	TileMaterial.initUnlit();

	// init quadGrid.
	_PatchQuadGrid.create(_PatchQuadGridSize, _PatchQuadGridEltSize);
}


// ***************************************************************************
void			CLandscape::setThreshold (float thre)
{
	thre= max(thre, 0.f);
	if(thre != _Threshold)
	{
		_Threshold= thre;
		_VPThresholdChange= true;
	}
}


// ***************************************************************************
void			CLandscape::setTileNear (float tileNear)
{
	tileNear= max(tileNear, _FarTransition);

	if(tileNear!=_TileDistNear)
	{
		_TileDistNear= tileNear;
		resetRenderFarAndDeleteVBFV();
	}

}


// ***************************************************************************
void			CLandscape::setTileMaxSubdivision (uint tileDiv)
{
	nlassert(tileDiv>=0 && tileDiv<=4);

	if(tileDiv!=_TileMaxSubdivision)
	{
		_TileMaxSubdivision= tileDiv;
		// Force at Tile==0. Nex refine will split correctly.
		forceMergeAtTileLevel();
	}
}
// ***************************************************************************
uint 			CLandscape::getTileMaxSubdivision ()
{
	return _TileMaxSubdivision;
}


// ***************************************************************************
void			CLandscape::resetRenderFarAndDeleteVBFV()
{
	// For all patch of all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		((*it).second)->resetRenderFarAndDeleteVBFV();
	}
}


// ***************************************************************************
void			CLandscape::forceMergeAtTileLevel()
{
	// For all patch of all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		((*it).second)->forceMergeAtTileLevel();
	}
}


// ***************************************************************************
bool			CLandscape::addZone(const CZone	&newZone)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: adding a zone may add vertices in VB in visible patchs (because of binds)=> buffers are locked.

	uint16	zoneId= newZone.getZoneId();

	if(Zones.find(zoneId)!=Zones.end())
	{
		unlockBuffers();
		return false;
	}
	CZone	*zone= new CZone;

	// copy zone.
	zone->build(newZone);

	// Affect the current lighting of pointLight to the zone.
	ItLightGroupColorMap	itLGC= _LightGroupColorMap.begin();
	while( itLGC != _LightGroupColorMap.end() )
	{
		zone->_PointLightArray.setPointLightFactor(itLGC->first, itLGC->second);
		itLGC++;
	}


	// apply the landscape heightField, modifying BBoxes.
	zone->applyHeightField(*this);

	// compile the zone for this landscape.
	zone->compile(this, Zones);

	// add patchs of this zone to the quadgrid.
	for(sint i= 0; i<zone->getNumPatchs(); i++)
	{
		const CPatch *pa= ((const CZone*)zone)->getPatch(i);
		CPatchIdent	paId;
		paId.ZoneId= zoneId;
		paId.PatchId= i;
		CAABBox		bb= pa->buildBBox();
		_PatchQuadGrid.insert(bb.getMin(), bb.getMax(), paId);
	}

	// Must realase VB Buffers
	unlockBuffers();

	// Because bind may add faces in other (visible) zones because of enforced split, we must check
	// and update any FaceVector.
	updateTessBlocksFaceVector();

	return true;
}
// ***************************************************************************
bool			CLandscape::removeZone(uint16 zoneId)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: remove a zone may change vertices in VB in visible patchs => buffers are locked.

	// find the zone.
	if(Zones.find(zoneId)==Zones.end())
	{
		unlockBuffers();
		return false;
	}
	CZone	*zone= Zones[zoneId];


	// delete patchs from this zone to the quadgrid.
	// use the quadgrid itself to find where patch are. do this using bbox of zone.
	CAABBoxExt		bb= zone->getZoneBB();
	// for security, expand a little the bbox of the zone.
	bb.setSize(bb.getSize()*1.1f);
	// select iterators in the area of this zone.
	_PatchQuadGrid.clearSelection();
	_PatchQuadGrid.select(bb.getMin(), bb.getMax());
	// for each patch, remove it if from deleted zone.
	CQuadGrid<CPatchIdent>::CIterator	it;
	for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end();)
	{
		if( (*it).ZoneId== zone->getZoneId() )
		{
			it= _PatchQuadGrid.erase(it);
		}
		else
			it++;
	}


	// remove the zone.
	zone->release(Zones);
	delete zone;

	// Must realase VB Buffers
	unlockBuffers();

	// because of forceMerge() at unbind, removeZone() can cause change in faces in other (visible) zones.
	updateTessBlocksFaceVector();

	return true;
}
// ***************************************************************************
void			CLandscape::clear()
{
	// Build the list of zoneId.
	vector<uint16>	zoneIds;
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		zoneIds.push_back((*it).first);
	}

	// Remove each zone one by one.
	sint i;
	for(i=0;i<(sint)zoneIds.size();i++)
	{
		nlverify(removeZone(zoneIds[i]));
	}

	// ensure the quadgrid is empty.
	_PatchQuadGrid.clear();


	// If not done, delete all VBhards allocated.
	_Far0VB.clear();
	_Far1VB.clear();
	_TileVB.clear();


	// Reset All Far Texture and unlink _ULRootTextureFar ciruclarList.
	// First "free" the Free list. 
	for(i=0;i<(sint)_FarRdrPassSetVectorFree.size();i++)
	{
		_FarRdrPassSetVectorFree[i].clear();
	}
	// Then free all Far RdrPass.
	ItSPRenderPassSet	itFar;
	// unitl set is empty
	while( (itFar= _FarRdrPassSet.begin()) != _FarRdrPassSet.end())
	{
		// erase with link update.
		eraseFarRenderPassFromSet(*itFar);
	}


	// reset driver.
	_Driver= NULL;
}

// ***************************************************************************
void			CLandscape::setDriver(IDriver *drv)
{
	nlassert(drv);
	if(_Driver != drv)
	{
		_Driver= drv;

		// Does the driver support VertexShader???
		// only if VP supported by GPU.
		_VertexShaderOk= (_Driver->isVertexProgramSupported() && !_Driver->isVertexProgramEmulated());


		// Does the driver has sufficient requirements for Vegetable???
		// only if VP supported by GPU, and Only if max vertices allowed.
		_DriverOkForVegetable= _VertexShaderOk && (_Driver->getMaxVerticesByVertexBufferHard()>=(uint)NL3D_LANDSCAPE_VEGETABLE_MAX_AGP_VERTEX_MAX);

	}
}

// ***************************************************************************
void			CLandscape::clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: clip may add/remove vertices in VB in visible patchs => buffers are locked.


	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->clip(pyramid);
	}

	// Must realase VB Buffers
	unlockBuffers();

	// clip() should not cause change in faces in visible patchs.
	// It should not happens, but check for security.
	nlassert(_TessBlockModificationRoot.getNextToModify()==NULL);
	updateTessBlocksFaceVector();

}
// ***************************************************************************
void			CLandscape::refine(const CVector &refineCenter)
{
	NL3D_PROFILE_LAND_SET(ProfNRefineFaces, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineComputeFaces, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineLeaves, 0);
	NL3D_PROFILE_LAND_SET(ProfNSplits, 0);
	NL3D_PROFILE_LAND_SET(ProfNMerges, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineInTileTransition, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineWithLowDistance, 0);
	NL3D_PROFILE_LAND_SET(ProfNSplitsPass, 0);

	if(!_RefineMode)
		return;

	// Update the priority list.
	// ==========================
	CTessFacePListNode		rootSplitTessFaceToUpdate;
	CTessFacePListNode		rootMergeTessFaceToUpdate;
	if( !_OldRefineCenterSetuped )
	{
		// If never refine, and setup OldRefineCetner
		_OldRefineCenterSetuped= true;
		_OldRefineCenter= refineCenter;

		// then shift all faces
		_SplitPriorityList.shiftAll(rootSplitTessFaceToUpdate);
		_MergePriorityList.shiftAll(rootMergeTessFaceToUpdate);
	}
	else
	{
		// else, compute delta between positions
		float	dist= (refineCenter - _OldRefineCenter).norm();
		_OldRefineCenter= refineCenter;

		// and shift according to distance of deplacement.
		_SplitPriorityList.shift(dist, rootSplitTessFaceToUpdate);
		_MergePriorityList.shift(dist, rootMergeTessFaceToUpdate);
	}


	// Refine Faces which may need it.
	// ==========================
	// Update globals
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: refine may change vertices in VB in visible patchs => buffers are locked.

	// Increment the update date.
	CLandscapeGlobals::CurrentDate++;

	/* While there is still face in list, update them
		NB: updateRefine() always insert the face in _***PriorityList, so face is removed from 
		root***TessFaceToUpdate list.
		NB: it is possible ( with enforced merge() ) that faces dissapears from root***TessFaceToUpdate list 
		before they are traversed here. It is why we must use a Circular list system, and not an array of elements.
		Basically. TessFaces are ALWAYS in a list, either in one of the entry list in _***PriorityList, or in
		root***TessFaceToUpdate list.

		It is newTessFace() and deleteTessFace() which insert/remove the nodes in the list.
	*/
	// Update the Merge priority list.
	while( rootMergeTessFaceToUpdate.nextInPList() )
	{
		// Get the face.
		CTessFace	*face= static_cast<CTessFace*>(rootMergeTessFaceToUpdate.nextInPList());

		// update the refine of this face. This may lead in deletion (merge) of other faces which are still in 
		// root***TessFaceToUpdate, but it's work.
		face->updateRefineMerge();
	}


	// Update the Split priority list.
	do
	{
		NL3D_PROFILE_LAND_ADD(ProfNSplitsPass, 1);

		// Append the new leaves, to the list of triangles to update
		rootSplitTessFaceToUpdate.appendPList(_RootNewLeaves);

		// While triangle to test for split exists
		while( rootSplitTessFaceToUpdate.nextInPList() )
		{
			// Get the face.
			CTessFace	*face= static_cast<CTessFace*>(rootSplitTessFaceToUpdate.nextInPList());

			// update the refine of this face.
			face->updateRefineSplit();
		}

	}
	// do it until we are sure no more split is needed, ie no more faces are created
	while( _RootNewLeaves.nextInPList() );


	// Before unlockBuffers, test for vegetable IG creation.
	// Because CLandscapeVegetableBlock::update() use OptFastFloor..
	OptFastFloorBegin();

	// For each vegetableBlock, test IG creation
	CLandscapeVegetableBlock	*vegetBlock= _VegetableBlockList.begin();
	for(;vegetBlock!=NULL; vegetBlock= (CLandscapeVegetableBlock*)vegetBlock->Next)
	{
		vegetBlock->update(refineCenter, _VegetableManager);
	}

	// update lighting for vegetables
	_VegetableManager->updateLighting();

	// Stop fastFloor optim.
	OptFastFloorEnd();


	// Must realase VB Buffers
	unlockBuffers();

	// refine() may cause change in faces in visible patchs.
	updateTessBlocksFaceVector();

}


// ***************************************************************************
void			CLandscape::refineAll(const CVector &refineCenter)
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: refineAll may change vertices in VB in visible patchs => buffers are locked.

	// Increment the update date.
	CLandscapeGlobals::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->refineAll();
	}

	// Must realase VB Buffers
	unlockBuffers();

	// refineAll() may cause change in faces in visible patchs.
	updateTessBlocksFaceVector();
}


// ***************************************************************************
void			CLandscape::excludePatchFromRefineAll(sint zoneId, uint patch, bool exclude)
{
	ItZoneMap it= Zones.find(zoneId);
	if(it!=Zones.end())
	{
		it->second->excludePatchFromRefineAll(patch, exclude);
	}

}


// ***************************************************************************
void			CLandscape::averageTesselationVertices()
{
	// -1. Update globals
	updateGlobalsAndLockBuffers (CVector::Null);
	// NB: averageTesselationVertices may change vertices in VB in visible patchs => buffers are locked.

	// Increment the update date.
	CLandscapeGlobals::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->averageTesselationVertices();
	}

	// Must realase VB Buffers
	unlockBuffers();

	// averageTesselationVertices() should not cause change in faces in any patchs.
	// It should not happens, but check for security.
	nlassert(_TessBlockModificationRoot.getNextToModify()==NULL);
	updateTessBlocksFaceVector();

}



// ***************************************************************************
void			CLandscape::updateGlobalsAndLockBuffers (const CVector &refineCenter)
{
	// Setup CLandscapeGlobals static members...

	// Far limits.
	CLandscapeGlobals::FarTransition= _FarTransition;

	// Tile subdivsion part.
	CLandscapeGlobals::TileMaxSubdivision= _TileMaxSubdivision;
	CLandscapeGlobals::TileDistNear = _TileDistNear;
	CLandscapeGlobals::TileDistFar = CLandscapeGlobals::TileDistNear+20;
	CLandscapeGlobals::TileDistNearSqr = sqr(CLandscapeGlobals::TileDistNear);
	CLandscapeGlobals::TileDistFarSqr = sqr(CLandscapeGlobals::TileDistFar);
	CLandscapeGlobals::OOTileDistDeltaSqr = 1.0f / (CLandscapeGlobals::TileDistFarSqr - CLandscapeGlobals::TileDistNearSqr);

	// Tile Pixel size part.
	// \todo yoyo: choose according to wanted tile pixel size.
	CLandscapeGlobals::TilePixelSize= 128.0f;

	// RefineThreshold.
	CLandscapeGlobals::RefineThreshold= _Threshold;

	if (_Threshold == 0.0f)
		CLandscapeGlobals::OORefineThreshold = FLT_MAX;
	else
		CLandscapeGlobals::OORefineThreshold = 1.0f / CLandscapeGlobals::RefineThreshold;

	// Refine Center*.
	CLandscapeGlobals::RefineCenter= refineCenter;
	CLandscapeGlobals::TileFarSphere.Center= CLandscapeGlobals::RefineCenter;
	CLandscapeGlobals::TileFarSphere.Radius= CLandscapeGlobals::TileDistFar;
	CLandscapeGlobals::TileNearSphere.Center= CLandscapeGlobals::RefineCenter;
	CLandscapeGlobals::TileNearSphere.Radius= CLandscapeGlobals::TileDistNear;

	// PZBModelPosition
	CLandscapeGlobals::PZBModelPosition= _PZBModelPosition;

	// VB Allocators.
	CLandscapeGlobals::CurrentFar0VBAllocator= &_Far0VB;
	CLandscapeGlobals::CurrentFar1VBAllocator= &_Far1VB;
	CLandscapeGlobals::CurrentTileVBAllocator= &_TileVB;

	// Must check driver, and create VB infos,locking buffers.
	if(_Driver)
	{
		_Far0VB.updateDriver(_Driver);
		_Far1VB.updateDriver(_Driver);
		_TileVB.updateDriver(_Driver);

		// must do the same for _VegetableManager.
		if(_DriverOkForVegetable)
			_VegetableManager->updateDriver(_Driver);

		lockBuffers ();
	}
}


// ***************************************************************************
void			CLandscape::lockBuffers ()
{
	_Far0VB.lockBuffer(CLandscapeGlobals::CurrentFar0VBInfo);
	_Far1VB.lockBuffer(CLandscapeGlobals::CurrentFar1VBInfo);
	_TileVB.lockBuffer(CLandscapeGlobals::CurrentTileVBInfo);

	// lock buffer of the vegetable manager.
	_VegetableManager->lockBuffers();

	// VertexProgrma mode???
	CLandscapeGlobals::VertexProgramEnabled= _VertexShaderOk;
}


// ***************************************************************************
void			CLandscape::unlockBuffers()
{
	_Far0VB.unlockBuffer();
	_Far1VB.unlockBuffer();
	_TileVB.unlockBuffer();

	// unlock buffer of the vegetable manager.
	_VegetableManager->unlockBuffers();
}


// ***************************************************************************
void			CLandscape::updateTessBlocksFaceVector()
{
	// while some tessBlock to update remains.
	CTessBlock	*tb;
	while( (tb=_TessBlockModificationRoot.getNextToModify()) !=NULL )
	{
		// Get the patch which owns this TessBlock.
		CPatch	*patch= tb->getPatch();

		// If this patch is visible, recreate faceVector for his tessBlock.
		patch->recreateTessBlockFaceVector(*tb);

		// remove from list.
		tb->removeFromModifyList();
	}
}


// ***************************************************************************
static inline void	drawPassTriArray(CMaterial &mat)
{
	if(CLandscapeGlobals::PassNTri>0)
	{
		CLandscapeGlobals::PatchCurrentDriver->setupMaterial(mat);
		CLandscapeGlobals::PatchCurrentDriver->renderSimpleTriangles(&CLandscapeGlobals::PassTriArray[0], CLandscapeGlobals::PassNTri);
		CLandscapeGlobals::PassNTri= 0;
	}
}


// ***************************************************************************
void			CLandscape::render(const CVector &refineCenter, const CVector &frontVector, const CPlane	pyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE], bool doTileAddPass)
{
	IDriver *driver= _Driver;
	nlassert(driver);

	// Increment the update date for preRender.
	CLandscapeGlobals::CurrentRenderDate++;


	ItZoneMap	it;
	sint		i;
	ItTileRdrPassSet	itTile;
	ItSPRenderPassSet	itFar;

	// Yoyo: profile.
	NL3D_PROFILE_LAND_SET(ProfNRdrFar0, 0);
	NL3D_PROFILE_LAND_SET(ProfNRdrFar1, 0);
	for(i=0;i<NL3D_MAX_TILE_PASS;i++)
	{
		NL3D_PROFILE_LAND_SET(ProfNRdrTile[i], 0);
	}


	// -2. Update globals
	//====================
	updateGlobalsAndLockBuffers (refineCenter);
	// NB: render may change vertices in VB in visible patchs => buffers are locked.


	// -1. clear all PatchRenderPass renderList
	//===================

	// Fars.
	for(itFar= _FarRdrPassSet.begin(); itFar!= _FarRdrPassSet.end(); itFar++)
	{
		CPatchRdrPass	&pass= **itFar;
		// clear list.
		pass.clearAllRenderList();
	}

	// Tiles.
	for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
	{
		CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
		// clear list.
		pass.clearAllRenderList();
	}

	// Lightmaps.
	for(sint lightRdrPass=0; lightRdrPass<(sint)_TextureNears.size(); lightRdrPass++)
	{
		CPatchRdrPass	&pass= *_TextureNears[lightRdrPass];
		// clear list.
		pass.clearAllRenderList();
	}

	// 0. preRender pass.
	//===================

	// change Far0 / Far1.
	// Clip TessBlocks against pyramid and Far Limit.
	for(i=0; i<NL3D_TESSBLOCK_NUM_CLIP_PLANE; i++)
	{
		CTessBlock::CurrentPyramid[i]= pyramid[i];
	}
	// Update VB with change of Far0 / Far1.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->preRender();
	}


	// Reallocation Mgt. If any of the VB is reallocated, we must refill it entirely.
	// NB: all VBs are refilled entirely. It is not optimal (maybe 3* too slow), but reallocation are supposed
	// to be very rare.
	if( _Far0VB.reallocationOccurs() || _Far1VB.reallocationOccurs() || _TileVB.reallocationOccurs() )
		_RenderMustRefillVB= true;

	// VertexProgram dependency on RefineThreshold Management. If VertexShader, and if the refineThreshold has
	// changed since the last time, we must refill All the VB, because data are out of date.
	if( _VertexShaderOk && _VPThresholdChange )
	{
		_VPThresholdChange= false;
		_RenderMustRefillVB= true;
	}

	// If we must refill the VB (for any reason).
	if(_RenderMustRefillVB )
	{
		// Ok, ok, we refill All the VB with good data.
		_RenderMustRefillVB= false;

		// First reset the flag, so fillVB() will effectively fill the VB.
		_Far0VB.resetReallocation();
		_Far1VB.resetReallocation();
		_TileVB.resetReallocation();

		// Then recompute good VBInfo (those in CurrentVBInfo are false!!).
		// Do it by unlocking then re-locking Buffers.
		unlockBuffers();
		lockBuffers();

		// Finally, fill the VB for all patchs visible.
		for(it= Zones.begin();it!=Zones.end();it++)
		{
			if((*it).second->ClipResult==CZone::ClipOut)
				continue;
			for(sint i=0;i<(*it).second->getNumPatchs(); i++)
			{
				CPatch	*patch= (*it).second->getPatch(i);
				patch->fillVBIfVisible();
			}
		}
	}


	// If software GeoMorph / Alpha Transition (no VertexShader), do it now.
	if(!_VertexShaderOk)
	{
		// For all patch visible, compute geomoprh and alpha in software.
		for(it= Zones.begin();it!=Zones.end();it++)
		{
			if((*it).second->ClipResult==CZone::ClipOut)
				continue;
			for(sint i=0;i<(*it).second->getNumPatchs(); i++)
			{
				CPatch	*patch= (*it).second->getPatch(i);
				// If visible, compute Geomorph And Alpha
				patch->computeSoftwareGeomorphAndAlpha();
			}
		}

		/*
			Optim note: here, lot of vertices are 
				1/ geomorphed twice (vertices on edges of patchs)
				2/ vertices are geomorphed, but not used (because o the Tessblock clip), 
					because lot of vertices used by faces in small TessBlocks are still in MasterBlock.

			Some tries have been made to solve this, but result are even worse (2 times or more), because:
				1/
					- does not really matter edges of patchs (and corner) because the majority is in interior of patch.
					- in this case, we need to reset all the flags which is very costly (reparse all zones...) .
				2/ Except for the old CTessBlockEdge management which not solve all the thing, the other solution is
					to test all faces not clipped (on a per TessBlock basis), to compute only vertices needed.
					But in this cases, result are worse, maybe because there is 6 times more tests, and with bad BTB cache.
		*/
	}


	// Must realase VB Buffers Now!! The VBuffers are now OK!
	// NB: no parallelism is made between 3dCard and Fill of vertices.
	// We Suppose Fill of vertices is rare, and so do not need to be parallelized.
	unlockBuffers();


	// If VertexShader enabled, setup VertexProgram Constants.
	if(_VertexShaderOk)
	{
		// c[0..3] take the ModelViewProjection Matrix.
		driver->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);
		// c[4] take usefull constants.
		driver->setConstant(4, 0, 1, 0.5f, 0);
		// c[5] take RefineCenter
		driver->setConstant(5, &refineCenter);
		// c[6] take info for Geomorph trnasition to TileNear.
		driver->setConstant(6, CLandscapeGlobals::TileDistFarSqr, CLandscapeGlobals::OOTileDistDeltaSqr, 0, 0);
		// c[8..11] take the ModelView Matrix.
		driver->setConstantMatrix(8, IDriver::ModelView, IDriver::Identity);
		// c[12] take the current landscape Center / delta Pos to apply
		driver->setConstant(12, &_PZBModelPosition);
	}


	// 1. TileRender pass.
	//====================


	// Active VB.
	// ==================

	// Active the good VB, and maybe activate the VertexProgram N°0.
	_TileVB.activate(0);


	// Render.
	// ==================
	// Before any render call. Set the global driver used to render.
	CLandscapeGlobals::PatchCurrentDriver= driver;


	// Render Order. Must "invert", since initial order is NOT the render order. This is done because the lightmap pass
	// DO NOT have to do any renderTile(), since it is computed in RGB0 pass.
	nlassert(NL3D_MAX_TILE_PASS==5);
	static	sint	RenderOrder[NL3D_MAX_TILE_PASS]= {NL3D_TILE_PASS_RGB0, NL3D_TILE_PASS_RGB1, NL3D_TILE_PASS_RGB2,
		NL3D_TILE_PASS_LIGHTMAP, NL3D_TILE_PASS_ADD};
	// For ALL pass..
	for(i=0; i<NL3D_MAX_TILE_PASS; i++)
	{
		sint	passOrder= RenderOrder[i];


		// If VertexShader enabled, and if lightmap or post Add pass, must setup good VertexProgram
		if(_VertexShaderOk)
		{
			if(passOrder == NL3D_TILE_PASS_LIGHTMAP)
			{
				// Must activate the vertexProgram to take TexCoord2 to stage0
				_TileVB.activate(1);
			}
			else if(passOrder == NL3D_TILE_PASS_ADD)
			{
				// Must re-activate the standard VertexProgram
				_TileVB.activate(0);
			}
		}


		// Do add pass???
		if((passOrder==NL3D_TILE_PASS_ADD) && !doTileAddPass)
			continue;


		// Setup common material for this pass.
		//=============================
		// Default: Modulate envmode.
		TileMaterial.texEnvOpRGB(0, CMaterial::Modulate);
		TileMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		TileMaterial.texEnvArg1RGB(0, CMaterial::Previous, CMaterial::SrcColor);
		TileMaterial.texEnvOpAlpha(0, CMaterial::Modulate);
		TileMaterial.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);
		TileMaterial.texEnvArg1Alpha(0, CMaterial::Previous, CMaterial::SrcAlpha);

		// Copy from stage 0 to stage 1.
		TileMaterial.setTexEnvMode(1, TileMaterial.getTexEnvMode(0));

		// setup multitex / blending.
		if(passOrder==NL3D_TILE_PASS_RGB0)
		{
			// first pass, no blend.
			TileMaterial.setBlend(false);
		}
		else
		{
			TileMaterial.setBlend(true);
			switch(passOrder)
			{
				case NL3D_TILE_PASS_RGB1: 
				case NL3D_TILE_PASS_RGB2: 
					// alpha blending.
					TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);

					// Must use a special envmode for stage1: "separateAlpha"!!.
					// keep the color from previous stage.
					TileMaterial.texEnvOpRGB(1, CMaterial::Replace);
					TileMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
					// take the alpha from current stage.
					TileMaterial.texEnvOpAlpha(1, CMaterial::Replace);
					TileMaterial.texEnvArg0Alpha(1, CMaterial::Texture, CMaterial::SrcAlpha);
					break;
				case NL3D_TILE_PASS_LIGHTMAP: 
					// modulate.
					TileMaterial.setBlendFunc(CMaterial::zero, CMaterial::srccolor);
					break;
				case NL3D_TILE_PASS_ADD: 
					// Use srcalpha for src (and not ONE), since additive are blended with alpha gouraud/AlphaTexture
					// (and for MAYBE LATER smooth night transition).
					TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::one);

					// Must use a special envmode for stage1: "separateAlpha"!!.
					// NB: it still works if The RdrPass has no texture.
					// keep the color from previous stage.
					TileMaterial.texEnvOpRGB(1, CMaterial::Replace);
					TileMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
					// modulate the alpha of current stage with diffuse (for MAYBE LATER smooth night transition).
					TileMaterial.texEnvOpAlpha(1, CMaterial::Modulate);
					TileMaterial.texEnvArg0Alpha(1, CMaterial::Texture, CMaterial::SrcAlpha);
					TileMaterial.texEnvArg1Alpha(1, CMaterial::Diffuse, CMaterial::SrcAlpha);

					break;
				default: 
					nlstop;
			};
		}
		// Reset the textures (so there is none in Addtive pass or in Lightmap).
		TileMaterial.setTexture(0, NULL);
		TileMaterial.setTexture(1, NULL);
		TileMaterial.setTexture(2, NULL);


		// Render All material RdrPass.
		//=============================
		// Special code for Lightmap and RGB0, for faster render.
		if(passOrder==NL3D_TILE_PASS_RGB0)
		{
			// RGB0 pass.
			ItTileRdrPassSet	itTile;
			for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
			{
				// Get a ref on the render pass. Const cast work because we only modify attribut from CPatchRdrPass 
				// that don't affect the operator< of this class
				CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);

				// Setup Diffuse texture of the tile.
				TileMaterial.setTexture(0, pass.TextureDiffuse);

				// Add triangles to array
				CRdrTileId		*tileToRdr= pass.getRdrTileRoot(passOrder);
				while(tileToRdr)
				{
					// renderSimpleTriangles() with the material setuped.
					tileToRdr->TileMaterial->renderTilePassRGB0();
					tileToRdr= (CRdrTileId*)tileToRdr->getNext();
				}
				// Render triangles.
				drawPassTriArray(TileMaterial);
			}
		}
		else if(passOrder==NL3D_TILE_PASS_LIGHTMAP)
		{
			// Lightmap Pass.
			/* \todo yoyo: TODO_CLOUD: setup stage2, and setup texcoord generation. COMPLEX because of interaction
			 with Dynamic LightMap
			*/

			// Setup the Dynamic Lightmap into stage 0.
			TileMaterial.setTexture(0, _TextureDLM);
			// Setup the material envCombine so DynamicLightmap (stage 0) is added to static lightmap.
			TileMaterial.texEnvOpRGB(1, CMaterial::Add);
			TileMaterial.texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
			TileMaterial.texEnvArg1RGB(1, CMaterial::Previous, CMaterial::SrcColor);

			// if vertex shader not used.
			if(!_VertexShaderOk)
			{
				// special setup  such that stage0 takes Uv2.
				driver->mapTextureStageToUV(0, 2);
			}


			// Render All the lightmaps.
			for(sint lightRdrPass=0; lightRdrPass<(sint)_TextureNears.size(); lightRdrPass++)
			{
				CPatchRdrPass	&pass= *_TextureNears[lightRdrPass];

				// Setup Lightmap into stage1. Because we share UV with RGB0. So we use UV1.
				// Cloud will be placed into stage0, and texture coordinate will be generated by T&L.
				TileMaterial.setTexture(1, pass.TextureDiffuse);

				// Add triangles to array
				CRdrTileId		*tileToRdr= pass.getRdrTileRoot(passOrder);
				while(tileToRdr)
				{
					// renderSimpleTriangles() with the material setuped.
					tileToRdr->TileMaterial->renderTilePassLightmap();
					tileToRdr= (CRdrTileId*)tileToRdr->getNext();
				}
				// Render triangles.
				drawPassTriArray(TileMaterial);
			}

			// if vertex shader not used.
			if(!_VertexShaderOk)
			{
				// Reset special stage/UV setup to normal behavior
				driver->mapTextureStageToUV(0, 0);
			}
		}
		else
		{
			// RGB1, RGB2, and ADD pass.

			// Render Base, Transitions or Additives.

			ItTileRdrPassSet	itTile;
			for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
			{
				// Get a ref on the render pass. Const cast work because we only modify attribut from CPatchRdrPass 
				// that don't affect the operator< of this class
				CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);

				// Add triangles to array
				CRdrTileId		*tileToRdr= pass.getRdrTileRoot(passOrder);
				while(tileToRdr)
				{
					// renderSimpleTriangles() with the material setuped.
					tileToRdr->TileMaterial->renderTile(passOrder);
					tileToRdr= (CRdrTileId*)tileToRdr->getNext();
				}

				// Pass not empty ?
				if(CLandscapeGlobals::PassNTri>0)
				{
					// Setup material.
					// Setup Diffuse texture of the tile.
					TileMaterial.setTexture(0, pass.TextureDiffuse);
					
					// If transition tile, must enable the alpha for this pass.
					// NB: Additive pass may have pass.TextureAlpha==NULL
					TileMaterial.setTexture(1, pass.TextureAlpha);
				}

				// Render triangles.
				drawPassTriArray(TileMaterial);
			}
		}
	}


	// 2. Far0Render pass.
	//====================

	// Active VB.
	// ==================

	// Active the good VB, and maybe activate the std VertexProgram.
	_Far0VB.activate(0);


	// Render.
	// ==================

	// Setup common material.
	FarMaterial.setBlend(false);

	// Render All material RdrPass0.
	itFar=_FarRdrPassSet.begin();
	while (itFar!=_FarRdrPassSet.end())
	{
		CPatchRdrPass	&pass= **itFar;

		// Setup the material.
		FarMaterial.setTexture(0, pass.TextureDiffuse);
		// If the texture need to be updated, do it now.
		if(pass.TextureDiffuse && pass.TextureDiffuse->touched())
			driver->setupTexture(*pass.TextureDiffuse);

		// Add triangles to array
		CRdrPatchId		*patchToRdr= pass.getRdrPatchFar0();
		while(patchToRdr)
		{
			// renderSimpleTriangles() with the material setuped.
			patchToRdr->Patch->renderFar0();
			patchToRdr= (CRdrPatchId*)patchToRdr->getNext();
		}
		// Render triangles.
		drawPassTriArray(FarMaterial);

		// Next render pass
		itFar++;
	}



	// 3. Far1Render pass.
	//====================

	// Active VB.
	// ==================

	// Active the good VB, and maybe activate the std VertexProgram.
	_Far1VB.activate(0);


	// Render
	// ==================

	// Setup common material.
	FarMaterial.setBlend(true);


	// Render All material RdrPass1.
	itFar=_FarRdrPassSet.begin();
	while (itFar!=_FarRdrPassSet.end())
	{
		CPatchRdrPass	&pass= **itFar;

		// Setup the material.
		FarMaterial.setTexture(0, pass.TextureDiffuse);
		// If the texture need to be updated, do it now.
		if(pass.TextureDiffuse && pass.TextureDiffuse->touched())
			driver->setupTexture(*pass.TextureDiffuse);

		// Add triangles to array
		CRdrPatchId		*patchToRdr= pass.getRdrPatchFar1();
		while(patchToRdr)
		{
			// renderSimpleTriangles() with the material setuped.
			patchToRdr->Patch->renderFar1();
			patchToRdr= (CRdrPatchId*)patchToRdr->getNext();
		}
		// Render triangles.
		drawPassTriArray(FarMaterial);

		// Next render pass
		itFar++;
	}


	// 4. "Release" texture materials.
	//================================
	FarMaterial.setTexture(0, NULL);
	FarMaterial.setTexture(1, NULL);
	FarMaterial.setTexture(2, NULL);
	FarMaterial.setTexture(3, NULL);
	TileMaterial.setTexture(0, NULL);
	TileMaterial.setTexture(1, NULL);
	TileMaterial.setTexture(2, NULL);
	TileMaterial.setTexture(3, NULL);

	// To ensure no use but in render()..
	CLandscapeGlobals::PatchCurrentDriver= NULL;

	// Desactive the vertex program (if anyone)
	if(_VertexShaderOk)
		driver->activeVertexProgram (NULL);


	// 5. Vegetable Management.
	//================================

	// profile.
	_VegetableManager->resetNumVegetableFaceRendered();

	// render all vegetables, only if driver support VertexProgram.
	if(isVegetableActive())
	{
		// Use same plane as TessBlock for faster clipping.
		vector<CPlane>		vegetablePyramid;
		vegetablePyramid.resize(NL3D_TESSBLOCK_NUM_CLIP_PLANE);
		for(i=0;i<NL3D_TESSBLOCK_NUM_CLIP_PLANE;i++)
		{
			vegetablePyramid[i]= pyramid[i];
		}
		_VegetableManager->render(refineCenter, frontVector, vegetablePyramid, driver);
	}

}


// ***************************************************************************
// ***************************************************************************
// Tile mgt.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
ITexture		*CLandscape::findTileTexture(const std::string &textName)
{
	ITexture	*text;
	text= TileTextureMap[textName];
	// If just inserted, RefPtr is NULL!!  :)
	// This test too if the RefPtr is NULL... (tile released)
	// The object is not owned by this map. It will be own by the multiple RdrPass via CSmartPtr.
	// They will destroy it when no more points to them.
	if(!text)
	{
		TileTextureMap[textName]= text= new CTextureFile(textName);
		text->setWrapS(ITexture::Clamp);
		text->setWrapT(ITexture::Clamp);
		text->setUploadFormat(ITexture::DXTC5);
	}
	return text;
}


// ***************************************************************************
CPatchRdrPass	*CLandscape::findTileRdrPass(const CPatchRdrPass &pass)
{
	ItTileRdrPassSet	it;
	// If already here, find it, else insert.
	it= (TileRdrPassSet.insert(pass)).first;

	return const_cast<CPatchRdrPass*>(&(*it));
}


// ***************************************************************************
void			CLandscape::loadTile(uint16 tileId)
{
	CTile		*tile;
	CTileInfo	*tileInfo;
	string		textName;

	// Retrieve or create texture.
	// ===========================
	// Tile Must exist.
	nlassert(tileId==0xFFFF || tileId<TileBank.getTileCount());
	if(tileId<TileBank.getTileCount())
		tile= TileBank.getTile(tileId);
	else
		tile= NULL;
	// TileInfo must not exist.
	nlassert(TileInfos[tileId]==NULL);
	TileInfos[tileId]= tileInfo= new CTileInfo;

	// Fill additive part.
	// ===================
	if(tile)
		textName= tile->getRelativeFileName(CTile::additive);
	else
		textName= "";
	// If no additive for this tile, rdrpass is NULL.
	if(textName=="")
		tileInfo->AdditiveRdrPass= NULL;
	else
	{
		// Fill rdrpass.
		CPatchRdrPass	pass;
		pass.TextureDiffuse= findTileTexture(TileBank.getAbsPath()+textName);

		// We may have an alpha part for additive.
		textName= tile->getRelativeFileName (CTile::alpha);
		if(textName!="")
			pass.TextureAlpha= findTileTexture(TileBank.getAbsPath()+textName);

		// Fill tileInfo.
		tileInfo->AdditiveRdrPass= findTileRdrPass(pass);
		// Fill UV Info.
		// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
		tileInfo->AdditiveUvScaleBias.x= 0;
		tileInfo->AdditiveUvScaleBias.y= 0;
		tileInfo->AdditiveUvScaleBias.z= 1;
	}


	// Fill diffuse part.
	// =======================
	// Fill rdrpass.
	CPatchRdrPass	pass;
	// The diffuse part for a tile is inevitable.
	if(tile)
	{
		textName= tile->getRelativeFileName(CTile::diffuse);
		if(textName!="")
			pass.TextureDiffuse= findTileTexture(TileBank.getAbsPath()+textName);
		else
		{
			pass.TextureDiffuse= new CTextureCross;
			nldebug("Missing Tile diffuse texname: %d", tileId);
		}
	}
	else
		pass.TextureDiffuse= new CTextureCross;
	if(tile)
	{
		textName= tile->getRelativeFileName (CTile::alpha);
		if(textName!="")
			pass.TextureAlpha= findTileTexture(TileBank.getAbsPath()+textName);
	}


	// Fill tileInfo.
	tileInfo->DiffuseRdrPass= findTileRdrPass(pass);
	// Fill UV Info.
	// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
	tileInfo->DiffuseUvScaleBias.x= 0;
	tileInfo->DiffuseUvScaleBias.y= 0;
	tileInfo->DiffuseUvScaleBias.z= 1;
	tileInfo->AlphaUvScaleBias.x= 0;
	tileInfo->AlphaUvScaleBias.y= 0;
	tileInfo->AlphaUvScaleBias.z= 1;
	// Retrieve the good rot alpha decal.
	if(tile)
		tileInfo->RotAlpha= tile->getRotAlpha();
	else
		tileInfo->RotAlpha= 0;


	// Increment RefCount of RenderPart.
	// =================================
	if(tileInfo->AdditiveRdrPass)
		tileInfo->AdditiveRdrPass->RefCount++;
	if(tileInfo->DiffuseRdrPass)
		tileInfo->DiffuseRdrPass->RefCount++;

}


// ***************************************************************************
void			CLandscape::releaseTile(uint16 tileId)
{
	CTileInfo	*tileInfo;
	tileInfo= TileInfos[tileId];
	nlassert(tileInfo!=NULL);

	// "Release" the rdr pass.
	if(tileInfo->AdditiveRdrPass)
		tileInfo->AdditiveRdrPass->RefCount--;
	if(tileInfo->DiffuseRdrPass)
		tileInfo->DiffuseRdrPass->RefCount--;

	delete tileInfo;
	TileInfos[tileId]= NULL;
}


// ***************************************************************************
CPatchRdrPass	*CLandscape::getTileRenderPass(uint16 tileId, bool additiveRdrPass)
{
	CTileInfo	*tile= TileInfos[tileId];

	// If not here, create it.
	//========================
	if(tile==NULL)
	{
		// Force loading of tile.
		loadTile(tileId);

		tile= TileInfos[tileId];
		nlassert(tile!=NULL);
	}

	// Retrieve.
	//========================
	if(additiveRdrPass)
	{
		// NB: additive pass is not lighted by the lightmap, so there is no lighted version of this rednerpass.
		return tile->AdditiveRdrPass;
	}
	else
	{
		return tile->DiffuseRdrPass;
	}
}


// ***************************************************************************
void			CLandscape::getTileUvScaleBiasRot(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias, uint8 &rotAlpha)
{
	CTileInfo	*tile= TileInfos[tileId];
	// tile should not be NULL.
	// Because load of tiles are always done in getTileRenderPass(), and this insertion always succeed.
	nlassert(tile);

	rotAlpha= 0;
	switch(bitmapType)
	{
		case CTile::diffuse:
			uvScaleBias= tile->DiffuseUvScaleBias; break;
		case CTile::additive:
			uvScaleBias= tile->AdditiveUvScaleBias; break;
		case CTile::alpha:
			uvScaleBias= tile->AlphaUvScaleBias; 
			rotAlpha= tile->RotAlpha;
			break;
	}
}


// ***************************************************************************
NLMISC::CSmartPtr<ITexture>		CLandscape::getTileTexture(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias)
{
	CPatchRdrPass	*pass;
	if(bitmapType== CTile::additive)
		pass= getTileRenderPass(tileId, true);
	else
		pass= getTileRenderPass(tileId, false);
	if(!pass)
		return NULL;
	uint8	dummy;
	getTileUvScaleBiasRot(tileId, bitmapType, uvScaleBias, dummy);

	// return the wanted texture.
	if(bitmapType==CTile::diffuse || bitmapType==CTile::additive)
		return pass->TextureDiffuse;
	else
		return pass->TextureAlpha;
}


// ***************************************************************************
void			CLandscape::flushTiles(IDriver *drv, uint16 tileStart, uint16 nbTiles)
{
	// Load tile rdrpass, force setup the texture.
	for(sint tileId= tileStart; tileId<tileStart+nbTiles; tileId++)
	{
		CTileInfo	*tile= TileInfos[tileId];
		if(tile==NULL)
		{
			loadTile(tileId);
		}
	}

	// For all rdrpass, force setup the texture.
	ItTileRdrPassSet	it;
	for(it= TileRdrPassSet.begin(); it!=TileRdrPassSet.end(); it++)
	{
		const CPatchRdrPass	&pass= *it;
		// If present and not already setuped...
		if(pass.TextureDiffuse && !pass.TextureDiffuse->setupedIntoDriver())
			drv->setupTexture(*pass.TextureDiffuse);
		// If present and not already setuped...
		if(pass.TextureAlpha && !pass.TextureAlpha->setupedIntoDriver())
			drv->setupTexture(*pass.TextureAlpha);
	}
}


// ***************************************************************************
void			CLandscape::releaseTiles(uint16 tileStart, uint16 nbTiles)
{
	// release tiles.
	for(sint tileId= tileStart; tileId<tileStart+nbTiles; tileId++)
	{
		CTileInfo	*tile= TileInfos[tileId];
		if(tile!=NULL)
		{
			releaseTile(tileId);
		}
	}

	// For all rdrpass, release one that are no more referenced.
	ItTileRdrPassSet	it;
	for(it= TileRdrPassSet.begin(); it!=TileRdrPassSet.end();)
	{
		// If no more tile access the rdrpass, delete it.
		if((*it).RefCount==0)
		{
			ItTileRdrPassSet itDel=it++;
			TileRdrPassSet.erase(itDel);
		}
		else
			it++;
	}

	// Textures are automaticly deleted by smartptr, but not their entry int the map (TileTextureMap). 
	// => doesn't matter since findTileTexture() manages this case.
	// And the memory overhead is not a problem (we talk about pointers).
}


// ***************************************************************************
uint		CLandscape::getTileLightMap(CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE], CPatchRdrPass *&lightmapRdrPass)
{
	sint	textNum;
	uint	lightMapId;
	/* 
		NB: TextureNear are a grow only Array... TextureNear are never deleted. Why? :
		2/ Unused near texture may be uncahced by opengl (and maybe by windows, to disk).

	  (old reason, no longer valid, since lightmaps are unlinked from tiles.
		1/ There is an important issue with releasing texture nears: tiles may acces them (see getTileRenderPass())
	  )
	*/
	// 0. Alloc Near Texture if necessary.
	//====================================
	if(_NFreeLightMaps==0)
	{
		CTextureNear	*text= new CTextureNear(TextureNearSize);
		TSPRenderPass	newPass= new CPatchRdrPass;

		newPass->TextureDiffuse= text;

		_TextureNears.push_back(newPass);
		_NFreeLightMaps+= text->getNbAvailableTiles();
	}

	// 1. Search the first texture which has a free tile.
	//==================================================
	CTextureNear	*nearText= NULL;
	CPatchRdrPass	*nearRdrPass= NULL;
	for(textNum=0;textNum<(sint)_TextureNears.size();textNum++)
	{
		nearRdrPass= _TextureNears[textNum];
		nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;
		if(nearText->getNbAvailableTiles()!=0)
			break;
	}
	nlassert(textNum<(sint)_TextureNears.size());
	// A empty space has been found.
	_NFreeLightMaps--;

	// 2. Fill the texture with the data, and updaterect.
	//===================================================
	lightMapId= nearText->getTileAndFillRect(map);
	// Compute the Id.
	lightMapId= textNum*NbTileLightMapByTexture + lightMapId;


	// 3. updateLighting
	//===================================================
	// Increment number of pixels to update for near.
	_ULTotalNearPixels+= NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE;


	// Result:
	lightmapRdrPass= nearRdrPass;
	return lightMapId;
}
// ***************************************************************************
void		CLandscape::getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias)
{
	uint	id, s,t;

	// Scale.
	static const float	scale10= (float)NL_TILE_LIGHTMAP_SIZE/TextureNearSize;
	static const float	scale4= 4.f/TextureNearSize;
	static const float	scale1= 1.f/TextureNearSize;
	// The size of a minilightmap, mapped onto the polygon, is still 4 pixels.
	uvScaleBias.z= scale4;

	// Get the id local in the texture.
	id= tileLightMapId%NbTileLightMapByTexture;

	// Commpute UVBias.
	// Get the coordinate of the tile, in tile number.
	s= id%NbTileLightMapByLine;
	t= id/NbTileLightMapByLine;
	// But the real size of a minilightmap is 10 pixels, and we must reach the pixel 1,1.
	uvScaleBias.x= s*scale10 + scale1;
	uvScaleBias.y= t*scale10 + scale1;
}
// ***************************************************************************
void		CLandscape::releaseTileLightMap(uint tileLightMapId)
{
	uint	id, textNum;

	// Get the id local in the texture.
	textNum= tileLightMapId / NbTileLightMapByTexture;
	id= tileLightMapId % NbTileLightMapByTexture;
	nlassert(textNum>=0 && textNum<_TextureNears.size());

	// Release the tile in this texture.
	CPatchRdrPass	*nearRdrPass= _TextureNears[textNum];
	CTextureNear	*nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;
	nearText->releaseTile(id);
	_NFreeLightMaps++;

	// updateLighting
	// Decrement number of pixels to update for near.
	_ULTotalNearPixels-= NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE;
}


// ***************************************************************************
void		CLandscape::refillTileLightMap(uint tileLightMapId, CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE])
{
	uint	id, textNum;

	// Get the id local in the texture.
	textNum= tileLightMapId / NbTileLightMapByTexture;
	id= tileLightMapId % NbTileLightMapByTexture;
	nlassert(textNum>=0 && textNum<_TextureNears.size());

	// get a ptr on the texture.
	CPatchRdrPass	*nearRdrPass= _TextureNears[textNum];
	CTextureNear	*nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;

	// refill this tile
	nearText->refillRect(id, map);
}



// ***************************************************************************
// ***************************************************************************
// Far.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CPatchRdrPass*	CLandscape::getFarRenderPass(CPatch* pPatch, uint farIndex, float& farUScale, float& farVScale, float& farUBias, float& farVBias, bool& bRot)
{
	// Check args
	nlassert (farIndex>0);

	// Get size of the far texture
	uint width=(pPatch->getOrderS ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);
	uint height=(pPatch->getOrderT ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);

	// For updateLighting: increment total of pixels to update.
	_ULTotalFarPixels+= width*height;

	// Render pass index
	uint passIndex=getRdrPassIndexWithSize (width, height);

	// Look for a free render pass
	if (_FarRdrPassSetVectorFree[passIndex].begin()==_FarRdrPassSetVectorFree[passIndex].end())
	{
		// Empty, add a new render pass
		CPatchRdrPass	*pass=new CPatchRdrPass;

		// Fill the render pass
		CTextureFar *pTextureFar=new CTextureFar;

		// Append this textureFar to the list of TextureFar to updateLighting.
		if(_ULRootTextureFar==NULL)
			_ULRootTextureFar= pTextureFar;
		else
			pTextureFar->linkBeforeUL(_ULRootTextureFar);

		// Set the bank
		pTextureFar->_Bank=&TileFarBank;

		// Set as diffuse texture for this renderpass
		pass->TextureDiffuse=pTextureFar;

		// Set the size for this texture
		pTextureFar->setSizeOfFarPatch (std::max (width, height), std::min (width, height));

		// Add the render pass
		_FarRdrPassSetVectorFree[passIndex].insert (pass);
		_FarRdrPassSet.insert (pass);
	}

	// Ok, add the patch to the first render pass in the free list
	TSPRenderPass pass=*_FarRdrPassSetVectorFree[passIndex].begin();

	// Get a pointer on the diffuse far texture
	CTextureFar *pTextureFar=(CTextureFar*)(&*(pass->TextureDiffuse));

	// Add the patch to the far texture
	if (pTextureFar->addPatch (pPatch, farUScale, farVScale, farUBias, farVBias, bRot))
	{
		// The render state is full, remove from the free list..
		_FarRdrPassSetVectorFree[passIndex].erase (pass);
	}

	// Return the renderpass
	return pass;
}


// ***************************************************************************
void		CLandscape::freeFarRenderPass (CPatch* pPatch, CPatchRdrPass* pass, uint farIndex)
{
	// Get size of the far texture
	uint width=(pPatch->getOrderS ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);
	uint height=(pPatch->getOrderT ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);

	// For updateLighting: decrement total of pixels to update.
	_ULTotalFarPixels-= width*height;
	nlassert(_ULTotalFarPixels>=0);

	// Render pass index
	uint passIndex=getRdrPassIndexWithSize (width, height);

	// Get a pointer on the diffuse far texture
	CTextureFar *pTextureFar=(CTextureFar*)(&*(pass->TextureDiffuse));

	// Remove from the patch from the texture if empty
	if (pTextureFar->removePatch (pPatch))
	{
		// Free list empty ?
		if (_FarRdrPassSetVectorFree[passIndex].begin()==_FarRdrPassSetVectorFree[passIndex].end())
		{
			// Let this render pass in the free list
			_FarRdrPassSetVectorFree[passIndex].insert (pass);
		}
		else
		{
			// Release for good
			_FarRdrPassSetVectorFree[passIndex].erase (pass);

			// update UL links, and Remove from draw list
			eraseFarRenderPassFromSet (pass);
		}
	}
	else
	{
		// Insert in the free list
		_FarRdrPassSetVectorFree[passIndex].insert (pass);
	}
}


// ***************************************************************************
void		CLandscape::eraseFarRenderPassFromSet (CPatchRdrPass* pass)
{
	// Before deleting, must remove TextureFar from UpdateLighting list.

	// Get a pointer on the diffuse far texture
	CTextureFar *pTextureFar=(CTextureFar*)(&*(pass->TextureDiffuse));

	// If I delete the textureFar which is the current root
	if(_ULRootTextureFar==pTextureFar)
	{
		// switch to next
		_ULRootTextureFar= pTextureFar->getNextUL();
		// if still the same, it means that the circular list is now empty
		if(_ULRootTextureFar==pTextureFar)
			_ULRootTextureFar= NULL;
		// reset patch counter.
		_ULFarCurrentPatchId= 0;
	}

	// unlink the texture from list
	pTextureFar->unlinkUL();


	// Remove from draw list
	_FarRdrPassSet.erase (pass);
}


// ***************************************************************************
// ***************************************************************************
// Misc.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CZone*			CLandscape::getZone (sint zoneId)
{
	TZoneMap::iterator	it;
	it= Zones.find(zoneId);
	if (it!=Zones.end())
		return (*it).second;
	else
		return NULL;
}


// ***************************************************************************
const CZone*	CLandscape::getZone (sint zoneId) const
{
	TZoneMap::const_iterator	it;

	it= Zones.find(zoneId);
	if (it!=Zones.end())
		return (*it).second;
	else
		return NULL;
}



// ***************************************************************************
void			CLandscape::checkZoneBinds(CZone &curZone, EBadBind &bindError)
{
	for(sint i=0;i<curZone.getNumPatchs();i++)
	{
		const CZone::CPatchConnect	&pa= *curZone.getPatchConnect(i);

		// Check the bindInfos.
		for(sint j=0;j<4;j++)
		{
			const CPatchInfo::CBindInfo	&bd=pa.BindEdges[j];
			// Just 1/1 for now.
			if(bd.NPatchs==1)
			{
				CZone	*oZone= getZone(bd.ZoneId);
				// If loaded zone.
				if(oZone)
				{
					const CZone::CPatchConnect	&po= *(oZone->getPatchConnect(bd.Next[0]));
					const CPatchInfo::CBindInfo	&bo= po.BindEdges[bd.Edge[0]];
					if(bo.NPatchs!=1 || bo.Next[0]!=i || bo.Edge[0]!=j)
						bindError.BindErrors.push_back( EBadBind::CBindError(curZone.getZoneId(), i));
				}
			}
		}
	}
}


// ***************************************************************************
void			CLandscape::checkBinds() throw(EBadBind)
{
	EBadBind	bindError;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		CZone	&curZone= *(*it).second;
		checkZoneBinds(curZone, bindError);
	}

	if(!bindError.BindErrors.empty())
		throw bindError;
}


// ***************************************************************************
void			CLandscape::checkBinds(uint16 zoneId) throw(EBadBind)
{
	EBadBind	bindError;

	ItZoneMap it= Zones.find(zoneId);
	if(it!= Zones.end())
	{
		CZone	&curZone= *(*it).second;
		checkZoneBinds(curZone, bindError);
		if(!bindError.BindErrors.empty())
			throw bindError;
	}
}



// ***************************************************************************
void			CLandscape::addTrianglesInBBox(sint zoneId, sint patchId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const
{
	// No clear here, just add triangles to the array.
	std::map<uint16, CZone*>::const_iterator	it= Zones.find(zoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId>=0);
		nlassert(patchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId);

		CPatchIdent		paId;
		paId.ZoneId= zoneId;
		paId.PatchId= patchId;
		pa->addTrianglesInBBox(paId, bbox, triangles, tileTessLevel);
	}
}


// ***************************************************************************
void			CLandscape::buildTrianglesInBBox(const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel)
{
	// clear selection.
	triangles.clear();

	// search path of interest.
	_PatchQuadGrid.clearSelection();
	_PatchQuadGrid.select(bbox.getMin(), bbox.getMax());
	CQuadGrid<CPatchIdent>::CIterator	it;

	// for each patch, add triangles to the array.
	for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end(); it++)
	{
		addTrianglesInBBox((*it).ZoneId, (*it).PatchId, bbox, triangles, tileTessLevel);
	}
}



// ***************************************************************************
void			CLandscape::addPatchBlocksInBBox(sint zoneId, sint patchId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds)
{
	// No clear here, just add blocks to the array.
	std::map<uint16, CZone*>::const_iterator	it= Zones.find(zoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId>=0);
		nlassert(patchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId);

		CPatchIdent		paId;
		paId.ZoneId= zoneId;
		paId.PatchId= patchId;
		pa->addPatchBlocksInBBox(paId, bbox, paBlockIds);
	}
}


// ***************************************************************************
void			CLandscape::buildPatchBlocksInBBox(const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds)
{
	// clear selection.
	paBlockIds.clear();

	// search path of interest.
	_PatchQuadGrid.clearSelection();
	_PatchQuadGrid.select(bbox.getMin(), bbox.getMax());
	CQuadGrid<CPatchIdent>::CIterator	it;

	// for each patch, add blocks to the array.
	for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end(); it++)
	{
		addPatchBlocksInBBox((*it).ZoneId, (*it).PatchId, bbox, paBlockIds);
	}
}


// ***************************************************************************
void			CLandscape::fillPatchQuadBlock(CPatchQuadBlock &quadBlock) const
{
	sint zoneId=  quadBlock.PatchBlockId.PatchId.ZoneId;
	sint patchId= quadBlock.PatchBlockId.PatchId.PatchId;
	std::map<uint16, CZone*>::const_iterator	it= Zones.find(zoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId>=0);
		nlassert(patchId<N);

		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId);
		pa->fillPatchQuadBlock(quadBlock);
	}
}




// ***************************************************************************
void			CLandscape::buildCollideFaces(const CAABBoxExt &bbox, vector<CTriangle>	&faces, bool faceClip)
{
	CBSphere	bsWanted(bbox.getCenter(), bbox.getRadius());

	faces.clear();
	// For all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		const CAABBoxExt	&bb= (*it).second->getZoneBB();
		CBSphere	bs(bb.getCenter(), bb.getRadius());
		// If zone intersect the wanted area.
		//===================================
		if(bs.intersect(bsWanted))
		{
			// Then trace all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(i);

				// If patch in wanted area....
				//============================
				if(bsWanted.intersect(pa->getBSphere()))
				{
					// 0. Build the faces.
					//====================
					sint	ordS= pa->getOrderS();
					sint	ordT= pa->getOrderT();
					sint	x,y,j;
					vector<CTriangle>	tmpFaces;
					tmpFaces.reserve(ordS*ordT);
					float	OOS= 1.0f/ordS;
					float	OOT= 1.0f/ordT;
					for(y=0;y<ordT;y++)
					{
						for(x=0;x<ordS;x++)
						{
							CTriangle	f;
							f.V0= pa->computeVertex(x*OOS, y*OOT);
							f.V1= pa->computeVertex(x*OOS, (y+1)*OOT);
							f.V2= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
							tmpFaces.push_back(f);
							f.V0= pa->computeVertex(x*OOS, y*OOT);
							f.V1= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
							f.V2= pa->computeVertex((x+1)*OOS, y*OOT);
							tmpFaces.push_back(f);
						}
					}

					// 1. Clip the faces.
					//===================
					if(faceClip)
					{
						// Insert only faces which are In the area.
						for(j=0;j<(sint)tmpFaces.size();j++)
						{
							CTriangle	&f= tmpFaces[j];
							if(bbox.intersect(f.V0, f.V1, f.V2))
							{
								faces.push_back(f);
							}
						}
					}
					else
					{
						// Else insert ALL.
						faces.insert(faces.end(), tmpFaces.begin(), tmpFaces.end());
					}
				}
			}
		}
	}
}


// ***************************************************************************
void			CLandscape::buildCollideFaces(sint zoneId, sint patch, std::vector<CTriangle> &faces)
{
	faces.clear();

	ItZoneMap it= Zones.find(zoneId);
	if(it!=Zones.end())
	{
		// Then trace all patch.
		sint	N= (*it).second->getNumPatchs();
		nlassert(patch>=0);
		nlassert(patch<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patch);

		// Build the faces.
		//=================
		sint	ordS= pa->getOrderS();
		sint	ordT= pa->getOrderT();
		sint	x,y;
		float	OOS= 1.0f/ordS;
		float	OOT= 1.0f/ordT;
		for(y=0;y<ordT;y++)
		{
			for(x=0;x<ordS;x++)
			{
				CTriangle	f;
				f.V0= pa->computeVertex(x*OOS, y*OOT);
				f.V1= pa->computeVertex(x*OOS, (y+1)*OOT);
				f.V2= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
				faces.push_back(f);
				f.V0= pa->computeVertex(x*OOS, y*OOT);
				f.V1= pa->computeVertex((x+1)*OOS, (y+1)*OOT);
				f.V2= pa->computeVertex((x+1)*OOS, y*OOT);
				faces.push_back(f);
			}
		}
	}
}


// ***************************************************************************
CVector			CLandscape::getTesselatedPos(const CPatchIdent &patchId, const CUV &uv) const
{
	// First, must update globals, for CTessFace::computeTesselatedPos() to work properly.

	// VertexProgrma mode???
	CLandscapeGlobals::VertexProgramEnabled= _VertexShaderOk;

	// If VertexProgram enabled
	if( CLandscapeGlobals::VertexProgramEnabled )
	{
		/* because VertexProgram enabled, CTessVertex::Pos (geomorphed Pos) are not computed each frame
		   Hence, CTessFace::computeTesselatedPos() will call CTessVertex::computeGeomPos() to have correct 
		   CTessVertex::Pos. ThereFore we must setup globals so CTessVertex::computeGeomPos() works properly.
		*/
		
		// see copy in updateGlobalsAndLockBuffers(). NB: Just copy what needed here!!!!

		// Tile subdivsion part.
		CLandscapeGlobals::TileDistNear = _TileDistNear;
		CLandscapeGlobals::TileDistFar = CLandscapeGlobals::TileDistNear+20;
		CLandscapeGlobals::TileDistNearSqr = sqr(CLandscapeGlobals::TileDistNear);
		CLandscapeGlobals::TileDistFarSqr = sqr(CLandscapeGlobals::TileDistFar);
		CLandscapeGlobals::OOTileDistDeltaSqr = 1.0f / (CLandscapeGlobals::TileDistFarSqr - CLandscapeGlobals::TileDistNearSqr);

		// RefineThreshold.
		CLandscapeGlobals::RefineThreshold= _Threshold;
		CLandscapeGlobals::OORefineThreshold= 1.0f / CLandscapeGlobals::RefineThreshold;

		// Refine Center*.
		// NB: setup the last setuped refineCenter.
		CLandscapeGlobals::RefineCenter= _OldRefineCenter;
	}


	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId.PatchId);

		return pa->getTesselatedPos(uv);
	}
	else
		return CVector::Null;
}


// ***************************************************************************
uint			CLandscape::getRdrPassIndexWithSize (uint width, uint height)
{
	// Check no NULL size
	nlassert (width);
	nlassert (height);

	// Find width order
	int orderWidth=0;
	while (!(width&(1<<orderWidth)))
		orderWidth++;

	// Find heightorder
	int orderHeight=0;
	while (!(height&(1<<orderHeight)))
		orderHeight++;

	if (orderWidth>orderHeight)
	{
		return NL_MAX_SIZE_OF_TEXTURE_EDGE_SHIFT*orderHeight+orderWidth;
	}
	else
	{
		return NL_MAX_SIZE_OF_TEXTURE_EDGE_SHIFT*orderWidth+orderHeight;
	}
}

#define NL_TILE_FAR_SIZE_ORDER0 (NL_NUM_PIXELS_ON_FAR_TILE_EDGE*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)
#define NL_TILE_FAR_SIZE_ORDER1 ((NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>1)*(NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>1))
#define NL_TILE_FAR_SIZE_ORDER2 ((NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>2)*(NL_NUM_PIXELS_ON_FAR_TILE_EDGE>>2))

// ***************************************************************************
// internal use
bool			CLandscape::eraseTileFarIfNotGood (uint tileNumber, uint sizeOrder0, uint sizeOrder1, uint sizeOrder2)
{
	// The same tiles ?
	bool bSame=true;

	// It is the same tile ?
	if (TileFarBank.getTile (tileNumber)->isFill (CTileFarBank::diffuse))
	{
		// Good diffuse size ?
		if (
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::diffuse, CTileFarBank::order0) != sizeOrder0) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::diffuse, CTileFarBank::order1) != sizeOrder1) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::diffuse, CTileFarBank::order2) != sizeOrder2)
			)
		{
			TileFarBank.getTile (tileNumber)->erasePixels (CTileFarBank::diffuse);
			bSame=false;
		}
	}

	// It is the same tile ?
	if (TileFarBank.getTile (tileNumber)->isFill (CTileFarBank::additive))
	{
		// Good additive size ?
		if (
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::additive, CTileFarBank::order0) != sizeOrder0) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::additive, CTileFarBank::order1) != sizeOrder1) ||
			(TileFarBank.getTile (tileNumber)->getSize (CTileFarBank::additive, CTileFarBank::order2) != sizeOrder2)
			)
		{
			TileFarBank.getTile (tileNumber)->erasePixels (CTileFarBank::additive);
			bSame=false;
		}
	}

	// Return true if the tiles seem to be the sames
	return bSame;
}

// ***************************************************************************
bool			CLandscape::initTileBanks ()
{
	// *** Check the two banks are OK
	_FarInitialized=false;

	// Compatibility check
	bool bCompatibility=true;

	// Same number of tiles
	if (TileBank.getTileCount()==TileFarBank.getNumTile())
	{
		// Same tileSet
		for (int tileSet=0; tileSet<TileBank.getTileSetCount(); tileSet++)
		{
			// Same tile128
			int tile;
			for (tile=0; tile<TileBank.getTileSet(tileSet)->getNumTile128(); tile++)
			{
				// tile number
				uint tileNumber=TileBank.getTileSet(tileSet)->getTile128(tile);

				// erase the tiles if not good
				bCompatibility&=eraseTileFarIfNotGood (tileNumber, NL_TILE_FAR_SIZE_ORDER0, NL_TILE_FAR_SIZE_ORDER1, NL_TILE_FAR_SIZE_ORDER2);
			}

			// Same tile256
			for (tile=0; tile<TileBank.getTileSet(tileSet)->getNumTile256(); tile++)
			{
				// tile number
				uint tileNumber=TileBank.getTileSet(tileSet)->getTile256(tile);

				// erase the tiles if not good
				bCompatibility&=eraseTileFarIfNotGood (tileNumber, NL_TILE_FAR_SIZE_ORDER0<<2, NL_TILE_FAR_SIZE_ORDER1<<2, NL_TILE_FAR_SIZE_ORDER2<<2);
			}

			// Same transition
			for (tile=0; tile<CTileSet::count; tile++)
			{
				// tile number
				uint tileNumber=TileBank.getTileSet(tileSet)->getTransition(tile)->getTile();

				// erase the tiles if not good
				bCompatibility&=eraseTileFarIfNotGood (tileNumber, NL_TILE_FAR_SIZE_ORDER0, NL_TILE_FAR_SIZE_ORDER1, NL_TILE_FAR_SIZE_ORDER2);
			}
		}
		
		// Far actived!
		_FarInitialized=true;
	}

	// Register / Load the vegetables.
	TileBank.initTileVegetableDescs(_VegetableManager);

	return bCompatibility;
}


// ***************************************************************************
void			CLandscape::setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply)
{
	sint nMultiply=(sint)(256.f*multiply);
	for (int i=0; i<256; i++)
	{
		sint max=0;
		sint r=(((nMultiply*diffuse.R*i)>>8)+ambiant.R*(256-i))>>8; 
		if (r>max)
			max=r;
		sint g=(((nMultiply*diffuse.G*i)>>8)+ambiant.G*(256-i))>>8;
		if (g>max)
			max=g;
		sint b=(((nMultiply*diffuse.B*i)>>8)+ambiant.B*(256-i))>>8;
		if (b>max)
			max=b;
		r<<=8;
		g<<=8;
		b<<=8;
		max=std::max(max, 256);
		r/=max;
		g/=max;
		b/=max;
		clamp (r, 0, 255);
		clamp (g, 0, 255);
		clamp (b, 0, 255);
		_LightValue[i].R=r;
		_LightValue[i].G=g;
		_LightValue[i].B=b;
		_LightValue[i].A=255;
	}
}

// ***************************************************************************
void			CLandscape::enableAutomaticLighting(bool enable)
{
	_AutomaticLighting= enable;
}

// ***************************************************************************
void			CLandscape::setupAutomaticLightDir(const CVector &lightDir)
{
	_AutomaticLightDir= lightDir;
	_AutomaticLightDir.normalize();
}


// ***************************************************************************
CVector		CLandscape::getHeightFieldDeltaZ(float x, float y) const
{
	if(_HeightField.ZPatchs.size()==0)
		return CVector::Null;

	// map to _HeightField coordinates.
	x-= _HeightField.OriginX;
	y-= _HeightField.OriginY;
	x*= _HeightField.OOSizeX;
	y*= _HeightField.OOSizeY;
	// get patch on the grid.
	sint	ix, iy;
	ix= (sint)floor(x);
	iy= (sint)floor(y);
	// out of the grid?
	if( ix<0 || ix>=(sint)_HeightField.Width || iy<0 || iy>=(sint)_HeightField.Height)
		return CVector::Null;

	// get patch.
	const CBezierPatchZ	&paz= _HeightField.ZPatchs[iy*_HeightField.Width + ix];

	// compute result.
	CVector	ret=CVector::Null;
	ret.x= 0;
	ret.y= 0;
	ret.z= paz.eval(x-ix, y-iy);

	return ret;
}



// ***************************************************************************
void		CLandscape::CBezierPatchZ::makeInteriors()
{
	float		&a = Vertices[0];
	float		&b = Vertices[1];
	float		&c = Vertices[2];
	float		&d = Vertices[3];
	Interiors[0] = Tangents[7] + Tangents[0] - a;
	Interiors[1] = Tangents[1] + Tangents[2] - b;
	Interiors[2] = Tangents[3] + Tangents[4] - c;
	Interiors[3] = Tangents[5] + Tangents[6] - d;
}
// ***************************************************************************
float		CLandscape::CBezierPatchZ::eval(float ps, float pt) const
{
	float	p;

	float ps2 = ps * ps;
	float ps1 = 1.0f - ps;
	float ps12 = ps1 * ps1;
	float s0 = ps12 * ps1;
	float s1 = 3.0f * ps * ps12;
	float s2 = 3.0f * ps2 * ps1;
	float s3 = ps2 * ps;
	float pt2 = pt * pt;
	float pt1 = 1.0f - pt;
	float pt12 = pt1 * pt1;
	float t0 = pt12 * pt1;
	float t1 = 3.0f * pt * pt12;
	float t2 = 3.0f * pt2 * pt1;
	float t3 = pt2 * pt;

	p = Vertices[0]	* s0 * t0	+ 
		Tangents[7] * s1 * t0	+ 
		Tangents[6] * s2 * t0	+ 
		Vertices[3] * s3 * t0;
	p+= Tangents[0] * s0 * t1	+ 
		Interiors[0]* s1 * t1	+ 
		Interiors[3]* s2 * t1	+ 
		Tangents[5] * s3 * t1;
	p+=	Tangents[1] * s0 * t2	+ 
		Interiors[1]* s1 * t2	+ 
		Interiors[2]* s2 * t2	+ 
		Tangents[4] * s3 * t2;
	p+=	Vertices[1] * s0 * t3	+ 
		Tangents[2] * s1 * t3	+ 
		Tangents[3] * s2 * t3	+ 
		Vertices[2] * s3 * t3;
	
	return p;
}


// ***************************************************************************
void		CLandscape::setHeightField(const CHeightMap &hf)
{
	if(hf.getWidth()<2)
		return;
	if(hf.getHeight()<2)
		return;

	// Fill basics.
	_HeightField.OriginX= hf.OriginX;
	_HeightField.OriginY= hf.OriginY;
	_HeightField.SizeX= hf.SizeX;
	_HeightField.SizeY= hf.SizeY;
	_HeightField.OOSizeX= 1/hf.SizeX;
	_HeightField.OOSizeY= 1/hf.SizeY;
	uint	w= hf.getWidth()-1;
	uint	h= hf.getHeight()-1;
	_HeightField.Width= w;
	_HeightField.Height= h;
	_HeightField.ZPatchs.resize(w * h);

	// compute  patchs
	sint	x,y;

	// compute vertices / tangents on each patch
	for(y=0;y<(sint)h;y++)
	{
		for(x=0;x<(sint)w;x++)
		{
			CBezierPatchZ	&paz= _HeightField.ZPatchs[y*w+x];
			// vertices.
			paz.Vertices[0]= hf.getZ(x, y);
			paz.Vertices[1]= hf.getZ(x, y+1);
			paz.Vertices[2]= hf.getZ(x+1, y+1);
			paz.Vertices[3]= hf.getZ(x+1, y);
		}
	}

	// compute tangents
	for(y=0;y<(sint)h;y++)
	{
		for(x=0;x<(sint)w;x++)
		{
			CBezierPatchZ	&paz= _HeightField.ZPatchs[y*w+x];
			sint	tg;
			// For each tangent, what vertex (relative to x,y) we must take.
			struct	CDeltaPos
			{
				sint	ox,oy;
				sint	dx1,dy1;
				sint	dx2,dy2;
			};
			static CDeltaPos	deltas[8]= {
				{0,0, 0,1, 0,-1} ,
				{0,1, 0,0, 0,2} ,
				{0,1, 1,1, -1,1} ,
				{1,1, 0,1, 2,1} ,
				{1,1, 1,0, 1,2} ,
				{1,0, 1,1, 1,-1} ,
				{1,0, 0,0, 2,0} ,
				{0,0, 1,0, -1,0} ,
				};

			// compute each tangent of this patch.
			for(tg=0; tg<8;tg++)
			{
				sint	x0,y0;
				sint	x1,y1;
				sint	x2,y2;
				x0= x+deltas[tg].ox; y0= y+deltas[tg].oy;
				x1= x+deltas[tg].dx1; y1= y+deltas[tg].dy1;
				x2= x+deltas[tg].dx2; y2= y+deltas[tg].dy2;

				// borders case.
				if(x2<0 || x2>=(sint)hf.getWidth() || y2<0 || y2>=(sint)hf.getHeight())
				{
					float		v,dv;
					// base of tangents.
					v= hf.getZ(x0,y0);
					// target tangents.
					dv= hf.getZ(x1,y1) - v;
					// result of tangent.
					paz.Tangents[tg]= v+dv/3;
				}
				// middle case.
				else
				{
					float		v,dv;
					// base of tangents.
					v= hf.getZ(x0,y0);
					// target tangents.
					dv= hf.getZ(x1,y1) - v;
					// add mirror target tangent.
					dv+= -(hf.getZ(x2,y2) - v);
					dv/=2;
					// result of tangent.
					paz.Tangents[tg]= v+dv/3;
				}
			}
		}
	}

	// compute interiors.
	for(y=0;y<(sint)h;y++)
	{
		for(x=0;x<(sint)w;x++)
		{
			CBezierPatchZ	&paz= _HeightField.ZPatchs[y*w+x];
			paz.makeInteriors();
		}
	}

}


// ***************************************************************************
void		CLandscape::getTessellationLeaves(std::vector<const CTessFace*>  &leaves) const
{
	leaves.clear();

	std::map<uint16, CZone*>::const_iterator	it;
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		// Then trace all patch.
		sint	N= (*it).second->getNumPatchs();
		for(sint i=0;i<N;i++)
		{
			const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(i);

			pa->appendTessellationLeaves(leaves);
		}
	}

}


// ***************************************************************************
void		CLandscape::setPZBModelPosition(const CVector &pos)
{
	_PZBModelPosition= pos;
}



// ***************************************************************************
// ***************************************************************************
// Allocators.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTessFace			*CLandscape::newTessFace()
{
	// allcoate the face.
	CTessFace		*face= TessFaceAllocator.allocate();

	// for refine() mgt, append the face to the list of newLeaves, so they will be tested in refine()
	face->linkInPList(_RootNewLeaves);

	return face;
}

// ***************************************************************************
CTessVertex			*CLandscape::newTessVertex()
{
	return TessVertexAllocator.allocate();
}

// ***************************************************************************
CTessNearVertex		*CLandscape::newTessNearVertex()
{
	return TessNearVertexAllocator.allocate();
}

// ***************************************************************************
CTessFarVertex		*CLandscape::newTessFarVertex()
{
	return TessFarVertexAllocator.allocate();
}

// ***************************************************************************
CTileMaterial		*CLandscape::newTileMaterial()
{
	return TileMaterialAllocator.allocate();
}

// ***************************************************************************
CTileFace			*CLandscape::newTileFace()
{
	return TileFaceAllocator.allocate();
}

// ***************************************************************************
void				CLandscape::deleteTessFace(CTessFace *f)
{
	// for refine() mgt, must remove from refine priority list, or from the temp rootTessFaceToUpdate list.
	f->unlinkInPList();

	TessFaceAllocator.free(f);
}

// ***************************************************************************
void				CLandscape::deleteTessVertex(CTessVertex *v)
{
	TessVertexAllocator.free(v);
}

// ***************************************************************************
void				CLandscape::deleteTessNearVertex(CTessNearVertex *v)
{
	TessNearVertexAllocator.free(v);
}

// ***************************************************************************
void				CLandscape::deleteTessFarVertex(CTessFarVertex *v)
{
	TessFarVertexAllocator.free(v);
}

// ***************************************************************************
void				CLandscape::deleteTileMaterial(CTileMaterial *tm)
{
	TileMaterialAllocator.free(tm);
}

// ***************************************************************************
void				CLandscape::deleteTileFace(CTileFace *tf)
{
	TileFaceAllocator.free(tf);
}



// ***************************************************************************
// ***************************************************************************
// Noise
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscape::setNoiseMode(bool enable)
{
	_NoiseEnabled= enable;
}

// ***************************************************************************
bool			CLandscape::getNoiseMode() const
{
	return _NoiseEnabled;
}


// ***************************************************************************
// ***************************************************************************
// Micro vegetation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CLandscape::enableVegetable(bool enable)
{
	_VegetableManagerEnabled= enable;

	// if false, delete all Vegetable IGs.
	if(!_VegetableManagerEnabled)
	{
		// Landscape always create ClipBlokcs, but IGs/addInstances() are created only if isVegetableActive().
		// For all zones.
		for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
		{
			// for all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				// delete vegetable Igs of this patch
				CPatch	*pa= ((*it).second)->getPatch(i);
				pa->deleteAllVegetableIgs();
			}

		}
	}
	// if true
	else
	{
		//  reload all Shapes (actually load only new shapes)
		TileBank.initTileVegetableDescs(_VegetableManager);

		// And recreate vegetable igs.
		// Landscape always create ClipBlokcs, but IGs/addInstances() are created only if isVegetableActive().
		// For all zones.
		for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
		{
			// for all patch.
			sint	N= (*it).second->getNumPatchs();
			for(sint i=0;i<N;i++)
			{
				// recreate vegetable Igs of this patch
				CPatch	*pa= ((*it).second)->getPatch(i);
				pa->recreateAllVegetableIgs();
			}

		}
	}

}

// ***************************************************************************
bool		CLandscape::isVegetableActive() const
{
	return _VegetableManagerEnabled && _DriverOkForVegetable;
}

// ***************************************************************************
void		CLandscape::loadVegetableTexture(const string &textureFileName)
{
	// load the texture in the manager
	_VegetableManager->loadTexture(textureFileName);
}

// ***************************************************************************
void		CLandscape::setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight)
{
	// set the directional light to the manager
	_VegetableManager->setDirectionalLight(ambient, diffuse, directionalLight);
}

// ***************************************************************************
void		CLandscape::setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin)
{
	// setup vegetable manager
	_VegetableManager->setWind(windDir, windFreq, windPower, windBendMin);
}


// ***************************************************************************
void		CLandscape::setVegetableTime(double time)
{
	// setup vegetable manager
	_VegetableManager->setTime(time);
}


// ***************************************************************************
uint		CLandscape::getNumVegetableFaceRendered() const
{
	return _VegetableManager->getNumVegetableFaceRendered();
}


// ***************************************************************************
const CTileVegetableDesc	&CLandscape::getTileVegetableDesc(uint16 tileId)
{
	return TileBank.getTileVegetableDesc(tileId);
}


// ***************************************************************************
void		CLandscape::createVegetableBlendLayersModels(CScene *scene)
{
	_VegetableManager->createVegetableBlendLayersModels(scene);
}


// ***************************************************************************
void		CLandscape::setVegetableUpdateLightingFrequency(float freq)
{
	_VegetableManager->setUpdateLightingFrequency(freq);
}

// ***************************************************************************
void		CLandscape::setupColorsFromTileFlags(const NLMISC::CRGBA colors[4])
{
	for (TZoneMap::iterator it = Zones.begin(); it != Zones.end(); ++it)
	{
		it->second->setupColorsFromTileFlags(colors);
	}
}

// ***************************************************************************
// ***************************************************************************
// Lightmap Get interface.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
uint8		CLandscape::getLumel(const CPatchIdent &patchId, const CUV &uv) const
{
	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId.PatchId);

		return pa->getLumel(uv);
	}
	else
		// Return full sun contribution as default
		return 255;
}

// ***************************************************************************
void		CLandscape::appendTileLightInfluences(const CPatchIdent &patchId, const CUV &uv, 
	std::vector<CPointLightInfluence> &pointLightList) const
{
	// \todo yoyo: TODO_ZONEID: change ZoneId in 32 bits...
	std::map<uint16, CZone*>::const_iterator	it= Zones.find((uint16)patchId.ZoneId);
	if(it!=Zones.end())
	{
		sint	N= (*it).second->getNumPatchs();
		// patch must exist in the zone.
		nlassert(patchId.PatchId<N);
		const CPatch	*pa= const_cast<const CZone*>((*it).second)->getPatch(patchId.PatchId);

		pa->appendTileLightInfluences(uv, pointLightList);
	}
}


// ***************************************************************************
// ***************************************************************************
// Lighting
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscape::removeAllPointLights()
{
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		// for all patch.
		sint	N= (*it).second->getNumPatchs();
		for(sint i=0;i<N;i++)
		{
			// Clear TileLightInfluences
			CPatch	*pa= ((*it).second)->getPatch(i);
			pa->resetTileLightInfluences();
		}

		// Remove all PointLights.
		(*it).second->_PointLightArray.clear();
	}

}


// ***************************************************************************
void			CLandscape::setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor)
{
	// Store the result in the map for addZone().
	_LightGroupColorMap[lightGroupName]= nFactor;

	// Affect currently added zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->_PointLightArray.setPointLightFactor(lightGroupName, nFactor);
	}
}


// ***************************************************************************
void			CLandscape::updateLighting(double time)
{
	_ULTime= time;

	// first time in this method??
	if(!_ULPrecTimeInit)
	{
		_ULPrecTimeInit= true;
		_ULPrecTime= _ULTime;
	}
	// compute delta time from last update.
	float dt= float(_ULTime - _ULPrecTime);
	_ULPrecTime= _ULTime;


	// If not disabled
	if(_ULFrequency)
	{
		// Do it for near and far in 2 distinct ways.
		updateLightingTextureFar(dt * _ULFrequency);
		updateLightingTextureNear(dt * _ULFrequency);
	}

}

// ***************************************************************************
void			CLandscape::setUpdateLightingFrequency(float freq)
{
	freq= max(freq, 0.f);
	_ULFrequency= freq;
}


// ***************************************************************************
void			CLandscape::linkPatchToNearUL(CPatch *patch)
{
	// Append this patch to the list of patch to updateLighting.
	if(_ULRootNearPatch==NULL)
		_ULRootNearPatch= patch;
	else
		patch->linkBeforeNearUL(_ULRootNearPatch);
}

// ***************************************************************************
void			CLandscape::unlinkPatchFromNearUL(CPatch *patch)
{
	// If I unlink the patch which is the current root
	if(_ULRootNearPatch==patch)
	{
		// switch to next
		_ULRootNearPatch= patch->getNextNearUL();
		// if still the same, it means that the circular list is now empty
		if(_ULRootNearPatch==patch)
			_ULRootNearPatch= NULL;
		// reset tessBlock counter.
		_ULNearCurrentTessBlockId= 0;
	}

	// unlink the patch from list
	patch->unlinkNearUL();
}


// ***************************************************************************
void			CLandscape::updateLightingTextureFar(float ratio)
{
	// compute number of pixels to update.
	_ULFarPixelsToUpdate+= ratio * _ULTotalFarPixels;
	// maximize, so at max, it computes all patchs, just one time.
	_ULFarPixelsToUpdate= min(_ULFarPixelsToUpdate, (float)_ULTotalFarPixels);

	// Test Profile Yoyo
	/*extern bool YOYO_LandULTest;
	if(YOYO_LandULTest)
	{
		nlinfo("YOYO_UL Far: %dK, %dK", (sint)_ULFarPixelsToUpdate/1024, (sint)_ULTotalFarPixels/1024);
	}*/

	// while there is still some pixels to update.
	while(_ULFarPixelsToUpdate > 0 && _ULRootTextureFar)
	{
		// update patch (if not null) in the textureFar.
		_ULFarPixelsToUpdate-= _ULRootTextureFar->touchPatch(_ULFarCurrentPatchId);
		// Next patch to process.
		_ULFarCurrentPatchId++;

		// last patch in the texture??
		if(_ULFarCurrentPatchId>=NL_NUM_FAR_PATCHES_BY_TEXTURE)
		{
			// yes, go to next texture.
			_ULRootTextureFar= _ULRootTextureFar->getNextUL();
			// reset to 0th patch.
			_ULFarCurrentPatchId=0;
		}
	}

	// Now, _ULFarPixelsToUpdate should be <=0. (most of the time < 0)
}


// ***************************************************************************
void			CLandscape::updateLightingTextureNear(float ratio)
{
	// compute number of pixels to update.
	_ULNearPixelsToUpdate+= ratio * _ULTotalNearPixels;
	// maximize, so at max, it computes all patchs, just one time.
	_ULNearPixelsToUpdate= min(_ULNearPixelsToUpdate, (float)_ULTotalNearPixels);


	// while there is still some pixels to update.
	while(_ULNearPixelsToUpdate > 0 && _ULRootNearPatch)
	{
		// update tessBlock (if lightmap exist for this tessBlock) in the patch.
		_ULNearPixelsToUpdate-= _ULRootNearPatch->updateTessBlockLighting(_ULNearCurrentTessBlockId);
		// Next tessBlock to process.
		_ULNearCurrentTessBlockId++;

		// last tessBlock in the patch??
		if(_ULNearCurrentTessBlockId>=_ULRootNearPatch->getNumNearTessBlocks())
		{
			// yes, go to next patch.
			_ULRootNearPatch= _ULRootNearPatch->getNextNearUL();
			// reset to 0th tessBlock.
			_ULNearCurrentTessBlockId=0;
		}
	}

}


// ***************************************************************************
void			CLandscape::computeDynamicLighting(const std::vector<CPointLight*>	&pls)
{
	uint	i;

	// Run all DLM Context create, to init Lighting process.
	//===============
	CPatchDLMContext	*ctxPtr= _PatchDLMContextList->begin();
	while(ctxPtr!=NULL)
	{
		// init lighting process, do differential from last computeDynamicLighting()
		ctxPtr->getPatch()->beginDLMLighting();

		// next
		ctxPtr= (CPatchDLMContext*)ctxPtr->Next;
	}


	// compile all pointLights
	//===============
	static vector<CPatchDLMPointLight>	dlmPls;
	dlmPls.resize(pls.size());
	for(i=0;i<dlmPls.size();i++)
	{
		// compile the pl.
		dlmPls[i].compile(*pls[i], _DLMMaxAttEnd);
	}


	// For all pointLight, intersect patch.
	//===============
	for(i=0;i<dlmPls.size();i++)
	{
		CPatchDLMPointLight	&pl= dlmPls[i];

		// search patchs of interest: those which interssect the pointLight
		_PatchQuadGrid.clearSelection();
		_PatchQuadGrid.select(pl.BBox.getMin(), pl.BBox.getMax());
		CQuadGrid<CPatchIdent>::CIterator	it;

		// for each patch, light it with the light.
		CZone	*lastZone= NULL;
		sint	lastZoneId= -1;
		for(it= _PatchQuadGrid.begin(); it!= _PatchQuadGrid.end(); it++)
		{
			uint	zoneId= (*it).ZoneId;
			uint	patchId= (*it).PatchId;

			// find zone, possibly look in lastZone cache
			CZone	*zone;
			if((sint)zoneId==lastZoneId)
			{
				// just get cache.
				zone= lastZone;
			}
			else
			{
				// search in map
				std::map<uint16, CZone*>::const_iterator	zoneit= Zones.find(zoneId);
				if(zoneit!=Zones.end())
					zone= (*zoneit).second;
				else
					zone= NULL;
				// bkup cahche
				lastZone= zone;
				lastZoneId= zoneId;
			}

			// if the zone exist.
			if(zone)
			{
				uint	N= zone->getNumPatchs();
				// patch must exist in the zone.
				nlassert(patchId>=0);
				nlassert(patchId<N);
				// get the patch
				const CPatch	*pa= const_cast<const CZone*>(zone)->getPatch(patchId);

				// More precise clipping: 
				if( pa->getBSphere().intersect( pl.BSphere ) )
				{
					// Ok, light the patch with this spotLight
					const_cast<CPatch*>(pa)->processDLMLight(pl);
				}
			}
		}

	}


	// Run all DLM Context create, to end Lighting process.
	//===============
	ctxPtr= _PatchDLMContextList->begin();
	while(ctxPtr!=NULL)
	{
		// get enxt now, because the DLM itself may be deleted in endDLMLighting()
		CPatchDLMContext	*next= (CPatchDLMContext*)ctxPtr->Next;

		// init lighting process, do differential from last computeDynamicLighting()
		ctxPtr->getPatch()->endDLMLighting();

		// next
		ctxPtr= next;
	}

}


// ***************************************************************************
void			CLandscape::setDynamicLightingMaxAttEnd(float maxAttEnd)
{
	maxAttEnd= max(maxAttEnd, 1.f);
	_DLMMaxAttEnd= maxAttEnd;
}


// ***************************************************************************
uint			CLandscape::getDynamicLightingMemoryLoad() const
{
	uint	mem= 0;
	// First, set size of global texture overhead.
	mem= NL3D_LANDSCAPE_DLM_WIDTH * NL3D_LANDSCAPE_DLM_HEIGHT * sizeof(CRGBA);

	// Then, for each patchContext created
	CPatchDLMContext	*ctxPtr= _PatchDLMContextList->begin();
	while(ctxPtr!=NULL)
	{
		// add its memory load.
		mem+= ctxPtr->getMemorySize();

		// next
		ctxPtr= (CPatchDLMContext*)ctxPtr->Next;
	}

	return mem;
}


} // NL3D
