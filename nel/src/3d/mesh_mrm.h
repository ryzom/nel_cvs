/** \file mesh_mrm.h
 * <File description>
 *
 * $Id: mesh_mrm.h,v 1.32 2002/07/02 12:27:19 berenguier Exp $
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
// Fast matrix in mesh_mrm_skin.cpp
class	CMatrix3x4;
class	CMatrix3x4SSE;


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
	~CMeshMRMGeom();

	/** Build a mesh, replacing old.
	 * this is much slower than CMeshGeom::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build(CMesh::CMeshBuild &m, std::vector<CMesh::CMeshBuild*> &bsList, uint numMaxMaterial, const CMRMParameters &params= CMRMParameters());

	/** Change MRM Distance setup.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void			changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);


	/// \name From IMeshGeom
	// @{

	/// Init instance info.
	virtual	void	initInstance(CMeshBaseInstance *mbi);

	/// clip this mesh in a driver. true if visible.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver, given an instance and his materials.
	virtual void	render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha);

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

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
	const CVertexBuffer &getVertexBuffer() const { return _VBufferFinal ; }


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


	/// \name Skinning Behavior
	// @{

	/// Return true if the mesh is skinned, else return false.
	bool isSkinned () const
	{
		return _Skinned;
	}

	/// Compute skinning id
	void			computeBonesId (CSkeletonModel *skeleton);

	/// update Skeleton Usage. increment or decrement. computeBonesId must has been called before.
	void			updateSkeletonUsage(CSkeletonModel *sm, bool increment);

	/// return array of bones used by the skin. computeBonesId must has been called before.
	const std::vector<sint32>	&getSkinBoneUsage() const {return _BonesId;}

	// @}


	/// \name Mesh Block Render Implementation
	// @{

	/** true if this meshGeom support meshBlock rendering.
	 *	return false if skinned/meshMorphed.
	 */
	virtual bool	supportMeshBlockRendering () const;

	virtual bool	sortPerMaterial() const;
	virtual uint	getNumRdrPasses() const ;
	virtual	void	beginMesh(CMeshGeomRenderContext &rdrCtx) ;
	virtual	void	activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount) ;
	virtual	void	renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, uint rdrPass) ;
	virtual	void	endMesh(CMeshGeomRenderContext &rdrCtx) ;

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


	/// A block of vertices descriptor.
	struct	CVertexBlock
	{
		// The index of the start vertex.
		uint32	VertexStart;
		// Number of vertices.
		uint32	NVertices;

		void	serial(NLMISC::IStream &f)
		{
			f.serial(VertexStart, NVertices);
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
		/** Skinning: list of vertex blocks to copy from RAM to AGP, for this Lod only.
		 *	NB: it is constructed from InfluencedVertices. Only usefull if skinned.
		 */
		std::vector<CVertexBlock>		SkinVertexBlocks;



		CLod()
		{
			// By default, this is supposed false.
			OriginalSkinRestored= false;
		}

		// Serialize a Lod.
		void		serial(NLMISC::IStream &f);

		// Used in CMeshMRMGeom::build().
		void		buildSkinVertexBlocks();
		void		optimizeTriangleOrder();
	};

	friend class	CLod;

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

		// The blend shapes
		std::vector<CBlendShape>	BlendShapes;

		/// \Degradation control.
		// @{
		/// The MRM has its max faces when dist<=DistanceFinest.
		float					DistanceFinest;
		/// The MRM has 50% of its faces at dist==DistanceMiddle.
		float					DistanceMiddle;
		/// The MRM has faces/Divisor when dist>=DistanceCoarsest.
		float					DistanceCoarsest;
		// @}

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
	std::vector<CVector>		_OriginalTGSpace;

	/// The Original VBuffer
	CVertexBuffer				_VBufferOriginal;
	/// The Final VBuffer
	CVertexBuffer				_VBufferFinal;
	
	/// This is the array of SkinWeights, same size as the VB.
	std::vector<CMesh::CSkinWeight>		_SkinWeights;

	/// This boolean is true if the bones id have been passed in the skeleton
	bool						_BoneIdComputed;
	/// true if the _BonesIdExt have been computed (for bone Usage).
	bool						_BoneIdExtended;

	/// This array give the name of the local bones
	std::vector<std::string>	_BonesName;
	/// This array give the index in the skeleton of the local bones used. computed at first computeBoneId()
	std::vector<sint32>			_BonesId;
	/// Same as _BonesId but with parent of bones added. (used for bone usage)
	std::vector<sint32>			_BonesIdExt;

	/// List of Lods.
	std::vector<CLod>			_Lods;
	/// For clipping. this is the BB of all vertices of all Lods.
	NLMISC::CAABBoxExt			_BBox;
	/// For Load balancing, the min number of faces this MRM use.
	uint32						_MinFaceUsed;
	/// For Load balancing, the max number of faces this MRM use.
	uint32						_MaxFaceUsed;


	/// Info for pre-loading Lods.
	std::vector<CLodInfo>		_LodInfos;
	uint						_NbLodLoaded;


	/// \Degradation control.
	// @{
	/// The MRM has its max faces when dist<=DistanceFinest. nlassert if <0.
	float						_DistanceFinest;
	/// The MRM has 50% of its faces at dist==DistanceMiddle. nlassert if <= DistanceFinest.
	float						_DistanceMiddle;
	/// The MRM has faces/Divisor when dist>=DistanceCoarsest. nlassert if <= DistanceMiddle.
	float						_DistanceCoarsest;

	float						_OODistanceDelta;
	float						_DistancePow;

	/// return a float [0,1], computed from a distance (should be >0).
	float						getLevelDetailFromDist(float dist);
	// @}


	/// \name Hard VB
	// @{

	CRefPtr<IVertexBufferHard>	_VBHard;
	// a refPtr on the driver, to delete VBuffer Hard at clear().
	CRefPtr<IDriver>			_Driver;
	bool						_VertexBufferHardDirty;

	/// NB: HERE FOR PACKING ONLY. For clipping. Estimate if we must do a Precise clipping (ie with bboxes)
	bool						_PreciseClipping;

	/* try to create a vertexBufferHard. NB: enlarge capacity of the VBHard as necessary.
		After this call, the vertexBufferHard may be NULL.
	*/
	void				updateVertexBufferHard(IDriver *drv, uint32 numVertices);
	void				deleteVertexBufferHard();

	// Fill skin in AGP, if VBhard exist/used
	void				fillAGPSkinPart(CLod &lod, IVertexBufferHard *currentVBHard);
	// @}

	// The Mesh Morpher
	CMeshMorpher				_MeshMorpher; 

	// Possible MeshVertexProgram to apply at render()
	NLMISC::CSmartPtr<IMeshVertexProgram>	_MeshVertexProgram;

private:
	class	CRawSkinNormalLod;
	class	CRawSkinTgSpaceLod;

	/// serial a subset of the vertices.
	void	serialLodVertexData(NLMISC::IStream &f, uint startWedge, uint endWedge);


	/// Apply the geomorph to the _VBuffer, or the VBhard, if exist/used
	void	applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod, IVertexBufferHard *currentVBHard);

	/// Skinning: bkup Vertex/Normal into _OriginalSkin* from VBuffer.
	void	bkupOriginalSkinVertices();
	void	bkupOriginalSkinVerticesSubset(uint wedgeStart, uint wedgeEnd);
	/// Skinning: restore Vertex/Normal from _OriginalSkin* to VBuffer.
	void	restoreOriginalSkinVertices();

	/// Skinning: Apply skinning to the _VBuffer (before geomorph).	  	  
	void	applySkin(CLod &lod, const CSkeletonModel *skeleton);

	/** The same as apply skin, but with normal modified. Normal is not normalized.
	  *	4 versions from slower to faster.
	  */
	void	applySkinWithNormal(CLod &lod, const CSkeletonModel *skeleton);
	void	applySkinWithNormalSSE(CLod &lod, const CSkeletonModel *skeleton);
	void	applyRawSkinWithNormal(CLod &lod, CRawSkinNormalLod &rawSkinLod, const CSkeletonModel *skeleton);
	void	applyRawSkinWithNormalSSE(CLod &lod, CRawSkinNormalLod &rawSkinLod, const CSkeletonModel *skeleton);

	/** The same as apply skin with normal, but with a tangent space added (encoded in a texture coordinate).
	  * The tangent space is modified, but not normalized (must be done in a vertex program).
	  *	4 versions from slower to faster.
	  */
	void	applySkinWithTangentSpace(CLod &lod, const CSkeletonModel *skeleton, uint tangentSpaceTexCoord);
	void	applySkinWithTangentSpaceSSE(CLod &lod, const CSkeletonModel *skeleton, uint tangentSpaceTexCoord);
	void	applyRawSkinWithTangentSpace(CLod &lod, CRawSkinTgSpaceLod &rawSkinLod, 
		const CSkeletonModel *skeleton, uint tangentSpaceTexCoord);
	void	applyRawSkinWithTangentSpaceSSE(CLod &lod, CRawSkinTgSpaceLod &rawSkinLod, 
		const CSkeletonModel *skeleton, uint tangentSpaceTexCoord);

	/// Skinning: same as restoreOriginalSkinVertices(), but for one Lod only.
	void	restoreOriginalSkinPart(CLod &lod, IVertexBufferHard *currentVBHard);


	/// load the header of this mesh. return the version of the header.
	sint		loadHeader(NLMISC::IStream &f) throw(NLMISC::EStream);
	/// load this mesh.
	void		load(NLMISC::IStream &f) throw(NLMISC::EStream);
	/// save the entire mesh.
	void		save(NLMISC::IStream &f) throw(NLMISC::EStream);

	// Build bone Usage information for serialized mesh <= version 2.
	void		buildBoneUsageVer2 ();

	/// compile precalc for distanceSetup
	void		compileDistanceSetup();

	// Some runtime not serialized compilation
	void		compileRunTime();

private:

	/// \name RawSkin optimisation.
	// @{


	// Skin with Normal version.
	//==============

	/// Vertices influenced by 1 matrix only.
	class	CRawVertexNormalSkin1
	{
	public:
		// The id of the matrix to use.
		uint32		MatrixId[1];
		CVector		Vertex;
		CVector		Normal;
		// The Dest VertexId
		uint32		VertexId;
	};

	/// Vertices influenced by 2 matrix only.
	class	CRawVertexNormalSkin2
	{
	public:
		// The id of the matrix to use.
		uint32		MatrixId[2];
		float		Weights[2];
		CVector		Vertex;
		CVector		Normal;
		// The Dest VertexId
		uint32		VertexId;
	};

	/// Vertices influenced by 3 or 4 matrix only. (simpler and rare)
	class	CRawVertexNormalSkin4
	{
	public:
		CMesh::CSkinWeight	SkinWeight;
		CVector		Vertex;
		CVector		Normal;
		// The Dest VertexId
		uint32		VertexId;
	};

	/// The array per lod.
	class	CRawSkinNormalLod
	{
	public:
		// The vertices influenced by 1 matrix.
		std::vector<CRawVertexNormalSkin1>	Vertices1;
		// The vertices influenced by 2 matrix.
		std::vector<CRawVertexNormalSkin2>	Vertices2;
		// The vertices influenced by 3 matrix.
		std::vector<CRawVertexNormalSkin4>	Vertices3;
		// The vertices influenced by 4 matrix.
		std::vector<CRawVertexNormalSkin4>	Vertices4;
	};

	/// List of RawLods
	std::vector<CRawSkinNormalLod>		_RawSkinNormalLods;


	// Skin with TgSpace version.
	//==============

	/// Vertices influenced by 1 matrix only.
	class	CRawVertexTgSpaceSkin1
	{
	public:
		// The id of the matrix to use.
		uint32		MatrixId[1];
		CVector		Vertex;
		CVector		Normal;
		CVector		TgSpace;
		// The Dest VertexId
		uint32		VertexId;
	};

	/// Vertices influenced by 2 matrix only.
	class	CRawVertexTgSpaceSkin2
	{
	public:
		// The id of the matrix to use.
		uint32		MatrixId[2];
		float		Weights[2];
		CVector		Vertex;
		CVector		Normal;
		CVector		TgSpace;
		// The Dest VertexId
		uint32		VertexId;
	};

	/// Vertices influenced by 3 or 4 matrix only. (simpler and rare)
	class	CRawVertexTgSpaceSkin4
	{
	public:
		CMesh::CSkinWeight	SkinWeight;
		CVector		Vertex;
		CVector		Normal;
		CVector		TgSpace;
		// The Dest VertexId
		uint32		VertexId;
	};

	/// The array per lod.
	class	CRawSkinTgSpaceLod
	{
	public:
		// The vertices influenced by 1 matrix.
		std::vector<CRawVertexTgSpaceSkin1>	Vertices1;
		// The vertices influenced by 2 matrix.
		std::vector<CRawVertexTgSpaceSkin2>	Vertices2;
		// The vertices influenced by 3 matrix.
		std::vector<CRawVertexTgSpaceSkin4>	Vertices3;
		// The vertices influenced by 4 matrix.
		std::vector<CRawVertexTgSpaceSkin4>	Vertices4;
	};

	/// List of RawLods
	std::vector<CRawSkinTgSpaceLod>		_RawSkinTgSpaceLods;


	// Misc RawSkin
	//===========

	/// 0 if RawSkin disabled. Else the number of Lods which have been already computed for RawSkin.
	uint		_NumLodRawSkin;

	/// compute RawSkin info according to current skin setup. Compute new lod loaded if needed.
	void		updateRawSkinNormal();
	/// same as updateRawSkinNormal(), but TgSpace version
	void		updateRawSkinTgSpace();
	/// clear all RawSkin if not already done
	void		clearRawSkin();

	// ApplySkin method
	void		applyArrayRawSkinNormal1(CRawVertexNormalSkin1 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinNormal2(CRawVertexNormalSkin2 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinNormal3(CRawVertexNormalSkin4 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinNormal4(CRawVertexNormalSkin4 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	// Same for SSE
	void		applyArrayRawSkinNormal1(CRawVertexNormalSkin1 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinNormal2(CRawVertexNormalSkin2 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinNormal3(CRawVertexNormalSkin4 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinNormal4(CRawVertexNormalSkin4 *src, uint normalOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);


	// ApplySkin method
	void		applyArrayRawSkinTgSpace1(CRawVertexTgSpaceSkin1 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinTgSpace2(CRawVertexTgSpaceSkin2 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinTgSpace3(CRawVertexTgSpaceSkin4 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinTgSpace4(CRawVertexTgSpaceSkin4 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf);
	// Same for SSE
	void		applyArrayRawSkinTgSpace1(CRawVertexTgSpaceSkin1 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinTgSpace2(CRawVertexTgSpaceSkin2 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinTgSpace3(CRawVertexTgSpaceSkin4 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);
	void		applyArrayRawSkinTgSpace4(CRawVertexTgSpaceSkin4 *src, uint normalOff, uint tgSpaceOff, uint8 *destVertexPtr, 
		CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf);

public:
	static  uint	NumCacheVertexNormal1;
	static  uint	NumCacheVertexNormal2;
	static  uint	NumCacheVertexNormal4;
	static  uint	NumCacheVertexTgSpace1;
	static  uint	NumCacheVertexTgSpace2;
	static  uint	NumCacheVertexTgSpace4;

	// @}
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
	void			build ( CMeshBase::CMeshBaseBuild &mBase, CMesh::CMeshBuild &m,
							std::vector<CMesh::CMeshBuild*> &listBS,
							const CMRMParameters &params= CMRMParameters() );


	/** Build a mesh, replacing old. build from a CMeshBaseBuild (materials info) and a previously builded CMeshMRMGeom.
	 *	WARNING: This has a side effect of deleting AnimatedMaterials.
	 *	this is much slower than CMesh::build(), because it computes the MRM.
	 * \param params parameters of the MRM build process.
	 */
	void			build (CMeshBase::CMeshBaseBuild &m, const CMeshMRMGeom &mgeom);


	/// Compute skinning id
	void			computeBonesId (CSkeletonModel *skeleton);

	/// update Skeleton Usage. increment or decrement.
	void			updateSkeletonUsage(CSkeletonModel *sm, bool increment);

	/** Change MRM Distance setup.
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	void			changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);

	/// \name From IShape
	// @{

	/// Create a CMeshInstance, which contains materials.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans, bool passOpaque);

	/// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshMRM);

	/// Get bbox.
	virtual void	getAABBox(NLMISC::CAABBox &bbox) const {bbox= getBoundingBox().getAABBox();}

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

	/// Get the mesh geom
	const CMeshMRMGeom& getMeshGeom () const;

	// @}


// ************************
private:

	CMeshMRMGeom		_MeshMRMGeom;


};


} // NL3D


#endif // NL_MESH_MRM_H

/* End of mesh_mrm.h */
