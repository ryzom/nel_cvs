/** \file mesh.h
 * <File description>
 *
 * $Id: mesh.h,v 1.17 2002/02/26 14:17:55 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_MESH_H
#define NL_MESH_H

#include "nel/misc/types_nl.h"
#include "3d/shape.h"
#include "3d/driver.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/uv.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/animated_material.h"
#include "3d/mesh_base.h"
#include "3d/mesh_geom.h"
#include "3d/mesh_morpher.h"
#include "3d/mesh_vertex_program.h"
#include <set>
#include <vector>


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CPlane;
using	NLMISC::CMatrix;


class CMeshGeom;


// ***************************************************************************
// Should be 4.
#define		NL3D_MESH_SKINNING_MAX_MATRIX	4


// ***************************************************************************
/**
 * An instanciable mesh.
 * Skinning support: support only palette skinning.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMesh : public CMeshBase
{
public:

	/// \name Structures for building a mesh.
	//@{

	/// A corner of a face.
	struct	CCorner
	{
		sint32		Vertex;		/// The vertex Id.
		CVector		Normal;
		NLMISC::CUV	Uvs[CVertexBuffer::MaxStage];
		CRGBA		Color;
		CRGBA		Specular;

		// Setup all to 0, but Color (to white)... Important for good corner comparison.
		// This is slow but doesn't matter since used at mesh building....
		CCorner();

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};

	/// A Triangle face.
	struct	CFace
	{
		CCorner		Corner[3];
		sint32		MaterialId;

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};


	/** Skinning: A skin weight for a vertex.
	 * NB: if you don't use all matrix for this vertex, use at least the 0th matrix, and simply set 0 on Weights you don't use.
	 */
	struct	CSkinWeight
	{
		/// What matrix of the skeleton shape this vertex use.
		uint32			MatrixId[NL3D_MESH_SKINNING_MAX_MATRIX];
		/// weight of this matrix (sum of 4 must be 1).
		float			Weights[NL3D_MESH_SKINNING_MAX_MATRIX];

		/// ctor.
		CSkinWeight()
		{
			for(uint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
			{
				MatrixId[i]=0;
				Weights[i]=0;
			}
		}

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};

	struct CVertLink
	{
		uint32 nFace, nCorner;
		uint32 VertVB;

		CVertLink (uint32 face, uint32 corner, uint32 iVB)
		{
			nFace = face;
			nCorner = corner;
			VertVB = iVB;
		}
	};

	/// A mesh information.
	struct	CMeshBuild
	{
		/** the IDRV_VF* flags which tells what vertices data are used. See IDriver::setVertexFormat() for 
		 * more information. NB: IDRV_VF_XYZ is always considered to true.
		 */
		sint32						VertexFlags;

		// Vertices array
		std::vector<CVector>		Vertices;

		// Palette Skinning Vertices array (same size as Vertices). NULL if no skinning.
		std::vector<CSkinWeight>	SkinWeights;

		// Faces array
		std::vector<CFace>			Faces;

		// Blend shapes if some
		std::vector<CBlendShape>	BlendShapes;

		// Link between VB and max vertex indices
		std::vector<CVertLink>		VertLink; // Filled when called build

		// MeshVertexProgram to copy to meshGeom.
		NLMISC::CSmartPtr<IMeshVertexProgram>	MeshVertexProgram;

		// Serialization
		//void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	};
	//@}


public:
	/// Constructor
	CMesh();
	/// dtor
	~CMesh();
	CMesh(const CMesh &mesh);
	CMesh	&operator=(const CMesh &mesh);


	/// Build a mesh, replacing old. WARNING: This has a side effect of deleting AnimatedMaterials.
	void			build(CMeshBase::CMeshBaseBuild &mbase, CMeshBuild &mbuild);


	/// Build a mesh from material info, and a builded MeshGeom. WARNING: This has a side effect of deleting AnimatedMaterials.
	void			build(CMeshBase::CMeshBaseBuild &mbuild, CMeshGeom &meshGeom);

	void			setBlendShapes(std::vector<CBlendShape>&bs);

	/// \name From IShape
	// @{

	/// Create a CMeshInstance, which contains materials.
	virtual	CTransformShape		*createInstance(CScene &scene);

	/// clip this mesh in a driver.
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans, bool opaquePass);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMesh);

	/// get trinagle count.
	virtual float	getNumTriangles (float distance);

	/// Get bbox.
	virtual void	getAABBox(NLMISC::CAABBox &bbox) const {bbox= getBoundingBox().getAABBox();}

	// @}

	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const;

	/// get the vertex buffer used by the mesh
	const CVertexBuffer &getVertexBuffer() const;

	/// get the number of matrix block
	uint getNbMatrixBlock() const;

	/** get the number of rendering pass for a given matrix block
	 *  \param matrixBlockIndex the index of the matrix block the rendering passes belong to
	 */
	uint getNbRdrPass(uint matrixBlockIndex) const;

	/** get the primitive block associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	const CPrimitiveBlock &getRdrPassPrimitiveBlock(uint matrixBlockIndex, uint renderingPassIndex) const;

	/** get the material ID associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint matrixBlockIndex, uint renderingPassIndex) const;

	/// Get the geom mesh
	const CMeshGeom &getMeshGeom () const;

	// @}

private:

	// The geometry.
	CMeshGeom		*_MeshGeom;
};




// ***************************************************************************
/**
 * A mesh geometry.
 * Skinning support: support only palette skinning.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMeshGeom: public IMeshGeom
{
public:

	/// Constructor
	CMeshGeom();
	virtual ~CMeshGeom();

	/// Build a meshGeom
	void			build(CMesh::CMeshBuild &mbuild, uint numMaxMaterial);

	void			setBlendShapes(std::vector<CBlendShape>&bs);

	/// \name From IMeshGeom
	// @{

	/// Init instance info.
	virtual	void	initInstance(CMeshBaseInstance *mbi);

	/// clip this mesh
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) ;

	/// render() this mesh in a driver.
	virtual void	render(IDriver *drv, CTransformShape *trans, bool passOpaque, float polygonCount, float globalAlpha=1);

	// get an approximation of the number of triangles this instance will render for a fixed distance.
	virtual float	getNumTriangles (float distance);

	/// serial this mesh.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshGeom);

	// @}


	/// \name Geometry accessors
	// @{

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const
	{
		return _BBox;
	}

	/// get the vertex buffer used by the mesh
	const CVertexBuffer &getVertexBuffer() const { return _VBuffer ; }

	/// get the number of matrix block
	uint getNbMatrixBlock() const { return _MatrixBlocks.size() ; }

	/** get the number of rendering pass for a given matrix block
	 *  \param matrixBlockIndex the index of the matrix block the rendering passes belong to
	 */
	uint getNbRdrPass(uint matrixBlockIndex) const { return _MatrixBlocks[matrixBlockIndex].RdrPass.size() ; }

	/** get the primitive block associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	const CPrimitiveBlock &getRdrPassPrimitiveBlock(uint matrixBlockIndex, uint renderingPassIndex) const
	{
		return _MatrixBlocks[matrixBlockIndex].RdrPass[renderingPassIndex].PBlock ;
	}

	/** get the material ID associated with a rendering pass of a matrix block
	 *  \param matrixBlockIndex the index of the matrix block the renderin pass belong to
	 *  \param renderingPassIndex the index of the rendering pass in the matrix block
	 */
	uint32 getRdrPassMaterial(uint matrixBlockIndex, uint renderingPassIndex) const
	{
		return _MatrixBlocks[matrixBlockIndex].RdrPass[renderingPassIndex].MaterialId ;
	}

	// @}


// ************************
private:

	/// A block of primitives, sorted by material used.
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


	/// A block of RdrPasses, sorted by matrix use.
	class	CMatrixBlock
	{
	public:
		/// Which matrix we use for this block.
		uint32					MatrixId[IDriver::MaxModelMatrix];
		/// Number of matrix actually used.
		uint32					NumMatrix;
		/// List of rdr pass, for this matrix block.
		std::vector<CRdrPass>	RdrPass;

		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(0);

			// Code written for IDriver::MaxModelMatrix==16 matrixs.
			nlassert(IDriver::MaxModelMatrix == 16);
			for(uint i=0;i<IDriver::MaxModelMatrix;i++)
				f.serial(MatrixId[i]);
			f.serial(NumMatrix);
			f.serialCont(RdrPass);
		}


		/// return the idx of this bone, in MatrixId. -1 if not found.
		sint	getMatrixIdLocation(uint32 boneId) const;
	};


private:
	/**  Just for build process.
	 * NB: we must store palette info by corner (not by vertex) because Matrix Block grouping may insert vertex
	 * discontinuities. eg: a vertex use Matrix18. After Matrix grouping (16matrix max), Matrix18 could be Matrix2 for a group
	 * of face, but Matrix13 for an other!!
	 */
	struct	CCornerTmp : public CMesh::CCorner
	{
		CPaletteSkin	Palette;
		float			Weights[NL3D_MESH_SKINNING_MAX_MATRIX];

		// The comparison.
		bool		operator<(const CCornerTmp &c) const;
		// The result of the compression.
		mutable sint	VBId;
		// The flags to know what to compare.
		static	sint	Flags;

		// Setup all to 0, but Color (to white)... Important for good corner comparison.
		// This is slow but doesn't matter since used at mesh building....
		CCornerTmp()
		{
			VBId= 0;
			for(sint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
			{
				Palette.MatrixId[i]=0;
				Weights[i]=0;
			}
		}

		// copy from corner.
		CCornerTmp &operator=(const CMesh::CCorner &o)
		{
			Vertex= o.Vertex;
			Normal= o.Normal;
			for(sint i=0;i<=CVertexBuffer::MaxStage;i++)
				Uvs[i]= o.Uvs[i];
			Color= o.Color;
			Specular= o.Specular;

			return *this;
		}

	};


	/** Just for build process. A Bone.
	 */
	struct	CBoneTmp
	{
		// How many ref points on it? (NB: a corner may have multiple (up to 4) on it).
		uint	RefCount;
		// Am i inserted into the current matrixblock?
		bool	Inserted;
		// If I am inserted into the current matrixblock, to which local bone (0-15) I am linked?
		uint	MatrixIdInMB;

		CBoneTmp()
		{
			RefCount= 0;
			Inserted=false;
		}
	};


	/** Just for build process. A map of Bone.
	 */
	typedef	std::map<uint, CBoneTmp>	TBoneMap;
	typedef	TBoneMap::iterator			ItBoneMap;


	/** Just for build process. A Triangle face.
	 */
	struct	CFaceTmp
	{
		CCornerTmp		Corner[3];
		uint			MaterialId;
		// which matrixblock own this face. -1 <=> Not owned.
		sint			MatrixBlockId;

		CFaceTmp()
		{
			MatrixBlockId= -1;
		}
		CFaceTmp	&operator=(const CMesh::CFace& o)
		{
			Corner[0]= o.Corner[0];
			Corner[1]= o.Corner[1];
			Corner[2]= o.Corner[2];
			MaterialId= o.MaterialId;

			return *this;
		}


		void	buildBoneUse(std::vector<uint>	&boneUse, std::vector<CMesh::CSkinWeight> &skinWeights);

	};


	/** Just for build process. A MatrixBlock remap.
	 */
	class	CMatrixBlockRemap
	{
	public:
		uint32					Remap[IDriver::MaxModelMatrix];
	};


private:
	/// VBuffer of the mesh (potentially modified by the mesh morpher)
	CVertexBuffer				_VBuffer;
	/// The original VBuffer of the mesh used only if there are blend shapes.
	CVertexBuffer				_VBufferOri;
	/// The matrix blocks.
	std::vector<CMatrixBlock>	_MatrixBlocks;
	/// For clipping.
	NLMISC::CAABBoxExt			_BBox;
	/// This tells if the mesh is correctly skinned.
	bool						_Skinned;


	/// \name VBufferHard mgt.
	// @{
	/// The only one VBufferHard of the mesh. NULL by default. 
	CRefPtr<IVertexBufferHard>		_VertexBufferHard;
	/// This tells if the VBuffer has changed since the last time or not.
	bool							_VertexBufferHardDirty;
	/// This is the driver used to setup the vbuffer hard. error if a mesh has not the same driver in his life.
	CRefPtr<IDriver>				_Driver;

	/// update the VertexBufferHard if NULL (ie not created or deleted by driver) or if VertexBufferDirty.
	void							updateVertexBufferHard(IDriver *drv);
	// @}

	// The Mesh Morpher
	CMeshMorpher	*_MeshMorpher; 


	// Possible MeshVertexProgram to apply at render()
	NLMISC::CSmartPtr<IMeshVertexProgram>	_MeshVertexProgram;


private:
	// Locals, for build.
	class	CCornerPred
	{
	public:
		bool operator()(const CCornerTmp *x, const CCornerTmp *y) const
		{
			return (*x<*y);
		}
	};
	typedef		std::set<CCornerTmp*, CCornerPred>	TCornerSet;
	typedef		TCornerSet::iterator ItCornerSet;

	// Find and fill the VBuffer.
	void	findVBId(TCornerSet  &corners, const CCornerTmp *corn, sint &currentVBIndex, const CVector &vert)
	{
		ItCornerSet  it= corners.find(const_cast<CCornerTmp *>(corn));
		if(it!=corners.end())
			corn->VBId= (*it)->VBId;
		else
		{
			// Add corner to the set to not insert same corner two times.
			corners.insert (const_cast<CCornerTmp *>(corn));
			sint	i;
			corn->VBId= currentVBIndex++;
			// Fill the VBuffer.
			_VBuffer.setNumVertices(currentVBIndex);
			sint	id= currentVBIndex-1;
			// XYZ.
			_VBuffer.setVertexCoord(id, vert);
			// Normal
			if(CCornerTmp::Flags & CVertexBuffer::NormalFlag)
				_VBuffer.setNormalCoord(id, corn->Normal);
			// Uvs.
			for(i=0;i<CVertexBuffer::MaxStage;i++)
			{
				if(CCornerTmp::Flags & (CVertexBuffer::TexCoord0Flag<<i))
					_VBuffer.setTexCoord(id, i, corn->Uvs[i].U, corn->Uvs[i].V);
			}
			// Color.
			if(CCornerTmp::Flags & CVertexBuffer::PrimaryColorFlag)
				_VBuffer.setColor(id, corn->Color);
			// Specular.
			if(CCornerTmp::Flags & CVertexBuffer::SecondaryColorFlag)
				_VBuffer.setSpecular(id, corn->Specular);

			// setup palette skinning.
			if ((CCornerTmp::Flags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag)
			{
				_VBuffer.setPaletteSkin(id, corn->Palette);
				for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
					_VBuffer.setWeight(id, i, corn->Weights[i]);
			}
		}
	}

	// build skinning.
	void	buildSkin(CMesh::CMeshBuild &m, std::vector<CFaceTmp>	&tmpFaces);

	// optimize triangles order of all render pass.
	void	optimizeTriangleOrder();

};




} // NL3D


#endif // NL_MESH_H
            
/* End of mesh.h */
