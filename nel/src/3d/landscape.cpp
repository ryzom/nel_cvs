/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.14 2000/12/01 18:41:18 berenguier Exp $
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

		// Null
		memset (&_Data[0][0], 16*16*4, 0);

		// Cross
		for (int x=0; x<16; x++)
		{
			_Data[0][(x*4)]=0xff;
			_Data[0][(x*4)+1]=0xff;
			_Data[0][(x*4)+2]=0xff;

			_Data[0][(x*16*4)]=0xff;
			_Data[0][(x*16*4)+1]=0xff;
			_Data[0][(x*16*4)+2]=0xff;

			_Data[0][(x*4)+(x*4*16)]=0xff;
			_Data[0][(x*4)+(x*4*16)+1]=0xff;
			_Data[0][(x*4)+(x*4*16)+2]=0xff;

			_Data[0][(16*4-x*4)+(x*4*16)]=0xff;
			_Data[0][(16*4-x*4)+(x*4*16)+1]=0xff;
			_Data[0][(16*4-x*4)+(x*4*16)+2]=0xff;
		}
	}
};

// ***************************************************************************
// TODO: may change this.
// The size of a tile, in pixel. UseFull for HalfPixel Scale/Bias.
const	float TileSize= 128;


// ***************************************************************************
CLandscape::CLandscape()
{
	TileInfos.resize(NL3D::NbTilesMax);
	fill(TileInfos.begin(), TileInfos.end(), (CTileInfo*)NULL);

	_TileDistNear=50.f;
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
	FarVB.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0] | IDRV_VF_RGBA );
	FarVB.reserve(1024);

	// v3f/t2f0/t2f1/c4ub
	TileVB.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0] | IDRV_VF_UV[1] | IDRV_VF_RGBA );
	TileVB.reserve(1024);

	// TODO_BUMP: Need t3f in IDriver.
	// v3f/t2f0/t2f1/t2f2/t2f3/c4ub


	// Fill Far mat and rdr pass.
	// TODO_TEXTURE.
	// For TEST only here, create The far rdrPass with random texture.
	// Must init his BlendFunction here!!! becaus it switch between blend on/off during rendering.
	FarMaterial.initUnlit();
	FarMaterial.setSrcBlend(CMaterial::srcalpha);
	FarMaterial.setDstBlend(CMaterial::invsrcalpha);
	FarRdrPass.TextureDiffuse= new CTextureFile("maps/slash.tga");

	// Init material for tile.
	TileMaterial.initUnlit();
}


// ***************************************************************************
bool			CLandscape::addZone(const CZone	&newZone)
{
	// -1. Update globals
	updateGlobals ();

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
	updateGlobals ();

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
	updateGlobals ();

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
	updateGlobals ();

	CTessFace::RefineCenter= refineCenter;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->clip(pyramid);
	}
}
// ***************************************************************************
void			CLandscape::refine(const CVector &refineCenter)
{
	// -1. Update globals
	updateGlobals ();

	CTessFace::RefineCenter= refineCenter;
	// Increment the update date.
	CTessFace::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->refine();
	}
}
// ***************************************************************************
void			CLandscape::updateGlobals () const
{
	// Setup CTessFace static members...
	CTessFace::TileDistNear = _TileDistNear;
	CTessFace::TileDistFar = CTessFace::TileDistNear+40;
	CTessFace::TileDistNearSqr = sqr(CTessFace::TileDistNear);
	CTessFace::TileDistFarSqr = sqr(CTessFace::TileDistFar);
	CTessFace::OOTileDistDeltaSqr = 1.0f / (CTessFace::TileDistFarSqr - CTessFace::TileDistNearSqr);
}
// ***************************************************************************
void			CLandscape::render(IDriver *driver, const CVector &refineCenter, bool doTileAddPass)
{
	CTessFace::RefineCenter= refineCenter;
	ItZoneMap	it;
	sint		i;

	// -1. Update globals
	updateGlobals ();

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
		for(it= Zones.begin();it!=Zones.end();it++)
		{
			(*it).second->renderTile(i);
		}

		// Active VB.
		CTessFace::CurrentVB->setNumVertices(CTessFace::CurrentVertexIndex);
		driver->activeVertexBuffer(*CTessFace::CurrentVB);

		// Setup common material for this pass.
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

		// Render All material RdrPass.
		ItTileRdrPassSet	itTile;
		for(itTile= TileRdrPassSet.begin(); itTile!= TileRdrPassSet.end(); itTile++)
		{
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
			// Setup texture.
			TileMaterial.setTexture(pass.TextureDiffuse);

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

	// Render All material RdrPass.
	// TODO_TEXTURE. For TEST only here. Do it on FarRdrPass.
	FarRdrPass.buildPBlock(PBlock);
	// must resetTriList at each end of each material process.
	FarRdrPass.resetTriList();
	FarMaterial.setTexture(FarRdrPass.TextureDiffuse);
	driver->render(PBlock, FarMaterial);


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

	// Render All material RdrPass.
	// TODO_TEXTURE. For TEST only here. Do it on FarRdrPass.
	FarRdrPass.buildPBlock(PBlock);
	// must resetTriList at each end of each material process.
	FarRdrPass.resetTriList();
	FarMaterial.setTexture(FarRdrPass.TextureDiffuse);
	driver->render(PBlock, FarMaterial);


	// 4. "Release" texture materials.
	//================================
	FarMaterial.setTexture(NULL, 0);
	FarMaterial.setTexture(NULL, 1);
	FarMaterial.setTexture(NULL, 2);
	FarMaterial.setTexture(NULL, 3);
	TileMaterial.setTexture(NULL, 0);
	TileMaterial.setTexture(NULL, 1);
	TileMaterial.setTexture(NULL, 2);
	TileMaterial.setTexture(NULL, 3);

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
	// If just inserted, SmartPtr is NULL!!  :)
	// This test too if the RefPtr is NULL... (tile released)
	if(!text)
	{
		TileTextureMap[textName]= text= new CTextureFile(textName);
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
		// no bump for additive.

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
	// TODODODO: hulud neg alpha.
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

	delete tileInfo;
	TileInfos[tileId]= NULL;
}


// ***************************************************************************
CPatchRdrPass	*CLandscape::getTileRenderPass(uint16 tileId, bool additiveRdrPass)
{
	/*
	// TODO_TEXTURE and TODO_BUMP.
		Actually, there should be more than One RdrPass per tile.
		The correct thing is: One per tuple  "tile - NearLightMapTexture".
		Optimisations could be done by:
			- have NearLightMapTexture of all patchs sticked together in multiple as big as possible texture.
				(maybe only one??)

		NB: a tile is himself a tuple of: diffuseTexture/bumpTexture/additiveTexture.
	*/

	CTileInfo	*tile= TileInfos[tileId];

	// If not here, create it.
	if(tile==NULL)
	{
		// Force loading of tile.
		loadTile(tileId);

		tile= TileInfos[tileId];
		nlassert(tile!=NULL);
	}

	// Retrieve.
	if(additiveRdrPass)
		return tile->AdditiveRdrPass;
	else
		return tile->DiffuseRdrPass;
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
		pass= getTileRenderPass(tileId, true);
	else
		pass= getTileRenderPass(tileId, false);
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
		if(pass.TextureDiffuse && pass.TextureDiffuse->DrvInfos==NULL)
			drv->setupTexture(*pass.TextureDiffuse);
		// If present and not already setuped...
		if(pass.TextureBump && pass.TextureBump->DrvInfos==NULL)
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

	// Textures are automaticly deleted, but not their entry. => doesn't matter since findTileTexture() manages this case.
	// But if CRefPtr doesn't work, textures are not deleted...
}


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


} // NL3D
