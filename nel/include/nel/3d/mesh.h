/** \file mesh.h
 * <File description>
 *
 * $Id: mesh.h,v 1.14 2001/04/23 09:14:27 besson Exp $
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
#include "nel/3d/shape.h"
#include "nel/3d/driver.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/uv.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/material.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/animated_material.h"
#include <set>
#include <vector>


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CPlane;
using	NLMISC::CMatrix;


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
class CMesh : public IShape
{
public:

	/// \name Structures for building a mesh.
	//@{

	/// A corner of a face.
	struct	CCorner
	{
		sint		Vertex;		/// The vertex Id.
		CVector		Normal;
		NLMISC::CUV			Uvs[IDRV_VF_MAXSTAGES];
		CRGBA		Color;
		CRGBA		Specular;

		// Setup all to 0, but Color (to white)... Important for good corner comparison.
		// This is slow but doesn't matter since used at mesh building....
		CCorner();
	};

	/// A Triangle face.
	struct	CFace
	{
		CCorner		Corner[3];
		sint		MaterialId;
	};


	/** Skinning: A skin weight for a vertex.
	 * NB: if you don't use all matrix for this vertex, use at least the 0th matrix, and simply set 0 on Weights you don't use.
	 */
	struct	CSkinWeight
	{
		/// What matrix of the skeleton shape this vertex use.
		uint			MatrixId[NL3D_MESH_SKINNING_MAX_MATRIX];
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
	};


	/// A mesh information.
	struct	CMeshBuild
	{
		/** the IDRV_VF* flags which tells what vertices data are used. See IDriver::setVertexFormat() for 
		 * more information. NB: IDRV_VF_XYZ is always considered to true.
		 */
		sint					VertexFlags;

		// Default value for position of this mesh
		CVector					DefaultPos;
		CVector					DefaultPivot;
		CVector					DefaultRotEuler;
		CQuat					DefaultRotQuat;
		CVector					DefaultScale;

		// Material array
		std::vector<CMaterial>	Materials;

		// Vertices array
		std::vector<CVector>	Vertices;

		// Palette Skinning Vertices array (same size as Vertices). NULL if no skinning.
		std::vector<CSkinWeight>	SkinWeights;


		// Faces array
		std::vector<CFace>		Faces;
	};
	//@}


public:
	/// Constructor
	CMesh();

	/// Build a mesh, replacing old. WARNING: This has a side effect of deleting AnimatedMaterials.
	void			build(CMeshBuild &mbuild);


	/// \name animated material mgt. do it after build().
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
	NLMISC_DECLARE_CLASS(CMesh);

	// @}

	/// Get the number of materials in the mesh
	uint getNbMaterial()
	{
		return _Materials.size();
	}

	/// Get the materials number id in the materials list
	const CMaterial& getMaterial(uint id)
	{
		return _Materials[id];
	}

	/// get the extended axis aligned bounding box of the mesh
	const NLMISC::CAABBoxExt& getBoundingBox() const
	{
		return _BBox;
	}


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


	/// The V2- Old CRdrPass structure.
	class	CRdrPassOldV2
	{
	public:
		CMaterial			Material;
		CPrimitiveBlock		PBlock;
		void	serial(NLMISC::IStream &f)
		{
			sint	ver= f.serialVersion(0);
			f.serial(Material);
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
	 * discontinuities. eg: a vertex use Matrix23. After Matrix grouping (16matrix), Matrix23 could be Matrix2 for a group
	 * of face, but Matrix13 for an other!!
	 */
	struct	CCornerTmp : public CCorner
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
		CCornerTmp &operator=(const CCorner &o)
		{
			Vertex= o.Vertex;
			Normal= o.Normal;
			for(sint i=0;i<= IDRV_VF_MAXSTAGES;i++)
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
		CFaceTmp	&operator=(const CFace& o)
		{
			Corner[0]= o.Corner[0];
			Corner[1]= o.Corner[1];
			Corner[2]= o.Corner[2];
			MaterialId= o.MaterialId;

			return *this;
		}


		void	buildBoneUse(std::vector<uint>	&boneUse, std::vector<CSkinWeight> &skinWeights);

	};


	/** Just for build process. A MatrixBlock remap.
	 */
	class	CMatrixBlockRemap
	{
	public:
		uint32					Remap[IDriver::MaxModelMatrix];
	};


private:
	/// The only one VBuffer of the mesh.
	CVertexBuffer				_VBuffer;
	/// The Materials.
	std::vector<CMaterial>		_Materials;
	/// The matrix blocks.
	std::vector<CMatrixBlock>	_MatrixBlocks;
	/// For clipping.
	NLMISC::CAABBoxExt			_BBox;
	/// This tells if the mesh is correctly skinned.
	bool						_Skinned;


	/// Animated Material mgt.
	typedef std::map<uint32, CMaterialBase>	TAnimatedMaterialMap;
	TAnimatedMaterialMap			_AnimatedMaterials;


	/// Transform default tracks. Those default tracks are instancied, ie, CInstanceMesh will have the same and can't specialize it.
	CTrackDefaultVector			_DefaultPos;
	CTrackDefaultVector			_DefaultPivot;
	CTrackDefaultVector			_DefaultRotEuler;
	CTrackDefaultQuat			_DefaultRotQuat;
	CTrackDefaultVector			_DefaultScale;

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
			if(CCornerTmp::Flags & IDRV_VF_NORMAL)
				_VBuffer.setNormalCoord(id, corn->Normal);
			// Uvs.
			for(i=0;i<IDRV_VF_MAXSTAGES;i++)
			{
				if(CCornerTmp::Flags & IDRV_VF_UV[i])
					_VBuffer.setTexCoord(id, i, corn->Uvs[i].U, corn->Uvs[i].V);
			}
			// Color.
			if(CCornerTmp::Flags & IDRV_VF_COLOR)
				_VBuffer.setColor(id, corn->Color);
			// Specular.
			if(CCornerTmp::Flags & IDRV_VF_SPECULAR)
				_VBuffer.setSpecular(id, corn->Specular);

			// setup palette skinning.
			if( (CCornerTmp::Flags & IDRV_VF_PALETTE_SKIN) == IDRV_VF_PALETTE_SKIN)
			{
				_VBuffer.setPaletteSkin(id, corn->Palette);
				for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
					_VBuffer.setWeight(id, i, corn->Weights[i]);
			}
		}
	}

	// build skinning.
	void	buildSkin(CMeshBuild &m, std::vector<CFaceTmp>	&tmpFaces);

};

} // NL3D


#endif // NL_MESH_H

/* End of mesh.h */
