/** \file cloud_scape.cpp
 * cloud_scape implementation
 *
 * $Id: cloud_scape.cpp,v 1.1 2002/10/25 16:22:48 besson Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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
#include "cloud_scape.h"
#include "3d/driver.h"
#include "nel/3d/scissor.h"
#include "nel/3d/viewport.h"

// ------------------------------------------------------------------------------------------------
using namespace NLMISC;

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
#define SQR(x) (x)*(x)

#define MAX_DIST	400.0f
#define MAX_CLOUDS	256
// QUEUE_SIZE must be at least 2*MAX_CLOUDS
#define QUEUE_SIZE	512 

// ------------------------------------------------------------------------------------------------
// SCloudTexture3D
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
SCloudTexture3D::SCloudTexture3D ()
{
	Mem = NULL;
	ToLight.initUnlit();
	ToLight.setShader (CMaterial::Normal);
	//ToLight.setTexture (0, Tex);
	ToLight.setZFunc (CMaterial::always);
	ToLight.setZWrite (false);
	ToLight.texEnvOpRGB (0, CMaterial::Replace);
	ToLight.texEnvArg0RGB (0, CMaterial::Diffuse, CMaterial::SrcColor);
	ToLight.texEnvOpAlpha (0, CMaterial::Replace);
	ToLight.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::InvSrcAlpha);
	ToLight.setBlend (true);
	ToLight.setBlendFunc (CMaterial::invsrcalpha, CMaterial::srcalpha);
	ToLight.setColor (CRGBA(0,0,0,255));

	ToBill.initUnlit();
	ToBill.setZFunc (CMaterial::always);
	ToBill.setZWrite (false);
	ToBill.setDoubleSided(true);

	ToBill.texEnvOpRGB (0, CMaterial::Add);
	ToBill.texEnvArg0RGB (0, CMaterial::Texture, CMaterial::SrcColor);
	ToBill.texEnvArg1RGB (0, CMaterial::Diffuse, CMaterial::SrcColor);
	ToBill.setColor (CRGBA(80,80,80,255));

	ToBill.texEnvOpAlpha (0, CMaterial::Replace);
	ToBill.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);

	ToBill.texEnvOpRGB (1, CMaterial::Modulate);
	ToBill.texEnvArg0RGB (1, CMaterial::Previous, CMaterial::SrcColor);
	ToBill.texEnvArg1RGB (1, CMaterial::Previous, CMaterial::SrcAlpha);
	ToBill.texEnvOpAlpha (1, CMaterial::Replace);
	ToBill.texEnvArg0Alpha (1, CMaterial::Previous, CMaterial::SrcAlpha);

	ToBill.setBlendFunc (CMaterial::one, CMaterial::invsrcalpha);
	ToBill.setBlend (true);
}

// ------------------------------------------------------------------------------------------------
void SCloudTexture3D::init (uint32 nWidth, uint32 nHeight, uint32 nDepth)
{
	Width = raiseToNextPowerOf2 (nWidth);
	Height = raiseToNextPowerOf2 (nHeight);
	Depth = raiseToNextPowerOf2 (nDepth);
	uint32 vdpo2 = getPowerOf2(Depth);
	NbW = 1 << (vdpo2 / 2);
	if ((vdpo2 & 1) != 0)
		NbH = 2 << (vdpo2 / 2);
	else
		NbH = 1 << (vdpo2 / 2);

	Mem = new uint8[NbW*Width*NbH*Height];
	Tex = new CTextureMem (Mem, NbW*Width*NbH*Height, true, false, NbW*Width, NbH*Height, CBitmap::RGBA);

	Tex->setWrapS (ITexture::Clamp);
	Tex->setWrapT (ITexture::Clamp);
	Tex->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	Tex->setReleasable (false);

	ToLight.setTexture (0, Tex);

	ToBill.setTexture(0, Tex);
	ToBill.setTexture(1, Tex);
}

// ------------------------------------------------------------------------------------------------
// SCloudTextureClamp
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
SCloudTextureClamp::SCloudTextureClamp ()
{
	Mem = NULL;
	ToClamp.initUnlit();
	ToClamp.setShader (CMaterial::Normal);
	ToClamp.texEnvOpAlpha (0, CMaterial::Add);
	ToClamp.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);
	ToClamp.texEnvArg1Alpha (0, CMaterial::Diffuse, CMaterial::SrcAlpha);
	ToClamp.setColor (CRGBA(255,255,255,255));
	ToClamp.setBlend (true);
	ToClamp.setBlendFunc (CMaterial::one, CMaterial::one);
	ToClamp.setZFunc (CMaterial::always);
	ToClamp.setZWrite (false);

}

// ------------------------------------------------------------------------------------------------
void SCloudTextureClamp::init (uint32 nWidth, uint32 nHeight, uint32 nDepth, const std::string &filename)
{
	Width = raiseToNextPowerOf2 (nWidth);
	Height = raiseToNextPowerOf2 (nHeight);
	Depth = raiseToNextPowerOf2 (nDepth);
	uint32 vdpo2 = getPowerOf2(Depth);
	NbW = 1 << (vdpo2 / 2);
	if ((vdpo2 & 1) != 0)
		NbH = 2 << (vdpo2 / 2);
	else
		NbH = 1 << (vdpo2 / 2);

	Mem = new uint8[NbW*Width*NbH*Height];
	uint32 i, j;

	if (filename == "")
	{
		// No filename so init with default
		for (i = 0; i < NbW; ++i)
		{
			for (j = 0; j < NbH; ++j)
			{
				uint32 d = i+j*NbW;
				uint32 k, l;
				for (k = 0; k < Width; ++k)
				for (l = 0; l < Height; ++l)
				{
					float x = k+0.5f;
					float y = l+0.5f;
					float z = d+0.5f;
					float xc = Width/2.0f;
					float yc = Height/2.0f;
					float zc = Depth/2.0f;

					float r = (x-xc)*(x-xc)/(Width*Width/4.0f) + (y-yc)*(y-yc)/(Height*Height/4.0f) 
							+ (z-zc)*(z-zc)/(Depth*Depth/4.0f);

					uint8 col = 255;
					if (r < 1.0f)
					{
						col = (uint8)((r)*223+32);
					}
					Mem[i*Width+k + (j*Height+l)*NbW*Width] = col;
				}
			}
		}
	}
	else
	{
		// Load file TODO !
	}

	Tex = new CTextureMem (Mem, NbW*Width*NbH*Height, true, false, NbW*Width, NbH*Height, CBitmap::Alpha);
	Tex->setWrapS (ITexture::Repeat);
	Tex->setWrapT (ITexture::Repeat);
	Tex->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);

	Tex->touch();
	Tex->generate();
	Tex->setReleasable (false);

	ToClamp.setTexture(0, Tex);

}


// ------------------------------------------------------------------------------------------------
// CCloudScape
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
CCloudScape::CCloudScape (NL3D::IDriver *pDriver) : _Noise3D (pDriver)
{
	_Driver = pDriver;
	// Misc purpose VB
	_VertexBuffer.setVertexFormat (CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);
	_VertexBuffer.setNumVertices (4);

	// Material used for cleaning
	_MatClear.initUnlit();
	_MatClear.setDoubleSided (true);
	_MatClear.setZFunc (CMaterial::always);
	_MatClear.setZWrite (false);
	_MatClear.setBlend (false);

	_MatBill.initUnlit();
	_MatBill.setShader (CMaterial::Normal); // not needed

	_MatBill.texEnvOpRGB (0, CMaterial::Replace);
	_MatBill.texEnvArg0RGB (0, CMaterial::Texture, CMaterial::SrcColor);

	_MatBill.texEnvOpAlpha (0, CMaterial::Replace);
	_MatBill.texEnvArg0Alpha (0, CMaterial::Texture, CMaterial::SrcAlpha);

	_MatBill.texEnvOpRGB (1, CMaterial::InterpolateDiffuse);
	_MatBill.texEnvArg0RGB (1, CMaterial::Texture, CMaterial::SrcColor);
	_MatBill.texEnvArg1RGB (1, CMaterial::Previous, CMaterial::SrcColor);

	_MatBill.texEnvOpAlpha (1, CMaterial::InterpolateDiffuse);
	_MatBill.texEnvArg0Alpha (1, CMaterial::Texture, CMaterial::SrcAlpha);
	_MatBill.texEnvArg1Alpha (1, CMaterial::Previous, CMaterial::SrcAlpha);

	_MatBill.setBlend (true);
	_MatBill.setBlendFunc(CMaterial::one, CMaterial::invsrcalpha);
	_MatBill.setZFunc (CMaterial::always);
	_MatBill.setZWrite (false);
	_MatBill.setDoubleSided (true);
	_MatBill.setAlphaTest(true);
	_MatBill.setAlphaTestThreshold(2.0f/256.0f);

	_LODQualityThreshold = 160.0f;
	_IsIncomingCSS = false;
	_DebugQuad = false;
	_NbHalfCloudToUpdate = 1;
}

// ------------------------------------------------------------------------------------------------
CCloudScape::~CCloudScape ()
{
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::init (SCloudScapeSetup *pCSS, NL3D::CCamera *pCamera)
{
	_ViewerCam = pCamera;
	
	_Noise3D.init();

	_AllClouds.resize (MAX_CLOUDS, CCloud(this));
	_CloudPower.resize (MAX_CLOUDS);
	_ShouldProcessCloud.resize (MAX_CLOUDS);

	// For the moment only one clamp texture (generated)
	Tex3DTemp.init (64, 32, 32);
	TexClamp.init (64, 32, 32,"");

	if (pCSS != NULL)
	{
		_CurrentCSS = *pCSS;
		_NewCSS = *pCSS;
		_OldCSS = *pCSS;
	}
	_IsIncomingCSS = false;
	_TimeNewCSS = 60.0*60.0;

	uint32 i;
	for (i = 0; i < MAX_CLOUDS; ++i)
	{
		float newX, newY, newZ, newSizeX, newSizeY, newSizeZ;

		CCloud &c = _AllClouds[i];

		c.setTex3DTemp (Tex3DTemp);
		c.setTexClamp (TexClamp);

		while (true)
		{
			bool bRecalc = false;
			newX = MAX_DIST*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newY = MAX_DIST*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newZ = 85.0f+40.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));

			newSizeX = 60.0f+10.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newSizeY = 30.0f+10.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			newSizeZ = 30.0f+10.0f*(1.0f-2.0f*(((float)rand())/RAND_MAX));
			float f = 0.7f+0.3f*((float)rand())/RAND_MAX;
			newSizeX *= 1.5f*f;
			newSizeY *= 1.5f*f;
			newSizeZ *= 1.5f*f;

			float d = sqrtf(SQR(newX)+SQR(newY));
			if (d > MAX_DIST) bRecalc = true;

			float r1 = sqrtf(SQR(newSizeX/2)+SQR(newSizeY/2)+SQR(newSizeZ/2));
			for (uint32 k = 0;k < i; ++k)
			{
				CCloud &c2 = _AllClouds[k];

				if ((fabs(newX-c2.getX()) < (newSizeX/2+c2.getSizeX()/2)) && 
					(fabs(newY-c2.getY()) < (newSizeY/2+c2.getSizeY()/2)) && 
					(fabs(newZ-c2.getZ()) < (newSizeZ/2+c2.getSizeZ()/2)))
					bRecalc = true;
			}
			if (!bRecalc) break;
		}

		c.init (64, 32, 32, 0.122f, 4);
		c.setX (newX-newSizeX/2);
		c.setY (newY-newSizeY/2);
		c.setZ (newZ-newSizeZ/2);

		c.setSizeX (newSizeX);
		c.setSizeY (newSizeY);
		c.setSizeZ (newSizeZ);

		c.Time = 0;
		c.FuturTime = _CurrentCSS.NbCloud * 2 * (0.04/_NbHalfCloudToUpdate);
		if (i < _CurrentCSS.NbCloud)
		{
			_CloudPower[i] = 255;
			_ShouldProcessCloud[i] = true;
		}
		else
		{
			_CloudPower[i] = 0;
			_ShouldProcessCloud[i] = false;
		}
	}

	_SortedClouds.resize (MAX_CLOUDS);

	_CloudSchedulerSize = _CurrentCSS.NbCloud;
	_CloudSchedulerLastAdded.resize (MAX_CLOUDS);
	_FrameCounter = 0;
	for (i = 0; i < QUEUE_SIZE; ++i)
	{
		sint32 nCloudNb = i%_CurrentCSS.NbCloud;
		SCloudSchedulerEntry cse;
		cse.CloudIndex = nCloudNb;
		if (_CloudSchedulerLastAdded[nCloudNb].ValidPos == true)
		{
			SCloudSchedulerEntry &lastCSE = *_CloudSchedulerLastAdded[nCloudNb].Pos;
			sint32 delta = _FrameCounter - lastCSE.Frame;
			lastCSE.DeltaNextCalc = delta;
		}
		cse.Frame = _FrameCounter;
		cse.Ambient = _CurrentCSS.Ambient;
		cse.Diffuse = _CurrentCSS.Diffuse;
		cse.Power = _CloudPower[cse.CloudIndex];
		_CloudScheduler.push_back (cse);
		_CloudSchedulerLastAdded[nCloudNb].ValidPos = true;
		_CloudSchedulerLastAdded[nCloudNb].Pos = _CloudScheduler.end()-1;
		++_FrameCounter;
	}
	_GlobalTime = 0.0f;
	_DTRest = 0.0f;
	_Generate = true;
	_AverageFrameRate.init(16);
	for (i = 0; i < 16; ++i)
		_AverageFrameRate.addValue (40.0f/1000.0f);

	_ExtrapolatedPriorities.resize (MAX_CLOUDS);

	for (i = 0; i < QUEUE_SIZE; ++i)
		anim (41.0/1000.0, _ViewerCam);
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::set (SCloudScapeSetup &css)
{
	_IncomingCSS = css;
	_IsIncomingCSS = true;
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::anim (double dt, NL3D::CCamera *pCamera)
{
	sint32 i;

	_ViewerCam = pCamera;

	// 10 fps -> 200 fps
	if (dt > 0.1) dt = 0.1;
	if (dt < 0.005) dt = 0.005;

	_DeltaTime = dt;
	_GlobalTime += _DeltaTime;
	_AverageFrameRate.addValue ((float)_DeltaTime);

	// Animate the CSS
	if (_TimeNewCSS > _NewCSS.TimeToChange)
	{
		_CurrentCSS = _NewCSS;
		_OldCSS = _NewCSS;
		if (_IsIncomingCSS)
		{
			_IsIncomingCSS = false;
			_NewCSS = _IncomingCSS;
			_TimeNewCSS = 0;
			if (_NewCSS.NbCloud > _OldCSS.NbCloud)
			for (i = 0; i < (sint32)(_NewCSS.NbCloud-_OldCSS.NbCloud); ++i)
			{
				CCloud &c = _AllClouds[_OldCSS.NbCloud+i];
				c.CloudPower = 0;
				_CloudPower[_OldCSS.NbCloud+i] = 0;
			}
		}
	}
	else
	{
		float inter = (float)(_TimeNewCSS / _NewCSS.TimeToChange);
		_CurrentCSS.WindSpeed = (_NewCSS.WindSpeed - _OldCSS.WindSpeed)*inter + _OldCSS.WindSpeed;
		_CurrentCSS.CloudSpeed = (_NewCSS.CloudSpeed - _OldCSS.CloudSpeed)*inter + _OldCSS.CloudSpeed;

		_CurrentCSS.Ambient.R = (uint8)((_NewCSS.Ambient.R - _OldCSS.Ambient.R)*inter + _OldCSS.Ambient.R);
		_CurrentCSS.Ambient.G = (uint8)((_NewCSS.Ambient.G - _OldCSS.Ambient.G)*inter + _OldCSS.Ambient.G);
		_CurrentCSS.Ambient.B = (uint8)((_NewCSS.Ambient.B - _OldCSS.Ambient.B)*inter + _OldCSS.Ambient.B);
		_CurrentCSS.Ambient.A = (uint8)((_NewCSS.Ambient.A - _OldCSS.Ambient.A)*inter + _OldCSS.Ambient.A);

		_CurrentCSS.Diffuse.R = (uint8)((_NewCSS.Diffuse.R - _OldCSS.Diffuse.R)*inter + _OldCSS.Diffuse.R);
		_CurrentCSS.Diffuse.G = (uint8)((_NewCSS.Diffuse.G - _OldCSS.Diffuse.G)*inter + _OldCSS.Diffuse.G);
		_CurrentCSS.Diffuse.B = (uint8)((_NewCSS.Diffuse.B - _OldCSS.Diffuse.B)*inter + _OldCSS.Diffuse.B);
		_CurrentCSS.Diffuse.A = (uint8)((_NewCSS.Diffuse.A - _OldCSS.Diffuse.A)*inter + _OldCSS.Diffuse.A);

		if (_NewCSS.NbCloud > _OldCSS.NbCloud)
		{
			// Add some clouds
			float slice = (_NewCSS.TimeToChange/4) / (_NewCSS.NbCloud-_OldCSS.NbCloud);
			sint32 diffCloud = _NewCSS.NbCloud-_OldCSS.NbCloud;

			_CurrentCSS.NbCloud = _OldCSS.NbCloud + (1+(uint32)(_TimeNewCSS/slice));
			if (_CurrentCSS.NbCloud > _NewCSS.NbCloud)
				_CurrentCSS.NbCloud = _NewCSS.NbCloud;

			for (i = 0; i < diffCloud; ++i)
			{
				_ShouldProcessCloud[_OldCSS.NbCloud+i] = true;
				if (_TimeNewCSS < i*slice)
					_CloudPower[_OldCSS.NbCloud+i] = 1;
				else if (_TimeNewCSS > (i*slice+3*_NewCSS.TimeToChange/4))
					_CloudPower[_OldCSS.NbCloud+i] = 255;
				else
					_CloudPower[_OldCSS.NbCloud+i] = (uint8)(255*(_TimeNewCSS-i*slice)/(3*_NewCSS.TimeToChange/4));
			}
		}
		else
		{
			// Remove some clouds
			float slice = (_NewCSS.TimeToChange/4) / (_OldCSS.NbCloud-_NewCSS.NbCloud);
			sint32 diffCloud = _OldCSS.NbCloud-_NewCSS.NbCloud;

			_CurrentCSS.NbCloud = _OldCSS.NbCloud;

			for (i = 0; i < diffCloud; ++i)
			{
				if (_TimeNewCSS < i*slice)
					_CloudPower[_OldCSS.NbCloud-i-1] = 255;
				else if (_TimeNewCSS > (i*slice+3*_NewCSS.TimeToChange/4))
					_CloudPower[_OldCSS.NbCloud-i-1] = 0;
				else
					_CloudPower[_OldCSS.NbCloud-i-1] = (uint8)(255-255*(_TimeNewCSS-i*slice)/(3*_NewCSS.TimeToChange/4));
			}
		}
	}

	// Make the right number of half cloud
	_DTRest += dt;

	while (_DTRest > (0.04/_NbHalfCloudToUpdate))
	{
		makeHalfCloud ();
		_DTRest -= 0.04/_NbHalfCloudToUpdate;

		for (i = 0; i < MAX_CLOUDS; ++i)
		{
			CCloud &c = _AllClouds[i];
			c.Time += 0.04/_NbHalfCloudToUpdate;
		}

		_TimeNewCSS += 0.04/_NbHalfCloudToUpdate;
	}
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::makeHalfCloud ()
{
	CVector Viewer = CVector(0,0,0); //_ViewerCam->getMatrix().getPos();

	if (_Generate)
	{
		// Find the next cloud in the list
		SCloudSchedulerEntry FrontCSE;

		FrontCSE = _CloudScheduler.front();

		// Is the cloud do not have another reference in the list add it now because it should be processed
		sint32 CloudIndexToAdd = -1;
		if ((_ShouldProcessCloud[FrontCSE.CloudIndex] == true) && 
			(	(_CloudSchedulerLastAdded[FrontCSE.CloudIndex].ValidPos == false) ||
				((_CloudSchedulerLastAdded[FrontCSE.CloudIndex].ValidPos == true) &&
				(_CloudSchedulerLastAdded[FrontCSE.CloudIndex].Pos == _CloudScheduler.begin()))
			))
		{
			// It should be added now !
			CloudIndexToAdd = FrontCSE.CloudIndex;
			FrontCSE.DeltaNextCalc = QUEUE_SIZE;
		}
		else
		{
			// Choose a Cloud Index To Add at the end of the list
			uint32 nPeriodeMax = _CurrentCSS.NbCloud+_CurrentCSS.NbCloud/10;
			sint32 Priority = -10000;
			uint32 i;

			float sumPrior = 0.0f;
			for (i = 0; i < MAX_CLOUDS; ++i)
			if (_ShouldProcessCloud[i])
			{
				CCloud &rC = _AllClouds[i];
				float ExtrapolatedTime = ((0.04f/_NbHalfCloudToUpdate) * QUEUE_SIZE * 2);
				float x = rC.getLastX () + ExtrapolatedTime * _CurrentCSS.WindSpeed;
				//float d = sqrtf(SQR(x+rC.getSizeX()/2-Viewer.x)+SQR(rC.getY()+rC.getSizeY()/2-Viewer.y)+
				//		SQR(rC.getZ()+rC.getSizeZ()/2-Viewer.z));
				float d = SQR(x+rC.getSizeX()/2-Viewer.x)+SQR(rC.getY()+rC.getSizeY()/2-Viewer.y)+
						SQR(rC.getZ()+rC.getSizeZ()/2-Viewer.z);
				float d05 = sqrtf(d);
				float d025 = sqrtf(d05);
				float d075 = d05*d025;

				_ExtrapolatedPriorities[i] = 1.0f / d075;
				sumPrior += _ExtrapolatedPriorities[i];
			}

			sint32 sumJeton = 0;
			for (i = 0; i < MAX_CLOUDS; ++i)
			if (_ShouldProcessCloud[i])
			{
				// Normalize priorities
				float factor = ((float)QUEUE_SIZE) / sumPrior;
				sint32 nbJeton = (sint32)(0.5f+(factor * _ExtrapolatedPriorities[i]));

				if (nbJeton < 1)
					nbJeton = 1;

				_ExtrapolatedPriorities[i] = (float)nbJeton;
				sumJeton += nbJeton;
			}

			if (sumJeton > QUEUE_SIZE)
			{
				do
				{
					for (i = 0; i < MAX_CLOUDS; ++i)
					if (_ShouldProcessCloud[i])
					{
						if (_ExtrapolatedPriorities[i] > 1)
						{
							_ExtrapolatedPriorities[i] -= 1;
							--sumJeton;
							if (sumJeton == QUEUE_SIZE) break;
						}
					}
				}
				while (sumJeton > QUEUE_SIZE);
			}

			for (i = 0; i < MAX_CLOUDS; ++i)
			if (_ShouldProcessCloud[i])
			{				
				// Cloud Period
				sint32 newPriority = nPeriodeMax;
				// Is there a last entry in array ?
				if (_CloudSchedulerLastAdded[i].ValidPos == true)
				{
					SCloudSchedulerEntry &rLastCSE = *_CloudSchedulerLastAdded[i].Pos;
					newPriority = (sint32)(QUEUE_SIZE/_ExtrapolatedPriorities[i]);
					newPriority = (_FrameCounter -  rLastCSE.Frame) - newPriority;
				}
				else
				{
					newPriority = 10000;
				}
				if (newPriority > Priority)
				{
					Priority = newPriority;
					CloudIndexToAdd = i;
				}
			}
			nlassert (CloudIndexToAdd != -1);
		}

		// Ok now we have a good cloud index to add so make the new cloud entry
		SCloudSchedulerEntry newCSE;

		newCSE.CloudIndex = CloudIndexToAdd;
		newCSE.Frame = _FrameCounter;
		newCSE.Ambient = _CurrentCSS.Ambient;
		newCSE.Diffuse = _CurrentCSS.Diffuse;
		newCSE.Power = _CloudPower[CloudIndexToAdd];

		// If the cloud where added previously to the list
		if (_CloudSchedulerLastAdded[CloudIndexToAdd].ValidPos == true)
		{
			// This means that the cloud were added from a long time ago
			SCloudSchedulerEntry &lastCSE = *_CloudSchedulerLastAdded[CloudIndexToAdd].Pos;
			sint32 delta = _FrameCounter - lastCSE.Frame;			
			lastCSE.DeltaNextCalc = delta;

			// But the cloud can be removed (if so we have to not process it anymore)
			if (newCSE.Power == 0)
				_ShouldProcessCloud[CloudIndexToAdd] = false;
		}
		else
		{
			// No the cloud do not appear previously in the list... So its a new one
			_AllClouds[CloudIndexToAdd].reset (_ViewerCam);
		}

		// If the last cloud occurence of the cloud appear at beginning so no more occurence in list
		if (_CloudSchedulerLastAdded[FrontCSE.CloudIndex].Pos == _CloudScheduler.begin())
			_CloudSchedulerLastAdded[FrontCSE.CloudIndex].ValidPos = false;

		_CloudScheduler.push_back (newCSE);
		_CloudSchedulerLastAdded[CloudIndexToAdd].ValidPos = true;
		_CloudSchedulerLastAdded[CloudIndexToAdd].Pos = _CloudScheduler.end()-1;
		_CloudScheduler.pop_front ();
		++_FrameCounter;
		// End of scheduling

		// Get the cloud to process (this must be the next occurence of front cloud)
		std::deque<SCloudSchedulerEntry>::iterator it = _CloudScheduler.begin();
		while (it != _CloudScheduler.end())
		{
			SCloudSchedulerEntry &rCSE = *it;
			if (rCSE.CloudIndex == FrontCSE.CloudIndex)
				break;
			++it;
		}

		SCloudSchedulerEntry CSEToCalc;
		// The cloud is no more present in the list
		if (it == _CloudScheduler.end())
		{
			FrontCSE.DeltaNextCalc = 1;
			CSEToCalc = FrontCSE;
		}
		else
		{
			CSEToCalc = *it;
		}

		_CurrentCloudInProcess = &_AllClouds[CSEToCalc.CloudIndex];
		CCloud &c = *_CurrentCloudInProcess;

		// To go from Front cloud to CSEToCalc cloud we should take the front DeltaNextCalc

		_CurrentCloudInProcessFuturTime = ((0.04/_NbHalfCloudToUpdate) * FrontCSE.DeltaNextCalc * 2);
		c.setX ((float)(c.getLastX() +  _CurrentCloudInProcessFuturTime * _CurrentCSS.WindSpeed));

		float d2D = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y));

		if (d2D > MAX_DIST)
			c.CloudDistAtt = 255;
		else if (d2D > (MAX_DIST-100.0f))
			c.CloudDistAtt = (uint8)(255*((d2D-(MAX_DIST-100.0f))/100.0f));
		else
			c.CloudDistAtt = 0;

		c.LastCloudPower = c.CloudPower;
		c.CloudPower = CSEToCalc.Power;
		c.CloudDiffuse = CSEToCalc.Diffuse;
		c.CloudAmbient = CSEToCalc.Ambient;

		c.anim (_CurrentCloudInProcessFuturTime*_CurrentCSS.CloudSpeed, 
				_CurrentCloudInProcessFuturTime*_CurrentCSS.WindSpeed);

		c.generate (_Noise3D);

	}
	else
	{
		CCloud &c = *_CurrentCloudInProcess;

		c.Time = 0;
		c.FuturTime = _CurrentCloudInProcessFuturTime;
		c.light();

		if (c.getX() > MAX_DIST)
		{
			c.setX (c.getX() - (2 * MAX_DIST));
			c.setLooping ();
		}
		
		float r = sqrtf(SQR(c.getSizeX()/2)+SQR(c.getSizeY()/2)+SQR(c.getSizeZ()/2));
		float d2D = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y));
		float d = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y)+
						SQR(c.getZ()+c.getSizeZ()/2-Viewer.z));
		uint32 lookAtSize = (uint32)(_LODQualityThreshold*r/d);
		lookAtSize = raiseToNextPowerOf2 (lookAtSize);
		if (lookAtSize > 128) lookAtSize = 128;

		c.genBill (_ViewerCam, lookAtSize);
	}
	_Generate = !_Generate;
}

// ------------------------------------------------------------------------------------------------
void CCloudScape::render ()
{
	uint32 i, j;
	
	CVector Viewer = CVector (0,0,0);

	CMatrix viewMat;
	viewMat = _ViewerCam->getMatrix ();
	viewMat.setPos(CVector(0,0,0));
	viewMat.invert ();
	CScissor s;
	s.initFullScreen ();
	_Driver->setupScissor (s);
	CViewport v;
	_Driver->setupViewport (v);
	CFrustum f = _ViewerCam->getFrustum();
	_Driver->setFrustum (f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
	_Driver->setupViewMatrix (viewMat);
	_Driver->setupModelMatrix (CMatrix::Identity);

	uint32 nNbCloudToRender = 0;

	for (i = 0; i < MAX_CLOUDS; ++i)
	{
		CCloud &c = _AllClouds[i];
		SSortedCloudEntry &sce = _SortedClouds[nNbCloudToRender];
		sce.Cloud = &c;
		sce.Distance = sqrtf(SQR(c.getX()+c.getSizeX()/2-Viewer.x)+SQR(c.getY()+c.getSizeY()/2-Viewer.y)+
						SQR(c.getZ()+c.getSizeZ()/2-Viewer.z));
		nNbCloudToRender++;
	}

	for (i = 0; i < nNbCloudToRender-1; ++i)
	for (j = i+1; j < nNbCloudToRender; ++j)
	{
		if (_SortedClouds[i].Distance < _SortedClouds[j].Distance)
		{
			SSortedCloudEntry sceTmp = _SortedClouds[i];
			_SortedClouds[i] = _SortedClouds[j];
			_SortedClouds[j] = sceTmp;
		}
	}

	for (i = 0; i < nNbCloudToRender; ++i)
	{
		CCloud *pC = _SortedClouds[i].Cloud;
		if ((pC->CloudPower > 0) || (pC->LastCloudPower > 0))
			pC->dispBill (_ViewerCam);
	}
}

// ------------------------------------------------------------------------------------------------
uint32 CCloudScape::getMemSize()
{
	uint32 nMemSize = 0;
	for (uint32 i = 0; i < MAX_CLOUDS; ++i)
	{
		CCloud &c = _AllClouds[i];
		nMemSize += c.getMemSize();
	}
	return nMemSize;
}

} // namespace NL3D