/** \file patch_render.cpp
 * CPatch implementation of render: VretexBuffer and PrimitiveBlock build.
 *
 * $Id: patch_render.cpp,v 1.1 2001/07/06 12:26:49 berenguier Exp $
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


#include "3d/patch.h"
#include "3d/tessellation.h"
#include "3d/bezier_patch.h"
#include "3d/zone.h"
#include "3d/landscape.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"
using	namespace	std;
using	namespace	NLMISC;


namespace NL3D 
{


// ***************************************************************************
void			CPatch::preRender(const std::vector<CPlane>	&pyramid)
{
	// Don't do anything if clipped.
	if(Clipped)
		return;

	// 0. Classify the patch.
	//=======================
	sint	newFar0,newFar1;
	float	r= (CTessFace::RefineCenter-BSphere.Center).norm() - BSphere.Radius;
	float	rr;
	if(r<CTessFace::TileDistNear)
		rr= r-CTessFace::TileDistNear, newFar0= 0;
	else if(r<CTessFace::Far0Dist)
		rr= r-CTessFace::Far0Dist, newFar0= 1;
	else if(r<CTessFace::Far1Dist)
		rr= r-CTessFace::Far1Dist, newFar0= 2;
	else
		newFar0= 3;
	// Transition with the next level.
	newFar1=0;
	if(newFar0<3 && rr>-(CTessFace::FarTransition+2*BSphere.Radius))
	{
		newFar1= newFar0+1;
	}


	// 1. Update Texture Info.
	//========================
	if(newFar0!=Far0 || newFar1!=Far1)
	{
		// Backup old pass0
		CPatchRdrPass	*oldPass0=Pass0;
		CPatchRdrPass	*oldPass1=Pass1;
		float oldFar0UScale=Far0UScale;
		float oldFar0VScale=Far0VScale;
		float oldFar0UBias=Far0UBias;
		float oldFar0VBias=Far0VBias;
		uint8 oldFlags=Flags;

		// Don't delete the pass0 if the new newFar1 will use it
		if ((newFar1==Far0)&&(Far0>0))
			Pass0=NULL;

		// Don't delete the pass1 if the new newFar0 will use it
		if ((newFar0==Far1)&&(Far1>0))
			Pass1=NULL;

		// Pass0 have changed ?
		if (newFar0!=Far0)
		{
			// Compute / get the texture Far.
			if(newFar0>0)
			{
				// Free the old pass, don't used any more
				if (Pass0)
					Zone->Landscape->freeFarRenderPass (this, Pass0, Far0);

				// Can we use the old pass1 ?
				if (newFar0==Far1)
				{
					// Yes, recycle it!
					Pass0=oldPass1;

					// Copy uv coordinates
					Far0UScale=Far1UScale;
					Far0VScale=Far1VScale;
					Far0UBias=Far1UBias;
					Far0VBias=Far1VBias;

					// Copy rotation flag
					Flags&=~NL_PATCH_FAR0_ROTATED;			// erase it
					if (Flags&NL_PATCH_FAR1_ROTATED)
						Flags|=NL_PATCH_FAR0_ROTATED;			// copy it
				}
				else	// get a new render pass
				{
					// Rotation boolean
					bool bRot;
					Pass0=Zone->Landscape->getFarRenderPass(this, newFar0, Far0UScale, Far0VScale, Far0UBias, Far0VBias, bRot);

					// Flags is set if the far texture is rotated of 90° to the left
					if (bRot)
						Flags|=NL_PATCH_FAR0_ROTATED;
					else
						Flags&=~NL_PATCH_FAR0_ROTATED;
				}
			}
			else	// no more far pass0
			{
				if (Pass0)
				{
					Zone->Landscape->freeFarRenderPass (this, Pass0, Far0);
					Pass0=NULL;
				}
			}
		}

		// Pass1 have changed ?
		if (newFar1!=Far1)
		{
			// Now let's go with pass1
			if(newFar1>0)
			{
				// Delete the pass1 if not used any more
				if (Pass1)
					Zone->Landscape->freeFarRenderPass (this, Pass1, Far1);

				// Can we use the old pass1 ?
				if (newFar1==Far0)
				{
					// Yes, recycle it!
					Pass1=oldPass0;

					// Copy uv coordinates
					Far1UScale=oldFar0UScale;
					Far1VScale=oldFar0VScale;
					Far1UBias=oldFar0UBias;
					Far1VBias=oldFar0VBias;

					// Copy rotation flag
					Flags&=~NL_PATCH_FAR1_ROTATED;			// erase it
					if (oldFlags&NL_PATCH_FAR0_ROTATED)
						Flags|=NL_PATCH_FAR1_ROTATED;			// copy it
				}
				else	// get a new render pass
				{
					// Rotation boolean
					bool bRot;
					Pass1=Zone->Landscape->getFarRenderPass(this, newFar1, Far1UScale, Far1VScale, Far1UBias, Far1VBias, bRot);

					// Flags is set if the far texture is rotated of 90° to the left
					if (bRot)
						Flags|=NL_PATCH_FAR1_ROTATED;
					else
						Flags&=~NL_PATCH_FAR1_ROTATED;
				}

				// Compute info for transition.
				float	farDist;
				switch(newFar1)
				{
					case 1: farDist= CTessFace::TileDistNear; break;
					case 2: farDist= CTessFace::Far0Dist; break;
					case 3: farDist= CTessFace::Far1Dist; break;
					default: nlstop;
				};
				TransitionSqrMin= sqr(farDist-CTessFace::FarTransition);
				OOTransitionSqrDelta= 1.0f/(sqr(farDist)-TransitionSqrMin);
			}
			else	// no more far pass1
			{
				if (Pass1)
				{
					Zone->Landscape->freeFarRenderPass (this, Pass1, Far1);
					Pass1=NULL;
				}
			}
		}
	}
	// Set new far values
	Far0= newFar0;
	Far1= newFar1;

	// 2. Clip tess blocks.
	//=====================
	// MasterBlock never clipped.
	MasterBlock.resetClip();
	// If we are in Tile/FarTransition
	bool	doClipFar= Far0==0 && Far1==1;
	for(sint i=0; i<(sint)TessBlocks.size(); i++)
	{
		TessBlocks[i].resetClip();
		TessBlocks[i].clip(pyramid);
		// If we are in Tile/FarTransition
		if(doClipFar)
			TessBlocks[i].clipFar(CTessFace::RefineCenter, CTessFace::TileDistNear, CTessFace::FarTransition);
	}
	// \todo yoyo: TODO_OPTIMIZE: CTessBlockEdge gestion.



	// 3. Count Vertex Buffer usage, and Triangles usage.
	//======================================

	// FAR0.
	//=======
	if(Pass0)
	{
		// Inc far vertices count.
		CTessFace::MaxFar0Index+= MasterBlock.FarVertexList.size();
		// Inc tri count.
		Pass0->addMaxTris(MasterBlock.FarFaceList.size());

		// For all tessBlocks not clipped.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			if(!tblock.Clipped && !tblock.FullFar1)
			{
				// Inc far vertices count.
				CTessFace::MaxFar0Index+= tblock.FarVertexList.size();
				// Inc tri count.
				Pass0->addMaxTris(tblock.FarFaceList.size());
			}
		}
	}
	// \todo yoyo: TODO_OPTIMIZE: CTessBlockEdge gestion (add new vertices).

	// FAR1.
	//=======
	if(Pass1)
	{
		// Inc far vertices count.
		CTessFace::MaxFar1Index+= MasterBlock.FarVertexList.size();
		// Inc tri count.
		Pass1->addMaxTris(MasterBlock.FarFaceList.size());
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			if(!tblock.Clipped && !tblock.EmptyFar1)
			{
				// Inc far vertices count.
				CTessFace::MaxFar1Index+= tblock.FarVertexList.size();
				// Inc tri count.
				Pass1->addMaxTris(tblock.FarFaceList.size());
			}
		}
	}
	// \todo yoyo: TODO_OPTIMIZE: CTessBlockEdge gestion (add new vertices).


	// TILE.
	//=======
	if(Far0==0)
	{
		// No Tiles in MasterBlock!!

		// Traverse the TessBlocks to add vertices, and each TileMaterial, to addMaxTris to pass.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			if(!tblock.Clipped && !tblock.FullFar1)
			{
				// Inc near vertices count.
				CTessFace::MaxTileIndex+= tblock.NearVertexList.size();
				// Inc tri count.
				for(sint j=0; j<NL3D_TESSBLOCK_TILESIZE; j++)
				{
					CTileMaterial	*tileMat= tblock.RdrTileRoot[j];
					if(tileMat)
					{
						nlassert(NL3D_MAX_TILE_PASS==5);
						nlassert(NL3D_TILE_PASS_LIGHTMAP==4);
						// We must always have a RGB0 pass, and a lightmap pass.
						nlassert(tileMat->Pass[NL3D_TILE_PASS_RGB0]);
						nlassert(tileMat->Pass[NL3D_TILE_PASS_LIGHTMAP]);

						// Add the max faces for RGB0 and LIGHTMAP. NB: lightmap pass use RGB0 faces...
						// Because of sharing of vertices.
						tileMat->Pass[NL3D_TILE_PASS_RGB0]->addMaxTris(tileMat->TileFaceList[NL3D_TILE_PASS_RGB0].size());
						tileMat->Pass[NL3D_TILE_PASS_LIGHTMAP]->addMaxTris(tileMat->TileFaceList[NL3D_TILE_PASS_RGB0].size());

						// Add the optionnal faces for each Mat Pass.
						if(tileMat->Pass[NL3D_TILE_PASS_RGB1])
							tileMat->Pass[NL3D_TILE_PASS_RGB1]->addMaxTris(tileMat->TileFaceList[NL3D_TILE_PASS_RGB1].size());
						if(tileMat->Pass[NL3D_TILE_PASS_RGB2])
							tileMat->Pass[NL3D_TILE_PASS_RGB2]->addMaxTris(tileMat->TileFaceList[NL3D_TILE_PASS_RGB2].size());
						if(tileMat->Pass[NL3D_TILE_PASS_ADD])
							tileMat->Pass[NL3D_TILE_PASS_ADD]->addMaxTris(tileMat->TileFaceList[NL3D_TILE_PASS_ADD].size());
					}
				}
			}
		}
	}
}



// ***************************************************************************
void		CPatch::fillFar0VB(CTessList<CTessFarVertex>  &vertList)
{
	// Point to start of vertices we fill now.
	static	uint8	*CurVBPtr;
	CurVBPtr= (uint8*)CTessFace::CurrentFar0VBInfo.VertexCoordPointer;
	CurVBPtr+= CTessFace::CurrentFar0Index * CTessFace::CurrentFar0VBInfo.VertexSize;


	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		// Compute/build the new vertex.
		pVert->Index0= CTessFace::CurrentFar0Index++;


		// NB: the filling order of data is important, for AGP write combiners.

		// Set Pos.
		*(CVector*)CurVBPtr= pVert->Src->Pos;


		// compute Uvs.
		static CUV	uv;
		CParamCoord	&pc= pVert->PCoord;
		if (Flags&NL_PATCH_FAR0_ROTATED)
		{
			uv.U= pc.getT()* Far0UScale + Far0UBias;
			uv.V= (1.f-pc.getS())* Far0VScale + Far0VBias;
		}
		else
		{
			uv.U= pc.getS()* Far0UScale + Far0UBias;
			uv.V= pc.getT()* Far0VScale + Far0VBias;
		}
		// Set Uvs.
		*(CUV*)(CurVBPtr + CTessFace::CurrentFar0VBInfo.TexCoordOff0)= uv;


		// Inc the ptr.
		CurVBPtr+= CTessFace::CurrentFar0VBInfo.VertexSize;
	}
}


// ***************************************************************************
void		CPatch::fillFar1VB(CTessList<CTessFarVertex>  &vertList)
{
	// Point to start of vertices we fill now.
	static	uint8	*CurVBPtr;
	CurVBPtr= (uint8*)CTessFace::CurrentFar1VBInfo.VertexCoordPointer;
	CurVBPtr+= CTessFace::CurrentFar1Index * CTessFace::CurrentFar1VBInfo.VertexSize;


	// Traverse the vertList.
	CTessFarVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessFarVertex*)pVert->Next)
	{
		// Compute/build the new vertex.
		pVert->Index1= CTessFace::CurrentFar1Index++;


		// NB: the filling order of data is important, for AGP write combiners.

		// Set Pos.
		*(CVector*)CurVBPtr= pVert->Src->Pos;


		// compute Uvs.
		static CUV		uv;
		CParamCoord	&pc= pVert->PCoord;
		if (Flags&NL_PATCH_FAR1_ROTATED)
		{
			uv.U= pc.getT()* Far1UScale + Far1UBias;
			uv.V= (1.f-pc.getS())* Far1VScale + Far1VBias;
		}
		else
		{
			uv.U= pc.getS()* Far1UScale + Far1UBias;
			uv.V= pc.getT()* Far1VScale + Far1VBias;
		}
		// Set Uvs.
		*(CUV*)(CurVBPtr + CTessFace::CurrentFar1VBInfo.TexCoordOff0)= uv;


		// Compute color.
		static CRGBA	col(255,255,255,255);
		// For Far1, use alpha fro transition.
		float	f= (pVert->Src->Pos - CTessFace::RefineCenter).sqrnorm();
		f= (f-TransitionSqrMin) * OOTransitionSqrDelta;
		clamp(f,0,1);
		col.A= (uint8)(f*255);
		// Set color.
		*(CRGBA*)(CurVBPtr + CTessFace::CurrentFar1VBInfo.ColorOff)= col;


		// Inc the ptr.
		CurVBPtr+= CTessFace::CurrentFar1VBInfo.VertexSize;
	}
}


// ***************************************************************************
void		CPatch::fillTileVB(CTessList<CTessNearVertex>  &vertList)
{
	// Point to start of vertices we fill now.
	static	uint8	*CurVBPtr;
	CurVBPtr= (uint8*)CTessFace::CurrentTileVBInfo.VertexCoordPointer;
	CurVBPtr+= CTessFace::CurrentTileIndex * CTessFace::CurrentTileVBInfo.VertexSize;


	// Traverse the vertList.
	CTessNearVertex	*pVert;
	for(pVert= vertList.begin(); pVert; pVert= (CTessNearVertex*)pVert->Next)
	{
		// Compute/build the new vertex.
		pVert->Index= CTessFace::CurrentTileIndex++;


		// NB: the filling order of data is important, for AGP write combiners.

		// Set Pos.
		*(CVector*)CurVBPtr= pVert->Src->Pos;

		// Set Uvs.
		*(CUV*)(CurVBPtr + CTessFace::CurrentTileVBInfo.TexCoordOff0)= pVert->PUv0;
		*(CUV*)(CurVBPtr + CTessFace::CurrentTileVBInfo.TexCoordOff1)= pVert->PUv1;

		// Inc the ptr.
		CurVBPtr+= CTessFace::CurrentTileVBInfo.VertexSize;
	}
}



// ***************************************************************************
void			CPatch::fillTileVertexBuffer()
{
	// If tile mode and not clipped.
	if(Far0==0 && !Clipped)
	{
		// Fill VBuffer.
		//=======
		// No Tiles in MasterBlock!!

		// Traverse the TessBlocks to add vertices.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			if(!tblock.Clipped && !tblock.FullFar1)
			{
				// Add the vertices.
				fillTileVB(tblock.NearVertexList);
			}
		}
	}

}


// ***************************************************************************
void			CPatch::addFar0TriList(CPatchRdrPass *pass, CTessList<CTessFace> &flist)
{
	CTessFace	*pFace;
	for(pFace= flist.begin(); pFace; pFace= (CTessFace*)pFace->Next)
	{
		pass->addTri(pFace->FVBase->Index0, pFace->FVLeft->Index0, pFace->FVRight->Index0);
	}
}


// ***************************************************************************
void			CPatch::addFar1TriList(CPatchRdrPass *pass, CTessList<CTessFace> &flist)
{
	CTessFace	*pFace;
	for(pFace= flist.begin(); pFace; pFace= (CTessFace*)pFace->Next)
	{
		pass->addTri(pFace->FVBase->Index1, pFace->FVLeft->Index1, pFace->FVRight->Index1);
	}
}


// ***************************************************************************
void			CPatch::addTileTriList(CPatchRdrPass *pass, CTessList<CTileFace> &flist)
{
	CTileFace	*pFace;
	for(pFace= flist.begin(); pFace; pFace= (CTileFace*)pFace->Next)
	{
		pass->addTri(pFace->VBase->Index, pFace->VLeft->Index, pFace->VRight->Index);
	}
}


// ***************************************************************************
void			CPatch::renderFar0()
{
	if(Pass0 && !Clipped)
	{

		// Fill VBuffer.
		//=======
		{
			// Fill VB.
			fillFar0VB(MasterBlock.FarVertexList);
			for(sint i=0; i<(sint)TessBlocks.size(); i++)
			{
				CTessBlock	&tblock= TessBlocks[i];
				if(!tblock.Clipped && !tblock.FullFar1)
				{
					fillFar0VB(tblock.FarVertexList);
				}
			}
		}
		// \todo yoyo: TODO_OPTIMIZE: CTessBlockEdge gestion (add new vertices).


		// Fill PBlock.
		//=======
		// Add tris of MasterBlock.
		addFar0TriList(Pass0, MasterBlock.FarFaceList);
		// Add tris of TessBlocks.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			if(!tblock.Clipped && !tblock.FullFar1)
				addFar0TriList(Pass0, tblock.FarFaceList);
		}

		// Check the pass is in the set
#ifdef NL_DEBUG
		if (Pass0)
		{
			nlassert (Zone->Landscape->_FarRdrPassSet.find (Pass0)!=Zone->Landscape->_FarRdrPassSet.end());
			if (Zone->Landscape->_FarRdrPassSet.find (Pass0)==Zone->Landscape->_FarRdrPassSet.end())
			{
				bool bFound=false;
				{
					for (sint t=0; t<(sint)Zone->Landscape->_FarRdrPassSetVectorFree.size(); t++)
					{
						if (Zone->Landscape->_FarRdrPassSetVectorFree[t].find (Pass0)!=Zone->Landscape->_FarRdrPassSetVectorFree[t].end())
						{
							bFound=true;
							break;
						}
					}
				}
				nlassert (bFound);
			}
		}
#endif // NL_DEBUG
	}
}


// ***************************************************************************
void			CPatch::renderFar1()
{
	if(Pass1 && !Clipped)
	{

		// Fill VBuffer.
		//=======
		if(Pass1)
		{
			// Fill VB.
			fillFar1VB(MasterBlock.FarVertexList);
			for(sint i=0; i<(sint)TessBlocks.size(); i++)
			{
				CTessBlock	&tblock= TessBlocks[i];
				if(!tblock.Clipped && !tblock.EmptyFar1)
				{
					fillFar1VB(tblock.FarVertexList);
				}
			}
		}
		// \todo yoyo: TODO_OPTIMIZE: CTessBlockEdge gestion (add new vertices).


		// Fill PBlock.
		//=======
		// Add tris of MasterBlock.
		addFar1TriList(Pass1, MasterBlock.FarFaceList);
		// Add tris of TessBlocks.
		for(sint i=0; i<(sint)TessBlocks.size(); i++)
		{
			CTessBlock	&tblock= TessBlocks[i];
			if(!tblock.Clipped && !tblock.EmptyFar1)
				addFar1TriList(Pass1, tblock.FarFaceList);
		}

		// Check the pass is in the set
#ifdef NL_DEBUG
		if (Pass1)
		{
			nlassert (Zone->Landscape->_FarRdrPassSet.find (Pass1)!=Zone->Landscape->_FarRdrPassSet.end());
			if (Zone->Landscape->_FarRdrPassSet.find (Pass1)==Zone->Landscape->_FarRdrPassSet.end())
			{
				bool bFound=false;
				{
					for (sint t=0; t<(sint)Zone->Landscape->_FarRdrPassSetVectorFree.size(); t++)
					{
						if (Zone->Landscape->_FarRdrPassSetVectorFree[t].find (Pass1)!=Zone->Landscape->_FarRdrPassSetVectorFree[t].end())
						{
							bFound=true;
							break;
						}
					}
				}
				nlassert (bFound);
			}
		}
#endif // NL_DEBUG
	}
}
// ***************************************************************************
void			CPatch::renderTile(sint pass)
{
	// If tile mode.
	if(Far0==0 && !Clipped)
	{
		// NB: VB is previously filled in fillVertexBuffer().

		// Fill PBlock.
		//=======
		// LIGHTMAP should not be rendered directly.
		nlassert(pass==NL3D_TILE_PASS_RGB0 || pass==NL3D_TILE_PASS_RGB1 || pass==NL3D_TILE_PASS_RGB2 || pass==NL3D_TILE_PASS_ADD);

		// No Tiles in MasterBlock!!!!

		// General Case: RGB0 + lightmap.
		if(pass==NL3D_TILE_PASS_RGB0)
		{
			// Traverse the TessBlocks to add faces to each TileMaterial.
			for(sint i=0; i<(sint)TessBlocks.size(); i++)
			{
				CTessBlock	&tblock= TessBlocks[i];
				if(!tblock.Clipped && !tblock.FullFar1)
				{
					// Add the faces.
					for(sint j=0; j<NL3D_TESSBLOCK_TILESIZE; j++)
					{
						CTileMaterial	*tileMat= tblock.RdrTileRoot[j];
						if(tileMat)
						{
							nlassert(tileMat->Pass[NL3D_TILE_PASS_RGB0]);
							nlassert(tileMat->Pass[NL3D_TILE_PASS_LIGHTMAP]);
							// Add the trilist of RGB0 both to RGB0 and LIGHTMAP.
							addTileTriList(tileMat->Pass[NL3D_TILE_PASS_RGB0], tileMat->TileFaceList[NL3D_TILE_PASS_RGB0]);
							addTileTriList(tileMat->Pass[NL3D_TILE_PASS_LIGHTMAP], tileMat->TileFaceList[NL3D_TILE_PASS_RGB0]);
						}
					}
				}
			}
		}
		// Else: RGB1, RGB2, ADD.
		else
		{
			// Traverse the TessBlocks to add faces to each TileMaterial.
			for(sint i=0; i<(sint)TessBlocks.size(); i++)
			{
				CTessBlock	&tblock= TessBlocks[i];
				if(!tblock.Clipped && !tblock.FullFar1)
				{
					// Add the faces.
					for(sint j=0; j<NL3D_TESSBLOCK_TILESIZE; j++)
					{
						CTileMaterial	*tileMat= tblock.RdrTileRoot[j];
						if(tileMat)
						{
							if(tileMat->Pass[pass])
								addTileTriList(tileMat->Pass[pass], tileMat->TileFaceList[pass]);
						}
					}
				}
			}
		}
	}
}



} // NL3D
