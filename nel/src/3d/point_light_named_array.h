/** \file point_light_named_array.h
 * <File description>
 *
 * $Id: point_light_named_array.h,v 1.2 2002/04/16 16:21:47 vizerie Exp $
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

#ifndef NL_POINT_LIGHT_NAMED_ARRAY_H
#define NL_POINT_LIGHT_NAMED_ARRAY_H

#include "nel/misc/types_nl.h"
#include "3d/point_light_named.h"


namespace NL3D {


// ***************************************************************************
/**
 * A class used bi CInstanceGroup and CZone. Just a list of pointLights, grouped by name.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightNamedArray
{
public:

	/// Constructor
	CPointLightNamedArray();

	/// clear All: pointLights and map.
	void			clear();

	/** sort pointLights and make Groups by name for setFactor().
	 *	indexRemap gets news Ids for pointLights. 
	 *	eg: old PointLight n°0 is now at indexRemap[0].
	 */
	void			build(const std::vector<CPointLightNamed> &pointLights, std::vector<uint> &indexRemap);


	/// get the pointLights sorted (const version).
	const std::vector<CPointLightNamed>		&getPointLights() const {return _PointLights;}

	/// get the pointLights sorted (mutable version). Be sure what you do!
	std::vector<CPointLightNamed>		&getPointLights() {return _PointLights;}

	/// set the Light factor for all pointLights "lightGroupName".
	void			setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor);

	// serial
	void			serial(NLMISC::IStream &f);

private:

	/// List of pointLight
	std::vector<CPointLightNamed>	_PointLights;

	/// LightGroupName mgt.
	struct	CPointLightGroup
	{
		uint32	StartId;	// start in the array.
		uint32	EndId;		// EndId-StartId==number of pointlights with this name.

		void	serial(NLMISC::IStream &f)
		{
			f.serial(StartId, EndId);
		}
	};
	typedef std::map<std::string, CPointLightGroup>				TPLGMap;
	typedef std::map<std::string, CPointLightGroup>::iterator	ItPLGMap;
	/// Info for LightGroupName and setPointLightFactor
	TPLGMap							_PointLightGroupMap;

};


} // NL3D


#endif // NL_POINT_LIGHT_NAMED_ARRAY_H

/* End of point_light_named_array.h */
