/** \file patchdlm_context.cpp
 * <File description>
 *
 * $Id: patchdlm_context.cpp,v 1.2 2002/04/16 09:44:03 berenguier Exp $
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
#define	NL_DLM_CLIP_FACTOR		1
#define	NL_DLM_CLIP_NUM_LEVEL	3

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
	uint	x,y;
	for(y=0; y<Height; y++, t+=dt)
	{
		s= 0;
		for(x=0; x<Width; x++, s+=ds)
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

	// Build bounding Spheres QuadTree
	//============

	// Size of the cluster array (at level 0)
	uint	bsx, bsy;
	bsx= max(1, (_Patch->getOrderS()/2)/NL_DLM_CLIP_FACTOR );
	bsy= max(1, (_Patch->getOrderT()/2)/NL_DLM_CLIP_FACTOR );

	// resize bboxes for level 0.
	static	vector<CAABBox>		tmpBBoxes[NL_DLM_CLIP_NUM_LEVEL];
	tmpBBoxes[0].resize(bsx * bsy);

	// Extend all leaves clusters BBoxes with patch coordinates
	for(y=0;y<bsy;y++)
	{
		// For Y, compute how many patch Positions used to extend bbox.
		uint	beginY= y*NL_DLM_CLIP_FACTOR;
		uint	endY= min( (y+1)*NL_DLM_CLIP_FACTOR+1, Height);
		for(x=0;x<bsx;x++)
		{
			// For X, compute how many patch Positions used to extend bbox.
			uint	beginX= x*NL_DLM_CLIP_FACTOR;
			uint	endX= min((x+1)*NL_DLM_CLIP_FACTOR+1, Width);
			// Build a bbox.
			CAABBox		bbox;
			bbox.setCenter(_Vertices[beginY*Width + beginX].Pos);
			for(uint yi= beginY; yi<endY; yi++)
			{
				for(uint xi= beginX; xi<endX; xi++)
				{
					bbox.extend(_Vertices[yi*Width + xi].Pos);
				}
			}
			// Set the BBox info.
			tmpBBoxes[0][y*bsx + x]= bbox;
		}
	}

	// build parent BSpheres for quadTree hierarchy
	uint	curLevel= 0;
	uint	nextLevel= 1;
	uint	nextBsx= max(1U, bsx/2);
	uint	nextBsy= max(1U, bsy/2);
	// the number of cluster Sons, and descendants this cluster level owns.
	uint	tmpClusterNumToSkip[NL_DLM_CLIP_NUM_LEVEL];
	// width for this cluster level.
	uint	tmpClusterWidth[NL_DLM_CLIP_NUM_LEVEL];
	// Number of sons per line/column
	uint	tmpClusterWSon[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterHSon[NL_DLM_CLIP_NUM_LEVEL];
	// Fill level 0 info
	tmpClusterNumToSkip[0]= 0;
	tmpClusterWidth[0]= bsx;
	tmpClusterWSon[0]= 0;
	tmpClusterHSon[0]= 0;
	uint	finalClusterSize= bsx * bsy;

	// If the next level has 1x1 cases, it is not usefull (since same sphere as entire Patch)
	while(nextBsx * nextBsy > 1 && nextLevel<NL_DLM_CLIP_NUM_LEVEL )
	{
		finalClusterSize+= nextBsx * nextBsy;

		uint	wSon= (bsx/nextBsx);
		uint	hSon= (bsy/nextBsy);
		// compute cluster level info.
		tmpClusterWidth[nextLevel]= nextBsx;
		tmpClusterWSon[nextLevel]= wSon;
		tmpClusterHSon[nextLevel]= hSon;
		// NB: level 0 has 0 sons to skip, hence level1 must skip (1+0)*4= 4  (wSon==hSon==2)
		// level2 must skip (1+4)*4= 20    (wSon==hSon==2)
		tmpClusterNumToSkip[nextLevel]= (1+tmpClusterNumToSkip[curLevel]) * wSon * hSon;

		// alloc bboxes.
		tmpBBoxes[nextLevel].resize(nextBsx * nextBsy);

		// For all cluster of upper level, build bb, as union of finers clusters
		for(y=0;y<nextBsy;y++)
		{
			for(x=0;x<nextBsx;x++)
			{
				// compute coordinate in curLevel tmpBBoxes to look
				uint	x2= x*wSon;
				uint	y2= y*hSon;
				// Build a bbox for 4 (or 2) children clusters
				if(wSon>1 && hSon>1)
				{
					CAABBox		bbox1;
					CAABBox		bbox2;
					bbox1= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][y2*bsx + x2], tmpBBoxes[curLevel][y2*bsx + x2+1]);
					bbox2= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][(y2+1)*bsx + x2], tmpBBoxes[curLevel][(y2+1)*bsx + x2+1]);
					// final father bbox.
					tmpBBoxes[nextLevel][y*nextBsx + x]= CAABBox::computeAABBoxUnion(bbox1, bbox2);
				}
				else if(wSon==1)
				{
					CAABBox		bbox1;
					bbox1= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][y2*bsx + x2], tmpBBoxes[curLevel][(y2+1)*bsx + x2]);
					// final father bbox.
					tmpBBoxes[nextLevel][y*nextBsx + x]= bbox1;
				}
				else if(hSon==1)
				{
					CAABBox		bbox1;
					bbox1= CAABBox::computeAABBoxUnion(
						tmpBBoxes[curLevel][y2*bsx + x2], tmpBBoxes[curLevel][y2*bsx + x2+1]);
					// final father bbox.
					tmpBBoxes[nextLevel][y*nextBsx + x]= bbox1;
				}
				else
					// impossible...
					nlstop;
			}
		}

		// upper level.
		bsx= nextBsx;
		bsy= nextBsy;
		nextBsx= max(1U, nextBsx/2);
		nextBsy= max(1U, nextBsy/2);
		curLevel++;
		nextLevel++;
	}


	// Resize clusters with size according to all levels
	_Clusters.resize(finalClusterSize);
	uint	iDstCluster= 0;

	// Fill cluster hierarchy, in _Clusters.
	uint	numLevels= nextLevel;
	// NB: the principle is recursive, but it is "iterated", with a stack-like: tmpClusterX and tmpClusterY;
	uint	tmpClusterX[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterY[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterXMin[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterYMin[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterXMax[NL_DLM_CLIP_NUM_LEVEL];
	uint	tmpClusterYMax[NL_DLM_CLIP_NUM_LEVEL];
	// we start at curLevel (the highest Level), and we must fill all the squares of this level
	tmpClusterX[curLevel]= 0;
	tmpClusterY[curLevel]= 0;
	tmpClusterXMin[curLevel]= 0;
	tmpClusterYMin[curLevel]= 0;
	tmpClusterXMax[curLevel]= bsx;
	tmpClusterYMax[curLevel]= bsy;
	// while the "root" level is not pop
	while(curLevel < numLevels)
	{
		// If we ended with this level (all lines done).
		if(tmpClusterY[curLevel] >= tmpClusterYMax[curLevel])
		{
			// Ok, finished with this level, pop up.
			curLevel++;
			// skip.
			continue;
		}

		nlassert(iDstCluster<_Clusters.size());

		// get the bbox from current position.
		CAABBox	bbox= tmpBBoxes[curLevel][ tmpClusterY[curLevel] * tmpClusterWidth[curLevel] + tmpClusterX[curLevel] ];
		// Fill _Clusters for this square.
		_Clusters[iDstCluster].BSphere.Center= bbox.getCenter(); 
		_Clusters[iDstCluster].BSphere.Radius= bbox.getRadius(); 
		// If leaf level, fill special info
		if(curLevel == 0)
		{
			_Clusters[iDstCluster].NSkips= 0;
			_Clusters[iDstCluster].X= tmpClusterX[0];
			_Clusters[iDstCluster].Y= tmpClusterY[0];
		}
		// else, set total number of sons to skips if "invisible"
		else
			_Clusters[iDstCluster].NSkips= tmpClusterNumToSkip[curLevel];

		// next dst cluster
		iDstCluster ++;


		// If not Leaf level, recurs. First pass, use curLevel params (tmpClusterX...)
		if(curLevel > 0)
		{
			// compute info for next level.
			tmpClusterXMin[curLevel-1]= tmpClusterX[curLevel] * tmpClusterWSon[curLevel];
			tmpClusterYMin[curLevel-1]= tmpClusterY[curLevel] * tmpClusterHSon[curLevel];
			tmpClusterXMax[curLevel-1]= (tmpClusterX[curLevel]+1) * tmpClusterWSon[curLevel];
			tmpClusterYMax[curLevel-1]= (tmpClusterY[curLevel]+1) * tmpClusterHSon[curLevel];
			// begin iteration of child level
			tmpClusterX[curLevel-1]= tmpClusterXMin[curLevel-1];
			tmpClusterY[curLevel-1]= tmpClusterYMin[curLevel-1];
		}


		// next square for this level
		tmpClusterX[curLevel]++;
		// if ended for X.
		if(tmpClusterX[curLevel] >= tmpClusterXMax[curLevel])
		{
			// reset X.
			tmpClusterX[curLevel]= tmpClusterXMin[curLevel];
			// next line.
			tmpClusterY[curLevel]++;
		}


		// If not Leaf level, recurs. Second pass, after tmpClusterX and tmpClusterY of curLevel are changed
		if(curLevel > 0)
		{
			// descend in hierarchy. (recurs)
			curLevel--;
		}

	}

	// All dst clusters must have been filled
	nlassert(iDstCluster == _Clusters.size());


	// fill texture with Black
	//============
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


	// precise clip: parse the quadTree of sphere
	//================
	uint	i, x,y;
	uint	startX, startY, endX, endY;
	startX= 0xFFFFFFFF;
	startY= 0xFFFFFFFF;
	endX= 0;
	endY= 0;
	for(i=0;i<_Clusters.size();)
	{
		// If the sphere intersect pl, 
		if(_Clusters[i].BSphere.intersect(pl.BSphere) )
		{
			// if this cluster is a leaf, extend start/end
			if(_Clusters[i].NSkips==0)
			{
				x= _Clusters[i].X;
				y= _Clusters[i].Y;
				startX= min(startX, x);
				startY= min(startY, y);
				endX= max(endX, x+1);
				endY= max(endY, y+1);
			}
			// go to next cluster (a brother, a parent or a son)
			i++;
		}
		else
		{
			// if this cluster is a leaf, just go to next cluster (a parent or a brother)
			if(_Clusters[i].NSkips==0)
				i++;
			// else, go to next brother or parent (NSkips say how to go)
			else
				i+= _Clusters[i].NSkips;
		}
	}
	// if never intersect, just quit.
	if(startX==0xFFFFFFFF)
		return;

	// get vertices in array to process.
	startX*=NL_DLM_CLIP_FACTOR;
	startY*=NL_DLM_CLIP_FACTOR;
	endX= min(endX*NL_DLM_CLIP_FACTOR+1, Width);
	endY= min(endY*NL_DLM_CLIP_FACTOR+1, Height);


	// process all vertices
	//================
	float	r,g,b;
	CRGBA	*dst= &_LightMap[0];
	// If the texture is already black, don't need to add to previous color
	/*if(_IsSrcTextureFullBlack)
	{
		// TODO_OPTIM
	}
	else*/
	{
		CVertex		*originVert= vert;
		CRGBA		*originDst= dst;
		for(y=startY; y<endY; y++)
		{
			nverts= endX - startX;
			// TempYoyo
			/*extern uint YOYO_LandDLCount;
			YOYO_LandDLCount+= nverts;*/

			vert= originVert + startX + y*Width;
			dst= originDst + startX + y*Width;
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
