/** \file ps_particle_basic.cpp
 * Some classes used for particle building.
 *
 * $Id: ps_particle_basic.cpp,v 1.4 2002/02/28 12:59:51 besson Exp $
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

#include "3d/ps_particle_basic.h"
#include "3d/ps_macro.h"
#include "3d/driver.h"
#include "3d/texture_grouped.h"
#include "3d/texture_bump.h"
#include "3d/texture_mem.h"
#include "3d/particle_system.h"



namespace NL3D 
{



/////////////////////////////////
// CPSParticle implementation  //
/////////////////////////////////

//=======================================
	CPSParticle::CPSParticle() : _DisableAutoLOD(false)
{

}

//=======================================
void CPSParticle::showTool()
{
	PARTICLES_CHECK_MEM;

	CVector I = CVector::I;
	CVector J = CVector::J;

	const CVector tab[] = { 2 * J, I + J
							, I + J, 2 * I + J
							, 2 * I + J, I
							, I,  2 * I - J
							, 2 * I - J, - .5f * J
							, - .5f * J, -2 * I - J
							, -2 * I - J, - I
							, - I, -2 * I + J
							, -2 * I + J, - I + J
							, - I + J, 2 * J
						};
	const uint tabSize = sizeof(tab) / (2 * sizeof(CVector));

	const float sSize = 0.1f;
	displayIcon2d(tab, tabSize, sSize);

	PARTICLES_CHECK_MEM;
}

//=======================================
void CPSParticle::computeSrcStep(uint32 &step, uint &numToProcess)
{		
	nlassert(_Owner && _Owner->getOwner());
	const CParticleSystem &ps = *(_Owner->getOwner());
	if (_DisableAutoLOD || !ps.isAutoLODEnabled() || _Owner->getSize() == 0) // Should Auto-LOD be used ?
	{
		step = (1 << 16);
		numToProcess = _Owner->getSize();
	}
	else
	{
		float oneMinusLODRatio = ps.getOneMinusCurrentLODRatio();
		float LODRatio = 1.f - oneMinusLODRatio;  
		if (LODRatio > ps.getAutoLODStartDistPercent())
		{
			float factor = (LODRatio - 1.f) / (ps.getAutoLODStartDistPercent() - 1.f);
			NLMISC::clamp(factor, 0.f, 1.f);
			float r = factor;
			for (uint k = 0; k < ps.getAutoLODDegradationExponent(); ++k)
			{
				r *= factor;
			}
			numToProcess = (uint) (_Owner->getSize() * r);
			if (numToProcess < 1) { numToProcess = 1; }

			step =	 ps.getAutoLODMode() ?				   // skip or limit number, depending on the mode
				(_Owner->getSize() << 16) / numToProcess : // skip particles
				(1<<16);							   // just display less particles
		}
		else
		{
			step = (1 << 16);
			numToProcess = _Owner->getSize();
		}
	}

}

//////////////////////////////////////
// coloured particle implementation //
//////////////////////////////////////

//=======================================
void CPSColoredParticle::setColorScheme(CPSAttribMaker<CRGBA> *col)
{	
	nlassert(col);
	delete _ColorScheme;		
	_ColorScheme = col;
	if (getColorOwner() && col->hasMemory()) col->resize(getColorOwner()->getMaxSize(), getColorOwner()->getSize());
	updateMatAndVbForColor();
}

//=======================================
void CPSColoredParticle::setColor(NLMISC::CRGBA col)
{	
	delete _ColorScheme;
	_ColorScheme = NULL;
	_Color = col;
	updateMatAndVbForColor();
}

//=======================================
CPSColoredParticle::CPSColoredParticle() :  _ColorScheme(NULL), _Color(CRGBA(255, 255, 255))
{	
}

//=======================================
CPSColoredParticle::~CPSColoredParticle()
{	
	delete _ColorScheme;	
}

//=======================================
void CPSColoredParticle::serialColorScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_ColorScheme)
		{
			delete _ColorScheme;
			_ColorScheme = NULL;
		}
	}
	bool useColorScheme = _ColorScheme != NULL;
	f.serial(useColorScheme);
	if (useColorScheme)
	{
		f.serialPolyPtr(_ColorScheme);
	}
	else
	{
		f.serial(_Color);
	}
}

///////////////////////////////////
// sized particle implementation //
///////////////////////////////////

//=======================================
void CPSSizedParticle::setSizeScheme(CPSAttribMaker<float> *size)
{	
	nlassert(size != NULL);
	delete _SizeScheme;	
	_SizeScheme = size;
	if (getSizeOwner() && size->hasMemory()) size->resize(getSizeOwner()->getMaxSize(), getSizeOwner()->getSize());
}

//=======================================
void CPSSizedParticle::setSize(float size)
{	
	delete _SizeScheme;
	_SizeScheme = NULL;	
	_ParticleSize = size;
}

//=======================================
CPSSizedParticle::CPSSizedParticle() : _SizeScheme(NULL), _ParticleSize(0.3f)
{
}

//=======================================
CPSSizedParticle::~CPSSizedParticle()
{	
	delete _SizeScheme;	
}

//=======================================
void CPSSizedParticle::serialSizeScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_SizeScheme)
		{
			delete _SizeScheme;
			_SizeScheme = NULL;
		}
	}
	bool useSizeScheme = _SizeScheme != NULL;
	f.serial(useSizeScheme);
	if (useSizeScheme)
	{		
		f.serialPolyPtr(_SizeScheme);
	}
	else
	{
		f.serial(_ParticleSize);
	}	
};

/////////////////////////////////////
// rotated particle implementation //
/////////////////////////////////////

float CPSRotated2DParticle::_RotTable[256 * 4];
bool CPSRotated2DParticle::_InitializedRotTab = false;

///===================================================================================
void CPSRotated2DParticle::setAngle2DScheme(CPSAttribMaker<float> *angle2DScheme)
{
	nlassert(angle2DScheme);
	delete _Angle2DScheme;	
	_Angle2DScheme = angle2DScheme;
	if (getAngle2DOwner() && angle2DScheme->hasMemory()) angle2DScheme->resize(getAngle2DOwner()->getMaxSize(), getAngle2DOwner()->getSize());
}

///===================================================================================
void CPSRotated2DParticle::setAngle2D(float angle2DScheme)
{	
	delete _Angle2DScheme;
	_Angle2DScheme = NULL;	
	_Angle2D = angle2DScheme;
}

///===================================================================================
CPSRotated2DParticle::CPSRotated2DParticle() : _Angle2D(0), _Angle2DScheme(NULL)
{
}

///===================================================================================
CPSRotated2DParticle::~CPSRotated2DParticle()
{	
	delete _Angle2DScheme;	
}

///===================================================================================
void CPSRotated2DParticle::serialAngle2DScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_Angle2DScheme)
		{
			delete _Angle2DScheme;
			_Angle2DScheme = NULL;
		}
	}
	bool useAngle2DScheme = _Angle2DScheme != NULL;
	f.serial(useAngle2DScheme);
	if (useAngle2DScheme)
	{
		f.serialPolyPtr(_Angle2DScheme);
	}
	else
	{		
		f.serial(_Angle2D);
	}	
}

///===================================================================================
void CPSRotated2DParticle::initRotTable(void)
{
	float *ptFloat = _RotTable;
	for (uint32 k = 0; k < 256; ++k)
	{
		const float ca = (float) cos(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi);
		const float sa = (float) sin(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi);

		*ptFloat++ = -ca - sa;
		*ptFloat++ = -sa + ca;

		*ptFloat++ = ca - sa;
		*ptFloat++ = sa + ca;
	}
	_InitializedRotTab = true;
}

//////////////////////////////////////
// textured particle implementation //
//////////////////////////////////////

///===================================================================================
void CPSTexturedParticle::setTextureIndexScheme(CPSAttribMaker<sint32> *animOrder)
{
	nlassert(animOrder);
	nlassert(_TexGroup); // setTextureGroup must have been called before this	
	delete _TextureIndexScheme;		
	_TextureIndexScheme = animOrder;
	if (getTextureIndexOwner() && animOrder->hasMemory()) animOrder->resize(getTextureIndexOwner()->getMaxSize(), getTextureIndexOwner()->getSize());


	updateMatAndVbForTexture();
}

///===================================================================================
void CPSTexturedParticle::setTextureIndex(sint32 index)
{	
	delete _TextureIndexScheme;	
	_TextureIndexScheme = NULL;	
	_TextureIndex = index;
}

///===================================================================================
void CPSTexturedParticle::setTextureGroup(NLMISC::CSmartPtr<CTextureGrouped> texGroup)
{
	nlassert(texGroup);
	if (_Tex)
	{
		_Tex = NULL;
	}
	_TexGroup = texGroup;
}

///===================================================================================
void CPSTexturedParticle::setTexture(CSmartPtr<ITexture> tex)
{	
	delete _TextureIndexScheme;
	_TextureIndexScheme = NULL;	
	_Tex = tex;
	_TexGroup = NULL; // release any grouped texture if one was set before
	updateMatAndVbForTexture();
}

///===================================================================================
CPSTexturedParticle::CPSTexturedParticle() : _TextureIndex(0)
											 ,_TexGroup(NULL), _TextureIndexScheme(NULL)
{
}

///===================================================================================
CPSTexturedParticle::~CPSTexturedParticle()
{	
	delete _TextureIndexScheme;	
}

///===================================================================================
void CPSTexturedParticle::serialTextureScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_TextureIndexScheme)
		{
			delete _TextureIndexScheme;
			_TextureIndexScheme = NULL;			
			_Tex = NULL;
			_TexGroup = NULL;
		}
	}

	bool useAnimatedTexture;	
	if (!f.isReading())
	{
		useAnimatedTexture = (_TexGroup != NULL);
	}
	f.serial(useAnimatedTexture);
	if (useAnimatedTexture)
	{
		if (f.isReading())
		{
			CTextureGrouped *ptTex = NULL;
			f.serialPolyPtr(ptTex);
			_TexGroup = ptTex;
		}
		else
		{
			CTextureGrouped *ptTex = _TexGroup;
			f.serialPolyPtr(ptTex);
		}
		
		bool useTextureIndexScheme = _TextureIndexScheme != NULL;		
		f.serial(useTextureIndexScheme);
		if (useTextureIndexScheme)
		{
			f.serialPolyPtr(_TextureIndexScheme);
			_TextureIndex = 0;			
		}
		else
		{			
			f.serial(_TextureIndex);
		}
	}
	else
	{
		if (f.isReading())
		{
			ITexture *ptTex = NULL;
			f.serialPolyPtr(ptTex);
			_Tex = ptTex;
		}
		else
		{
			ITexture *ptTex = _Tex;
			f.serialPolyPtr(ptTex);
		}
	}	
}

////////////////////////////////////////////////////////
//       CPSRotated3DPlaneParticle  implementation    //
////////////////////////////////////////////////////////

///===================================================================================
void CPSRotated3DPlaneParticle::setPlaneBasisScheme(CPSAttribMaker<CPlaneBasis> *basisMaker)
{
	nlassert(basisMaker);
	delete _PlaneBasisScheme;	
	_PlaneBasisScheme = basisMaker;
	if (getPlaneBasisOwner() && basisMaker->hasMemory()) basisMaker->resize(getPlaneBasisOwner()->getMaxSize(), getPlaneBasisOwner()->getSize());
}

///===================================================================================
void CPSRotated3DPlaneParticle::setPlaneBasis(const CPlaneBasis &basis)
{	
	delete _PlaneBasisScheme;		
	_PlaneBasisScheme = NULL;	
	_PlaneBasis = basis;
}

///===================================================================================
CPSRotated3DPlaneParticle::CPSRotated3DPlaneParticle() : _PlaneBasisScheme(NULL)														
{
	_PlaneBasis.X = CVector::I;
	_PlaneBasis.Y = CVector::J;
}

///===================================================================================
CPSRotated3DPlaneParticle::~CPSRotated3DPlaneParticle()
{	
	delete _PlaneBasisScheme;	
}

///===================================================================================
void CPSRotated3DPlaneParticle::serialPlaneBasisScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	f.serialPolyPtr(_PlaneBasisScheme);	
	bool usePlaneBasisScheme = _PlaneBasisScheme != NULL;	
	if (!usePlaneBasisScheme)
	{
		f.serial(_PlaneBasis);
	}
}

////////////////////////////////
// CPSMaterial implementation //
////////////////////////////////

///===================================================================================
CPSMaterial::CPSMaterial()
{
	_Mat.setBlend(true);
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one);
	_Mat.setZWrite(false);	
}

///===================================================================================
void CPSMaterial::serialMaterial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	TBlendingMode m = getBlendingMode();
	f.serialEnum(m);
	setBlendingMode(m);	
}

///===================================================================================
void CPSMaterial::setBlendingMode(CPSMaterial::TBlendingMode mode)
{
	switch (mode)
	{		
		case add:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::one, CMaterial::one);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case modulate:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::zero, CMaterial::srccolor);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case alphaBlend:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case alphaTest:
			_Mat.setBlend(false);
			_Mat.setZWrite(true);
			_Mat.setAlphaTest(true);
		break;
	}
}

///===================================================================================
CPSMaterial::TBlendingMode CPSMaterial::getBlendingMode(void) const
{
	if (_Mat.getBlend())
	{
		CMaterial::TBlend srcBlend = _Mat.getSrcBlend();	
		CMaterial::TBlend destBlend = _Mat.getDstBlend();

		if (srcBlend == CMaterial::one && destBlend == CMaterial::one) return add;
		if (srcBlend == CMaterial::zero && destBlend == CMaterial::srccolor) return modulate;
		if (srcBlend == CMaterial::srcalpha && destBlend == CMaterial::invsrcalpha) return alphaBlend;

		// unrecognized mode
		nlassert(0);
		return alphaTest; // to avoid a warning only ...
	}
	else
	{
		return alphaTest;
	}
}

///===================================================================================
void CPSMaterial::forceModulateConstantColor(bool force, const NLMISC::CRGBA &col)
{
	if (force)
	{
		/// TODO : caching..
		_Mat.texConstantColor(1, col);
		_Mat.texEnvOpRGB(1, CMaterial::Modulate);
		_Mat.texEnvOpAlpha(1, CMaterial::Modulate);
		_Mat.texEnvArg0RGB(1, CMaterial::Previous, CMaterial::SrcColor);
		_Mat.texEnvArg1RGB(1, CMaterial::Constant, CMaterial::SrcColor);
		_Mat.texEnvArg0Alpha(1, CMaterial::Previous, CMaterial::SrcAlpha);
		_Mat.texEnvArg1Alpha(1, CMaterial::Constant, CMaterial::SrcAlpha);
		forceTexturedMaterialStages(2);
	}
	else
	{		
		if (_Mat.getTexture(1) != NULL)
		{
			_Mat.setTexture(1, NULL);
		}
	}
}


///===================================================================================
void CPSMaterial::forceTexturedMaterialStages(uint numStages)
{
	ITexture *blankTex = NULL;
	uint k;
	for (k = 0; k < numStages; ++k)
	{
		if (_Mat.getTexture(k) == NULL)
		{
			if (!blankTex)
			{
				blankTex = CTextureMem::Create1x1WhiteTex();
			}
			_Mat.setTexture(k, blankTex);
		}
	}
	for (; k < IDRV_MAT_MAXTEXTURES; ++k)
	{
		if (_Mat.getTexture(k) != NULL)
		{
			_Mat.setTexture(k, NULL);
		}		
	}
}


/////////////////////////////////////////////
// CPSMultiTexturedParticle implementation //
/////////////////////////////////////////////

//=======================================
bool CPSMultiTexturedParticle::_ForceBasicCaps  = false;

//=======================================
CPSMultiTexturedParticle::CPSMultiTexturedParticle() : _MultiTexState(TouchFlag), _BumpFactor(1.f)
{	
}

//=======================================
void	CPSMultiTexturedParticle::enableMultiTexture(bool enabled /*= true*/)
{
	if (isMultiTextureEnabled() == enabled) return;
	if (!enabled)
	{
		_Texture2		   = NULL;
		_AlternateTexture2 = NULL;
		_MultiTexState = 0;
	}
	else
	{		
		_MainOp = Modulate;
		_TexScroll[0].set(0, 0);
		_TexScroll[1].set(0, 0);
		_MultiTexState = (uint8) MultiTextureEnabled;
	}
	touch();	
}

//=======================================
void	CPSMultiTexturedParticle::enableAlternateTex(bool enabled /*= true*/)
{	
	nlassert(isMultiTextureEnabled()); // multitexturing must have been enabled first
	if (enabled == isAlternateTexEnabled()) return;

	if (enabled)
	{
		_TexScrollAlternate[0].set(0, 0);
		_TexScrollAlternate[1].set(0, 0);
		_AlternateOp = Modulate;
		_MultiTexState |= (uint8) AlternateTextureEnabled;
	}
	else
	{
		_Texture2 = NULL;
		_MultiTexState &= ~(uint8) AlternateTextureEnabled;
	}
	touch();
}

//=======================================
void	CPSMultiTexturedParticle::serialMultiTex(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/// version 1 : bump factor
	sint ver = f.serialVersion(1);
	f.serial(_MultiTexState);
	if (isMultiTextureEnabled())
	{
		f.serialEnum(_MainOp);
		if (_MainOp == EnvBumpMap && ver >= 1)
		{
			f.serial(_BumpFactor);
		}
		ITexture *tex = NULL;
		if (f.isReading())
		{
			f.serialPolyPtr(tex);
			_Texture2 = tex;
		}
		else
		{
			tex = _Texture2;
			f.serialPolyPtr(tex);
		}
		f.serial(_TexScroll[0], _TexScroll[1]);
		
		if (isAlternateTexEnabled())
		{
			f.serialEnum(_AlternateOp);			
			if (f.isReading())
			{
				f.serialPolyPtr(tex);
				_AlternateTexture2 = tex;
			}
			else
			{
				tex = _AlternateTexture2;
				f.serialPolyPtr(tex);
			}
			f.serial(_TexScrollAlternate[0], _TexScrollAlternate[1]);
		}
		else
		{
			_AlternateTexture2 = NULL;
		}
	}
	else
	{
		if (f.isReading())
		{
			_Texture2		   = NULL;
			_AlternateTexture2 = NULL;
		}
	}	
}

//=======================================
void CPSMultiTexturedParticle::setupMaterial(ITexture *primary, IDriver *driver, CMaterial &mat)
{
	/// if bump is used, the matrix must be setupped each time (not a material field)
	if (isMultiTextureEnabled() && _MainOp  == EnvBumpMap && driver->isTextureAddrModeSupported(CMaterial::OffsetTexture))
	{
		CTextureBump *tb = dynamic_cast<CTextureBump *>((ITexture *) _Texture2);
		if (tb != NULL)
		{					
			float normFactor = tb->getNormalizationFactor();
			if (normFactor == 0.f) // have we computed the normalization factor ?
			{
				/// todo : caching of this value
				tb->generate();
				normFactor = tb->getNormalizationFactor();
				tb->release();
			}
			const float bMat[] = { _BumpFactor * normFactor, 0.f, 0.f, _BumpFactor * normFactor};	
			driver->setMatrix2DForTextureOffsetAddrMode(1, bMat);
		}		
	}

	if (!isTouched() && areBasicCapsForcedLocal() == areBasicCapsForced()) return;
	if (!isMultiTextureEnabled())		
	{		
		mat.setTexture(0, primary);
		mat.texEnvOpRGB(0, CMaterial::Modulate);
		mat.setTexture(1, NULL);
	}
	else
	{				
		if (_MainOp  != EnvBumpMap)
		{			
			setupMultiTexEnv(_MainOp, primary, _Texture2, mat);
			_MultiTexState &= ~(uint8) EnvBumpMapUsed;
			_MultiTexState &= ~(uint8) AlternateTextureUsed;
		}
		else
		{
			if (!_ForceBasicCaps && driver->isTextureAddrModeSupported(CMaterial::OffsetTexture)) // envbumpmap supported ?
			{
				CTextureBump *tb = dynamic_cast<CTextureBump *>((ITexture *) _Texture2);
				if (tb != NULL)
				{					
					float normFactor = tb->getNormalizationFactor();
					if (normFactor == 0.f) // have we computed the normalization factor ?
					{
						/// todo : caching of this value
						tb->generate();
						normFactor = tb->getNormalizationFactor();
						tb->release();
					}					
					setupMultiTexEnv(_MainOp, primary, _Texture2, mat);	
				}
				_MultiTexState &= ~(uint8) AlternateTextureUsed;
				_MultiTexState |= (uint8) EnvBumpMapUsed;				
			}
			else // switch to alternate
			{
				if (isAlternateTexEnabled())
				{
					_MultiTexState |= (uint8) AlternateTextureUsed;
					setupMultiTexEnv(_AlternateOp, primary, _AlternateTexture2, mat);
					_MultiTexState &= ~(uint8) EnvBumpMapUsed;
				}
				else // display the texture as it
				{
					setupMultiTexEnv(Decal, primary, NULL, mat);
					_MultiTexState &= ~(uint8) AlternateTextureUsed;
					_MultiTexState &= ~(uint8) EnvBumpMapUsed;
				}
			}
		}
	}
	
	(areBasicCapsForced());
	unTouch();
}

//=======================================
void	CPSMultiTexturedParticle::setupMultiTexEnv(TOperator op, ITexture *tex1, ITexture *tex2, CMaterial &mat)
{
	switch (op)
	{
		case Add:			
			mat.setTexture(0, tex1);
			mat.setTexture(1, tex2);
			mat.texEnvOpRGB(0, CMaterial::Modulate);
			mat.texEnvOpRGB(1, CMaterial::Add);
			mat.enableTexAddrMode(false);
		break;
		case Modulate:
			mat.setTexture(0, tex1);
			mat.setTexture(1, tex2);
			mat.texEnvOpRGB(0, CMaterial::Modulate);
			mat.texEnvOpRGB(1, CMaterial::Modulate);
			mat.enableTexAddrMode(false);
		break;
		case EnvBumpMap:
			mat.setTexture(0, tex2);
			mat.setTexture(1, tex1);
			mat.texEnvOpRGB(0, CMaterial::Replace);
			mat.texEnvOpRGB(1, CMaterial::Modulate);
			mat.enableTexAddrMode(true);
			mat.setTexAddressingMode(0, CMaterial::FetchTexture);		
			mat.setTexAddressingMode(1, CMaterial::OffsetTexture);
		break;
		case Decal:
			mat.setTexture(0, tex1);
			mat.texEnvOpRGB(0, CMaterial::Replace);
			mat.setTexture(1, NULL);
			mat.enableTexAddrMode(false);
		break;
	}
}

//=====static func to convert a texture to a bumpmap
static void ConvertToBumpMap(NLMISC::CSmartPtr<ITexture> &ptr)
{
	if (!dynamic_cast<CTextureBump *>( (ITexture *) ptr))
	{
		// convert to a bumpmap
		CTextureBump *tb = new CTextureBump;
		tb->setAbsoluteOffsets();
		tb->setHeightMap((ITexture *) ptr);
		ptr = tb;
	}	
}

//=====static func to convert a bumpmap to a texture (its heightmap)
static void ConvertFromBumpMap(NLMISC::CSmartPtr<ITexture> &ptr)
{
	CTextureBump *bm = dynamic_cast<CTextureBump *>( (ITexture *) ptr);
	if (bm)
	{
		// retrieve the heightmap from the bumpmap
		NLMISC::CSmartPtr<ITexture> hm = bm->getHeightMap();		
		ptr = hm;
	}
}

//=========================================
void	CPSMultiTexturedParticle::setTexture2Alternate(ITexture *tex)
{ 
	_AlternateTexture2 = tex;
	if (_AlternateOp != EnvBumpMap)
	{		
		ConvertFromBumpMap(_AlternateTexture2);
	}
	else
	{
		ConvertToBumpMap(_AlternateTexture2);		
	}
	touch();
}

//==========================================
void	CPSMultiTexturedParticle::setTexture2(ITexture *tex) 
{ 
	_Texture2 = tex;
	if (_MainOp != EnvBumpMap)
	{		
		ConvertFromBumpMap(_Texture2);
	}
	else
	{
		if (!dynamic_cast<NL3D::CTextureBump *>((ITexture *) _Texture2))
		{
			ConvertToBumpMap(_Texture2);
		}
	}
	touch();
}

//==========================================
void	CPSMultiTexturedParticle::setMainTexOp(TOperator op)
{ 
	_MainOp = op;
	if (_MainOp == EnvBumpMap)
	{
		ConvertToBumpMap(_Texture2);
	}
	else
	{
		ConvertFromBumpMap(_Texture2);
	}
	touch();
}

//==========================================
void	CPSMultiTexturedParticle::setAlternateTexOp(TOperator op)
{ 
	_AlternateOp = op;
	if (_AlternateOp == EnvBumpMap)
	{
		ConvertToBumpMap(_AlternateTexture2);
	}
	else
	{
		ConvertFromBumpMap(_AlternateTexture2);
	}
	touch();
}



//==========================================
void	CPSMultiTexturedParticle::setUseLocalDate(bool use)
{
	if (use) _MultiTexState |= ScrollUseLocalDate;
	else _MultiTexState &= ~ ScrollUseLocalDate;
}


//==========================================
void	CPSMultiTexturedParticle::setUseLocalDateAlt(bool use)
{
	if (use) _MultiTexState |= ScrollUseLocalDateAlternate;
	else _MultiTexState &= ~ ScrollUseLocalDateAlternate;
}

} // NL3D





