/** \file mesh_multi_lod.h
 * Mesh with several LOD meshes.
 *
 * $Id: mesh_multi_lod.h,v 1.3 2001/07/04 16:24:41 corvazier Exp $
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

#ifndef NL_MESH_MULTI_LOD_H
#define NL_MESH_MULTI_LOD_H

#include "nel/misc/types_nl.h"

#include "3d/mesh.h"
#include "3d/mesh_base.h"
#include "3d/mesh_geom.h"
#include "3d/mrm_parameters.h"

namespace NL3D 
{

class CMeshMultiLodInstance;

/**
 * Mesh with several LOD meshes.
 *
 * This mesh handle several meshes of any kind of shape (MRM, standard, coarse meshes..)
 * At run time, it chooses what LOD meshes it must render according to its settings.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMultiLod : public CMeshBase
{
public:

	/// \name Structures for building a multi lod mesh.
	//@{

	/// Class used to build a multi lod mesh
	class CMeshMultiLodBuild
	{
	public:

		/// A slot of mesh for the build
		class CBuildSlot
		{
		public:
			/** 
			  * Flags for the build of a slot
			  * 
			  * BlendIn:	if this flag is specified, this mesh will blend before be displayed.
			  * BlendOut:	if this flag is specified, this mesh will blend before disapear.
			  * CoarseMesh: if this flag is specified, this mesh is a coarse mesh.
			  * StaticCoarseMesh: if this flag and CoarseMesh are specified, this mesh is a static 
			  *		coarse mesh else if only CoarseMesh is specified, this mesh is a dynamic coarse mesh.
			  */
			enum
			{
				BlendIn				=	0x01,
				BlendOut			=	0x02,
				CoarseMesh			=	0x04,
			};

			/**
			  * A mesh base build to describe the mesh. Can't be NULL. The pointer is owned by the CMeshMultiLod
			  * after the call.
			  */
			IMeshGeom			*MeshGeom;

			/// Distance before which this lod is displayed
			float				DistMax;

			/// Length of the blend used to show this mesh
			float				BlendLength;

			/// Flags for the build. See flags description.
			uint8				Flags;
		};

		/// True if this mesh is a static lod (static means it doesn't move at each frame), else false for dynamic.
		bool						StaticLod;

		/// The mesh base build structure
		CMeshBase::CMeshBaseBuild	BaseMesh;

		/// An array of basic mesh build 
		std::vector<CBuildSlot>		LodMeshes;
	};

	/// Build a mesh from material info, and a builded MeshGeom. WARNING: This has a side effect of deleting AnimatedMaterials.
	void			build(CMeshMultiLodBuild &mbuild);

	/// \name From IShape
	// @{

	/// Create a CMeshInstance, which contains materials.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid);

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans);

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// Declare name of the shape
	NLMISC_DECLARE_CLASS(CMeshMultiLod);

	// @}

private:

	/// Flags of CMeshSlot
	enum
	{
		BlendIn				=	0x01,
		BlendOut			=	0x02,
		CoarseMesh			=	0x04,
		CoarseMeshLoaded	=	0x08,
	};
	
	/** 
	  * This is a slot of the mesh base list
	  * 
	  * A LOD "currentLOD" is displayed between distances:  
	  *
	  *    [previousLOD->DistMax - currentLOD->BlendLength   ;   currentLOD->DistMax]
	  */
	class CMeshSlot
	{
	public:
		~CMeshSlot ();

		/// The mesh base. Can be NULL if the geom mesh has not been loaded.
		IMeshGeom	*MeshGeom;

		/// Dist max to show this mesh
		float		DistMax;

		/// Polygon count at the begining of the slot interval
		float		BeginPolygonCount;

		/// Polygon count at the end of the slot interval
		float		EndPolygonCount;

		/// Length of the blend used to show this mesh
		float		StartBlendPolygonCount;

		/// Length of the blend used to show this mesh
		float		BlendLength;

		/// Blend On/Off, misc flags
		uint8		Flags;

		/// Coarse mesh id
		uint64		CoarseMeshId;

		/// Serial
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};

	/// Static or dynamic load ?
	bool						_StaticLod;

	/// Vector of meshes
	std::vector<CMeshSlot>		_MeshVector;

	/// Clear the mesh
	void	clear ();

	/// Render a slot
	void	render (uint slot, IDriver *drv, CMeshMultiLodInstance *trans, float numPoylgons, float alpha, bool staticLod);
};


} // NL3D


#endif // NL_MESH_MULTI_LOD_H

/* End of mesh_multi_lod.h */
