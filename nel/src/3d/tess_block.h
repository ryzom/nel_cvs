/** \file tess_block.h
 * <File description>
 *
 * $Id: tess_block.h,v 1.3 2001/09/14 09:44:25 berenguier Exp $
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

#ifndef NL_TESS_BLOCK_H
#define NL_TESS_BLOCK_H

#include "nel/misc/types_nl.h"
#include "3d/tessellation.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/bsphere.h"
#include <list>


namespace NL3D 
{


// clip Only with Left/Right and Top/Bottom planes, for faster clip.
#define	NL3D_TESSBLOCK_NUM_CLIP_PLANE	4


// block of 2*2 tiles, for accurate clipping.
#define	NL3D_TESSBLOCK_TILESIZE	4


class	CPatchRdrPass;
class	CLandscapeFaceVectorManager;


// ***************************************************************************
/**
 * A block of 2*2 tiles, for accurate clipping.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTessBlock
{
private:
	// Clip info.
	NLMISC::CAABBox		BBox;
	NLMISC::CBSphere	BSphere;
	bool		Empty;			// empty bbox? (first extend).

public:
	// Clip result.
	bool		Clipped;
	bool		FullFar1;		// block Entirely IN transition. (all alpha of Far1 should be 255)
	bool		EmptyFar1;		// block Entirely OUT transition. (all alpha of Far1 should be 0)

	// Vertices.
	CTessList<CTessFarVertex>	FarVertexList;
	CTessList<CTessNearVertex>	NearVertexList;

	// Faces.
	CTessList<CTessFace>		FarFaceList;
	CTileMaterial				*RdrTileRoot[NL3D_TESSBLOCK_TILESIZE];
	// contains FarFaceList.size() + "RdrTileRoot.size()"
	uint						FaceTileMaterialRefCount;

	// FaceVectors.
	CLandscapeFaceVector		*Far0FaceVector;
	CLandscapeFaceVector		*Far1FaceVector;
	// tiles one are stored in material.


	// For memory optimisation, Info for Lightmap per block of 2x2 tiles are stored here too.
	uint						LightMapId;
	uint						LightMapRefCount;
	CPatchRdrPass				*LightMapRdrPass;

public:
	CTessBlock()
	{
		_Patch= NULL;

		// init bounding info.
		Empty= true;
		Clipped= false;
		FullFar1= false;
		EmptyFar1= false;

		// init vert/face list.
		for(sint i=0;i<NL3D_TESSBLOCK_TILESIZE;i++)
		{
			RdrTileRoot[i]=NULL;
		}

		// init LightMap.
		LightMapRefCount= 0;

		Far0FaceVector= NULL;
		Far1FaceVector= NULL;

		_PrecToModify= NULL;
		_NextToModify= NULL;

		FaceTileMaterialRefCount= 0;
	}

	// in dtor, remove me from list of TessBlock to modify, if needed.
	~CTessBlock();

	// Must have a patch, for good TessBlock FaceVector (re)construction.
	void			init(CPatch *patch);
	CPatch			*getPatch();

	// BBox.
	void			extendSphere(const CVector &vec);

	// Reset clip to no clipped.
	void			resetClip();
	// Clip. (set Clipped to true/false). Use the CurrentPyramid static.
	static CPlane	CurrentPyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE];
	void			clip();
	// Clip Tile/Far. Use CTessFace::RefineCenter, CTessFace::TileDistNear, and CTessFace::FarTransition.
	void			clipFar(const CVector &refineCenter, float tileDistNear, float farTransition);


	// create and build FaceVector, from the FaceList.
	void			createFaceVectorFar0(CLandscapeFaceVectorManager &mgr);
	void			deleteFaceVectorFar0(CLandscapeFaceVectorManager &mgr);
	void			createFaceVectorFar1(CLandscapeFaceVectorManager &mgr);
	void			deleteFaceVectorFar1(CLandscapeFaceVectorManager &mgr);
	void			createFaceVectorTile(CLandscapeFaceVectorManager &mgr);
	void			deleteFaceVectorTile(CLandscapeFaceVectorManager &mgr);



	// append/remove to modify list. This is for FaceVector update durint refine(), addZone() ....
	bool			isInModifyList() const {return _PrecToModify!=NULL;}
	// append to modify list has the side effect od deleting ALL FaceVector.
	void			appendToModifyListAndDeleteFaceVector(CTessBlock &root, CLandscapeFaceVectorManager &mgr);
	void			removeFromModifyList();


public:
	// For CLandscape only.
	CTessBlock		*getNextToModify() {return _NextToModify;}

private:
	CPatch			*_Patch;

	// If in list, prec is !NULL.
	CTessBlock		*_PrecToModify;
	CTessBlock		*_NextToModify;
};


} // NL3D


#endif // NL_TESS_BLOCK_H

/* End of tess_block.h */
