/** \file animated_material.cpp
 * <File description>
 *
 * $Id: animated_material.cpp,v 1.5 2001/03/28 10:33:00 berenguier Exp $
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

#include "nel/3d/animated_material.h"
#include "nel/misc/common.h"

using namespace NLMISC;

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void	CMaterialBase::CAnimatedTexture::serial(NLMISC::IStream &f)
{
	ITexture	*text= NULL;
	if(f.isReading())
	{
		f.serialPolyPtr(text);
		Texture= text;
	}
	else
	{
		text= Texture;
		f.serialPolyPtr(text);
	}
}



// ***************************************************************************
CMaterialBase::CMaterialBase()
{
	DefaultAmbient.setValue(CRGBA(64,64,64));
	DefaultDiffuse.setValue(CRGBA(128,128,128));
	DefaultSpecular.setValue(CRGBA(0,0,0));
	DefaultShininess.setValue(10);
	DefaultEmissive.setValue(1);
	DefaultOpacity.setValue(1);
	DefaultTexture.setValue(0x7FFFFFFF);


	EmissiveFactor.set(0,0,0,0);
}


// ***************************************************************************
void	CMaterialBase::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serial(Name);
	f.serial(DefaultAmbient, DefaultDiffuse, DefaultSpecular);
	f.serial(DefaultShininess, DefaultEmissive, DefaultOpacity, DefaultTexture);
	f.serial(EmissiveFactor);

	f.serialMap(_AnimatedTextures);
}


// ***************************************************************************
void	CMaterialBase::copyFromMaterial(CMaterial *pMat)
{
	DefaultAmbient.setValue(pMat->getAmbient());
	DefaultDiffuse.setValue(pMat->getDiffuse());
	DefaultSpecular.setValue(pMat->getSpecular());
	DefaultShininess.setValue(pMat->getShininess());
	DefaultEmissive.setValue(1);
	DefaultOpacity.setValue(pMat->getDiffuse().A/255.f);

	EmissiveFactor= pMat->getEmissive();
}


// ***************************************************************************
void			CMaterialBase::setAnimatedTexture(uint32 id, CSmartPtr<ITexture>  pText)
{
	// add or replace the texture.
	_AnimatedTextures[id].Texture= pText;
}
// ***************************************************************************
bool			CMaterialBase::validAnimatedTexture(uint32 id)
{
	TAnimatedTextureMap::iterator	it;
	it= _AnimatedTextures.find(id);
	return it!=_AnimatedTextures.end();
}
// ***************************************************************************
ITexture*		CMaterialBase::getAnimatedTexture(uint32 id)
{
	TAnimatedTextureMap::iterator	it;
	it= _AnimatedTextures.find(id);
	if( it!=_AnimatedTextures.end() )
		return it->second.Texture;
	else
		return NULL;
}




// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CAnimatedMaterial::CAnimatedMaterial(CMaterialBase *baseMat)
{
	nlassert(baseMat);
	_MaterialBase= baseMat;

	_EmissiveFactor= _MaterialBase->EmissiveFactor;

	_Ambient.affect(_MaterialBase->DefaultAmbient.getValue());
	_Diffuse.affect(_MaterialBase->DefaultDiffuse.getValue());
	_Specular.affect(_MaterialBase->DefaultSpecular.getValue());
	_Shininess.affect(_MaterialBase->DefaultShininess.getValue());
	_Emissive.affect(_MaterialBase->DefaultEmissive.getValue());
	_Opacity.affect(_MaterialBase->DefaultOpacity.getValue());


	// IAnimatable.
	IAnimatable::resize(AnimValueLast);
}


// ***************************************************************************
void	CAnimatedMaterial::setMaterial(CMaterial *pMat)
{
	_Material= pMat;
}


// ***************************************************************************
std::string		CAnimatedMaterial::getMaterialName() const
{
	nlassert(_MaterialBase);
	return _MaterialBase->Name;
}


// ***************************************************************************
void	CAnimatedMaterial::update()
{
	if(isTouched(OwnerBit) && _Material!=NULL && _Material->isLighted())
	{
		// well, just update all...  :)

		// em part.
		CRGBA	em= _EmissiveFactor;
		sint	c= (sint)(_Emissive.Value*255);
		clamp(c, 0, 255);
		em.blendFromui(CRGBA(0,0,0,0), em, c);

		// diffuse part.
		CRGBA	diff= _Diffuse.Value;
		c= (sint)(_Opacity.Value*255);
		clamp(c, 0, 255);
		diff.A= c;

		// setup material.
		_Material->setLighting(true, false, em, _Ambient.Value, diff, _Specular.Value, _Shininess.Value);

		// clear flags.
		clearFlag(AmbientValue);
		clearFlag(DiffuseValue);
		clearFlag(SpecularValue);
		clearFlag(ShininessValue);
		clearFlag(EmissiveValue);
		clearFlag(OpacityValue);


		// Texture Anim.
		if(isTouched(TextureValue))
		{
			nlassert(_MaterialBase);

			uint32	id= _Texture.Value;
			if(_MaterialBase->validAnimatedTexture(id))
			{
				_Material->setTexture(0, _MaterialBase->getAnimatedTexture(id) );
			}
			clearFlag(TextureValue);
		}


		// We are OK!
		IAnimatable::clearFlag(OwnerBit);
	}
}


// ***************************************************************************
IAnimatedValue* CAnimatedMaterial::getValue (uint valueId)
{
	switch(valueId)
	{
	case AmbientValue: return &_Ambient;
	case DiffuseValue: return &_Diffuse;
	case SpecularValue: return &_Specular;
	case ShininessValue: return &_Shininess;
	case EmissiveValue: return &_Emissive;
	case OpacityValue: return &_Opacity;
	case TextureValue: return &_Texture;
	};

	// shoudl not be here!!
	nlstop;
	return NULL;
}
// ***************************************************************************
const char *CAnimatedMaterial::getValueName (uint valueId) const
{
	switch(valueId)
	{
	case AmbientValue: return getAmbientValueName();
	case DiffuseValue: return getDiffuseValueName();
	case SpecularValue: return getSpecularValueName();
	case ShininessValue: return getShininessValueName();
	case EmissiveValue: return getEmissiveValueName();
	case OpacityValue: return getOpacityValueName();
	case TextureValue: return getTextureValueName();
	};

	// shoudl not be here!!
	nlstop;
	return "";
}
// ***************************************************************************
ITrack*	CAnimatedMaterial::getDefaultTrack (uint valueId)
{
	nlassert(_MaterialBase);

	switch(valueId)
	{
	case AmbientValue: return	&_MaterialBase->DefaultAmbient;
	case DiffuseValue: return	&_MaterialBase->DefaultDiffuse;
	case SpecularValue: return	&_MaterialBase->DefaultSpecular;
	case ShininessValue: return	&_MaterialBase->DefaultShininess;
	case EmissiveValue: return	&_MaterialBase->DefaultEmissive;
	case OpacityValue: return	&_MaterialBase->DefaultOpacity;
	case TextureValue: return	&_MaterialBase->DefaultTexture;
	};

	// shoudl not be here!!
	nlstop;
	return NULL;
}
// ***************************************************************************
void	CAnimatedMaterial::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// For CAnimatedMaterial, channels are detailled (material rendered after clip)!
	addValue(chanMixer, AmbientValue, OwnerBit, prefix, true);
	addValue(chanMixer, DiffuseValue, OwnerBit, prefix, true);
	addValue(chanMixer, SpecularValue, OwnerBit, prefix, true);
	addValue(chanMixer, ShininessValue, OwnerBit, prefix, true);
	addValue(chanMixer, EmissiveValue, OwnerBit, prefix, true);
	addValue(chanMixer, OpacityValue, OwnerBit, prefix, true);
	addValue(chanMixer, TextureValue, OwnerBit, prefix, true);

}


} // NL3D
