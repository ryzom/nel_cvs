/** \file export_mesh.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_mesh.cpp,v 1.14 2001/08/07 14:23:02 vizerie Exp $
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
#include "export_lod.h"

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"
#include "../nel_export/nel_export_scene.h"


#include <3d/texture_file.h>
#include <3d/mesh_mrm.h>
#include <3d/mesh_multi_lod.h>
#include <3d/particle_system_model.h>
#include <3d/particle_system_shape.h>
#include <3d/coarse_mesh_manager.h>
#include <3d/flare_shape.h>

using namespace NLMISC;
using namespace NL3D;

#define NEL_OBJET_NAME_DATA 1970


// ***************************************************************************


void buildNeLMatrix (CMatrix& tm, const CVector& scale, const CQuat& rot, const CVector& pos)
{
	CMatrix scaleTM, rotTM, posTM;
	scaleTM.identity();
	scaleTM.scale (scale);
	rotTM.identity();
	rotTM.setRot (rot);
	posTM.identity();
	posTM.setPos (pos);
	tm=posTM*rotTM*scaleTM;
}

// ***************************************************************************


CMesh::CMeshBuild*	CExportNel::createMeshBuild(INode& node, TimeValue tvTime, bool bAbsPath, CMesh::CMeshBaseBuild*& baseBuild)
{
	CMesh::CMeshBuild *pMeshBuild = new CMesh::CMeshBuild();
	baseBuild = new CMeshBase::CMeshBaseBuild();

	// Get a pointer on the object's node
    Object *obj = node.EvalWorldState(tvTime).obj;

	// Check if there is an object
	if (obj)
	{		

		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != tri) 
				deleteIt = true;

			// Description of materials
			CMaxMeshBaseBuild maxBaseBuild;

			// Fill the build interface of CMesh

			// Reset the material array of the buildMesh because it will be rebuild by the exporter
			baseBuild->Materials.clear();

			buildBaseMeshInterface (*baseBuild, maxBaseBuild, node, tvTime, bAbsPath);
			buildMeshInterface (*tri, *pMeshBuild, maxBaseBuild, node, tvTime, NULL, bAbsPath);

			// Delete the triObject if we should...
			if (deleteIt)
				delete tri;
		}
	}

	// Return the shape pointer or NULL if an error occured.
	return pMeshBuild;
}

// ***************************************************************************

// Export a mesh
IShape* CExportNel::buildShape (INode& node, Interface& ip, TimeValue time, 
								const CSkeletonShape* skeletonShape, bool absolutePath,
								CExportNelOptions &opt, bool view)
{

	// Here, we must check what kind of node we can build with this mesh.
	// For the time, just Triobj is supported.
	IShape *retShape=NULL;

	// If skinning, disable skin modifier
	if (skeletonShape)
		enableSkinModifier (node, false);

	// Get a pointer on the object's node
    Object *obj = node.EvalWorldState(time).obj;

	// Check if there is an object
	if (obj)
	{
		Class_ID  clid = obj->ClassID() ;
		// is the object a particle system ? (we do this defore meshs, because for now there is a mesh in max scenes to say where a particle system is...)
		if (clid.PartA() == NEL_PARTICLE_SYSTEM_CLASS_ID)
		{
			// build the shape from the file name
			AppDataChunk *ad = obj->GetAppDataChunk(MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL_OBJET_NAME_DATA );
			if (ad&&ad->data)
			{											
				NL3D::CShapeStream ss ;
				NLMISC::CIFile iF ;
				if (iF.open((const char *) ad->data))
				{
					iF.serial(ss) ;

					CParticleSystemShape *pss = NLMISC::safe_cast<CParticleSystemShape *>(ss.getShapePointer()) ;

					// ********************************
					// *** Export default transformation
					// ********************************

						// Get the node matrix
						Matrix3 localTM;
						getLocalMatrix (localTM, node, time);

						// Get the translation, rotation, scale of the node
						CVector pos, scale;
						CQuat rot;
						decompMatrix (scale, rot, pos, localTM);

						// Set the default values
						pss->getDefaultPos()->setValue(pos) ;					
						pss->getDefaultScale()->setValue(scale) ;					
						pss->getDefaultRotQuat()->setValue(rot) ;												
						return pss ;
				}
				else
				{
					throw NLMISC::EStream("file not found (particle system file)") ;
				}				
			}
		}

		// is the object a flare ?
		if (clid.PartA() == NEL_FLARE_CLASS_ID_A /*&& clid.PartB() == NEL_FLARE_CLASS_ID_B*/)
		{
			// build the shape
			CFlareShape *fs = new CFlareShape ;

			Point3 col ;
			float persistence, size, spacing, attenuationRange, pos, maxViewDistRatio ;
			int   attenuable ;
			int	  firstFlareKeepSize ;
			int   hasDazzle ;


			// retrieve the color of the flare from the node
			CExportNel::getValueByNameUsingParamBlock2(node, "ColorParam", (ParamType2)TYPE_RGBA, &col, 0) ;
			fs->setColor(NLMISC::CRGBA((uint) (255.f * col.x), (uint) (255.f * col.y), (uint) (255.f * col.z))) ;
			// retrieve the persistence of the flare
			CExportNel::getValueByNameUsingParamBlock2(node, "PersistenceParam", (ParamType2)TYPE_FLOAT, &persistence, 0) ;
			fs->setPersistence(persistence) ;
			// retrieve spacing of the flare
			CExportNel::getValueByNameUsingParamBlock2(node, "Spacing", (ParamType2)TYPE_FLOAT, &spacing, 0) ;
			fs->setFlareSpacing(spacing) ;
			// retrieve use of radial attenuation
			CExportNel::getValueByNameUsingParamBlock2(node, "Attenuable", (ParamType2) TYPE_BOOL, &attenuable, 0) ;			
			if (attenuable)
			{
				fs->setAttenuable() ;
				CExportNel::getValueByNameUsingParamBlock2(node, "AttenuationRange", (ParamType2) TYPE_FLOAT, &attenuationRange, 0) ;			
				fs->setAttenuationRange(attenuationRange) ;
			}			
			CExportNel::getValueByNameUsingParamBlock2(node, "FirstFlareKeepSize", (ParamType2) TYPE_BOOL, &firstFlareKeepSize, 0) ;			
			fs->setFirstFlareKeepSize(firstFlareKeepSize ? true : false) ; // avoid VC++ warning

			/// check for dazzle
			CExportNel::getValueByNameUsingParamBlock2(node, "HasDazzle", (ParamType2) TYPE_BOOL, &hasDazzle, 0) ;			

			if (hasDazzle)
			{
				fs->enableDazzle() ;
				// get dazzle color
				CExportNel::getValueByNameUsingParamBlock2(node, "DazzleColor", (ParamType2) TYPE_RGBA, &col, 0) ;			
				fs->setDazzleColor(NLMISC::CRGBA((uint) (255.f * col.x), (uint) (255.f * col.y), (uint) (255.f * col.z))) ;
				// get dazzle attenuation range
				CExportNel::getValueByNameUsingParamBlock2(node, "DazzleAttenuationRange", (ParamType2) TYPE_FLOAT, &attenuationRange, 0) ;			
				fs->setDazzleAttenuationRange(attenuationRange) ;
			}

			/// retrieve maxViewDistRatio
			CExportNel::getValueByNameUsingParamBlock2(node, "MaxViewDistRatio", (ParamType2) TYPE_BOOL, &maxViewDistRatio, 0) ;
			fs->setMaxViewDistRatio(maxViewDistRatio) ;



			// retrieve sizes & tex
			for (uint k = 0 ; k < MaxFlareNum ; ++k)
			{
				char out[16] ; sprintf(out, "size%d", k) ;
				CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2)TYPE_FLOAT, &size, 0) ;
				fs->setSize(k, size) ;
				// get relative position
				sprintf(out, "pos%d", k) ;
				CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2)TYPE_FLOAT, &pos, 0) ;
				fs->setRelativePos(k, pos) ;

				// check wether the flare is used
				int texUsed ;
				sprintf(out, "flareUsed%d", k) ;
				CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2) TYPE_BOOL, &texUsed, 0) ;			
				if (texUsed)
				{
					sprintf(out, "texFileName%d", k) ;
					// retrieve the texture name
					std::string fileName ;
					CExportNel::getValueByNameUsingParamBlock2(node, out, (ParamType2) TYPE_STRING, &fileName, 0) ;
					if (absolutePath)
					{
						fs->setTexture(k, new NL3D::CTextureFile(fileName.c_str())) ;
					}
					else
					{
						char fName[_MAX_FNAME];
						// get file name only
						::_splitpath(NULL, NULL, NULL, fName, NULL) ;
						fs->setTexture(k, new NL3D::CTextureFile(fName)) ;
					}
				}
				else
				{
					fs->setTexture(k, NULL) ;
				}	
			}

			// Get the node matrix
			Matrix3 localTM;
			getLocalMatrix (localTM, node, time);			


			Point3  fp = localTM.GetTrans() ;

			// export default transformation
			fs->getDefaultPos()->setValue( CVector(fp.x, fp.y, fp.z) ) ;					
			

			return fs ;
		}


		

		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != tri) 
				deleteIt = true;

			/// \toto hulud: here, check if it is another kind of shape than mesh, multi lod mesh or mrm mesh. 
			{
				// Mesh base ?
				CMeshBase *meshBase;

				// Is a multi lod object ?
				uint lodCount=getScriptAppData (&node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
				if (lodCount)
				{
					// Listy of material names
					std::vector<std::string> listMaterialName;

					// Make the root mesh
					CMeshMultiLod::CMeshMultiLodBuild multiLodBuild;
					multiLodBuild.LodMeshes.reserve (lodCount+1);

					// Resize to one
					bool isTransparent;
					bool isOpaque;
					multiLodBuild.LodMeshes.resize (1);
					multiLodBuild.LodMeshes[0].MeshGeom=buildMeshGeom (node, ip, time, skeletonShape, absolutePath, opt, multiLodBuild.BaseMesh, 
						listMaterialName, isTransparent, isOpaque, CMatrix::Identity, view);
					multiLodBuild.LodMeshes[0].DistMax=getScriptAppData (&node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
					multiLodBuild.LodMeshes[0].BlendLength=getScriptAppData (&node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
					multiLodBuild.LodMeshes[0].Flags=0;
					if (getScriptAppData (&node, NEL3D_APPDATA_LOD_BLEND_IN, NEL3D_APPDATA_LOD_BLEND_IN_DEFAULT))
						multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendIn;
					if (getScriptAppData (&node, NEL3D_APPDATA_LOD_BLEND_OUT, NEL3D_APPDATA_LOD_BLEND_OUT_DEFAULT))
						multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendOut;
					if ((getScriptAppData (&node, NEL3D_APPDATA_LOD_COARSE_MESH, NEL3D_APPDATA_LOD_COARSE_MESH_DEFAULT)) && (!view))
						multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::CoarseMesh;
					if (isTransparent)
						multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsTransparent;
					if (isOpaque)
						multiLodBuild.LodMeshes[0].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsOpaque;
					multiLodBuild.StaticLod=getScriptAppData (&node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, NEL3D_APPDATA_LOD_DYNAMIC_MESH_DEFAULT)==0;

					// Bacup scale, rot and pos
					CVector backupedScale=multiLodBuild.BaseMesh.DefaultScale;
					CQuat	backupedRot=multiLodBuild.BaseMesh.DefaultRotQuat;
					CVector backupedPos=multiLodBuild.BaseMesh.DefaultPos;

					// Build a world to local matrix
					CMatrix worldToNodeMatrix;
					buildNeLMatrix (worldToNodeMatrix, backupedScale, backupedRot, backupedPos); 
					worldToNodeMatrix.invert ();

					// For all the other lods
					for (uint lod=0; lod<lodCount; lod++)
					{
						// Get the name
						std::string nodeName=getScriptAppData (&node, NEL3D_APPDATA_LOD_NAME+lod, "");

						// Get the node
						INode *lodNode=ip.GetINodeByName(nodeName.c_str());
						if (lodNode)
						{
							// Index of the lod in the build structure
							uint index=multiLodBuild.LodMeshes.size();

							// Resize the build structure
							multiLodBuild.LodMeshes.resize (index+1);

							// Get matrix node
							CMatrix nodeTM;
							convertMatrix (nodeTM, lodNode->GetNodeTM (time));

							// Fill the structure
							multiLodBuild.LodMeshes[index].MeshGeom=buildMeshGeom (*lodNode, ip, time, skeletonShape, absolutePath, opt, multiLodBuild.BaseMesh, 
								listMaterialName, isTransparent, isOpaque, worldToNodeMatrix*nodeTM, view);
							multiLodBuild.LodMeshes[index].DistMax=getScriptAppData (lodNode, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
							multiLodBuild.LodMeshes[index].BlendLength=getScriptAppData (lodNode, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
							multiLodBuild.LodMeshes[index].Flags=0;
							if (getScriptAppData (lodNode, NEL3D_APPDATA_LOD_BLEND_IN, NEL3D_APPDATA_LOD_BLEND_IN_DEFAULT))
								multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendIn;
							if (getScriptAppData (lodNode, NEL3D_APPDATA_LOD_BLEND_OUT, NEL3D_APPDATA_LOD_BLEND_OUT_DEFAULT))
								multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::BlendOut;
							if ((getScriptAppData (lodNode, NEL3D_APPDATA_LOD_COARSE_MESH, NEL3D_APPDATA_LOD_COARSE_MESH_DEFAULT)) && (!view))
								multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::CoarseMesh;
							if (isTransparent)
								multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsTransparent;
							if (isOpaque)
								multiLodBuild.LodMeshes[index].Flags|=CMeshMultiLod::CMeshMultiLodBuild::CBuildSlot::IsOpaque;
						}
					}

					// Restaure default pos, scale and rot
					multiLodBuild.BaseMesh.DefaultScale=backupedScale;
					multiLodBuild.BaseMesh.DefaultRotQuat=backupedRot;
					multiLodBuild.BaseMesh.DefaultPos=backupedPos;

					// Make a CMeshMultiLod mesh object
					CMeshMultiLod* multiMesh=new CMeshMultiLod;

					// Build it
					multiMesh->build (multiLodBuild);

					// Return this pointer
					meshBase=multiMesh;

					// ** force material to be animatable
					/// todo hulud: check if material are animated before
					for (uint i=0; i<listMaterialName.size(); i++)
					{
						meshBase->setAnimatedMaterial (i, listMaterialName[i]);
					}
				}
				else
				{
					// Array of name for the material
					CMaxMeshBaseBuild maxBaseBuild;

					// Fill the build interface of CMesh
					CMeshBase::CMeshBaseBuild buildBaseMesh;
					buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, absolutePath);

					CMesh::CMeshBuild buildMesh;
					buildMeshInterface (*tri, buildMesh, maxBaseBuild, node, time, skeletonShape, absolutePath);

					if( hasLightMap( node, time ) && opt.bExportLighting )
						calculateLM(&buildMesh, &buildBaseMesh, node, ip, time, absolutePath, opt);

					// MRM mesh ?
					if (getScriptAppData (&node, NEL3D_APPDATA_LOD_MRM, 0))
					{
						// Build a MRM parameters block
						CMRMParameters	parameters;
						buildMRMParameters (node, parameters);

						// Make a CMesh object
						CMeshMRM* meshMRM=new CMeshMRM;

						// Build the mesh with the build interface
						meshMRM->build (buildBaseMesh, buildMesh, parameters);

						// Return this pointer
						meshBase=meshMRM;
					}
					else
					{
						// Make a CMesh object
						CMesh* mesh=new CMesh;

						// Build the mesh with the build interface
						mesh->build (buildBaseMesh, buildMesh);

						// Return this pointer
						meshBase=mesh;
					}

					// ** force material to be animatable
					/// todo hulud: check if material are animated before
					for (uint i=0; i<maxBaseBuild.NumMaterials; i++)
					{
						meshBase->setAnimatedMaterial (i, maxBaseBuild.MaterialNames[i]);
					}
				}

				// Return the mesh base
				retShape=meshBase;
			}

			// Delete the triObject if we should...
			if (deleteIt)
				delete tri;
		}
	}

	// If skinning, renable skin modifier
	if (skeletonShape)
		enableSkinModifier (node, true);

	// Return the shape pointer or NULL if an error occured.
	return retShape;
}

// ***************************************************************************

// Get the normal of a face for a given corner in localSpace
Point3 CExportNel::getLocalNormal (int face, int corner, Mesh& mesh)
{
	// Vertex number
	int nVertex=mesh.faces[face].v[corner];

	// Pointer on a render vertex
	RVertex *pRVertex=mesh.getRVertPtr(nVertex);

	// Smoothing group of the face
	DWORD nSmoothGroup=mesh.faces[face].smGroup;

	// No group ?
	if (nSmoothGroup==0)
		// Return face normal
		return mesh.getFaceNormal (face);

	// Specified normal
	if(pRVertex->rFlags & SPECIFIED_NORMAL)
	{
		return pRVertex->rn.getNormal();
	}

	// Get the number of normals in this vertex
	int nNumNormal=pRVertex->rFlags & NORCT_MASK;

	// If only on normal, get it
	if (nNumNormal==1)
	{
		return pRVertex->rn.getNormal();
	}
	// Else look for the good one
	else if (nNumNormal>1)
	{
		// Enum other normals and find the one with same smoothing group flags
		for (int nNormal=0; nNormal<nNumNormal; nNormal++)
		{
			// Get the normal's pointer
			RNormal* pNormal=&pRVertex->ern[nNormal];
			if (pNormal->getSmGroup()&nSmoothGroup)
			{
				return pNormal->getNormal();
			}
		}
	}
	
	// Return face normal
	return mesh.getFaceNormal (face);
}

// ***************************************************************************

// Build a base mesh interface
void CExportNel::buildBaseMeshInterface (NL3D::CMeshBase::CMeshBaseBuild& buildMesh, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
										 TimeValue time, bool absolutePath)
{
	buildMesh.bCastShadows = (node.CastShadows() != 0);
	buildMesh.bRcvShadows  = (node.RcvShadows() != 0);

	// *** ****************
	// *** Export materials
	// *** ****************

	// Build materials in NeL format and get the number of materials exported in NeL format
	buildMaterials (buildMesh.Materials, maxBaseBuild, node, time, absolutePath);

	// Some check. should have one rempa vertMap channel table by material
	nlassert (maxBaseBuild.RemapChannel.size()==maxBaseBuild.NumMaterials);

	// *** *****************************
	// *** Export default transformation
	// *** *****************************

	// Get the node matrix
	Matrix3 localTM;
	getLocalMatrix (localTM, node, time);

	// Get the translation, rotation, scale of the node
	CVector pos, scale;
	CQuat rot;
	decompMatrix (scale, rot, pos, localTM);

	// Set the default values
	buildMesh.DefaultScale=scale;
	buildMesh.DefaultRotQuat=rot;
	buildMesh.DefaultRotEuler=CVector (0,0,0);
	buildMesh.DefaultPivot=CVector (0,0,0);
	buildMesh.DefaultPos=pos;

	// Ok, done.
}

// ***************************************************************************

// Build a mesh interface
void CExportNel::buildMeshInterface (TriObject &tri, CMesh::CMeshBuild& buildMesh, const CMaxMeshBaseBuild& maxBaseBuild, INode& node, TimeValue time, 
									 const NL3D::CSkeletonShape* skeletonShape, bool absolutePath, const CMatrix& newBasis)
{
	// Get a pointer on the 3dsmax mesh
	Mesh *pMesh=&tri.mesh;

	// Build normals.
	// * "buildRenderNormals()" smooth faces that are not in the same material but at least in the same smoothing group.
	// * "buildNormals()" smooth faces that are in the same material and at least in the same smoothing group.
	// * I have choose buildRenderNormals (Hulud).
	pMesh->buildRenderNormals();		// I prefer this way to compute normals (Hulud)
	//pMesh->buildNormals();

	// Put default vertex flags. xyz and normals...
	buildMesh.VertexFlags=IDRV_VF_XYZ|IDRV_VF_NORMAL;

	// Is this mesh skined ?
	bool skined=skeletonShape!=NULL;

	// *** ***************************
	// *** Compute the export matrix
	// *** ***************************

	// *** If the mesh is skined, vertices will be exported in world space.
	// *** If the mesh is not skined, vertices will be exported in offset space.

	// Compute matrices: object -> export space and the inverted one.
	CMatrix ToExportSpace;
	CMatrix FromExportSpace;

	if (skined)
	{
		// *** Get an "object to world" matrix

		// Simply go into world space
		Matrix3 tm=node.GetObjectTM(time);
		convertMatrix (ToExportSpace, tm);
		FromExportSpace=ToExportSpace;
		FromExportSpace.invert ();
	}
	else
	{
		// *** Get an "object to local" matrix

		/*
		 * In max, vertices are not in a pre specified basis. We must transform them by an object matrix (ObjectTM)
		 * to get them in the world basis. This matrix can change regarding the pipline flow of max.
		 *
		 * We will call local matrix the 3dsmax node offset matrix.
		 * We will call object matrix the matrix an object needs to be multiplied by to transform it into world space.
		 * We will call transformation matrix the matrix given by the animation of the node.
		 * We will call parent matrix the node matrix of the parent node of this node.
		 * We will call node matrix the complete transformation of a node without the local matrix.
		 *
		 * Our model vertices will be exported in local basis.
		 *
		 * The entire transformation used to transform the points in 3dsmax is: ( parent * transformation * local )
		 *
		 * objectToLocal = ( transformation-1 * parent-1 * object )
		 *
		 * But, parent * transformation = node
		 *
		 * objectToLocal = ( node-1 * object )
		 */

		// Get the invert node matrix
		Matrix3 invNodeTM=node.GetNodeTM(time);
		invNodeTM.Invert();

		// Get the object matrix
		Matrix3 objectTM=node.GetObjectTM(time);

		// Compute the local to world matrix
		Matrix3 objectToLocal=objectTM*invNodeTM;

		// Invert matrix in NeL format
		convertMatrix (ToExportSpace, objectToLocal);
		ToExportSpace=newBasis*ToExportSpace;
		FromExportSpace=ToExportSpace;
		FromExportSpace.invert ();
	}

	// *** *********************
	// *** Look for RGB vertices
	// *** *********************

	// Does the mesh have RGB vertices ? 0 is the color vertex channel.
	if (pMesh->mapSupport (0))
	{
		// Add flag for color vertices
		buildMesh.VertexFlags|=IDRV_VF_COLOR;
	}
	
	
	// *** ***************************
	// *** Build remap uv channel data
	// *** ***************************

	// Get the max number of UVs channel used in the materials
	int nMapsChannelUsed=0;
	for (int nMat=0; nMat<(sint)maxBaseBuild.RemapChannel.size(); nMat++)
	{
		// Greater than previous ?
		if ((sint)maxBaseBuild.RemapChannel[nMat].size()>nMapsChannelUsed)
		{
			nMapsChannelUsed=maxBaseBuild.RemapChannel[nMat].size();
		}
	}

	// Add flags to the mesh vertex format
	for (int nChannels=0; nChannels<nMapsChannelUsed; nChannels++)
	{
		buildMesh.VertexFlags|=IDRV_VF_UV[nChannels];
	}


	// *** ***************
	// *** Export vertices
	// *** ***************

	// Number of vertices
	int nNumVertices=pMesh->getNumVerts();

	// Resize the vertex table
	buildMesh.Vertices.resize (nNumVertices);

	// Export vertex
	for (int vertex=0; vertex<nNumVertices; vertex++)
	{
		// Transforme the vertex in local coordinate
		Point3 v=pMesh->getVert(vertex);
		CVector vv=ToExportSpace*CVector (v.x, v.y, v.z);

		// Build a NeL vertex
		buildMesh.Vertices[vertex]=vv;
	}

	
	// *** ************
	// *** Export faces
	// *** ************

	// Number of faces
	int nNumFaces=pMesh->getNumFaces();
	
	// Resize the vertex table
	buildMesh.Faces.resize (nNumFaces);

	// Export vertex
	for (int face=0; face<nNumFaces; face++)
	{
		// Max face pointer
		Face *pFace=&pMesh->faces[face];


		// *** ******************
		// *** Export material ID
		// *** ******************

		// Material index used by this face
		sint nMaterialID=(sint)pFace->getMatID();

		// Trunc the ID cause max take MatID%MatCount as final matID.
		// So MatId can be > than the number of material at this mod.
		if (maxBaseBuild.NumMaterials>0)
			nMaterialID%=maxBaseBuild.NumMaterials;
		else
			nMaterialID=0;

		// Material offset 
		nMaterialID+=maxBaseBuild.FirstMaterial;

		// Get the material ID
		buildMesh.Faces[face].MaterialId=nMaterialID;

		// Check the matId is valid
		nlassert (buildMesh.Faces[face].MaterialId>=0);
		nlassert (buildMesh.Faces[face].MaterialId<(sint)(maxBaseBuild.FirstMaterial+maxBaseBuild.NumMaterials));

		//if( buildMesh.Materials[buildMesh.Faces[face].MaterialId].getShader() == CMaterial::Specular )
		//	buildMesh.VertexFlags |= IDRV_VF_UV[1];

		// Export the 3 corners
		for (int corner=0; corner<3; corner++)
		{
			// *** *************
			// *** Export vertex
			// *** *************

			// Corner pointer
			CMesh::CCorner *pCorner=&buildMesh.Faces[face].Corner[corner];

			// Vertex index
			pCorner->Vertex=pFace->v[corner];

			// Normal value in local
			Point3 vNormal=getLocalNormal (face, corner, *pMesh);


			// *** *************
			// *** Export normal
			// *** *************

			// *** Transform normal in world coordinate

			// Make a plane with the normal
			CPlane plane (vNormal.x, vNormal.y, vNormal.z, 0.f);

			// Use transformation of the plane to transforme the normal in world coordinate. (plane * M-1)
			plane=plane*FromExportSpace;

			// Store the normal
			pCorner->Normal=plane.getNormal();

			// Renormalize for security
			pCorner->Normal.normalize();


			// *** **********
			// *** Export Uvs
			// *** **********

			// Num of channels used in this material
			int nNumChannelUsed=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial].size();

			// For each mapping channels used by this material
			for (int uv=0; uv<nNumChannelUsed; uv++)
			{
				// Corresponding max channel
				int nMaxChan=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._IndexInMaxMaterial;

				if( ! pMesh->mapSupport(nMaxChan) )
				{
					nMaxChan = maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._IndexInMaxMaterialAlternative;
				}

				// UVs matrix
				const Matrix3 &uvMatrix=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._UVMatrix;

				// Crop values
				float fCropU=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._CropU;
				float fCropV=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._CropV;
				float fCropW=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._CropW;
				float fCropH=maxBaseBuild.RemapChannel[nMaterialID-maxBaseBuild.FirstMaterial][uv]._CropH;

				// Check kind of channel and if channel is supported
				if ((nMaxChan>=0)&&(nMaxChan<MAX_MESHMAPS)&&pMesh->mapSupport(nMaxChan))
				{
					// *** Explicit channel

					// Get a pointer on Mappingvertex for this channel
					TVFace *pMapFace=pMesh->mapFaces(nMaxChan);

					// Get the index of the mapping vertex
					DWORD nMapVert=pMapFace[face].getTVert (corner);

					// Check it's a valid channel (not a color channel ie !=0)
					nlassert (nMaxChan>0);
					nlassert (nMaxChan<MAX_MESHMAPS);
					
					// Pointer on the mapping vertex
					UVVert *pMapVert=&pMesh->mapVerts(nMaxChan)[nMapVert];

					// Transforme the UV vertex
					Point3 uvTransformed=(*pMapVert)*uvMatrix;

					// Crop it
					uvTransformed.x=uvTransformed.x*fCropW+fCropU;

					// Max UV coordinate origine is the BottomLeft corner. In NeL, it is in the TopLeft corner.
					// So, inverse the V coordinate
					uvTransformed.y=(1.f-uvTransformed.y)*fCropH+fCropV;

					// Store value
					pCorner->Uvs[uv].U=uvTransformed.x;
					pCorner->Uvs[uv].V=uvTransformed.y;
				}
				else
				{
					// No explicit channel or unsupported mapping channel, fill with garbage
					pCorner->Uvs[uv].U=0.f;
					pCorner->Uvs[uv].V=0.f;
				}
			}

			// For other channels, fill with garbage..
			for (; uv<nMapsChannelUsed; uv++)
			{
				pCorner->Uvs[uv].U=0.f;
				pCorner->Uvs[uv].V=0.f;
			}


			// *** ************
			// *** Export Color
			// *** ************

			// Export colors ?
			if (buildMesh.VertexFlags&IDRV_VF_COLOR)
			{
				// Get a pointer on Mappingvertex for this channel. Channel 0 is the color channel.
				TVFace *pMapVert=pMesh->mapFaces(0);

				// Get the index of the mapping vertex
				DWORD nMapVert=pMapVert[face].getTVert (corner);

				// Pointer on the Color vertex. Channel 0 is the color channel.
				UVVert *pColorVert=&pMesh->mapVerts(0)[nMapVert];

				// Store the color
				float fR=(pColorVert->x*255.f+0.5f);
				float fG=(pColorVert->y*255.f+0.5f);
				float fB=(pColorVert->z*255.f+0.5f);
				clamp (fR, 0.f, 255.f);
				clamp (fG, 0.f, 255.f);
				clamp (fB, 0.f, 255.f);
				pCorner->Color.R=(uint8)fR;
				pCorner->Color.G=(uint8)fG;
				pCorner->Color.B=(uint8)fB;
			}
			else
			{
				// Default value used by corse meshes
				pCorner->Color.R=255;
				pCorner->Color.G=255;
				pCorner->Color.B=255;
			}

			
			/// TODO : export Specular ? I'm not sure it's realy useful.
			// *** ***************
			// *** Export Specular
			// *** ***************


		}
	}

	// *** **************
	// *** Export Weights
	// *** **************

	if (skined)
	{
		// Add skinning information to the buildMesh struct
		uint error=buildSkinning (buildMesh, *skeletonShape, node);

		// Error code ?
		if (error!=NoError)
		{
			char msg[512];
			sprintf (msg, "%s skin: %s", getName (node).c_str(), ErrorMessage[error]);
			MessageBox (NULL, msg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			// Active skinning
			buildMesh.VertexFlags|=IDRV_VF_PALETTE_SKIN;
		}
	}

	// Ok, done.
}

// ***************************************************************************

void CExportNel::buildMRMParameters (Animatable& node, CMRMParameters& params)
{
	// Lods count
	params.NLods=getScriptAppData (&node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT);
	
	// Divisor
	params.Divisor=getScriptAppData (&node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT);

	// Get skin reduction parmaters
	switch (getScriptAppData (&node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT))
	{
	case 0:
		params.SkinReduction=CMRMParameters::SkinReductionMin;
		break;
	case 1:
		params.SkinReduction=CMRMParameters::SkinReductionMax;
		break;
	case 2:
		params.SkinReduction=CMRMParameters::SkinReductionBest;
		break;
	default:
		nlassert (0);	// no!
	}

	// Distance finest
	params.DistanceFinest=getScriptAppData (&node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT);

	// Distance middle
	params.DistanceMiddle=getScriptAppData (&node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT);

	// Distance coarsest
	params.DistanceCoarsest=getScriptAppData (&node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT);
}

// ***************************************************************************

IMeshGeom *CExportNel::buildMeshGeom (INode& node, Interface& ip, TimeValue time, 
								const CSkeletonShape* skeletonShape, bool absolutePath,
								CExportNelOptions &opt, CMeshBase::CMeshBaseBuild &buildBaseMesh, 
								std::vector<std::string>& listMaterialName,
								bool& isTransparent,
								bool& isOpaque, const CMatrix& nodeToParentMatrix, bool view)
{
	// Here, we must check what kind of node we can build with this mesh.
	// For the time, just Triobj is supported.
	IMeshGeom *meshGeom=NULL;

	// Skinning at this lod ?
	if (!isSkin (node))
		skeletonShape=NULL;

	// If skinning, disable skin modifier
	if (skeletonShape)
		enableSkinModifier (node, false);

	// Get a pointer on the object's node
    Object *obj = node.EvalWorldState(time).obj;

	// Check if there is an object
	if (obj)
	{
		// Object can be converted in triObject ?
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != tri) 
				deleteIt = true;

			// Coarse mesh ?
			bool coarseMesh=(getScriptAppData (&node, NEL3D_APPDATA_LOD_COARSE_MESH, 0)!=0) && (!view);

			// No skeleton shape
			if (coarseMesh)
				skeletonShape=NULL;

			// Array of name for the material
			CMaxMeshBaseBuild maxBaseBuild;

			// Append material to the base
			buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, absolutePath);

			// Fill the build interface of CMesh
			CMesh::CMeshBuild buildMesh;
			buildMeshInterface (*tri, buildMesh, maxBaseBuild, node, time, skeletonShape, absolutePath, nodeToParentMatrix);

			// Append material names
			isTransparent=false;
			isOpaque=false;
			for (uint i=0; i<maxBaseBuild.MaterialNames.size(); i++)
			{
				// Is opaque, transparent ?
				if( buildBaseMesh.Materials[i+maxBaseBuild.FirstMaterial].getBlend() )
					isTransparent=true;
				else
					isOpaque=true;

				// Push the name
				listMaterialName.push_back (maxBaseBuild.MaterialNames[i]);
			}

			if( hasLightMap( node, time ) && opt.bExportLighting )
				calculateLM(&buildMesh, &buildBaseMesh, node, ip, time, absolutePath, opt);

			// MRM mesh ?
			if (getScriptAppData (&node, NEL3D_APPDATA_LOD_MRM, 0) && (!coarseMesh) )
			{
				// Build a MRM parameters block
				CMRMParameters	parameters;
				buildMRMParameters (node, parameters);

				// Make a CMesh object
				CMeshMRMGeom* meshMRMGeom=new CMeshMRMGeom;

				// Build the mesh with the build interface
				meshMRMGeom->build (buildMesh, buildBaseMesh.Materials.size(), parameters);

				// Return this pointer
				meshGeom=meshMRMGeom;
			}
			else
			{
				// Make a CMesh object
				CMeshGeom* mGeom=new CMeshGeom;

				// Coarse mesh ?
				if (coarseMesh)
				{
					// Force vertex format
					buildMesh.VertexFlags=NL3D_COARSEMESH_VERTEX_FORMAT;
				}

				// Build the mesh with the build interface
				mGeom->build (buildMesh, buildBaseMesh.Materials.size());

				// Return this pointer
				meshGeom=mGeom;
			}

			// Delete the triObject if we should...
			if (deleteIt)
				delete tri;
		}
	}

	// If skinning, renable skin modifier
	if (skeletonShape)
		enableSkinModifier (node, true);

	// Return the shape pointer or NULL if an error occured.
	return meshGeom;
}

// ***************************************************************************

