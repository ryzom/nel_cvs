/** \file driver_direct3d_light.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_light.cpp,v 1.3 2004/04/26 13:48:23 corvazier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
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

#include "stddirect3d.h"

#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

const D3DLIGHTTYPE  RemapLightTypeNeL2D3D[3]=
{
	D3DLIGHT_DIRECTIONAL,	// CLight::DirectionalLight
	D3DLIGHT_POINT,			// CLight::PointLight
	D3DLIGHT_SPOT,			// CLight::SpotLight 
};

// ***************************************************************************

void CDriverD3D::setLight (uint8 index, const CLight &light)
{
	nlassert (_DeviceInterface);
	if (index<MaxLight)
	{
		// Ref on the state
		D3DLIGHT9 &lightRef = _LightCache[index].Light;

		lightRef.Type = RemapLightTypeNeL2D3D[light.getMode ()];
		NL_D3DCOLORVALUE_RGBA (lightRef.Diffuse, light.getDiffuse());
		NL_D3DCOLORVALUE_RGBA (lightRef.Specular, light.getSpecular());
		NL_D3DCOLORVALUE_RGBA (lightRef.Ambient, light.getAmbiant());
		CVector vect = light.getPosition();
		NL_D3DVECTOR_VECTOR (lightRef.Position, vect);
		vect = light.getDirection();
		NL_D3DVECTOR_VECTOR (lightRef.Direction, vect);
		lightRef.Range = (float)sqrt(FLT_MAX);
		lightRef.Falloff = 1;
		lightRef.Attenuation0 = light.getConstantAttenuation();
		lightRef.Attenuation1 = light.getLinearAttenuation();
		lightRef.Attenuation2 = light.getQuadraticAttenuation();
		lightRef.Phi = light.getCutoff();
		
		float divid=light.getExponent();
		if (divid==0.f)
			divid=0.0001f;
		float hotSpotAngle = (float)acos(exp(log (0.9)/divid));
		lightRef.Theta = hotSpotAngle;

		// Settings touched
		_LightCache[index].SettingsTouched = true;

		// Touch only if enabled
		if (_LightCache[index].Enabled)
			touchRenderVariable (&_LightCache[index]);
	}
}

// ***************************************************************************

void CDriverD3D::enableLight (uint8 index, bool enable)
{
	nlassert (_DeviceInterface);
	if (index<MaxLight)
	{
		if (_LightCache[index].Enabled != enable)
		{
			_LightCache[index].Enabled = enable;
			_LightCache[index].EnabledTouched = true;
			touchRenderVariable (&_LightCache[index]);
		}
	}
}

// ***************************************************************************

uint CDriverD3D::getMaxLight () const 
{
	return _MaxLight;
}

// ***************************************************************************

void CDriverD3D::setAmbientColor (CRGBA color)
{
	setRenderState(D3DRS_AMBIENT, NL_D3DCOLOR_RGBA(color));
}

// ***************************************************************************

} // NL3D
