/** \file export_nel.h
 * Export from 3dsmax to NeL
 *
 * $Id: export_nel.h,v 1.52 2002/05/13 16:49:00 berenguier Exp $
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
#include "3d/mesh.h"
#include "3d/material.h"
#include "3d/mesh_vertex_program.h"
#include <3d/key.h>
#include <3d/track_keyframer.h>
#include <3d/bone.h>
#include <nel/3d/animation_time.h>
#include <nel/3d/animation_time.h>
#include <pacs/collision_mesh_build.h>

#define UVGEN_MISSING (-1)
#define UVGEN_REFLEXION (-2)
#define FLOAT_EPSILON 0.001

#define NEL_MTL_A						0x64c75fec
#define NEL_MTL_B						0x222b9eb9
#define NEL_LIGHT_CLASS_ID_A			0x36e3181f
#define NEL_LIGHT_CLASS_ID_B			0x3ac24049
#define NEL_PACS_BOX_CLASS_ID_A			0x7f374277
#define NEL_PACS_BOX_CLASS_ID_B			0x5d3971df
#define NEL_PACS_CYL_CLASS_ID_A			0x62a56810
#define NEL_PACS_CYL_CLASS_ID_B			0x4b3d601c
#define NEL_PARTICLE_SYSTEM_CLASS_ID	0x58ce2893
#define NEL_FLARE_CLASS_ID_A			0x4e913532
#define NEL_FLARE_CLASS_ID_B			0x3c2f2307
#define NEL_WAVE_MAKER_CLASS_ID_A		0x77e24828
#define NEL_WAVE_MAKER_CLASS_ID_B		0x329a1de5
#define MAX_MORPHER_CLASS_ID			Class_ID(0x17bb6854, 0xa5cba2a3)



// ***************************************************************************

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

// ***************************************************************************

enum TNelScriptValueType
{
	scriptNothing,
	scriptFloat,
	scriptBool,
};

// ***************************************************************************

namespace NL3D
{
	class CAnimation;
	class ITrack;
	class CTrackKeyFramerConstBool;
	class CSkeletonShape;
	class CMRMParameters;
	class IMeshGeom;
	class CInstanceGroup;
	class CVegetableShape;
	class CLodCharacterShapeBuild;
};


namespace NLPACS
{
	class CRetrieverBank;
	class CGlobalRetriever;
	class CPrimitiveBlock;
};


// ***************************************************************************
// Interface for feed back during calculation
class IProgress
{
public:
	virtual void setLine (uint32 LineNumber, std::string &LineText)=0;
	virtual void update()=0;
};

// ***************************************************************************
struct CExportNelOptions
{
	bool bShadow;
	bool bExportLighting;
	bool bExportBgColor;
	std::string sExportLighting;
	sint32 nExportLighting;
	float rLumelSize;
	sint32 nOverSampling;
	bool bExcludeNonSelected;
	IProgress *FeedBack;
	bool bShowLumel;
	bool bTestSurfaceLighting;
	float SurfaceLightingCellSize;
	float SurfaceLightingDeltaZ;


	CExportNelOptions::CExportNelOptions()
	{
		// If no configuration file
		bShadow = false;
		bExportLighting = false;
		bExportBgColor = true;
		sExportLighting = "c:\\temp";
		nExportLighting = 0; // Normal lighting
		rLumelSize = 0.25f;
		nOverSampling = 1;
		bExcludeNonSelected = false;
		FeedBack = NULL;
		bShowLumel = false;
		bTestSurfaceLighting= true;
		SurfaceLightingCellSize= 1.5f;
		SurfaceLightingDeltaZ= 0.8f;
	}

	void serial(NLMISC::IStream& stream)
	{
		sint version = stream.serialVersion (4);

		// Check version
		switch (version)
		{
		case 4:
			stream.serial (SurfaceLightingDeltaZ);
		case 3:
			stream.serial (bTestSurfaceLighting);
			stream.serial (SurfaceLightingCellSize);
		case 2:
			stream.serial (bExportBgColor);
		case 1:
			stream.serial (bShowLumel);
			stream.serial (bShadow);
			stream.serial (bExportLighting);
			stream.serial (sExportLighting);
			stream.serial (nExportLighting);
			stream.serial (rLumelSize);
			stream.serial (nOverSampling);
			stream.serial (bExcludeNonSelected);
		}
	}
};

// ***************************************************************************

class CExportDesc;

// ***************************************************************************

typedef std::map<INode*, sint32> TInodePtrInt;


// ***************************************************************************
/**	Descriptor of a WindTree VertexProgram AppData
 */
class	CVPWindTreeAppData
{
public:
	enum	{HrcDepth= 3, NumTicks=100};

	/// Scale value for sliders
	float		FreqScale;
	float		DistScale;

	/// Frequency of the wind for 3 Hierachy levels. Slider Value
	int			Frequency[HrcDepth];
	/// Additional frequency, multiplied by the globalWindPower. Slider Value
	int			FrequencyWindFactor[HrcDepth];
	/// Power of the wind on XY. Mul by globalWindPower. Slider Value
	int			DistXY[HrcDepth];
	/// Power of the wind on Z. Mul by globalWindPower. Slider Value
	int			DistZ[HrcDepth];
	/// Bias result of the cosinus: f= cos(time)+bias. Slider Value
	int			Bias[HrcDepth];

	/// BST_CHECKED if want Specular Lighting.
	int			SpecularLighting;
};


// ***************************************************************************

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
	CExportNel (bool errorInDialog, bool view, bool absolutePath, Interface *ip, std::string errorTitle);

	// *********************
	// *** Ëxport mesh
	// *********************

	/**
	  * Build a NeL mesh
	  *
	  * skeletonShape must be NULL if no bones.
	  */
	NL3D::IShape*					buildShape (INode& node, TimeValue time, const TInodePtrInt *nodeMap, 
												CExportNelOptions &opt);

	/**
	  * Build a NeL meshBuild
	  * 
	  * This method does not care of the skeletonShape
	  */
	NL3D::CMesh::CMeshBuild*		createMeshBuild(INode& node, TimeValue tvTime, NL3D::CMesh::CMeshBaseBuild*& baseBuild, const NLMISC::CMatrix &finalSpace = NLMISC::CMatrix::Identity);

	/** Test wether has app datas specifying interface meshs.
	  * \see applyInterfaceToMeshBuild
	  */ 
	static bool						useInterfaceMesh(INode &node);


	/** Use interface mesh from a max file to unify normal at extremities of a mesh
	  * Example : a character is sliced in slots, each parts being stored in a file.
	  * The normals at the junction part are incorrect.
	  * An "interface" is a polygon that is used to unify normal between various meshs 
	  * IMPORTANT : the meshbuild should have been exported in WORLD SPACE
	  * Note : the name of the max file that contains the name of the interface is stored in an app data attached to this node
	  * \param node the node from which datas must be retrieved (name of the .max file containing the interfaces)
	  * \param meshBuildToModify The mesh build whose normal will be modified
  	  * \param toWorldMat a matrix to put the meshbuild vertices into worldspace
	  * \param tvTime time aty which evaluate the mesh
	  */
	void							applyInterfaceToMeshBuild(INode &node, NL3D::CMesh::CMeshBuild &meshBuildToModify, const NLMISC::CMatrix &toWorldMat, TimeValue tvTime);

	/** This takes a max mesh, and select the vertices that match vertices of a mesh interface
	  * This has no effect if the mesh has no app datas specifying a mesh interface
	  * \see applyInterfaceToMeshBuild
	  * \return true if the operation succeed
	  */ 
	bool							selectInterfaceVertices(INode &node, TimeValue time);	

	/**
	  * Build a NeL instance group
	  *
	  */
	NL3D::CInstanceGroup*			buildInstanceGroup(const std::vector<INode*>& vectNode, std::vector<INode*>& resultInstanceNode, TimeValue tvTime);

	/**
	  * Return true if it is a mesh.
	  *
	  * skeletonShape must be NULL if no bones.
	  *
	  *	if excludeCollision then return false if the mesh is a collision (NL3D_APPDATA_COLLISION)
	  *	else don't test NL3D_APPDATA_COLLISION.
	  */
	static bool						isMesh (INode& node, TimeValue time, bool excludeCollision= true);
	static bool						isVegetable (INode& node, TimeValue time);

	/**
	  * Return true if the node is a mesh and has a Nel_Material attached to it
	  */
	static bool						hasLightMap (INode& node, TimeValue time);
	static void						deleteLM (INode& node, CExportNelOptions& structExport);
	bool							calculateLM (NL3D::CMesh::CMeshBuild *pZeMeshBuild, 
												NL3D::CMeshBase::CMeshBaseBuild *pZeMeshBaseBuild,
												INode& ZeNode, 
												TimeValue tvTime, CExportNelOptions& structExport, uint firstMaterial);

	bool							calculateLMRad(NL3D::CMesh::CMeshBuild *pZeMeshBuild, 
												NL3D::CMeshBase::CMeshBaseBuild *pZeMeshBaseBuild,
												INode& ZeNode, 
												TimeValue tvTime, CExportNelOptions& structExport);
	

	// *********************
	// *** Export animation
	// *********************

	// Add animation track of this node into the animation object pass in parameter.
	void							addAnimation (NL3D::CAnimation& animation, INode& node, const char* sBaseName, bool root);

	// Build a NeL track with a 3dsmax node and a controller.
	NL3D::ITrack*					buildATrack (NL3D::CAnimation& animation, Control& c, TNelValueType type, Animatable& node, const CExportDesc& desc, 
												std::set<TimeValue>* previousKeys, std::set<TimeValue>* previousKeysSampled, bool bodyBiped=false);

	// Build a Nel bool track from a On/Off max Controller (doesn't work with buildATRack, which require a keyframer interface
	// , which isn't provided by an on / off controller)
	static NL3D::CTrackKeyFramerConstBool*			buildOnOffTrack(Control& c);

	// Add tracks for particle systems
	void							addParticleSystemTracks(NL3D::CAnimation& animation, INode& node, const char* parentName) ;

	// Add tracks for the bone and its children (recursive)
	void							addBoneTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, bool root);

	// Add biped tracks
	void							addBipedNodeTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, bool root);
	void							addBipedNodeTrack (NL3D::CAnimation& animation, INode& node, const char* parentName,
										std::set<TimeValue>& previousKeys, std::set<TimeValue>& previousKeysSampled, bool root);
//	void							addBipedPathTrack (NL3D::CAnimation& animation, INode& node, const char* parentName);


	// Add a note track. It tackes the first note track of the object
	static void						addNoteTrack(NL3D::CAnimation& animation, INode& node);

	// Convert keyframe methods
	static void						buildNelKey (NL3D::CKeyFloat& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyInt& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyVector& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyRGBA& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyQuat& nelKey, ILinRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyVector& nelKey, ILinScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);

	static void						buildNelKey (NL3D::CKeyBezierFloat& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBezierQuat& nelKey, IBezQuatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);

	static void						buildNelKey (NL3D::CKeyTCBFloat& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyTCBQuat& nelKey, ITCBRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);	


	// Create the transform matrix tracks
	void							createBipedKeyFramer (NL3D::ITrack *&nelRot, NL3D::ITrack *&nelPos, bool isRot, bool isPos, 
														float ticksPerSecond, const Interval& range, int oRT, const CExportDesc& desc, 
														INode& node, std::set<TimeValue>* previousKeys, 
														std::set<TimeValue>* previousKeysSampled);

	// convert to nel time value
	static NL3D::TAnimationTime		convertTime (TimeValue time);

	// ************************************
	// *** Export skeleton and skinning
	// ************************************

	/* 
	 * Build a skeleton shape
	 *
	 * mapBindPos is the pointer of the map of bind pos by bone. Can be NULL if the skeleton is already in the bind pos.
	 */
	void							buildSkeletonShape (NL3D::CSkeletonShape& skeletonShape, INode& node, mapBoneBindPos* mapBindPos, 
														TInodePtrInt& mapId, TimeValue time);

	// Build an array of CBoneBase
	void							buildSkeleton (std::vector<NL3D::CBoneBase>& bonesArray, INode& node, mapBoneBindPos* mapBindPos, 
														TInodePtrInt& mapId, std::set<std::string> &nameSet, 
														TimeValue time, sint32& idCount, sint32 father=-1);

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
	static uint						buildSkinning (NL3D::CMesh::CMeshBuild& buildMesh, const TInodePtrInt& skeletonShape, INode& node);

	// Return true if the mesh is a skin, else return false.
	static bool						isSkin (INode& node);

	// Return the root node of the skeleton attached to the node. Return NULL if no skeleton.
	static INode*					getSkeletonRootBone (INode& node);

	// Add bind pose matrix of the bone used by the model in the map.
	static void						addSkeletonBindPos (INode& node, mapBoneBindPos& boneBindPos);

	// Enable / disable the skin modifier
	static void						enableSkinModifier (INode& node, bool enable);

	// **************
	// *** Ëxport Lod
	// **************

	void							addChildLodNode (std::set<INode*> &lodListToExclude, INode *current = NULL);
	void							addParentLodNode (INode &node, std::set<INode*> &lodListToExclude, INode *current = NULL);

	// *********************
	// *** Ëxport collision
	// *********************

	/** Export a CCollisionMeshBuild from a list of node.
	 *	NB: do not check NEL3D_APPDATA_COLLISION.
	 */
	NLPACS::CCollisionMeshBuild*	createCollisionMeshBuild(std::vector<INode *> &nodes, TimeValue tvTime);

	/** Export a list of CCollisionMeshBuild from a list of node, grouped by igName. User must delete the meshBuilds.
	 *	meshBuildList is a vector of tuple igName-Cmb
	 *	NB: check NEL3D_APPDATA_COLLISION. if not set, the node is not exported
	 */
	bool							createCollisionMeshBuildList(std::vector<INode *> &nodes, TimeValue time, 
																	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> > &meshBuildList);

	/**
	 *	Parse all the scene and build retrieverBank and globalRetriever. NULL if no collisions mesh found.
	 *	\param retIgName eg: if IgName of a collisionMesh "col_pipo_1" is found in the scene, and if 
	 *	igNamePrefix=="col_" and igNameSuffix=="_", then retIgName returned is "pipo".
	 *	If different igName may match, result is undefined (random).
	 */
	void							computeCollisionRetrieverFromScene(TimeValue time, 
																	NLPACS::CRetrieverBank *&retrieverBank, NLPACS::CGlobalRetriever *&globalRetriever,
																	const char *igNamePrefix, const char *igNameSuffix, std::string &retIgName);

	/**
	  * Retrieve the Z rotation in radian of an object with its I matrix vector, assuming that the K matrix vector is near CVector::K
	  */
	static float getZRot (const NLMISC::CVector &i);

	/**
	  * Build a primtive block with an array of inode. Return false if one of the object is not a PACS primitive object.
	  */
	bool							buildPrimitiveBlock (TimeValue time, std::vector<INode*> objects, NLPACS::CPrimitiveBlock &primitiveBlock);

	// *********************
	// *** Ëxport misc
	// *********************

	// Transforme a 3dsmax view matrix to camera matrix.
	static Matrix3					viewMatrix2CameraMatrix (const Matrix3& viewMatrix);

	// Convert a 3dsmax matrix in NeL matrix
	static void						convertMatrix (NLMISC::CMatrix& nelMatrix, const Matrix3& maxMatrix);

	// Convert a NeL matrix in 3dsmax matrix
	static void						convertMatrix (Matrix3& maxMatrix, const NLMISC::CMatrix& nelMatrix);

	/// Convert a 3dsmax uv matrix to a nel uv matrix
	static void						uvMatrix2NelUVMatrix (const Matrix3& uvMatrix, NLMISC::CMatrix &dest);

	// Convert a 3dsmax vector in NeL vector
	static void						convertVector (NLMISC::CVector& nelVector, const Point3& maxVector);

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

	// Get the NEL node name. ie either NEL_OBJET_NAME_DATA appData or just GetName().
	static std::string				getNelObjectName (INode& node);

	// Get lights
	void							getLights (std::vector<NL3D::CLight>& vectLight, TimeValue time, INode* node=NULL);

	// Get All node (objects only) of a hierarchy. NULL => all the scene
	void							getObjectNodes (std::vector<INode*>& vectNode, TimeValue time, INode* node=NULL);

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
	NLMISC::CRGBA					getAmbientColor (TimeValue time);

	// Get the backgorund value
	NLMISC::CRGBA					getBackGroundColor (TimeValue time);

	// Get LightGroupName, from AppData (New style), or NelLight modifier (oldStyle) if appData groupName undefined
	static std::string				getLightGroupName (INode *node);

	// *** Script access

	// Eval a scripted function
	static bool						scriptEvaluate (const char *script, void *out, TNelScriptValueType type);

	// *** Appdata access

	// Get an appData float
	static float 					getScriptAppData (Animatable *node, uint32 id, float def);

	// Set an appData float
	static void						setScriptAppData (Animatable *node, uint32 id, float value);

	// Get an appData integer
	static int						getScriptAppData (Animatable *node, uint32 id, int def);

	// Set an appData integer
	static void						setScriptAppData (Animatable *node, uint32 id, int value);

	// Get an appData integer
	static std::string				getScriptAppData (Animatable *node, uint32 id, const std::string& def);

	// Set an appData integer
	static void						setScriptAppData (Animatable *node, uint32 id, const std::string& value);

	// Output error message
	void							outputErrorMessage (const char *message);


	// Get an appData VertexProgram WindTree (ensure same default values for all retrieve).
	static void						getScriptAppDataVPWT (Animatable *node, CVPWindTreeAppData &apd);

	// Set an appData VertexProgram WindTree.
	static void						setScriptAppDataVPWT (Animatable *node, const CVPWindTreeAppData &apd);

	// private func : this convert a polygon expressed as a max mesh into a list of ordered vectors
	static void						maxPolygonMeshToOrderedPoly(Mesh &mesh, std::vector<NLMISC::CVector> &dest);


	// ********************
	// *** Export Vegetable
	// ********************

	/* 
	 * Build a skeleton shape
	 *
	 * mapBindPos is the pointer of the map of bind pos by bone. Can be NULL if the skeleton is already in the bind pos.
	 */
	bool							buildVegetableShape (NL3D::CVegetableShape& skeletonShape, INode& node, TimeValue time);


	// ********************
	// *** Export Lod Character
	// ********************

	/* is this node a lod character ???
	 */
	static bool						isLodCharacter (INode& node, TimeValue time);

	/* 
	 * Build a lod character from a node
	 */
	bool							buildLodCharacter (NL3D::CLodCharacterShapeBuild& lodBuild, INode& node, TimeValue time, const TInodePtrInt *nodeMap);

	// *************
	// *** Misc  ***
	// *************

	// get a ptr to max interface
	Interface					  *getInterface() const { return _Ip; }	


private:

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
		const Matrix3 &getUVMatrix() const { return _UVMatrix; }
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

	// Max material info
	class CMaxMaterialInfo
	{
	public:
		// Default constructor
		CMaxMaterialInfo ()
		{
			AlphaVertex = false;
			ColorVertex = false;
			AlphaVertexChannel = 0;
			MappingChannelUsed = 0;
			TextureMatrixEnabled = false;
		};

		// Remap UV channel
		std::vector<CMaterialDesc>					RemapChannel;

		// Material names
		std::string									MaterialName;

		// Alpha vertex in this material
		bool										AlphaVertex;

		// Mapping channel used
		uint										MappingChannelUsed;

		// Color vertex in this material
		bool										ColorVertex;

		// Alpha vertex channel for this material
		uint										AlphaVertexChannel;

		// allow to export a user texture matrix
		bool										TextureMatrixEnabled;
	};

	// Max base build structure
	class CMaxMeshBaseBuild
	{
	public:
		CMaxMeshBaseBuild ()
		{
			NeedVertexColor = false;
			MappingChannelUsed = 0;
		}

		// First material in the array
		uint										FirstMaterial;

		// Num of materials
		uint										NumMaterials;

		// Mapping channel used
		uint										MappingChannelUsed;

		// Need vertex color
		bool										NeedVertexColor;

		// Remap UV channel
		std::vector<CMaxMaterialInfo>				MaterialInfo;
	};

	// *********************
	// *** Ëxport mesh
	// *********************

	// Get 3ds UVs channel used by a texmap and make a good index channel
	// Can return an interger in [0; MAX_MESHMAPS-1] or on of the following value: UVGEN_REFLEXION, UVGEN_MISSING
	static int						getVertMapChannel (Texmap& texmap, Matrix3& channelMatrix, TimeValue time);

	// Build a CLight
	static bool						buildLight (GenLight &maxLight, NL3D::CLight& nelLight, INode& node, TimeValue time);

	/**
	  * Build a NeL base mesh interface
	  *
	  * if skeletonShape is NULL, no skinning is exported.
	  */
	void							buildBaseMeshInterface (NL3D::CMeshBase::CMeshBaseBuild& buildMesh, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
															TimeValue time, const NLMISC::CMatrix& basis);

	/**
	  * Build a NeL mesh interface
	  *
	  * if skeletonShape is NULL, no skinning is exported.
	  */
	void							buildMeshInterface (TriObject &tri, NL3D::CMesh::CMeshBuild& buildMesh, const NL3D::CMeshBase::CMeshBaseBuild& buildBaseMesh, 
														const CMaxMeshBaseBuild& maxBaseBuild, INode& node, TimeValue time, const TInodePtrInt* nodeMap, 
														const NLMISC::CMatrix& newBasis=NLMISC::CMatrix::Identity, 
														const NLMISC::CMatrix& finalSpace=NLMISC::CMatrix::Identity);


	/**
	  * Get all the blend shapes from a node in the meshbuild form
	  */
	void							getBSMeshBuild (std::vector<NL3D::CMesh::CMeshBuild*> &bsList, INode &node, TimeValue time, bool skined);
	
	/**
	  * Build a NeL mrm parameters block
	  */
	static void						buildMRMParameters (Animatable& node, NL3D::CMRMParameters& params);

	/**
	  * Build a mesh geom with a node
	  */
	NL3D::IMeshGeom					*buildMeshGeom (INode& node, TimeValue time, const TInodePtrInt *nodeMap,
													CExportNelOptions &opt, NL3D::CMeshBase::CMeshBaseBuild &buildBaseMesh, std::vector<std::string>& listMaterialName,
													bool& isTransparent, bool& isOpaque, const NLMISC::CMatrix& parentMatrix);
	/**
	  * Build the mesh morpher info in the mesh geom	  */
	void							buildMeshMorph (NL3D::CMesh::CMeshBuild& buildMesh, INode &node, TimeValue time, bool skined);

	// Get the normal of a face for a given corner in localSpace
	static Point3					getLocalNormal (int face, int corner, Mesh& mesh);

	// Build a water shape. The given node must have a water materiel, an assertion is raised otherwise
	NL3D::IShape					*buildWaterShape(INode& node, TimeValue time);

	// build a wave maker shape
	NL3D::IShape					*CExportNel::buildWaveMakerShape(INode& node, TimeValue time);

	// *********************
	// *** Ëxport material
	// *********************

	/** Test wether the given max node has a water material. A water object should only have one material, and must have planar, convex geometry.
	* Morevover, the mapping should only have scale and offsets, no rotation
	*/
	static bool						hasWaterMaterial(INode& node, TimeValue time);

	// Build an array of NeL material corresponding with max material at this node.
	void							buildMaterials (std::vector<NL3D::CMaterial>& Materials, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
													TimeValue time);

	// Build a NeL material corresponding with a max material.
	void							buildAMaterial (NL3D::CMaterial& material, CMaxMaterialInfo& materialInfo, Mtl& mtl, TimeValue time);

	// Build a NeL texture corresponding with a max Texmap.
	NL3D::ITexture*					buildATexture (Texmap& texmap, CMaterialDesc& remap3dsTexChannel, TimeValue time, bool forceCubic=false);

public:
	 /** Return true if a mesh has a material whose shader requires a specific vertex shader to work (for example, per-pixel lighting).
	   * This also stores the result in shader
	   */
	 static bool					hasMaterialWithShaderForVP(INode &node, TimeValue time, NL3D::CMaterial::TShader &shader);

	/// Test wether the given material need a specific vertex program to work correctly
	 static	bool                      needVP(Mtl &mat, TimeValue time, NL3D::CMaterial::TShader &shader);
private:

	/** Build a mesh vertex program associated with the given shader
	  * For now there can be only one vertex program per mesh, so you must specify the shader this v.p must be built for.
	  * The meshBuild may be modfied by this operation (to add tangent space infos for example)
	  * \return The vertex program or NULL if the op failed, or if there's no need for a v.p.
	  */
	 static NL3D::IMeshVertexProgram           *buildMeshMaterialShaderVP(NL3D::CMaterial::TShader shader, NL3D::CMesh::CMeshBuild *mb);

	/// Test wether a material need a vertex program

	// *********************
	// *** Export Animation
	// *********************

	// Add tracks for the node
	void							addNodeTracks (NL3D::CAnimation& animation, INode& node, const char* parentName,
													std::set<TimeValue>* previousKeys, std::set<TimeValue>* previousKeysSampled, bool root, 
													bool bodyBiped=false);

	// Add tracks for the node's bones 
	void							addBonesTracks (NL3D::CAnimation& animation, INode& node, const char* parentName);

	// Add tracks for the light
	void							addLightTracks (NL3D::CAnimation& animation, INode& node, const char* parentName);

	// Add tracks for the morphing
	void							addMorphTracks (NL3D::CAnimation& animation, INode& node, const char* parentName);
	
	// Add tracks for the object
	void							addObjTracks (NL3D::CAnimation& animation, Object& obj, const char* parentName);

	// Add tracks for the material
	void							addMtlTracks (NL3D::CAnimation& animation, Mtl& mtl, const char* parentName);

	// Add tracks for the texture
	void							addTexTracks (NL3D::CAnimation& animation, Texmap& tex, uint stage, const char* parentName);

	// Add controller key time in the set
	void							addBipedKeyTime (Control& c, std::set<TimeValue>& keys, std::set<TimeValue>& keysSampled, bool subKeys, 
													const char *nodeName);
	

	// Get a biped key parameter using script
	bool							getBipedKeyInfo (const char* nodeName, const char* paramName, uint key, float& res);

	// Get inplace biped mode
	bool							getBipedInplaceMode (const char* nodeName, const char* inplaceFunction, 
															bool &res);

	// Change inplace biped mode
	bool							setBipedInplaceMode (const char* nodeName, const char* inplaceFunction, 
															bool onOff);
private:

	// Pointer on the interface
	Interface						*_Ip;

	// Texture are built path absolute
	bool							_AbsolutePath;

	// Build to view the scene
	bool							_View;

	// Errors goes in dialog
	bool							_ErrorInDialog;

	// Error title
	std::string						_ErrorTitle;
};


#endif // NL_EXPORT_NEL_H

/* End of export_nel.h */
