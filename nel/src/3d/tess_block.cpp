/** \file tess_block.cpp
 * <File description>
 *
 * $Id: tess_block.cpp,v 1.9 2002/02/28 12:59:51 besson Exp $
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

#include "3d/tess_block.h"
#include "3d/patch_rdr_pass.h"
#include "3d/landscape_face_vector_manager.h"


namespace NL3D 
{


CPlane	CTessBlock::CurrentPyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE];



// ***************************************************************************
CTessBlock::CTessBlock()
{
	_Patch= NULL;

	// init bounding info.
	Empty= true;
	// By default, the tessBlock is clipped.
	Clipped= true;
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
	TileMaterialRefCount= 0;

	// Micro-vegetation.
	VegetableBlock= NULL;
}


// ***************************************************************************
CTessBlock::~CTessBlock()
{
	if(isInModifyList())
	{
		removeFromModifyList();
	}
}

// ***************************************************************************
void			CTessBlock::init(CPatch *patch)
{
	_Patch= patch;
}
// ***************************************************************************
CPatch			*CTessBlock::getPatch()
{
	return _Patch;
}



// ***************************************************************************
void			CTessBlock::extendSphere(const CVector &vec)
{
	if(Empty)
	{
		Empty= false;
		BBox.setCenter(vec);
		BBox.setHalfSize(CVector::Null);
		BSphere.Center= vec;
		BSphere.Radius= 0;
	}
	else
	{
		if( !BBox.include(vec) )
		{
			BBox.extend(vec);
			BSphere.Center= BBox.getCenter();
			BSphere.Radius= BBox.getRadius();
		}
	}

}
// ***************************************************************************
void			CTessBlock::resetClip()
{
	Clipped= false;
	FullFar1= false;
	EmptyFar1= false;
}


// ***************************************************************************
void			CTessBlock::forceClip()
{
	Clipped= true;
}


// ***************************************************************************
void			CTessBlock::clip()
{
	Clipped= false;
	for(sint i=0;i<NL3D_TESSBLOCK_NUM_CLIP_PLANE;i++)
	{
		// If entirely out.
		if(!BSphere.clipBack( CTessBlock::CurrentPyramid[i] ))
		{
			Clipped= true;
			break;
		}
	}
}
// ***************************************************************************
void			CTessBlock::clipFar(const CVector &refineCenter, float tileDistNear, float farTransition)
{
	float	r= (refineCenter-BSphere.Center).norm();
	if( (r-BSphere.Radius) > tileDistNear)
	{
		FullFar1= true;
	}
	else
	{
		if( (r+BSphere.Radius) < (tileDistNear-farTransition) )
			EmptyFar1= true;
	}
}



// ***************************************************************************
void			CTessBlock::refillFaceVectorFar0()
{
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		nlassert(Far0FaceVector!=NULL);

		// Fill this faceVector, with FarFaceList
		CTessFace	*pFace;
		uint32		*dest= Far0FaceVector->TriPtr;
		for(pFace= FarFaceList.begin(); pFace; pFace= (CTessFace*)pFace->Next)
		{
			*(dest++)= pFace->FVBase->Index0;
			*(dest++)= pFace->FVLeft->Index0;
			*(dest++)= pFace->FVRight->Index0;
		}
	}
}


// ***************************************************************************
void			CTessBlock::createFaceVectorFar0(CLandscapeFaceVectorManager &mgr)
{
	nlassert(Far0FaceVector==NULL);
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		// Create a faceVector of the wanted triangles size.
		Far0FaceVector= mgr.createFaceVector(FarFaceList.size());

		// init.
		refillFaceVectorFar0();
	}

}
// ***************************************************************************
void			CTessBlock::deleteFaceVectorFar0(CLandscapeFaceVectorManager &mgr)
{
	if(Far0FaceVector)
	{
		mgr.deleteFaceVector(Far0FaceVector);
		Far0FaceVector= NULL;
	}
}

// ***************************************************************************
void			CTessBlock::refillFaceVectorFar1()
{
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		nlassert(Far1FaceVector!=NULL);
		// Fill this faceVector, with FarFaceList
		CTessFace	*pFace;
		uint32		*dest= Far1FaceVector->TriPtr;
		for(pFace= FarFaceList.begin(); pFace; pFace= (CTessFace*)pFace->Next)
		{
			*(dest++)= pFace->FVBase->Index1;
			*(dest++)= pFace->FVLeft->Index1;
			*(dest++)= pFace->FVRight->Index1;
		}
	}
}

// ***************************************************************************
void			CTessBlock::createFaceVectorFar1(CLandscapeFaceVectorManager &mgr)
{
	nlassert(Far1FaceVector==NULL);
	// If size is not 0.
	if(FarFaceList.size()>0)
	{
		// Create a faceVector of the wanted triangles size.
		Far1FaceVector= mgr.createFaceVector(FarFaceList.size());

		// init.
		refillFaceVectorFar1();
	}
}
// ***************************************************************************
void			CTessBlock::deleteFaceVectorFar1(CLandscapeFaceVectorManager &mgr)
{
	if(Far1FaceVector)
	{
		mgr.deleteFaceVector(Far1FaceVector);
		Far1FaceVector= NULL;
	}
}


// ***************************************************************************
void			CTessBlock::refillFaceVectorTile()
{
	// For all tiles existing, and for all facePass existing, fill the faceVector.
	for(uint tileId=0; tileId<NL3D_TESSBLOCK_TILESIZE; tileId++)
	{
		// if tile exist.
		if(RdrTileRoot[tileId])
		{
			// For all Pass faces of the tile.
			for(uint facePass=0; facePass<NL3D_MAX_TILE_FACE; facePass++)
			{
				CTessList<CTileFace>	&faceList= RdrTileRoot[tileId]->TileFaceList[facePass];
				CLandscapeFaceVector	*faceVector= RdrTileRoot[tileId]->TileFaceVectors[facePass];
				// If some triangles create them.
				if(faceList.size()>0)
				{
					nlassert( faceVector!=NULL );

					// Fill this faceVector, with the TileFaceList
					CTileFace	*pFace;
					uint32		*dest= faceVector->TriPtr;
					for(pFace= faceList.begin(); pFace; pFace= (CTileFace*)pFace->Next)
					{
						*(dest++)= pFace->VBase->Index;
						*(dest++)= pFace->VLeft->Index;
						*(dest++)= pFace->VRight->Index;
					}
				}
			}
		}
	}
}


// ***************************************************************************
void			CTessBlock::createFaceVectorTile(CLandscapeFaceVectorManager &mgr)
{
	// For all tiles existing, and for all facePass existing, create the faceVector.
	for(uint tileId=0; tileId<NL3D_TESSBLOCK_TILESIZE; tileId++)
	{
		// if tile exist.
		if(RdrTileRoot[tileId])
		{
			// For all Pass faces of the tile.
			for(uint facePass=0; facePass<NL3D_MAX_TILE_FACE; facePass++)
			{
				CTessList<CTileFace>	&faceList= RdrTileRoot[tileId]->TileFaceList[facePass];
				CLandscapeFaceVector	*&faceVector= RdrTileRoot[tileId]->TileFaceVectors[facePass];
				// If some triangles create them.
				if(faceList.size()>0)
				{
					// Create a faceVector of the wanted triangles size.
					faceVector= mgr.createFaceVector(faceList.size());
				}
			}
		}
	}

	// init.
	refillFaceVectorTile();
}
// ***************************************************************************
void			CTessBlock::deleteFaceVectorTile(CLandscapeFaceVectorManager &mgr)
{
	// For all tiles existing, and for all facePass existing, delete the faceVector.
	for(uint tileId=0; tileId<NL3D_TESSBLOCK_TILESIZE; tileId++)
	{
		// if tile exist.
		if(RdrTileRoot[tileId])
		{
			// For all Pass faces of the tile.
			for(uint facePass=0; facePass<NL3D_MAX_TILE_FACE; facePass++)
			{
				CTessList<CTileFace>	&faceList= RdrTileRoot[tileId]->TileFaceList[facePass];
				CLandscapeFaceVector	*&faceVector= RdrTileRoot[tileId]->TileFaceVectors[facePass];
				// If the faceVector exist, delete it.
				if(faceVector)
				{
					mgr.deleteFaceVector(faceVector);
					faceVector= NULL;
				}
			}
		}
	}
}


// ***************************************************************************
void			CTessBlock::appendToModifyListAndDeleteFaceVector(CTessBlock &root, CLandscapeFaceVectorManager &mgr)
{
	// If already appened, return.
	if(isInModifyList())
		return;

	// append to root.
	_PrecToModify= &root;
	_NextToModify= root._NextToModify;
	if(root._NextToModify)
		root._NextToModify->_PrecToModify= this;
	root._NextToModify= this;

	// Then delete All faceVector that may exist.
	deleteFaceVectorFar0(mgr);
	deleteFaceVectorFar1(mgr);
	deleteFaceVectorTile(mgr);
}
// ***************************************************************************
void			CTessBlock::removeFromModifyList()
{
	// If already removed, return.
	// _PrecToModify must be !NULL
	if(!isInModifyList())
		return;

	// unlink.
	_PrecToModify->_NextToModify= _NextToModify;
	if(_NextToModify)
		_NextToModify->_PrecToModify= _PrecToModify;
	_PrecToModify= NULL;
	_NextToModify= NULL;
}


} // NL3D
