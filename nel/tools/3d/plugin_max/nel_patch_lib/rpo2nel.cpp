/** \file rpo2nel.cpp
 * <File description>
 *
 * $Id: rpo2nel.cpp,v 1.14 2002/03/07 10:33:12 corvazier Exp $
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

// For mprintf
#include <maxscrpt.h>

using namespace std;
using namespace NL3D;
using namespace NLMISC;

// ***************************************************************************

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

// ***************************************************************************

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

// ***************************************************************************

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

// ***************************************************************************

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

// ***************************************************************************

int getScriptAppData (Animatable *node, uint32 id, int def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	int value;
	if (sscanf ((const char*)ap->data, "%d", &value)==1)
		return value;
	else
		return def;
}

// ***************************************************************************

bool RPatchMesh::getTileSymmetryRotate (const CTileBank &bank, uint tile, bool &symmetry, uint &rotate)
{
	// Tile exist ?
	if (tile < (uint)bank.getTileCount())
	{
		// Get xref
		int tileSet;
		int number;
		CTileBank::TTileType type;

		// Get tile xref
		bank.getTileXRef ((int)tile, tileSet, number, type);

		// Is it an oriented tile ?
		if (bank.getTileSet (tileSet)->getOriented())
		{
			// New rotation value
			rotate = 0;
		}

		// Ok
		return true;
	}

	return false;
}

// ***************************************************************************

bool RPatchMesh::transformTile (const CTileBank &bank, uint &tile, uint &tileRotation, bool symmetry, uint rotate)
{
	// Tile exist ?
	if ( (rotate!=0) || symmetry )
	{
		if (tile < (uint)bank.getTileCount())
		{
			// Get xref
			int tileSet;
			int number;
			CTileBank::TTileType type;

			// Get tile xref
			bank.getTileXRef ((int)tile, tileSet, number, type);

			// Transition ?
			if (type == CTileBank::transition)
			{
				// Number should be ok
				nlassert (number>=0);
				nlassert (number<CTileSet::count);

				// Tlie set number
				const CTileSet *pTileSet = bank.getTileSet (tileSet);

				// Get border desc
				CTileSet::TFlagBorder oriented[4] = 
				{	
					pTileSet->getOrientedBorder (CTileSet::left, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::left)),
					pTileSet->getOrientedBorder (CTileSet::bottom, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::bottom)),
					pTileSet->getOrientedBorder (CTileSet::right, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::right)),
					pTileSet->getOrientedBorder (CTileSet::top, CTileSet::getEdgeType ((CTileSet::TTransition)number, CTileSet::top))
				};

				// Symmetry ?
				if (symmetry)
				{
					CTileSet::TFlagBorder tmp = oriented[0];
					oriented[0] = CTileSet::getInvertBorder (oriented[2]);
					oriented[2] = CTileSet::getInvertBorder (tmp);
					oriented[1] = CTileSet::getInvertBorder (oriented[1]);
					oriented[3] = CTileSet::getInvertBorder (oriented[3]);
				}

				// Rotation
				CTileSet::TFlagBorder edges[4];
				edges[0] = pTileSet->getOrientedBorder (CTileSet::left, oriented[(0 + rotate)&3]);
				edges[1] = pTileSet->getOrientedBorder (CTileSet::bottom, oriented[(1 + rotate)&3]);
				edges[2] = pTileSet->getOrientedBorder (CTileSet::right, oriented[(2 + rotate)&3]);
				edges[3] = pTileSet->getOrientedBorder (CTileSet::top, oriented[(3 + rotate)&3]);

				// Get the good tile number
				CTileSet::TTransition transition = pTileSet->getTransitionTile (edges[3], edges[1], edges[0], edges[2]);
				nlassert ((CTileSet::TTransition)transition != CTileSet::notfound);
				tile = (uint)(pTileSet->getTransition (transition)->getTile ());
			}

			// Transform rotation
			tileRotation += rotate;
			tileRotation &= 3;
		}
		else
			return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

void RPatchMesh::transform256Case (const CTileBank &bank, uint &case256, uint tileRotation, bool symmetry, uint rotate)
{
	// Tile exist ?
	if ( (rotate!=0) || symmetry )
	{
		// Remove its rotation
		case256 += tileRotation;
		case256 &= 3;

		// Symmetry ?
		if (symmetry)
		{
			// Take the symmetry
			uint symArray[4] = {3, 2, 1, 0};
			case256 = symArray[case256];
		}

		// Rotation ?
		case256 -= rotate + tileRotation;
		case256 &= 3;
	}
}

// ***************************************************************************

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
	// --- Get the rotation value and symmetry flags
	// ---
	bool symmetry = getScriptAppData (pNode, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;
	int rotate = getScriptAppData (pNode, NEL3D_APPDATA_ZONE_ROTATE, 0);

	// Need a tile bank ?
	if (symmetry || rotate)
	{
		// Bank loaded
		bool loaded = false;

		// Get the bank name
		std::string sName=GetBankPathName ();
		if (sName!="")
		{
			// Open the bank
			CIFile file;
			if (file.open (sName))
			{
				try
				{
					// Read it
					bank.clear();
					bank.serial (file);
					bank.computeXRef ();

					// Ok
					loaded = true;
				}
				catch (EStream& stream)
				{
					MessageBox (NULL, stream.what(), "Error", MB_OK|MB_ICONEXCLAMATION);
				}
			}
		}

		// Not loaded ?
		if (loaded == false)
		{
			nlwarning ("Can't load any tile bank. Select on with the tile_utility plug-in");
			return false;
		}
	}

	// ---
	// --- Basic checks
	// ---

	// Map edge count
	map<pair<uint, uint>, uint >	edgeSet;

	// Triple edge Patch error
	set<uint>	patchError;

	// For each patches
	for (uint patch=0; patch<(uint)pPM->numPatches; patch++)
	{
		// For each edges
		for (uint edge=0; edge<4; edge++)
		{
			// Two vertices
			uint v1 = pPM->edges[pPM->patches[patch].edge[edge]].v1;
			uint v2 = pPM->edges[pPM->patches[patch].edge[edge]].v2;

			// Insert in the map
			map<pair<uint, uint>, uint >::iterator	ite;
			ite = edgeSet.find (pair<uint, uint>(min(v1, v2), max(v1, v2)));
			
			// Inserted ?
			if (ite == edgeSet.end())
				ite = edgeSet.insert (pair<pair<uint, uint>, uint>(pair<uint, uint>(min(v1, v2), max(v1, v2)), 1)).first;
			else
			{
				// Add a ref
				ite->second++;

				// Patch error ?
				if (ite->second>=3)
				{
					// Add a patch error
					patchError.insert (patch);
				}
			}
		}
	}

	// Some errors ?
	if (!patchError.empty())
	{
		// Make an error message
		char error[2098];
		smprintf (error, 2098, "Error: triple edge detected in ");

		// For each error
		set<uint>::iterator ite=patchError.begin();
		while (ite!=patchError.end())
		{
			// Sub error message
			char subError[512];
			smprintf (subError, 512, "patch %d ", (*ite)+1);
			strcat (error, subError);

			// Next error
			ite++;
		}

		// Show the message
		mprintf (error);

		// Error
		return false;
	}

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

		// Symmetry ?
		if (symmetry)
		{
			// Vertices
			CVector tmp = pi.Patch.Vertices[0];
			pi.Patch.Vertices[0] = pi.Patch.Vertices[3];
			pi.Patch.Vertices[3] = tmp;
			tmp = pi.Patch.Vertices[1];
			pi.Patch.Vertices[1] = pi.Patch.Vertices[2];
			pi.Patch.Vertices[2] = tmp;

			// Tangents
			tmp = pi.Patch.Tangents[0];
			pi.Patch.Tangents[0] = pi.Patch.Tangents[5];
			pi.Patch.Tangents[5] = tmp;
			tmp = pi.Patch.Tangents[1];
			pi.Patch.Tangents[1] = pi.Patch.Tangents[4];
			pi.Patch.Tangents[4] = tmp;
			tmp = pi.Patch.Tangents[2];
			pi.Patch.Tangents[2] = pi.Patch.Tangents[3];
			pi.Patch.Tangents[3] = tmp;
			tmp = pi.Patch.Tangents[6];
			pi.Patch.Tangents[6] = pi.Patch.Tangents[7];
			pi.Patch.Tangents[7] = tmp;

			// Interior
			tmp = pi.Patch.Interiors[0];
			pi.Patch.Interiors[0] = pi.Patch.Interiors[3];
			pi.Patch.Interiors[3] = tmp;
			tmp = pi.Patch.Interiors[1];
			pi.Patch.Interiors[1] = pi.Patch.Interiors[2];
			pi.Patch.Interiors[2] = tmp;
		}

		// Tile infos
		int u,v;
		for (v=0; v<pi.OrderT; v++)
		for (u=0; u<pi.OrderS; u++)
		{
			// U tile
			int uSymmetry = symmetry ? (pi.OrderS-u-1) : u;

			tileDesc &desc=getUIPatch (i).getTileDesc (u+v*pi.OrderS);
			for (int l=0; l<3; l++)
			{
				if (l>=desc.getNumLayer ())
				{
					pi.Tiles[uSymmetry+v*pi.OrderS].Tile[l]=0xffff;
				}
				else
				{
					// Get the tile index
					uint tile = desc.getLayer (l).Tile;
					uint tileRotation = desc.getLayer (l).Rotate;

					// Get rot and symmetry for this tile
					uint tileRotate = rotate;
					bool tileSymmetry = symmetry;

					// Transform the transfo
					if (getTileSymmetryRotate (bank, tile, tileSymmetry, tileRotate))
					{
						// Transform the tile
						if (!transformTile (bank, tile, tileRotation, tileSymmetry, (4-tileRotate)&3))
						{
							// Info
							nlwarning ("Error getting symmetrical / rotated zone tile.");
							return false;
						}
					}
					else
					{
						// Info
						nlwarning ("Error getting symmetrical / rotated zone tile.");
						return false;
					}

					// Set the tile
					pi.Tiles[uSymmetry+v*pi.OrderS].Tile[l] = tile;
					pi.Tiles[uSymmetry+v*pi.OrderS].setTileOrient (l, (uint8)tileRotation);
				}
			}
			if (pi.Tiles[uSymmetry+v*pi.OrderS].Tile[0]==0xffff)
				pi.Tiles[uSymmetry+v*pi.OrderS].setTile256Info (false, 0);
			else
			{
				if (desc.getCase()==0)
					pi.Tiles[uSymmetry+v*pi.OrderS].setTile256Info (false, 0);
				else
				{
					// Transform 256 case
					uint case256 = desc.getCase()-1;

					// Get rot and symmetry for this tile
					uint tileRotate = rotate;
					bool tileSymmetry = symmetry;

					// Transform the transfo
					getTileSymmetryRotate (bank, pi.Tiles[uSymmetry+v*pi.OrderS].Tile[0], tileSymmetry, tileRotate);

					// Transform the case
					transform256Case (bank, case256, 0, tileSymmetry, (4-tileRotate)&3);

					pi.Tiles[uSymmetry+v*pi.OrderS].setTile256Info (true, case256);
				}
			}
			pi.Tiles[uSymmetry+v*pi.OrderS].setTileSubNoise (desc.getDisplace());

			// Default VegetableState: AboveWater. Important: must not be VegetableDisabled
			pi.Tiles[uSymmetry+v*pi.OrderS].setVegetableState (CTileElement::AboveWater);
		}

		// ** Export tile colors

		// Resize color table
		pi.TileColors.resize ((pi.OrderS+1)*(pi.OrderT+1));

		// Export it
		for (v=0; v<pi.OrderT+1; v++)
		for (u=0; u<pi.OrderS+1; u++)
		{
			// U tile
			int uSymmetry = symmetry ? (pi.OrderS-u) : u;

			// Get rgb value at this vertex
			uint color=getUIPatch (i).getColor (u+v*(pi.OrderS+1));

			// Create a rgba value
			CRGBA rgba ( (color&0xff0000)>>16, (color&0x00ff00)>>8, color&0xff );

			// Store it in the tile info
			pi.TileColors[uSymmetry+v*(pi.OrderS+1)].Color565=rgba.get565();
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
			}
#endif // (MAX_RELEASE < 4000)
		}
	}

	// ---
	// --- Pass 3 :
	// --- Symmetry of the bind info.
	// --- Parse each patch and each edge
	// ---
	if (symmetry)
	{
		// For each patches
		for (i=0 ; i<pPM->numPatches ; i++)
		{
			// Ref on the patch info
			CPatchInfo &patchInfo = patchinfo[i];

			// Xchg left and right
			swap (patchInfo.BindEdges[0], patchInfo.BindEdges[2]);
			swap (patchInfo.BaseVertices[0], patchInfo.BaseVertices[3]);
			swap (patchInfo.BaseVertices[1], patchInfo.BaseVertices[2]);

			// Flip edges
			for (uint edge=0; edge<4; edge++)
			{
				// Ref on the patch info
				CPatchInfo::CBindInfo &bindEdge = patchinfo[i].BindEdges[edge];

				uint next;
				// Look if it is a bind ?
				if ( (bindEdge.NPatchs>1) && (bindEdge.NPatchs!=5) )
				{
					for (next=0; next<(uint)bindEdge.NPatchs/2; next++)
					{
						swap (bindEdge.Next[bindEdge.NPatchs - next - 1], bindEdge.Next[next]);
						swap (bindEdge.Edge[bindEdge.NPatchs - next - 1], bindEdge.Edge[next]);
					}
				}

				// Look if we are binded on a reversed edge
				uint bindCount = (bindEdge.NPatchs==5) ? 1 : bindEdge.NPatchs;
				for (next=0; next<bindCount; next++)
				{
					// Left or right ?
					if ( (bindEdge.Edge[next] & 1) == 0)
					{
						// Invert
						bindEdge.Edge[next] += 2;
						bindEdge.Edge[next] &= 3;
					}
				}
			}
		}
	}

	zone.build(zoneId, patchinfo, std::vector<CBorderVertex>());
	return true;
}

// ***************************************************************************

void RPatchMesh::importZone (PatchMesh* pPM, NL3D::CZone& zone, int &zoneId)
{
	// Patch info
	std::vector<CPatchInfo> patchs;
	std::vector<CBorderVertex> borderVertices;

	// Retrieve the geometry
	zone.retrieve (patchs, borderVertices);

	// Get the zone id
	zoneId = zone.getZoneId ();

	// Vertex number
	int vertexNum = 0;

	// Vertex map
	map<pair<uint, uint>, uint> mapEdgeVertex;

	// Number of vertices
	pPM->setNumVerts (4*patchs.size());
	SetNumVerts (4*patchs.size());

	// Number of patches
	pPM->setNumPatches (patchs.size());
	SetNumPatches (patchs.size());

	// Number of tangents
	// Number of interiors
	pPM->setNumVecs (12*patchs.size());

	// Number of edges
	pPM->setNumEdges (4*patchs.size());

	// Fill the vertices and tangents
	for (uint patch=0; patch<patchs.size(); patch++)
	{
		// The vector
		for (uint vert=0; vert<4; vert++)
		{
			// Pos ref
			CVector &pos = patchs[patch].Patch.Vertices[vert];
			CVector &inter = patchs[patch].Patch.Interiors[vert];

			// Dest ref
			PatchVert &destVert = pPM->verts[patch*4+vert];
			PatchVec &destVect = pPM->vecs[patch*12+vert];

			// Set the position
			destVert.p = Point3 (pos.x, pos.y, pos.z);
			destVect.p = Point3 (inter.x, inter.y, inter.z);

			// Set the flag
			destVert.flags = PVERT_CORNER;
			destVect.flags = PVEC_INTERIOR;
		}

		// The tan
		for (uint tang=0; tang<8; tang++)
		{
			// Pos ref
			CVector &pos = patchs[patch].Patch.Tangents[tang];

			// Dest ref
			PatchVec &destVect = pPM->vecs[patch*12+4+tang];

			// Set the position
			destVect.p = Point3 (pos.x, pos.y, pos.z);

			// Set the flag
			destVect.flags = 0;
		}

		// The indexes
		Patch &patchRef = pPM->patches[patch];
		for (uint i=0; i<4; i++)
		{
			patchRef.v[i] = patch*4 + i;
			patchRef.vec[2*i] = patch*12 + 4 + 2*i;
			patchRef.vec[2*i+1] = patch*12 + 4 + 2*i + 1;
			patchRef.interior[i] = patch*12 + i;
			patchRef.edge[i] = patch*4 + i;
			patchRef.smGroup = 1;
			patchRef.flags = 0;
			patchRef.type = PATCH_QUAD;
		}

		// Get the userinfo patch
		UI_PATCH &uiRef = getUIPatch (patch);
		uiRef.Init (getPowerOf2 (patchs[patch].OrderS), getPowerOf2 (patchs[patch].OrderT));

		// Copy tiles
		uint u, v;
		for (v=0; v<patchs[patch].OrderT; v++)
		for (u=0; u<patchs[patch].OrderS; u++)
		{
			// Tile index 
			uint tileindex = u+v*patchs[patch].OrderS;

			// Get the tile des
			tileDesc& desc = uiRef.getTileDesc (tileindex);
			int numLayer = 
				(patchs[patch].Tiles[tileindex].Tile[0]==NL_TILE_ELM_LAYER_EMPTY)?0:
				(patchs[patch].Tiles[tileindex].Tile[1]==NL_TILE_ELM_LAYER_EMPTY)?1:
				(patchs[patch].Tiles[tileindex].Tile[2]==NL_TILE_ELM_LAYER_EMPTY)?2:
				3;

			// Case info
			bool is256x256;
			uint8 uvOff;
			CTileElement &tileElement = patchs[patch].Tiles[tileindex];
			tileElement.getTile256Info(is256x256, uvOff);

			// Set the tile
			desc.setTile (numLayer, is256x256?uvOff+1:0, tileElement.getTileSubNoise(), 
				tileIndex (false, tileElement.Tile[0], tileElement.getTileOrient (0)), 
				tileIndex (false, tileElement.Tile[1], tileElement.getTileOrient (1)), 
				tileIndex (false, tileElement.Tile[2], tileElement.getTileOrient (2)));
		}

		// Tile colors
		for (v=0; v<(uint)(patchs[patch].OrderT+1); v++)
		for (u=0; u<(uint)(patchs[patch].OrderS+1); u++)
		{
			CRGBA color;
			color.set565 (patchs[patch].TileColors[u+v*(patchs[patch].OrderS+1)].Color565);
			uiRef.setColor (u+v*(patchs[patch].OrderS+1), (color.R<<16)|(color.G<<8)|(color.B));
		}

		// Edge flags
		for (uint edge=0; edge<4; edge++)
		{
			uiRef.setEdgeFlag (edge, (patchs[patch].Flags&(1<<edge))==0);
		}
	}

	// Rebuild the patch mesh
	nlverify (pPM->buildLinkages ()==TRUE);
}

// ***************************************************************************
