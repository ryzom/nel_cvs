/** \file point_light_user.cpp
 * <File description>
 *
 * $Id: point_light_user.cpp,v 1.5 2003/02/05 09:56:49 corvazier Exp $
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

#include "nel/misc/debug.h"
#include "3d/point_light_user.h"
#include "3d/point_light_model.h"


using namespace NLMISC;

namespace NL3D 
{



// ***************************************************************************
CPointLightUser::CPointLightUser(CScene *scene, IModel *trans) : 
  CTransformUser(scene, trans, true)
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel= safe_cast<CPointLightModel*>(_Transform);
}


// ***************************************************************************
void			CPointLightUser::setAmbient (NLMISC::CRGBA ambient)
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->PointLight.setAmbient(ambient);
}
void			CPointLightUser::setDiffuse (NLMISC::CRGBA diffuse)
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->PointLight.setDiffuse (diffuse);
}
void			CPointLightUser::setSpecular (NLMISC::CRGBA specular)
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->PointLight.setSpecular (specular);
}
void			CPointLightUser::setColor (NLMISC::CRGBA color)
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->PointLight.setColor (color);
}

// ***************************************************************************
NLMISC::CRGBA	CPointLightUser::getAmbient () const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getAmbient();
}
NLMISC::CRGBA	CPointLightUser::getDiffuse () const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getDiffuse ();
}
NLMISC::CRGBA	CPointLightUser::getSpecular () const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getSpecular();
}

// ***************************************************************************
void			CPointLightUser::setupAttenuation(float attenuationBegin, float attenuationEnd)
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->PointLight.setupAttenuation(attenuationBegin, attenuationEnd);
}
float			CPointLightUser::getAttenuationBegin() const
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getAttenuationBegin();
}
float			CPointLightUser::getAttenuationEnd() const
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getAttenuationEnd();
}


// ***************************************************************************
void			CPointLightUser::enableSpotlight(bool enable) 
{
	NL3D_MEM_POINTLIGHT
	if(enable)
		_PointLightModel->PointLight.setType(CPointLight::SpotLight);
	else
		_PointLightModel->PointLight.setType(CPointLight::PointLight);
}
bool			CPointLightUser::isSpotlight() const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getType() == CPointLight::SpotLight;
}
void			CPointLightUser::setupSpotAngle(float spotAngleBegin, float spotAngleEnd) 
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->PointLight.setupSpotAngle(spotAngleBegin, spotAngleEnd);
}
float			CPointLightUser::getSpotAngleBegin() const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getSpotAngleBegin();
}
float			CPointLightUser::getSpotAngleEnd() const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->PointLight.getSpotAngleEnd();
}

// ***************************************************************************
void			CPointLightUser::setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos) 
{
	NL3D_MEM_POINTLIGHT
	_PointLightModel->setDeltaPosToSkeletonWhenOutOfFrustum(deltaPos) ;
}
const CVector	&CPointLightUser::getDeltaPosToSkeletonWhenOutOfFrustum() const 
{
	NL3D_MEM_POINTLIGHT
	return _PointLightModel->getDeltaPosToSkeletonWhenOutOfFrustum() ;
}




} // NL3D
