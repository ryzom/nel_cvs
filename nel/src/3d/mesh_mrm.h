/** \file mesh_mrm.h
 * <File description>
 *
 * $Id: mesh_mrm.h,v 1.7 2001/06/27 14:01:14 berenguier Exp $
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
#include "3d/mesh_geom.h"
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
 * An MRM mesh geometry, with no materials information.
 *
 * To build a CMeshMRMGeom, you should:
 *	- build a CMesh::CMeshBuild   meshBuild (see CMesh)
 *	- call MeshMRMGeom.build(MeshBuild);
 *
 * NB: internally, build() use CMRMBuilder, a builder of MRM.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CMeshMRMGeom : public IMeshGeom
{
public:
	/// Constructor
	CMeshMRMGeom();

	/** Build a mesh, replacing old.
	 * this is much slower than CMeshGeom::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build(CMesh::CMeshBuild &m, const CMRMParameters &params= CMRMParameters());


	/// \name From IMeshGeom
	// @{

	/// clip this mesh in a driver. true if visible.
	virtual bool	clip(const std::vector<CPlane>	&pyramid);

	/// render() this mesh in a driver, given an instance and his materials.
	virtual void	render(IDriver *drv, CTransformShape *trans);

	/// serial this meshGeom.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshMRMGeom);

	// @}


	/// \name Lod management.
	// @{

	/** get the number of LOD currently loaded.
	 */
	uint			getNbLodLoaded() const { return _NbLodLoaded ; }


	/** Load the header and the first lod of a MRM in a stream.
	 *	\param f the input stream to read. NB: after load, f.getPos() return the position of the second lod in the stream.
	 */
	void			loadFirstLod(NLMISC::IStream &f);


	/** Load next lod of a stream. use getNbLodLoaded() to know what Lod will be loaded.
	 * NB: if getNbLodLoaded() == getNbLod(), no op.
	 *	\param f the same input stream passed to loadFirstLod(). NB: after load, f.getPos() is "unedfined" 
	 *	(actually return the position of the next lod in the stream).
	 */
	void			loadNextLod(NLMISC::IStream &f);


	/** UnLoad Lod getNbLodLoaded()-1 from memory. use getNbLodLoaded()-1 to know what Lod will be unloaded.
	 * NB: if getNbLodLoaded() <= 1, no op.
	 */
	void			unloadNextLod(NLMISC::IStream &f);


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


	/// A LOD of the MRM.
	class	CLodInfo
	{
	public:
		/// The frist new wedge this lod use.
		uint32		StartAddWedge;
		/// The last+1 new wedge this lod use. NB: Lod.NWedges== LodInfo.EndAddWedges.
		uint32		EndAddWedges;
		/// the absolute Lod offset in the last Stream which has been used to read this MRM.
		sint32		LodOffset;

		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(0);

			f.serial(StartAddWedge);
			f.serial(EndAddWedges);
			// do not serial LodOffset here.
		}
	};



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


	/// Info for pre-loading Lods.
	std::vector<CLodInfo>		_LodInfos;
	uint						_NbLodLoaded;


	/// serial a subset of the vertices.
	void	serialLodVertexData(NLMISC::IStream &f, uint startWedge, uint endWedge);


	/// Apply the geomorph to the _VBuffer.
	void	applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod);

	/// Skinning: bkup Vertex/Normal into _OriginalSkin* from VBuffer.
	void	bkupOriginalSkinVertices();
	void	bkupOriginalSkinVerticesSubset(uint wedgeStart, uint wedgeEnd);
	/// Skinning: restore Vertex/Normal from _OriginalSkin* to VBuffer.
	void	restoreOriginalSkinVertices();

	/// Skinning: Apply skinning to the _VBuffer (before geomorph).
	void	applySkin(CLod &lod, const std::vector<CBone> &bones);
	/// Skinning: same as restoreOriginalSkinVertices(), but for one Lod only.
	void	restoreOriginalSkinPart(CLod &lod);



	/// load the header of this mesh.
	void		loadHeader(NLMISC::IStream &f) throw(NLMISC::EStream);
	/// load this mesh.
	void		load(NLMISC::IStream &f) throw(NLMISC::EStream);
	/// save the entire mesh.
	void		save(NLMISC::IStream &f) throw(NLMISC::EStream);

};



// ***************************************************************************
/**
 * An instanciable MRM mesh.
 *
 * To build a CMeshMRM, you should:
 *	- build a CMesh::CMeshBuild   meshBuild (see CMesh)
 *	- call MeshMRM.build(MeshBuild);
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


	/** Build a mesh, replacing old. build from a CMeshBaseBuild (materials info) and a previously builded CMeshMRMGeom.
	 *	WARNING: This has a side effect of deleting AnimatedMaterials.
	 *	this is much slower than CMesh::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build(CMeshBase::CMeshBaseBuild &m, const CMeshMRMGeom &mgeom);


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
		return _MeshMRMGeom.getBoundingBox();
	}

	/// get the vertex buffer used by the mrm mesh. NB: this VB store all Vertices used by All LODs.
	const CVertexBuffer &getVertexBuffer() const { return _MeshMRMGeom.getVertexBuffer(); }


	/** get the number of LOD.
	 */
	uint getNbLod() const { return _MeshMRMGeom.getNbLod()  ; }


	/** get the number of rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 */
	uint getNbRdrPass(uint lodId) const { return _MeshMRMGeom.getNbRdrPass(lodId) ; }


	/** get the primitive block associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *  \param renderingPassIndex the index of the rendering pass
	 */
	const CPrimitiveBlock &getRdrPassPrimitiveBlock(uint lodId, uint renderingPassIndex) const
	{
		return _MeshMRMGeom.getRdrPassPrimitiveBlock(lodId, renderingPassIndex) ;
	}


	/** get the material ID associated with a rendering pass of a LOD.
	 *	\param lodId the id of the LOD.
	 *	\param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint lodId, uint renderingPassIndex) const
	{
		return _MeshMRMGeom.getRdrPassMaterial(lodId, renderingPassIndex) ;
	}

	// @}


// ************************
private:

	CMeshMRMGeom		_MeshMRMGeom;


};


} // NL3D


#endif // NL_MESH_MRM_H

/* End of mesh_mrm.h */
