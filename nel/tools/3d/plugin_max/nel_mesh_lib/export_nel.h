/** \file export_nel.h
 * Export from 3dsmax to NeL
 *
 * $Id: export_nel.h,v 1.2 2001/06/11 09:21:53 besson Exp $
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

#ifndef NL_EXPORT_NEL_H
#define NL_EXPORT_NEL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh.h"
#include <nel/3d/key.h>
#include <nel/3d/track_keyframer.h>
#include <nel/3d/bone.h>
#include <nel/3d/tmp/animation_time.h>

#define UVGEN_MISSING (-1)
#define UVGEN_OBJXYZ (MAX_MESHMAPS+1)
#define UVGEN_WORLDXYZ (MAX_MESHMAPS+2)
#define FLOAT_EPSILON 0.001

enum TNelValueType
{
	typeFloat,
	typePos,
	typeScale,
	typeRotation,
	typeColor,
	typeInt,
	typeBoolean,
	typeString,
	typeMatrix,
};

enum TNelScriptValueType
{
	scriptFloat,
};

namespace NL3D
{
	class CAnimation;
	class ITrack;
	class CSkeletonShape;
};

// -----------------------------------------------
struct CExportNelOptions
{
	bool bShadow;
	bool bExportLighting;
	std::string sExportLighting;
	sint32 nExportLighting;
	float rLumelSize;
	sint32 nOverSampling;

	CExportNelOptions::CExportNelOptions()
	{
		// If no configuration file
		bShadow = false;
		bExportLighting = false;
		sExportLighting = "c:\\mydir\\lightmap\\";
		nExportLighting = 0; // Normal lighting
		rLumelSize = 0.25f;
		nOverSampling = 1;
	}

	void serial(NLMISC::IStream& stream)
	{
		stream.serial( bShadow );
		stream.serial( bExportLighting );
		stream.serial( sExportLighting );
		stream.serial( nExportLighting );
		stream.serial( rLumelSize );
		stream.serial( nOverSampling );
	}
};
// -----------------------------------------------


class CExportDesc;

/**
 * 3dsmax to NeL export interface for other things that landscape.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CExportNel
{
public:
	enum
	{
		NoError=0,
		VertexWithoutWeight,
		InvalidSkeleton,
		CodeCount
	};

	static const char* ErrorMessage[CodeCount];

	typedef std::map<INode*, Matrix3> mapBoneBindPos;

	/// Constructor
	CExportNel();

	// *********************
	// *** Ëxport mesh
	// *********************

	/**
	  * Build a NeL mesh
	  *
	  * skeletonShape must be NULL if no bones.
	  */
	static NL3D::IShape*			buildShape (INode& node, Interface& ip, TimeValue tvTime, 
												const NL3D::CSkeletonShape* skeletonShape, bool absolutePath,
												CExportNelOptions &opt);

	/**
	  * Build a NeL meshBuild
	  * 
	  * This method does not care of the skeletonShape
	  */
	static NL3D::CMesh::CMeshBuild*	createMeshBuild(INode& node, TimeValue tvTime);

	/**
	  * Return true if it is a mesh.
	  *
	  * skeletonShape must be NULL if no bones.
	  */
	static bool						isMesh (INode& node, TimeValue time);

	/**
	  * Return true if the node is a mesh and has a Nel_Material attached to it
	  */
	static bool						hasLightMap (INode& node, TimeValue time);
	static void						deleteLM (INode& node, CExportNelOptions& structExport);
	static bool						calculateLM (NL3D::CMesh::CMeshBuild *pZeMeshBuild, INode& ZeNode, 
												Interface& ip, TimeValue tvTime, bool absolutePath,
												CExportNelOptions& structExport);


	// *********************
	// *** Export animation
	// *********************

	// Add animation track of this node into the animation object pass in parameter.
	static void						addAnimation (NL3D::CAnimation& animation, INode& node, const char* sBaseName, Interface *ip);

	// Build a NeL track with a 3dsmax node and a controller.
	static NL3D::ITrack*			buildATrack (NL3D::CAnimation& animation, Control& c, TNelValueType type, Animatable& node, const CExportDesc& desc, 
												Interface *ip);

	// Add tracks for the bone and its children (recursive)
	static void						addBoneTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, Interface *ip);

	// Convert keyframe methods
	static void						buildNelKey (NL3D::CKeyFloat& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyInt& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyVector& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyRGBA& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyQuat& nelKey, ILinRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyVector& nelKey, ILinScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc);

	static void						buildNelKey (NL3D::CKeyBezierFloat& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyBezierQuat& nelKey, IBezQuatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc);

	static void						buildNelKey (NL3D::CKeyTCBFloat& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyTCBQuat& nelKey, ITCBRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc);
	static void						buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc);

	// Create the transform matrix tracks
	static void						createBipedKeyFramer (NL3D::ITrack *&nelRot, NL3D::ITrack *&nelPos, bool isRot, bool isPos, 
														float ticksPerSecond, const Interval& range, int oRT, const CExportDesc& desc, 
														INode& node, const std::set<TimeValue>& ikeys, Interface *ip);

	// convert to nel time value
	static NL3D::CAnimationTime		convertTime (TimeValue time);

	// ************************************
	// *** Export skeleton and skinning
	// ************************************

	/* 
	 * Build a skeleton shape
	 *
	 * mapBindPos is the pointer of the map of bind pos by bone. Can be NULL if the skeleton is already in the bind pos.
	 */
	static void						buildSkeletonShape (NL3D::CSkeletonShape& skeletonShape, INode& node, mapBoneBindPos* mapBindPos, TimeValue time);

	// Build an array of CBoneBase
	static void						buildSkeleton (std::vector<NL3D::CBoneBase>& bonesArray, INode& node, mapBoneBindPos* mapBindPos, TimeValue time, sint32& idCount, sint32 father=-1);

	/*
	 * Add Skinning data into the build structure.
	 * You must call first isSkin. If isSkin return true, then you can call this method.
	 * Can return the following errors:
	 *
	 * skeletonShape must be NULL if no bones.
	 * 
	 * NoError
	 * VertexWithoutWeight
	 * InvalidSkeleton
	 */
	static uint						buildSkinning (NL3D::CMesh::CMeshBuild& buildMesh, const NL3D::CSkeletonShape& skeletonShape, INode& node);

	// Return true if the mesh is a skin, else return false.
	static bool						isSkin (INode& node);

	// Return the root node of the skeleton attached to the node. Return NULL if no skeleton.
	static INode*					getSkeletonRootBone (INode& node);

	// Add bind pose matrix of the bone used by the model in the map.
	static void						addSkeletonBindPos (INode& node, mapBoneBindPos& boneBindPos);

	// Enable / disable the skin modifier
	static void						enableSkinModifier (INode& node, bool enable);

	// *********************
	// *** Ëxport misc
	// *********************

	// Transforme a 3dsmax view matrix to camera matrix.
	static Matrix3					viewMatrix2CameraMatrix (const Matrix3& viewMatrix);

	// Convert a 3dsmax matrix in NeL matrix
	static void						convertMatrix (NLMISC::CMatrix& nelMatrix, const Matrix3& maxMatrix);

	// Get local node matrix
	static void						getLocalMatrix (Matrix3& localMatrix, INode& node, TimeValue time);

	// Decompose a 3dsmax matrix in NeL translation, scale, quat.
	static void						decompMatrix (NLMISC::CVector& nelScale, NLMISC::CQuat& nelRot, NLMISC::CVector& nelPos, 
													const Matrix3& maxMatrix);
	// Convert a 3dsmax color in NeL color
	static void						convertColor (NLMISC::CRGBA& nelColor, const Color& maxColor);

	// Return true if the node as the classId classid or is derived from a class with classId class id
	static bool						isClassIdCompatible (Animatable& node, Class_ID& classId);

	// Return the pointer on ther subanim with the name sName of the node. If it doesn't exist, return NULL.
	static Animatable*				getSubAnimByName (Animatable& node, const char* sName);

	// Get the node name
	static std::string				getName (MtlBase& mtl);

	// Get the node name
	static std::string				getName (INode& node);

	// Get lights
	static void						getLights (std::vector<NL3D::CLight>& vectLight, TimeValue time, Interface& ip, INode* node=NULL);

	// *** Paramblock2 access

	// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
	static Control*					getControlerByName (Animatable& node, const char* sName);

	// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
	// Support only those type:
	// TYPE_FLOAT
	// TYPE_INT
	// TYPE_POINT3
	// TYPE_BOOL
	static bool						getValueByNameUsingParamBlock2 (Animatable& node, const char* sName, 
																	ParamType2 type, void *pValue, TimeValue time);

	// Get the first modifier in the pipeline of a node by its class identifier
	static Modifier*				getModifier (INode* pNode, Class_ID modCID);

	// Get the ambient value
	static NLMISC::CRGBA			getAmbientColor (Interface& ip, TimeValue time);

	// *** Script access
	static bool						scriptEvaluate (Interface *ip, char *script, void *out, TNelScriptValueType type);

private:

	// *********************
	// *** Ëxport mesh
	// *********************

	// Get 3ds UVs channel used by a texmap and make a good index channel
	// Can return an interger in [0; MAX_MESHMAPS-1] or on of the following value: UVGEN_MISSING, UVGEN_OBJXYZ or UVGEN_WORLDXYZ
	static int						getVertMapChannel (Texmap& texmap, Matrix3& channelMatrix);

	// Build a CLight
	static bool						buildLight (GenLight &maxLight, NL3D::CLight& nelLight, INode& node, TimeValue time);

	/**
	  * Build a NeL mesh interface
	  *
	  * if skeletonShape is NULL, no skinning is exported.
	  */
	static void						buildMeshInterface (TriObject &tri, NL3D::CMesh::CMeshBuild& buildMesh, std::vector<std::string>& materialNames, 
														INode& node, TimeValue time, const NL3D::CSkeletonShape* skeletonShape, bool absolutePath);

	// Get the normal of a face for a given corner in localSpace
	static Point3					getLocalNormal (int face, int corner, Mesh& mesh);

	// *********************
	// *** Ëxport material
	// *********************

	// A class that describe how build a NeL material with a max one
	class CMaterialDesc
	{
	public:
		friend class CExportNel;

		// Default constructor init _IndexInMaxMaterial to MISSING_VALUE, _UVMatrix to indentity and clamp to 0, 0, 1, 1
		CMaterialDesc ()
		{
			_IndexInMaxMaterial=UVGEN_MISSING;
			_IndexInMaxMaterialAlternative=UVGEN_MISSING;
			_UVMatrix.IdentityMatrix();
			_CropU=0.f;
			_CropV=0.f;
			_CropW=1.f;
			_CropH=1.f;
		}
	private:
		// *** Data

		// Index in material
		sint		_IndexInMaxMaterial;
		sint		_IndexInMaxMaterialAlternative;

		// Matrix for UVs
		Matrix3		_UVMatrix;

		// Crop region
		float		_CropU;
		float		_CropV;
		float		_CropW;
		float		_CropH;
	};

	// Build an array of NeL material corresponding with max material at this node. Return the number of material exported.
	static int						buildMaterials (std::vector<NL3D::CMaterial>& Materials, std::vector<std::vector<CMaterialDesc> >& 
													remap3dsTexChannel,	std::vector<std::string>& materialNames, INode& node, 
													TimeValue time, bool absolutePath);

	// Build a NeL material corresponding with a max material.
	static std::string				buildAMaterial (NL3D::CMaterial& material, std::vector<CMaterialDesc>& remap3dsTexChannel, 
													Mtl& mtl, TimeValue time, bool absolutePath);
	// Build a NeL texture corresponding with a max Texmap.
	static NL3D::ITexture*			buildATexture (Texmap& texmap, std::vector<CMaterialDesc>& remap3dsTexChannel, TimeValue time, bool absolutePath);

	// *********************
	// *** Export Animation
	// *********************

	// Add tracks for the node
	static void						addNodeTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, Interface *ip);

	// Add tracks for the node's bones 
	static void						addBonesTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, Interface *ip);

	// Add tracks for the object
	static void						addObjTracks (NL3D::CAnimation& animation, Object& obj, const char* parentName);

	// Add tracks for the material
	static void						addMtlTracks (NL3D::CAnimation& animation, Mtl& mtl, const char* parentName, Interface *ip);

	// Add tracks for the texture
	static void						addTexTracks (NL3D::CAnimation& animation, Texmap& tex, const char* parentName);

	// Add controller key time in the set
	static void						addKeyTime (Control& c, std::set<TimeValue>& ikeys, bool subKeys);

	// Get a biped key parameter using script
	static bool						getBipedKeyInfo (Interface *ip, const char* nodeName, const char* paramName, uint key, float& res);
};


#endif // NL_EXPORT_NEL_H

/* End of export_nel.h */
