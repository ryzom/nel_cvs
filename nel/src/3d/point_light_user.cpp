/** \file point_light_user.cpp
 * <File description>
 *
 * $Id: point_light_user.cpp,v 1.1 2002/02/06 16:54:56 berenguier Exp $
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

#include "nel/misc/debug.h"
#include "3d/point_light_user.h"
#include "3d/point_light_model.h"


using namespace NLMISC;

namespace NL3D 
{



// ***************************************************************************
CPointLightUser::CPointLightUser(CScene *scene, IModel *trans) : 
  CTransformUser(scene, trans)
{
	_PointLightModel= safe_cast<CPointLightModel*>(_Transform);
}


// ***************************************************************************
void			CPointLightUser::setAmbient (NLMISC::CRGBA ambient)
{
	_PointLightModel->PointLight.setAmbient(ambient);
}
void			CPointLightUser::setDiffuse (NLMISC::CRGBA diffuse)
{
	_PointLightModel->PointLight.setDiffuse (diffuse);
}
void			CPointLightUser::setSpecular (NLMISC::CRGBA specular)
{
	_PointLightModel->PointLight.setSpecular (specular);
}
void			CPointLightUser::setColor (NLMISC::CRGBA color)
{
	_PointLightModel->PointLight.setColor (color);
}

// ***************************************************************************
NLMISC::CRGBA	CPointLightUser::getAmbient () const 
{
	return _PointLightModel->PointLight.getAmbient();
}
NLMISC::CRGBA	CPointLightUser::getDiffuse () const 
{
	return _PointLightModel->PointLight.getDiffuse ();
}
NLMISC::CRGBA	CPointLightUser::getSpecular () const 
{
	return _PointLightModel->PointLight.getSpecular();
}

// ***************************************************************************
void			CPointLightUser::setupAttenuation(float attenuationBegin, float attenuationEnd)
{
	_PointLightModel->PointLight.setupAttenuation(attenuationBegin, attenuationEnd);
}
float			CPointLightUser::getAttenuationBegin() const
{
	return _PointLightModel->PointLight.getAttenuationBegin();
}
float			CPointLightUser::getAttenuationEnd() const
{
	return _PointLightModel->PointLight.getAttenuationEnd();
}




} // NL3D
