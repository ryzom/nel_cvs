/** \file calc_lm.cpp
 * <File description>
 *
 * $Id: calc_lm.cpp,v 1.6 2001/06/18 15:45:01 besson Exp $
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
//#include "nel_export.h"
#include "export_nel.h"
#include "../nel_patch_lib/rpo.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/triangle.h"
#include "nel/misc/bsphere.h"
#include "3d/quad_tree.h"
#include "3d/scene_group.h"
#include "3d/skeleton_shape.h"
#include "3d/texture_file.h"
#include "3d/light.h"

//#include "nel_export_scene.h"

#include "3d/bsp_tree.h"
#include "3d/quad_grid.h"

#include <vector>

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// TOOLS
// *****

#define NEL_LIGHT_CLASS_ID_A	0x36e3181f
#define NEL_LIGHT_CLASS_ID_B	0x3ac24049

#define MAXLIGHTMAPSIZE 1024

// ***********************************************************************************************
struct SLightBuild
{
	string GroupName;
	enum EType { LightAmbient, LightPoint, LightDir, LightSpot };
	EType Type;
	CVector Position;				// Used by LightPoint and LightSpot
	CVector Direction;				// Used by LightSpot and LightDir
	float rRadiusMin, rRadiusMax;	// Used by LightPoint and LightSpot
	float rHotspot, rFallof;		// Used by LightSpot
	CRGBA Ambient;
	CRGBA Diffuse;
	CRGBA Specular;
	bool bCastShadow;
	float rMult;
	Bitmap *pProjMap;
	CMatrix mProj;

	set<string> setExclusion;

	// -----------------------------------------------------------------------------------------------
	SLightBuild()
	{
		Type = LightPoint;
		Position = CVector(0.0, 0.0, 0.0);
		Direction = CVector(1.0, 0.0, 0.0);
		rRadiusMin = 1.0f;
		rRadiusMax = 2.0f;
		Ambient = CRGBA(0, 0, 0, 0);
		Diffuse = CRGBA(0, 0, 0, 0);
		Specular = CRGBA(0, 0, 0, 0);
		rMult = 1.0f;
		GroupName = "GlobalLight";
		pProjMap = NULL;
	}

	// -----------------------------------------------------------------------------------------------
	bool convertFromMaxLight (INode*node, TimeValue tvTime)
	{
		// Get a pointer on the object's node
		Object *obj = node->EvalWorldState(tvTime).obj;

		// Check if there is an object
		if (!obj)
			return false;

		// Get a GenLight from the node
		if (!(obj->SuperClassID()==LIGHT_CLASS_ID))
			return false;

		GenLight *maxLight = (GenLight *) obj;
		bool deleteIt=false;
		if (obj != maxLight) 
			deleteIt = true;

		Interval valid=NEVER;
		LightState ls;
		if (maxLight->EvalLightState(tvTime, valid, &ls)!=REF_SUCCEED)
			return false;

		// Is the light is animatable ? (TEMP MAT)
		Modifier *pModifier = CExportNel::getModifier( node, Class_ID(NEL_LIGHT_CLASS_ID_A, 
																	NEL_LIGHT_CLASS_ID_B) );
		if( pModifier != NULL )
		{
			int bDynamic;
			string sGroup;
			// Get the value of the parameters
			CExportNel::getValueByNameUsingParamBlock2( *pModifier, "bDynamic", 
														(ParamType2)TYPE_BOOL, &bDynamic, 0);
			CExportNel::getValueByNameUsingParamBlock2( *pModifier, "sGroup", 
														(ParamType2)TYPE_STRING, &sGroup, 0);
			if( bDynamic )
				this->GroupName = sGroup;
			else
				this->GroupName = "GlobalLight";
		}
		else
		{
			this->GroupName = "GlobalLight";
		}

		// Eval the light state fot this tvTime
		// Set the light mode
		switch (maxLight->Type())
		{
			case OMNI_LIGHT:
				this->Type = SLightBuild::EType::LightPoint;
			break;
			case TSPOT_LIGHT:
			case FSPOT_LIGHT:
				this->Type = SLightBuild::EType::LightSpot;
			break;
			case DIR_LIGHT:
			case TDIR_LIGHT:
				this->Type = SLightBuild::EType::LightDir;
			break;
			default:
				// Not initialized
			break;
		}

		// *** Set the light color

		// Get the color
		CRGBA nelColor;
		Point3 maxColor = maxLight->GetRGBColor(tvTime);

		// Mul by multiply
		CRGBAF nelFColor;
		nelFColor.R = maxColor.x;
		nelFColor.G = maxColor.y;
		nelFColor.B = maxColor.z;
		nelFColor.A = 1.f;
		// nelFColor   *= maxLight->GetIntensity(tvTime);
		nelColor = nelFColor;

		// Affect the ambiant color ?
		this->Ambient = CRGBA (0,0,0);
		this->Diffuse = CRGBA (0,0,0);
		this->Specular = CRGBA (0,0,0);

		
		if (maxLight->GetAmbientOnly())
		{
			this->Ambient = nelColor;
		}
		else
		{
			// Affect the diffuse color ?
			if( maxLight->GetAffectDiffuse() )
				this->Diffuse = nelColor;
			// Affect the specular color ?
			if (maxLight->GetAffectSpecular())
				this->Specular = nelColor;
		}

		// Set the light position
		Point3 pos = node->GetNodeTM(tvTime).GetTrans ();
		CVector position;
		position.x=pos.x;
		position.y=pos.y;
		position.z=pos.z;

		// Set the position
		this->Position = position;

		// Set the light direction
		CVector direction;
		INode* target = node->GetTarget ();
		if (target)
		{
			// Get the position of the target
			Point3 posTarget=target->GetNodeTM (tvTime).GetTrans ();
			CVector positionTarget;
			positionTarget.x=posTarget.x;
			positionTarget.y=posTarget.y;
			positionTarget.z=posTarget.z;

			// Direction
			direction=positionTarget-position;
			direction.normalize ();
		}
		else	// No target
		{
			// Get orientation of the source as direction
			CMatrix nelMatrix;
			CExportNel::convertMatrix (nelMatrix, node->GetNodeTM(tvTime));

			// Direction is -Z
			direction=-nelMatrix.getK();
			direction.normalize ();
		}

		// Set the direction
		this->Direction = direction;

		this->rHotspot = (float)(Pi * maxLight->GetHotspot(tvTime) /(2.0*180.0));
		this->rFallof =  (float)(Pi * maxLight->GetFallsize(tvTime)/(2.0*180.0));

		if (maxLight->GetUseAtten())
		{
			this->rRadiusMin = maxLight->GetAtten (tvTime, ATTEN_START);
			this->rRadiusMax = maxLight->GetAtten (tvTime, ATTEN_END);
		}
		else
		{	// Limit
			this->rRadiusMin = 10.0;
			this->rRadiusMax = 10.0;
		}

		this->bCastShadow = ( maxLight->GetShadow() != 0 );
		this->rMult = maxLight->GetIntensity (tvTime);

		if (maxLight->GetProjector() != 0)
		{
			Texmap* tm = maxLight->GetProjMap();
			CExportNel::convertMatrix (this->mProj, node->GetNodeTM(tvTime));
			if (CExportNel::isClassIdCompatible( *tm, Class_ID (BMTEX_CLASS_ID,0)))
			{
				BitmapTex* bmt = (BitmapTex*)tm;
				this->pProjMap = bmt->GetBitmap(tvTime);
			}
		}

		// Convert exclusion list
		NameTab& ntExclu = maxLight->GetExclusionList();
		for( sint i = 0; i < ntExclu.Count(); ++i )
		{
			string tmp = *ntExclu.Addr(i);
			this->setExclusion.insert( tmp );
		}
	
		if( deleteIt )
			delete maxLight;

		return true;
	}

};

// ***********************************************************************************************
struct SLMPixel
{
	CRGBAF p[8]; // 8 Layers of lightmap possible
};

// ***********************************************************************************************
struct SLMPlane
{
	sint32 x, y; // Pos in lightmap
	uint32 w, h; // Size
	vector<uint8> msk;	// 0 - No pixel
						// 1 - Pixel must be calculated
						// 2 - Pixel is interior and is calculated
						// 3 - Pixel is exterior in this plane but interior in another of the same smooth group
						// 4 - Pixel is exterior and is extrapolated
	vector<SLMPixel> col; // 32 bits value for each pixel of each layer
	uint32 nNbLayerUsed;
	vector<CMesh::CFace*> faces;	

	// -----------------------------------------------------------------------------------------------
	SLMPlane()
	{ 
		nNbLayerUsed = 0;
		x = y = 0; w = h = 1; msk.resize(1); msk[0] = 0; 
		col.resize(1); 
		for( sint32 i = 0; i < 8; ++i )
		{ col[0].p[i].R = col[0].p[i].G = col[0].p[i].B = col[0].p[i].A = 0.0f; }
	}

	// -----------------------------------------------------------------------------------------------
	bool isAllBlack (uint8 nLayerNb)
	{
		for( uint32 i = 0; i < w*h; ++i )
			if( (col[i].p[nLayerNb].R > 0.06f) || // around 15/255
				(col[i].p[nLayerNb].G > 0.06f) ||
				(col[i].p[nLayerNb].B > 0.06f) )
				return false; // Not all is black
		return true;
	}

	// -----------------------------------------------------------------------------------------------
	void copyColToBitmap32( CBitmap* pImage, sint32 nLayerNb )
	{
		if( ( pImage->getWidth() != w ) ||
			( pImage->getHeight() != h ) )
		{
			// ResizeBitmap32
			vector<uint8> vImgTemp;
			uint32 i, j;

			vImgTemp.resize( 4*w*h );
			for( i = 0; i < 4*w*h; ++i )
				vImgTemp[i] = 0;

			vector<uint8> &vBitmap = pImage->getPixels();
			uint32 nCurSizeX = pImage->getWidth();
			uint32 nCurSizeY = pImage->getHeight();
			for( j = 0; j < min(nCurSizeY,h); ++j )
			for( i = 0; i < min(nCurSizeX,w); ++i )
			{
				vImgTemp[4*(i+j*w)+0] = vBitmap[4*(i+j*pImage->getWidth())+0];
				vImgTemp[4*(i+j*w)+1] = vBitmap[4*(i+j*pImage->getWidth())+1];
				vImgTemp[4*(i+j*w)+2] = vBitmap[4*(i+j*pImage->getWidth())+2];
				vImgTemp[4*(i+j*w)+3] = vBitmap[4*(i+j*pImage->getWidth())+3];
			}

			pImage->resize(w,h);
			vBitmap = pImage->getPixels();
			for( i = 0; i < 4*w*h; ++i )
				vBitmap[i] = vImgTemp[i];
		}

		vector<uint8> &vBitmap = pImage->getPixels();

		for( uint32 i = 0; i < w*h; ++i )
		{
			if( (127.0*col[i].p[nLayerNb].R) > 255.0 )
				vBitmap[4*i+0] = 255;
			else
				vBitmap[4*i+0] = (uint8)(127.0*col[i].p[nLayerNb].R);
			if( (127.0*col[i].p[nLayerNb].G) > 255.0 )
				vBitmap[4*i+1] = 255;
			else
				vBitmap[4*i+1] = (uint8)(127.0*col[i].p[nLayerNb].G);
			if( (127.0*col[i].p[nLayerNb].B) > 255.0 )
				vBitmap[4*i+2] = 255;
			else
				vBitmap[4*i+2] = (uint8)(127.0*col[i].p[nLayerNb].B);
			vBitmap[4*i+3] = 255;
		}
	}

	// -----------------------------------------------------------------------------------------------
	// Put me in the plane Dst (copy the col and mask or mask only)
	void putIn( SLMPlane &Dst, bool bMaskOnly = false )
	{
		uint32 a, b;
		if( ( (this->w + this->x) > Dst.w ) || ( (this->h + this->y) > Dst.h ) )
		{
			a = 0; b = 0;
		}
		for( b = 0; b < this->h; ++b )
		for( a = 0; a < this->w; ++a )
			if( this->msk[a+b*this->w] != 0 )
			{
				Dst.msk[(this->x+a)+(this->y+b)*Dst.w] = this->msk[a+b*this->w];
				if( bMaskOnly == false )
					Dst.col[(this->x+a)+(this->y+b)*Dst.w] = this->col[a+b*this->w];
			}
	}

	// -----------------------------------------------------------------------------------------------
	// Test the mask between me and the plane dst (with my decalage)
	bool testIn( SLMPlane &Dst )
	{
		uint32 a, b;
		for( b = 0; b < this->h; ++b )
		for( a = 0; a < this->w; ++a )
			if( this->msk[a+b*this->w] != 0 )
				if( Dst.msk[(this->x+a)+(this->y+b)*Dst.w] != 0 )
					return false;
		return true;
	}

	// -----------------------------------------------------------------------------------------------
	// Try all position to put me in the plane dst
	bool tryAllPosToPutIn( SLMPlane &Dst )
	{
		uint32 i, j;

		if( this->w > Dst.w ) return false;
		if( this->h > Dst.h ) return false;

		// For all position test if the Src plane can be put in
		for( j = 0; j < (Dst.h-this->h); ++j )
		for( i = 0; i < (Dst.w-this->w); ++i )
		{
			this->x = i; this->y = j;
			if( testIn( Dst ) )
				return true;
		}
		return false;
	}

	// -----------------------------------------------------------------------------------------------
	// Do not stretch the image inside the plane just enlarge and fill with black
	void resize( uint32 nNewSizeX, uint32 nNewSizeY )
	{
		vector<uint8> vImgTemp;
		vector<SLMPixel> vImgTemp2;
		uint32 i, j;

		vImgTemp.resize(nNewSizeX*nNewSizeY);
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			vImgTemp[i] = 0;

		for( j = 0; j < min(this->h,nNewSizeY); ++j )
		for( i = 0; i < min(this->w,nNewSizeX); ++i )
		{
			vImgTemp[i+j*nNewSizeX] = this->msk[i+j*this->w];
		}

		this->msk.resize(nNewSizeX*nNewSizeY);
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			this->msk[i] = vImgTemp[i];

		// The same as the mask but for the bitmap
		vImgTemp2.resize(nNewSizeX*nNewSizeY);
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			for( j = 0; j < 8; ++j )
			{ vImgTemp2[i].p[j].R = vImgTemp2[i].p[j].G = vImgTemp2[i].p[j].B = vImgTemp2[i].p[j].A = 0.0f; }

		for( j = 0; j < min(this->h,nNewSizeY); ++j )
		for( i = 0; i < min(this->w,nNewSizeX); ++i )
			vImgTemp2[i+j*nNewSizeX] = this->col[i+j*this->w];

		this->col.resize(nNewSizeX*nNewSizeY);
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			this->col[i] = vImgTemp2[i];

		this->w = nNewSizeX;
		this->h = nNewSizeY;
	}

	// -----------------------------------------------------------------------------------------------
	// Stretch a plane by a given factor 4.0 -> multiply its size by 4 and 0.5 -> halves its size
	// Take care the decalage is affected by the scaling factor (like homothetie)
	void stretch( double osFactor )
	{
		if( osFactor < 0.0 )
			osFactor = 0.0;
		uint32 nNewSizeX = (uint32)(this->w * osFactor);
		uint32 nNewSizeY = (uint32)(this->h * osFactor);
		vector<uint8> vImgTemp;
		vector<SLMPixel> vImgTemp2;
		uint32 i, j, k;

		// Reduce the color
		vImgTemp2.resize( nNewSizeX * nNewSizeY );
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			for( j = 0; j < 8; ++j )
			{ vImgTemp2[i].p[j].R = vImgTemp2[i].p[j].G = vImgTemp2[i].p[j].B = vImgTemp2[i].p[j].A = 0.0f; }

		vImgTemp.resize( nNewSizeX * nNewSizeY );
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			vImgTemp[i] = 0;

		double dx, dy, x, y;
		if( osFactor > 1.0 ) // Up
		{
			dx = 1.0/osFactor;
			dy = 1.0/osFactor;
			y = 0.0;
			for( j = 0; j < nNewSizeY; ++j )
			{
				x = 0.0;
				for( i = 0; i < nNewSizeX; ++i )
				{
					if( this->msk[((sint32)x)+((sint32)y)*this->w] != 0 )
					{
						vImgTemp[i+j*nNewSizeX] = 1;
					}
					for( k = 0; k < 8; ++k )
					{
						vImgTemp2[i+j*nNewSizeX].p[k].R += this->col[((sint32)x)+((sint32)y)*this->w].p[k].R;
						vImgTemp2[i+j*nNewSizeX].p[k].G += this->col[((sint32)x)+((sint32)y)*this->w].p[k].G;
						vImgTemp2[i+j*nNewSizeX].p[k].B += this->col[((sint32)x)+((sint32)y)*this->w].p[k].B;
						vImgTemp2[i+j*nNewSizeX].p[k].A += 1.0f;
					}
					x += dx;
				}
				y += dy;
			}
		}
		else // Down
		{
			dx = osFactor;
			dy = osFactor;
			y = 0.0;
			for( j = 0; j < this->h; ++j )
			{
				x = 0.0;
				for( i = 0; i < this->w; ++i )
				{
					if( this->msk[i+j*this->w] != 0 )
					{
						vImgTemp[((sint32)x)+((sint32)y)*nNewSizeX] = 1;
					}
					for( k = 0; k < 8; ++k )
					{
						vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].R += this->col[i+j*this->w].p[k].R;
						vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].G += this->col[i+j*this->w].p[k].G;
						vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].B += this->col[i+j*this->w].p[k].B;
						vImgTemp2[((sint32)x)+((sint32)y)*nNewSizeX].p[k].A += 1.0f;
					}
					x += dx;
				}
				y += dy;
			}
		}

		for( j = 0; j < nNewSizeY; ++j )
		for( i = 0; i < nNewSizeX; ++i )
		for( k = 0; k < 8; ++k )
		if( vImgTemp2[i+j*nNewSizeX].p[k].A > 1.0f )
		{
			vImgTemp2[i+j*nNewSizeX].p[k].R /= vImgTemp2[i+j*nNewSizeX].p[k].A;
			vImgTemp2[i+j*nNewSizeX].p[k].G /= vImgTemp2[i+j*nNewSizeX].p[k].A;
			vImgTemp2[i+j*nNewSizeX].p[k].B /= vImgTemp2[i+j*nNewSizeX].p[k].A;
			vImgTemp2[i+j*nNewSizeX].p[k].A = 1.0f;			
		}

		this->col.resize( nNewSizeX * nNewSizeY );
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			this->col[i] = vImgTemp2[i];

		this->msk.resize( nNewSizeX * nNewSizeY );
		for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
			this->msk[i] = vImgTemp[i];

		this->w = nNewSizeX;
		this->h = nNewSizeY;
		this->x = (sint32)(this->x * osFactor);
		this->y = (sint32)(this->y * osFactor);
	}
};

// ***********************************************************************************************
struct SGradient
{
	// Vertex gradient
	double InitPx, InitPy, InitPz;
	double GraduPx, GradvPx;
	double GraduPy, GradvPy;
	double GraduPz, GradvPz;
	// Normal gradient
	double InitNx, InitNy, InitNz;
	double GraduNx, GradvNx;
	double GraduNy, GradvNy;
	double GraduNz, GradvNz;
	// Color gradient
	double InitR, InitG, InitB;
	double GraduR, GradvR;
	double GraduG, GradvG;
	double GraduB, GradvB;
	// Initial u,v
	double InitU, InitV;

	// -----------------------------------------------------------------------------------------------
	void init( CMesh::CFace *pF, vector<CVector>& vVertices, CVector &n1, CVector &n2, CVector &n3 )
	{
		double	u1 = pF->Corner[0].Uvs[1].U,
				v1 = pF->Corner[0].Uvs[1].V,
				u2 = pF->Corner[1].Uvs[1].U,
				v2 = pF->Corner[1].Uvs[1].V,
				u3 = pF->Corner[2].Uvs[1].U,
				v3 = pF->Corner[2].Uvs[1].V;
		CVector p1 = vVertices[pF->Corner[0].Vertex],
				p2 = vVertices[pF->Corner[1].Vertex],
				p3 = vVertices[pF->Corner[2].Vertex];

		CRGBA	c1 = pF->Corner[0].Color,
				c2 = pF->Corner[1].Color,
				c3 = pF->Corner[2].Color;

		double GradDen = 1.0 / ( (u3-u1)*(v2-v1) - (u2-u1)*(v3-v1) );

		this->InitU = u1;
		this->InitV = v1;

		this->InitPx = p1.x;
		this->InitPy = p1.y;
		this->InitPz = p1.z;

		this->InitNx = n1.x;
		this->InitNy = n1.y;
		this->InitNz = n1.z;

		this->InitR = c1.R;
		this->InitG = c1.G;
		this->InitB = c1.B;

		// Gradients for the vertex

		this->GraduPx = ( (p3.x-p1.x)*(v2-v1)-(p2.x-p1.x)*(v3-v1) ) * GradDen;
		this->GradvPx = ( (p2.x-p1.x)*(u3-u1)-(p3.x-p1.x)*(u2-u1) ) * GradDen;

		this->GraduPy = ( (p3.y-p1.y)*(v2-v1)-(p2.y-p1.y)*(v3-v1) ) * GradDen;
		this->GradvPy = ( (p2.y-p1.y)*(u3-u1)-(p3.y-p1.y)*(u2-u1) ) * GradDen;

		this->GraduPz = ( (p3.z-p1.z)*(v2-v1)-(p2.z-p1.z)*(v3-v1) ) * GradDen;
		this->GradvPz = ( (p2.z-p1.z)*(u3-u1)-(p3.z-p1.z)*(u2-u1) ) * GradDen;

		// The same for the normal
						
		this->GraduNx = ( (n3.x-n1.x)*(v2-v1)-(n2.x-n1.x)*(v3-v1) ) * GradDen;
		this->GradvNx = ( (n2.x-n1.x)*(u3-u1)-(n3.x-n1.x)*(u2-u1) ) * GradDen;

		this->GraduNy = ( (n3.y-n1.y)*(v2-v1)-(n2.y-n1.y)*(v3-v1) ) * GradDen;
		this->GradvNy = ( (n2.y-n1.y)*(u3-u1)-(n3.y-n1.y)*(u2-u1) ) * GradDen;

		this->GraduNz = ( (n3.z-n1.z)*(v2-v1)-(n2.z-n1.z)*(v3-v1) ) * GradDen;
		this->GradvNz = ( (n2.z-n1.z)*(u3-u1)-(n3.z-n1.z)*(u2-u1) ) * GradDen;

		// The same for the color

		this->GraduR = ( (c3.R-c1.R)*(v2-v1)-(c2.R-c1.R)*(v3-v1) ) * GradDen;
		this->GradvR = ( (c2.R-c1.R)*(u3-u1)-(c3.R-c1.R)*(u2-u1) ) * GradDen;

		this->GraduG = ( (c3.G-c1.G)*(v2-v1)-(c2.G-c1.G)*(v3-v1) ) * GradDen;
		this->GradvG = ( (c2.G-c1.G)*(u3-u1)-(c3.G-c1.G)*(u2-u1) ) * GradDen;

		this->GraduB = ( (c3.B-c1.B)*(v2-v1)-(c2.B-c1.B)*(v3-v1) ) * GradDen;
		this->GradvB = ( (c2.B-c1.B)*(u3-u1)-(c3.B-c1.B)*(u2-u1) ) * GradDen;
		
	}

	// -----------------------------------------------------------------------------------------------
	CVector getInterpolatedVertex( double u, double v )
	{
		CVector vRet;
		vRet.x = (float)(this->GraduPx*(u-this->InitU) + this->GradvPx*(v-this->InitV) + this->InitPx);
		vRet.y = (float)(this->GraduPy*(u-this->InitU) + this->GradvPy*(v-this->InitV) + this->InitPy);
		vRet.z = (float)(this->GraduPz*(u-this->InitU) + this->GradvPz*(v-this->InitV) + this->InitPz);
		return vRet;
	}

	// -----------------------------------------------------------------------------------------------
	CVector getInterpolatedNormal( double u, double v )
	{
		CVector vRet;
		vRet.x = (float)(this->GraduNx*(u-this->InitU) + this->GradvNx*(v-this->InitV) + this->InitNx);
		vRet.y = (float)(this->GraduNy*(u-this->InitU) + this->GradvNy*(v-this->InitV) + this->InitNy);
		vRet.z = (float)(this->GraduNz*(u-this->InitU) + this->GradvNz*(v-this->InitV) + this->InitNz);
		vRet.normalize();
		return vRet;
	}

	// -----------------------------------------------------------------------------------------------
	CRGBAF getInterpolatedColor( double u, double v )
	{
		CRGBAF vRet;
		vRet.R = (float)(this->GraduR*(u-this->InitU) + this->GradvR*(v-this->InitV) + this->InitR);
		vRet.G = (float)(this->GraduG*(u-this->InitU) + this->GradvG*(v-this->InitV) + this->InitG);
		vRet.B = (float)(this->GraduB*(u-this->InitU) + this->GradvB*(v-this->InitV) + this->InitB);
		return vRet;
	}

	// -----------------------------------------------------------------------------------------------
	// Uin and Vin are out of the face pF so calculate a U,V in face
	CVector getInterpolatedVertexInFace( double Uin, double Vin, CMesh::CFace *pF )
	{
		double Uout, Vout;
		double Utmp, Vtmp;
		double u1 = pF->Corner[0].Uvs[1].U, v1 = pF->Corner[0].Uvs[1].V;
		double u2 = pF->Corner[1].Uvs[1].U, v2 = pF->Corner[1].Uvs[1].V;
		double u3 = pF->Corner[2].Uvs[1].U, v3 = pF->Corner[2].Uvs[1].V;
		double rDist = 10000000.0f, rDistTmp, factor;
		// Get the nearest point from (Uin,Vin) to the face pF
		rDistTmp = sqrt( (Uin-u1)*(Uin-u1) + (Vin-v1)*(Vin-v1) );
		if( rDistTmp < rDist )
		{
			rDist = rDistTmp;
			Uout = u1;	Vout = v1;
		}
		rDistTmp = sqrt( (Uin-u2)*(Uin-u2) + (Vin-v2)*(Vin-v2) );
		if( rDistTmp < rDist )
		{
			rDist = rDistTmp;
			Uout = u2;	Vout = v2;
		}
		rDistTmp = sqrt( (Uin-u3)*(Uin-u3) + (Vin-v3)*(Vin-v3) );
		if( rDistTmp < rDist )
		{
			rDist = rDistTmp;
			Uout = u3;	Vout = v3;
		}

		factor = ( (Uin-u1)*(u2-u1) + (Vin-v1)*(v2-v1) ) / ( (u2-u1)*(u2-u1) + (v2-v1)*(v2-v1) );
		if( ( factor >= 0.0 ) && ( factor <= 1.0 ) )
		{
			Utmp = u1+(u2-u1)*factor; Vtmp = v1+(v2-v1)*factor;
			rDistTmp = sqrt( (Uin-Utmp)*(Uin-Utmp) + (Vin-Vtmp)*(Vin-Vtmp) );
			if( rDistTmp < rDist )
			{ rDist = rDistTmp; Uout = Utmp; Vout = Vtmp; }
		}

		factor = ( (Uin-u2)*(u3-u2) + (Vin-v2)*(v3-v2) ) / ( (u3-u2)*(u3-u2) + (v3-v2)*(v3-v2) );
		if( ( factor >= 0.0 ) && ( factor <= 1.0 ) )
		{
			Utmp = u2+(u3-u2)*factor; Vtmp = v2+(v3-v2)*factor;
			rDistTmp = sqrt( (Uin-Utmp)*(Uin-Utmp) + (Vin-Vtmp)*(Vin-Vtmp) );
			if( rDistTmp < rDist )
			{ rDist = rDistTmp; Uout = Utmp; Vout = Vtmp; }
		}

		factor = ( (Uin-u3)*(u1-u3) + (Vin-v3)*(v1-v3) ) / ( (u1-u3)*(u1-u3) + (v1-v3)*(v1-v3) );
		if( ( factor >= 0.0 ) && ( factor <= 1.0 ) )
		{
			Utmp = u3+(u1-u3)*factor; Vtmp = v3+(v1-v3)*factor;
			rDistTmp = sqrt( (Uin-Utmp)*(Uin-Utmp) + (Vin-Vtmp)*(Vin-Vtmp) );
			if( rDistTmp < rDist )
			{ rDist = rDistTmp; Uout = Utmp; Vout = Vtmp; }
		}

		// Calculate the 3d point
		return getInterpolatedVertex( Uout, Vout );
	}

};

// ***********************************************************************************************
// An element of the cube grid
struct SCubeGridCell
{
	CMesh::CFace* pF;
	CMesh::CMeshBuild* pMB;
};

// ***********************************************************************************************
// Represent a cube made of grids centered on (0,0,0) with a size of 1
class SCubeGrid
{
	enum gridPos { kUp = 0, kDown, kLeft, kRight, kFront, kBack };
	CQuadGrid<SCubeGridCell> grids[6];

	sint32 nSelGrid;
	CQuadGrid<SCubeGridCell>::CIterator itSel;

public:

	// -----------------------------------------------------------------------------------------------
	void project( CTriangle &tri, CPlane pyr[4], CPlane &gridPlane, 
					sint32 nGridNb, SCubeGridCell &cell )
	{
		CVector vIn[7], vOut[7];
		sint32 i, nOut;
		vIn[0] = tri.V0; vIn[1] = tri.V1; vIn[2] = tri.V2;
		nOut = pyr[0].clipPolygonFront( vIn, vOut, 3 );
		if( nOut == 0 ) return;
		for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
		nOut = pyr[1].clipPolygonFront( vIn, vOut, nOut );
		if( nOut == 0 ) return;
		for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
		nOut = pyr[2].clipPolygonFront( vIn, vOut, nOut );
		if( nOut == 0 ) return;
		for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
		nOut = pyr[3].clipPolygonFront( vIn, vOut, nOut );
		if( nOut >= 3 )
		{
			CVector vMin(1,1,1), vMax(-1,-1,-1);
			for( i = 0; i < nOut; ++i )
			{
				vOut[i] = gridPlane.intersect( CVector(0,0,0), vOut[i] );
				if( vMin.x > vOut[i].x ) vMin.x = vOut[i].x;
				if( vMin.y > vOut[i].y ) vMin.y = vOut[i].y;
				if( vMin.z > vOut[i].z ) vMin.z = vOut[i].z;
				if( vMax.x < vOut[i].x ) vMax.x = vOut[i].x;
				if( vMax.y < vOut[i].y ) vMax.y = vOut[i].y;
				if( vMax.z < vOut[i].z ) vMax.z = vOut[i].z;
			}
			// Create the bbox
			grids[nGridNb].insert( vMin, vMax, cell );
		}
	}

public :
	
	// -----------------------------------------------------------------------------------------------
	SCubeGrid()
	{
		CMatrix	tmp;
		CVector	I, J, K;

		// grids[kUp].changeBase(  );
		I = CVector(  1,  0,  0 );
		J = CVector(  0, -1,  0 );
		K = CVector(  0,  0, -1 );
		tmp.identity(); tmp.setRot( I, J, K, true );
		grids[kDown].changeBase( tmp );

		I = CVector(  0,  0,  1 );
		J = CVector(  0,  1,  0 );
		K = CVector( -1,  0,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kLeft].changeBase( tmp );

		I = CVector(  0,  0, -1 );
		J = CVector(  0,  1,  0 );
		K = CVector(  1,  0,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kRight].changeBase( tmp );

		I = CVector(  1,  0,  0 );
		J = CVector(  0,  0,  1 );
		K = CVector(  0, -1,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kFront].changeBase( tmp );

		I = CVector(  1,  0,  0 );
		J = CVector(  0,  0, -1 );
		K = CVector(  0,  1,  0 );
		tmp.identity(); tmp.setRot( I, J, K, true);
		grids[kBack].changeBase( tmp );
	}

	// -----------------------------------------------------------------------------------------------
	void create( int nSize )
	{
		grids[kUp].create	( nSize, 1.0f / ((float)nSize) );
		grids[kDown].create	( nSize, 1.0f / ((float)nSize) );
		grids[kLeft].create	( nSize, 1.0f / ((float)nSize) );
		grids[kRight].create( nSize, 1.0f / ((float)nSize) );
		grids[kFront].create( nSize, 1.0f / ((float)nSize) );
		grids[kBack].create	( nSize, 1.0f / ((float)nSize) );
	}

	// -----------------------------------------------------------------------------------------------
	void insert( CTriangle &tri, SCubeGridCell &cell )
	{
		CPlane p[4], gp;
		// Construct clip pyramid for grid : UP
		p[0].make( CVector(0,0,0), CVector( -1,-1,+1 ), CVector( +1,-1,+1 ) );
		p[1].make( CVector(0,0,0), CVector( +1,-1,+1 ), CVector( +1,+1,+1 ) );
		p[2].make( CVector(0,0,0), CVector( +1,+1,+1 ), CVector( -1,+1,+1 ) );
		p[3].make( CVector(0,0,0), CVector( -1,+1,+1 ), CVector( -1,-1,+1 ) );
		gp.make( CVector(0,0,1), CVector(0,0,0.5) );
		project( tri, p, gp, kUp, cell );
		// Construct clip pyramid for grid : DOWN
		p[0].make( CVector(0,0,0), CVector( +1,-1,-1 ), CVector( -1,-1,-1 ) );
		p[1].make( CVector(0,0,0), CVector( -1,-1,-1 ), CVector( -1,+1,-1 ) );
		p[2].make( CVector(0,0,0), CVector( -1,+1,-1 ), CVector( +1,+1,-1 ) );
		p[3].make( CVector(0,0,0), CVector( +1,+1,-1 ), CVector( +1,-1,-1 ) );
		gp.make( CVector(0,0,-1), CVector(0,0,-0.5) );
		project( tri, p, gp, kDown, cell );
		// Construct clip pyramid for grid : LEFT
		p[0].make( CVector(0,0,0), CVector( -1,-1,-1 ), CVector( -1,-1,+1 ) );
		p[1].make( CVector(0,0,0), CVector( -1,-1,+1 ), CVector( -1,+1,+1 ) );
		p[2].make( CVector(0,0,0), CVector( -1,+1,+1 ), CVector( -1,+1,-1 ) );
		p[3].make( CVector(0,0,0), CVector( -1,+1,-1 ), CVector( -1,-1,-1 ) );
		gp.make( CVector(-1,0,0), CVector(-0.5,0,0) );
		project( tri, p, gp, kLeft, cell );
		// Construct clip pyramid for grid : RIGHT
		p[0].make( CVector(0,0,0), CVector( +1,-1,+1 ), CVector( +1,-1,-1 ) );
		p[1].make( CVector(0,0,0), CVector( +1,-1,-1 ), CVector( +1,+1,-1 ) );
		p[2].make( CVector(0,0,0), CVector( +1,+1,-1 ), CVector( +1,+1,+1 ) );
		p[3].make( CVector(0,0,0), CVector( +1,+1,+1 ), CVector( +1,-1,+1 ) );
		gp.make( CVector(1,0,0), CVector(0.5,0,0) );
		project( tri, p, gp, kRight, cell );
		// Construct clip pyramid for grid : FRONT
		p[0].make( CVector(0,0,0), CVector( -1,-1,-1 ), CVector( +1,-1,-1 ) );
		p[1].make( CVector(0,0,0), CVector( +1,-1,-1 ), CVector( +1,-1,+1 ) );
		p[2].make( CVector(0,0,0), CVector( +1,-1,+1 ), CVector( -1,-1,+1 ) );
		p[3].make( CVector(0,0,0), CVector( -1,-1,+1 ), CVector( -1,-1,-1 ) );
		gp.make( CVector(0,-1,0), CVector(0,-0.5,0) );
		project( tri, p, gp, kFront, cell );
		// Construct clip pyramid for grid : BACK
		p[0].make( CVector(0,0,0), CVector( +1,+1,+1 ), CVector( +1,+1,-1 ) );
		p[1].make( CVector(0,0,0), CVector( +1,+1,-1 ), CVector( -1,+1,-1 ) );
		p[2].make( CVector(0,0,0), CVector( -1,+1,-1 ), CVector( -1,+1,+1 ) );
		p[3].make( CVector(0,0,0), CVector( -1,+1,+1 ), CVector( +1,+1,+1 ) );
		gp.make( CVector(0,1,0), CVector(0,0.5,0) );
		project( tri, p, gp, kBack, cell );
	}

	// -----------------------------------------------------------------------------------------------
	// Select the square of one of the 6 grids which is intersected by the 
	// following ray : (0,0,0) -> v
	void select( CVector &v )
	{
		CPlane gp;
		// Get the plane
		if( ( -v.z <= v.x ) && ( v.x <= v.z ) &&
			( -v.z <= v.y ) && ( v.y <= v.z ) &&
			( 0.0f <= v.z ) )
		{
			nSelGrid = kUp;
			gp.make( CVector(0,0,1), CVector(0,0,0.5) );
		}
		if( ( v.z <= v.x ) && ( v.x <= -v.z ) &&
			( v.z <= v.y ) && ( v.y <= -v.z ) &&
			( v.z <= 0.0f ) )
		{
			nSelGrid = kDown;
			gp.make( CVector(0,0,-1), CVector(0,0,-0.5) );
		}
		if( ( v.x <= 0.0f ) &&
			( v.x <= v.y ) && ( v.y <= -v.x ) &&
			( v.x <= v.z ) && ( v.z <= -v.x ) )
		{
			nSelGrid = kLeft;
			gp.make( CVector(-1,0,0), CVector(-0.5,0,0) );
		}
		if( ( 0.0f <= v.x ) &&
			( -v.x <= v.y ) && ( v.y <= v.x ) &&
			( -v.x <= v.z ) && ( v.z <= v.x ) )
		{
			nSelGrid = kRight;
			gp.make( CVector(1,0,0), CVector(0.5,0,0) );
		}
		if( ( v.y <= v.x ) && ( v.x <= -v.y ) &&
			( v.y <= 0.0f ) &&
			( v.y <= v.z ) && ( v.z <= -v.y ) )
		{
			nSelGrid = kFront;
		gp.make( CVector(0,-1,0), CVector(0,-0.5,0) );
		}
		if( ( -v.y <= v.x ) && ( v.x <= v.y ) &&
			( 0.0f <= v.y ) &&
			( -v.y <= v.z ) && ( v.z <= v.y ) )
		{
			nSelGrid = kBack;
			gp.make( CVector(0,1,0), CVector(0,0.5,0) );
		}
		nlassert(nSelGrid!=-1);
		CVector newV = gp.intersect( CVector(0,0,0), v );
		grids[nSelGrid].select(newV, newV);
		itSel = grids[nSelGrid].begin();
	}

	// -----------------------------------------------------------------------------------------------
	SCubeGridCell getSel()
	{
		return *itSel;
	}

	// -----------------------------------------------------------------------------------------------
	void nextSel()
	{
		++itSel;
	}

	// -----------------------------------------------------------------------------------------------
	// To call after a nextSel to test if this is the end of the selection
	bool isEndSel()
	{
		return (itSel == grids[nSelGrid].end());
	}
};

// ***********************************************************************************************
struct SWorldRT
{
	vector<CMesh::CMeshBuild *> vMB;
	vector<INode *>				vINode;
	
	vector<SCubeGrid> cgAccel; // One cube grid by light
	vector<CBitmap> proj; // One projector by light
};

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

TTicks timerCalcRT = 0;
TTicks timerExportLighting = 0;
TTicks timerInit = 0;
TTicks timerCalc = 0;
TTicks timerPlac = 0;
TTicks timerSave = 0;

CExportNelOptions gOptions;

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
void SortFaceByTextureName(vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB)
{
	int i, j;
	int nNbFace = AllFaces.size();

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	{
		ITexture *pT = pMB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		pT = pMB->Materials[AllFaces[j]->MaterialId].getTexture(0);
		pTF = dynamic_cast<CTextureFile *>(pT);
		string namej = "Default";
		if( pTF != NULL )
			namej = pTF->getFileName();
		if( namei < namej )
		{
			CMesh::CFace *pFaceTemp = AllFaces[i];
			AllFaces[i] = AllFaces[j];
			AllFaces[j] = pFaceTemp;
		}
	}
}

// -----------------------------------------------------------------------------------------------
// TextureNames is an array which indicates the number of faces that follows which have the same texture name
void ComputeAreaOfTextureName(vector<sint32> &TextureNames, vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB)
{
	int i, nNbFace = AllFaces.size();
	TextureNames.resize(nNbFace);
	ITexture *pT = pMB->Materials[AllFaces[0]->MaterialId].getTexture(0);
	CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
	string CurrentName = "Default";
	sint32 lastface = 0, nNbTexName = 0;
	if( pTF != NULL )
		CurrentName = pTF->getFileName();
	for( i = 0; i < nNbFace; ++i )
	{
		ITexture *pT = pMB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		if( ( namei != CurrentName ) || ( i == (nNbFace-1) ) )
		{
			CurrentName = namei;
			TextureNames[nNbTexName] = i-lastface;
			nNbTexName++;
			lastface = i;
		}
	}
	TextureNames[nNbTexName-1] += 1;
	TextureNames.resize( nNbTexName );
}

void ClearFaceWithNoLM( CMesh::CMeshBuild *pMB, vector<CMesh::CFace*> &ZeFaces )
{
	sint32 i;
	vector<CMesh::CFace*>::iterator ItParseI = ZeFaces.begin();
	sint32 nNbFace = ZeFaces.size();
	
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = *ItParseI;
		if( pMB->Materials[pF->MaterialId].getShader() != CMaterial::TShader::LightMap )
		{
			ItParseI = ZeFaces.erase( ItParseI );
			nNbFace--;
			i--;
		}
		else
		{
			++ItParseI;
		}
	}
}


// -----------------------------------------------------------------------------------------------
void SortFaceByMaterialId(  vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFaces, sint32 nNbFace )
{
	int i, j;
	sint32 nMatID;

	// Bubble sort face

	vector<CMesh::CFace*>::iterator ItParseI = ItFaces;
	for( i = 0; i < nNbFace-1; ++i )
	{
		vector<CMesh::CFace*>::iterator ItParseJ = ItParseI;
		++ItParseJ;
		for( j = i+1; j < nNbFace; ++j )
		{
			if( (*ItParseI)->MaterialId < (*ItParseJ)->MaterialId )
			{
				CMesh::CFace *pFaceTemp = *ItParseI;
				*ItParseI = *ItParseJ;
				*ItParseJ = pFaceTemp;
			}
			++ItParseJ;
		}
		++ItParseI;
	}

	// Indicates the groups
	
	FaceGroup.resize( nNbFace );
	ItParseI = ItFaces;
	j = 0; nMatID = (*ItParseI)->MaterialId; ++ItParseI;
	FaceGroup[j] = 1;
	for( i = 1; i < nNbFace; ++i )
	{
		if( (*ItParseI)->MaterialId != nMatID )
		{
			nMatID = (*ItParseI)->MaterialId; ++j;
			FaceGroup[j] = 1;
		}
		else
		{
			FaceGroup[j] ++;
		}
		++ItParseI;
	}
	FaceGroup.resize( j+1 );
}

// -----------------------------------------------------------------------------------------------
// Test if the 2 faces are continuous (same vertex, same normal (if wanted), same uv (if wanted))
bool FaceContinuous( CMesh::CFace *pF1, CMesh::CFace *pF2, bool bTestUV = true, bool bTestNormal = true )
{
	sint32 i, j;
	sint32 F1c[2] = { -1, -1 };
	sint32 F2c[2] = { -1, -1 };

	// Is there a vertices continuity
	for( j = 0; j < 3; ++j )
	for( i = 0; i < 3; ++i )
	{
		if( (pF1->Corner[j].Vertex == pF2->Corner[i].Vertex) && 
			(pF1->Corner[(j+1)%3].Vertex == pF2->Corner[(i+1)%3].Vertex) )
		{
			F1c[0] = j; F1c[1] = (j+1)%3;
			F2c[0] = i; F2c[1] = (i+1)%3;
		}
		if( (pF1->Corner[j].Vertex == pF2->Corner[(i+1)%3].Vertex) && 
			(pF1->Corner[(j+1)%3].Vertex == pF2->Corner[i].Vertex) )
		{
			F1c[0] = (j+1)%3; F1c[1] = j;
			F2c[0] = i;		  F2c[1] = (i+1)%3;
		}
	}
	// No -> out
	if( F1c[0] == -1 ) 
		return false;
	// Here we get the vertex continuity between F1c[0] and F2c[0], and, F1c[1] and F2c[1]
	// Is there a normal continuity
	if( bTestNormal )
	for( i = 0; i < 2; ++i )
	{
		CVector n1 = pF1->Corner[F1c[i]].Normal;
		CVector n2 = pF2->Corner[F2c[i]].Normal;
		// is n1 equal to n2 ?
		double epsilon = 1.0 - (n1*n2); // theorically n1*n2 equal to 1.0 but epsilon error
		if( epsilon > 0.001 )
			return false;
	}
	// Is there a mapping continuity
	if( bTestUV )
	for( i = 0; i < 2; ++i )
	{
		if((fabs( pF1->Corner[F1c[i]].Uvs[1].U - pF2->Corner[F2c[i]].Uvs[1].U) > 0.001) ||
		   (fabs( pF1->Corner[F1c[i]].Uvs[1].V - pF2->Corner[F2c[i]].Uvs[1].V) > 0.001) )
		   return false;
	}
	return true;
}

// -----------------------------------------------------------------------------------------------
void SortFaceBySMoothGroup( vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFaces, sint32 nNbFace )
{
	sint32 j, k, nGroupNb = 0, nGroupOffset = 1;
	bool bFaceAdded;

	// Bubble sort face
	FaceGroup.resize(nNbFace);
	for( j = 0; j < nNbFace; ++j )
		FaceGroup[j] = 1;
	vector<CMesh::CFace*>::iterator CurGrpBeg = ItFaces;
	vector<CMesh::CFace*>::iterator CurGrpEnd = ItFaces;

	for( nGroupOffset = 1; nGroupOffset <= nNbFace; )
	{
		do 
		{
			bFaceAdded = false;
			vector<CMesh::CFace*>::iterator ItParseJ = CurGrpEnd;
			++ItParseJ;
			for( j = nGroupOffset; j < nNbFace; ++j )
			{
				// Is the face is connected to one of the current group
				vector<CMesh::CFace*>::iterator ItParseK = CurGrpBeg;
				for( k = 0; k < FaceGroup[nGroupNb]; ++k )
				{
					if( FaceContinuous( *ItParseK, *ItParseJ, false ) )
					{
						// Yes the face must be added at the end of the group
						++CurGrpEnd;
						CMesh::CFace *pFaceTemp = *CurGrpEnd;
						*CurGrpEnd = *ItParseJ;
						*ItParseJ = pFaceTemp;
						nGroupOffset += 1;
						FaceGroup[nGroupNb] += 1;
						bFaceAdded = true;
						break;
					}
					++ItParseK;
				}
				++ItParseJ;
			}
		} while( bFaceAdded ); // In this pass have we added faces ?
		// No -> Next smooth group
		++CurGrpEnd;
		CurGrpBeg = CurGrpEnd;
		++nGroupNb;
		nGroupOffset += 1;
	}
	FaceGroup.resize(nGroupNb);
}


// -----------------------------------------------------------------------------------------------
void SortFaceByTextureSurface( int offset, int nNbFace, vector<CMesh::CFace*> &AllFaces )
{
	int i, j;

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	if( AllFaces[i]->MaterialId == AllFaces[j]->MaterialId )
	{
		// Texture surface of the i face = .5*|(u1-u0)*(v2-v0)-(v1-v0)*(u2-u0)|
		// in fact this is lightmap mapping surface
		double surfacei = 0.5*fabs(
		(AllFaces[i]->Corner[1].Uvs[1].U - AllFaces[i]->Corner[0].Uvs[1].U)*
		(AllFaces[i]->Corner[2].Uvs[1].V - AllFaces[i]->Corner[0].Uvs[1].V)-
		(AllFaces[i]->Corner[1].Uvs[1].V - AllFaces[i]->Corner[0].Uvs[1].V)*
		(AllFaces[i]->Corner[2].Uvs[1].U - AllFaces[i]->Corner[0].Uvs[1].U) );
		double surfacej = 0.5*fabs(
		(AllFaces[j]->Corner[1].Uvs[1].U - AllFaces[j]->Corner[0].Uvs[1].U)*
		(AllFaces[j]->Corner[2].Uvs[1].V - AllFaces[j]->Corner[0].Uvs[1].V)-
		(AllFaces[j]->Corner[1].Uvs[1].V - AllFaces[j]->Corner[0].Uvs[1].V)*
		(AllFaces[j]->Corner[2].Uvs[1].U - AllFaces[j]->Corner[0].Uvs[1].U) );
		if( surfacei < surfacej )
		{
			CMesh::CFace *pFaceTemp = AllFaces[i];
			AllFaces[i] = AllFaces[j];
			AllFaces[j] = pFaceTemp;
		}
	}
}

// -----------------------------------------------------------------------------------------------
bool isInTriangleOrEdge(double x, double y, double xt1, double yt1, double xt2, double yt2, double xt3, double yt3)
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}

// -----------------------------------------------------------------------------------------------
bool isInTriangle(double x, double y, double xt1, double yt1, double xt2, double yt2, double xt3, double yt3)
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 < 0.0)&&(sign2 < 0.0)&&(sign3 < 0.0) )
		return true;
	if( (sign1 > 0.0)&&(sign2 > 0.0)&&(sign3 > 0.0) )
		return true;
	return false;
}

// Segment line intersection P1P2 and P3P4
// -----------------------------------------------------------------------------------------------
bool segmentIntersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double denominator = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
	if( denominator == 0.0 )
		return false; // The segment are colinear
	double k = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	k = ( (x2-x1)*(y1-y3) - (y2-y1)*(x1-x3) ) / denominator;
	if( (k<=0.0) || (k>=1.0) ) return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
bool intersectionTriangleSphere( CTriangle &t, CBSphere &s )
{
	// if a vertex of the triangle is in the sphere
	CVector v = t.V0 - s.Center;
	float f = v.norm();
	if( f < s.Radius )
		return true;
	v = t.V1 - s.Center;
	f = v.norm();
	if( f < s.Radius )
		return true;
	v = t.V2 - s.Center;
	f = v.norm();
	if( f < s.Radius )
		return true;
	// Ok sonow project the center of the triangle on the plane
	CPlane p;
	p.make( t.V0, t.V1, t.V2 );
	p.normalize();
	
	CVector newCenter = p.project( s.Center );
	v = newCenter - s.Center;
	float newRadius = v.norm() / s.Radius;
	if( newRadius > 1.0 )
		newRadius = 1.0;
	newRadius = cosf( newRadius * PI / 2.0f );

	CVector n = p.getNormal();
	CPlane p2;
	p2.make( t.V0, t.V1, t.V0 + n ); p2.normalize();
	f = p2*newCenter;
	p2.make( t.V1, t.V2, t.V1 + n ); p2.normalize();
	float f2 = p2*newCenter;
	p2.make( t.V2, t.V0, t.V2 + n ); p2.normalize();
	float f3 = p2*newCenter;

	// Is the newcenter insied the triangle ?
	if( ( f <= 0.0 ) && ( f2 <= 0.0 ) && ( f3 <= 0.0 ) )
		return true;
	if( ( f >= 0.0 ) && ( f2 >= 0.0 ) && ( f3 >= 0.0 ) )
		return true;

	// Is the newCenter at a distance < newradius from one of the triangle edge ?
	if( ( fabs(f) < newRadius ) || ( fabs(f2) < newRadius ) || ( fabs(f3) < newRadius ) )
		return true;
	return false;
}

// -----------------------------------------------------------------------------------------------
void CreatePiece( vector<uint8>& Piece, sint32& nSizeX, sint32& nSizeY, sint32 &nPosX, sint32 &nPosY,
				 float lumx1, float lumy1,
				 float lumx2, float lumy2, 
				 float lumx3, float lumy3, uint8 nCol )
{
	double minx, miny;
	double maxx, maxy;
	int j,k;

	if( nCol == 0 )
		nCol = 1;
	minx = lumx1;
	if( minx > lumx2 ) minx = lumx2;
	if( minx > lumx3 ) minx = lumx3;
	maxx = lumx1;
	if( maxx < lumx2 ) maxx = lumx2;
	if( maxx < lumx3 ) maxx = lumx3;
	miny = lumy1;
	if( miny > lumy2 ) miny = lumy2;
	if( miny > lumy3 ) miny = lumy3;
	maxy = lumy1;
	if( maxy < lumy2 ) maxy = lumy2;
	if( maxy < lumy3 ) maxy = lumy3;

	// Put the piece in the new basis (nPosX,nPosY)
	nPosX = ((sint32)floor(minx-0.5));
	nPosY = ((sint32)floor(miny-0.5));

	lumx1 -= nPosX; lumy1 -= nPosY;
	lumx2 -= nPosX; lumy2 -= nPosY;
	lumx3 -= nPosX;	lumy3 -= nPosY;

	nSizeX = 1 + ((sint32)floor(maxx+0.5)) - ((sint32)floor(minx-0.5));
	nSizeY = 1 + ((sint32)floor(maxy+0.5)) - ((sint32)floor(miny-0.5));
	Piece.resize( nSizeX*nSizeY );
	for( j = 0; j < nSizeX*nSizeY; ++j )
		Piece[j] = 0;

// The square interact with the triangle if an edge of the square is cut by an edge of the triangle
// Or the square is in the triangle
	
	for( j = 0; j < nSizeY-1; ++j )
	for( k = 0; k < nSizeX-1; ++k )
	{
		// Is the square (j,k) is interacting with the triangle
		// This means : The square contains a point of the triangle (can be done for the 3 points)
		//              The triangle contains a point of the square
		// If so then we have to turn on all the 4 pixels of the square
		if( isInTriangleOrEdge(k+0.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+0.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) )
		{
			Piece[k   + j    *nSizeX] = nCol;
			Piece[1+k + j    *nSizeX] = nCol;
			Piece[k   + (1+j)*nSizeX] = nCol;
			Piece[1+k + (1+j)*nSizeX] = nCol;
		}

		if( segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) )
		{
			Piece[k   + j    *nSizeX] = nCol;
			Piece[1+k + j    *nSizeX] = nCol;
			Piece[k   + (1+j)*nSizeX] = nCol;
			Piece[1+k + (1+j)*nSizeX] = nCol;
		}

	}
	// For all the points of the triangle update the square
	Piece[((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*nSizeX] = nCol;

	Piece[((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*nSizeX] = nCol;
	
	Piece[((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*nSizeX] = nCol;
}

// -----------------------------------------------------------------------------------------------
void ResizeBitmap( vector<uint8> &vBitmap, sint32 &nSizeX, sint32 &nSizeY, sint32 nNewSizeX, sint32 nNewSizeY )
{
	vector<uint8> vImgTemp;
	int i, j;

	vImgTemp.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < min(nSizeY,nNewSizeY); ++j )
	for( i = 0; i < min(nSizeX,nNewSizeX); ++i )
	{
		vImgTemp[i+j*nNewSizeX] = vBitmap[i+j*nSizeX];
	}

	vBitmap.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vBitmap[i] = vImgTemp[i];

	nSizeX = nNewSizeX;
	nSizeY = nNewSizeY;
}

// -----------------------------------------------------------------------------------------------
// Same as ResizeBitmap but for 32 bits image
//void ResizeBitmap32( CBitmap *pImage, sint32 nNewSizeX, sint32 nNewSizeY )
//{
//}

// -----------------------------------------------------------------------------------------------
bool PutPieceInLightMap( vector<uint8>& Piece, sint32 nPieceSizeX, sint32 nPieceSizeY, 
						 vector<uint8>& LightMap, sint32 nLightMapSizeX, sint32 nLightMapSizeY,
						 sint32 &nNewPosX, sint32 &nNewPosY )
{
	sint32 i, j, a, b;
	bool bGoodPosition;

	if( nPieceSizeX > nLightMapSizeX ) return false;
	if( nPieceSizeY > nLightMapSizeY ) return false;

	// For all position test if the piece can be put in
	for( j = 0; j < (nLightMapSizeY-nPieceSizeY); ++j )
	for( i = 0; i < (nLightMapSizeX-nPieceSizeX); ++i )
	{
		bGoodPosition = true;
		for( b = 0; b < nPieceSizeY; ++b )
		{
			for( a = 0; a < nPieceSizeX; ++a )
			{
				if( Piece[a+b*nPieceSizeX] != 0 )
					if( LightMap[(i+a)+(j+b)*nLightMapSizeX] != 0 )
					{
						bGoodPosition = false;
						break;
					}
			}
			if( bGoodPosition == false )
				break;
		}
		if( bGoodPosition )
		{
			// Write the piece in the lightmap !!!
			for( b = 0; b < nPieceSizeY; ++b )
			{
				for( a = 0; a < nPieceSizeX; ++a )
				{
					if( Piece[a+b*nPieceSizeX] != 0 )
						LightMap[(i+a)+(j+b)*nLightMapSizeX] = Piece[a+b*nPieceSizeX];
				}
			}
			nNewPosX = i;
			nNewPosY = j;
			return true;
		}
	}

	return false;
}

// -----------------------------------------------------------------------------------------------
void MapFace( CMesh::CFace *pFace, vector<CVector> &Vertices, float rRatio )
{
	CVector V01 = Vertices[pFace->Corner[1].Vertex] - Vertices[pFace->Corner[0].Vertex];
	CVector V02 = Vertices[pFace->Corner[2].Vertex] - Vertices[pFace->Corner[0].Vertex];
	CVector n = V01 ^ V02;
	n.normalize();
	
	// Quantize the normal

	// Table of unitary vector with relevant direction to map The I vector represent the plane normal
	// and the J,K vector the U,V vector
	CMatrix QuantizationTbl[3]; 
	QuantizationTbl[0].identity();
	QuantizationTbl[1].identity(); QuantizationTbl[1].rotateZ((float)(Pi/2.0));
	QuantizationTbl[2].identity(); QuantizationTbl[2].rotateY((float)(Pi/2.0));

	float fMax = 0.0f;
	int pos = 0;
	for( int i = 0; i < 3; ++i )
	{
		if( fMax < fabsf(QuantizationTbl[i].getI()*n) )
		{
			fMax = fabsf(QuantizationTbl[i].getI()*n);
			pos = i;
		}
	}

	// Map with the i_th vector from the quantization table
	// Projection of the 3 vertices of the triangle on the plane 
	// defined by the quantized vector (as the plane normal) and the origin (as a point in the plane)

	// This is equivalent to a base changement with annulation of the I vector
	CMatrix invMat = QuantizationTbl[pos].inverted();
	CVector newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[0].Vertex]);
	pFace->Corner[0].Uvs[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[0].Uvs[1].V = newPtinUVBasis.z / rRatio;
	newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[1].Vertex]);
	pFace->Corner[1].Uvs[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[1].Uvs[1].V = newPtinUVBasis.z / rRatio;
	newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[2].Vertex]);
	pFace->Corner[2].Uvs[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[2].Uvs[1].V = newPtinUVBasis.z / rRatio;	
}

// -----------------------------------------------------------------------------------------------
CMatrix getObjectToWorldMatrix( CMesh::CMeshBuild *pMB )
{
	CMatrix m1, m2, m3, m4, m5;
	m1.identity();
	m1.setPos( pMB->DefaultPivot );
	m1.invert();

	m2.identity();
	m2.scale( pMB->DefaultScale );

	m3.identity();
	m3.setRot( pMB->DefaultRotQuat );

	m4.identity();
	m4.setPos( pMB->DefaultPivot );

	m5.identity();
	m5.setPos( pMB->DefaultPos );

	m1 = m5*m4*m3*m2*m1;
	
	return m1;
}

// -----------------------------------------------------------------------------------------------
float getUVDist( CUV& UV1, CUV& UV2 )
{
	return sqrtf( (UV2.U - UV1.U)*(UV2.U - UV1.U) + (UV2.V - UV1.V)*(UV2.V - UV1.V) );
}

// -----------------------------------------------------------------------------------------------
void getLightBuildList(std::vector<SLightBuild>& vectLight, TimeValue tvTime, Interface& ip, INode*node=NULL )
{
	if( node == NULL )
		node = ip.GetRootNode();

	SLightBuild nelLight;

	if( nelLight.convertFromMaxLight(node, tvTime))
		vectLight.push_back (nelLight);

	// Recurse sub node
	for (int i=0; i<node->NumberOfChildren(); i++)
		getLightBuildList(vectLight, tvTime, ip, node->GetChildNode(i));
}

// -----------------------------------------------------------------------------------------------
void getLightBuilds( vector<SLightBuild> &lights, TimeValue tvTime, Interface& ip )
{
	SLightBuild amb;

	amb.Type = SLightBuild::EType::LightAmbient;
	amb.GroupName = "GlobalLight";
	amb.Ambient.R = (uint8)(ip.GetAmbient( tvTime, FOREVER ).x*255);
	amb.Ambient.G = (uint8)(ip.GetAmbient( tvTime, FOREVER ).y*255);
	amb.Ambient.B = (uint8)(ip.GetAmbient( tvTime, FOREVER ).z*255);
	amb.Ambient.A = 255;
	amb.Specular = amb.Diffuse = CRGBA(0,0,0,0);
	lights.push_back( amb );
	getLightBuildList( lights, tvTime, ip );

}

// -----------------------------------------------------------------------------------------------
double calculateTriangleSurface( CVector &p1, CVector &p2, CVector &p3 )
{
	CVector n = ((p2-p1)^(p3-p1));
	return 0.5 * n.norm(); // Half of the norm
}

// -----------------------------------------------------------------------------------------------
void MoveFaceUV1( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace, double rOffsU, double rOffsV )
{
	sint32 i,j ;
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = (*ItFace);
		for( j = 0; j < 3; ++j )
		{
			pF->Corner[j].Uvs[1].U += (float)rOffsU;
			pF->Corner[j].Uvs[1].V += (float)rOffsV;
		}
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void MultiplyFaceUV1( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace, double rFactor )
{
	sint32 i,j ;
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = (*ItFace);
		for( j = 0; j < 3; ++j )
		{
			pF->Corner[j].Uvs[1].U *= (float)rFactor;
			pF->Corner[j].Uvs[1].V *= (float)rFactor;
		}
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
bool PutFaceUV1InLumelCoord( double rRatioLightMap, vector<CVector> &Vertices, 
						    vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i, j;
	double SpaceSurf = 0.0, TextureSurf = 0.0;
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		CVector p1, p2, p3;
		CMesh::CFace* pF = (*ItParseI);
		p1 = Vertices[pF->Corner[0].Vertex];
		p2 = Vertices[pF->Corner[1].Vertex];
		p3 = Vertices[pF->Corner[2].Vertex];
		SpaceSurf += calculateTriangleSurface( p1, p2, p3 );
		p1.x = pF->Corner[0].Uvs[1].U; p1.y = pF->Corner[0].Uvs[1].V; p1.z = 0.0f;
		p2.x = pF->Corner[1].Uvs[1].U; p2.y = pF->Corner[1].Uvs[1].V; p2.z = 0.0f;
		p3.x = pF->Corner[2].Uvs[1].U; p3.y = pF->Corner[2].Uvs[1].V; p3.z = 0.0f;
		TextureSurf += calculateTriangleSurface( p1, p2, p3 );
		// Next face
		++ItParseI;
	}
	if( TextureSurf < 0.0001 )
		return false;
	double LMTextRatio = sqrt(SpaceSurf / TextureSurf) * (1.0/rRatioLightMap);

	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace* pF = (*ItParseI);
		for( j = 0; j < 3; ++j ) // Express the UVs in lumel for each corner
		{
			pF->Corner[j].Uvs[1].U *= (float)LMTextRatio;
			pF->Corner[j].Uvs[1].V *= (float)LMTextRatio;
		}
		++ItParseI;
	}
	return true;
}

// -----------------------------------------------------------------------------------------------
void PutFaceUV1InTextureCoord( sint32 TextureSizeX, sint32 TextureSizeY, 
								vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i,j;
	for( i = 0; i < nNbFace; ++i )
	{
		for( j = 0; j < 3; ++j )
		{
			CMesh::CFace *pF = *ItFace;
			pF->Corner[j].Uvs[1].U /= (float)TextureSizeX;
			pF->Corner[j].Uvs[1].V /= (float)TextureSizeY;
		}
		// Next face
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
bool IsFaceCoverFace( CMesh::CFace *pF1, CMesh::CFace *pF2 )
{
	sint32 i, j;

	for( j = 0; j < 3; ++j )
	for( i = 0; i < 3; ++i )
		if( segmentIntersection(pF1->Corner[i].Uvs[1].U, pF1->Corner[i].Uvs[1].V, 
								pF1->Corner[(i+1)%3].Uvs[1].U, pF1->Corner[(i+1)%3].Uvs[1].V, 
								pF2->Corner[j].Uvs[1].U, pF2->Corner[j].Uvs[1].V, 
								pF2->Corner[(j+1)%3].Uvs[1].U, pF2->Corner[(j+1)%3].Uvs[1].V ) )
			return true;

	for( i = 0; i < 3; ++i )
		if( isInTriangle(	pF1->Corner[i].Uvs[1].U, pF1->Corner[i].Uvs[1].V, 
							pF2->Corner[0].Uvs[1].U, pF2->Corner[0].Uvs[1].V, 
							pF2->Corner[1].Uvs[1].U, pF2->Corner[1].Uvs[1].V, 
							pF2->Corner[2].Uvs[1].U, pF2->Corner[2].Uvs[1].V ) )
			return true;

	for( i = 0; i < 3; ++i )
		if( isInTriangle(	pF2->Corner[i].Uvs[1].U, pF2->Corner[i].Uvs[1].V, 
							pF1->Corner[0].Uvs[1].U, pF1->Corner[0].Uvs[1].V, 
							pF1->Corner[1].Uvs[1].U, pF1->Corner[1].Uvs[1].V, 
							pF1->Corner[2].Uvs[1].U, pF1->Corner[2].Uvs[1].V ) )
			return true;

	return false;
}

// -----------------------------------------------------------------------------------------------
void SortFaceByPlane( vector<sint32> &FaceGroup, vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 j, k, nGroupNb = 0;
	FaceGroup.resize( nNbFace );
	for( j = 0; j < nNbFace; ++j )
		FaceGroup[j] = 1;

	vector<CMesh::CFace*>::iterator CurGrpBeg = ItFace;
	vector<CMesh::CFace*>::iterator CurGrpEnd = ItFace;
	sint32 nGroupOffset = 1;
	list<CMesh::CFace*> lifo;
	lifo.clear();
	for( nGroupOffset = 1; nGroupOffset <= nNbFace; )
	{
		lifo.push_front( *CurGrpBeg );
		// Do a complete plane : Graph traversal in width
		while( ! lifo.empty() )
		{
			CMesh::CFace *pFace = lifo.back();
			lifo.pop_back();
			
			vector<CMesh::CFace*>::iterator ItParseJ = CurGrpEnd;
			++ItParseJ;
			for( j = nGroupOffset; j < nNbFace; ++j )		
			{
				if( FaceContinuous( pFace, *ItParseJ ) )
				{
					// Is this face cover other face present in the current group ?
					vector<CMesh::CFace*>::iterator ItParseK = CurGrpBeg;
					bool bFaceCovering = false;
					for( k = 0; k < FaceGroup[nGroupNb]; ++k )
					{
						if( IsFaceCoverFace( *ItParseK, *ItParseJ ) )
						{
							bFaceCovering = true;
							break;
						}
						++ItParseK;
					}

					// The face do not cover other face -> add it to current group
					if( !bFaceCovering )
					{
						lifo.push_front( *ItParseJ );
						++CurGrpEnd;
						CMesh::CFace *pFaceTemp = *CurGrpEnd;
						*CurGrpEnd = *ItParseJ;
						*ItParseJ = pFaceTemp;
						nGroupOffset += 1;
						FaceGroup[nGroupNb] += 1;
					}

				}
				++ItParseJ;
			}
		}
		++CurGrpEnd;
		CurGrpBeg = CurGrpEnd;
		++nGroupNb;
		nGroupOffset += 1;
	}
	FaceGroup.resize( nGroupNb );
}

// -----------------------------------------------------------------------------------------------
void SortPlanesBySurface( vector<SLMPlane*> &planes )
{
	uint32 i, j;

	for( i = 0; i < planes.size()-1; ++i )
	for( j = i+1; j < planes.size(); ++j )
	{
		if( (planes[i]->w *planes[i]->h) < (planes[j]->w *planes[j]->h) )
		{
			SLMPlane *tmp = planes[i];
			planes[i] = planes[j];
			planes[j] = tmp;
		}
	}
}
/* // To keep
void SortPlanesBySurface( vector<sint32> &PlaneGroup, vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFace )
{
	sint32 i, j, k;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;

	CMesh::CFace *pF;
	vector< pair < sint32, sint32 > > sizes;

	sizes.resize( PlaneGroup.size() );
	// Get the size of surface for each plane
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	for( i = 0; i < sizes.size(); ++i )
	{
		rMinU = 1000000.0; rMaxU = -1000000.0;
		rMinV = 1000000.0; rMaxV = -1000000.0;
		for( j = 0; j < PlaneGroup[i]; ++j )
		{
			pF = *ItParseI;
			for( k = 0; k < 3; ++k )
			{
				if( rMinU > pF->Corner[k].Uvs[1].U ) rMinU = pF->Corner[k].Uvs[1].U;
				if( rMaxU < pF->Corner[k].Uvs[1].U ) rMaxU = pF->Corner[k].Uvs[1].U;
				if( rMinV > pF->Corner[k].Uvs[1].V ) rMinV = pF->Corner[k].Uvs[1].V;
				if( rMaxV < pF->Corner[k].Uvs[1].V ) rMaxV = pF->Corner[k].Uvs[1].V;
			}
			++ItParseI;
		}
		sizes[i].first = i;
		sizes[i].second = (rMaxU - rMinU) * (rMaxV - rMinV);
	}
	// Sort surfaces to put the biggest first
	for( i = 0; i < sizes.size()-1; ++i )
	for( j = i+1; j < sizes.size(); ++j )
	{
		if( sizes[i].second < sizes[j].second )
		{
			pair< sint32, sint32 > tmp = sizes[i];
			sizes[i] = sizes[j];
			sizes[j] = tmp;
		}
	}

	vector<CMesh::CFace*> TempGrp;
	vector<CMesh::CFace*>::iterator ItParseOut;
	TempGrp.resize( nNbFace );
	
	ItParseOut = TempGrp.begin();
	for( i = 0; i < sizes.size(); ++i )
	{
		// Treating group 
		j = sizes[i].first;
		ItParseI = ItFace;
		// Positionnement of the pointer to the first face of the j th group
		for( k = 0; k < j; ++k ) 
			ItParseI += PlaneGroup[k];
		// Copy the group j at the end of TempGrp
		for( k = 0; k < PlaneGroup[j]; ++k )
		{
			*ItParseOut = *ItParseI;
			++ItParseOut;
			++ItParseI;
		}
	}
	// So now we just have to copy back the temporary group
	ItParseOut = TempGrp.begin();
	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		*ItParseI = *ItParseOut;
		++ItParseOut;
		++ItParseI;
	}
	// And the same with the group delimiter
	for( i = 0; i < sizes.size(); ++i )
	{
		j = sizes[i].first;
		sizes[i].first = PlaneGroup[j];
	}
	for( i = 0; i < sizes.size(); ++i )
		PlaneGroup[i] = sizes[i].first;
}
*/

// -----------------------------------------------------------------------------------------------
void CreateLMPlaneFromFace( SLMPlane &Out, CMesh::CFace *pF )
{
	double	lumx1 = pF->Corner[0].Uvs[1].U, lumy1 = pF->Corner[0].Uvs[1].V, 
			lumx2 = pF->Corner[1].Uvs[1].U, lumy2 = pF->Corner[1].Uvs[1].V, 
			lumx3 = pF->Corner[2].Uvs[1].U, lumy3 = pF->Corner[2].Uvs[1].V;
	double minx, miny;
	double maxx, maxy;
	uint32 j, k;

	minx = lumx1;
	if( minx > lumx2 ) minx = lumx2;
	if( minx > lumx3 ) minx = lumx3;
	maxx = lumx1;
	if( maxx < lumx2 ) maxx = lumx2;
	if( maxx < lumx3 ) maxx = lumx3;
	miny = lumy1;
	if( miny > lumy2 ) miny = lumy2;
	if( miny > lumy3 ) miny = lumy3;
	maxy = lumy1;
	if( maxy < lumy2 ) maxy = lumy2;
	if( maxy < lumy3 ) maxy = lumy3;

	// Put the piece in the new basis (nPosX,nPosY)
	Out.x = ((sint32)floor(minx-0.5));
	Out.y = ((sint32)floor(miny-0.5));

	lumx1 -= Out.x; lumy1 -= Out.y;
	lumx2 -= Out.x; lumy2 -= Out.y;
	lumx3 -= Out.x;	lumy3 -= Out.y;

	Out.resize( 1 + ((sint32)floor(maxx+0.5)) - ((sint32)floor(minx-0.5)),
				1 + ((sint32)floor(maxy+0.5)) - ((sint32)floor(miny-0.5)) );

	for( j = 0; j < Out.w*Out.h; ++j )
	{
		Out.msk[j] = 0;
	}

// The square interact with the triangle if an edge of the square is cut by an edge of the triangle
// Or the square is in the triangle
	
	for( j = 0; j < Out.h-1; ++j )
	for( k = 0; k < Out.w-1; ++k )
	{
		// Is the square (j,k) is interacting with the triangle
		// This means : The square contains a point of the triangle (can be done for the 3 points)
		//              The triangle contains a point of the square
		// If so then we have to turn on all the 4 pixels of the square
		if( isInTriangleOrEdge(k+0.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+0.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangleOrEdge(k+1.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) )
		{
			Out.msk[k   + j    *Out.w] = 1;
			Out.msk[1+k + j    *Out.w] = 1;
			Out.msk[k   + (1+j)*Out.w] = 1;
			Out.msk[1+k + (1+j)*Out.w] = 1;
		}

		if( segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) )
		{
			Out.msk[k   + j    *Out.w] = 1;
			Out.msk[1+k + j    *Out.w] = 1;
			Out.msk[k   + (1+j)*Out.w] = 1;
			Out.msk[1+k + (1+j)*Out.w] = 1;
		}

	}
	// For all the points of the triangle update the square

// TODO : Test if we need it !

	Out.msk[((sint32)(lumx1-0.5))   + ((sint32)(lumy1-0.5))    *Out.w] = 1;
	Out.msk[1+((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))    *Out.w] = 1;
	Out.msk[((sint32)(lumx1-0.5))   + (1+((sint32)(lumy1-0.5)))*Out.w] = 1;
	Out.msk[1+((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*Out.w] = 1;

	Out.msk[((sint32)(lumx2-0.5))   + ((sint32)(lumy2-0.5))    *Out.w] = 1;
	Out.msk[1+((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))    *Out.w] = 1;
	Out.msk[((sint32)(lumx2-0.5))   + (1+((sint32)(lumy2-0.5)))*Out.w] = 1;
	Out.msk[1+((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*Out.w] = 1;
	
	Out.msk[((sint32)(lumx3-0.5))   + ((sint32)(lumy3-0.5))    *Out.w] = 1;
	Out.msk[1+((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))    *Out.w] = 1;
	Out.msk[((sint32)(lumx3-0.5))   + (1+((sint32)(lumy3-0.5)))*Out.w] = 1;
	Out.msk[1+((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*Out.w] = 1;

}

// -----------------------------------------------------------------------------------------------
void CreateLMPlaneFromFaceGroup( SLMPlane &Plane, vector<CMesh::CFace*>::iterator ItFace, uint32 nNbFace )
{
	uint32 i, j;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	CMesh::CFace *pF;

	Plane.faces.resize( nNbFace );

	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItParseI;
		for( j = 0; j < 3; ++j )
		{
			if( rMinU > pF->Corner[j].Uvs[1].U ) rMinU = pF->Corner[j].Uvs[1].U;
			if( rMaxU < pF->Corner[j].Uvs[1].U ) rMaxU = pF->Corner[j].Uvs[1].U;
			if( rMinV > pF->Corner[j].Uvs[1].V ) rMinV = pF->Corner[j].Uvs[1].V;
			if( rMaxV < pF->Corner[j].Uvs[1].V ) rMaxV = pF->Corner[j].Uvs[1].V;
		}
		Plane.faces[i] = pF;
		++ItParseI;
	}

	uint32 w = ( 1 + ((sint32)floor( rMaxU + 0.5 )) - ((sint32)floor( rMinU - 0.5 )) );
	uint32 h = ( 1 + ((sint32)floor( rMaxV + 0.5 )) - ((sint32)floor( rMinV - 0.5 )) );
	Plane.resize( w, h );
	Plane.x = ( ((sint32)floor( rMinU - 0.5 )) );
	Plane.y = ( ((sint32)floor( rMinV - 0.5 )) );
	for( j = 0; j < Plane.w*Plane.h; ++j )
		Plane.msk[j] = 0;

	ItParseI = ItFace;
	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItParseI;
		// Create Mask
		SLMPlane Piece;

		CreateLMPlaneFromFace( Piece, pF );
		// Because all is in absolute coordinate
		Piece.x -= Plane.x;
		Piece.y -= Plane.y;
		Piece.putIn( Plane );
		
		++ItParseI;
	}
}

// -----------------------------------------------------------------------------------------------
void ModifyLMPlaneWithOverSampling( SLMPlane *pPlane, double rOverSampling )
{	
	uint32 i, j;
	vector<CMesh::CFace*>::iterator ItFace = pPlane->faces.begin();
	uint32 nNbFace = pPlane->faces.size();

	pPlane->stretch( rOverSampling );
	for( j = 0; j < pPlane->w*pPlane->h; ++j ) // Reset the mask
		pPlane->msk[j] = 0;
	MultiplyFaceUV1( ItFace, nNbFace, rOverSampling );
	ItFace = pPlane->faces.begin();
	// Recreate the form
	for( i = 0; i < nNbFace; ++i )
	{
		CMesh::CFace *pF = *ItFace;
		SLMPlane Piece;
		CreateLMPlaneFromFace( Piece, pF );
		Piece.x -= pPlane->x;
		Piece.y -= pPlane->y;
		Piece.putIn( *pPlane, true );		
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void PlaceLMPlaneInLMPLane( SLMPlane &Dst, SLMPlane &Src )
{
	while( true )
	{
		if( ! Src.tryAllPosToPutIn( Dst ) )
		{
			if( ( Dst.w < MAXLIGHTMAPSIZE ) || ( Dst.h < MAXLIGHTMAPSIZE ) )
			{
				if( Dst.w < Dst.h )
					Dst.resize( Dst.w*2, Dst.h );
				else
					Dst.resize( Dst.w, Dst.h*2 );
			}
			else
			{
				// ERROR: we reached the maximum texture size
				nlstop;
			}
		}
		else
		{
			// We found a position
			Src.putIn( Dst );
			break;
		}
	}
}

// -----------------------------------------------------------------------------------------------
CRGBAF TestRay( CVector &vLightPos, CVector &vVertexPos, SWorldRT &wrt, sint32 nLightNb )
{
	CRGBAF retValue(1.0f, 1.0f, 1.0f, 1.0f);
	// Optim avec Cube Grid
	wrt.cgAccel[nLightNb].select( vVertexPos - vLightPos );
	while( !wrt.cgAccel[nLightNb].isEndSel() )
	{
		SCubeGridCell cell = wrt.cgAccel[nLightNb].getSel();

		CVector hit;

		CTriangle t(cell.pMB->Vertices[cell.pF->Corner[0].Vertex],
					cell.pMB->Vertices[cell.pF->Corner[1].Vertex],
					cell.pMB->Vertices[cell.pF->Corner[2].Vertex] );
		CPlane plane;
		plane.make( t.V0, t.V1, t.V2 );

		if( t.intersect( vLightPos, vVertexPos, hit, plane ) )
		{
			if( cell.pMB->Materials[cell.pF->MaterialId].getBlend() )
			{ // This is a transparent face we have to look in the texture
				/*
				ITexture *pT = cell.pMB->Materials[cell.pF->MaterialId].getTexture(0);
				if( pT == NULL )
				{
					retValue *= 1.0f - (cell.pMB->Materials[cell.pF->MaterialId].getOpacity()/255.0f);
				}
				else
				{
					CVector gradU, gradV;
					t.computeGradient(	cell.pF->Corner[0].Uvs[0].U,
										cell.pF->Corner[1].Uvs[0].U,
										cell.pF->Corner[2].Uvs[0].U, gradU );
					t.computeGradient(	cell.pF->Corner[0].Uvs[0].V,
										cell.pF->Corner[1].Uvs[0].V,
										cell.pF->Corner[2].Uvs[0].V, gradV );
					float u = cell.pF->Corner[0].Uvs[0].U+gradU*(hit-t.V0);
					float v = cell.pF->Corner[0].Uvs[0].V+gradV*(hit-t.V0);
					u = fmodf( u, 1.0f ); if( u < 0.0f ) u += 1.0f;
					v = fmodf( v, 1.0f ); if( v < 0.0f ) v += 1.0f;

					if( pT->getWidth() == 0 )
						((CTextureFile*)pT)->doGenerate();
					CRGBAF cPixMap = pT->getColor( u,v );
					cPixMap /= 255.0f;
					retValue *= 1.0f - (cPixMap.A * cell.pMB->Materials[cell.pF->MaterialId].getOpacity()/255.0f);
				}
				*/
				ITexture *pT = cell.pMB->Materials[cell.pF->MaterialId].getTexture(0);
				CRGBAF cPixMap;
				if( pT == NULL )
				{
					retValue *= 1.0f - (cell.pMB->Materials[cell.pF->MaterialId].getOpacity()/255.0f);
					cPixMap = CRGBAF(1.0f, 1.0f, 1.0f, 0.0f);
				}
				else
				{
					CVector gradU, gradV;
					t.computeGradient(	cell.pF->Corner[0].Uvs[0].U,
										cell.pF->Corner[1].Uvs[0].U,
										cell.pF->Corner[2].Uvs[0].U, gradU );
					t.computeGradient(	cell.pF->Corner[0].Uvs[0].V,
										cell.pF->Corner[1].Uvs[0].V,
										cell.pF->Corner[2].Uvs[0].V, gradV );
					float u = cell.pF->Corner[0].Uvs[0].U+gradU*(hit-t.V0);
					float v = cell.pF->Corner[0].Uvs[0].V+gradV*(hit-t.V0);
					u = fmodf( u, 1.0f ); if( u < 0.0f ) u += 1.0f;
					v = fmodf( v, 1.0f ); if( v < 0.0f ) v += 1.0f;

					if( pT->getWidth() == 0 )
						((CTextureFile*)pT)->generate();
					cPixMap = pT->getColor( u,v );
					cPixMap /= 255.0f;
				}
				cPixMap.A *= cell.pMB->Materials[cell.pF->MaterialId].getOpacity()/255.0f;
				cPixMap.R *= cell.pMB->Materials[cell.pF->MaterialId].getDiffuse().R/255.0f;
				cPixMap.G *= cell.pMB->Materials[cell.pF->MaterialId].getDiffuse().G/255.0f;
				cPixMap.B *= cell.pMB->Materials[cell.pF->MaterialId].getDiffuse().B/255.0f;
				if (cell.pMB->Materials[cell.pF->MaterialId].getStainedGlassWindow())
				{
					retValue = (1.0f - cPixMap.A)*(	retValue*(1.0f-cPixMap.A) + 
													retValue*cPixMap*cPixMap.A );
				}
				else
				{
					retValue *= (1.0f - cPixMap.A);
				}
			}
			else
			{ // This is not a transparent face so if we intersect we get shadow
				return CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}

		// Next selected element
		wrt.cgAccel[nLightNb].nextSel();
	}
	return retValue;
}

// -----------------------------------------------------------------------------------------------
CRGBAF RayTraceAVertex( CVector &p, SWorldRT &wrt, sint32 nLightNb, SLightBuild& rLight )
{
	CRGBAF Factor = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);

	TTicks zeTime = CTime::getPerformanceTime();

	switch( rLight.Type )
	{
		case SLightBuild::LightAmbient:
			Factor = CRGBAF(1.0f, 1.0f, 1.0f, 1.0f);
		break;
		case SLightBuild::LightSpot:
		case SLightBuild::LightPoint:
		{
			CVector light_p = p - rLight.Position;
			float light_p_distance = light_p.norm();
			light_p_distance = light_p_distance - (0.01f+(0.05f*light_p_distance/100.0f)); // Substract n centimeter
			light_p.normalize();
			light_p *= light_p_distance;
			Factor = TestRay( rLight.Position, rLight.Position + light_p, wrt, nLightNb );
		}
		break;
		case SLightBuild::LightDir:
		// Not handled for the moment
		//{
		//	CVector r1 = p-rLight.Direction/100;
		//	CVector r2 = p-100*rLight.Direction;
		//	rFactor = TestRay( r1, r2, wrt );
		//	nLightForFactor++;
		//}
			Factor = CRGBAF(1.0f, 1.0f, 1.0f, 1.0f);
		break;
		default:
		break;
	}

	timerCalcRT += CTime::getPerformanceTime() - zeTime;

	return Factor;
}

// -----------------------------------------------------------------------------------------------
CRGBAF LightAVertex( CVector &pRT, CVector &p, CVector &n, 
					vector<sint32> &vLights, vector<SLightBuild> &AllLights,
					SWorldRT &wrt, bool bDoubleSided, bool bRcvShadows )
{
	CRGBAF rgbafRet;
					
	rgbafRet.R = rgbafRet.G = rgbafRet.B = rgbafRet.A = 0.0;
	// Color calculation
	for( uint32 nLight = 0; nLight < vLights.size(); ++nLight )
	{
		SLightBuild &rLight = AllLights[vLights[nLight]];
		CRGBAF lightAmbiCol = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		CRGBAF lightDiffCol = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		CRGBAF lightSpecCol = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		CRGBAF RTFactor = CRGBAF(0.0f, 0.0f, 0.0f, 0.0f);
		float light_intensity = 0.0;

		switch( rLight.Type )
		{
			case SLightBuild::LightAmbient:
				lightAmbiCol.R = rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = rLight.Ambient.A / 255.0f;
				light_intensity = 1.0;
			break;
			case SLightBuild::LightPoint:
			{
				CVector p_light = rLight.Position - p;
				float p_light_distance = p_light.norm();
				if( p_light_distance < rLight.rRadiusMin )
					light_intensity = 1.0f;
				else
				if( p_light_distance > rLight.rRadiusMax )
					light_intensity = 0.0f;
				else
					light_intensity = 1.0f - (p_light_distance-rLight.rRadiusMin)/(rLight.rRadiusMax-rLight.rRadiusMin);
				p_light.normalize();

				// ??? light_intensity *= light_intensity * light_intensity;
				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				light_intensity *= rLight.rMult;
				lightAmbiCol.R = light_intensity * rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = light_intensity * rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = light_intensity * rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = light_intensity * rLight.Ambient.A / 255.0f;
				light_intensity *= max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;
			}
			break;
			case SLightBuild::LightDir:
			{	
				CVector p_light = - rLight.Direction;
				p_light.normalize();
				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				light_intensity *= rLight.rMult;
				lightAmbiCol.R = light_intensity * rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = light_intensity * rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = light_intensity * rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = light_intensity * rLight.Ambient.A / 255.0f;
				light_intensity = max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;

			}
			break;
			case SLightBuild::LightSpot:
			{
				CVector p_light = rLight.Position - p;
				float p_light_distance = p_light.norm();
				if( p_light_distance < rLight.rRadiusMin )
					light_intensity = 1.0f;
				else
				if( p_light_distance > rLight.rRadiusMax )
					light_intensity = 0.0f;
				else
					light_intensity = 1.0f - (p_light_distance-rLight.rRadiusMin)/(rLight.rRadiusMax-rLight.rRadiusMin);
				p_light.normalize();

				float ang = acosf( p_light * (-rLight.Direction) );
				if( ang > rLight.rFallof )
					light_intensity = 0.0f;
				else
				if( ang > rLight.rHotspot )
					light_intensity *= 1.0f - (ang-rLight.rHotspot)/(rLight.rFallof-rLight.rHotspot);
				light_intensity *= rLight.rMult;
				// ??? light_intensity *= light_intensity * light_intensity;
				if( bDoubleSided && (n*p_light < 0.0f) )
				{
					p_light = -p_light;
				}
				lightAmbiCol.R = light_intensity * rLight.Ambient.R / 255.0f;
				lightAmbiCol.G = light_intensity * rLight.Ambient.G / 255.0f;
				lightAmbiCol.B = light_intensity * rLight.Ambient.B / 255.0f;
				lightAmbiCol.A = light_intensity * rLight.Ambient.A / 255.0f;
				light_intensity *= max(0.0f, n*p_light);
				lightDiffCol.R = light_intensity * rLight.Diffuse.R / 255.0f;
				lightDiffCol.G = light_intensity * rLight.Diffuse.G / 255.0f;
				lightDiffCol.B = light_intensity * rLight.Diffuse.B / 255.0f;
				lightDiffCol.A = light_intensity * rLight.Diffuse.A / 255.0f;
				if (( rLight.pProjMap != NULL ) && (light_intensity > 0.0f ))
				{
					// Make the plane where the texture is
					CPlane plane; // Projection plane
					CVector ori = rLight.Position + rLight.mProj.getK();
					CVector norm = rLight.mProj.getK();
					plane.make( norm, ori );
					CVector inter = plane.intersect( rLight.Position, p );
					// Intersection conversion in i,j coordinate system with ori as origin
					//float dotSize = gOptions.rLumelSize * (rLight.Position-inter).norm() / (rLight.Position-p).norm();
					float x = -(inter-ori)*rLight.mProj.getI();
					float y = (inter-ori)*rLight.mProj.getJ();
					// Normalization x [-tan(fallof),tan(fallof)] -> [0,1]
					x = ((x / tanf( rLight.rFallof ))+1.0f)/2.0f;
					y = ((y / tanf( rLight.rFallof ))+1.0f)/2.0f;
					
					//x *= rLight.pProjMap->Width();
					//y *= rLight.pProjMap->Height();
					//dotSize = ((rLight.pProjMap->Width()+rLight.pProjMap->Height())/2.0)*
					//			(dotSize / tanf( rLight.rFallof ))/2.0f;
					CRGBAF col = wrt.proj[vLights[nLight]].getColor(x, y);
					lightDiffCol.R *= col.R/255.0f;
					lightDiffCol.G *= col.G/255.0f;
					lightDiffCol.B *= col.B/255.0f;
					lightDiffCol.A *= col.A/255.0f;
/*
					x *= rLight.pProjMap->Width();
					y *= rLight.pProjMap->Height();
					if( x < 0.0f ) x = 0.0f;
					if( y < 0.0f ) y = 0.0f;
					if( x >= rLight.pProjMap->Width()  ) x = (float)rLight.pProjMap->Width() -1.0f;
					if( y >= rLight.pProjMap->Height() ) y = (float)rLight.pProjMap->Height()-1.0f;
					BMM_Color_64 OnePixel;
					rLight.pProjMap->GetPixels( (sint32)x, (sint32)y, 1, &OnePixel );
					lightDiffCol.R *= OnePixel.r/65535.0f;
					lightDiffCol.G *= OnePixel.g/65535.0f;
					lightDiffCol.B *= OnePixel.b/65535.0f;
					lightDiffCol.A *= OnePixel.a/65535.0f;
*/
				}
			}
			break;
			default:
			break;
		}
		if( light_intensity > 0.0f )
		{
			if( bRcvShadows && rLight.bCastShadow && gOptions.bShadow )
				RTFactor = RayTraceAVertex( pRT, wrt, vLights[nLight], rLight );
			else
				RTFactor = CRGBAF(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		rgbafRet.R += lightAmbiCol.R + lightDiffCol.R * RTFactor.R;
		if( rgbafRet.R > 2.0f ) rgbafRet.R = 2.0;
		rgbafRet.G += lightAmbiCol.G + lightDiffCol.G * RTFactor.G;
		if( rgbafRet.G > 2.0f ) rgbafRet.G = 2.0;
		rgbafRet.B += lightAmbiCol.B + lightDiffCol.B * RTFactor.B;
		if( rgbafRet.B > 2.0f ) rgbafRet.B = 2.0;
		rgbafRet.A += lightAmbiCol.A + lightDiffCol.A * RTFactor.A;
		if( rgbafRet.A > 2.0f ) rgbafRet.A = 2.0;
	}
	return rgbafRet;
}

// -----------------------------------------------------------------------------------------------
bool segmentIntersectBSphere( CVector &p1, CVector &p2, CBSphere &bs )
{
	// Is one point is in the sphere ?
	CVector r = bs.Center - p1;
	float f;
	if( r.norm() <= bs.Radius )
		return true;
	r = bs.Center - p2;
	if( r.norm() <= bs.Radius )
		return true;
	// Is the orthogonal projection of the center on the segment is in the sphere ?
	r = p2 - p1;
	f = r.norm();
	f = ( r * (bs.Center - p1) ) / ( f * f );
	if( ( f >= 0.0 ) && ( f <= 1.0 ) )
	{
		r = bs.Center - (p1 + r*f);
		if( r.norm() <= bs.Radius )
			return true;
	}
	return false;
}


// -----------------------------------------------------------------------------------------------
void FirstLight( CMesh::CMeshBuild* pMB, SLMPlane &Plane, vector<CVector> &vVertices, 
				CMatrix& ToWorldMat, vector<sint32> &vLights, vector<SLightBuild> &AllLights,
				uint32 nLayerNb, SWorldRT &wrt )
{
	// Fill interiors
	vector<CMesh::CFace*>::iterator ItFace = Plane.faces.begin();
	uint32 nNbFace = Plane.faces.size();
	uint32 i;
	sint32 j, k;
	double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
	sint32 nPosMinU, nPosMaxU, nPosMinV, nPosMaxV;
	CMesh::CFace *pF;
	SGradient g;
	
	for( i = 0; i < Plane.w*Plane.h; ++i )
		if( Plane.msk[i] != 0 )
			Plane.msk[i] = 1;

	for( i = 0; i < nNbFace; ++i )
	{
		pF = *ItFace;

		bool doubleSided = pMB->Materials[pF->MaterialId].detDoubleSided();

		// Select bounding square of the triangle
		for( j = 0; j < 3; ++j )
		{
			if( rMinU > pF->Corner[j].Uvs[1].U ) rMinU = pF->Corner[j].Uvs[1].U;
			if( rMaxU < pF->Corner[j].Uvs[1].U ) rMaxU = pF->Corner[j].Uvs[1].U;
			if( rMinV > pF->Corner[j].Uvs[1].V ) rMinV = pF->Corner[j].Uvs[1].V;
			if( rMaxV < pF->Corner[j].Uvs[1].V ) rMaxV = pF->Corner[j].Uvs[1].V;
		}
		nPosMaxU = ((sint32)floor( rMaxU + 0.5 ));
		nPosMaxV = ((sint32)floor( rMaxV + 0.5 ));
		nPosMinU = ((sint32)floor( rMinU - 0.5 ));
		nPosMinV = ((sint32)floor( rMinV - 0.5 ));

		CVector n1 = ToWorldMat.mulVector( pF->Corner[0].Normal );
		CVector n2 = ToWorldMat.mulVector( pF->Corner[1].Normal );
		CVector n3 = ToWorldMat.mulVector( pF->Corner[2].Normal );
	
		g.init( pF, vVertices, n1, n2, n3 );

		// Process all the interior
		for( k = nPosMinV; k <= nPosMaxV; ++k )
		for( j = nPosMinU; j <= nPosMaxU; ++j )
		{
			if( isInTriangleOrEdge( j+0.5, k+0.5,
									pF->Corner[0].Uvs[1].U, pF->Corner[0].Uvs[1].V,
									pF->Corner[1].Uvs[1].U, pF->Corner[1].Uvs[1].V,
									pF->Corner[2].Uvs[1].U, pF->Corner[2].Uvs[1].V ) )
			{
				CVector p = g.getInterpolatedVertex( j+0.5, k+0.5);
				CVector n = g.getInterpolatedNormal( j+0.5, k+0.5);
				CRGBAF vl = g.getInterpolatedColor( j+0.5, k+0.5);
				CRGBAF col = LightAVertex( p, p, n, vLights, AllLights, wrt, doubleSided, pMB->bRcvShadows );
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].R = col.R*(vl.R/255.0f);
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].G = col.G*(vl.G/255.0f);
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].B = col.B*(vl.B/255.0f);
				Plane.col[j-Plane.x + (k-Plane.y)*Plane.w].p[nLayerNb].A = 1.0f;
				// Darken the plane to indicate pixel is calculated
				Plane.msk[j-Plane.x + (k-Plane.y)*Plane.w] = 2;
			}
		}
		// Next Face
		++ItFace;
	}
}

// -----------------------------------------------------------------------------------------------
void SecondLight( CMesh::CMeshBuild *pMB, vector<SLMPlane*>::iterator ItPlanes, uint32 nNbPlanes,
					vector<CVector> &vVertices, CMatrix& ToWorldMat, 
					vector<sint32> &vLights, vector<SLightBuild> &AllLights,
					uint32 nLayerNb, SWorldRT &wrt)
{
	// Fill interiors
	uint32 nPlanes1;

	vector<SLMPlane*>::iterator ItPlanes1 = ItPlanes;
	for( nPlanes1 = 0; nPlanes1 < nNbPlanes; ++nPlanes1 )
	{
		uint32 i;
		sint32 j, k;
		sint32 nPosMinU, nPosMaxU, nPosMinV, nPosMaxV;
		SGradient g;
		
		SLMPlane *pPlane1 = *ItPlanes1;
		vector<CMesh::CFace*>::iterator ItParseI = pPlane1->faces.begin();
		uint32 nNbFace1 = pPlane1->faces.size();
		for( i = 0; i < nNbFace1; ++i )
		{
			CMesh::CFace *pF1 = *ItParseI;
			double rMinU = 1000000.0, rMaxU = -1000000.0, rMinV = 1000000.0, rMaxV = -1000000.0;
			bool doubleSided = pMB->Materials[pF1->MaterialId].detDoubleSided();
			// Select bounding square of the triangle
			for( j = 0; j < 3; ++j )
			{
				if( rMinU > pF1->Corner[j].Uvs[1].U ) rMinU = pF1->Corner[j].Uvs[1].U;
				if( rMaxU < pF1->Corner[j].Uvs[1].U ) rMaxU = pF1->Corner[j].Uvs[1].U;
				if( rMinV > pF1->Corner[j].Uvs[1].V ) rMinV = pF1->Corner[j].Uvs[1].V;
				if( rMaxV < pF1->Corner[j].Uvs[1].V ) rMaxV = pF1->Corner[j].Uvs[1].V;
			}
			nPosMaxU = ((sint32)floor( rMaxU + 0.5 ));
			nPosMaxV = ((sint32)floor( rMaxV + 0.5 ));
			nPosMinU = ((sint32)floor( rMinU - 0.5 ));
			nPosMinV = ((sint32)floor( rMinV - 0.5 ));

			CVector n1 = ToWorldMat.mulVector( pF1->Corner[0].Normal );
			CVector n2 = ToWorldMat.mulVector( pF1->Corner[1].Normal );
			CVector n3 = ToWorldMat.mulVector( pF1->Corner[2].Normal );
	
			g.init( pF1, vVertices, n1, n2, n3 );

			double	lumx1 = pF1->Corner[0].Uvs[1].U, lumy1 = pF1->Corner[0].Uvs[1].V, 
					lumx2 = pF1->Corner[1].Uvs[1].U, lumy2 = pF1->Corner[1].Uvs[1].V, 
					lumx3 = pF1->Corner[2].Uvs[1].U, lumy3 = pF1->Corner[2].Uvs[1].V;

			// Process all the exterior and try to link with other planes
			for( k = nPosMinV; k < nPosMaxV; ++k )
			for( j = nPosMinU; j < nPosMaxU; ++j )
			if( ( pPlane1->msk[j-pPlane1->x   + (k-pPlane1->y)*pPlane1->w]   == 1 ) ||
				( pPlane1->msk[1+j-pPlane1->x + (k-pPlane1->y)*pPlane1->w]   == 1 ) ||
				( pPlane1->msk[1+j-pPlane1->x + (1+k-pPlane1->y)*pPlane1->w] == 1 ) ||
				( pPlane1->msk[j-pPlane1->x   + (1+k-pPlane1->y)*pPlane1->w] == 1 ) )
			if( segmentIntersection(j+0.5, k+0.5, j+1.5, k+0.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+0.5, k+0.5, j+1.5, k+0.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+0.5, k+0.5, j+1.5, k+0.5, lumx3, lumy3, lumx1, lumy1) ||
	
				segmentIntersection(j+0.5, k+0.5, j+0.5, k+1.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+0.5, k+0.5, j+0.5, k+1.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+0.5, k+0.5, j+0.5, k+1.5, lumx3, lumy3, lumx1, lumy1) ||
	
				segmentIntersection(j+1.5, k+1.5, j+1.5, k+0.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+1.5, k+1.5, j+1.5, k+0.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+1.5, k+1.5, j+1.5, k+0.5, lumx3, lumy3, lumx1, lumy1) ||
	
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx1, lumy1, lumx2, lumy2) ||
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx2, lumy2, lumx3, lumy3) ||
				segmentIntersection(j+1.5, k+1.5, j+0.5, k+1.5, lumx3, lumy3, lumx1, lumy1) )
			{
				// If all segment of the current face are linked with a face in this plane, no need to continue
				vector<CMesh::CFace*>::iterator ItParseM = pPlane1->faces.begin();
				uint32 nNbSeg = 0;
				uint32 m, n;
				for( m = 0; m < nNbFace1; ++m )
				{
					CMesh::CFace *pF2 = *ItParseM;
					if( m != i )
						if( FaceContinuous( pF1, pF2 ) )
							++nNbSeg;
					++ItParseM;
				}
				if( nNbSeg >= 3 )
					continue;
				// Get the face on the other plane with a common segment
				vector<SLMPlane*>::iterator ItParsePlanes = ItPlanes;
				for( m = 0; m < nNbPlanes; ++m )
				{
					SLMPlane *pPlane2 = *ItParsePlanes;
					if( pPlane2 != pPlane1 )
					for( n = 0; n < pPlane2->faces.size(); ++n )
					{
						CMesh::CFace *pF2 = pPlane2->faces[n];
						if( FaceContinuous( pF1, pF2 ) )
						{
							for( uint32 o = 0; o < 4; ++o )
							{
								sint32 nAbsX = j + (o/2), nAbsY = k + (o%2);
								// Is it a pixel to treat and pixel in the 2nd plane
								if( ( pPlane1->msk[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w] == 1 ) &&
									(nAbsX >= pPlane2->x) && (nAbsX < (pPlane2->x+(sint32)pPlane2->w) ) &&
									(nAbsY >= pPlane2->y) && (nAbsY < (pPlane2->y+(sint32)pPlane2->h) ) )
								{
									// Is it an interior calculated pixel ?
									if( pPlane2->msk[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w] == 2 )
									{ // Yes -> ok so get it
										pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb] = 
													pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb];
										pPlane1->msk[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w] = 3;
									}
									else
									if( pPlane2->msk[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w] == 1 )
									{ // No -> Add extrapolated value
										CVector iv = g.getInterpolatedVertex( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
										CVector in = g.getInterpolatedNormal( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
										CRGBAF vl = g.getInterpolatedColor( j+0.5, k+0.5);
										CVector rv = g.getInterpolatedVertexInFace( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5, pF1 );
										CRGBAF col = LightAVertex( rv, iv, in, vLights, AllLights, wrt, doubleSided, pMB->bRcvShadows );
										//float f = 1.0f;
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].R += col.R*(vl.R/255.0f);
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].G += col.G*(vl.G/255.0f);
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].B += col.B*(vl.B/255.0f);
										pPlane2->col[nAbsX-pPlane2->x + (nAbsY-pPlane2->y)*pPlane2->w].p[nLayerNb].A += 1.0f;
									}
								}
							}
						}
					}
					++ItParsePlanes;
				}

				for( sint32 o = 0; o < 4; ++o )
				{
					sint32 nAbsX = j + (o/2), nAbsY = k + (o%2);
					if( pPlane1->msk[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w] == 1 )
					{
						CVector iv = g.getInterpolatedVertex( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
						CVector in = g.getInterpolatedNormal( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5);
						CRGBAF vl = g.getInterpolatedColor( j+0.5, k+0.5);
						CVector rv = g.getInterpolatedVertexInFace( ((double)nAbsX)+0.5, ((double)nAbsY)+0.5, pF1 );
						CRGBAF col = LightAVertex( rv, iv, in, vLights, AllLights, wrt, doubleSided, pMB->bRcvShadows );
						//float f = 1.0f;
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].R += col.R*(vl.R/255.0f);
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].G += col.G*(vl.G/255.0f);
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].B += col.B*(vl.B/255.0f);
						pPlane1->col[nAbsX-pPlane1->x + (nAbsY-pPlane1->y)*pPlane1->w].p[nLayerNb].A += 1.0f;
					}
				}
			}
			// Next Face
			++ItParseI;
		}
		++ItPlanes1;
	}
	// All planes are done so now we have to average the value of lumels grouping severals normals
	ItPlanes1 = ItPlanes;
	for( nPlanes1 = 0; nPlanes1 < nNbPlanes; ++nPlanes1 )
	{
		uint32 j, k;
		SLMPlane *pPlane1 = *ItPlanes1;
		
		for( k = 0; k < pPlane1->h; ++k )
		for( j = 0; j < pPlane1->w; ++j )
		{
			if( pPlane1->msk[j+k*pPlane1->w] == 1 )
			{
				sint32 nNbNormals = (sint32)pPlane1->col[j + k*pPlane1->w].p[nLayerNb].A;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].R /= nNbNormals;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].G /= nNbNormals;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].B /= nNbNormals;
				pPlane1->col[j + k*pPlane1->w].p[nLayerNb].A = 1.0f;
				pPlane1->msk[j + k*pPlane1->w] = 4;
			}
		}
		++ItPlanes1;
	}
}

// -----------------------------------------------------------------------------------------------
bool isAllFaceMapped( vector<CMesh::CFace*>::iterator ItFace, sint32 nNbFaces )
{
	sint32 i, j;

	vector<CMesh::CFace*>::iterator ItParseI = ItFace;
	for( i = 0; i < nNbFaces; ++i )
	{
		CMesh::CFace *pF = *ItParseI;
		for( j = 0; j < 3; ++j )
		{
			if( (fabsf(pF->Corner[j].Uvs[1].U) > 64.0) || 
				(fabsf(pF->Corner[j].Uvs[1].V) > 64.0) )
				return false;
		}
		++ItParseI;
	}
	double TextureSurf = 0.0f;
	ItParseI = ItFace;
	for( i = 0; i < nNbFaces; ++i )
	{
		CMesh::CFace *pF = *ItParseI;
		CVector p1, p2, p3;
		p1.x = pF->Corner[0].Uvs[1].U; p1.y = pF->Corner[0].Uvs[1].V; p1.z = 0.0f;
		p2.x = pF->Corner[1].Uvs[1].U; p2.y = pF->Corner[1].Uvs[1].V; p2.z = 0.0f;
		p3.x = pF->Corner[2].Uvs[1].U; p3.y = pF->Corner[2].Uvs[1].V; p3.z = 0.0f;
		TextureSurf += calculateTriangleSurface( p1, p2, p3 );
		++ItParseI;
	}
	if( fabs(TextureSurf) < 0.000001 )
		return false;
	return true;
}

// -----------------------------------------------------------------------------------------------
CAABBox getMeshBBox( CMesh::CMeshBuild& rMB, bool bNeedToTransform )
{
	CAABBox meshBox;
	if( bNeedToTransform )
	{
		CMatrix MBMatrix = getObjectToWorldMatrix( &rMB );

		for( uint32 j = 0; j < rMB.Vertices.size(); ++j )
			if( j == 0 )
				meshBox.setCenter( MBMatrix * rMB.Vertices[j] );
			else
				meshBox.extend( MBMatrix * rMB.Vertices[j] );
	}
	else
	{
		for( uint32 j = 0; j < rMB.Vertices.size(); ++j )
			if( j == 0 )
				meshBox.setCenter( rMB.Vertices[j] );
			else
				meshBox.extend( rMB.Vertices[j] );
	}
	return meshBox;
}

// -----------------------------------------------------------------------------------------------
CAABBox getLightBBox( CVector &vPos, float rRadius )
{
	CAABBox lightBox;
	lightBox.setCenter( vPos );
	lightBox.extend( vPos - CVector(rRadius,0,0) );
	lightBox.extend( vPos + CVector(rRadius,0,0) );
	lightBox.extend( vPos - CVector(0,rRadius,0) );
	lightBox.extend( vPos + CVector(0,rRadius,0) );
	lightBox.extend( vPos - CVector(0,0,rRadius) );
	lightBox.extend( vPos + CVector(0,0,rRadius) );
	return lightBox;
}

// -----------------------------------------------------------------------------------------------
bool isLightCanCastShadowOnBox( SLightBuild &rSLB, CAABBox &b )
{
	switch( rSLB.Type )
	{
		case SLightBuild::LightAmbient: // No need an ambient light...
			// No ambient handled for the moment
		break;
		case SLightBuild::LightSpot: // For the moment spot like point
		case SLightBuild::LightPoint:
		{
			CAABBox lightBox = getLightBBox( rSLB.Position, rSLB.rRadiusMax );
			if( lightBox.intersect( b ) )
				return true;

			if( b.include( lightBox.getMin() ) )
				return true;

			if( lightBox.include( b.getMin() ) )
				return true;
		}
		break;
		case SLightBuild::LightDir:
			// Attenuation not handled (ask cyril later)
		break;
	}
	return false;
}

// -----------------------------------------------------------------------------------------------
bool isInteractionLightMesh( SLightBuild &rSLB, CMesh::CMeshBuild &rMB )
{
	CAABBox meshBox;

	if( rSLB.Type == SLightBuild::LightAmbient )
		return true;
	meshBox = getMeshBBox( rMB, true );
	return isLightCanCastShadowOnBox( rSLB, meshBox );
}

// -----------------------------------------------------------------------------------------------
bool isInteractionLightMeshWithoutAmbient( SLightBuild &rSLB, CMesh::CMeshBuild &rMB )
{
	CAABBox meshBox;

	if( rSLB.Type == SLightBuild::LightAmbient )
		return false;
	meshBox = getMeshBBox( rMB, true );
	return isLightCanCastShadowOnBox( rSLB, meshBox );
}

// -----------------------------------------------------------------------------------------------
// Get all lights that can cast shadows on the current mesh
void getLightInteract( CMesh::CMeshBuild* pMB, vector<SLightBuild> &AllLights, vector< vector<sint32> >&vvLights )
{
	uint32 nNbGroup = 0;
	vector<sint32> vlbTmp;
	uint32 i, j;

	for( i = 0; i < AllLights.size(); ++i )
	{
		if( isInteractionLightMesh( AllLights[i], *pMB ) )
		{
			// Is the light name already exist
			for( j = 0; j < nNbGroup; ++j )
				if( AllLights[vvLights[j].operator[](0)].GroupName == AllLights[i].GroupName )
					break;
			// The light name does not exist create a new group
			if( ( j == nNbGroup ) && ( nNbGroup < 8 ) )
			{
				vvLights.push_back( vlbTmp ); // Static lighting
				vvLights[nNbGroup].push_back( i );
				++nNbGroup;
			}
			else
			{ // The light name already exist or there is not enought groups
				if( j == nNbGroup )
					j = nNbGroup - 1;
				vvLights[j].push_back( i );
			}
		}
	}
}

// -----------------------------------------------------------------------------------------------
void GetAllSelectedNode( vector< CMesh::CMeshBuild* > &Meshes,  vector< INode* > &INodes, 
						Interface& ip, vector<SLightBuild> &AllLights, bool bAbsPath )
{
	// Get time
	TimeValue tvTime = ip.GetTime();
	// Get node count
	int nNumSelNode = ip.GetSelNodeCount();
	// Save all selected objects
	for (int nNode=0; nNode<nNumSelNode; nNode++)
	{
		// Get the node
		INode* pNode = ip.GetSelNode (nNode);

		if (! RPO::isZone (*pNode, tvTime) )
		if (CExportNel::isMesh (*pNode, tvTime))
		{
			CMesh::CMeshBuild *pMB;
			pMB = CExportNel::createMeshBuild( *pNode, tvTime, bAbsPath );
			// If the mesh has no interaction with one of the light selected we do not need it
			bool bInteract = false;
			if( pMB->bCastShadows )
			for( uint32 i = 0; i < AllLights.size(); ++i )
			if( isInteractionLightMeshWithoutAmbient( AllLights[i], *pMB ) )
			{
				bInteract = true;
				break;
			}
			if( bInteract )
			{
				Meshes.push_back( pMB );
				INodes.push_back( pNode );
			}
			else
				delete pMB; // No interaction so delete the mesh
		}
	}
}

// -----------------------------------------------------------------------------------------------
void GetAllNodeInScene( vector< CMesh::CMeshBuild* > &Meshes, vector< INode* > &INodes,
					   Interface& ip, vector<SLightBuild> &AllLights, bool bAbsPath,
					   INode* pNode = NULL )
{
	if( pNode == NULL )
		pNode = ip.GetRootNode();

	// Get a pointer on the object's node
	TimeValue tvTime = ip.GetTime();

	if( ! RPO::isZone( *pNode, tvTime ) )
	if( CExportNel::isMesh( *pNode, tvTime ) )
	{
		CMesh::CMeshBuild *pMB;
		pMB = CExportNel::createMeshBuild( *pNode, tvTime, bAbsPath );
		// If the mesh has no interaction with one of the light selected we do not need it
		bool bInteract = false;
		if( pMB->bCastShadows )
		for( uint32 i = 0; i < AllLights.size(); ++i )
		if( isInteractionLightMeshWithoutAmbient( AllLights[i], *pMB ) )
		{
			bInteract = true;
			break;
		}
		if( bInteract )
		{
			Meshes.push_back( pMB );
			INodes.push_back( pNode );
		}
		else
			delete pMB; // No interaction so delete the mesh
	}

	for( sint32 i = 0; i < pNode->NumberOfChildren(); ++i )
		GetAllNodeInScene( Meshes, INodes, ip, AllLights, bAbsPath, pNode->GetChildNode(i) );
}

// -----------------------------------------------------------------------------------------------
void buildWorldRT( SWorldRT &wrt, vector<SLightBuild> &AllLights, Interface &ip, bool absPath )
{
	uint32 i, j, k;

	// Get all the nodes in the scene
	if( gOptions.bExcludeNonSelected )
		GetAllSelectedNode( wrt.vMB, wrt.vINode, ip, AllLights, absPath );
	else
		GetAllNodeInScene( wrt.vMB, wrt.vINode, ip, AllLights, absPath );

	// Transform the meshbuilds vertices and normals to have world coordinates
	for( i = 0; i < wrt.vMB.size(); ++i )
	{
		CMatrix MBMatrix = getObjectToWorldMatrix( wrt.vMB[i] );
		// Update vertices
		for( j = 0; j < wrt.vMB[i]->Vertices.size(); ++j )
			wrt.vMB[i]->Vertices[j] = MBMatrix * wrt.vMB[i]->Vertices[j];
		// Update normals
		MBMatrix.invert();
		MBMatrix.transpose();
		for( j = 0; j < wrt.vMB[i]->Faces.size(); ++j )
			for( k = 0; k < 3 ; ++k )
				wrt.vMB[i]->Faces[j].Corner[k].Normal = 
									MBMatrix.mulVector( wrt.vMB[i]->Faces[j].Corner[k].Normal );
	}

	// Construct all cube grids from all lights
	wrt.cgAccel.resize( AllLights.size() );
	for( i = 0; i < AllLights.size(); ++i )
	{
		wrt.cgAccel[i].create( 64 ); // width of each grid in number of square
		switch( AllLights[i].Type )
		{
			case SLightBuild::LightAmbient:
				// No ambient handled for the moment
			break;
			case SLightBuild::LightSpot: // For the moment spot like point
			case SLightBuild::LightPoint:
			{
			for( j = 0; j < wrt.vMB.size(); ++j )
			{
				if( AllLights[i].setExclusion.find( wrt.vINode[j]->GetName() ) != AllLights[i].setExclusion.end() ) 
					continue;
				
				for( k = 0; k < wrt.vMB[j]->Faces.size(); ++k )
				{
					SCubeGridCell cell;
					cell.pF = &(wrt.vMB[j]->Faces[k]);
					cell.pMB = wrt.vMB[j];
					CTriangle tri = CTriangle( 
						cell.pMB->Vertices[cell.pF->Corner[0].Vertex] - AllLights[i].Position,
						cell.pMB->Vertices[cell.pF->Corner[1].Vertex] - AllLights[i].Position,
						cell.pMB->Vertices[cell.pF->Corner[2].Vertex] - AllLights[i].Position );
					if( intersectionTriangleSphere( tri, CBSphere(CVector(0,0,0), AllLights[i].rRadiusMax) ) )
						wrt.cgAccel[i].insert( tri, cell );
				}
			}
			}
			break;
			case SLightBuild::LightDir:
				// No directionnal handled for the moment
			break;
		}
	}

	// Construct the projector bitmap
	wrt.proj.resize( AllLights.size() );
	for( i = 0; i < AllLights.size(); ++i )
	if( AllLights[i].Type == SLightBuild::LightSpot )
	if( AllLights[i].pProjMap != NULL )
	{
		wrt.proj[i].resize( AllLights[i].pProjMap->Width(), AllLights[i].pProjMap->Height(), CBitmap::RGBA );
		// Copy the bitmap
		std::vector<uint8> &rBitmap = wrt.proj[i].getPixels();
		BMM_Color_64 OnePixel;
		for( k = 0; k < wrt.proj[i].getHeight(); ++k )
		for( j = 0; j < wrt.proj[i].getWidth(); ++j )
		{
			AllLights[i].pProjMap->GetPixels( j, k, 1, &OnePixel );
			rBitmap[(j+k*wrt.proj[i].getWidth())*4+0] = OnePixel.r>>8;
			rBitmap[(j+k*wrt.proj[i].getWidth())*4+1] = OnePixel.g>>8;
			rBitmap[(j+k*wrt.proj[i].getWidth())*4+2] = OnePixel.b>>8;
			rBitmap[(j+k*wrt.proj[i].getWidth())*4+3] = OnePixel.a>>8;
		}
		wrt.proj[i].buildMipMaps();
	}
}

// -----------------------------------------------------------------------------------------------
// Is the box b1 can cast shadow on the box b2 with the light l ?
bool isBoxCanCastShadowOnBoxWithLight( CAABBox &b1, CAABBox &b2, SLightBuild &l )
{
	// if the light is included in the box b2
	return isLightCanCastShadowOnBox( l, b1 );
}

// -----------------------------------------------------------------------------------------------
void supprLightNoInteract( vector<SLightBuild> &vLights, 
						  vector< pair < CMesh::CMeshBuild*,INode* > > &AllSelectedMeshes )
{
	uint32 i, j;

	for( i = 0; i < vLights.size(); ++i )
	{
		bool bInteract = false;

		for( j = 0; j < AllSelectedMeshes.size(); ++j )
		if( isInteractionLightMesh( vLights[i], *AllSelectedMeshes[j].first ) )
		{
			bInteract = true;
			break;
		}
		if( !bInteract )
		{
			// Suppress the light because it has no interaction with selected meshes
			for( j = i; j < (vLights.size()-1); ++j )
				vLights[j] = vLights[j+1];
			vLights.resize(vLights.size()-1);
			--i;
		}
	}
}

// -----------------------------------------------------------------------------------------------
void supprLightNoInteractOne( vector<SLightBuild> &vLights, CMesh::CMeshBuild* pMB, INode &node )
{
	uint32 i, j;

	for( i = 0; i < vLights.size(); ++i )
	{
		bool bInteract = false;

		if( vLights[i].setExclusion.find( node.GetName() ) != vLights[i].setExclusion.end() )
		{
			bInteract = false;
		}
		else
			if( isInteractionLightMesh( vLights[i], *pMB ) )
			{
				bInteract = true;			
			}
		if( !bInteract )
		{
			// Suppress the light because it has no interaction with selected meshes
			for( j = i; j < (vLights.size()-1); ++j )
				vLights[j] = vLights[j+1];
			vLights.resize(vLights.size()-1);
			--i;
		}
	}
}

// -----------------------------------------------------------------------------------------------
// Add information for ont mesh to reference all the lights that interact with him
void AddLightInfo( CMesh::CMeshBuild *pMB, string &LightName, uint8 nMatNb, uint8 nStageNb )
{
	CMesh::CMatStage ms;
	ms.nMatNb = nMatNb;
	ms.nStageNb = nStageNb;
	CMesh::CLightInfoMapList listTemp;
	//list< pair< uint8, uint8 > > listTemp;
	CMesh::TLightInfoMap::iterator itMap = pMB->LightInfoMap.find( LightName );
	if( itMap == pMB->LightInfoMap.end() )
	{
		listTemp.push_back(	ms );
		pMB->LightInfoMap.insert( pair< string, CMesh::CLightInfoMapList >(LightName, listTemp) );
	}
	else
	{
		itMap->second.push_back( ms );
	}
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CExportNel::deleteLM(INode& ZeNode, CExportNelOptions& opt)
{
	sint32 i;

	// Suppress all lightmap files
	for( i = 0; i < 8; ++i )
	{		
		string sSaveName;
		sSaveName = opt.sExportLighting;
		if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
		sSaveName += ZeNode.GetName();
		char tmp[32];
		sprintf( tmp, "%d", i );
		sSaveName += tmp;
		sSaveName += ".tga";
		FILE	*file;
		if( file = fopen(sSaveName.c_str(),"rb") )
		{
			fclose( file );
			DeleteFile( sSaveName.c_str() );
		}
	}
}


// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// Caclulate the lightmap of the specified mesh build

// absolutePath tell this code to put the name of the lightmap in absolute or relative path
// this is very usefull for viewer inside MAX
bool CExportNel::calculateLM( CMesh::CMeshBuild *pZeMeshBuild, INode& ZeNode, 
							Interface& ip, TimeValue tvTime, bool absolutePath,
							CExportNelOptions &structExport )
{
	uint32 i, j;

	gOptions = structExport;

	SWorldRT WorldRT; // The static world for raytrace
	vector<SLightBuild> AllLights;

	// Select meshes to test for raytrace
	// Get all lights from MAX
	getLightBuilds( AllLights, tvTime, ip );
	// Get all lights L that have influence over the mesh selected
	// supprLightNoInteract( AllLights, AllMeshBuilds );
	supprLightNoInteractOne( AllLights, pZeMeshBuild, ZeNode );
	// Get all meshes that are influenced by the lights L			
	buildWorldRT( WorldRT, AllLights, ip, absolutePath );

	//for( nNode=0; nNode < nNbMesh; ++nNode )
	{
		// First order face by Material and by texture surface
		CMesh::CMeshBuild *pMB = pZeMeshBuild;
		vector<CMesh::CFace*> AllFaces;
		CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
		vector<CVector> AllVertices; // All vertices in world space
		vector<sint32> FaceGroupByMat; // Number of faces with the same properties
		uint32 nNbFace = pMB->Faces.size(), nNbVertex = pMB->Vertices.size();
		sint32 offsetMat, offsetSmooth, offsetPlane;
		vector<SLMPlane*> AllPlanes;
		sint32 AllPlanesPrevSize;
		vector< vector<sint32> > vvLights;

		// Select Lights interacting with the node
		getLightInteract( pMB, AllLights, vvLights );

		AllPlanes.clear();
		// Make Geometry like we want			
		// Make a vector of pointer to all the faces of the MeshBuild
		AllFaces.resize( nNbFace );
		for( i = 0; i < nNbFace; ++i )
			AllFaces[i] = &pMB->Faces[i];
		// Make All vertices of the mesh in the world basis
		AllVertices.resize(nNbVertex);
		for( i = 0; i < nNbVertex; ++i )
			AllVertices[i] = MBMatrix * pMB->Vertices[i];
		// Invert and transpose for use of futur normal
		MBMatrix.invert();
		MBMatrix.transpose();

		// Bubble sort pointer to the faces (Material sorting)
		ClearFaceWithNoLM( pMB, AllFaces );
		if( AllFaces.size() == 0 )
			return false;
		SortFaceByMaterialId( FaceGroupByMat, AllFaces.begin(), AllFaces.size() );
		if( ! isAllFaceMapped( AllFaces.begin(), AllFaces.size() ) )
		{
			string thetext;
			//thetext = "Object ";
			thetext += ZeNode.GetName();
			thetext = "have not all this faces mapped";
			MessageBox( NULL, thetext.c_str(), "LightMap Warning", MB_OK|MB_ICONERROR );
			return false;
		}

		// PATCH
		FaceGroupByMat.resize(1);
		FaceGroupByMat[0] = AllFaces.size();

		offsetMat = 0;
		for( uint32 nMat = 0; nMat < FaceGroupByMat.size(); ++nMat )
		{
			vector<sint32> FaceGroupBySmooth;

			// Sort faces by smoothing group
			SortFaceBySMoothGroup( FaceGroupBySmooth, AllFaces.begin()+offsetMat, FaceGroupByMat[nMat] );

			offsetSmooth = offsetMat;
			for( uint32 nSmoothNb = 0; nSmoothNb < FaceGroupBySmooth.size(); ++nSmoothNb )
			{
				uint32 nPlaneNb, nLight;
				vector<sint32> FaceGroupByPlane;
				
				if( ! PutFaceUV1InLumelCoord( gOptions.rLumelSize, AllVertices, 
										AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] ) )
					continue;

				SortFaceByPlane( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );
				//AllPlanes.resize( FaceGroupByPlane.size() );

				//SortPlanesBySurface( FaceGroupByPlane, AllFaces.begin()+offsetSmooth, FaceGroupBySmooth[nSmoothNb] );

				AllPlanesPrevSize = AllPlanes.size();
				AllPlanes.resize( AllPlanesPrevSize + FaceGroupByPlane.size() );

				offsetPlane = offsetSmooth;
				for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
				{
					AllPlanes[AllPlanesPrevSize+nPlaneNb] = new SLMPlane;
					AllPlanes[AllPlanesPrevSize+nPlaneNb]->nNbLayerUsed = vvLights.size();
					// Fill planes (part of lightmap)
					CreateLMPlaneFromFaceGroup( *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
												AllFaces.begin()+offsetPlane, FaceGroupByPlane[nPlaneNb] );
					// Next group of face with the same plane in the same smooth group of the same material
					offsetPlane += FaceGroupByPlane[nPlaneNb];
				}
				// Make join between all planes (all planes must be created)

				for( nLight = 0; nLight < vvLights.size(); ++nLight )
				{
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
					{					
						// Light the LightMap for the plane (interior only)
						FirstLight( pMB, *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
									AllVertices, MBMatrix, vvLights[nLight], AllLights,
									nLight, WorldRT );
					}
					// Make extoriors
					SecondLight( pMB, AllPlanes.begin()+AllPlanesPrevSize, FaceGroupByPlane.size(),
								AllVertices, MBMatrix, vvLights[nLight], AllLights,
								nLight, WorldRT );
				}
				if( gOptions.nOverSampling > 1 )
				{
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
						ModifyLMPlaneWithOverSampling( AllPlanes[AllPlanesPrevSize+nPlaneNb],
														gOptions.nOverSampling );
					for( nLight = 0; nLight < vvLights.size(); ++nLight )
					{
						for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
							FirstLight( pMB, *AllPlanes[AllPlanesPrevSize+nPlaneNb], 
										AllVertices, MBMatrix, vvLights[nLight], AllLights,
										nLight, WorldRT );
						SecondLight( pMB, AllPlanes.begin()+AllPlanesPrevSize, FaceGroupByPlane.size(),
									AllVertices, MBMatrix, vvLights[nLight],  AllLights,
									nLight, WorldRT );
					}						
					for( nPlaneNb = 0; nPlaneNb < FaceGroupByPlane.size(); ++nPlaneNb )
						ModifyLMPlaneWithOverSampling( AllPlanes[AllPlanesPrevSize+nPlaneNb],
														1.0/((double)gOptions.nOverSampling) );
				}
				// Next group of face with the same smooth group and the same material
				offsetSmooth += FaceGroupBySmooth[nSmoothNb];
			}
			// Next group of face with the same material
			offsetMat += FaceGroupByMat[nMat];
		}
		
		// Create the lightmap
		
		SLMPlane LightMap;
		SortPlanesBySurface( AllPlanes );
		for( i = 0; i < AllPlanes.size(); ++i )
		{
			// Put in the basis of the plane
			MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
						-AllPlanes[i]->x, -AllPlanes[i]->y );
			PlaceLMPlaneInLMPLane( LightMap, *AllPlanes[i] );
			LightMap.nNbLayerUsed = AllPlanes[i]->nNbLayerUsed;
			// Put in the new basis
			MoveFaceUV1( AllPlanes[i]->faces.begin(), AllPlanes[i]->faces.size(), 
						AllPlanes[i]->x, AllPlanes[i]->y );
			delete AllPlanes[i];
		}
		
		// Save the lightmap				
		// Assign the name of the lightmap and get the complete save name

		// Update UV coords to Texture space
		PutFaceUV1InTextureCoord( LightMap.w, LightMap.h, AllFaces.begin(), AllFaces.size() );
		uint32 nLightMapNb = 0;
		for( j = 0; j < LightMap.nNbLayerUsed; ++j )
		if( (j == 0) || (!LightMap.isAllBlack( (uint8)j )) )
		{
			CTextureFile *pLightMap = new CTextureFile();
			//string sSaveName = AllMeshBuilds[nNode].second->GetName();
			string sSaveName = ZeNode.GetName();
			char tmp[32];
			sprintf( tmp, "%d", nLightMapNb );
			sSaveName += tmp;
			sSaveName += ".tga";
			pLightMap->setFileName( sSaveName );
			sSaveName = gOptions.sExportLighting;
			if( sSaveName[sSaveName.size()-1] != '\\' ) sSaveName += "\\";
			sSaveName += pLightMap->getFileName();
			if( absolutePath )
				pLightMap->setFileName( sSaveName );
			LightMap.copyColToBitmap32( pLightMap, j );
			COFile f( sSaveName );
			pLightMap->writeTGA( f, 32 );	

			for( i = 0; i < pMB->Materials.size(); ++i )
			if( pMB->Materials[i].getShader() == CMaterial::TShader::LightMap )
			{
				pMB->Materials[i].setLightMap( nLightMapNb, pLightMap );
				//AllMeshBuilds[nNode].first->Materials[i].setLighting( false );
				AddLightInfo( pMB, AllLights[vvLights[j].operator[](0)].GroupName, (uint8)i, (uint8)nLightMapNb );
				//////int a = pMB->LightInfoMap.size();
			}
			++nLightMapNb;
		}		
		// Next mesh
	}

	// End of the lighting process for this node we have to export the data
	CMesh::CMeshBuild *pMB = pZeMeshBuild;
	pMB->VertexFlags |= IDRV_VF_UV[1];
	// Build the mesh with the build interface

	for( i = 0; i < pMB->Materials.size(); ++i )
	{
		pMB->Materials[i].setLighting( false );
		pMB->Materials[i].setColor( CRGBA(255,255,255,255) );
	}

	for( i = 0; i < WorldRT.vMB.size(); ++i )
		delete WorldRT.vMB[i];

	return true;	
}

