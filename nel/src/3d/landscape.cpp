/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.10 2000/12/01 11:14:46 corvazier Exp $
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
// TODO: may change this.
// The size of a tile, in pixel. UseFull for HalfPixel Scale/Bias.
const	float TileSize= 128;


// ***************************************************************************
CLandscape::CLandscape()
{
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


	// TODO_TEXTURE.
	// For TEST only here, create 2 rdrPass with random texture.

	// Fill mat and rdr pass.
	// Must init their BlendFunction here!!! becaus they switch between blend on/off during rendering.
	FarRdrPass.Mat.initUnlit();
	FarRdrPass.Mat.setSrcBlend(CMaterial::srcalpha);
	FarRdrPass.Mat.setDstBlend(CMaterial::invsrcalpha);
	FarRdrPass.Mat.setTexture(new CTextureFile("maps/slash.tga"));
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

		// Render All material RdrPass.
		ItTileRdrPassMap	itTile;
		for(itTile= TileRdrPassMap.begin(); itTile!= TileRdrPassMap.end(); itTile++)
		{
			CPatchRdrPass	&pass= (*itTile).second.RdrPass;
			if(pass.NTris==0)
				continue;

			// Build the PBlock.
			pass.buildPBlock(PBlock);
			// must resetTriList at each end of each material process.
			pass.resetTriList();
			if(i==0)
				pass.Mat.setBlend(false);
			else
				pass.Mat.setBlend(true);
			// Render!
			driver->render(PBlock, pass.Mat);
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

	// Render All material RdrPass.
	// TODO_TEXTURE. For TEST only here. Do it on FarRdrPass.
	FarRdrPass.buildPBlock(PBlock);
	// must resetTriList at each end of each material process.
	FarRdrPass.resetTriList();
	FarRdrPass.Mat.setBlend(false);
	driver->render(PBlock, FarRdrPass.Mat);


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

	// Render All material RdrPass.
	// TODO_TEXTURE. For TEST only here. Do it on FarRdrPass.
	FarRdrPass.buildPBlock(PBlock);
	// must resetTriList at each end of each material process.
	FarRdrPass.resetTriList();
	FarRdrPass.Mat.setBlend(true);
	driver->render(PBlock, FarRdrPass.Mat);


}



// ***************************************************************************
void			CLandscape::loadTile(const CTileKey &key)
{
	CTile		*tile;
	bool		tileOk= true;

	// Retrieve or create texture.
	// ===========================
	if(key.TileId>=TileBank.getTileCount())
		tile= NULL;
	else
		tile= TileBank.getTile(key.TileId);
	string		textName;
	if(!key.Additive)
	{
		// Diffuse part, Must be here, so always return some texture, dummy texture if necessary.
		if(tile==NULL)
			textName= "YourMotherInShort";		// To have the dummy texture "?"  :)
		else
			textName= tile->getFileName(CTile::diffuse);
	}
	else
	{
		// If no additive pass for this tile, return NULL.
		if(tile==NULL)
			tileOk= false;
		else
		{
			textName= tile->getFileName(CTile::additive);
			if(textName=="")
				tileOk= false;
		}
	}
	ITexture	*text;
	if(tileOk)
	{
		text= TileTextureMap[textName];
		// If just inserted, SmartPtr is NULL!!  :)
		if(!text)
		{
			TileTextureMap[textName]= text= new CTextureFile(textName);
		}
	}

	// Insert/fill the render pass.
	//=============================
	ItTileRdrPassMap	it;
	it= TileRdrPassMap.insert(TTileRdrPassMap::value_type(key, CTileInfo())).first;
	if(tileOk)
	{
		(*it).second.TileOk= true;
		// Fill rdr pass info.
		CMaterial	&mat= ((*it).second).RdrPass.Mat;
		mat.initUnlit();
		// Must init their BlendFunction here!!! becaus they switch between blend on/off during rendering.
		mat.setSrcBlend(CMaterial::srcalpha);
		mat.setDstBlend(CMaterial::invsrcalpha);
		mat.setTexture(text);
		// Fill UV Info.
		// NB: for now, One Tile== One Texture, so UVScaleBias is simple.
		(*it).second.UvScaleBias.x= 1/(2*NL3D::TileSize);
		(*it).second.UvScaleBias.y= 1/(2*NL3D::TileSize);
		(*it).second.UvScaleBias.z= 1-1/NL3D::TileSize;
	}
	else
	{
		(*it).second.TileOk= false;
	}
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

	ItTileRdrPassMap	it;
	CTileKey			key;
	key.TileId= tileId;
	key.Additive= additiveRdrPass;
	it= TileRdrPassMap.find(key);

	// If not here, create it.
	if(it== TileRdrPassMap.end())
	{
		// Force loading of tile.
		loadTile(key);

		it= TileRdrPassMap.find(key);
		nlassert(it!=TileRdrPassMap.end());
	}

	// Retrieve.
	CTileInfo	&tileInfo= (*it).second;
	if(tileInfo.TileOk)
		return &(tileInfo.RdrPass);
	else
		return NULL;
}


// ***************************************************************************
void			CLandscape::getTileUvScaleBias(sint tileId, bool additiveRdrPass, CVector &uvScaleBias)
{
	ItTileRdrPassMap	it;
	CTileKey			key;
	key.TileId= tileId;
	key.Additive= additiveRdrPass;
	it= TileRdrPassMap.find(key);

	if(it== TileRdrPassMap.end())
	{
		// WE SHOULD NOT BE HERE!!
		// Because insertion of tiles are always done in getTileRenderPass(), and this insertion 
		// always succeed (or return NULL => no tile...)
		nlstop;
	}
	else
	{
		uvScaleBias= (*it).second.UvScaleBias;
	}
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
