/** \file lod_character_shape.h
 * <File description>
 *
 * $Id: lod_character_shape.h,v 1.1 2002/05/07 08:15:58 berenguier Exp $
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

#ifndef NL_LOD_CHARACTER_SHAPE_H
#define NL_LOD_CHARACTER_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "3d/mesh.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * A build structure information for building a CLodCharacterShape
 *	This is the structure exported from the 3D editor
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterShapeBuild
{
public:
	// The Vertices of the shapes
	std::vector<CVector>			Vertices;

	// Palette Skinning Vertices array (same size as Vertices).
	std::vector<CMesh::CSkinWeight>	SkinWeights;

	// Bones name. Each matrix id used in SkinWeights must have a corresponding string in the bone name array.
	std::vector<std::string>		BonesNames;

	// Faces array
	std::vector<uint32>				TriangleIndices;


	void	serial(NLMISC::IStream &f);

};


// ***************************************************************************
/**
 * A very Small Shape with anims encoded as Key Meshes. Used for Lod of skinned meshes
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CLodCharacterShape
{
public:

	/// A lod Animation Build information
	struct	CAnimBuild
	{
		/// Name of the Anim.
		std::string		Name;
		/// The number of Keys.
		uint			NumKeys;
		/// The effective time of the animation
		float			AnimLength;
		/// Anim Loop?
		bool			AnimLoop;

		/// List of Keys * NumVertices
		std::vector<CVector>	Keys;
	};

	/// A compressed vector information
	struct	CVector3s
	{
		sint16	x,y,z;

		void		serial(NLMISC::IStream &f)
		{
			f.serial(x,y,z);
		}
	};

public:

	/// Constructor
	CLodCharacterShape();

	/** build the Mesh base information
	 *	NB: SkinWeights array tells for each vertex what bone color to use.
	 */
	void			buildMesh(const std::string &name, const CLodCharacterShapeBuild &lodBuild);

	/** Add an animation. many nlAssert to verify array size etc... return false, if same AnimName exist.
	 *	NB: the entire animation is compressed to CVector3s internally.
	 */
	bool			addAnim(const CAnimBuild &animBuild);

	/// serial this shape
	void			serial(NLMISC::IStream &f);

	/// Get name of this lod
	const std::string &		getName() const {return _Name;}

	/// get the animId from a name. -1 if not found
	sint			getAnimIdByName(const std::string &name) const;

	/// get the number of vertices of this mesh
	uint			getNumVertices() const {return _NumVertices;}

	/// get the number of triangles of this mesh
	uint			getNumTriangles() const {return _NumTriangles;}

	/// get the number of bones
	uint			getNumBones() const {return _Bones.size();}

	/// get a bone id, according to its name. -1 if not found
	sint			getBoneIdByName(const std::string &name) const;

	/// get a ptr to the triangles indices
	const uint32	*getTriangleArray() const;

	/// \name Vertex per Bone coloring
	// @{

	/// init the process by resize-ing a tmp RGBAF vector of getNumVertices() size, and reset to full black
	void			startBoneColor(std::vector<NLMISC::CRGBAF>	&tmpColors) const;

	/// Add a bone color influence to tmpColors. 
	void			addBoneColor(uint boneId, CRGBA	color, std::vector<NLMISC::CRGBAF> &tmpColors);

	/** Compile boneColor result into a CRGBA vector (resize-ed by the method), averaging with weight stored in A.
	 *	NB: if a final vertex is not influenced by any BoneColor, then it will receive (128,128,128,0).
	 *	Hence it will be somewhat transparent (AlphaTest is used to render lod character shapes). 
	 *	This may be interressant to hide some parts if they are not used.
	 */
	void			endBoneColor(const std::vector<NLMISC::CRGBAF> &tmpColors, std::vector<NLMISC::CRGBA>	&dstColors);

	// @}

	/** get a ptr to the vertices of the key according to animId and time.
	 *	Key is selected according to the AnimLoop state.
	 *	\param unPackScaleFactor return value is the scale factor which to multiply
	 *	\return NULL if animId is not valid
	 */
	const CVector3s	*getAnimKey(uint animId, float time, CVector &unPackScaleFactor) const;


// *******************************
private:

	/// A lod Animation
	struct	CAnim
	{
		/// Name of the Anim.
		std::string		Name;
		/// The number of Keys.
		uint32			NumKeys;
		/// The effective time of the animation
		float			AnimLength;
		float			OOAnimLength;
		/// Anim Loop?
		bool			AnimLoop;
		/// The Scale factor to be multiplied to transform CVector3s to CVector
		CVector			UnPackScaleFactor;

		/// List of Keys * NumVertices
		std::vector<CVector3s>	Keys;

		void			serial(NLMISC::IStream &f);
	};

	struct	CVertexInf
	{
		// Id of the vertex that the bone influence.
		uint32		VertexId;
		// weight of influence
		float		Influence;

		void		serial(NLMISC::IStream &f)
		{
			f.serial(VertexId);
			f.serial(Influence);
		}
	};

	/// A Bone influence: list of all vertices to influence.
	struct	CBoneInfluence
	{
		// Name of the bone.
		std::string				Name;

		// list of vertex this bone influence.
		std::vector<CVertexInf>	InfVertices;

		void		serial(NLMISC::IStream &f);
	};

	/// Map name To Id.
	typedef	std::map<std::string, uint32>	TStrIdMap;
	typedef	TStrIdMap::iterator				ItStrIdMap;
	typedef	TStrIdMap::const_iterator		CstItStrIdMap;

private:
	std::string			_Name;
	uint32				_NumVertices;
	uint32				_NumTriangles;
	/// List of bones and vertices they influence
	std::vector<CBoneInfluence>		_Bones;

	// The map of bone.
	TStrIdMap			_BoneMap;

	/// numTriangles * 3.
	std::vector<uint32>	_TriangleIndices;

	/// List of animation.
	std::vector<CAnim>	_Anims;

	// The map of animation.
	TStrIdMap			_AnimMap;

};


} // NL3D


#endif // NL_LOD_CHARACTER_SHAPE_H

/* End of lod_character_shape.h */
