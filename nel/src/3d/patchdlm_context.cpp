/** \file patchdlm_context.cpp
 * <File description>
 *
 * $Id: patchdlm_context.cpp,v 1.1 2002/04/12 15:59:57 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "3d/patchdlm_context.h"
#include "3d/patch.h"
#include "3d/bezier_patch.h"
#include "3d/point_light.h"
#include "3d/texture_dlm.h"
#include "3d/fast_floor.h"


using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CPatchDLMPointLight::compile(const CPointLight &pl, float maxAttEnd)
{
	nlassert(maxAttEnd>0);

	// copy color
	R= pl.getDiffuse().R;
	G= pl.getDiffuse().G;
	B= pl.getDiffuse().B;
	// Copy Spot/Pos/Dir.
	IsSpot= pl.getType() == CPointLight::SpotLight;
	Pos= pl.getPosition();
	Dir= pl.getSpotDirection();
	
	// compute spot params
	if(IsSpot)
	{
		CosMax= cosf(pl.getSpotAngleBegin());
		CosMin= cosf(pl.getSpotAngleEnd());
	}
	else
	{
		// with tesse Values, we have always (cosSpot-CosMin) * OOCosDelta > 1.0f
		CosMax= -1;
		CosMin= -2;
	}
	OOCosDelta= 1.f / (CosMax-CosMin);

	// compute att params
	AttMax= pl.getAttenuationEnd();
	AttMin= pl.getAttenuationBegin();
	// infinite pointLight?
	if(AttMax==0)
	{
		AttMax= maxAttEnd;
		AttMin= maxAttEnd*0.99f;
	}
	// To big pointLigt?
	else if(AttMax>maxAttEnd)
	{
		AttMax= maxAttEnd;
		AttMin= min(AttMin, maxAttEnd*0.99f);
	}
	// compile distance
	OOAttDelta= 1.f / (AttMin-AttMax);


	// Compute bounding sphere.
	// If not a spot or if angleMin>Pi/2
	if(!IsSpot || CosMin<0)
	{
		// Take sphere of pointlight sphere
		BSphere.Center= Pos;
		BSphere.Radius= AttMax;
		// The bbox englobe the sphere.
		BBox.setCenter(Pos);
		BBox.setHalfSize(CVector(AttMax, AttMax, AttMax));
	}
	else
	{
		// Compute BSphere.
		//==============

		// compute sinus of AngleMin
		float	sinMin= sqrtf(1-sqr(CosMin));

		// Test 2 centers: Center of radius along Dir: Pos+Dir*AttMax/2, and intersection of end cone with line (Pos,Dir)
		// Don't know why but I think they are sufficiently good :)
		// See below for computing of those centers.

		/* compute radius of each sphere by taking max of 3 distances: distance to spotLight center, distance
			to spotLight forward extremity, and distance to spotLight circle interstion Cone/Sphere. (named DCCS)
			NB: Do the compute with radius=1 at first, then multiply later.
		*/
		float	radius1= 0.5f;		// =max(0.5, 0.5); max distance to spot center and extremity center :)
		// for distance DCCS, this is the hypothenuse of (cosMin-0.5) + sinMin.
		float	dccs= sqrtf( sqr(CosMin-0.5f) + sqr(sinMin));
		// take the bigger.
		radius1= max(radius1, dccs );

		// Same reasoning for center2.
		float	radius2= max(CosMin, 1-CosMin);	// max distance to spot center and extremity center :)
		// for distance DCCS, it is simply sinMin!!
		dccs= sinMin;
		// take the bigger.
		radius2= max(radius2, dccs );


		// Then take the center which gives the smaller sphere
		if(radius1<radius2)
		{
			BSphere.Center= Pos + (Dir*0.5f*AttMax);
			// radius1 E [0,1], must take real size.
			BSphere.Radius= radius1 * AttMax;
		}
		else
		{
			BSphere.Center= Pos + (Dir*CosMin*AttMax);
			// radius2 E [0,1], must take real size.
			BSphere.Radius= radius2 * AttMax;
		}


		// Compute BBox.
		//==============
		
		// just take bbox of the sphere, even if not optimal.
		BBox.setCenter(BSphere.Center);
		float	rad= BSphere.Radius;
		BBox.setHalfSize( CVector(rad, rad, rad) );
	}
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CPatchDLMContext::CPatchDLMContext()
{
	_Patch= NULL;
	_DLMTexture= NULL;
	_DLMContextList= NULL;
	OldPointLightCount= 0;
	CurPointLightCount= 0;
	// By default there is crash in textures
	_IsSrcTextureFullBlack= false;
	_IsDstTextureFullBlack= false;
}


// ***************************************************************************
CPatchDLMContext::~CPatchDLMContext()
{
	// release the lightmap in the texture
	if(_DLMTexture)
	{
		_DLMTexture->releaseLightMap(TextPosX, TextPosY);
	}
	// exit
	_Patch= NULL;
	_DLMTexture= NULL;

	// remove it from list.
	if(_DLMContextList)
		_DLMContextList->remove(this);
}

// ***************************************************************************
bool			CPatchDLMContext::generate(CPatch *patch, CTextureDLM *textureDLM, CPatchDLMContextList *ctxList)
{
	nlassert(patch);
	nlassert(textureDLM);
	nlassert(ctxList);

	// keep info on patch/landscape.
	_Patch= patch;
	_DLMTexture= textureDLM;
	// append to the list.
	_DLMContextList= ctxList;
	_DLMContextList->append(this);

	// Get Texture Size info; get coord at cornes of tessBlocks
	Width= (_Patch->getOrderS()/2)+1;
	Height= (_Patch->getOrderT()/2)+1;

	// Allocate space in texture
	if(!_DLMTexture->createLightMap(Width, Height, TextPosX, TextPosY))
	{
		// Mark as not allocated.
		// NB: the context still work with NULL _DLMTexture, but do nothing (excpetionnal case)
		_DLMTexture= NULL;
	}

	// Ig the lightmap is correclty allocated in the global texture, compute UVBias.
	if(_DLMTexture)
	{
		// Compute patch UV matrix from pixels. Must map to center of pixels.
		DLMUScale= (float)(Width-1) / (float)_DLMTexture->getWidth();
		DLMVScale= (float)(Height-1) / (float)_DLMTexture->getHeight();
		DLMUBias= ((float)TextPosX+0.5f) / (float)_DLMTexture->getWidth();
		DLMVBias= ((float)TextPosY+0.5f) / (float)_DLMTexture->getHeight();
	}
	else
	{
		// Build UVBias such that the UVs point to Black
		// TODODO: must do some hints to ensure this... must do it correclty 
		DLMUScale= 0;
		DLMVScale= 0;
		DLMUBias= 1;
		DLMVBias= 1;
	}

	// Allocate RAM Lightmap
	_LightMap.resize(Width*Height);

	// generate Vertices: pos and normals
	_Vertices.resize(Width*Height);
	float	s, t;
	float	ds= 1.0f / (Width-1);
	float	dt= 1.0f / (Height-1);
	// eval all the patch.
	t= 0;
	for(uint y=0; y<Height; y++, t+=dt)
	{
		s= 0;
		for(uint x=0; x<Width; x++, s+=ds)
		{
			CVertex	&vert= _Vertices[y*Width+x];
			// NB: use the bezier patch, and don't take Noise into account, for speed reason.
			CBezierPatch	*bpatch= _Patch->unpackIntoCache();
			// Eval pos.
			vert.Pos= bpatch->eval(s, t);
			// Eval Normal.
			vert.Normal= bpatch->evalNormal(s, t);
		}
	}

	// fill with Black
	clearLighting();

	return true;
}

// ***************************************************************************
void			CPatchDLMContext::clearLighting()
{
	// If the srcTexture is not already black.
	if(!_IsSrcTextureFullBlack)
	{
		// Reset Lightmap with black.
		uint	count= _LightMap.size();
		if(count>0)
		{
			memset(&_LightMap[0], 0, count * sizeof(CRGBA));
		}

		// Now the src lightmap is fully black
		_IsSrcTextureFullBlack= true;
	}
}

// ***************************************************************************
void			CPatchDLMContext::addPointLightInfluence(const CPatchDLMPointLight &pl)
{
	uint		nverts= _Vertices.size();
	nlassert(nverts==_LightMap.size());

	if(nverts==0)
		return;
	CVertex		*vert= &_Vertices[0];

	// process all vertices
	float	r,g,b;
	CRGBA	*dst= &_LightMap[0];
	// If the texture is already black, don't need to add.
	if(_IsSrcTextureFullBlack)
	{
		for(;nverts>0; nverts--, vert++, dst++)
		{
			CVector	dirToP= vert->Pos - pl.Pos;
			float	dist= dirToP.norm();
			dirToP/= dist;

			// compute cos for pl. attenuation
			float	cosSpot= dirToP * pl.Dir;
			float	attSpot= (cosSpot-pl.CosMin) * pl.OOCosDelta;
			clamp(attSpot, 0.f, 1.f);

			// distance attenuation
			float	attDist= (dist-pl.AttMax) * pl.OOAttDelta;
			clamp(attDist, 0.f, 1.f);

			// compute diffuse lighting
			float	diff= -(vert->Normal * dirToP);
			clamp(diff, 0.f, 1.f);
			
			// compute colors.
			diff*= attSpot * attDist;
			r= pl.R*diff;
			g= pl.G*diff;
			b= pl.B*diff;

			CRGBA	col;
			col.R= (uint8)OptFastFloor(r);
			col.G= (uint8)OptFastFloor(g);
			col.B= (uint8)OptFastFloor(b);

			// replace in Map
			*dst= col;
		}
	}
	else
	{
		for(;nverts>0; nverts--, vert++, dst++)
		{
			CVector	dirToP= vert->Pos - pl.Pos;
			float	dist= dirToP.norm();
			dirToP/= dist;

			// compute cos for pl. attenuation
			float	cosSpot= dirToP * pl.Dir;
			float	attSpot= (cosSpot-pl.CosMin) * pl.OOCosDelta;
			clamp(attSpot, 0.f, 1.f);

			// distance attenuation
			float	attDist= (dist-pl.AttMax) * pl.OOAttDelta;
			clamp(attDist, 0.f, 1.f);

			// compute diffuse lighting
			float	diff= -(vert->Normal * dirToP);
			clamp(diff, 0.f, 1.f);
			
			// compute colors.
			diff*= attSpot * attDist;
			r= pl.R*diff;
			g= pl.G*diff;
			b= pl.B*diff;

			CRGBA	col;
			col.R= (uint8)OptFastFloor(r);
			col.G= (uint8)OptFastFloor(g);
			col.B= (uint8)OptFastFloor(b);

			// add to map.
			dst->addRGBOnly(*dst, col);
		}
	}

	// Src texture is modified, hence it can't be black.
	_IsSrcTextureFullBlack= false;
}


// ***************************************************************************
void			CPatchDLMContext::compileLighting()
{
	// If srcTexture is full black, and if dst texture is already full black too, don't need to update dst texture
	if(! (_IsSrcTextureFullBlack && _IsDstTextureFullBlack) )
	{
		// Just Copy into the texture (if lightMap allocated!!)
		if(_LightMap.size()>0 && _DLMTexture)
			_DLMTexture->fillRect(TextPosX, TextPosY, Width, Height, &_LightMap[0]);

		// copy full black state
		_IsDstTextureFullBlack= _IsSrcTextureFullBlack;
	}
}



} // NL3D
