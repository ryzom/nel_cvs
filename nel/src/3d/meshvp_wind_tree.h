/** \file meshvp_wind_tree.h
 * <File description>
 *
 * $Id: meshvp_wind_tree.h,v 1.1 2002/02/26 14:17:55 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_MESHVP_WIND_TREE_H
#define NL_MESHVP_WIND_TREE_H

#include "nel/misc/types_nl.h"
#include "3d/mesh_vertex_program.h"
#include "3d/vertex_program.h"


namespace NL3D {


// ***************************************************************************
/**
 * VertexProgram for an effect of Wind on Tree meshes.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CMeshVPWindTree : public IMeshVertexProgram
{
public:

	enum	{HrcDepth= 3};

	/// \name WindTree Parameters;
	// @{

	/// Frequency of the wind for 3 Hierachy levels
	float		Frequency[HrcDepth];
	/// Additional frequency, multiplied by the globalWindPower
	float		FrequencyWindFactor[HrcDepth];
	/// Power of the wind on XY. Mul by globalWindPower
	float		PowerXY[HrcDepth];
	/// Power of the wind on Z. Mul by globalWindPower
	float		PowerZ[HrcDepth];
	/// Bias result of the cosinus: f= cos(time)+bias.
	float		Bias[HrcDepth];

	/// true if want Specular Lighting.
	bool		SpecularLighting;

	// @}

public:

	/// Constructor
	CMeshVPWindTree();
	virtual ~CMeshVPWindTree();


	/// \name IMeshVertexProgram implementation
	// @{

	/// Setup a rand phase for wind in mbi
	virtual	void	initInstance(CMeshBaseInstance *mbi);
	/// Setup Wind constants, Light constants, and activate the VP.
	virtual	void	begin(IDriver *drv, CScene *scene, CMeshBaseInstance *mbi);
	/// disable the VertexProgram.
	virtual	void	end(IDriver *drv);
	/// use Lighting. (ret true). supportSpecular= SpecularLighting
	virtual bool	useSceneVPLightSetup(bool &supportSpecular, uint &lightCteStart) const;


	// Serial.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshVPWindTree);

	// @}

private:

	// The 4 versions: Specular or not (0 or 2), + normalize normal or not (0 or 1).
	static	std::auto_ptr<CVertexProgram>	_VertexProgram[4];

	// WindTree Time for this mesh param setup. Stored in mesh because same for all instances.
	float		_CurrentTime[HrcDepth];
	double		_LastSceneTime;


	// Compute a cosinus with an angle given in 0-1 <=> 0-2Pi. Actual values goes from 0 to 2.
	static float	speedCos(float angle);
};


} // NL3D


#endif // NL_MESHVP_WIND_TREE_H

/* End of meshvp_wind_tree.h */
