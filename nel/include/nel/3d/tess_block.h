/** \file tess_block.h
 * <File description>
 *
 * $Id: tess_block.h,v 1.1 2001/02/20 11:03:39 berenguier Exp $
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
#include "nel/3d/tessellation.h"
#include "nel/3d/aabbox.h"
#include "nel/3d/bsphere.h"
#include <list>


namespace NL3D 
{


// block of 2*2 tiles, for accurate clipping.
#define	NL3D_TESSBLOCK_TILESIZE	4


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
	CAABBox		BBox;
	CBSphere	BSphere;
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


public:
	CTessBlock()
	{
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
	}

	// BBox.
	void			extendSphere(const CVector &vec);

	// Reset clip to no clipped.
	void			resetClip();
	// Clip. (set Clipped to true/false).
	void			clip(const std::vector<CPlane>	&pyramid);
	// Clip Tile/Far. Use CTessFace::RefineCenter, CTessFace::TileDistNear, and CTessFace::FarTransition.
	void			clipFar(const CVector &refineCenter, float tileDistNear, float farTransition);

};


} // NL3D


#endif // NL_TESS_BLOCK_H

/* End of tess_block.h */
