/** \file mesh_base.h
 * <File description>
 *
 * $Id: mesh_base.h,v 1.2 2001/06/19 10:22:33 berenguier Exp $
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

#ifndef NL_MESH_BASE_H
#define NL_MESH_BASE_H

#include "nel/misc/types_nl.h"
#include "3d/shape.h"
#include "3d/driver.h"
#include "3d/material.h"
#include "3d/animated_material.h"
#include <set>
#include <vector>



namespace NL3D {


class	CMeshBaseInstance;


// ***************************************************************************
/**
 * A base mesh with just material manipulation. Used as an abstract base class for CMesh or CMeshMRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshBase : public IShape
{
public:

	/// \name Structures for building a mesh.
	//@{

	struct CMatStage
	{ 
		uint8 nMatNb, nStageNb; 
		void	serial(NLMISC::IStream &f)
		{
			f.serial(nMatNb);
			f.serial(nStageNb);
		}
	};
	struct CLightInfoMapList : std::list< CMatStage >
	{
		void	serial(NLMISC::IStream &f)
		{
			f.serialCont((std::list< CMatStage >&)*this);
		}
	};
	typedef std::map< std::string, CLightInfoMapList >	TLightInfoMap;

	/// A mesh material information.
	struct	CMeshBaseBuild
	{
		bool					bCastShadows;
		bool					bRcvShadows;

		// Default value for position of this mesh
		CVector					DefaultPos;
		CVector					DefaultPivot;
		CVector					DefaultRotEuler;
		CQuat					DefaultRotQuat;
		CVector					DefaultScale;

		// Material array
		std::vector<CMaterial>	Materials;

		// Map of light information
		TLightInfoMap			LightInfoMap;


		CMeshBaseBuild();

		// Serialization
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	//@}


public:
	/// Constructor
	CMeshBase();

	/// \name animated material mgt. do it after CMesh::build(), or CMeshMRM::build()
	// @{
	/// setup a material as animated. Material must exist or else no-op.
	void			setAnimatedMaterial(uint id, const std::string &matName);
	/// return NULL if this material is NOT animated. (or if material do not exist)
	CMaterialBase	*getAnimatedMaterial(uint id);
	// @}

	/// \name access default tracks.
	// @{
	CTrackDefaultVector*	getDefaultPos ()		{return &_DefaultPos;}
	CTrackDefaultVector*	getDefaultPivot ()		{return &_DefaultPivot;}
	CTrackDefaultVector*	getDefaultRotEuler ()	{return &_DefaultRotEuler;}
	CTrackDefaultQuat*		getDefaultRotQuat ()	{return &_DefaultRotQuat;}
	CTrackDefaultVector*	getDefaultScale ()		{return &_DefaultScale;}
	// @}

	/// \name Material accessors
	// @{

	/// Get the number of materials in the mesh
	uint getNbMaterial() const
	{
		return _Materials.size();
	}

	/// Get a material
	const CMaterial& getMaterial(uint id) const
	{
		return _Materials[id];
	}
	// @}


	/// serial the base Part of this mesh.
	void	serialMeshBase(NLMISC::IStream &f) throw(NLMISC::EStream);


// ************************
protected:
	/// The Materials.
	std::vector<CMaterial>		_Materials;


public:
	// Map of light information ( LightName, list(MaterialNb, StageNb) )
	TLightInfoMap				_LightInfos;	


protected:
	/// Animated Material mgt.
	typedef std::map<uint32, CMaterialBase>	TAnimatedMaterialMap;
	TAnimatedMaterialMap		_AnimatedMaterials;


	/// Transform default tracks. Those default tracks are instancied, ie, CInstanceMesh will have the same and can't specialize it.
	CTrackDefaultVector			_DefaultPos;
	CTrackDefaultVector			_DefaultPivot;
	CTrackDefaultVector			_DefaultRotEuler;
	CTrackDefaultQuat			_DefaultRotQuat;
	CTrackDefaultVector			_DefaultScale;


protected:
	/// Just copy informations from a CMeshBaseBuild.
	void	buildMeshBase(CMeshBaseBuild &m);

	/// instanciate MeshBase part to an instance (a CMeshBaseInstance).
	void	instanciateMeshBase(CMeshBaseInstance *mi);

};


} // NL3D


#endif // NL_MESH_BASE_H

/* End of mesh_base.h */
