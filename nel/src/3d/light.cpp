/** \file 3d/light.cpp
 * CLight definition
 *
 * $Id: light.cpp,v 1.6 2002/02/28 12:59:49 besson Exp $
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

#include "3d/light.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

void CLight::setupDirectional (const CRGBA& ambiant, const CRGBA& diffuse, const CRGBA& specular, const CVector& direction,
							   float constant, float linear, float quadratic)
{
	// Set the mode
	setMode (DirectionalLight);

	// Set the colors
	setAmbiant (ambiant);
	setDiffuse (diffuse);
	setSpecular (specular);

	// Set the direction
	setDirection (direction);

	// Set attenuation
	setConstantAttenuation (constant);
	setLinearAttenuation (linear);
	setQuadraticAttenuation (quadratic);
}
	
// ***************************************************************************

void CLight::setupPointLight (const CRGBA& ambiant, const CRGBA& diffuse, const CRGBA& specular, const CVector& position, 
						const CVector& direction, float constant, float linear, float quadratic)
{
	// Set the mode
	setMode (PointLight);

	// Set the colors
	setAmbiant (ambiant);
	setDiffuse (diffuse);
	setSpecular (specular);

	// Set the position and direction
	setPosition (position);
	setDirection (direction);

	// Set attenuation
	setConstantAttenuation (constant);
	setLinearAttenuation (linear);
	setQuadraticAttenuation (quadratic);
}

// ***************************************************************************

void CLight::setupSpotLight (const CRGBA& ambiant, const CRGBA& diffuse, const CRGBA& specular, const CVector& position, 
						const CVector& direction, float exponent, float cutoff, float constant, float linear, float quadratic)
{
	// Set the mode
	setMode (SpotLight);

	// Set the colors
	setAmbiant (ambiant);
	setDiffuse (diffuse);
	setSpecular (specular);

	// Set the position and direction
	setPosition (position);
	setDirection (direction);

	// Set spotlight parameters
	setExponent (exponent);
	setCutoff (cutoff);

	// Set attenuation
	setConstantAttenuation (constant);
	setLinearAttenuation (linear);
	setQuadraticAttenuation (quadratic);
}

// ***************************************************************************

void CLight::setupAttenuation (float farAttenuationBegin, float farAttenuationEnd)
{
	_ConstantAttenuation=1.f;
	_QuadraticAttenuation=(float)(0.1/(0.9*farAttenuationBegin*farAttenuationBegin));
	_LinearAttenuation=(float)(0.1/(0.9*farAttenuationBegin));

	// blend factor
	float factor = (0.1f*_LinearAttenuation*farAttenuationEnd-0.1f*_QuadraticAttenuation*farAttenuationEnd*farAttenuationEnd);

	if (factor == 0.0f)
		factor = 0.0001f;
	factor = (0.9f-0.1f*_QuadraticAttenuation*farAttenuationEnd*farAttenuationEnd)/factor;

	if ((factor<0.f)||(factor>1.f))
	{
		// Better factor
		float d0_1Lin=1.f / ( _ConstantAttenuation + _LinearAttenuation*farAttenuationEnd );
		float d0_1Quad=1.f / ( _ConstantAttenuation + _QuadraticAttenuation*farAttenuationEnd*farAttenuationEnd );

		// Better
		if (fabs (d0_1Lin-0.1f)<fabs (d0_1Quad-0.1f))
			_QuadraticAttenuation=0.f;
		else
			_LinearAttenuation=0.f;
	}
	else
	{
		_LinearAttenuation*=factor;
		_QuadraticAttenuation*=(1.f-factor);
	}

#ifdef NL_DEBUG
	// Should be near previous result
	float d1_0=1.f / ( _ConstantAttenuation + _LinearAttenuation*0.f + _QuadraticAttenuation*0.f*0.f );
	float d0_9=1.f / ( _ConstantAttenuation + _LinearAttenuation*farAttenuationBegin + _QuadraticAttenuation*farAttenuationBegin*farAttenuationBegin );
	float d0_1=1.f / ( _ConstantAttenuation + _LinearAttenuation*farAttenuationEnd + _QuadraticAttenuation*farAttenuationEnd*farAttenuationEnd );
#endif // NL_DEBUG*/
}

// ***************************************************************************

void CLight::setupSpotExponent (float hotSpotAngle)
{
	float divid=(float)log (cos (hotSpotAngle));
	if (divid==0.f)
		divid=0.0001f;
	setExponent ((float)(log (0.9)/divid));
}

// ***************************************************************************

void CLight::setNoAttenuation ()
{
	_ConstantAttenuation=1.f;
	_QuadraticAttenuation=0.f;
	_LinearAttenuation=0.f;
}

// ***************************************************************************

} // NL3D
