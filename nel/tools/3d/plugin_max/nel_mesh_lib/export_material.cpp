/** \file export_material.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_material.cpp,v 1.14 2001/08/02 12:17:56 besson Exp $
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
#include <3d/texture_file.h>
#include <3d/texture_cube.h>

#include <vector>
#include <string>

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// Name of animatable values
#define BMTEX_CROP_APPLY "apply"
#define BMTEX_CROP_U_NAME "clipu"
#define BMTEX_CROP_V_NAME "clipv"
#define BMTEX_CROP_W_NAME "clipw"
#define BMTEX_CROP_H_NAME "cliph"

#define NEL_MTL_A 0x64c75fec
#define NEL_MTL_B 0x222b9eb9

// Build an array of NeL material corresponding with max material at this node. Return the number of material exported.
// Fill an array to remap the 3ds vertexMap channels for each materials. maxBaseBuild.RemapChannel.size() must be == to materials.size(). 
// maxBaseBuild.RemapChannel[mat].size() is the final count of NeL vertexMap channels used for the material n° mat.
// For each NeL channel of a material, copy the 3ds channel maxBaseBuild.RemapChannel[nelChannel]._IndexInMaxMaterial using the transformation matrix
// maxBaseBuild.RemapChannel[nelChannel]._UVMatrix.
// This method append the node material to the vector passed.
void CExportNel::buildMaterials (std::vector<NL3D::CMaterial>& materials, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
								TimeValue time, bool absolutePath)
{
	// Material count
	maxBaseBuild.FirstMaterial=materials.size();
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
			materials.resize (materials.size()+nMaterialCount);

			// Resize the vertMap remap table
			maxBaseBuild.RemapChannel.resize (nMaterialCount);

			// Export all the sub materials
			for (int nSub=0; nSub<nMaterialCount; nSub++)
			{
				// Get a pointer on the sub material
				Mtl* pSub=pNodeMat->GetSubMtl(nSub);

				// Should not be NULL
				nlassert (pSub);

				// Export it
				maxBaseBuild.MaterialNames.push_back (buildAMaterial (materials[maxBaseBuild.FirstMaterial+nSub], maxBaseBuild.RemapChannel[nSub], *pSub, time, absolutePath));
			}
		}
		// Else export only this material, so, count is 1
		else
		{
			// Only one material
			nMaterialCount=1;

			// Resize the destination array
			materials.resize (materials.size()+1);

			// Resize the vertMap remap table
			maxBaseBuild.RemapChannel.resize (1);

			// Export the main material
			maxBaseBuild.MaterialNames.push_back (buildAMaterial (materials[maxBaseBuild.FirstMaterial], maxBaseBuild.RemapChannel[0], *pNodeMat, time, absolutePath));
		}
	}

	// If no material exported
	if (nMaterialCount==0)
	{
		// Insert at least a material
		materials.resize (materials.size()+1);
		nMaterialCount=1;

		// Resize the vertMap remap table
		maxBaseBuild.RemapChannel.resize (1);

		// Init the first material
		materials[maxBaseBuild.FirstMaterial].initLighted();

		// Export the main material
		maxBaseBuild.MaterialNames.push_back ("Default");
	}

	// Return the count of material
	maxBaseBuild.NumMaterials=nMaterialCount;
}

// Build a NeL material corresponding with a max material.
std::string CExportNel::buildAMaterial (CMaterial& material, std::vector<CMaterialDesc>& remap3dsTexChannel, Mtl& mtl, TimeValue tvTime, bool absolutePath)
{
	// Init the material lighted
	material.initLighted ();

	// *** ***************
	// *** Export Textures
	// *** ***************

	/// TODO: Only one texture for the time. Add multitexture support, and shaders support.

	// Look for a diffuse texmap
	vector<bool> mapEnables;
	CExportNel::getValueByNameUsingParamBlock2 (mtl, "mapEnables", (ParamType2)TYPE_BOOL_TAB, &mapEnables, tvTime);

	Texmap *pDifTexmap = NULL;
	Texmap *pOpaTexmap = NULL;
	Texmap *pSpeTexmap = NULL;

	if (mapEnables[ID_DI])
		pDifTexmap = mtl.GetSubTexmap (ID_DI);
	if (mapEnables[ID_OP])
		pOpaTexmap = mtl.GetSubTexmap (ID_OP);
	if (mapEnables[ID_SP])
		pSpeTexmap = mtl.GetSubTexmap (ID_SP);

	// Is there a lightmap handling wanted
	int bLightMap = 0; // false
	int bAlphaTest = 1; // true
	int bForceZWrite = 0; // false
	int bForceNoZWrite = 0; // false

	CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceZWrite", (ParamType2)TYPE_BOOL, &bForceZWrite, tvTime);
	CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceNoZWrite", (ParamType2)TYPE_BOOL, &bForceNoZWrite, tvTime);

	CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaTest", (ParamType2)TYPE_BOOL, &bAlphaTest, tvTime);

	if( pSpeTexmap != NULL )
	{
		material.setShader (CMaterial::Specular);
	}
	else
	{
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, tvTime);		
		if (bLightMap)
		{
			material.setShader (CMaterial::LightMap);
		}
		else
		{
			material.setShader (CMaterial::Normal);
		}
	}

	int bStainedGlassWindow = 0;
	CExportNel::getValueByNameUsingParamBlock2 (mtl, "bStainedGlassWindow", (ParamType2)TYPE_BOOL, &bStainedGlassWindow, tvTime);
	material.setStainedGlassWindow( bStainedGlassWindow!=0 );

	material.setAlphaTest(false);

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
			pTexture=buildATexture (*pDifTexmap, _3dsTexChannel, tvTime, absolutePath);

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
			if( pOpaTexmap!=NULL )
			{
				if( bAlphaTest )
				{ // If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
					material.setAlphaTest(true);
					if( bForceNoZWrite )
						material.setZWrite( false );
					else
						material.setZWrite( true );
				}
				else
				{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
					material.setBlend( true );
					if( bForceZWrite )
						material.setZWrite( true );
					else
						material.setZWrite( false );
				}
			}
			

			// Export mapping channel 2 if lightmap asked.
			if( bLightMap ) // lightmap enabled ?
			{
				remap3dsTexChannel.resize( 2 );
				// Copy information from channel 0
				remap3dsTexChannel[1] = remap3dsTexChannel[0];
				remap3dsTexChannel[1]._IndexInMaxMaterial = 2;
				remap3dsTexChannel[1]._IndexInMaxMaterialAlternative = remap3dsTexChannel[0]._IndexInMaxMaterial;
			}
		}
	}

	if (pSpeTexmap)
	{
		// Pointer on the  diffuse texture
		static ITexture* pTexture=NULL;
		static CTextureCube* pTextureCube=NULL;

		pTextureCube = new CTextureCube;
		pTextureCube->setWrapS( ITexture::Clamp );
		pTextureCube->setWrapT( ITexture::Clamp );
		pTextureCube->setFilterMode(ITexture::Linear,ITexture::LinearMipMapOff);

		if( isClassIdCompatible(*pSpeTexmap, Class_ID (ACUBIC_CLASS_ID,0)) )
		{
			CTextureCube::TFace tfNewOrder[6] = {	CTextureCube::positive_z, CTextureCube::negative_z,
													CTextureCube::negative_x, CTextureCube::positive_x,
													CTextureCube::negative_y, CTextureCube::positive_y	};
			vector<string> names;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bitmapName", (ParamType2)TYPE_STRING_TAB, &names, tvTime);
			for( int i = 0; i< (int)names.size(); ++i )
			{
				CTextureFile *pT = new CTextureFile;

				char sFileName[512];
				strcpy(sFileName, names[i].c_str());
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
				pT->setFileName(sFileName);

				pTextureCube->setTexture(tfNewOrder[i], pT);
			}
		}
		else
		if( isClassIdCompatible(*pSpeTexmap, Class_ID (COMPOSITE_CLASS_ID,0)) )
		{
			int nNbSubMap = pSpeTexmap->NumSubTexmaps();
			if( nNbSubMap > 6 )
				nNbSubMap = 6;	
			for( int i = 0; i < nNbSubMap; ++i )
			{
				std::vector<CMaterialDesc> _3dsTexChannel;
				Texmap *pSubMap = pSpeTexmap->GetSubTexmap(i);

				if( pSubMap != NULL )
				if (isClassIdCompatible(*pSubMap, Class_ID (BMTEX_CLASS_ID,0)))
				{					
					std::vector<CMaterialDesc> _3dsTexChannel;
					pTexture = buildATexture (*pSubMap, _3dsTexChannel, tvTime, absolutePath);
					pTextureCube->setTexture((CTextureCube::TFace)i, pTexture);
				}
			}
		}
		else
		// Is it a simple file ?
		if (isClassIdCompatible(*pSpeTexmap, Class_ID (BMTEX_CLASS_ID,0)))
		{
			// List of channels used by this texture
			std::vector<CMaterialDesc> _3dsTexChannel;
			
			// Ok export the texture in NeL format
			pTexture = buildATexture (*pSpeTexmap, _3dsTexChannel, tvTime, absolutePath);
			pTextureCube->setTexture(CTextureCube::positive_x, pTexture);
		}
		// Add the texture if it exist
		material.setTexture(1, pTextureCube);
	}

	// Blend mode
	int opacityType = 0; // 0-filter 1-substractive 2-additive
	CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacityType", (ParamType2)TYPE_INT, &opacityType, tvTime);
	if( opacityType == 0 ) // Filter
		material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);
	else
		material.setBlendFunc (CMaterial::srcalpha, CMaterial::one);

	// Z function by default. TODO.
	material.setZFunc (CMaterial::lessequal);

	// Z bias by default. TODO.
	material.setZBias (0.f);

	// Is the mtl a std material ?
	// The class Id can be the StdMat one, or StdMat2
	// It can be the superClassId if the mtl is derived from StdMat or StdMat2.
    if (
		isClassIdCompatible (mtl, Class_ID(DMTL_CLASS_ID, 0))	||
		isClassIdCompatible (mtl, Class_ID(DMTL2_CLASS_ID, 0))	||
		isClassIdCompatible (mtl, Class_ID(NEL_MTL_A,NEL_MTL_B))
		)
	{
		// Get a pointer on a stdmat
		//StdMat2* stdmat=(StdMat2*)&mtl;

		// *****************************************
		// *** Colors, self illumination and opacity
		// *****************************************

		// Get the diffuse color of the max material
		//Color color=stdmat->GetDiffuse (tvTime);
		Point3 maxDiffuse;
		CRGBA  nelDiffuse;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "diffuse", (ParamType2)TYPE_RGBA, &maxDiffuse, tvTime);

		// Convert to NeL color
		convertColor (nelDiffuse, maxDiffuse);
		// Get the opacity value from the material
		// float fOp=stdmat->GetOpacity (tvTime);
		float fOp = 0.0f;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacity", (ParamType2)TYPE_PCNT_FRAC, &fOp, tvTime);

		// Add alpha to the value
		float fA=(fOp*255.f+0.5f);
		clamp (fA, 0.f, 255.f);
		nelDiffuse.A=(uint8)fA;
		// Set le NeL diffuse color material
		material.setColor (nelDiffuse);

		// Set the blend mode on if opacity is not 1.f
		if( fOp < 0.99f )
		{
			if( bAlphaTest )
			{ // If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
				material.setAlphaTest(true);
				if( bForceNoZWrite )
					material.setZWrite( false );
				else
					material.setZWrite( true );
			}
			else
			{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
				material.setBlend( true );
				if( bForceZWrite )
					material.setZWrite( true );
				else
					material.setZWrite( false );
			}
		}

		// Get colors of 3dsmax material
		CRGBA nelEmissive;
		CRGBA nelAmbient;
		CRGBA nelSpecular;
		//if (stdmat->GetSelfIllumColorOn())
		//	convertColor (emissiveColor, stdmat->GetSelfIllumColor (tvTime));
		//else
		//	convertColor (emissiveColor, stdmat->GetDiffuse (tvTime)*stdmat->GetSelfIllum (tvTime));
		int bSelfIllumColorOn;
		Point3 maxSelfIllum;
		float fTemp;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "useSelfIllumColor", (ParamType2)TYPE_BOOL, &bSelfIllumColorOn, tvTime);
		if( bSelfIllumColorOn )
		{
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumColor", (ParamType2)TYPE_RGBA, &maxSelfIllum, tvTime);
		}
		else
		{
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumAmount", (ParamType2)TYPE_PCNT_FRAC, &fTemp, tvTime);
			maxSelfIllum = maxDiffuse * fTemp;
		}
		convertColor( nelEmissive, maxSelfIllum );

		Point3 maxAmbient;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "ambient", (ParamType2)TYPE_RGBA, &maxAmbient, tvTime);
		convertColor (nelAmbient, maxAmbient);

		Point3 maxSpecular;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "specular", (ParamType2)TYPE_RGBA, &maxSpecular, tvTime);
		convertColor (nelSpecular, maxSpecular);

		// Specular level
		float shininess; //=stdmat->GetShinStr(tvTime);
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "specularLevel", (ParamType2)TYPE_PCNT_FRAC, &shininess, tvTime);
		CRGBAF fColor = nelSpecular;
		fColor *= shininess;
		nelSpecular = fColor;

		// Shininess
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "glossiness", (ParamType2)TYPE_PCNT_FRAC, &shininess, tvTime);
		//shininess=stdmat->GetShader()->GetGlossiness(tvTime);
		shininess=(float)pow(2.0, shininess * 10.0) * 4.f;

		// Light parameters
		material.setLighting (true, false, nelEmissive, nelAmbient, nelDiffuse, nelSpecular, shininess);

		// Double sided
		int bDoubleSided;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "twoSided", (ParamType2)TYPE_BOOL, &bDoubleSided, tvTime);

		//material.setDoubleSided (stdmat->GetTwoSided()!=FALSE);
		material.setDoubleSided ( bDoubleSided!=0 );
	}

	if( ! bLightMap )
	{
		int bUnlighted = 0; // false
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bUnlighted", (ParamType2)TYPE_BOOL, &bUnlighted, 0);
		if( bUnlighted )	
		{
			material.setLighting( false );
		}
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
