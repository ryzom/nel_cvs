/** \file export_skinning.cpp
 * Export skinning from 3dsmax to NeL. Works only with the com_skin2 plugin.
 *
 * $Id: export_skinning.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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
#include "nel/3d/skeleton_shape.h"

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

#define SKIN_INTERFACE 0x00010000

// ***************************************************************************

#define SKIN_CLASS_ID Class_ID(9815843,87654)

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

void CExportNel::buildSkeletonShape (CSkeletonShape& skeletonShape, INode& node, mapBoneBindPos* mapBindPos, TimeValue time)
{
	// Build a bone vector
	std::vector<CBoneBase> bonesArray;

	// Counter
	sint32 idCount=0;

	// Parse the tree
	buildSkeleton (bonesArray, node, mapBindPos, time, idCount);

	// Then build the object
	skeletonShape.build (bonesArray);
}

// ***************************************************************************

void CExportNel::buildSkeleton (std::vector<CBoneBase>& bonesArray, INode& node, mapBoneBindPos* mapBindPos, TimeValue time, sint32& idCount, sint32 father)
{
	// **** Save the current the id
	int id=idCount;

	// **** Create a bone
	CBoneBase bone;

	// ** Set Name and id

	// Bone name
	bone.Name=getName (node);

	// Id of the father
	bone.FatherId=father;

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
		bone.UnheritScale=(flags&(INHERIT_SCL_X|INHERIT_SCL_Y|INHERIT_SCL_Z))==0;
	}

	// ** Set default tracks

	// Get the local matrix for the default pos
	Matrix3 localTM;
	getLocalMatrix (localTM, node, time);

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
		buildSkeleton (bonesArray, *node.GetChildNode(children), mapBindPos, time, ++idCount, id);
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
		if (skin->IsEnabled())
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
	return ok;
}

// ***************************************************************************

uint CExportNel::buildSkinning (CMesh::CMeshBuild& buildMesh, const CSkeletonShape& skeletonShape, INode& node)
{
	// Return success
	uint ok=NoError;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASS_ID);

	// Should been controled with isSkin before.
	nlassert (skin);

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
					// Rebuild the array
					buildMesh.SkinWeights.resize (vertCount);

					// If not the same count, return false (perhaps, the modifier is not in the good place in the stack..)
					if (buildMesh.Vertices.size()==vertCount)
					{
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

								// Get the bone name
								std::string name=getName (*comSkinInterface->GetBone(ite->second));
								
								// Get the bones ID
								sint32 matrixId=skeletonShape.getBoneIdByName (name);

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

			// Release the interface
			skin->ReleaseInterface (SKIN_INTERFACE, comSkinInterface);
		}
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

	// Return result;
	return ret;
}

// ***************************************************************************

void CExportNel::addSkeletonBindPos (INode& node, mapBoneBindPos& boneBindPos)
{
	// Return success
	uint ok=NoError;

	// Get the skin modifier
	Modifier* skin=getModifier (&node, SKIN_CLASS_ID);

	// Should been controled with isSkin before.
	nlassert (skin);

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
			ISkinContextData *localData=comSkinInterface->GetContextInterface(&node);

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
}

// ***************************************************************************

