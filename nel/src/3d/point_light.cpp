/** \file point_light.cpp
 * <File description>
 *
 * $Id: point_light.cpp,v 1.1 2002/02/06 16:54:56 berenguier Exp $
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

#include "3d/point_light.h"
#include "3d/light.h"
#include "3d/transform.h"
#include "nel/misc/common.h"
#include <algorithm>

using namespace NLMISC;
using namespace std;

namespace NL3D 
{


// ***************************************************************************
NLMISC::CBlockMemory<CTransform*, false>		CPointLight::_LightedModelListMemory(NL3D_LIGHTED_MODEL_ALLOC_BLOCKSIZE);


// ***************************************************************************
CPointLight::CPointLight() : _LightedModels(&_LightedModelListMemory)
{
	_Position= CVector::Null;
	_Ambient= CRGBA::Black;
	_Diffuse= _Specular= CRGBA::White;

	// Default setup. this is arbitrary
	_AttenuationBegin= 10;
	_AttenuationEnd= 30;

	// compute AttenuationFactors only one time.
	static	bool	done= false;
	static	float	cAtt, lAtt, qAtt;
	if(!done)
	{
		done= true;
		computeAttenuationFactors();
		// bkup setup.
		cAtt= _ConstantAttenuation;
		lAtt= _LinearAttenuation;
		qAtt= _QuadraticAttenuation;
	}
	else
	{
		// just copy bkuped setup.
		_ConstantAttenuation= cAtt;
		_LinearAttenuation= lAtt;
		_QuadraticAttenuation= qAtt;
	}
}


// ***************************************************************************
CPointLight::~CPointLight()
{
	resetLightedModels();
}


// ***************************************************************************
void			CPointLight::setupAttenuation(float attenuationBegin, float attenuationEnd)
{
	// set values.
	attenuationBegin= max(attenuationBegin, 0.f);
	attenuationEnd= max(attenuationEnd, attenuationBegin);
	_AttenuationBegin= attenuationBegin;
	_AttenuationEnd= attenuationEnd;

	// update factors.
	computeAttenuationFactors();

}


// ***************************************************************************
void			CPointLight::computeAttenuationFactors()
{
	// disable attenuation?
	if(_AttenuationBegin==0 && _AttenuationEnd==0)
	{
		// setup for attenuation disabled.
		_ConstantAttenuation= 1;
		_LinearAttenuation= 0;
		_QuadraticAttenuation= 0;
	}
	else
	{
		// precompute attenuation values, with help of CLight formula!!
		static	CLight	dummyLight;
		dummyLight.setupAttenuation(_AttenuationBegin, _AttenuationEnd);
		_ConstantAttenuation= dummyLight.getConstantAttenuation();
		_LinearAttenuation= dummyLight.getLinearAttenuation();
		_QuadraticAttenuation= dummyLight.getQuadraticAttenuation();
	}
}


// ***************************************************************************
void			CPointLight::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serial(_Position);
	f.serial(_Ambient);
	f.serial(_Diffuse);
	f.serial(_Specular);
	f.serial(_AttenuationBegin);
	f.serial(_AttenuationEnd);

	// precompute.
	if(f.isReading())
	{
		computeAttenuationFactors();
	}

}


// ***************************************************************************
void			CPointLight::setupDriverLight(CLight &light, uint8 factor)
{
	// expand 0..255 to 0..256, to avoid loss of precision.
	uint	ufactor= factor + (factor>>7);	// add 0 or 1.

	// modulate with factor
	CRGBA	ambient, diffuse, specular;
	ambient.modulateFromuiRGBOnly(_Ambient, ufactor);
	diffuse.modulateFromuiRGBOnly(_Diffuse, ufactor);
	specular.modulateFromuiRGBOnly(_Specular, ufactor);

	// setup the pointLight
	light.setupPointLight(ambient, diffuse, specular, _Position, CVector::Null, 
		_ConstantAttenuation, _LinearAttenuation, _QuadraticAttenuation);
}


// ***************************************************************************
void			CPointLight::resetLightedModels()
{
	// For each transform, resetLighting him.
	while(_LightedModels.begin() != _LightedModels.end() )
	{
		CTransform	*model= *_LightedModels.begin();
		// reset lighting
		model->resetLighting();

		// NB: the transform must erase him from this list.
		nlassert( _LightedModels.begin() == _LightedModels.end() || *_LightedModels.begin() != model );
	}
}


// ***************************************************************************
CPointLight::ItTransformList	CPointLight::appendLightedModel(CTransform *model)
{
	// append the entry in the list
	_LightedModels.push_back(model);
	ItTransformList	it= _LightedModels.end();
	it--;
	return it;
}
// ***************************************************************************
void			CPointLight::removeLightedModel(ItTransformList it)
{
	// delete the entry in the list.
	_LightedModels.erase(it);
}


} // NL3D
