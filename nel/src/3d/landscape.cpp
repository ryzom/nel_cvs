/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.62 2001/07/02 14:43:17 berenguier Exp $
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


#include "3d/landscape.h"
#include "nel/misc/bsphere.h"
#include "3d/texture_file.h"
#include "3d/texture_far.h"
#include "3d/landscape_profile.h"
#include "nel/3d/height_map.h"

using namespace NLMISC;
using namespace std;



namespace NL3D 
{


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
CLandscape::CLandscape()
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
}
// ***************************************************************************
CLandscape::~CLandscape()
{
	clear();
}


// ***************************************************************************
void			CLandscape::init()
{
	// v3f/t2f/c4ub
	FarVB.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0] | IDRV_VF_COLOR );
	FarVB.reserve(1024);

	// v3f/t2f0/t2f1
	TileVB.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0] | IDRV_VF_UV[1]);
	TileVB.reserve(1024);

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
void			CLandscape::setTileNear (float tileNear)
{
	tileNear= max(tileNear, _FarTransition);

	if(tileNear!=_TileDistNear)
	{
		_TileDistNear= tileNear;
		resetRenderFar();
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
void			CLandscape::resetRenderFar()
{
	// For all patch of all zones.
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		((*it).second)->resetRenderFar();
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
	updateGlobals (CVector::Null);

	uint16	zoneId= newZone.getZoneId();

	if(Zones.find(zoneId)!=Zones.end())
		return false;
	CZone	*zone= new CZone;

	// copy zone.
	zone->build(newZone);

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

	return true;
}
// ***************************************************************************
bool			CLandscape::removeZone(uint16 zoneId)
{
	// -1. Update globals
	updateGlobals (CVector::Null);

	// find the zone.
	if(Zones.find(zoneId)==Zones.end())
		return false;
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

	return true;
}
// ***************************************************************************
void			CLandscape::clear()
{
	// -1. Update globals
	updateGlobals (CVector::Null);

	// Build the list of zoneId.
	vector<uint16>	zoneIds;
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		zoneIds.push_back((*it).first);
	}

	// Remove each zone one by one.
	for(sint i=0;i<(sint)zoneIds.size();i++)
	{
		nlverify(removeZone(zoneIds[i]));
	}

	// ensure the quadgrid is empty.
	_PatchQuadGrid.clear();

}
// ***************************************************************************
void			CLandscape::clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid)
{
	// -1. Update globals
	updateGlobals (refineCenter);


	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->clip(pyramid);
	}
}
// ***************************************************************************
void			CLandscape::refine(const CVector &refineCenter)
{
	NL3D_PROFILE_LAND_SET(ProfNRefineFaces, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineComputeFaces, 0);
	NL3D_PROFILE_LAND_SET(ProfNRefineLeaves, 0);
	NL3D_PROFILE_LAND_SET(ProfNSplits, 0);
	NL3D_PROFILE_LAND_SET(ProfNMerges, 0);


	if(!_RefineMode)
		return;

	// -1. Update globals
	updateGlobals (refineCenter);

	// Increment the update date.
	CTessFace::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->refine();
	}
}
// ***************************************************************************
void			CLandscape::refineAll(const CVector &refineCenter)
{
	// -1. Update globals
	updateGlobals (refineCenter);

	// Increment the update date.
	CTessFace::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->refineAll();
	}
}

// ***************************************************************************
void			CLandscape::updateGlobals (const CVector &refineCenter) const
{
	// Setup CTessFace static members...

	// Far limits.
	CTessFace::FarTransition= _FarTransition;

	// Tile subdivsion part.
	CTessFace::TileMaxSubdivision= _TileMaxSubdivision;
	CTessFace::TileDistNear = _TileDistNear;
	CTessFace::TileDistFar = CTessFace::TileDistNear+20;
	CTessFace::TileDistNearSqr = sqr(CTessFace::TileDistNear);
	CTessFace::TileDistFarSqr = sqr(CTessFace::TileDistFar);
	CTessFace::OOTileDistDeltaSqr = 1.0f / (CTessFace::TileDistFarSqr - CTessFace::TileDistNearSqr);

	// Tile Pixel size part.
	// \todo yoyo: choose according to wanted tile pixel size.
	CTessFace::TilePixelSize= 128.0f;

	// RefineThreshold.
	CTessFace::RefineThreshold= _Threshold;
	CTessFace::OORefineThreshold= 1.0f / CTessFace::RefineThreshold;

	// Refine Center*.
	CTessFace::RefineCenter= refineCenter;
	CTessFace::TileFarSphere.Center= CTessFace::RefineCenter;
	CTessFace::TileFarSphere.Radius= CTessFace::TileDistFar;
	CTessFace::TileNearSphere.Center= CTessFace::RefineCenter;
	CTessFace::TileNearSphere.Radius= CTessFace::TileDistNear;
}
// ***************************************************************************
void			CLandscape::render(IDriver *driver, const CVector &refineCenter, const std::vector<CPlane>	&pyramid, bool doTileAddPass)
{
	CTessFace::RefineCenter= refineCenter;
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


	// -1. Update globals
	//====================
	updateGlobals (refineCenter);
	// Render.
	CTessFace::CurrentTileVB= &TileVB;
	CTessFace::CurrentTileIndex= 0;
	CTessFace::CurrentFarVB= &FarVB;
	CTessFace::CurrentFarIndex= 0;


	// 0. preRender pass.
	//===================
	// Reset MaxTris info.
	CPatchRdrPass::resetGlobalIndex();
	for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
	{
		CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
		pass.resetMaxTriList();
	}
	for(itFar=_FarRdrPassSet.begin(); itFar!=_FarRdrPassSet.end(); itFar++)
	{
		CPatchRdrPass	&pass= **itFar;
		pass.resetMaxTriList();
	}
	for(i=0; i<(sint)_TextureNears.size(); i++)
	{
		CPatchRdrPass	&pass= *_TextureNears[i];
		pass.resetMaxTriList();
	}

	// preRender.
	// Clip TessBlocks against pyramid and Far Limit.
	// Compute MaxTris of each RenderPass (tile and far).
	// Compute Vertices in VBuffers.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->preRender(pyramid);
	}
	CTessFace::CurrentFarVB->setNumVertices(CTessFace::CurrentFarIndex);
	CTessFace::CurrentTileVB->setNumVertices(CTessFace::CurrentTileIndex);



	// 0.a for each RenderPass, compute his starting ptr.
	//===================================================
	CPatchRdrPass::resetGlobalIndex();
	for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
	{
		CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
		pass.computeStartIndex();
		// Must reset the list, so that CurIndex begin at StartIndex!!
		pass.resetTriList();
	}
	for(itFar=_FarRdrPassSet.begin(); itFar!=_FarRdrPassSet.end(); itFar++)
	{
		CPatchRdrPass	&pass= **itFar;
		pass.computeStartIndex();
		// Must reset the list, so that CurIndex begin at StartIndex!!
		pass.resetTriList();
	}
	for(i=0; i<(sint)_TextureNears.size(); i++)
	{
		CPatchRdrPass	&pass= *_TextureNears[i];
		pass.computeStartIndex();
		// Must reset the list, so that CurIndex begin at StartIndex!!
		pass.resetTriList();
	}


	// 1. TileRender pass.
	//====================
	// Active VB.
	driver->activeVertexBuffer(*CTessFace::CurrentTileVB);

	// Render Order. Must "invert", since initial order is NOT the render order. This is done because the lightmap pass
	// DO NOT have to do any renderTil(), since it is computed in RGB0 pass.
	nlassert(NL3D_MAX_TILE_PASS==5);
	static	sint	RenderOrder[NL3D_MAX_TILE_PASS]= {NL3D_TILE_PASS_RGB0, NL3D_TILE_PASS_RGB1, NL3D_TILE_PASS_RGB2,
		NL3D_TILE_PASS_LIGHTMAP, NL3D_TILE_PASS_ADD};
	// For ALL pass..
	for(i=0; i<NL3D_MAX_TILE_PASS; i++)
	{
		sint	passOrder= RenderOrder[i];

		// Do add pass???
		if((passOrder==NL3D_TILE_PASS_ADD) && !doTileAddPass)
			continue;

		// Process all zones.
		//=============================
		if(passOrder!= NL3D_TILE_PASS_LIGHTMAP)
		{
			for(it= Zones.begin();it!=Zones.end();it++)
			{
				(*it).second->renderTile(passOrder);
			}
		}
		// NB: if passOrder is RGB0, primitives will be added to to the RGB0 pass AND the LIGHTMAP pass.
		// So there is nothing to do here if LIGHTMAP PASS.


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
					// Use srcalpha for src (and not ONE), since additive are blended with alpha gouraud 
					// (for smooth night transition).
					TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::one);
					break;
				default: 
					nlstop;
			};
		}
		// Reset the textures (so there is none in Addtive pass or in Lightmap).
		TileMaterial.setTexture(0, NULL);
		TileMaterial.setTexture(1, NULL);


		// Render All material RdrPass.
		//=============================
		if(passOrder!=NL3D_TILE_PASS_LIGHTMAP)
		{
			// Render Base, Transitions or Additives.
			bool	alphaStage= (passOrder==NL3D_TILE_PASS_RGB1) || (passOrder==NL3D_TILE_PASS_RGB2);

			ItTileRdrPassSet	itTile;
			for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
			{
				// Get a ref on the render pass. Const cast work because we only modify attribut from CPatchRdrPass 
				// that don't affect the operator< of this class
				CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
				if(pass.NTris==0)
					continue;

				// Setup material.
				// Setup Diffuse texture of the tile.
				TileMaterial.setTexture(0, pass.TextureDiffuse);
				// If transition tile, must enable the alpha for this pass.
				if(alphaStage)
					TileMaterial.setTexture(1, pass.TextureAlpha);

				// Render!
				driver->renderTriangles(TileMaterial, pass.getStartPointer(), pass.NTris);

				// Yoyo: profile.
				NL3D_PROFILE_LAND_ADD(ProfNRdrTile[passOrder], pass.NTris);


				// must resetTriList at each end of each material process.
				pass.resetTriList();
			}
		}
		else
		{
			// \todo yoyo: TODO_CLOUD: setup stage0, and setup texcoord generation.

			// Render the lightmap.
			for(sint lightRdrPass=0; lightRdrPass<(sint)_TextureNears.size(); lightRdrPass++)
			{
				CPatchRdrPass	&pass= *_TextureNears[lightRdrPass];
				if(pass.NTris==0)
					continue;

				// Setup Lightmap into stage1. Because we share UV with RGB0. So we use UV1.
				// Cloud will be placed into stage0, and texture coordinate will be generated by T&L.
				TileMaterial.setTexture(1, pass.TextureDiffuse);

				// Render!
				driver->renderTriangles(TileMaterial, pass.getStartPointer(), pass.NTris);

				// Yoyo: profile.
				NL3D_PROFILE_LAND_ADD(ProfNRdrTile[passOrder], pass.NTris);

				
				// must resetTriList at each end of each material process.
				pass.resetTriList();
			}
		}
	}


	// 2. Far0Render pass.
	//====================
	// Process all zones.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->renderFar0();
	}

	// Active VB.
	driver->activeVertexBuffer(*CTessFace::CurrentFarVB);

	// Setup common material.
	FarMaterial.setBlend(false);

	// Render All material RdrPass0.
	itFar=_FarRdrPassSet.begin();
	while (itFar!=_FarRdrPassSet.end())
	{
		CPatchRdrPass	&pass= **itFar;

		FarMaterial.setTexture(0, pass.TextureDiffuse);
		driver->renderTriangles(FarMaterial, pass.getStartPointer(), pass.NTris);

		// Yoyo: profile.
		NL3D_PROFILE_LAND_ADD(ProfNRdrFar0, pass.NTris);

		// must resetTriList at each end of each material process. (because Far1 and Far0 are blended into same rdrpass).
		pass.resetTriList();

		// Next render pass
		itFar++;
	}

	// 3. Far1Render pass.
	//====================
	// Process all zones.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->renderFar1();
	}

	// Active VB.
	driver->activeVertexBuffer(*CTessFace::CurrentFarVB);

	// Setup common material.
	FarMaterial.setBlend(true);


	// Render All material RdrPass1.
	itFar=_FarRdrPassSet.begin();
	while (itFar!=_FarRdrPassSet.end())
	{
		CPatchRdrPass	&pass= **itFar;

		FarMaterial.setTexture(0, pass.TextureDiffuse);
		driver->renderTriangles(FarMaterial, pass.getStartPointer(), pass.NTris);

		// Yoyo: profile.
		NL3D_PROFILE_LAND_ADD(ProfNRdrFar1, pass.NTris);

		// must resetTriList at each end of each material process.
		pass.resetTriList();

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
		// No alpha part for additive.

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
	lightMapId= textNum*NbTilesByTexture + lightMapId;

	// Result:
	lightmapRdrPass= nearRdrPass;
	return lightMapId;
}
// ***************************************************************************
void		CLandscape::getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias)
{
	uint	id, s,t;

	// Scale.
	float	scale5= (float)NL_TILE_LIGHTMAP_SIZE/TextureNearSize;
	float	scale4= (float)(NL_TILE_LIGHTMAP_SIZE-1)/TextureNearSize;
	float	scale1= (float)(1)/TextureNearSize;
	// The size of a minilightmap, mapped onto the polygon, is still 4 pixels.
	uvScaleBias.z= scale4;

	// Get the id local in the texture.
	id= tileLightMapId%NbTilesByTexture;

	// Commpute UVBias.
	// Get the coordinate of the tile, in tile number.
	s= id%NbTilesByLine;
	t= id/NbTilesByLine;
	// But the real size of a minilightmap is 5 pixels, and we must reach the center of the pixel.
	uvScaleBias.x= s*scale5 + 0.5f*scale1;
	uvScaleBias.y= t*scale5 + 0.5f*scale1;
}
// ***************************************************************************
void		CLandscape::releaseTileLightMap(uint tileLightMapId)
{
	uint	id, textNum;

	// Get the id local in the texture.
	textNum= tileLightMapId/NbTilesByTexture;
	id= tileLightMapId%NbTilesByTexture;
	nlassert(textNum>=0 && textNum<_TextureNears.size());

	// Release the tile in this texture.
	CPatchRdrPass	*nearRdrPass= _TextureNears[textNum];
	CTextureNear	*nearText= (CTextureNear*)(ITexture*)nearRdrPass->TextureDiffuse;
	nearText->releaseTile(id);
	_NFreeLightMaps++;
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

	// If no far texture, return
	if (!_FarInitialized)
		return NULL;

	// Get size of the far texture
	uint width=(pPatch->getOrderS ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);
	uint height=(pPatch->getOrderT ()*NL_NUM_PIXELS_ON_FAR_TILE_EDGE)>>(farIndex-1);

	// Render pass index
	uint passIndex=getRdrPassIndexWithSize (width, height);

	// Look for a free render pass
	if (_FarRdrPassSetVectorFree[passIndex].begin()==_FarRdrPassSetVectorFree[passIndex].end())
	{
		// Empty, add a new render pass
		CPatchRdrPass	*pass=new CPatchRdrPass;

		// Fill the render pass
		CTextureFar *pTextureFar=new CTextureFar;

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

			// Remove from draw list
			_FarRdrPassSet.erase (pass);
		}
	}
	else
	{
		// Insert in the free list
		_FarRdrPassSetVectorFree[passIndex].insert (pass);
	}
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
	std::map<uint16, CZone*>::const_iterator	it= Zones.find(patchId.ZoneId);
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



} // NL3D
