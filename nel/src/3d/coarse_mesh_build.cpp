/** \file coarse_mesh_build.cpp
 * <File description>
 *
 * $Id: coarse_mesh_build.cpp,v 1.3 2002/02/28 12:59:49 besson Exp $
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

#include "std3d.h"

#include "3d/coarse_mesh_build.h"

#include "3d/mesh.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

bool CCoarseMeshBuild::build (const std::vector<CCoarseMeshDesc>& coarseMeshes, CBitmap& bitmap, CStats& stats, float mulArea)
{
	// 1. build the bitmap
	MapBitmapDesc desc;
	if (buildBitmap (coarseMeshes, bitmap, stats, desc, mulArea)==false)
		return false;

	// 2. remap coordinates
	remapCoordinates (coarseMeshes, desc);

	// 3. ok
	return true;
}

// ***************************************************************************

// Class descriptor for bitmap inserted
class CInsertedBitmap
{
public:
	// Width and height
	uint Width;
	uint Height;

	// Coordinates
	uint U;
	uint V;
};

// ***************************************************************************

bool CCoarseMeshBuild::buildBitmap (const std::vector<CCoarseMeshDesc>& coarseMeshes, CBitmap& bitmap, CStats& stats, MapBitmapDesc& desc, float mulArea)
{
	// Total area used by texture
	uint totalArea=0;

	// ***************************************************************************

	// 1. scan each bitmap: calc the area of the bitmap and it its name in the maps sorted by area
	typedef std::multimap<uint, CBitmapDesc> MapAreaBitmap;
	MapAreaBitmap mapArea;
	uint mesh;
	for (mesh=0; mesh<coarseMeshes.size(); mesh++)
	{
		// Geom mesh pointer
		CMeshGeom *meshGeom=coarseMeshes[mesh].MeshGeom;

		// Base mesh pointer
		const CMeshBase *meshBase=coarseMeshes[mesh].MeshBase;

		// For each matrix block
		uint matrixBlock;
		uint nbMatrixBlock=meshGeom->getNbMatrixBlock();
		for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
		{
			// For each render pass
			uint renderPass;
			uint numRenderPass=meshGeom->getNbRdrPass(matrixBlock);
			for (renderPass=0; renderPass<numRenderPass; renderPass++)
			{
				// Render pass material
				uint32 matId=meshGeom->getRdrPassMaterial(matrixBlock, renderPass);

				// Checks
				nlassert (matId<meshBase->getNbMaterial());

				// Get the material
				const CMaterial &material=meshBase->getMaterial(matId);

				// Get the texture
				ITexture *texture=material.getTexture(0);
				if (texture)
				{
					// Get its name
					std::string name;
					if (texture->supportSharing())
					{
						// Get sharing name
						name=texture->getShareName();
					}
					else
					{
						// Build a name
						name=toString ((uint)texture);
					}

					// Already added ?
					if (desc.find (name)==desc.end())
					{
						// Add it..

						// Generate the texture
						texture->generate();

						// Convert to RGBA
						texture->convertToType (CBitmap::RGBA);

						// Backup original size
						float originalWidth = (float)texture->getWidth();
						float originalHeight = (float)texture->getHeight();

						// Expand the texture
						expand (*texture);

						// Descriptor for this texture
						CBitmapDesc descBitmap;
						uint area = texture->getWidth() * texture->getHeight();
						descBitmap.Texture = texture;
						descBitmap.Name = name;
						descBitmap.FactorU = originalWidth;
						descBitmap.FactorV = originalHeight;

						// Insert it in the maps
						desc.insert (MapBitmapDesc::value_type (name, descBitmap));
						mapArea.insert (MapAreaBitmap::value_type(area, descBitmap));

						// Sum area if added
						totalArea+=area;
					}
				}
			}
		}
	}

	// ***************************************************************************

	// 2. Calc the best area for the dest texture and resize the bitmap
	
	// Total area used by the textures + a little more
	uint newArea=getPowerOf2 (raiseToNextPowerOf2 (totalArea));
	while ((1<<newArea)<(sint)(mulArea*(float)totalArea))
	{
		newArea++;
	}

	// Calc width and height with HEIGHT==WIDTH or HEIGHT=2*WIDTH
	uint width=1<<(newArea/2);
	uint height=1<<(newArea/2 + (newArea&1));

	// Resize the bitmap and set the pixel format
	bitmap.resize (width, height, CBitmap::RGBA);

	// Checks
	if (totalArea==0)
	{
		// No texture, ok computed.
		stats.TextureUsed=1;

		return true;
	}

	// ***************************************************************************

	// 3. Place each texture in the bitmap in uncreasing order
	typedef std::multimap<sint, CInsertedBitmap> mapInsertedBitmap;
	
	// For each texture
	MapAreaBitmap::iterator ite=mapArea.end();

	// Inserted bitmap desc
	mapInsertedBitmap inserted;

	// Max texture height
	uint maxTexHeight=0;

	do 
	{
		ite--;
		nlassert (ite!=mapArea.end());

		// Texture
		ITexture *texture=ite->second.Texture;

		// Size of the texture
		uint widthTex=texture->getWidth();
		uint heightTex=texture->getHeight();

		// Width and height max
		uint widthMax=width-widthTex;
		uint heightMax=height-heightTex;

		// Test against others..
		bool enter;

		// For each row and each column
		for (uint v=0; v<heightMax; v++)
		{
			for (uint u=0; u<widthMax; u++)
			{
				// Test against others..
				enter=true;

				// Get the first to test
				mapInsertedBitmap::iterator toTest=inserted.lower_bound ((sint)v-(sint)maxTexHeight);
				while (toTest!=inserted.end())
				{
					// Make a test ?
					if ((sint)(v+heightTex)<=(toTest->first))
					{
						// Ok, end test
						break;
					}

					// Test it
					uint otherU=toTest->second.U;
					uint otherV=toTest->second.V;
					uint otherWidth=toTest->second.Width;
					uint otherHeight=toTest->second.Height;
					if ((v<otherV+otherHeight) && (v+heightTex>otherV) &&
						(u<otherU+otherWidth) && (u+widthTex>otherU))
					{
						// Collision
						enter=false;
						u=toTest->second.U+otherWidth-1;
						break;
					}

					// Next to test
					toTest++;
				}

				// Enter ?
				if (enter)
				{
					// Ok, enter

					// Insert an inserted descriptor
					CInsertedBitmap descInserted;
					descInserted.Width=widthTex;
					descInserted.Height=heightTex;
					descInserted.U=u;
					descInserted.V=v;
					inserted.insert (mapInsertedBitmap::value_type (v, descInserted));
	
					// Max height
					if (heightTex>maxTexHeight)
						maxTexHeight=heightTex;

					// Blit in the texture
					bitmap.blit (texture, u, v);

					// Get the descriptor by the name
					MapBitmapDesc::iterator iteInserted=desc.find (ite->second.Name);
					nlassert (iteInserted!=desc.end());

					// Set the U and V texture coordinates
					iteInserted->second.U=(float)(u+1)/(float)width;
					iteInserted->second.V=(float)(v+1)/(float)height;

					// Set ratio
					iteInserted->second.FactorU /= (float)width;
					iteInserted->second.FactorV /= (float)height;

					// End
					break;
				}

				// next..
			}

			// Enter ?
			if (enter)
				break;
		}

		// Not enter ?
		if (!enter)
			// Texture too small..
			return false;
	}
	while (ite!=mapArea.begin());

	// Some stats
	stats.TextureUsed=(float)totalArea/(float)(width*height);

	return true;
}

// ***************************************************************************

void CCoarseMeshBuild::expand (CBitmap& bitmap)
{
	// Get size
	uint width=bitmap.getWidth();
	uint height=bitmap.getHeight();

	// Valid size ?
	if ((width!=0) && (height!=0))
	{
		// Copy the bitmap
		CBitmap copy=bitmap;

		// Resize the bitmap
		bitmap.resize (width+2, height+2);

		// Copy old bitmap
		bitmap.blit (&copy, 1, 1);

		// Make a top and bottom border
		uint32 *topSrc=(uint32*)&(copy.getPixels()[0]);
		uint32 *topDest=((uint32*)&(bitmap.getPixels()[0]))+1;
		memcpy (topDest, topSrc, 4*width);
		uint32 *bottomSrc=topSrc+width*(height-1);
		uint32 *bottomDest=((uint32*)&(bitmap.getPixels()[0]))+(width+2)*(height+1)+1;
		memcpy (bottomDest, bottomSrc, 4*width);

		// Make a left and right border
		uint32 *leftSrc=(uint32*)&(copy.getPixels()[0]);
		uint32 *leftDest=((uint32*)&(bitmap.getPixels()[0]))+width+2;
		uint32 *rightSrc=leftSrc+width-1;
		uint32 *rightDest=leftDest+width+1;
		uint i;
		for (i=0; i<height; i++)
		{
			// Copy the borders
			*leftDest=*leftSrc;
			*rightDest=*rightSrc;

			// Move pointers
			leftDest+=width+2;
			rightDest+=width+2;
			leftSrc+=width;
			rightSrc+=width;
		}

		// Make corners

		// Left top
		*(uint32*)&(bitmap.getPixels()[0])=*(uint32*)&(copy.getPixels()[0]);

		// Rigth top
		*(((uint32*)&(bitmap.getPixels()[0]))+width+1)=*(((uint32*)&(copy.getPixels()[0]))+width-1);

		// Rigth bottom
		*(((uint32*)&(bitmap.getPixels()[0]))+(width+2)*(height+2)-1)=*(((uint32*)&(copy.getPixels()[0]))+width*height-1);

		// Left bottom
		*(((uint32*)&(bitmap.getPixels()[0]))+(width+2)*(height+1))=*(((uint32*)&(copy.getPixels()[0]))+width*(height-1));
	}
}

// ***************************************************************************

void CCoarseMeshBuild::remapCoordinates (const std::vector<CCoarseMeshDesc>& coarseMeshes, const MapBitmapDesc& desc)
{
	// 1. scan each bitmap: calc the area of the bitmap and it its name in the maps sorted by area
	typedef std::multimap<float, CBitmapDesc> MapAreaBitmap;
	MapAreaBitmap mapArea;
	uint mesh;
	for (mesh=0; mesh<coarseMeshes.size(); mesh++)
	{
		// Geom mesh pointer
		CMeshGeom *meshGeom=coarseMeshes[mesh].MeshGeom;

		// Base mesh pointer
		const CMeshBase *meshBase=coarseMeshes[mesh].MeshBase;

		// The vertex buffer
		CVertexBuffer &vertexBuffer=const_cast<CVertexBuffer&> (meshGeom->getVertexBuffer());

		// For each matrix block
		uint matrixBlock;
		uint nbMatrixBlock=meshGeom->getNbMatrixBlock();
		for (matrixBlock=0; matrixBlock<nbMatrixBlock; matrixBlock++)
		{
			// For each render pass
			uint renderPass;
			uint numRenderPass=meshGeom->getNbRdrPass(matrixBlock);
			for (renderPass=0; renderPass<numRenderPass; renderPass++)
			{
				// Render pass material
				uint32 matId=meshGeom->getRdrPassMaterial(matrixBlock, renderPass);

				// Checks
				nlassert (matId<meshBase->getNbMaterial());

				// Get the material
				const CMaterial &material=meshBase->getMaterial(matId);

				// Get the texture
				ITexture *texture=material.getTexture(0);
				if (texture)
				{
					// Get its name
					std::string name;
					if (texture->supportSharing())
					{
						// Get sharing name
						name=texture->getShareName();
					}
					else
					{
						// Build a name
						name=toString ((uint)texture);
					}

					// Find the texture
					MapBitmapDesc::const_iterator ite=desc.find (name);
					nlassert (ite!=desc.end());

					// Descriptor ref
					const CBitmapDesc& descBitmap=ite->second;

					// Get primitives
					const CPrimitiveBlock &primitiveBlock=meshGeom->getRdrPassPrimitiveBlock(matrixBlock,renderPass);

					// Set of vertex to remap
					std::set<uint> vertexToRemap;

					// Remap triangles
					uint index;
					const uint32 *indexPtr=primitiveBlock.getTriPointer();
					uint32 numIndex=3*primitiveBlock.getNumTri();
					for (index=0; index<numIndex; index++)
						vertexToRemap.insert (indexPtr[index]);

					// Remap quad
					indexPtr=primitiveBlock.getQuadPointer();
					numIndex=4*primitiveBlock.getNumQuad();
					for (index=0; index<numIndex; index++)
						vertexToRemap.insert (indexPtr[index]);

					// Remap the vertex
					std::set<uint>::iterator iteRemap=vertexToRemap.begin();
					while (iteRemap!=vertexToRemap.end())
					{
						// Remap the vertex
						float *UVCoordinate=(float*)vertexBuffer.getTexCoordPointer(*iteRemap);
						UVCoordinate[0]=UVCoordinate[0]*descBitmap.FactorU+descBitmap.U;
						UVCoordinate[1]=UVCoordinate[1]*descBitmap.FactorV+descBitmap.V;

						// Next vertex
						iteRemap++;
					}					
				}
			}
		}
	}
}

// ***************************************************************************

} // NL3D
