/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.36 2001/01/16 14:46:45 berenguier Exp $
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


#include "nel/3d/landscape.h"
#include "nel/3d/bsphere.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_far.h"
using namespace NLMISC;
using namespace std;

namespace NL3D 
{

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
	virtual void generate()
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
// TODO: may change this.
// The size of a tile, in pixel. UseFull for HalfPixel Scale/Bias.
const	float TileSize= 128;


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

	_TileDistNear=100.f;
	_Threshold= 0.001f;
	_RefineMode=true;
	_NFreeLightMaps= 0;
}
// ***************************************************************************
CLandscape::~CLandscape()
{
	clear();
}


// ***************************************************************************
void			CLandscape::init(bool bumpTiles)
{
	// v3f/t2f/c4ub
	FarVB.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0] | IDRV_VF_COLOR );
	FarVB.reserve(1024);

	// v3f/t2f0/t2f1/c4ub
	TileVB.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0] | IDRV_VF_UV[1] | IDRV_VF_COLOR );
	TileVB.reserve(1024);

	// TODO_BUMP: Need t3f in IDriver.
	// v3f/t2f0/t2f1/t2f2/t2f3/c4ub


	// Fill Far mat.
	// Must init his BlendFunction here!!! becaus it switch between blend on/off during rendering.
	FarMaterial.initUnlit();
	FarMaterial.setSrcBlend(CMaterial::srcalpha);
	FarMaterial.setDstBlend(CMaterial::invsrcalpha);

	// Init material for tile.
	TileMaterial.initUnlit();
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
	zone->build(newZone);
	zone->compile(this, Zones);

	return true;
}
// ***************************************************************************
bool			CLandscape::removeZone(uint16 zoneId)
{
	// -1. Update globals
	updateGlobals (CVector::Null);

	if(Zones.find(zoneId)==Zones.end())
		return false;
	CZone	*zone= Zones[zoneId];
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
void			CLandscape::updateGlobals (const CVector &refineCenter) const
{
	// Setup CTessFace static members...
	CTessFace::TileDistNear = _TileDistNear;
	CTessFace::TileDistFar = CTessFace::TileDistNear+20;
	CTessFace::TileDistNearSqr = sqr(CTessFace::TileDistNear);
	CTessFace::TileDistFarSqr = sqr(CTessFace::TileDistFar);
	CTessFace::OOTileDistDeltaSqr = 1.0f / (CTessFace::TileDistFarSqr - CTessFace::TileDistNearSqr);

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
void			CLandscape::render(IDriver *driver, const CVector &refineCenter, bool doTileAddPass)
{
	CTessFace::RefineCenter= refineCenter;
	ItZoneMap	it;
	sint		i;

	// -1. Update globals
	updateGlobals (refineCenter);

	// 0. preRender pass.
	//===================
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->preRender();
	}

	// 1. TileRender pass.
	//====================
	for(i=0; i<NL3D_MAX_TILE_PASS; i++)
	{
		// Do add pass???
		if((i&1) && !doTileAddPass)
			continue;

		// Reset VB inbfos.
		CTessFace::CurrentVB= &TileVB;
		CTessFace::CurrentVertexIndex= 1;
		CPatchRdrPass::resetGlobalTriList();

		// Process all zones.
		//=============================
		// Inc at each pass!!
		CTessFace::CurrentRenderDate++;
		for(it= Zones.begin();it!=Zones.end();it++)
		{
			(*it).second->renderTile(i);
		}

		// Active VB.
		//=============================
		CTessFace::CurrentVB->setNumVertices(CTessFace::CurrentVertexIndex);
		driver->activeVertexBuffer(*CTessFace::CurrentVB);

		// Setup common material for this pass.
		//=============================
		if(i==0)
		{
			TileMaterial.setBlend(false);
		}
		else
		{
			TileMaterial.setBlend(true);
			// Use srcalpha for src, since additive are blended with alpha gouraud (for smooth night transition).
			if(i&1)
				TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::one);
			// Else Blendfunc will change during render (negative or not material...)
		}
		// Reset the lightmap (so there is none in Addtive pass).
		TileMaterial.setTexture(1, NULL);


		// Render All material RdrPass.
		//=============================
		ItTileRdrPassSet	itTile;
		for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
		{
			// Get a ref on the render pass. Const cast work because we only modify attribut from CPatchRdrPass 
			// that don't affect the operator< of this class
			CPatchRdrPass	&pass= const_cast<CPatchRdrPass&>(*itTile);
			if(pass.NTris==0)
				continue;

			// Build the PBlock.
			pass.buildPBlock(PBlock);
			// must resetTriList at each end of each material process.
			pass.resetTriList();

			// Setup material.
			// If diffuse part and not pass 0...
			if((i&1)==0 && i>0)
			{
				// Choose beetween negative alpha or not.
				if(pass.BlendType==CPatchRdrPass::Alpha)
					TileMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
				else
					TileMaterial.setBlendFunc(CMaterial::invsrcalpha, CMaterial::srcalpha);
			}
			// Setup Diffuse texture of the tile.
			TileMaterial.setTexture(0, pass.TextureDiffuse);
			// If diffuse part, must enable the lightmap for this pass.
			if((i&1)==0)
			{
				TileMaterial.setTexture(1, pass.LightMap);
			}


			// Render!
			driver->render(PBlock, TileMaterial);
		}
	}


	// 2. Far0Render pass.
	//====================
	// Reset VB inbfos.
	CTessFace::CurrentVB= &FarVB;
	CTessFace::CurrentVertexIndex= 1;
	CPatchRdrPass::resetGlobalTriList();

	// Process all zones.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->renderFar0();
	}

	// Active VB.
	CTessFace::CurrentVB->setNumVertices(CTessFace::CurrentVertexIndex);
	driver->activeVertexBuffer(*CTessFace::CurrentVB);

	// Setup common material.
	FarMaterial.setBlend(false);

	// Render All material RdrPass0.
	ItSPRenderPassSet		itTile=_FarRdrPassSet.begin();
	while (itTile!=_FarRdrPassSet.end())
	{
		CPatchRdrPass	&pass= **itTile;

		// Build the pblock of this render pass
		pass.buildPBlock(PBlock);
		
		// must resetTriList at each end of each material process.
		pass.resetTriList();
		FarMaterial.setTexture(0, pass.TextureDiffuse);
		driver->render(PBlock, FarMaterial);

		// Next render pass
		itTile++;
	}

	// 3. Far1Render pass.
	//====================
	// Reset VB inbfos.
	CTessFace::CurrentVB= &FarVB;
	CTessFace::CurrentVertexIndex= 1;
	CPatchRdrPass::resetGlobalTriList();

	// Process all zones.
	for(it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->renderFar1();
	}

	// Active VB.
	CTessFace::CurrentVB->setNumVertices(CTessFace::CurrentVertexIndex);
	driver->activeVertexBuffer(*CTessFace::CurrentVB);

	// Setup common material.
	FarMaterial.setBlend(true);


	// Render All material RdrPass1.
	itTile=_FarRdrPassSet.begin();
	while (itTile!=_FarRdrPassSet.end())
	{
		CPatchRdrPass	&pass= **itTile;

		// Build the pblock of this render pass
		pass.buildPBlock(PBlock);
		
		// must resetTriList at each end of each material process.
		pass.resetTriList();
		FarMaterial.setTexture(0, pass.TextureDiffuse);
		driver->render(PBlock, FarMaterial);

		// Next render pass
		itTile++;
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
		textName= tile->getFileName(CTile::additive);
	else
		textName= "";
	// If no additive for this tile, rdrpass is NULL.
	if(textName=="")
		tileInfo->AdditiveRdrPass= NULL;
	else
	{
		// Fill rdrpass.
		CPatchRdrPass	pass;
		pass.BlendType= CPatchRdrPass::Additive;
		pass.TextureDiffuse= findTileTexture(textName);
		// no bump for additive, nor LightMap.

		// Fill tileInfo.
		tileInfo->AdditiveRdrPass= findTileRdrPass(pass);
		// Fill UV Info.
		// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
		tileInfo->AdditiveUvScaleBias.x= 1/(2*NL3D::TileSize);
		tileInfo->AdditiveUvScaleBias.y= 1/(2*NL3D::TileSize);
		tileInfo->AdditiveUvScaleBias.z= 1-1/NL3D::TileSize;
	}


	// Fill diffuse/bump part.
	// =======================
	// Fill rdrpass.
	CPatchRdrPass	pass;
	// TODO: hulud neg alpha.
	if(true)
		pass.BlendType= CPatchRdrPass::Alpha;
	else
		pass.BlendType= CPatchRdrPass::NegativeAlpha;
	// The diffuse part for a tile is inevitable.
	if(tile)
		pass.TextureDiffuse= findTileTexture(tile->getFileName(CTile::diffuse));
	else
		pass.TextureDiffuse= new CTextureCross;
	if(tile)
	{
		textName= tile->getFileName(CTile::bump);
		if(textName!="")
			pass.TextureBump= findTileTexture(textName);
	}
	// Do not Fill the LightMap pass here.
	// RdrPass duplicated after.


	// Fill tileInfo.
	tileInfo->DiffuseRdrPass= findTileRdrPass(pass);
	// Fill UV Info.
	// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
	tileInfo->DiffuseUvScaleBias.x= 1/(2*NL3D::TileSize);
	tileInfo->DiffuseUvScaleBias.y= 1/(2*NL3D::TileSize);
	tileInfo->DiffuseUvScaleBias.z= 1-1/NL3D::TileSize;
	tileInfo->BumpUvScaleBias.x= 1/(2*NL3D::TileSize);
	tileInfo->BumpUvScaleBias.y= 1/(2*NL3D::TileSize);
	tileInfo->BumpUvScaleBias.z= 1-1/NL3D::TileSize;


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

	// Release the lighted render passes.
	ItTileRdrPassPtrSet	itPtr;
	for(itPtr= tileInfo->LightedRdrPass.begin(); itPtr!= tileInfo->LightedRdrPass.end(); itPtr++)
	{
		(*itPtr)->RefCount--;
	}
	// Delete those render passes...
	tileInfo->LightedRdrPass.clear();

	delete tileInfo;
	TileInfos[tileId]= NULL;
}


// ***************************************************************************
CPatchRdrPass	*CLandscape::getTileRenderPass(uint16 tileId, bool additiveRdrPass, ITexture *lightmap)
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
		// If no lightmap is given, return the one with no LightMap.
		if(!lightmap)
			return tile->DiffuseRdrPass;
		else
		{
			// Must get the good render pass for the correct lightmaped version of this tile.

			// Copy the render pass from the one with no lightmap.
			CPatchRdrPass	pass;
			pass.BlendType= tile->DiffuseRdrPass->BlendType;
			pass.TextureDiffuse= tile->DiffuseRdrPass->TextureDiffuse;
			pass.TextureBump= tile->DiffuseRdrPass->TextureBump;
			// Set the wanted lightmap.
			pass.LightMap= lightmap;

			// Insert/Retrieve this rdrpass.
			CPatchRdrPass	*rdrpass= findTileRdrPass(pass);

			// If not already inserted in the tile list, insert it.
			// This is important for release...
			ItTileRdrPassPtrSet	itPtr;
			itPtr= tile->LightedRdrPass.find(rdrpass);
			if(itPtr== tile->LightedRdrPass.end())
			{
				tile->LightedRdrPass.insert(rdrpass);
				// Now, we have one more tile which use this lighted rdrpass...
				rdrpass->RefCount++;
			}

			return rdrpass;
		}
	}
}


// ***************************************************************************
void			CLandscape::getTileUvScaleBias(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias)
{
	CTileInfo	*tile= TileInfos[tileId];
	// tile should not be NULL.
	// Because load of tiles are always done in getTileRenderPass(), and this insertion always succeed.
	nlassert(tile);

	switch(bitmapType)
	{
		case CTile::diffuse:
			uvScaleBias= tile->DiffuseUvScaleBias; break;
		case CTile::additive:
			uvScaleBias= tile->AdditiveUvScaleBias; break;
		case CTile::bump:
			uvScaleBias= tile->BumpUvScaleBias; break;
	}
}


// ***************************************************************************
NLMISC::CSmartPtr<ITexture>		CLandscape::getTileTexture(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias)
{
	CPatchRdrPass	*pass;
	if(bitmapType== CTile::additive)
		pass= getTileRenderPass(tileId, true, NULL);
	else
		pass= getTileRenderPass(tileId, false, NULL);
	if(!pass)
		return NULL;
	getTileUvScaleBias(tileId, bitmapType, uvScaleBias);
	if(bitmapType== CTile::diffuse)
		return pass->TextureDiffuse;
	else
		return pass->TextureBump;
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
		if(pass.TextureDiffuse && !pass.TextureDiffuse->loadedIntoDriver())
			drv->setupTexture(*pass.TextureDiffuse);
		// If present and not already setuped...
		if(pass.TextureBump && !pass.TextureBump->loadedIntoDriver())
			drv->setupTexture(*pass.TextureBump);
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

	// Textures are automaticly deleted, but not their entry int the map. 
	// => doesn't matter since findTileTexture() manages this case.
	// And the memory overhead is not a problem (we talk about pointers).
}


// ***************************************************************************
uint		CLandscape::getTileLightMap(CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE], ITexture *&lightmap)
{
	sint	textNum;
	uint	lightMapId;
	/* 
		NB: TextureNear are a grow only Array... TextureNear are never deleted. Why? :
		1/ There is an important issue with releasing texture nears: tiles may acces them (see getTileRenderPass())
		2/ Unused near texture may be uncahced by opengl (and maybe by windows, in memory).
	*/
	// 0. Alloc Near Texture if necessary.
	//====================================
	if(_NFreeLightMaps==0)
	{
		CTextureNear	*text= new CTextureNear(TextureNearSize);
		_TextureNears.push_back(text);
		_NFreeLightMaps+= text->getNbAvailableTiles();
	}

	// 1. Search the first texture which has a free tile.
	//==================================================
	CTextureNear	*nearText= NULL;
	for(textNum=0;textNum<(sint)_TextureNears.size();textNum++)
	{
		nearText= _TextureNears[textNum];
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
	lightmap= nearText;
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
	uvScaleBias.z= scale4;

	// Get the id local in the texture.
	id= tileLightMapId%NbTilesByTexture;

	// Commpute UVBias.
	// Get the coordinate of the tile, in tile number.
	s= id%NbTilesByLine;
	t= id/NbTilesByLine;
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
	_TextureNears[textNum]->releaseTile(id);
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
void			CLandscape::checkBinds()
{
	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		CZone	&curZone= *(*it).second;
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
						nlassert(bo.NPatchs==1);
						nlassert(bo.Next[0]==i);
						nlassert(bo.Edge[0]==j);
					}
				}
			}
		}
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
void			CLandscape::setupStaticLight (CRGBA &diffuse, CRGBA &ambiant, float multiply)
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

} // NL3D
