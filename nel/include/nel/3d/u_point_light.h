/** \file u_point_light.h
 * <File description>
 *
 * $Id: u_point_light.h,v 1.1 2002/02/06 16:53:13 berenguier Exp $
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

#ifndef NL_U_POINT_LIGHT_H
#define NL_U_POINT_LIGHT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/3d/u_transform.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * Game interface for manipulating Dynamic Lights
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UPointLight : virtual public UTransform
{
protected:

	/// \name Object
	// @{
	UPointLight() {}
	virtual	~UPointLight() {}
	// @}


public:

	/// Set the ambient color of the light. Default to Black
	virtual void			setAmbient (NLMISC::CRGBA ambient) =0;
	/// Set the diffuse color of the light. Default to White
	virtual void			setDiffuse (NLMISC::CRGBA diffuse) =0;
	/// Set the specular color of the light. Default to White
	virtual void			setSpecular (NLMISC::CRGBA specular) =0;
	/// Set the diffuse and specular color of the light to the same value. don't modify _Ambient.
	virtual void			setColor (NLMISC::CRGBA color) =0;

	/// Get the ambient color of the light.
	virtual NLMISC::CRGBA	getAmbient () const =0;
	/// Get the diffuse color of the light.
	virtual NLMISC::CRGBA	getDiffuse () const =0;
	/// Get the specular color of the light.
	virtual NLMISC::CRGBA	getSpecular () const =0;


	/** setup the attenuation of the light. if (0,0) attenuation is disabled.
	 *	clamp(attenuationBegin,0 , +oo) and clamp(attenuationEnd, attenuationBegin, +oo)
	 *	By default, attenuation is 10-30.
	 *	PERFORMANCE WARNING: big lights (disabled attenuation and big attenuationEnd) slow down
	 *	performances. (by experience, with a factor of 2).
	 */
	virtual void			setupAttenuation(float attenuationBegin, float attenuationEnd) =0;
	/// get the begin radius of the attenuation.
	virtual float			getAttenuationBegin() const =0;
	/// get the end radius of the attenuation.
	virtual float			getAttenuationEnd() const =0;

};


} // NL3D


#endif // NL_U_POINT_LIGHT_H

/* End of u_point_light.h */
