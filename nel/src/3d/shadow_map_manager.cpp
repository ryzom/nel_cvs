/** \file shadow_map_manager.cpp
 * <File description>
 *
 * $Id: shadow_map_manager.cpp,v 1.5 2004/01/15 17:33:18 lecroart Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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
#include "3d/shadow_map_manager.h"
#include "nel/misc/aabbox.h"
#include "3d/driver.h"
#include "3d/scene.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "3d/dru.h"
#include "3d/texture_mem.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/fast_floor.h"


using namespace NLMISC;
using namespace std;

namespace NL3D {


// ***************************************************************************
// easineasout
static inline float	easeInEaseOut(float x)
{
	float	y;
	// cubic such that f(0)=0, f'(0)=0, f(1)=1, f'(1)=0.
	float	x2=x*x;
	float	x3=x2*x;
	y= -2*x3 + 3*x2;
	return y;
}


// ***************************************************************************
CShadowMapManager::CShadowMapManager()
{
	uint	i;

	setQuadGridSize(NL3D_SMM_QUADGRID_SIZE, NL3D_SMM_QUADCELL_SIZE);
	_ShadowCasters.reserve(256);
	_GenerateShadowCasters.reserve(256);
	_NumShadowReceivers= 0;
	_PolySmooth= true;

	// **** Setup Fill
	_FillQuads.setVertexFormat(CVertexBuffer::PositionFlag);
	_FillMaterial.initUnlit();
	_FillMaterial.setColor(CRGBA(0,0,0,0));
	_FillMaterial.setZWrite(false);
	_FillMaterial.setZFunc(CMaterial::always);
	_FillMaterial.setDoubleSided(true);

	// **** Setup Blur
	_BlurQuads.setVertexFormat(CVertexBuffer::PositionFlag |
		CVertexBuffer::TexCoord0Flag |
		CVertexBuffer::TexCoord1Flag |
		CVertexBuffer::TexCoord2Flag |
		CVertexBuffer::TexCoord3Flag);
	// Only 2 quads are used to blur
	_BlurQuads.setNumVertices(8);
	_BlurMaterial.initUnlit();
	_BlurMaterial.setColor(CRGBA::White);
	_BlurMaterial.setZWrite(false);
	_BlurMaterial.setZFunc(CMaterial::always);
	_BlurMaterial.setDoubleSided(true);
	// Setup The Blur. NB: it will take advantage of Max 4 texture driver support, but will still 
	// work with 2 or 3 (less beautifull).
	for(i=1;i<4;i++)
	{
		_BlurMaterial.texEnvOpRGB(i, CMaterial::InterpolateConstant);
		_BlurMaterial.texEnvArg0RGB(i, CMaterial::Texture, CMaterial::SrcColor);
		_BlurMaterial.texEnvArg1RGB(i, CMaterial::Previous, CMaterial::SrcColor);
		_BlurMaterial.texEnvOpAlpha(i, CMaterial::InterpolateConstant);
		_BlurMaterial.texEnvArg0Alpha(i, CMaterial::Texture, CMaterial::SrcAlpha);
		_BlurMaterial.texEnvArg1Alpha(i, CMaterial::Previous, CMaterial::SrcAlpha);
	}
	// Factor for Stage so the sum is 1.
	_BlurMaterial.texConstantColor(1, CRGBA(128,128,128,128));		// factor= 1/2
	_BlurMaterial.texConstantColor(2, CRGBA(85,85,85,85));			// factor= 1/3
	_BlurMaterial.texConstantColor(3, CRGBA(64,64,64,64));			// factor= 1/4

	_BlurTextureW= 0;
	_BlurTextureH= 0;

	// **** Setup Receiving

	// Setup the clamp texture.
	const	uint	clampTextSize= 512;
	const	uint	clampNearFadeSize= 32;
	const	uint	clampFarFadeSize= 128;
	uint			textMemSize= 4*clampTextSize*1;
	// Fill mem
	uint8	*tmpMem= new uint8[textMemSize];
	memset(tmpMem, 255, textMemSize);
	for(i=0;i<clampNearFadeSize;i++)
	{
		float	f= (float)i/clampNearFadeSize;
		f= easeInEaseOut(f);
		tmpMem[4*i+3]= (uint)(255*f);
	}
	for(i=0;i<clampFarFadeSize;i++)
	{
		float	f= (float)i/clampFarFadeSize;
		f= easeInEaseOut(f);
		tmpMem[4*(clampTextSize-i-1)+3]= (uint)(255*f);
	}
	// build the texture
	_ClampTexture = new CTextureMem (tmpMem, 4*clampTextSize*1, true, false, clampTextSize, 1);
	_ClampTexture->setWrapS (ITexture::Clamp);
	_ClampTexture->setWrapT (ITexture::Clamp);
	_ClampTexture->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	_ClampTexture->generate();
	_ClampTexture->setReleasable (false);

	// init material
	_ReceiveShadowMaterial.initUnlit();
	_ReceiveShadowMaterial.setBlend(true);
	_ReceiveShadowMaterial.setBlendFunc(CMaterial::zero, CMaterial::srccolor);
	// FillRate Optim
	_ReceiveShadowMaterial.setAlphaTest(true);
	_ReceiveShadowMaterial.setAlphaTestThreshold(0.01f);

	// ---- Stage 0. Project the ShadowMap. Blend the color between ShadowColor and White.
	// setup texture coord gen
	_ReceiveShadowMaterial.enableUserTexMat(0, true);
	_ReceiveShadowMaterial.setTexCoordGen(0, true);
	_ReceiveShadowMaterial.setTexCoordGenMode(0, CMaterial::TexCoordGenObjectSpace);
	// Setup the stage so we interpolate ShadowColor and White (according to shadowmap alpha)
	_ReceiveShadowMaterial.texEnvOpRGB(0, CMaterial::InterpolateTexture);
	_ReceiveShadowMaterial.texEnvArg0RGB(0, CMaterial::Diffuse, CMaterial::SrcColor);
	_ReceiveShadowMaterial.texEnvArg1RGB(0, CMaterial::Constant, CMaterial::SrcColor);
	_ReceiveShadowMaterial.texConstantColor(0, CRGBA::White);
	// Take Alpha for AlphaTest only.
	_ReceiveShadowMaterial.texEnvOpAlpha(0, CMaterial::Replace);
	_ReceiveShadowMaterial.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

	// ---- Stage 1. "Modulate" by Clamp Texture. Blend the color between stage0 color and White.
	// setup texture coord gen
	_ReceiveShadowMaterial.enableUserTexMat(1, true);
	_ReceiveShadowMaterial.setTexCoordGen(1, true);
	_ReceiveShadowMaterial.setTexCoordGenMode(1, CMaterial::TexCoordGenObjectSpace);
	_ReceiveShadowMaterial.setTexture(1, _ClampTexture);
	// Setup the stage so we interpolate Shadow and White (according to clamp alpha)
	_ReceiveShadowMaterial.texEnvOpRGB(1, CMaterial::InterpolateTexture);
	//_ReceiveShadowMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
	_ReceiveShadowMaterial.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
	_ReceiveShadowMaterial.texEnvArg1RGB(1, CMaterial::Constant, CMaterial::SrcColor);
	_ReceiveShadowMaterial.texConstantColor(1, CRGBA::White);
	// Take Alpha for AlphaTest only. (take 1st texture alpha...)
	_ReceiveShadowMaterial.texEnvOpAlpha(0, CMaterial::Replace);
	_ReceiveShadowMaterial.texEnvArg0Alpha(0, CMaterial::Previous, CMaterial::SrcAlpha);

	// Trans matrix from Nel basis (Z up) to UVW basis (V up)
	_XYZToUWVMatrix.setRot(CVector::I, CVector::K, CVector::J, true);
	// Trans Matrix so Y is now the U (for clamp map).
	_XYZToWUVMatrix.setRot(CVector::K, CVector::I, CVector::J, true);

	// **** Setup Casting
	_CasterShadowMaterial.initUnlit();
	_CasterShadowMaterial.setColor(CRGBA::White);
	_CasterShadowMaterial.setZWrite(false);
	_CasterShadowMaterial.setZFunc(CMaterial::always);
	_CasterShadowMaterial.setDoubleSided(true);
	// Alpha Polygon coverage accumulate, for polygon smoothing
	_CasterShadowMaterial.setBlend(true);
	_CasterShadowMaterial.setBlendFunc(CMaterial::one, CMaterial::one);
}

// ***************************************************************************
CShadowMapManager::~CShadowMapManager()
{
	_ShadowReceiverGrid.clear();
	_ShadowCasters.clear();
	clearGenerateShadowCasters();
	_NumShadowReceivers= 0;
}

// ***************************************************************************
void			CShadowMapManager::setQuadGridSize(uint size, float cellSize)
{
	_ShadowReceiverGrid.create(size, cellSize);
}

// ***************************************************************************
void			CShadowMapManager::addShadowCaster(CTransform *model)
{
	_ShadowCasters.push_back(model);
}

// ***************************************************************************
void			CShadowMapManager::addShadowCasterGenerate(CTransform *model)
{
	_GenerateShadowCasters.push_back(model);
	// Indicate this model that it will render its ShadowMap
	model->setGeneratingShadowMap(true);
}

// ***************************************************************************
void			CShadowMapManager::addShadowReceiver(CTransform *model)
{
	CAABBox	bb;
	model->getReceiverBBox(bb);

	_ShadowReceiverGrid.insert(bb.getMin(), bb.getMax(), model);

	_NumShadowReceivers++;
}

// ***************************************************************************
void			CShadowMapManager::renderGenerate(CScene *scene)
{
	H_AUTO( NL3D_ShadowManager_Generate );

	// Each frame, do a small garbage collector for unused free textures.
	garbageShadowTextures(scene);

	IDriver *driverForShadowGeneration= scene->getRenderTrav().getAuxDriver();

	// Init
	// ********
	uint32	wndW= 0, wndH= 0;
	// get some text/screen size.
	if(driverForShadowGeneration)
		driverForShadowGeneration->getWindowSize(wndW, wndH);
	uint	baseTextureSize= scene->getShadowMapTextureSize();
	// Minimize the Dest Texture size, so the blurTexture don't get to heavy in VRAM.
	uint32	textDestW= min(wndW, (uint32)NL3D_SMM_MAX_TEXTDEST_SIZE);
	uint32	textDestH= min(wndH, (uint32)NL3D_SMM_MAX_TEXTDEST_SIZE);

	// if not needed or if not possible, exit. test for wndSize is also important when window is minimized
	if( _ShadowCasters.empty() || 
		_NumShadowReceivers==0 ||
		textDestW<baseTextureSize || textDestH<baseTextureSize)
	{
		_ShadowReceiverGrid.clear();
		_NumShadowReceivers= 0;
		_ShadowCasters.clear();
		clearGenerateShadowCasters();
		return;
	}

	// If Needed to project some ShadowCaster, but none to compute this frame, quit.
	if( _GenerateShadowCasters.empty() )
	{
		// But here don't reset since the renderProject() will do job
		return;
	}

	// get the number of shadowMap compute we can do in one screen.
	uint	numTextW= textDestW/baseTextureSize;
	uint	numTextH= textDestH/baseTextureSize;
	if(!isPowerOf2(numTextW))
		numTextW= raiseToNextPowerOf2(numTextW)/2;
	if(!isPowerOf2(numTextH))
		numTextH= raiseToNextPowerOf2(numTextH)/2;
	// the max shadow casters we can do in 1 screen pass.
	uint	maxSCPerPass= numTextW * numTextH;

	// compute vp float size.
	float	vpWidth= (float)baseTextureSize / (float)wndW;
	float	vpHeight= (float)baseTextureSize / (float)wndH;


	// Create / Update the Blur Texture 
	updateBlurTexture(numTextW * baseTextureSize, numTextH * baseTextureSize);


	// Do NPass if a screen is not sufficient to render all shadow maps...
	// ********

	// bkup driver state
	CViewport	bkupViewport;
	driverForShadowGeneration->getViewport(bkupViewport);
	bool		bkupFog= driverForShadowGeneration->fogEnabled();

	// setup some state
	driverForShadowGeneration->enableFog(false);
	// Allow Writing on alpha only. => don't write on RGB objects!
	driverForShadowGeneration->setColorMask(false, false, false, true);

	uint	numSC= _GenerateShadowCasters.size();
	uint	baseSC= 0;
	while(numSC>0)
	{
		uint	numPassSC= min(maxSCPerPass, numSC);
		uint	textX, textY;
		uint	i;

		// Render All Shadow Map
		// ********

		// Render the polygons with Smooth Anti-Alias. Less jittering for little performance overcost
		if(_PolySmooth)
			driverForShadowGeneration->enablePolygonSmoothing(true);

		textX=0;
		textY=0;
		for(i=0;i<numPassSC;i++)
		{
			// get the transform to compute shadow map.
			CTransform	*sc= _GenerateShadowCasters[baseSC+i];

			// select the shadow direction
			CVector		lightDir;
			computeShadowDirection(scene, sc, lightDir);

			// setup viewport to render to
			CViewport	vp;
			vp.init(textX*baseTextureSize/(float)wndW, textY*baseTextureSize/(float)wndH, vpWidth, vpHeight);
			driverForShadowGeneration->setupViewport(vp);

			// TODO_SHADOW: optim: one big erase per pass, but just bbox needed (according to number of SC to render)
			// do a siccor or prefer do a polygon clear?
			CScissor	sic;
			sic.init(textX*baseTextureSize/(float)wndW, textY*baseTextureSize/(float)wndH, vpWidth, vpHeight);
			driverForShadowGeneration->setupScissor(sic);
			driverForShadowGeneration->clear2D(CRGBA(0,0,0,0));

			// render to screen
			sc->generateShadowMap(lightDir);

			// next text
			textX++;
			if(textX==numTextW)
			{
				textX= 0;
				textY++;
			}
		}

		// Restore
		if(_PolySmooth)
			driverForShadowGeneration->enablePolygonSmoothing(false);

		// For Subsequent operations, setup a full viewport and a "Screen Frustum"
		CScissor	sic;
		sic.initFullScreen();
		// TODO_SHADOW: optim: need scissor?
		driverForShadowGeneration->setupScissor(sic);
		CViewport	vp;
		vp.initFullScreen();
		driverForShadowGeneration->setupViewport(vp);
		driverForShadowGeneration->setFrustum(0, (float)wndW, 0, (float)wndH, -1,1,false);
		driverForShadowGeneration->setupViewMatrix(CMatrix::Identity);
		driverForShadowGeneration->setupModelMatrix(CMatrix::Identity);

		// Ensure the One pixel black security on texture border
		fillBlackBorder(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize);

		// Blur.
		// ********
		uint	numBlur= scene->getShadowMapBlurSize();
		clamp(numBlur, 0U, 3U);
		for(i=0;i<numBlur;i++)
		{
			// copy from FB to BlurTexture
			copyScreenToBlurTexture(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize);

			// blur
			applyFakeGaussianBlur(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize);

			// Ensure the One pixel black security on texture border
			fillBlackBorder(driverForShadowGeneration, numPassSC, numTextW, numTextH, baseTextureSize);
		}

		// Store Screen in ShadowMaps
		// ********
		textX=0;
		textY=0;
		for(i=0;i<numPassSC;i++)
		{
			// get the transform to compute shadow map.
			CTransform	*sc= _GenerateShadowCasters[baseSC+i];
			CShadowMap	*sm= sc->getShadowMap();
			if(sm)
			{
				ITexture	*text= sm->getTexture();
				if(text)
				{
					uint	bts= baseTextureSize;
					driverForShadowGeneration->copyFrameBufferToTexture(text, 0, 0, 0, textX*bts, textY*bts, bts, bts);
					// Indicate to the ShadowMap that we have updated his Texture
					sm->LastGenerationFrame= scene->getNumRender();
				}
			}

			// next text
			textX++;
			if(textX==numTextW)
			{
				textX= 0;
				textY++;
			}
		}


		// next screen pass.
		baseSC+= numPassSC;
		numSC-= numPassSC;
	}

	// Allow Writing on all.
	driverForShadowGeneration->setColorMask(true, true, true, true);
	// Restore driver state. (do it here because driverForShadowGeneration may be the main screen).
	driverForShadowGeneration->setupViewport(bkupViewport);
	driverForShadowGeneration->enableFog(bkupFog);
	// TODO_SHADOW: optim need scissor?
	CScissor	sic;
	sic.initFullScreen();
	driverForShadowGeneration->setupScissor(sic);

	// ensure the Scene Driver has correct matrix setup (in common case where AuxDriver == Std Driver)
	scene->getRenderTrav().setupDriverCamera();


	// Clear ShadowCaster Generation
	clearGenerateShadowCasters();
}

// ***************************************************************************
void			CShadowMapManager::renderProject(CScene *scene)
{
	// if not needed exit. NB renderGenerate() must have been called before.
	if( _NumShadowReceivers==0 )
	{
		nlassert(_ShadowCasters.empty());
		return;
	}


	// Project ShadowMap on receivers.
	// ********

	H_AUTO( NL3D_ShadowManager_Project );


	/* Fog Case: Since we do a modulate, we don't want to modulate the fog color with himself.
		Instead, if the shadowed pixel is in full fog, we have to modulate him with White
		=> replace fog color with white temporarly.
	*/
	IDriver	*driver= scene->getRenderTrav().getDriver();
	CRGBA	bkupFogColor= driver->getFogColor();
	driver->setupFog(driver->getFogStart(), driver->getFogEnd(), CRGBA::White);

	// For each ShadowMap
	for(uint i=0;i<_ShadowCasters.size();i++)
	{
		CTransform	*caster= _ShadowCasters[i];
		CShadowMap	*sm= caster->getShadowMap();
		nlassert(sm);
		// NB: the ShadowCaster may not have a texture yet, for example because of Generate selection...
		// If the final fade is 1, don't render!
		if( sm->getTexture() && sm->getFinalFade()<1 )
		{
			CVector		casterPos= caster->getWorldMatrix().getPos();

			// Compute the World bbox (for quadGrid intersection)
			CAABBox		worldBB= sm->LocalBoundingBox;
			worldBB.setCenter(worldBB.getCenter() + casterPos);

			// compute the world matrix of the projection.
			CMatrix		worldProjMat= sm->LocalProjectionMatrix;
			worldProjMat.setPos(worldProjMat.getPos()+casterPos);

			// Now compute the textureMatrix, from WorldSpace to UV.
			CMatrix		wsTextMat;
			CMatrix		osTextMat;
			wsTextMat= worldProjMat;
			wsTextMat.invert();

			// setup the Material.
			_ReceiveShadowMaterial.setTexture(0, sm->getTexture());
			/// Get The Mean Ambiant and Diffuse the caster receive (and cast, by approximation)
			CRGBA	ambient, diffuse;
			computeShadowColors(scene, caster, ambient, diffuse);
			// copute the shadowColor so that modulating a Medium diffuse terrain will  get the correct result.
			uint	R= ambient.R + (diffuse.R>>1);
			uint	G= ambient.G + (diffuse.G>>1);
			uint	B= ambient.B + (diffuse.B>>1);
			clamp(R, 1U, 256U);
			clamp(G, 1U, 256U);
			clamp(B, 1U, 256U);
			/* screen= text*(a+d*0.5) (mean value). if we do shadowColor= a/(a+d*0.5f), 
				then we'll have "in theory"  screen= text*a
			*/
			R= (uint)(256 * ambient.R / (float)R);
			G= (uint)(256 * ambient.G / (float)G);
			B= (uint)(256 * ambient.B / (float)B);
			clamp(R,0U,255U);
			clamp(G,0U,255U);
			clamp(B,0U,255U);
			/// Finally "modulate" with FinalFade.
			if(sm->getFinalFade()>0)
			{
				sint	factor= OptFastFloor( 256 * sm->getFinalFade() );
				clamp(factor, 0, 256);
				R= 255*factor + R*(256-factor); R>>=8;
				G= 255*factor + G*(256-factor); G>>=8;
				B= 255*factor + B*(256-factor); B>>=8;
			}
			_ReceiveShadowMaterial.setColor(CRGBA(R,G,B,255));


			// select receivers.
			_ShadowReceiverGrid.select(worldBB.getMin(), worldBB.getMax());
			// For all receivers
			TShadowReceiverGrid::CIterator	it;
			for(it= _ShadowReceiverGrid.begin();it!=_ShadowReceiverGrid.end();it++)
			{
				CTransform	*receiver= *it;
				// Avoid Auto-Casting.
				if(receiver==caster)
					continue;

				/* The problem is material don't support WorldSpace Coordinate Generation, but ObjectSpace ones.
					Hence must take back the coordinate in ObjectSpace before set textMat. 
					see getReceiverRenderWorldMatrix() Doc for why using this instead of getWorldMatrix()
				*/
				osTextMat.setMulMatrix(wsTextMat, receiver->getReceiverRenderWorldMatrix());

				/* Set the TextureMatrix for ShadowMap projection so that UVW= mat * XYZ. 
					its osTextMat but must rotate so Z map to V
				*/
				_ReceiveShadowMaterial.setUserTexMat(0, _XYZToUWVMatrix * osTextMat);
				/* Set the TextureMatrix for ClampMap projection so that UVW= mat * XYZ. 
					its osTextMat but must rotate so Y map to U
				*/
				_ReceiveShadowMaterial.setUserTexMat(1, _XYZToWUVMatrix * osTextMat);

				// cast the shadow on them
				receiver->receiveShadowMap(sm, casterPos, _ReceiveShadowMaterial);
			}
		}
	}

	// Restore fog color
	driver->setupFog(driver->getFogStart(), driver->getFogEnd(), bkupFogColor);


	// TestYoyo. Display Projection BBox.
	/*{
		for(uint i=0;i<_ShadowCasters.size();i++)
		{
			// get the transform to compute shadow map.
			CTransform	*sc= _ShadowCasters[i];

			CShadowMap	*sm= sc->getShadowMap();
			if(sm)
			{
				CVector		p0= sm->LocalProjectionMatrix.getPos() + sc->getWorldMatrix().getPos();
				IDriver		&drv= *driver;

				drv.setupModelMatrix(CMatrix::Identity);

				CDRU::drawWiredBox(p0, sm->LocalProjectionMatrix.getI(), sm->LocalProjectionMatrix.getJ(), 
					sm->LocalProjectionMatrix.getK(), CRGBA::White, drv);
			}
		}
	}*/


	// Release pass.
	// ********
	_ShadowReceiverGrid.clear();
	_ShadowCasters.clear();
	_NumShadowReceivers= 0;
}


// ***************************************************************************
void			CShadowMapManager::computeShadowDirection(CScene *scene, CTransform *sc, CVector &lightDir)
{
	// merge the sunLight and pointLights into a single directional
	lightDir= scene->getSunDirection();
	const	CLightContribution	&lc= sc->getLightContribution();
	// For Better result, weight with the light color too.
	CRGBA	color= scene->getSunDiffuse();
	lightDir*= (float)lc.SunContribution * (color.R + color.G + color.B);

	// merge pointLights
	const CVector		&modelPos= sc->getWorldMatrix().getPos();
	for(uint i=0;i<NL3D_MAX_LIGHT_CONTRIBUTION;i++)
	{
		CPointLight		*pl= lc.PointLight[i];
		// End of List?
		if(!pl)
			break;

		CVector	plDir= modelPos - pl->getPosition();
		plDir.normalize();
		// Sum with this light, weighted by AttFactor, and light color
		color= pl->getDiffuse();
		lightDir+= plDir * (float)lc.AttFactor[i] * (float)(color.R + color.G + color.B);
	}

	// normalize merged dir
	lightDir.normalize();
}


// ***************************************************************************
void			CShadowMapManager::computeShadowColors(CScene *scene, CTransform *sc, CRGBA &ambient, CRGBA &diffuse)
{
	const	CLightContribution	&lc= sc->getLightContribution();

	// Get the current ambiant
	ambient= lc.computeCurrentAmbient(scene->getSunAmbient());

	// Compute the current diffuse as a sum (not a mean)
	uint	r, g, b;
	CRGBA	color= scene->getSunDiffuse();
	r= color.R * lc.SunContribution;
	g= color.G * lc.SunContribution;
	b= color.B * lc.SunContribution;

	// Add PointLights contribution
	for(uint i=0;i<NL3D_MAX_LIGHT_CONTRIBUTION;i++)
	{
		CPointLight		*pl= lc.PointLight[i];
		// End of List?
		if(!pl)
			break;

		// Sum with this light, weighted by AttFactor
		color= pl->getDiffuse();
		r+= color.R * lc.AttFactor[i];
		g+= color.G * lc.AttFactor[i];
		b+= color.B * lc.AttFactor[i];
	}

	// normalize
	r>>=8;
	g>>=8;
	b>>=8;

	// Don't take the MergedPointLight into consideration (should add to the diffuse part here, but rare case)

	diffuse.R= min(r, 255U);
	diffuse.G= min(g, 255U);
	diffuse.B= min(b, 255U);
}


// ***************************************************************************
void			CShadowMapManager::fillBlackBorder(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize)
{
	if(numPassText==0)
		return;

	// the number of lines that have all their column disp.
	uint	numFullLine= numPassText/numTextW;
	// for the last line not full, the number of column setuped
	uint	lastLineNumCol=  numPassText - (numFullLine*numTextW);
	// number of line including the last line if not empty
	uint	numTotalLine= numFullLine + (lastLineNumCol?1:0);

	// Compute how many quads to render
	uint	numHQuads= numTotalLine * 2;
	uint	numTotalCol;
	uint	numVQuads;
	if(numFullLine)
		numTotalCol= numTextW;
	else
		numTotalCol= lastLineNumCol;
	numVQuads= numTotalCol * 2;

	_FillQuads.setNumVertices((numVQuads + numHQuads)*4);

	// Fill HQuads.
	uint	i;
	for(i=0;i<numTotalLine;i++)
	{
		uint	w;
		if(i<numFullLine)
			w= numTextW*baseTextureSize;
		else
			w= lastLineNumCol*baseTextureSize;
		// bottom of text
		setBlackQuad(i*2+0, 0, i*baseTextureSize, w, 1);
		// top of text
		setBlackQuad(i*2+1, 0, (i+1)*baseTextureSize-1, w, 1);
	}

	// Fill VQuads;
	uint	baseId= numTotalLine*2;
	for(i=0;i<numTotalCol;i++)
	{
		uint	h;
		if(i<lastLineNumCol)
			h= numTotalLine*baseTextureSize;
		else
			h= numFullLine*baseTextureSize;
		// left of text
		setBlackQuad(baseId + i*2+0, i*baseTextureSize, 0, 1, h);
		// right of text
		setBlackQuad(baseId + i*2+1, (i+1)*baseTextureSize-1, 0, 1, h);
	}

	// Render Quads
	_FillMaterial.setColor(CRGBA(0,0,0,0));
	drv->activeVertexBuffer(_FillQuads);
	drv->renderQuads(_FillMaterial, 0, numHQuads+numVQuads);
}


// ***************************************************************************
void			CShadowMapManager::setBlackQuad(uint index, sint x, sint y, sint w, sint h)
{
	float	x0= (float)x;
	float	y0= (float)y;
	float	x1= (float)x+(float)w;
	float	y1= (float)y+(float)h;
	index*= 4;
	_FillQuads.setVertexCoord (index+0, CVector (x0, 0, y0));
	_FillQuads.setVertexCoord (index+1, CVector (x1, 0, y0));
	_FillQuads.setVertexCoord (index+2, CVector (x1, 0, y1));
	_FillQuads.setVertexCoord (index+3, CVector (x0, 0, y1));
}

// ***************************************************************************
void			CShadowMapManager::updateBlurTexture(uint w, uint h)
{
	w= max(w, 2U);
	h= max(h, 2U);
	// if same size than setup, quit
	if(_BlurTextureW==w && _BlurTextureH==h)
		return;

	// release old SmartPtr
	_BlurMaterial.setTexture(0, NULL);
	_BlurMaterial.setTexture(1, NULL);
	_BlurMaterial.setTexture(2, NULL);
	_BlurMaterial.setTexture(3, NULL);
	_BlurTexture= NULL;
	_BlurTextureW= w;
	_BlurTextureH= h;
	// NB: the format must be RGBA; else slow copyFrameBufferToTexture()
	uint8	*tmpMem= new uint8[4*_BlurTextureW*_BlurTextureH];
	_BlurTexture = new CTextureMem (tmpMem, 4*_BlurTextureW*_BlurTextureH, true, false, _BlurTextureW, _BlurTextureH);
	_BlurTexture->setWrapS (ITexture::Clamp);
	_BlurTexture->setWrapT (ITexture::Clamp);
	_BlurTexture->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	_BlurTexture->generate();
	_BlurTexture->setReleasable (false);

	// set to the material
	_BlurMaterial.setTexture(0, _BlurTexture);
	_BlurMaterial.setTexture(1, _BlurTexture);
	_BlurMaterial.setTexture(2, _BlurTexture);
	_BlurMaterial.setTexture(3, _BlurTexture);

	// compute values for texturing
	_BlurTextureOOW= 1.f / _BlurTextureW;
	_BlurTextureOOH= 1.f / _BlurTextureH;
	// The Delta HalfPixel
	_BlurTextureD05W= 0.5f*_BlurTextureOOW;
	_BlurTextureD05H= 0.5f*_BlurTextureOOH;
}


// ***************************************************************************
void			CShadowMapManager::copyScreenToBlurTexture(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize)
{
	if(numPassText==0)
		return;

	// TODO_SHADOW: optim: split into 2 copy for less pixel draw on the last line? No because of OverHead?

	// number of line including the last line if not empty
	uint	numTotalLine= (numPassText+numTextW-1)/numTextW;
	// number of column.
	uint	numTotalCol= (numPassText<numTextW)?numPassText:numTextW;

	drv->copyFrameBufferToTexture(_BlurTexture, 0, 0, 0, 0, 0, numTotalCol*baseTextureSize, numTotalLine*baseTextureSize);
}

// ***************************************************************************
void			CShadowMapManager::applyFakeGaussianBlur(IDriver *drv, uint numPassText, uint numTextW, uint numTextH, uint baseTextureSize)
{
	if(numPassText==0)
		return;

	// the number of lines that have all their column disp.
	uint	numFullLine= numPassText/numTextW;
	// for the last line not full, the number of column setuped
	uint	lastLineNumCol=  numPassText - (numFullLine*numTextW);

	// Split into 2 quads. one for the first full lines, and one for the last not full line.
	uint	index= 0;
	if(numFullLine)
		setBlurQuadFakeGaussian(index++, 0, 0, numTextW*baseTextureSize, numFullLine*baseTextureSize);
	if(lastLineNumCol)
		setBlurQuadFakeGaussian(index++, 0, numFullLine*baseTextureSize, lastLineNumCol*baseTextureSize, baseTextureSize);

	// render
	drv->activeVertexBuffer(_BlurQuads);
	drv->renderQuads(_BlurMaterial, 0, index);
}


// ***************************************************************************
void			CShadowMapManager::setBlurQuadFakeGaussian(uint index, sint x, sint y, sint w, sint h)
{
	float	x0= (float)x;
	float	y0= (float)y;
	float	x1= (float)x+(float)w;
	float	y1= (float)y+(float)h;
	float	u0= x0*_BlurTextureOOW;
	float	v0= y0*_BlurTextureOOH;
	float	u1= x1*_BlurTextureOOW;
	float	v1= y1*_BlurTextureOOH;
	index*= 4;

	// NB: the order of the Delta (--,++,-+,+-) is made so it works well with 2,3 or 4 texture support.

	// vertex 0
	_BlurQuads.setVertexCoord (index+0, CVector (x0, 0, y0));
	_BlurQuads.setTexCoord(index+0, 0, u0-_BlurTextureD05W, v0-_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+0, 1, u0+_BlurTextureD05W, v0+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+0, 2, u0-_BlurTextureD05W, v0+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+0, 3, u0+_BlurTextureD05W, v0-_BlurTextureD05H);
	// vertex 1
	_BlurQuads.setVertexCoord (index+1, CVector (x1, 0, y0));
	_BlurQuads.setTexCoord(index+1, 0, u1-_BlurTextureD05W, v0-_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+1, 1, u1+_BlurTextureD05W, v0+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+1, 2, u1-_BlurTextureD05W, v0+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+1, 3, u1+_BlurTextureD05W, v0-_BlurTextureD05H);
	// vertex 2
	_BlurQuads.setVertexCoord (index+2, CVector (x1, 0, y1));
	_BlurQuads.setTexCoord(index+2, 0, u1-_BlurTextureD05W, v1-_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+2, 1, u1+_BlurTextureD05W, v1+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+2, 2, u1-_BlurTextureD05W, v1+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+2, 3, u1+_BlurTextureD05W, v1-_BlurTextureD05H);
	// vertex 3
	_BlurQuads.setVertexCoord (index+3, CVector (x0, 0, y1));
	_BlurQuads.setTexCoord(index+3, 0, u0-_BlurTextureD05W, v1-_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+3, 1, u0+_BlurTextureD05W, v1+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+3, 2, u0-_BlurTextureD05W, v1+_BlurTextureD05H);
	_BlurQuads.setTexCoord(index+3, 3, u0+_BlurTextureD05W, v1-_BlurTextureD05H);
}


// ***************************************************************************
struct	CShadowMapSort
{
	CTransform		*Caster;
	float			Weight;

	bool		operator<(const CShadowMapSort &o) const
	{
		return Weight<o.Weight;
	}
};

// ***************************************************************************
void			CShadowMapManager::selectShadowMapsToGenerate(CScene *scene)
{
	// TODO: Scene option.
	const uint		maxPerFrame= 8;
	const float		minCamDist= 10;
	const CVector	&camPos= scene->getRenderTrav().CamPos;
	uint			i;

	// **** Clear first
	clearGenerateShadowCasters();

	// If the scene filter skeleton render, suppose no generation at all. Ugly.
	if(! (scene->getFilterRenderFlags() & UScene::FilterSkeleton) )
		return;

	// **** Select
	// For all ShadowCaster inserted
	static vector<CShadowMapSort>		sortList;
	sortList.clear();
	sortList.reserve(_ShadowCasters.size());
	for(i=0;i<_ShadowCasters.size();i++)
	{
		CTransform	*caster= _ShadowCasters[i];
		/* If the shadowMap exist, and if not totaly faded
			NB: take FinalFade here because if 1, it won't be rendered in renderProject()
			so don't really need to update (usefull for update reason, but LastGenerationFrame do the job)
		*/
		if(caster->getShadowMap() && caster->getShadowMap()->getFinalFade()<1 )
		{
			CShadowMapSort		sms;
			sms.Caster= caster;
			// The Weight is the positive delta of frame
			sms.Weight= (float)(scene->getNumRender() - caster->getShadowMap()->LastGenerationFrame);
			// Modulated by Caster Distance from Camera.
			float	distToCam= (caster->getWorldMatrix().getPos() - camPos).norm();
			distToCam= max(distToCam, minCamDist);
			// The farthest, the less important
			sms.Weight/= distToCam;

			// Append
			sortList.push_back(sms);
		}
	}

	// Sort increasing
	sort(sortList.begin(), sortList.end());

	// Select the best
	uint	numSel= min((uint)sortList.size(), maxPerFrame);
	_GenerateShadowCasters.resize(numSel);
	for(i= 0;i<numSel;i++)
	{
		_GenerateShadowCasters[i]= sortList[sortList.size()-1-i].Caster;
	}

	// **** Flag selecteds
	// For All selected models, indicate that they will generate shadowMap for this Frame.
	for(i=0;i<_GenerateShadowCasters.size();i++)
	{
		_GenerateShadowCasters[i]->setGeneratingShadowMap(true);
	}
}


// ***************************************************************************
void			CShadowMapManager::clearGenerateShadowCasters()
{
	// Reset first each flag of all models
	for(uint i=0;i<_GenerateShadowCasters.size();i++)
	{
		_GenerateShadowCasters[i]->setGeneratingShadowMap(false);
	}

	_GenerateShadowCasters.clear();
}

// ***************************************************************************
ITexture		*CShadowMapManager::allocateTexture(uint textSize)
{
	nlassert( isPowerOf2(textSize) );

	// **** First, find a free texture already allocated.
	if(!_FreeShadowTextures.empty())
	{
		ITexture	*freeText= _FreeShadowTextures.back()->second;
		// If Ok for the size.
		if(freeText->getWidth() == textSize)
		{
			// take this texture => no more free.
			_FreeShadowTextures.pop_back();
			return freeText;
		}
		// else, suppose that we still take this slot.
		else
		{
			// but since bad size, delete this slot from the map and create a new one (below)
			_ShadowTextureMap.erase(_FreeShadowTextures.back());
			// no more valid it
			_FreeShadowTextures.pop_back();
		}
	}

	// **** Else Allocate new one.
	// NB: the format must be RGBA; else slow copyFrameBufferToTexture()
	uint8	*tmpMem= new uint8[4*textSize*textSize];
	ITexture	*text;
	text = new CTextureMem (tmpMem, 4*textSize*textSize, true, false, textSize, textSize);
	text->setWrapS (ITexture::Clamp);
	text->setWrapT (ITexture::Clamp);
	text->setFilterMode (ITexture::Linear, ITexture::LinearMipMapOff);
	text->generate();
	text->setReleasable (false);

	// Setup in the map.
	_ShadowTextureMap[text]= text;

	return text;
}

// ***************************************************************************
void			CShadowMapManager::releaseTexture(ITexture *text)
{
	if(!text)
		return;

	ItTextureMap	it= _ShadowTextureMap.find(text);
	nlassert(it!=_ShadowTextureMap.end());

	// Don't release it, but insert in Free Space
	_FreeShadowTextures.push_back(it);
}

// ***************************************************************************
void			CShadowMapManager::garbageShadowTextures(CScene *scene)
{
	uint	defSize= scene->getShadowMapTextureSize();

	// For all Free Textures only, release the one that are no more of the wanted default ShadowMap Size.
	std::vector<ItTextureMap>::iterator		itVec= _FreeShadowTextures.begin();
	for(;itVec!=_FreeShadowTextures.end();)
	{
		if((*itVec)->second->getWidth() != defSize)
		{
			// release the map texture iterator
			_ShadowTextureMap.erase(*itVec);
			// release the Vector Free iterator.
			itVec= _FreeShadowTextures.erase(itVec);
		}
		else
		{
			itVec++;
		}
	}

	// For memory optimisation, allow only a small extra of Texture allocated.
	if(_FreeShadowTextures.size()>NL3D_SMM_MAX_FREETEXT)
	{
		// Release the extra texture (Hysteresis: divide by 2 the max wanted free to leave)
		uint	startToFree= NL3D_SMM_MAX_FREETEXT/2;
		for(uint i=startToFree;i<_FreeShadowTextures.size();i++)
		{
			// Free the texture entry.
			_ShadowTextureMap.erase(_FreeShadowTextures[i]);
		}
		// resize vector
		_FreeShadowTextures.resize(startToFree);
	}
}


} // NL3D

