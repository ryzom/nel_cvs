/** \file landscape.cpp
 * <File description>
 *
 * $Id: landscape.cpp,v 1.1 2000/11/06 15:04:12 berenguier Exp $
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

	// TODO: bump. Need t3f in IDriver.
	// v3f/t2f0/t2f1/t2f2/t2f3/c4ub


	// TODO_TEXTURE.
	// For TEST only here, create 2 rdrPass with random texture.
	FarText= new CTexture;
	TileText= new CTexture;
	FarText->resize(64,64);
	TileText->resize(64,64);
	vector<CRGBA>	image;
	image.resize(64*64);
	for(int y=0;y<64;y++)
	{
		for(int x=0;x<64;x++)
		{
			image[y*64+x].R= rand()&255;
			image[y*64+x].G= rand()&255;
			image[y*64+x].B= rand()&255;
			image[y*64+x].A= 255;
		}
	}
	FarText->fillData(image);
	TileText->fillData(image);

	// Fill mat and rdr pass.
	//FarMat.setDefault();
	FarMat.setTexture(FarText);
	//TileMat.setDefault();
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
	for(int i=0;i<(sint)zoneIds.size();i++)
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
void			CLandscape::render(const CVector &refineCenter, bool doTileAddPass)
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
		IDriver::currentDriver()->activeVertexBuffer(*CTessFace::CurrentVB);

		// Render All material RdrPass.
		// TODO_TEXTURE. For TEST only here. Do it on TileRdrPass.
		TileRdrPass.buildPBlock(PBlock);
		// must resetTriList at each end of each material process.
		TileRdrPass.resetTriList();
		IDriver::currentDriver()->render(PBlock, *TileRdrPass.Mat);
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
	IDriver::currentDriver()->activeVertexBuffer(*CTessFace::CurrentVB);

	// Render All material RdrPass.
	// TODO_TEXTURE. For TEST only here. Do it on FarRdrPass.
	FarRdrPass.buildPBlock(PBlock);
	// must resetTriList at each end of each material process.
	FarRdrPass.resetTriList();
	IDriver::currentDriver()->render(PBlock, *FarRdrPass.Mat);


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
	IDriver::currentDriver()->activeVertexBuffer(*CTessFace::CurrentVB);

	// Render All material RdrPass.
	// TODO_TEXTURE. For TEST only here. Do it on FarRdrPass.
	FarRdrPass.buildPBlock(PBlock);
	// must resetTriList at each end of each material process.
	FarRdrPass.resetTriList();
	IDriver::currentDriver()->render(PBlock, *FarRdrPass.Mat);


}


// ***************************************************************************
// ***************************************************************************
// The Landscape in MOT.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CLandscapeModel::registerBasic()
{
	CMOT::registerModel(LandscapeId, TransformId, CLandscapeModel::creator);
	CMOT::registerObs(ClipTravId, LandscapeId, CLandscapeClipObs::creator);
	CMOT::registerObs(RenderTravId, LandscapeId, CLandscapeRenderObs::creator);
}

// ***************************************************************************
bool	CLandscapeClipObs::clip(IBaseClipObs *caller, bool &renderable)
{
	renderable= true;
	
	CLandscapeModel		*landModel= (CLandscapeModel*)Model;
	if(landModel->Landscape)
	{
		CClipTrav		*trav= (CClipTrav*)Trav;
		vector<CPlane>	&pyramid= trav->WorldPyramid;
		// We are sure that pyramid has normalized plane normals.
		landModel->Landscape->clip(trav->CamPos, pyramid);
	}

	// Well, always visible....
	return true;
}

// ***************************************************************************
void	CLandscapeRenderObs::traverse(IObs *caller)
{
	CLandscapeModel		*landModel= (CLandscapeModel*)Model;
	if(landModel->Landscape)
	{
		CRenderTrav		*trav= (CRenderTrav*)Trav;

		// First, refine.
		landModel->Landscape->refine(trav->CamPos);
		// then render.
		landModel->Landscape->render(trav->CamPos);
	}
}


} // RK3D
