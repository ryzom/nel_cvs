/** \file u_point_light.h
 * <File description>
 *
 * $Id: u_point_light.h,v 1.2 2002/02/18 13:23:34 berenguier Exp $
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


	/** Setup SpotLight. SpotLight is disabled by default. The direction of the spot is lead by the J vector of the 
	 *	UPointLight WorldMatrix
	 */
	virtual void			enableSpotlight(bool enable) =0;
	/// Is Spotlight enabled?
	virtual bool			isSpotlight() const =0;
	/** setup the spot AngleBegin and AngleEnd that define spot attenuation of the light. Usefull only if SpotLight
	 *	NB: clamp(angleBegin, 0, PI); clamp(angleEnd, angleBegin, PI); Default is PI/4, PI/2
	 */
	virtual void			setupSpotAngle(float spotAngleBegin, float spotAngleEnd) =0;
	/// get the begin radius of the SpotAngles.
	virtual float			getSpotAngleBegin() const =0;
	/// get the end radius of the SpotAngles.
	virtual float			getSpotAngleEnd() const =0;




	/**	setup the deltaPosToSkeletonWhenOutOfFrustum
	 *	When a light is sticked to a skeleton, and if this skeleton is clipped, then the position of the light 
	 *	can't be computed correctly without animating the skeleton. To allow good position of the light,
	 *	and to avoid recomputing the skeleton even if it is clipped, the light position is set to 
	 *	skeletonMatrix * this "deltaPosToSkeletonWhenOutOfFrustum".
	 *
	 *	Default is (0, 0, 1.5).
	 *	You may change this according to the approximate size of the skeleton (dwarf or giant), and you must
	 *	take into account any mount (horse etc...). eg for a man on a elephant, a good value would be (0,0,5) :)
	 */
	virtual void			setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos) =0;
	/// see setDeltaPosToSkeletonWhenOutOfFrustum()
	virtual const CVector	&getDeltaPosToSkeletonWhenOutOfFrustum() const =0;


};


} // NL3D


#endif // NL_U_POINT_LIGHT_H

/* End of u_point_light.h */
