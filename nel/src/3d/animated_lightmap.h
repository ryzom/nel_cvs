/** \file animated_lightmap.h
 * <File description>
 *
 * $Id: animated_lightmap.h,v 1.3 2002/08/21 09:39:51 lecroart Exp $
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

#ifndef NL_ANIMATED_LIGHTMAP_H
#define NL_ANIMATED_LIGHTMAP_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "3d/animatable.h"
#include "3d/track.h"
#include <map>


namespace NL3D 
{

// ***************************************************************************
/**
 * An animated lightmap
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedLightmap : public IAnimatable
{
public:

	/// \name ctor / setup.
	// @{
	/** Constructor.
	 */
	CAnimatedLightmap();
	virtual ~CAnimatedLightmap() {}
	// @}


	/// \name Herited from IAnimatable
	// @{
	/// Added values.
	enum	TAnimValues
	{
		OwnerBit= IAnimatable::AnimValueLast,
		FactorValue, 
		AnimValueLast
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);

	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;

	/// From IAnimatable.
	virtual ITrack* getDefaultTrack (uint valueId);

	/// From IAnimatable.
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}

	void setName( const std::string &s ) { _Name = s; }
	std::string getName() const { return _Name; }

	NLMISC::CRGBA	getFactor() { return _Factor.Value; }

// ********************
private:

	std::string				_Name;

	// AnimValues.

	CAnimatedValueRGBA		_Factor;
	CTrackDefaultRGBA		_DefaultFactor; 
};


} // NL3D


#endif // NL_ANIMATED_LIGHTMAP_H

/* End of animated_lightmap.h */
