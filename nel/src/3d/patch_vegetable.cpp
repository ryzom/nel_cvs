/** \file patch_vegetable.cpp
 * CPatch implementation for vegetable management
 *
 * $Id: patch_vegetable.cpp,v 1.1 2001/10/31 10:19:40 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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


#include "3d/patch.h"
#include "3d/vegetable.h"
#include "3d/vegetable_manager.h"
#include "3d/landscape.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"


using namespace std;
using namespace NLMISC;

namespace NL3D 
{


// Suppose a constant size for a tile of 2m*2m.
#define	NL3D_PATCH_TILE_AREA	4


// ***************************************************************************
void		CPatch::generateTileVegetable(uint ts, uint tt)
{
	uint	i;

	// get the vegetableManager
	CVegetableManager	*vegetableManager= getLandscape()->_VegetableManager;

	// Get tile infos for vegetable
	// =========================

	// compute tessBlock coordinate.
	uint	tbs= ts >> 1;
	uint	tbt= tt >> 1;
	// get the vegetable IG in TessBlocks, to store the vegetable.
	nlassert(NL3D_TESSBLOCK_TILESIZE==4);
	CVegetableInstanceGroup		*vegetIg= TessBlocks[tbt * (OrderS>>1) + tbs].VegetableInstanceGroup;

	// get the tileId under this tile (<=> the tile material)
	uint	tileId= Tiles[tt * OrderS + ts].Tile[0];

	// get list of vegetable for this tile.
	const vector<CVegetable*>	&vegetableList= getLandscape()->getTileVegetableList(tileId);
	

	// compute approximate tile position and normal: get the middle
	float	tileU= (ts + 0.5f) / (float)OrderS;
	float	tileV= (tt + 0.5f) / (float)OrderT;
	// TODO_VEGET_OPTIM: too slow, get coordinate from CTessVertex around the tile
	// NB: because temp, incorect here: don't take noise into account.
	CBezierPatch	*bpatch= unpackIntoCache();
	CVector		tilePos= bpatch->eval(tileU, tileV);
	CVector		tileNormal= bpatch->evalNormal(tileU, tileV);

	// compute a rotation matrix with the normal
	CMatrix		matInstance;
	matInstance.setRot(CVector::I, CVector::J, tileNormal);
	// must normalize the matrix. use the vector which is the most orthogonal to tileNormal
	// If tileNormal is much more a J vector, then use plane (I,tileNormal), and vice-versa
	if(fabs(tileNormal.y) > fabs(tileNormal.x))
		matInstance.normalize(CMatrix::ZXY);
	else
		matInstance.normalize(CMatrix::ZYX);


	// prepare color.

	// say that ambient never change.
	CRGBA	ambient= getLandscape()->_VegetableAmbient;
	CRGBAF	ambientF= ambient;
	CRGBA	diffuseMaterial= getLandscape()->_VegetableDiffuse;

	// Compute the tileLightmap (not modified by tileColor).
	static	uint8	tileLumelmap[NL_LUMEL_BY_TILE * NL_LUMEL_BY_TILE];
	getTileLumelmapPrecomputed(ts, tt, tileLumelmap, NL_LUMEL_BY_TILE);
	// compute diffuse color by substracting from ambient.
	CRGBAF	diffuseColorF[NL_LUMEL_BY_TILE * NL_LUMEL_BY_TILE];
	// TODO_VEGET_OPTIM: optimize this.
	// For all lumel of this tile.
	for(i= 0; i<NL_LUMEL_BY_TILE*NL_LUMEL_BY_TILE; i++)
	{
		// mul by 2, because shade is done twice here: by vertex, and by landscape.
		sint	R= 2*tileLumelmap[i] * diffuseMaterial.R;
		sint	G= 2*tileLumelmap[i] * diffuseMaterial.G;
		sint	B= 2*tileLumelmap[i] * diffuseMaterial.B;
		clamp(R, 0, 65025);
		clamp(G, 0, 65025);
		clamp(B, 0, 65025);
		diffuseColorF[i].R= R / 65025.f;
		diffuseColorF[i].G= G / 65025.f;
		diffuseColorF[i].B= B / 65025.f;
		diffuseColorF[i].A= 1;
	}


	// for all vegetable of this list, generate instances.
	// =========================
	uint	numVegetable= vegetableList.size();
	for(i= 0; i<numVegetable; i++)
	{
		// get the vegetable
		CVegetable	*veget=	vegetableList[i];

		// generate instance for this vegetable.
		static	vector<CVector2f>	instanceUV;
		veget->generateGroup(tilePos, tileNormal, NL3D_PATCH_TILE_AREA, instanceUV);

		// For all instance, generate the real instances.
		// TODO_VEGET_OPTIM: too slow here too...
		for(uint j=0; j<instanceUV.size(); j++)
		{
			// generate the position in world Space.
			// instancePos is in [0..1] interval, which maps to a tile, so explode to the patch
			tileU= (ts + instanceUV[j].x) / (float)OrderS;
			tileV= (tt + instanceUV[j].y) / (float)OrderT;
			// eval the position in 3d space. use same normal for rotation.
			matInstance.setPos( bpatch->eval(tileU, tileV) );

			// peek color into the lightmap.
			sint	lumelS= (sint)floor(instanceUV[j].x * NL_LUMEL_BY_TILE);
			sint	lumelT= (sint)floor(instanceUV[j].y * NL_LUMEL_BY_TILE);
			clamp(lumelS, 0, NL_LUMEL_BY_TILE-1);
			clamp(lumelT, 0, NL_LUMEL_BY_TILE-1);

			// generate the instance of the vegetable
			veget->generateInstance(vegetIg, matInstance, ambientF, 
				diffuseColorF[ (lumelT<<NL_LUMEL_BY_TILE_SHIFT) + lumelS ]);
		}
	}
}




} // NL3D
