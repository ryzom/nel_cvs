/** \file point_light_named.cpp
 * <File description>
 *
 * $Id: point_light_named.cpp,v 1.4 2003/03/31 12:47:48 corvazier Exp $
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

#include "3d/point_light_named.h"


namespace NL3D {


// ***************************************************************************
CPointLightNamed::CPointLightNamed()
{
	// copy setup from current
	_DefaultAmbient= getAmbient();
	_DefaultDiffuse= getDiffuse();
	_DefaultSpecular= getSpecular();
}


// ***************************************************************************
void			CPointLightNamed::setLightFactor(NLMISC::CRGBA nFactor)
{
	CRGBA	col;
	// setup current ambient.
	col.modulateFromColor(_DefaultAmbient, nFactor);
	setAmbient(col);
	// setup current diffuse.
	col.modulateFromColor(_DefaultDiffuse, nFactor);
	setDiffuse(col);
	// setup current specular.
	col.modulateFromColor(_DefaultSpecular, nFactor);
	setSpecular(col);
}


// ***************************************************************************
void			CPointLightNamed::serial(NLMISC::IStream &f)
{
	sint ver = f.serialVersion(1);

	// Serialize parent.
	CPointLight::serial(f);

	// Serialize my data
	f.serial(AnimatedLight);
	f.serial(_DefaultAmbient);
	f.serial(_DefaultDiffuse);
	f.serial(_DefaultSpecular);

	if (ver>=1)
		f.serial(LightGroup);
}

} // NL3D
