/** \file export_skinning.cpp
 * Export skinning from 3dsmax to NeL. Works only with the com_skin2 plugin.
 *
 * $Id: export_skinning.cpp,v 1.13 2002/03/06 10:24:47 corvazier Exp $
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

#include "stdafx.h"
#include "export_nel.h"
#include "3d/skeleton_shape.h"

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

#define SKIN_INTERFACE 0x00010000

// ***************************************************************************

#define SKIN_CLASS_ID Class_ID(9815843,87654)
#define PHYSIQUE_CLASS_ID Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)

// ***************************************************************************

class ISkinContextData
{
public:
	virtual int GetNumPoints()=0;
	virtual int GetNumAssignedBones(int vertexIdx)=0;
	virtual int GetAssignedBone(int vertexIdx, int boneIdx)=0;
	virtual float GetBoneWeight(int vertexIdx, int boneIdx)=0;
	
	virtual int GetSubCurveIndex(int vertexIdx, int boneIdx)=0;
	virtual int GetSubSegmentIndex(int vertexIdx, int boneIdx)=0;
	virtual float GetSubSegmentDistance(int vertexIdx, int boneIdx)=0;
	virtual Point3 GetTangent(int vertexIdx, int boneIdx)=0;
	virtual Point3 GetOPoint(int vertexIdx, int boneIdx)=0;

	virtual void SetWeight(int vertexIdx, int boneIdx, float weight)=0;
	virtual void SetWeight(int vertexIdx, INode* bone, float weight)=0;
	virtual void SetWeights(int vertexIdx, Tab<int> boneIdx, Tab<float> weights)=0;
	virtual void SetWeights(int vertexIdx, INodeTab boneIdx, Tab<float> weights)=0;

};

// ***************************************************************************

class ISkin 
{
public:
	ISkin() {}
	~ISkin() {}
	virtual int GetBoneInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset = false)=0;
	virtual int GetSkinInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset = false)=0;
	virtual int GetNumBones()=0;
	virtual INode *GetBone(int idx)=0;
	virtual DWORD GetBoneProperty(int idx)=0;
	virtual ISkinContextData *GetContextInterface(INode *pNode)=0;

	virtual BOOL AddBone(INode *bone)=0;
	virtual BOOL AddBones(INodeTab *bones)=0;
	virtual BOOL RemoveBone(INode *bone)=0;
	virtual void Invalidate()=0;
};

// ***************************************************************************

void CExportNel::buildSkeletonShape (CSkeletonShape& skeletonShape, INode& node, mapBoneBindPos* mapBindPos, TInodePtrInt& mapId, 
									 TimeValue time, bool view)
{
	// Build a bone vector
	std::vector<CBoneBase> bonesArray;

	// Counter
	sint32 idCount=0;

	// Set for node name
	std::set<std::string> nameSet;

	// Parse the tree
	buildSkeleton (bonesArray, node, mapBindPos, mapId, nameSet, time, view, idCount);

	// Then build the object
	skeletonShape.build (bonesArray);
}

// ***************************************************************************

void CExportNel::buildSkeleton (std::vector<CBoneBase>& bonesArray, INode& node, mapBoneBindPos* mapBindPos, TInodePtrInt& mapId, 
								std::set<std::string> &nameSet, TimeValue time, bool view, sint32& idCount, sint32 father)
{
	// **** Save the current the id
	int id=idCount;

	// **** Create a bone
	CBoneBase bone;

	// ** Set Name and id

	// Bone name
	bone.Name=getName (node);

	// Inserted ?
	if (!nameSet.insert (bone.Name).second)
		bone.Name+="_Second";

	// Id of the father
	bone.FatherId=father;

	// Insert id with name
	mapId.insert (TInodePtrInt::value_type(&node, id));

	// ** Set inherit flags

	// Get the TM controler
	Control *c;
	c = node.GetTMController();

	// If a TM controler existd
	if (c)
	{
		// Get the inherit flags
		DWORD flags=c->GetInheritanceFlags();

		// Unherit scale if all scale inherit flags are cleared
		bone.UnheritScale=(flags&(INHERIT_SCL_X|INHERIT_SCL_Y|INHERIT_SCL_Z))!=0;
	}

	// ** Set default tracks

	// Get the local matrix for the default pos
	Matrix3 localTM (TRUE);

	// Root must be exported with Identity because path are setuped interactively in the root of the skeleton
	if (id!=0)
		getLocalMatrix (localTM, node, time);
	else
		int thisIsABodyControl=0;

	// Decomp the matrix to get default animations tracks
	NLMISC::CVector		nelScale;
	NLMISC::CQuat		nelRot;
	NLMISC::CVector		nelPos;
	decompMatrix (nelScale, nelRot, nelPos, localTM);

	// Set the default tracks
	bone.DefaultScale=nelScale;
	bone.DefaultRotQuat=nelRot;
	bone.DefaultRotEuler=CVector (0,0,0);
	bone.DefaultPos=nelPos;

		// Get node bindpos matrix
	Matrix3 worldTM;
	bool matrixComputed=false;
	if (mapBindPos)
	{
		// Look for an entry in the map
		mapBoneBindPos::iterator bindPos=mapBindPos->find (&node);

		// Found ?
		if (bindPos!=mapBindPos->end())
		{
			// Get bind pos
			worldTM=bindPos->second;

			// Computed
			matrixComputed=true;
		}
	}

	// Compute the matrix the normal way ?
	if (!matrixComputed)
	{
		worldTM=node.GetNodeTM (time);
	}

	// Set the bind pos of the bone, it is invNodeTM;
	convertMatrix (bone.InvBindPos, worldTM);
	bone.InvBindPos.invert();

	// **** Add the bone
	bonesArray.push_back (bone);

	// **** Call on child
	for (int children=0; children<node.NumberOfChildren(); children++)
		buildSkeleton (bonesArray, *node.GetChildNode(children), mapBindPos, mapId, nameSet, time, view, ++idCount, id);
}

// ***************************************************************************

bool CExportNel::isSkin (INode& node)
{
	// Return success
	bool ok=false;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASS_ID);

	// Found it ?
	if (skin)
	{
		// Modifier enabled ?
		//if (skin->IsEnabled())
		{
			// Get a com_skin2 interface
			ISkin *comSkinInterface=(ISkin*)skin->GetInterface (SKIN_INTERFACE);

			// Found com_skin2 ?
			if (comSkinInterface)
			{
				// Get local data
				ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

				// Found ?
				if (localData)
				{
					// Skinned
					ok=true;

					// Release the interface
					skin->ReleaseInterface (SKIN_INTERFACE, comSkinInterface);
				}
			}
		}
	}

	// Try physic
	if (!ok)
	{
		// Get the skin modifier
		Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Found it ?
		if (skin)
		{
			// Modifier enabled ?
			//if (skin->IsEnabled())
			{
				// Get a com_skin2 interface
				IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

				// Found com_skin2 ?
				if (physiqueInterface)
				{
					// Skinned
					ok=true;

					// Release the interface
					skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
				}
			}
		}
	}
	return ok;
}

// ***************************************************************************

uint CExportNel::buildSkinning (CMesh::CMeshBuild& buildMesh, const TInodePtrInt& skeletonShape, INode& node)
{
	// Return success
	uint ok=NoError;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASS_ID);

	// Build a the name array
	buildMesh.BonesNames.resize (skeletonShape.size());
	TInodePtrInt::const_iterator ite=skeletonShape.begin();
	while (ite != skeletonShape.end())
	{
		// Check the id
		nlassert ((uint)ite->second<buildMesh.BonesNames.size());

		// Names
		buildMesh.BonesNames[ite->second] = ite->first->GetName();

		// Next
		ite++;
	}

	// Found it ?
	if (skin)
	{
		// **********    COMSKIN EXPORT    **********

		// Get a com_skin2 interface
		ISkin *comSkinInterface=(ISkin*)skin->GetInterface (SKIN_INTERFACE);

		// Should been controled with isSkin before.
		nlassert (comSkinInterface);

		// Found com_skin2 ?
		if (comSkinInterface)
		{
			// Get local data
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

			// Should been controled with isSkin before.
			nlassert (localData);

			// Found ?
			if (localData)
			{
				// Check same vertices count
				uint vertCount=localData->GetNumPoints();

				// Ctrl we have the same number of vertices in the mesh and in the modifier.
				if (buildMesh.Vertices.size()!=vertCount)
				{
					ok=InvalidSkeleton;
				}
				else
				{
					// If not the same count, return false (perhaps, the modifier is not in the good place in the stack..)
					if (buildMesh.Vertices.size()==vertCount)
					{
						// Rebuild the array
						buildMesh.SkinWeights.resize (vertCount);

						// For each vertex
						for (uint vert=0; vert<vertCount; vert++)
						{
							// Get bones count for this vertex
							uint boneCount=localData->GetNumAssignedBones (vert);

							// No bones, can't export
							if (boneCount==0)
							{
								// Error
								ok=VertexWithoutWeight;
								break;
							}

							// A map of float / string
							std::multimap<float, uint> weightMap;

							// For each bones
							for (uint bone=0; bone<boneCount; bone++)
							{
								// Get the bone weight
								float weight=localData->GetBoneWeight (vert, bone);

								// Get bone number
								uint boneId=localData->GetAssignedBone (vert, bone);

								// Insert in the map
								weightMap.insert (std::map<float, uint>::value_type (weight, boneId));
							}

							// Keep only the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							while (weightMap.size()>NL3D_MESH_SKINNING_MAX_MATRIX)
							{
								// Remove the lowest weights
								weightMap.erase (weightMap.begin());
							}

							// Sum the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							float sum=0.f;
							std::map<float, uint>::iterator ite=weightMap.begin();
							while (ite!=weightMap.end())
							{
								// Add to the sum
								sum+=ite->first;
								
								// Next value
								ite++;
							}

							// Erase bones
							for (uint i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
							{
								// Erase
								buildMesh.SkinWeights[vert].MatrixId[i]=0;
								buildMesh.SkinWeights[vert].Weights[i]=0;
							}

							// For each bones in the list, build the skin information
							uint id=0;
							ite=weightMap.end();
							while (ite!=weightMap.begin())
							{
								// Previous value
								ite--;
								
								// Get the bones ID
								sint32 matrixId=-1;
								TInodePtrInt::const_iterator itId=skeletonShape.find (comSkinInterface->GetBone(ite->second));
								if (itId!=skeletonShape.end())
									matrixId=itId->second;

								// Find the bone ?
								if (matrixId==-1)
								{
									// no, error, wrong skeleton
									ok=InvalidSkeleton;
									break;
								}

								// Set the weight
								buildMesh.SkinWeights[vert].MatrixId[id]=matrixId;
								buildMesh.SkinWeights[vert].Weights[id]=ite->first/sum;
								
								// Next Id
								id++;
							}
							// Breaked ?
							if (ite!=weightMap.begin())
							{
								// break again to exit
								break;
							}
						}
					}
				}
			}
		}

		// Release the interface
		skin->ReleaseInterface (SKIN_INTERFACE, comSkinInterface);
	}
	else
	{
		// **********    PHYSIQUE EXPORT    **********

		// Physique mode
		Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Must exist
		nlassert (skin);
		
		// Get a com_skin2 interface
		IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

		// Should been controled with isSkin before.
		nlassert (physiqueInterface);

		// Found com_skin2 ?
		if (physiqueInterface)
		{
			// Get local data
			IPhyContextExport *localData=physiqueInterface->GetContextInterface(&node);

			// Should been controled with isSkin before.
			nlassert (localData);

			// Found ?
			if (localData)
			{
				// Use rigid export
				localData->ConvertToRigid (TRUE);

				// Allow blending
				localData->AllowBlending (TRUE);

				// Check same vertices count
				uint vertCount=localData->GetNumberVertices();

				// Ctrl we have the same number of vertices in the mesh and in the modifier.
				if (buildMesh.Vertices.size()!=vertCount)
				{
					ok=InvalidSkeleton;
				}
				else
				{
					// If not the same count, return false (perhaps, the modifier is not in the good place in the stack..)
					if (buildMesh.Vertices.size()==vertCount)
					{
						// Rebuild the array
						buildMesh.SkinWeights.resize (vertCount);

						// For each vertex
						for (uint vert=0; vert<vertCount; vert++)
						{
							// Get a vertex interface
							IPhyVertexExport *vertexInterface=localData->GetVertexInterface (vert);

							// Check if it is a rigid vertex or a blended vertex
							IPhyRigidVertex			*rigidInterface=NULL;
							IPhyBlendedRigidVertex	*blendedInterface=NULL;
							int type=vertexInterface->GetVertexType ();
							if (type==RIGID_TYPE)
							{
								// this is a rigid vertex
								rigidInterface=(IPhyRigidVertex*)vertexInterface;
							}
							else
							{
								// It must be a blendable vertex
								nlassert (type==RIGID_BLENDED_TYPE);
								blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;
							}

							// Get bones count for this vertex
							uint boneCount;
							if (blendedInterface)
							{
								// If blenvertex, only one bone
								boneCount=blendedInterface->GetNumberNodes();
							}
							else
							{
								// If rigid vertex, only one bone
								boneCount=1;
							}

							// No bones, can't export
							if (boneCount==0)
							{
								// Error
								ok=VertexWithoutWeight;
								break;
							}

							// A map of float / string
							std::multimap<float, INode*> weightMap;

							// For each bones
							for (uint bone=0; bone<boneCount; bone++)
							{
								if (blendedInterface)
								{
									// Get the bone weight
									float weight=blendedInterface->GetWeight(bone);

									// Get node
									INode *node=blendedInterface->GetNode(bone);
									nlassert (node);

									// Insert in the map
									weightMap.insert (std::map<float, INode*>::value_type (weight, node));
								}
								else
								{
									// Get node
									INode *node=rigidInterface->GetNode();
									nlassert (node);

									// Insert in the map
									weightMap.insert (std::map<float, INode*>::value_type (1, node));
								}
							}

							// Keep only the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							while (weightMap.size()>NL3D_MESH_SKINNING_MAX_MATRIX)
							{
								// Remove the lowest weights
								weightMap.erase (weightMap.begin());
							}

							// Sum the NL3D_MESH_SKINNING_MAX_MATRIX highest bones
							float sum=0.f;
							std::map<float, INode*>::iterator ite=weightMap.begin();
							while (ite!=weightMap.end())
							{
								// Add to the sum
								sum+=ite->first;
								
								// Next value
								ite++;
							}

							// Erase bones
							for (uint i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
							{
								// Erase
								buildMesh.SkinWeights[vert].MatrixId[i]=0;
								buildMesh.SkinWeights[vert].Weights[i]=0;
							}

							// For each bones in the list, build the skin information
							uint id=0;
							ite=weightMap.end();
							while (ite!=weightMap.begin())
							{
								// Previous value
								ite--;

								// Get the bones ID
								sint32 matrixId=-1;
								TInodePtrInt::const_iterator itId=skeletonShape.find (ite->second);
								if (itId!=skeletonShape.end())
									matrixId=itId->second;

								// Find the bone ?
								if (matrixId==-1)
								{
									// no, error, wrong skeleton
									ok=InvalidSkeleton;
									break;
								}

								// Set the weight
								buildMesh.SkinWeights[vert].MatrixId[id]=matrixId;
								buildMesh.SkinWeights[vert].Weights[id]=ite->first/sum;
								
								// Next Id
								id++;
							}
							
							// Breaked ?
							if (ite!=weightMap.begin())
							{
								// break again to exit
								break;
							}

							// Release vertex interfaces
							localData->ReleaseVertexInterface (vertexInterface);
						}
					}
				}

				// Release locaData interface
				physiqueInterface->ReleaseContextInterface (localData);
			}

		}

		// Release the interface
		skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
	}

	return ok;
}

// ***************************************************************************

INode *getRoot (INode *pNode)
{
	INode* parent=pNode->GetParentNode();
	if (parent)
	{
		if (parent->IsRootNode())
			return pNode;
		else
			return getRoot (parent);
	}
	else 
		return NULL;
}

// ***************************************************************************

INode* CExportNel::getSkeletonRootBone (INode& node)
{
	// Return node
	INode* ret=NULL;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASS_ID);

	// Found it ?
	if (skin)
	{
		// Get a com_skin2 interface
		ISkin *comSkinInterface=(ISkin*)skin->GetInterface (SKIN_INTERFACE);

		// Found com_skin2 ?
		if (comSkinInterface)
		{
			// Get local data
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

			// Found ?
			if (localData)
			{
				// Look for a bone...

				// For each vertices
				for (uint vtx=0; vtx<(uint)localData->GetNumPoints(); vtx++)
				{
					// For each bones
					uint bone;
					for (bone=0; bone<(uint)localData->GetNumAssignedBones (vtx); bone++)
					{
						// Get the bone pointer
						INode *newBone=comSkinInterface->GetBone(localData->GetAssignedBone(vtx, bone));

						// Get the root of the hierarchy
						ret=getRoot (newBone);
						break;
					}
					// Rebreak
					if (bone!=(uint)localData->GetNumAssignedBones (vtx))
						break;
				}
			}

			// Release the interface
			skin->ReleaseInterface (SKIN_INTERFACE, comSkinInterface);
		}
	}
	else
	{
		// Get the skin modifier
		skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Found it ?
		if (skin)
		{
			// Get a com_skin2 interface
			IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

			// Found com_skin2 ?
			if (physiqueInterface)
			{
				// Get local data
				IPhyContextExport *localData=physiqueInterface->GetContextInterface(&node);

				// Found ?
				if (localData)
				{
					// Use rigid export
					localData->ConvertToRigid (TRUE);

					// Allow blending
					localData->AllowBlending (TRUE);

					// Look for a bone...

					// For each vertices
					uint numVert=(uint)localData->GetNumberVertices();
					for (uint vtx=0; vtx<numVert; vtx++)
					{
						bool found=false;

						// Get a vertex interface
						IPhyVertexExport *vertexInterface=localData->GetVertexInterface (vtx);

						// Check if it is a rigid vertex or a blended vertex
						int type=vertexInterface->GetVertexType ();
						if (type==RIGID_TYPE)
						{
							// this is a rigid vertex
							IPhyRigidVertex			*rigidInterface=(IPhyRigidVertex*)vertexInterface;

							// Get the bone
							INode *newBone=rigidInterface->GetNode();

							// Get the root of the hierarchy
							ret=getRoot (newBone);
							found=true;
							break;
						}
						else
						{
							// It must be a blendable vertex
							nlassert (type==RIGID_BLENDED_TYPE);
							IPhyBlendedRigidVertex	*blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;

							// For each bones
							uint bone;
							uint count=(uint)blendedInterface->GetNumberNodes ();
							for (bone=0; bone<count; bone++)
							{
								// Get the bone pointer
								INode *newBone=blendedInterface->GetNode(bone);

								// Get the root of the hierarchy
								ret=getRoot (newBone);
								found=true;
								break;
							}
						}

						// Release vertex interfaces
						localData->ReleaseVertexInterface (vertexInterface);

						// Rebreak
						if (found)
							break;

						// Release vertex interfaces
						localData->ReleaseVertexInterface (vertexInterface);
					}

					// Release locaData interface
					physiqueInterface->ReleaseContextInterface (localData);
				}

				// Release the interface
				skin->ReleaseInterface (I_PHYINTERFACE, physiqueInterface);
			}
		}
	}

	// Return result;
	return ret;
}

// ***************************************************************************

void CExportNel::addSkeletonBindPos (INode& skinedNode, mapBoneBindPos& boneBindPos)
{
	// Return success
	uint ok=NoError;

	// Get the skin modifier
	Modifier* skin=getModifier (&skinedNode, SKIN_CLASS_ID);

	// Found it ?
	if (skin)
	{
		// Get a com_skin2 interface
		ISkin *comSkinInterface=(ISkin*)skin->GetInterface (SKIN_INTERFACE);

		// Should been controled with isSkin before.
		nlassert (comSkinInterface);

		// Found com_skin2 ?
		if (comSkinInterface)
		{
			// Get local data
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&skinedNode);

			// Should been controled with isSkin before.
			nlassert (localData);

			// Found ?
			if (localData)
			{
				// Check same vertices count
				uint vertCount=localData->GetNumPoints();

				// For each vertex
				for (uint vert=0; vert<vertCount; vert++)
				{
					// Get bones count for this vertex
					uint boneCount=localData->GetNumAssignedBones (vert);

					// For each bones
					for (uint bone=0; bone<boneCount; bone++)
					{
						// Get the bone id
						int boneId=localData->GetAssignedBone(vert, bone);

						// Get bone INode*
						INode *bone=comSkinInterface->GetBone(boneId);

						// Get the bind matrix of the bone
						Matrix3 bindPos;
						comSkinInterface->GetBoneInitTM(bone, bindPos);

						// Add an entry inthe map
						boneBindPos.insert (mapBoneBindPos::value_type (bone, bindPos));
					}
				}
			}

			// Release the interface
			skin->ReleaseInterface (SKIN_INTERFACE, comSkinInterface);
		}
	}
	else
	{
		// Get the skin modifier
		Modifier* skin=getModifier (&skinedNode, PHYSIQUE_CLASS_ID);

		// Should been controled with isSkin before.
		nlassert (skin);

		// Found it ?
		if (skin)
		{
			// Get a com_skin2 interface
			IPhysiqueExport *physiqueInterface=(IPhysiqueExport *)skin->GetInterface (I_PHYINTERFACE);

			// Should been controled with isSkin before.
			nlassert (physiqueInterface);

			// Found com_skin2 ?
			if (physiqueInterface)
			{
				// Get local data
				IPhyContextExport *localData=physiqueInterface->GetContextInterface(&skinedNode);

				// Should been controled with isSkin before.
				nlassert (localData);

				// Found ?
				if (localData)
				{
					// Use rigid export
					localData->ConvertToRigid (TRUE);

					// Allow blending
					localData->AllowBlending (TRUE);

					// Check same vertices count
					uint vertCount=localData->GetNumberVertices();

					// For each vertex
					for (uint vert=0; vert<vertCount; vert++)
					{
						if (vert==111)
							int toto=0;
						// Get a vertex interface
						IPhyVertexExport *vertexInterface=localData->GetVertexInterface (vert);

						// Check if it is a rigid vertex or a blended vertex
						int type=vertexInterface->GetVertexType ();
						if (type==RIGID_TYPE)
						{
							// this is a rigid vertex
							IPhyRigidVertex			*rigidInterface=(IPhyRigidVertex*)vertexInterface;

							// Get bone INode*
							INode *bone=rigidInterface->GetNode();

							// Get the bind matrix of the bone
							Matrix3 bindPos;
							int res=physiqueInterface->GetInitNodeTM (bone, bindPos);
							nlassert (res==MATRIX_RETURNED);

							// Add an entry inthe map
							if (boneBindPos.insert (mapBoneBindPos::value_type (bone, bindPos)).second)
							{
#ifdef NL_DEBUG
								// *** Debug info

								// Bone name
								std::string boneName=getName (*bone);

								// Local matrix
								Matrix3 nodeTM;
								nodeTM=bone->GetNodeTM (0);

								// Offset matrix
								Matrix3 offsetScaleTM (TRUE);
								Matrix3 offsetRotTM (TRUE);
								Matrix3 offsetPosTM (TRUE);
								ApplyScaling (offsetScaleTM, bone->GetObjOffsetScale ());
								offsetRotTM.SetRotate (bone->GetObjOffsetRot ());
								offsetPosTM.SetTrans (bone->GetObjOffsetPos ());
								Matrix3 offsetTM = offsetScaleTM * offsetRotTM * offsetPosTM;

								// Local + offset matrix
								Matrix3 nodeOffsetTM = offsetTM * nodeTM;

								// Init TM
								Matrix3 initTM;
								int res=physiqueInterface->GetInitNodeTM (bone, initTM);
								nlassert (res==MATRIX_RETURNED);

								// invert
								initTM.Invert();
								Matrix3 compNode=nodeTM*initTM;
								Matrix3 compOffsetNode=nodeOffsetTM*initTM;
								Matrix3 compOffsetNode2=nodeOffsetTM*initTM;
#endif // NL_DEBUG
							}
						}
						else
						{
							// It must be a blendable vertex
							nlassert (type==RIGID_BLENDED_TYPE);
							IPhyBlendedRigidVertex	*blendedInterface=(IPhyBlendedRigidVertex*)vertexInterface;

							// For each bones
							uint boneIndex;
							uint count=(uint)blendedInterface->GetNumberNodes ();
							for (boneIndex=0; boneIndex<count; boneIndex++)
							{
								// Get the bone pointer
								INode *bone=blendedInterface->GetNode(boneIndex);

								// Get the bind matrix of the bone
								Matrix3 bindPos;
								int res=physiqueInterface->GetInitNodeTM (bone, bindPos);
								nlassert (res==MATRIX_RETURNED);

								// Add an entry inthe map
								if (boneBindPos.insert (mapBoneBindPos::value_type (bone, bindPos)).second)
								{
#ifdef NL_DEBUG
									// *** Debug info

									// Bone name
									std::string boneName=getName (*bone);

									// Local matrix
									Matrix3 nodeTM;
									nodeTM=bone->GetNodeTM (0);

									// Offset matrix
									Matrix3 offsetScaleTM (TRUE);
									Matrix3 offsetRotTM (TRUE);
									Matrix3 offsetPosTM (TRUE);
									ApplyScaling (offsetScaleTM, bone->GetObjOffsetScale ());
									offsetRotTM.SetRotate (bone->GetObjOffsetRot ());
									offsetPosTM.SetTrans (bone->GetObjOffsetPos ());
									Matrix3 offsetTM = offsetScaleTM * offsetRotTM * offsetPosTM;

									// Local + offset matrix
									Matrix3 nodeOffsetTM = offsetTM * nodeTM;

									// Init TM
									Matrix3 initTM;
									int res=physiqueInterface->GetInitNodeTM (bone, initTM);
									nlassert (res==MATRIX_RETURNED);

									// invert
									initTM.Invert();
									Matrix3 compNode=nodeTM*initTM;
									Matrix3 compOffsetNode=nodeOffsetTM*initTM;
									Matrix3 compOffsetNode2=nodeOffsetTM*initTM;
#endif // NL_DEBUG
								}
							}
						}
					
						// Release vertex interfaces
						localData->ReleaseVertexInterface (vertexInterface);
					}

					// Release locaData interface
					physiqueInterface->ReleaseContextInterface (localData);
				}

				// Release the interface
				skin->ReleaseInterface (SKIN_INTERFACE, physiqueInterface);
			}
		}
	}
}

// ***************************************************************************


void CExportNel::enableSkinModifier (INode& node, bool enable)
{
	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASS_ID);

	// Found it ?
	if (skin)
	{
		// Enable ?
		if (enable)
			skin->EnableMod ();
		else
			skin->DisableMod ();
	}
	else
	{
		// Get the physique modifier
		Modifier* skin=getModifier (&node, PHYSIQUE_CLASS_ID);

		// Found it ?
		if (skin)
		{
			// Enable ?
			if (enable)
				skin->EnableMod ();
			else
				skin->DisableMod ();
		}
	}
}

// ***************************************************************************

