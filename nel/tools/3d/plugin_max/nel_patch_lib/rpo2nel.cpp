/** \file rpo2nel.cpp
 * <File description>
 *
 * $Id: rpo2nel.cpp,v 1.8 2001/10/08 15:02:51 corvazier Exp $
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

#include "stdafx.h"
#include "rpo.h"
#include "3d/zone.h"

// For MAX_RELEASE
#include <plugapi.h>

using namespace NL3D;
using namespace NLMISC;

static int getCommonEdge(PatchMesh* pPM, int edge, Patch& patch2)
{
	for(int e=0 ; e<4 ; e++)
	{
		if (patch2.edge[e]==edge)
			return(e);
	}
	nlassert (0);		// no!
	return(-1);
}

static int getCommonVertex(PatchMesh* pPM, int ipatch1, int ipatch2, int* pordervtx=NULL)
{
	Patch*	patch1;
	Patch*	patch2;

	patch1=&pPM->patches[ipatch1];
	patch2=&pPM->patches[ipatch2];

	for(int i=0 ; i<4 ; i++)
	{
		if (patch1->v[i]==patch2->v[0])			
		{
			break;
		}
		if (patch1->v[i]==patch2->v[1])			
		{
			break;
		}
		if (patch1->v[i]==patch2->v[2])			
		{
			break;
		}
		if (patch1->v[i]==patch2->v[3])			
		{
			break;
		}
	}
	if (i==4)
	{
		return(-1);
	}
	if (pordervtx)
	{
		*pordervtx=i;
	}
	return(patch1->v[i]);
}

static int getOtherBindedVertex(RPatchMesh*	pRPM, PatchMesh* pPM, int ipatch1, int ipatch2, int iOtherVertex)
{
	Patch*		patch1;
	Patch*		patch2;

	patch1=&pPM->patches[ipatch1];
	patch2=&pPM->patches[ipatch2];

	for(int i=0 ; i<4 ; i++)
	{
		UI_VERTEX uiv=pRPM->getUIVertex (patch1->v[i]);
		if (uiv.Binding.bBinded)
		{
			if ((int)uiv.Binding.nPatch==ipatch2 && i!=iOtherVertex)
			{
				return(patch1->v[i]);
			}
		}
	}
	return(-1);
}

static int getEdge(PatchMesh* pPM, Patch* patch, int iv1, int iv2)
{
	for(int i=0 ; i<4 ; i++)
	{
		PatchEdge edge=pPM->edges[patch->edge[i]];
		if (edge.v1==iv1 && edge.v2==iv2)
		{
			return(i);
		}
		if (edge.v2==iv1 && edge.v1==iv2)
		{
			return(i);
		}

	}
	return(-1);
}

bool RPatchMesh::exportZone(INode* pNode, PatchMesh* pPM, NL3D::CZone& zone, int zoneId)
{
	Matrix3					TM;
	CPatchInfo				pi;
	std::vector<CPatchInfo>	patchinfo;
	sint					i,j;
	Point3					v;
	Patch*					pPatch;
	
	TM=pNode->GetObjectTM(0);

	// ---
	// --- Basic exports
	// ---
	for(i=0 ; i<pPM->numPatches ; i++)
	{
		pPatch=&pPM->patches[i];
		// - Vertices
		for(j=0 ; j<4 ; j++)
		{
			v=pPM->verts[pPatch->v[j]].p;
			v=v*TM;
			pi.Patch.Vertices[j].x=v.x;
			pi.Patch.Vertices[j].y=v.y;
			pi.Patch.Vertices[j].z=v.z;
		}
		// - Tangents
		for(j=0 ; j<8 ; j++)
		{
			v=pPM->vecs[pPatch->vec[j]].p;
			v=v*TM;
			pi.Patch.Tangents[j].x=v.x;
			pi.Patch.Tangents[j].y=v.y;
			pi.Patch.Tangents[j].z=v.z;
		}
		// - Interiors
		for(j=0 ; j<4 ; j++)
		{
			v=pPM->vecs[pPatch->interior[j]].p;
			v=v*TM;
			pi.Patch.Interiors[j].x=v.x;
			pi.Patch.Interiors[j].y=v.y;
			pi.Patch.Interiors[j].z=v.z;
		}
		pi.OrderS=1<<getUIPatch (i).NbTilesU;
		pi.OrderT=1<<getUIPatch (i).NbTilesV;
		pi.BindEdges[0].ZoneId=zoneId;
		pi.BindEdges[1].ZoneId=zoneId;
		pi.BindEdges[2].ZoneId=zoneId;
		pi.BindEdges[3].ZoneId=zoneId;
		pi.BaseVertices[0]=pPatch->v[0];
		pi.BaseVertices[1]=pPatch->v[1];
		pi.BaseVertices[2]=pPatch->v[2];
		pi.BaseVertices[3]=pPatch->v[3];
		pi.Tiles.resize(pi.OrderS*pi.OrderT);

		// Tile infos
		int u,v;
		for (v=0; v<pi.OrderT; v++)
		for (u=0; u<pi.OrderS; u++)
		{
			tileDesc &desc=getUIPatch (i).getTileDesc (u+v*pi.OrderS);
			for (int l=0; l<3; l++)
			{
				if (l>=desc.getNumLayer ())
				{
					pi.Tiles[u+v*pi.OrderS].Tile[l]=0xffff;
				}
				else
				{
					pi.Tiles[u+v*pi.OrderS].Tile[l]=desc.getLayer (l).Tile;
					pi.Tiles[u+v*pi.OrderS].setTileOrient (l, desc.getLayer (l).Rotate);
				}
			}
			if (pi.Tiles[u+v*pi.OrderS].Tile[0]==0xffff)
				pi.Tiles[u+v*pi.OrderS].setTile256Info (false, 0);
			else
			{
				if (desc.getCase()==0)
					pi.Tiles[u+v*pi.OrderS].setTile256Info (false, 0);
				else
					pi.Tiles[u+v*pi.OrderS].setTile256Info (true, desc.getCase()-1);
			}
			pi.Tiles[u+v*pi.OrderS].setTileSubNoise (desc.getDisplace());
		}

		// ** Export tile colors

		// Resize color table
		pi.TileColors.resize ((pi.OrderS+1)*(pi.OrderT+1));

		// Export it
		for (v=0; v<pi.OrderT+1; v++)
		for (u=0; u<pi.OrderS+1; u++)
		{
			// Get rgb value at this vertex
			uint color=getUIPatch (i).getColor (u+v*(pi.OrderS+1));

			// Create a rgba value
			CRGBA rgba ( (color&0xff0000)>>16, (color&0x00ff00)>>8, color&0xff );

			// Store it in the tile info
			pi.TileColors[u+v*(pi.OrderS+1)].Color565=rgba.get565();
		}

		// ** Export tile shading

		pi.Lumels.resize ((pi.OrderS*4)*(pi.OrderT*4), 255);

		// ---
		// --- Smooth flags
		// ---

		// Clear smooth flags
		pi.Flags&=~0xf;

		for (int edge=0; edge<4; edge++)
		{
			// Edge smooth ?
			if (!getUIPatch (i).getEdgeFlag (edge))
			{
				// Don't smooth
				pi.Flags|=(1<<edge);
			}
		}

		// Add this tile info
		patchinfo.push_back(pi);

	}
	// ---
	// --- Pass 1 :
	// --- Parse each patch and then each vertex.
	// ---
	int isrcpatch;
	Patch* srcpatch;
	for(isrcpatch=0 ; isrcpatch<pPM->numPatches ; isrcpatch++)
	{
		srcpatch=&pPM->patches[isrcpatch];

		for(int nv=0 ; nv<4 ; nv++)
		{
			UI_VERTEX uiv=getUIVertex (srcpatch->v[nv]);

			if (uiv.Binding.bBinded)
			{				
				int isrcedge;
				int n;
				int	icv;
				int idstpatch=uiv.Binding.nPatch;
				int idstedge=uiv.Binding.nEdge;
				int	orderdstvtx;

				n=-1;
				// -
				if (uiv.Binding.nType==BIND_SINGLE)
				{
					icv=getCommonVertex(pPM,idstpatch,isrcpatch,&orderdstvtx);			
					if (icv==-1)
					{
						return false;
					}
					if (idstedge==orderdstvtx) 
					{
						n=0;
					}
					else
					{
						n=1;
					}
				}
				// -
				if (uiv.Binding.nType==BIND_25)
				{
					n=1;
					icv=getOtherBindedVertex(this, pPM, isrcpatch,idstpatch,nv);
					if (icv==-1)
					{
						n=0;
						icv=getCommonVertex(pPM,idstpatch,isrcpatch);			
						if (icv==-1)
						{
							return false;
						}
					}
				}
				// -
				if (uiv.Binding.nType==BIND_75)
				{
					n=2;
					icv=getOtherBindedVertex(this, pPM, isrcpatch,idstpatch,nv);
					if (icv==-1)
					{
						n=3;
						icv=getCommonVertex(pPM,idstpatch,isrcpatch);			
						if (icv==-1)
						{
							return false;
						}
					}
				}
				// -
				if (n!=-1)
				{
					isrcedge=getEdge(pPM,srcpatch,srcpatch->v[nv],icv);
					if (isrcedge==-1)
					{
							return false;
					}
					// let's fill the dst patch (n is important here... it's the order)
					patchinfo[idstpatch].BindEdges[idstedge].NPatchs++;
					patchinfo[idstpatch].BindEdges[idstedge].Edge[n]=isrcedge;
					patchinfo[idstpatch].BindEdges[idstedge].Next[n]=isrcpatch;

					// let's fill the src patch also...
					patchinfo[isrcpatch].BindEdges[isrcedge].NPatchs=5;
					patchinfo[isrcpatch].BindEdges[isrcedge].Edge[0]=idstedge;
					patchinfo[isrcpatch].BindEdges[isrcedge].Next[0]=idstpatch;
				}
			}
		}
	}
	// ---
	// --- Pass 2 :
	// --- Get all one/one cases.
	// --- Parse each patch and each edge
	// ---
	for(i=0 ; i<pPM->numPatches ; i++)
	{
		pPatch=&pPM->patches[i];
		for(int e=0 ; e<4 ; e++)
		{
			PatchEdge edge=pPM->edges[pPatch->edge[e]];

			// One/One binding
#if (MAX_RELEASE < 4000)
			if (edge.patch2>=0)
			{		
				patchinfo[i].BindEdges[e].NPatchs=1;
				// 'coz i don't know wether edge.patch1 or edge.patch2 is
				// the patch that i am parsing
				if (edge.patch2!=i)
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patch2;
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patch2]);
				}
				else
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patch1;
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patch1]);
				}				
#else // (MAX_RELEASE < 4000)
			if (edge.patches[1]>=0)
			{		
				patchinfo[i].BindEdges[e].NPatchs=1;
				// 'coz i don't know wether edge.patch1 or edge.patch2 is
				// the patch that i am parsing
				if (edge.patches[1]!=i)
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patches[1];
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patches[1]]);
				}
				else
				{
					patchinfo[i].BindEdges[e].Next[0]=edge.patches[0];
					patchinfo[i].BindEdges[e].Edge[0]=getCommonEdge(pPM, pPatch->edge[e], pPM->patches[edge.patches[0]]);
				}				
#endif // (MAX_RELEASE < 4000)
			}
		}
	}

	zone.build(zoneId, patchinfo, std::vector<CBorderVertex>());
	return true;
}
