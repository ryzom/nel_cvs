/** \file mesh_mrm.h
 * <File description>
 *
 * $Id: mesh_mrm.h,v 1.5 2001/06/22 16:26:46 berenguier Exp $
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

#ifndef NL_MESH_MRM_H
#define NL_MESH_MRM_H

#include "nel/misc/types_nl.h"
#include "3d/shape.h"
#include "3d/driver.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/uv.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/animated_material.h"
#include "3d/mesh_base.h"
#include "3d/mesh.h"
#include "3d/mrm_mesh.h"
#include "3d/mrm_parameters.h"
#include "3d/bone.h"
#include <set>
#include <vector>


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CPlane;
using	NLMISC::CMatrix;
class	CMRMBuilder;


// ***************************************************************************
/**
 * An instanciable MRM mesh.
 *
 * To build a CMeshMRM, you should:
 *	- build a CMesh::CMeshBuild   meshBuild (see CMesh)
 *	- call MeshMRM.build(mrmMeshBuild);
 *	- call if you want setAnimatedMaterial() etc...
 *
 * NB: internally, build() use CMRMBuilder, a builder of MRM.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshMRM : public CMeshBase
{
public:
	/// Constructor
	CMeshMRM();

	/** Build a mesh, replacing old. WARNING: This has a side effect of deleting AnimatedMaterials.
	 * this is much slower than CMesh::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build(CMesh::CMeshBuild &m, const CMRMParameters &params= CMRMParameters());


	/// \name From IShape
	// @{

	/// Create a CMeshInstance, which contains materials.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid);

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshMRM);

	// @}


	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const
	{
		return _BBox;
	}

	/// get the vertex buffer used by the mrm mesh. NB: this VB store all Vertices used by All LODs.
	const CVertexBuffer &getVertexBuffer() const { return _VBuffer ; }


	/** get the number of LOD.
	 */
	uint getNbLod() const { return _Lods.size() ; }


	/** get the number of rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 */
	uint getNbRdrPass(uint lodId) const { return _Lods[lodId].RdrPass.size() ; }


	/** get the primitive block associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *  \param renderingPassIndex the index of the rendering pass
	 */
	const CPrimitiveBlock &getRdrPassPrimitiveBlock(uint lodId, uint renderingPassIndex) const
	{
		return _Lods[lodId].RdrPass[renderingPassIndex].PBlock ;
	}


	/** get the material ID associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *	\param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint lodId, uint renderingPassIndex) const
	{
		return _Lods[lodId].RdrPass[renderingPassIndex].MaterialId ;
	}

	// @}


// ************************
private:
	friend class	CMRMBuilder;

	/// \name Structures for building a MRM mesh.
	//@{

	/// A block of primitives, sorted by material use.
	class	CRdrPass
	{
	public:
		// The id of this material.
		uint32				MaterialId;
		// The list of primitives.
		CPrimitiveBlock		PBlock;


		// Serialize a rdrpass.
		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(0);

			f.serial(MaterialId);
			f.serial(PBlock);
		}
	};


	/// A LOD of the MRM.
	class	CLod
	{
	public:
		/// The number of vertex in The VB this Lod needs.
		uint32						NWedges;
		/// List of geomorph, for this LOD.
		std::vector<CMRMWedgeGeom>	Geomorphs;
		/// List of rdr pass, for this LOD.
		std::vector<CRdrPass>		RdrPass;

		/** Skinning: list of influenced vertices to compute, for this lod only. There is 4 array, 0th 
		 *	is for vertices which have only one matrix. 1st if for vertices which have only 2 matrix ....
		 */
		std::vector<uint32>				InfluencedVertices[NL3D_MESH_SKINNING_MAX_MATRIX];
		/// Skinning: list of Matrix which influence this Lod. So we know what matrix to compute.
		std::vector<uint32>				MatrixInfluences;
		/// Skinning: does the VBuffer part of this Lod contains original skin vertices.
		bool						OriginalSkinRestored;

		CLod()
		{
			// By default, this is supposed false.
			OriginalSkinRestored= false;
		}

		// Serialize a Lod.
		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(0);
			uint	i;

			f.serial(NWedges);
			f.serialCont(RdrPass);
			f.serialCont(Geomorphs);
			f.serialCont(MatrixInfluences);

			// Serial array of InfluencedVertices. NB: code written so far for NL3D_MESH_SKINNING_MAX_MATRIX==4 only.
			nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
			for(i= 0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
				f.serialCont(InfluencedVertices[i]);
		}
	};


	/** A mesh information. NB: private. unlike CMesh::CMeshBuild, do not herit from CMeshBase::CMeshBuild, because
	 * computed internally with CMRMBuilder, and only geometry is of interest.
	 */
	struct	CMeshBuildMRM
	{
		// This tells if the mesh is correctly skinned.
		bool								Skinned;

		// This is the array of SkinWeights, same size as the VB.
		std::vector<CMesh::CSkinWeight>		SkinWeights;

		// This VB is computed with CMRMBuilder and is ready to used
		CVertexBuffer			VBuffer;

		// Lod array, computed with CMRMBuilder and ready to used
		std::vector<CLod>		Lods;

	};
	//@}


private:

	/// Skinning: This tells if the mesh is correctly skinned (suuport skinning).
	bool						_Skinned;
	/// Skinning: this is the list of vertices (mirror of VBuffer), at the bind Pos.
	std::vector<CVector>		_OriginalSkinVertices;
	std::vector<CVector>		_OriginalSkinNormals;
	/// The only one VBuffer of the mesh.
	CVertexBuffer				_VBuffer;
	/// This is the array of SkinWeights, same size as the VB.
	std::vector<CMesh::CSkinWeight>		_SkinWeights;
	/// List of Lods.
	std::vector<CLod>			_Lods;
	/// For clipping. this is the BB of all vertices of all Lods.
	NLMISC::CAABBoxExt			_BBox;



	/// Apply the geomorph to the _VBuffer.
	void	applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod);

	/// Skinning: bkup Vertex/Normal into _OriginalSkin* from VBuffer.
	void	bkupOriginalSkinVertices();
	/// Skinning: restore Vertex/Normal from _OriginalSkin* to VBuffer.
	void	restoreOriginalSkinVertices();

	/// Skinning: Apply skinning to the _VBuffer (before geomorph).
	void	applySkin(CLod &lod, const std::vector<CBone> &bones);
	/// Skinning: same as restoreOriginalSkinVertices(), but for one Lod only.
	void	restoreOriginalSkinPart(CLod &lod);

};


} // NL3D


#endif // NL_MESH_MRM_H

/* End of mesh_mrm.h */
