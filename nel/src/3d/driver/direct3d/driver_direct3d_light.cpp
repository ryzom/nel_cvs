/** \file driver_direct3d_light.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_light.cpp,v 1.5.4.1 2004/09/14 15:33:43 vizerie Exp $
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
	H_AUTO_D3D(CDriverD3D_setLight);
	// bkup real light, for lightmap dynamic lighting purpose
	if(index==0)
	{
		_UserLight0= light;
		// because the D3D setup change, must dirt lightmap rendering
		_LightMapDynamicLightDirty= true;
	}
	
	setLightInternal(index, light);
}
	

// ***************************************************************************
void CDriverD3D::enableLight (uint8 index, bool enable)
{
	H_AUTO_D3D(CDriverD3D_enableLight);
	// User call => set the User flag
	if(index<MaxLight)
	{
		_UserLightEnable[index]= enable;
	}
	
	// enable the light in D3D
	enableLightInternal(index, enable);
	
	// because the D3D setup has changed, must dirt lightmap rendering
	_LightMapDynamicLightDirty= true;
}

static const float sqrtFLT_MAX = (float) sqrtf(FLT_MAX);

// ***************************************************************************
void CDriverD3D::setLightInternal (uint8 index, const CLight &light)
{
	H_AUTO_D3D(CDriverD3D_setLightInternal);
	nlassert (_DeviceInterface);	
	if (index<MaxLight)
	{
		// Ref on the state
		D3DLIGHT9 &lightRef = _LightCache[index].Light;		
		lightRef.Type = RemapLightTypeNeL2D3D[light.getMode ()];
		NL_D3DCOLORVALUE_RGBA(lightRef.Diffuse, light.getDiffuse());
		NL_D3DCOLORVALUE_RGBA(lightRef.Specular, light.getSpecular());
		NL_D3DCOLORVALUE_RGBA(lightRef.Ambient, light.getAmbiant());
		CVector vect = light.getPosition();
		NL_D3DVECTOR_VECTOR (lightRef.Position, vect);
		vect = light.getDirection();
		NL_D3DVECTOR_VECTOR (lightRef.Direction, vect);
		lightRef.Range = sqrtFLT_MAX;
		lightRef.Falloff = 1;
		lightRef.Attenuation0 = light.getConstantAttenuation();
		lightRef.Attenuation1 = light.getLinearAttenuation();
		lightRef.Attenuation2 = light.getQuadraticAttenuation();
		if (lightRef.Type == D3DLIGHT_SPOT)
		{			
			lightRef.Phi = light.getCutoff();			
			float divid=light.getExponent();
			if (divid==0.f)
				divid=0.0001f;
			float hotSpotAngle = (float)acos(exp(log (0.9)/divid));
			lightRef.Theta = hotSpotAngle;
		}
		else
		{
			lightRef.Phi = (float) NLMISC::Pi * 0.5f;
			lightRef.Theta = (float) NLMISC::Pi * 0.25f;
		}

		// Settings touched
		_LightCache[index].SettingsTouched = true;

		// Touch only if enabled
		if (_LightCache[index].Enabled)
			touchRenderVariable (&_LightCache[index]);		
	}	
}

// ***************************************************************************
void CDriverD3D::enableLightInternal (uint8 index, bool enable)
{
	H_AUTO_D3D(CDriverD3D_enableLightInternal);
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
	H_AUTO_D3D(CDriverD3D_getMaxLight);	
	return _MaxLight;
}

// ***************************************************************************
void CDriverD3D::setAmbientColor (CRGBA color)
{
	H_AUTO_D3D(CDriverD3D_setAmbientColor);	
	setRenderState(D3DRS_AMBIENT, NL_D3DCOLOR_RGBA(color));
}


// ***************************************************************************
void CDriverD3D::setLightMapDynamicLight (bool enable, const CLight& light)
{
	H_AUTO_D3D(CDriverD3D_setLightMapDynamicLight);	
	// just store, for future setup in lightmap material rendering
	_LightMapDynamicLightEnabled= enable;
	_LightMapDynamicLight= light;
	_LightMapDynamicLightDirty= true;
}


// ***************************************************************************
void			CDriverD3D::setupLightMapDynamicLighting(bool enable)
{
 H_AUTO_D3D(CDriverD3D_setupLightMapDynamicLighting);
	// start lightmap dynamic lighting
	if(enable)
	{
		// disable all lights but the 0th.
		for(uint i=1;i<_MaxLight;i++)
			enableLightInternal(i, false);
		
		// if the dynamic light is really enabled
		if(_LightMapDynamicLightEnabled)
		{
			// then setup and enable
			setLightInternal(0, _LightMapDynamicLight);
			enableLightInternal(0, true);
		}
		// else just disable also the light 0
		else
		{
			enableLightInternal(0, false);
		}
		
		// ok it has been setup
		_LightMapDynamicLightDirty= false;
	}
	// restore old lighting
	else
	{
		// restore the light 0
		setLightInternal(0, _UserLight0);
		
		// restore all standard light enable states
		for(uint i=0;i<_MaxLight;i++)
			enableLightInternal(i, _UserLightEnable[i]);
	}
}



} // NL3D

