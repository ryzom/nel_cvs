/** \file export_scene.cpp
 * Export from 3dsmax to NeL the instance group and cluster/portal accelerators
 *
 * $Id: export_scene.cpp,v 1.19 2002/06/07 08:56:56 vizerie Exp $
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

#include "stdafx.h"
#include "export_nel.h"

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"
#include "../nel_export/nel_export_scene.h"
#include "export_lod.h"
#include "../nel_patch_lib/rpo.h"

#include "3d/scene_group.h"

#include <vector>

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// ***************************************************************************

#include "calc_lm.h" // For convertToWorldCoordinate

extern CVector vGlobalPos;

// ***************************************************************************

CInstanceGroup*	CExportNel::buildInstanceGroup(const vector<INode*>& vectNode, vector<INode*>& resultInstanceNode, TimeValue tvTime)
{
	// Extract from the node the name, the transformations and the parent

	CInstanceGroup::TInstanceArray aIGArray;
	uint32 i, nNumIG;
	uint32 j,k,m;

	aIGArray.empty ();
	resultInstanceNode.empty ();
	aIGArray.resize (vectNode.size());
	resultInstanceNode.resize (vectNode.size());

	int nNbInstance = 0;
	for (i = 0; i < vectNode.size(); ++i)
	{
		INode *pNode = vectNode[i];

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh (*pNode, tvTime) || CExportNel::isDummy(*pNode, tvTime))
		{
			++nNbInstance;
		}
	}

	// Check integrity of the hierarchy and set the parents
	std::vector<INode*>::const_iterator it = vectNode.begin();
	nNumIG = 0;
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone( *pNode, tvTime ))
		if (CExportNel::isMesh( *pNode, tvTime ) || CExportNel::isDummy(*pNode, tvTime))
		{
			aIGArray[nNumIG].DontAddToScene = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_DONT_ADD_TO_SCENE, 0)?true:false;
			aIGArray[nNumIG].InstanceName = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_INSTANCE_NAME, "");
			resultInstanceNode[nNumIG] = pNode;
			if (aIGArray[nNumIG].InstanceName == "") // no instance name was set, takes the node name instead
			{
				aIGArray[nNumIG].InstanceName = pNode->GetName();
			}

			INode *pParent = pNode->GetParentNode();

			// Set the DontCastShadow flag.
			aIGArray[nNumIG].DontCastShadow= pNode->CastShadows()==0;

			// Is the pNode has the root node for parent ?
			if( pParent->IsRootNode() == 0 )
			{
				// Look if the parent is in the selection
				int nNumIG2 = 0;
				for (j = 0; j < vectNode.size(); ++j)
				{
					INode *pNode2 = vectNode[j];

					int nAccelType2 = CExportNel::getScriptAppData (pNode2, NEL3D_APPDATA_ACCEL, 32);
					if ((nAccelType2&3) == 0) // If not an accelerator
					if (!RPO::isZone( *pNode2, tvTime ))
					if (CExportNel::isMesh( *pNode2, tvTime ))
					{
						if (pNode2 == pParent)
							break;
						++nNumIG2;
					}
				}
				if (nNumIG2 == nNbInstance)
				{
					// The parent is not selected ! link to root
					aIGArray[nNumIG].nParent = -1;
				}
				else
				{
					aIGArray[nNumIG].nParent = nNumIG2;
				}
			}
			else
			{
				aIGArray[nNumIG].nParent = -1;
			}
			++nNumIG;
		}
	}
	aIGArray.resize( nNumIG );
	resultInstanceNode.resize( nNumIG );

	// Build the array of node
	vGlobalPos = CVector(0,0,0);
	nNumIG = 0;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh (*pNode, tvTime) || CExportNel::isDummy(*pNode, tvTime))
		{
			CVector vScaleTemp;
			CQuat qRotTemp;
			CVector vPosTemp;

			// Get Nel Name for the object.
			aIGArray[nNumIG].Name= CExportNel::getNelObjectName(*pNode);

			//Get the local transformation matrix
			Matrix3 nodeTM = pNode->GetNodeTM(0);
			INode *pParent = pNode->GetParentNode();
			Matrix3 parentTM = pParent->GetNodeTM(0);
			Matrix3 localTM	= nodeTM*Inverse(parentTM);

			// Extract transformations
			CExportNel::decompMatrix (vScaleTemp, qRotTemp, vPosTemp, localTM);
			aIGArray[nNumIG].Rot   = qRotTemp;
			aIGArray[nNumIG].Pos   = vPosTemp;
			aIGArray[nNumIG].Scale = vScaleTemp;
			vGlobalPos += vPosTemp;
			++nNumIG;
		}
	}
	/// \todo Make this work (precision):
	/*
	vGlobalPos = vGlobalPos / nNumIG;
	for (i = 0; i < nNumIG; ++i)
		aIGArray[i].Pos -= vGlobalPos;
	*/

	vGlobalPos = CVector(0,0,0); // Temporary !!!

	// Accelerator Portal/Cluster part
	//=================

	// Creation of all the clusters
	vector<CCluster> vClusters;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);
		bool bFatherVisible = nAccelType&4?true:false;
		bool bVisibleFromFather = nAccelType&8?true:false;

		if ((nAccelType&3) == 2) // If cluster
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh(*pNode, tvTime))
		{
			CCluster clusterTemp;

			CMesh::CMeshBuild *pMB;
			CMeshBase::CMeshBaseBuild *pMBB;
			pMB = createMeshBuild (*pNode, tvTime, pMBB);

			convertToWorldCoordinate( pMB, pMBB );

			for (j = 0; j < pMB->Faces.size(); ++j)
			{
				if (!clusterTemp.makeVolume (pMB->Vertices[pMB->Faces[j].Corner[0].Vertex],
											 pMB->Vertices[pMB->Faces[j].Corner[1].Vertex],
											 pMB->Vertices[pMB->Faces[j].Corner[2].Vertex]) )
				{
					// ERROR : The volume is not convex !!!
					char tam[256];
					sprintf(tam,"The cluster %s is not convex.",vectNode[i]->GetName());
					//MessageBox(NULL,tam,"Error",MB_OK|MB_ICONERROR);
					nlwarning(tam);
				}
			}

			clusterTemp.FatherVisible = bFatherVisible;
			clusterTemp.VisibleFromFather = bVisibleFromFather;
			clusterTemp.Name = pNode->GetName();

			vClusters.push_back (clusterTemp);
			delete pMB;
			delete pMBB;
		}
	}

	// Creation of all the portals
	vector<CPortal> vPortals;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 1) // If Portal
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh(*pNode, tvTime))
		{
			CPortal portalTemp;

			CMesh::CMeshBuild *pMB;
			CMeshBase::CMeshBaseBuild *pMBB;
			pMB = createMeshBuild (*pNode, tvTime, pMBB);

			convertToWorldCoordinate( pMB, pMBB );

			vector<sint32> poly;
			vector<bool> facechecked;
			facechecked.resize (pMB->Faces.size());
			for (j = 0; j < pMB->Faces.size(); ++j)
				facechecked[j] = false;

			poly.push_back(pMB->Faces[0].Corner[0].Vertex);
			poly.push_back(pMB->Faces[0].Corner[1].Vertex);
			poly.push_back(pMB->Faces[0].Corner[2].Vertex);
			facechecked[0] = true;
			for (j = 0; j < pMB->Faces.size(); ++j)
			if (!facechecked[j])
			{
				bool found = false;

				for(k = 0; k < 3; ++k)
				{
					for(m = 0; m < poly.size(); ++m)
					{
						if ((pMB->Faces[j].Corner[k].Vertex == poly[m]) &&
							(pMB->Faces[j].Corner[(k+1)%3].Vertex == poly[(m+1)%poly.size()]))
						{
							found = true;
							break;
						}
						if ((pMB->Faces[j].Corner[(k+1)%3].Vertex == poly[m]) &&
							(pMB->Faces[j].Corner[k].Vertex == poly[(m+1)%poly.size()]))
						{
							found = true;
							break;
						}
					}
					if (found)
						break;
				}
				if (found)
				{
					// insert an empty space in poly between m and m+1
					poly.resize (poly.size()+1);
					for (uint32 a = poly.size()-2; a > m; --a)
						poly[a+1] = poly[a];
					poly[m+1] = pMB->Faces[j].Corner[(k+2)%3].Vertex;
					facechecked[j] = true;
					j = 0;
				}
			}
			vector<CVector> polyv;
			polyv.resize (poly.size());
			for (j = 0; j < poly.size(); ++j)
				polyv[j] = pMB->Vertices[poly[j]];
			
			if (!portalTemp.setPoly (polyv))
			{
				// ERROR : Poly not convex, or set of vertices not plane
				char tam[256];
				sprintf(tam,"The portal %s is not convex.",vectNode[i]->GetName());
				//MessageBox(NULL,tam,"Error",MB_OK|MB_ICONERROR);
				nlwarning(tam);
			}

			if (nAccelType&16) // is dynamic portal ?
				portalTemp.setName (string(pNode->GetName()));

			// Check if portal has 2 cluster
			int nNbCluster = 0;
			for (j = 0; j < vClusters.size(); ++j)
			{
				bool bPortalInCluster = true;
				for (k = 0; k < polyv.size(); ++k)
					if (!vClusters[j].isIn (polyv[k]) )
					{
						bPortalInCluster = false;
						break;
					}
				if (bPortalInCluster)
					++nNbCluster;
			}
			if (nNbCluster != 2)
			{
				// ERROR
				char tam[256];
				sprintf(tam,"The portal %s has not 2 clusters.",vectNode[i]->GetName());
				//MessageBox(NULL,tam,"Error",MB_OK|MB_ICONERROR);
				nlwarning(tam);
			}


			vPortals.push_back (portalTemp);
			delete pMB;
			delete pMBB;
		}
	}

	// Link instance to clusters (an instance has a list of clusters)
	nNumIG = 0;
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		int nAccelType = CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_ACCEL, 32);

		if ((nAccelType&3) == 0) // If not an accelerator
		if (!RPO::isZone (*pNode, tvTime))
		if (CExportNel::isMesh (*pNode, tvTime) || CExportNel::isDummy(*pNode, tvTime))
		{
			if (nAccelType&32) // Is the flag clusterize set ?
			{
				// Test against all clusters

				CMesh::CMeshBuild *pMB;
				CMeshBase::CMeshBaseBuild *pMBB;
				pMB = createMeshBuild (*pNode, tvTime, pMBB);

				convertToWorldCoordinate( pMB, pMBB );

				for(k = 0; k < vClusters.size(); ++k)
				{
					bool bMeshInCluster = false;

					for(j = 0; j < pMB->Vertices.size(); ++j)
					{
						if (vClusters[k].isIn (pMB->Vertices[j]))
						{
							bMeshInCluster = true;
							break;
						}
					}

					if (bMeshInCluster)
					{
						aIGArray[nNumIG].Clusters.push_back (k);
					}
				}
				
				// debug purpose : to remove
				if (vClusters.size() > 0)
				if (aIGArray[nNumIG].Clusters.size() == 0)
				{
					char tam[256];
					sprintf(tam,"Object %s is not attached to any cluster\nbut his flag clusterize is set", pNode->GetName());
					//MessageBox(NULL, tam, "Warning", MB_OK);
					nlwarning(tam);
				}
				// debug purpose : to remove

				delete pMB;
				delete pMBB;
			}
			
			++nNumIG;
		}
		// debug purpose : to remove
		/*
		if ((nAccelType&3) == 0) // If not an accelerator
		if (!(nAccelType&32))
		{
			char tam[256];
			sprintf(tam,"Object %s is not clusterized", pNode->GetName());
			MessageBox(NULL, tam, "Info", MB_OK);
		}
		*/
		// debug purpose : to remove

	}


	// PointLight part
	//=================
	bool	sunLightEnabled= false;
	sint	nNumPointLight = 0;
	vector<CPointLightNamed>	pointLights;
	pointLights.resize(vectNode.size());
	// For all nodes
	it = vectNode.begin();
	for (i = 0; i < (sint)vectNode.size(); ++i, ++it)
	{
		INode *pNode = *it;

		SLightBuild		sLightBuild;

		// If it is a Max Light.
		if ( sLightBuild.canConvertFromMaxLight(pNode, tvTime) )
		{
			// And if this light is checked to realtime export
			int		nRTExport= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED);
			if(nRTExport == BST_CHECKED)
			{
				// get Max Light info.
				sLightBuild.convertFromMaxLight(pNode, tvTime);

				// Skip if LightDir
				if(sLightBuild.Type != SLightBuild::LightDir)
				{
					// Fill PointLight Info.
					NL3D::CPointLightNamed	&plNamed= pointLights[nNumPointLight];

					// Position
					plNamed.setPosition(sLightBuild.Position);
					// Attenuation
					plNamed.setupAttenuation(sLightBuild.rRadiusMin, sLightBuild.rRadiusMax);
					// Colors
					// Ensure A=255 for localAmbient to work.
					NLMISC::CRGBA	ambient= sLightBuild.Ambient;
					ambient.A= 255;
					plNamed.setDefaultAmbient(ambient);
					plNamed.setAmbient(ambient);
					plNamed.setDefaultDiffuse(sLightBuild.Diffuse);
					plNamed.setDiffuse(sLightBuild.Diffuse);
					plNamed.setDefaultSpecular(sLightBuild.Specular);
					plNamed.setSpecular(sLightBuild.Specular);
					// GroupName.
					plNamed.LightGroupName= sLightBuild.GroupName;

					// Which light type??
					if(sLightBuild.bAmbientOnly || sLightBuild.Type== SLightBuild::LightAmbient)
						plNamed.setType(CPointLight::AmbientLight);
					else if(sLightBuild.Type== SLightBuild::LightPoint)
						plNamed.setType(CPointLight::PointLight);
					else if(sLightBuild.Type== SLightBuild::LightSpot)
					{
						plNamed.setType(CPointLight::SpotLight);
						// Export Spot infos.
						plNamed.setupSpotDirection(sLightBuild.Direction);
						plNamed.setupSpotAngle(sLightBuild.rHotspot, sLightBuild.rFallof);
					}
					else
					{
						// What???
						nlstop;
					}


					// inc Size
					++nNumPointLight;
				}
			}

			// if this light is a directionnal and checked to export as Sun Light
			int		nExportSun= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
			if(nExportSun== BST_CHECKED)
			{
				// get Max Light info.
				sLightBuild.convertFromMaxLight(pNode, tvTime);

				// Skip if not dirLight.
				if(sLightBuild.Type == SLightBuild::LightDir)
					sunLightEnabled= true;
			}
		}
	}
	// Good size
	pointLights.resize(nNumPointLight);


	// Build the ig
	//=================

	CInstanceGroup* pIG = new CInstanceGroup;

	// Link portals and clusters and create meta cluster if one
	pIG->build (vGlobalPos,  aIGArray, vClusters, vPortals, pointLights);

	// IG touched by sun ??
	pIG->enableRealTimeSunContribution(sunLightEnabled);

	return pIG;
}
