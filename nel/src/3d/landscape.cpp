/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.7 2000/11/22 13:15:24 berenguier Exp $
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
CLandscape::CLandscape()
{
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

	FarText= new CTextureFile("maps/slash.tga");
	TileText= new CTextureFile("maps/xray.tga");


	// Fill mat and rdr pass.
	// Must init their BlendFunction here!!! becaus they switch between blend on/off during rendering.
	FarMat.initUnlit();
	FarMat.setSrcBlend(CMaterial::srcalpha);
	FarMat.setDstBlend(CMaterial::invsrcalpha);
	FarMat.setTexture(FarText);
	TileMat.initUnlit();
	TileMat.setSrcBlend(CMaterial::srcalpha);
	TileMat.setDstBlend(CMaterial::invsrcalpha);
	TileMat.setTexture(TileText);
	FarRdrPass.Mat= &FarMat;
	TileRdrPass.Mat= &TileMat;
}


// ***************************************************************************
bool			CLandscape::addZone(const CZone	&newZone)
{
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
	CTessFace::RefineCenter= refineCenter;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->clip(pyramid);
	}
}
// ***************************************************************************
void			CLandscape::refine(const CVector &refineCenter)
{
	CTessFace::RefineCenter= refineCenter;
	// Increment the update date.
	CTessFace::CurrentDate++;

	for(ItZoneMap it= Zones.begin();it!=Zones.end();it++)
	{
		(*it).second->refine();
	}
}
// ***************************************************************************
void			CLandscape::render(IDriver *driver, const CVector &refineCenter, bool doTileAddPass)
{
	CTessFace::RefineCenter= refineCenter;
	ItZoneMap	it;
	sint		i;


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
		// TODO_TEXTURE. For TEST only here. Do it on TileRdrPass.
		TileRdrPass.buildPBlock(PBlock);
		// must resetTriList at each end of each material process.
		TileRdrPass.resetTriList();
		if(i==0)
			TileRdrPass.Mat->setBlend(false);
		else
			TileRdrPass.Mat->setBlend(true);
		driver->render(PBlock, *TileRdrPass.Mat);
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
	FarRdrPass.Mat->setBlend(false);
	driver->render(PBlock, *FarRdrPass.Mat);


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
	FarRdrPass.Mat->setBlend(true);
	driver->render(PBlock, *FarRdrPass.Mat);


}



} // NL3D
