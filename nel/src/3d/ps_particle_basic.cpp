/** \file ps_particle_basic.cpp
 * base classes for particles
 *
 * $Id: ps_particle_basic.cpp,v 1.1 2001/12/06 17:03:43 vizerie Exp $
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

#include "3d/ps_particle_basic.h"
#include "3d/driver.h"
#include "3d/texture_bump.h"



namespace NL3D 
{

bool CPSMultiTexturedParticle::_ForceBasicCaps  = false;

//=======================================
CPSMultiTexturedParticle::CPSMultiTexturedParticle() : _MultiTexState(TouchFlag)
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
	sint ver = f.serialVersion(0);
	f.serial(_MultiTexState);
	if (isMultiTextureEnabled())
	{
		f.serialEnum(_MainOp);
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
				setupMultiTexEnv(_MainOp, primary, _Texture2, mat);
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
	forceBasicCapsLocal(areBasicCapsForced());
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





