/** \file export_material.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_material.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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
#include "export_nel.h"
#include <nel/3d/texture_file.h>

using namespace NLMISC;
using namespace NL3D;

// Name of animatable values
#define BMTEX_CROP_APPLY "apply"
#define BMTEX_CROP_U_NAME "clipu"
#define BMTEX_CROP_V_NAME "clipv"
#define BMTEX_CROP_W_NAME "clipw"
#define BMTEX_CROP_H_NAME "cliph"

// Build an array of NeL material corresponding with max material at this node. Return the number of material exported.
// Fill an array to remap the 3ds vertexMap channels for each materials. remap3dsTexChannel.size() must be == to materials.size(). 
// remap3dsTexChannel[mat].size() is the final count of NeL vertexMap channels used for the material n° mat.
// For each NeL channel of a material, copy the 3ds channel remap3dsTexChannel[nelChannel]._IndexInMaxMaterial using the transformation matrix
// remap3dsTexChannel[nelChannel]._UVMatrix.
int CExportNel::buildMaterials (std::vector<CMaterial>& materials, std::vector<std::vector<CMaterialDesc> >& remap3dsTexChannel, 
								std::vector<std::string>& materialNames, INode& node, TimeValue time, bool absolutePath)
{
	// Material count
	int nMaterialCount=0;

	// Get primary material pointer of the node
	Mtl* pNodeMat=node.GetMtl();

	// If NULL, no material at all at this node
	if (pNodeMat!=NULL)
	{
		// Number of sub material at in this material
		nMaterialCount=pNodeMat->NumSubMtls();

		// If it is a multisub object, export all its sub materials
		if (nMaterialCount>0)
		{
			// Resize the destination array
			materials.resize (nMaterialCount);

			// Resize the vertMap remap table
			remap3dsTexChannel.resize (nMaterialCount);

			// Export all the sub materials
			for (int nSub=0; nSub<nMaterialCount; nSub++)
			{
				// Get a pointer on the sub material
				Mtl* pSub=pNodeMat->GetSubMtl(nSub);

				// Should not be NULL
				nlassert (pSub);

				// Export it
				materialNames.push_back (buildAMaterial (materials[nSub], remap3dsTexChannel[nSub], *pSub, time, absolutePath));
			}
		}
		// Else export only this material, so, count is 1
		else
		{
			// Only one material
			nMaterialCount=1;

			// Resize the destination array
			materials.resize (1);

			// Resize the vertMap remap table
			remap3dsTexChannel.resize (1);

			// Export the main material
			materialNames.push_back (buildAMaterial (materials[0], remap3dsTexChannel[0], *pNodeMat, time, absolutePath));
		}
	}

	// If no material exported
	if (nMaterialCount==0)
	{
		// Insert at least a material
		materials.resize (1);
		nMaterialCount=1;

		// Resize the vertMap remap table
		remap3dsTexChannel.resize (1);

		// Init the first material
		materials[0].initLighted();

		// Export the main material
		materialNames.push_back ("Default");
	}

	// Return the count of material
	return nMaterialCount;
}

// Build a NeL material corresponding with a max material.
std::string CExportNel::buildAMaterial (CMaterial& material, std::vector<CMaterialDesc>& remap3dsTexChannel, Mtl& mtl, TimeValue time, bool absolutePath)
{
	// Init the material lighted
	material.initLighted ();

	// *** ***************
	// *** Export Textures
	// *** ***************

	/// TODO: Only one texture for the time. Add multitexture support, and shaders support.

	// Look for a diffuse texmap
	Texmap* pDifTexmap=mtl.GetSubTexmap(ID_DI);
	Texmap* pOpaTexmap=mtl.GetSubTexmap(ID_OP);

	// Set a normal shader. TODO.
	material.setShader (CMaterial::Normal);

	// By default set blend to false
	material.setBlend (false);

	// Diffuse texmap is present ?
	if (pDifTexmap)
	{
		// Pointer on the  diffuse texture
		static ITexture* pTexture=NULL;

		// Is it a simple file ?
		if (isClassIdCompatible(*pDifTexmap, Class_ID (BMTEX_CLASS_ID,0)))
		{
			// List of channels used by this texture
			std::vector<CMaterialDesc> _3dsTexChannel;
			
			// Ok export the texture in NeL format
			pTexture=buildATexture (*pDifTexmap, _3dsTexChannel, time, absolutePath);

			// Check vertMap size
			nlassert (_3dsTexChannel.size()==1);

			// For this shader, only need a texture channel.
			remap3dsTexChannel.resize (1);

			// Need an explicit channel, not generated
			if ((_3dsTexChannel[0]._IndexInMaxMaterial==UVGEN_OBJXYZ)||(_3dsTexChannel[0]._IndexInMaxMaterial==UVGEN_WORLDXYZ))
			{
				remap3dsTexChannel[0]._IndexInMaxMaterial=UVGEN_MISSING;
				remap3dsTexChannel[0]._UVMatrix.IdentityMatrix();
			}
			// Else copy it
			else 
				remap3dsTexChannel[0]=_3dsTexChannel[0];

			// Add the texture if it exist
			material.setTexture(0, pTexture);

			// Active blend if texture in opacity
			material.setBlend (pOpaTexmap!=NULL);
		}
	}

	// Blend mode by default. TODO.
	material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);

	// Z function by default. TODO.
	material.setZFunc (CMaterial::lessequal);

	// Z bias by default. TODO.
	material.setZBias (0.f);

	// Is the mtl a std material ?
	// The class Id can be the StdMat one, or StdMat2
	// It can be the superClassId if the mtl is derived from StdMat or StdMat2.
    if (
		isClassIdCompatible (mtl, Class_ID(DMTL_CLASS_ID, 0))	||
		isClassIdCompatible (mtl, Class_ID(DMTL2_CLASS_ID, 0))
		)
	{
		// Get a pointer on a stdmat
		StdMat2* stdmat=(StdMat2*)&mtl;
		
		// *****************************************
		// *** Colors, self illumination and opacity
		// *****************************************

		// Get the diffuse color of the max material
		Color color=stdmat->GetDiffuse (time);

		// Convert to NeL color
		CRGBA nelColor;
		convertColor (nelColor, color);

		// Get the opacity value from the material
		float fOp=stdmat->GetOpacity (time);

		// Add alpha to the value
		float fA=(fOp*255.f+0.5f);
		clamp (fA, 0.f, 255.f);
		nelColor.A=(uint8)fA;

		// Set le NeL diffuse color material
		material.setColor (nelColor);

		// Set the blend mode on if opacity is not 1.f
		if (fOp<0.99f)
			material.setBlend (true);

		// Get colors of 3dsmax material
		CRGBA emissiveColor;
		CRGBA ambientColor;
		CRGBA diffuseColor;
		CRGBA specularColor;
		if (stdmat->GetSelfIllumColorOn())
			convertColor (emissiveColor, stdmat->GetSelfIllumColor (time));
		else
			convertColor (emissiveColor, stdmat->GetDiffuse (time)*stdmat->GetSelfIllum (time));
		convertColor (ambientColor, stdmat->GetAmbient (time));
		convertColor (diffuseColor, stdmat->GetDiffuse (time));
		diffuseColor.A=(uint8)fA;
		convertColor (specularColor, stdmat->GetSpecular (time));

		// Specular level
		float shininess=stdmat->GetShinStr(time);
		CRGBAF fColor=specularColor;
		fColor*=shininess;
		specularColor=fColor;

		// Shininess
		shininess=stdmat->GetShader()->GetGlossiness(time);
		shininess=(float)pow(2.0, shininess * 10.0) * 4.f;

		// Light parameters
		material.setLighting (true, false, emissiveColor, ambientColor, diffuseColor, specularColor, shininess);

		// Double sided
		material.setDoubleSided (stdmat->GetTwoSided()!=FALSE);
	}

	// Get material name
	TSTR name=mtl.GetName();
	return (const char*)name;
}

// Get 3ds channels uv used by a texmap and make a good index channel
int CExportNel::getVertMapChannel (Texmap& texmap, Matrix3& channelMatrix)
{
	// *** Get the channel matrix

	// Set to identity because deafult implementation of texmapGetUVTransform make nothing
	channelMatrix.IdentityMatrix();

	// Get UV channel matrix
	texmap.GetUVTransform(channelMatrix);

	// *** Get the uvs channel

	// Channel to find
	int nChannel=UVGEN_MISSING;

	// Get the UVs coordinate source
	switch (texmap.GetUVWSource())
	{
	
	// Use object space generated UVs ?
	case UVWSRC_OBJXYZ:
		// return object space generated
		return UVGEN_OBJXYZ;

	// Use world space generated UVs ?
	case UVWSRC_WORLDXYZ:
		// Set object space generated
		return UVGEN_WORLDXYZ;

	// UVs explicit ?
	case UVWSRC_EXPLICIT:
		// Get the channel number
		nChannel=texmap.GetMapChannel();
		break;

	// Use vertex color channel ?
	case UVWSRC_EXPLICIT2:
		// Set vertex color channel
		nChannel=0;
		break;

	default:
		nlassert (0);	// no, should one of the previous case.
	}

	// return the channel
	return nChannel;
}
	
// Build a NeL texture corresponding with a max Texmap.
// Fill an array with the 3ds vertexMap used by this texture. 
// Texture file uses only 1 channel.
ITexture* CExportNel::buildATexture (Texmap& texmap, std::vector<CMaterialDesc>& remap3dsTexChannel, TimeValue time, bool absolutePath)
{
	/// TODO: support other texmap than Bitmap

	// By default, not build
	ITexture* pTexture=NULL;

	// Is it a bitmap texture file ?
	if (isClassIdCompatible(texmap, Class_ID (BMTEX_CLASS_ID,0)))
	{
		// Cast the pointer
		BitmapTex* pBitmap=(BitmapTex*)&texmap;

		// Alloc a texture
		CTextureFile *pTextureFile=new CTextureFile ();

		// File name, maxlen 256 under windows
		char sFileName[512];
		strcpy (sFileName, pBitmap->GetMapName());

		// Let absolute path ?
		if (!absolutePath)
		{
			// Decompose bitmap file name
			char sName[256];
			char sExt[256];
			_splitpath (sFileName, NULL, NULL, sName, sExt);

			// Make the final path
			_makepath (sFileName, NULL, NULL, sName, sExt);
		}

		// Set the file name
		pTextureFile->setFileName (sFileName);

		// Resize the channel count
		remap3dsTexChannel.resize (1);

		// Get the UVs channel and the channel matrix
		Matrix3	channelMatrix;
		int nChannel=getVertMapChannel (texmap, channelMatrix);

		// Add the UVs channel
		remap3dsTexChannel[0]._IndexInMaxMaterial=nChannel;
		remap3dsTexChannel[0]._UVMatrix=channelMatrix;

		// Get the apply crop value
		int bApply;
		bool bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_APPLY, (ParamType2)TYPE_BOOL, &bApply, time);
		nlassert (bRes);

		// If a crop is applyed
		if (bApply)
		{
			// Get the crop value U
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_U_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel[0]._CropU), time);
			nlassert (bRes);

			// Get the crop value V
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_V_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel[0]._CropV), time);
			nlassert (bRes);

			// Get the crop value W
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_W_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel[0]._CropW), time);
			nlassert (bRes);

			// Get the crop value H
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_H_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel[0]._CropH), time);
			nlassert (bRes);
		}

		// Ok, good texture
		pTexture=pTextureFile;
	}
	// else ...  to be continued

	// Return the texture pointer
	return pTexture;
}
