/** \file nel_export_view.cpp
 * <File description>
 *
 * $Id: nel_export_view.cpp,v 1.24 2002/02/28 14:24:52 berenguier Exp $
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

#include "std_afx.h"

#include "3d/scene.h"
#include "3d/register_3d.h"
#include "3d/skeleton_shape.h"
#include "3d/skeleton_model.h"
#include "3d/mesh_instance.h"
#include "3d/light.h"
#include "3d/water_pool_manager.h"
#include "3d/instance_lighter.h"

#include "r:/code/nel/tools/3d/object_viewer/object_viewer_interface.h"


#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"
#include "../nel_mesh_lib/calc_lm.h"
#include "../nel_mesh_lib/export_lod.h"

#include "nel/misc/time_nl.h"

#include "nel_export.h"
#include "progress.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

#define VIEW_WIDTH 800
#define VIEW_HEIGHT 600

typedef map<INode*, CExportNel::mapBoneBindPos > mapRootMapBoneBindPos;

// -----------------------------------------------------------------------------------------------

class CSkeletonDesc
{
public:
	CSkeletonDesc (CSkeletonModel *skeletonShape, const TInodePtrInt& mapId)
	{
		SkeletonShape=skeletonShape;
		MapId=mapId;
	}
	CSkeletonModel	*SkeletonShape;
	TInodePtrInt	MapId;
};


// -----------------------------------------------------------------------------------------------
class	CMaxInstanceLighter : public NL3D::CInstanceLighter
{
public:
	CProgressBar	ProgressBar;

	void	initMaxLighter(Interface& ip)
	{
		ProgressBar.initProgressBar (100, ip);
	}
	void	closeMaxLighter()
	{
		ProgressBar.uninitProgressBar ();
	}

	virtual void progress (const char *message, float progress)
	{
		ProgressBar.setLine(0, string(message));
		ProgressBar.update();
		ProgressBar.updateProgressBar ((uint32)(100 * progress));
	}
};

// -----------------------------------------------------------------------------------------------

void CNelExport::viewMesh (Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Register classes
	// done in dllentry registerSerial3d ();

	CScene::registerBasics ();
	
	// Create an object viewer
	IObjectViewer* view = IObjectViewer::getInterface();

	// set the water pool manager
	view->setWaterPoolManager(NL3D::GetWaterPoolManager());
	
	// Build a skeleton map
	mapRootMapBoneBindPos				skeletonMap;

	std::map<INode*, CSkeletonDesc>	mapSkeletonShape;

	if (view)
	{
		// Init it
		view->initUI ();

		// Get node count
		int nNumSelNode=ip.GetSelNodeCount();
		int nNbMesh=0;
		// Create an animation for the models
		CAnimation *anim=new CAnimation;
		CAnimation *autoAnim=new CAnimation;

		// *******************
		// * First build skeleton bind pos information and animations
		// *******************

		int nNode;
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=ip.GetSelNode (nNode);

			// It is a zone ?
			if (RPO::isZone (*pNode, time))
			{
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, time))
			{
				// Build skined ?
				bool skined=false;
				
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Get root of the skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);

					// HULUD TEST
					//skeletonRoot=NULL;
					
					// Root exist ?
					if (skeletonRoot)
					{
						// Ok, look for the set in the map of desc
						mapRootMapBoneBindPos::iterator iteSkeleton=skeletonMap.find (skeletonRoot);

						// Not found ?
						if (iteSkeleton==skeletonMap.end())
						{
							// Insert one
							skeletonMap.insert (mapRootMapBoneBindPos::value_type (skeletonRoot, CExportNel::mapBoneBindPos()));
							iteSkeleton=skeletonMap.find (skeletonRoot);

							// Add tracks
							CExportNel::addBoneTracks (*anim, *skeletonRoot, (CExportNel::getName (*skeletonRoot)+".").c_str(), &ip, true, true);
						}
						
						// Add the bind pos for the skin
						CExportNel::addSkeletonBindPos (*pNode, iteSkeleton->second);
					}
				}
			}
		}

		// *******************
		// * Then, build skeleton shape
		// *******************

		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=ip.GetSelNode (nNode);

			// It is a zone ?
			if (RPO::isZone (*pNode, time))
			{
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, time))
			{
				// Build skined ?
				bool skined=false;
				++nNbMesh;
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Get root of the skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);
					
					// HULUD TEST
					//skeletonRoot=NULL;
					
					// Root exist ?
					if (skeletonRoot)
					{
						// Ok, look for the set in the map of desc
						mapRootMapBoneBindPos::iterator iteSkeleton = skeletonMap.find (skeletonRoot);
						std::map<INode*, CSkeletonDesc>::iterator skelBindPod = mapSkeletonShape.find (skeletonRoot);

						// Not found ?
						if (skelBindPod==mapSkeletonShape.end())
						{
							// Insert it
							CSkeletonShape *skelShape=new CSkeletonShape();

							// A matrix id map
							TInodePtrInt mapId;

							// Build the skeleton based on the bind pos information
							CExportNel::buildSkeletonShape (*skelShape, *skeletonRoot, &(iteSkeleton->second), mapId, time, true);

							// Add the shape in the view
							CSkeletonModel *skelInstance=view->addSkel (skelShape, skeletonRoot->GetName(), 
								(CExportNel::getName (*skeletonRoot)+".").c_str());

							// Insert in the map
							mapSkeletonShape.insert (std::map<INode*, CSkeletonDesc>::value_type ( skeletonRoot, CSkeletonDesc (skelInstance, mapId)));
						}
					}
				}
			}
		}

		view->setAutoAnimation (NULL);
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=ip.GetSelNode (nNode);

			// Is it a automatic light ? if yes add tracks from nel_light (color controller)
			Modifier *modifier = CExportNel::getModifier( pNode, Class_ID(NEL_LIGHT_CLASS_ID_A,NEL_LIGHT_CLASS_ID_B) );
			if( modifier != NULL )
				CExportNel::addAnimation( *autoAnim, *pNode, "", &ip, true, true);
		}
		view->setAutoAnimation (autoAnim);

		// *******************
		// * Then, build Mesh shapes
		// *******************

		// Prepare ig export.
		std::vector<INode*>						igVectNode;
		std::map<std::string, NL3D::IShape *>	igShapeMap;
		// SunDirection.
		NLMISC::CVector							igSunDirection(0, 1, -1);
		// SunColor.
		NLMISC::CRGBA							igSunColor(255, 255, 255);
		SLightBuild								sgLightBuild;


		// Build Mesh Shapes.
		CProgressBar ProgBar;
		ProgBar.initProgressBar (nNbMesh, ip);
		opt.FeedBack = &ProgBar;
		nNbMesh = 0;
		// View all selected objects
		for (nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=ip.GetSelNode (nNode);

			string sTmp = "Object Name: ";
			sTmp += pNode->GetName();
			ProgBar.setLine (0, sTmp);
			sTmp = "";
			for (uint32 i = 1; i < 10; ++i) 
				ProgBar.setLine (i, sTmp);
			sTmp = "Last Error";
			ProgBar.setLine (10, sTmp);
			ProgBar.update();
			
			// It is a zone ?
			if (RPO::isZone (*pNode, time))
			{
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, time))
			{
				// Build skined ?
				bool skined=false;
				++nNbMesh;
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Create a skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);

					// HULUD TEST
					//skeletonRoot=NULL;
					
					// Skeleton exist ?
					if (skeletonRoot)
					{
						// Look for bind pos info for this skeleton
						mapRootMapBoneBindPos::iterator iteSkel=skeletonMap.find (skeletonRoot);
						std::map<INode*, CSkeletonDesc>::iterator iteSkelShape=mapSkeletonShape.find (skeletonRoot);
						nlassert (iteSkel!=skeletonMap.end());
						nlassert (iteSkelShape!=mapSkeletonShape.end());

						// Export the shape
						IShape *pShape;
						CSkeletonShape *skeletonShape=dynamic_cast<CSkeletonShape*> ((IShape*)iteSkelShape->second.SkeletonShape->Shape);
						pShape=CExportNel::buildShape (*pNode, ip, time, &iteSkelShape->second.MapId, true, opt, true, true);

						// Build succesful ?
						if (pShape)
						{
							// Add the shape in the view
							CMeshInstance* meshInstance=(CMeshInstance*)view->addMesh (pShape, pNode->GetName(), 
								(CExportNel::getName (*pNode)+".").c_str(), iteSkelShape->second.SkeletonShape);

							// ok
							skined=true;
						}
						else
							delete skeletonShape;
					}
				}
				// Build skined ?
				if (!skined)
				{
					// Export the shape
					IShape *pShape = NULL;
					pShape=CExportNel::buildShape (*pNode, ip, time, NULL, true, opt, true, true);

					// Export successful ?
					if (pShape)
					{
						// get the nelObjectName, as used in building of the instanceGroup.
						std::string	nelObjectName= CExportNel::getNelObjectName(*pNode);

						// ugly way to verify the shape is really added to the sahepBank: use a refPtr :)
						NLMISC::CRefPtr<IShape>		prefShape= pShape;

						// Add to the view, but don't create the instance (created in ig).
						view->addMesh (pShape, (nelObjectName + ".shape").c_str(), (nelObjectName + ".").c_str(), NULL, false);

						// If the shape is not destroyed in addMesh() (with smarPtr), then add it to the shape map.
						if(prefShape)
						{
							igShapeMap.insert( std::make_pair(nelObjectName, pShape) );
						}
						
						// Add to list of node for IgExport.
						igVectNode.push_back(pNode);
					}
				}

				// Add tracks
				CExportNel::addAnimation (*anim, *pNode, (CExportNel::getName (*pNode)+".").c_str(), &ip, true, true);
			}
			// Try to export a light for Ig. Only if ExportLighting
			// Only if View only selected lights.
			else if( sgLightBuild.canConvertFromMaxLight(pNode, time) && opt.bExportLighting && opt.bExcludeNonSelected)
			{
				// Convert it.
				sgLightBuild.convertFromMaxLight(pNode, time);

				// PointLight/SpotLight/AmbientLight ??
				if(sgLightBuild.Type != SLightBuild::LightDir)
				{
					// Add to list of node for IgExport.
					igVectNode.push_back(pNode);
				}
				// "SunLight" ??
				else if( sgLightBuild.Type == SLightBuild::LightDir )
				{
					// if this light is checked to export as Sun Light
					int		nExportSun= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
					if(nExportSun== BST_CHECKED)
					{
						// Add to list of node for IgExport (enabling SunLight in the ig)
						igVectNode.push_back(pNode);

						// Replace sun Direciton.
						igSunDirection= sgLightBuild.Direction;
						// Replace sun Color.
						igSunColor= sgLightBuild.Diffuse;
					}
				}
			}

			ProgBar.updateProgressBar (nNbMesh);
			if( ProgBar.isCanceledProgressBar() )
				break;
		}

		// if ExportLighting and Export all lights in scene (not only selected ones).
		if(opt.bExportLighting && !opt.bExcludeNonSelected)
		{
			// List all nodes in scene.
			vector<INode*>	nodeList;
			CExportNel::getObjectNodes(nodeList, time, ip);
			
			// For all of them.
			for(uint i=0;i<nodeList.size();i++)
			{
				INode	*pNode= nodeList[i];

				if( sgLightBuild.canConvertFromMaxLight(pNode, time) )
				{
					// Convert it.
					sgLightBuild.convertFromMaxLight(pNode, time);

					// PointLight/SpotLight/AmbientLight ??
					if(sgLightBuild.Type != SLightBuild::LightDir)
					{
						// Add to list of node for IgExport.
						igVectNode.push_back(pNode);
					}
					// "SunLight" ??
					else if( sgLightBuild.Type == SLightBuild::LightDir )
					{
						// if this light is checked to export as Sun Light
						int		nExportSun= CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);
						if(nExportSun== BST_CHECKED)
						{
							// Add to list of node for IgExport (enabling SunLight in the ig)
							igVectNode.push_back(pNode);

							// Replace sun Direciton.
							igSunDirection= sgLightBuild.Direction;
							// Replace sun Color.
							igSunColor= sgLightBuild.Diffuse;
						}
					}
				}
			}
		}


		ProgBar.uninitProgressBar();
		opt.FeedBack = NULL;
	

		// *******************
		// * Export instance Group.
		// *******************


		// Build the ig (with pointLights)
		NL3D::CInstanceGroup	*ig= CExportNel::buildInstanceGroup(igVectNode, time);
		if(ig)
		{
			// If ExportLighting
			if( opt.bExportLighting )
			{
				// Light the ig.
				NL3D::CInstanceGroup	*igOut= new NL3D::CInstanceGroup;
				// Init the lighter.
				CMaxInstanceLighter		maxInstanceLighter;
				maxInstanceLighter.initMaxLighter(ip);

				// Setup LightDesc Ig.
				CInstanceLighter::CLightDesc	lightDesc;
				// Copy map to get info on shapes.
				lightDesc.UserShapeMap= igShapeMap;
				// Setup Shadow and overSampling.
				lightDesc.Shadow= opt.bShadow;
				lightDesc.OverSampling= NLMISC::raiseToNextPowerOf2(opt.nOverSampling);
				clamp(lightDesc.OverSampling, 0U, 32U);
				if(lightDesc.OverSampling==1)
					lightDesc.OverSampling= 0;
				// Setup LightDirection.
				lightDesc.LightDirection= igSunDirection.normed();
				// For interiors ig, disable Sun contrib according to ig.
				lightDesc.DisableSunContribution= !ig->getRealTimeSunContribution();


				// Simply Light Ig.
				CInstanceLighter::lightIgSimple(maxInstanceLighter, *ig, *igOut, lightDesc);

				// Close the lighter.
				maxInstanceLighter.closeMaxLighter();

				// Swap pointer and release unlighted one.
				swap(ig, igOut);
				delete igOut;
			}


			// Setup the ig in Viewer.
			view->addInstanceGroup(ig);
		}


		// *******************
		// * Launch
		// *******************

		// Set the single animation
		view->setSingleAnimation (anim, "3dsmax current animation");

		// Setup ambient light
		view->setAmbientColor (CExportNel::getAmbientColor (ip, time));

		// Setup background color
		if (opt.bExportBgColor)
			view->setBackGroundColor(CExportNel::getBackGroundColor(ip, time));

		// ExportLighting?
		if ( opt.bExportLighting )
		{
			// setup lighting and sun, if any light added. Else use std OpenGL front lighting
			view->setupSceneLightingSystem(true, igSunDirection, CRGBA::Black, igSunColor, igSunColor);
		}
		else
		{
			// Build light vector
			std::vector<CLight> vectLight;
			CExportNel::getLights (vectLight, time, ip);

			// Light in the scene ?
			if (!vectLight.empty())
			{
				// Use old Driver Light mgt.
				view->setupSceneLightingSystem(false, igSunDirection, CRGBA::Black, igSunColor, igSunColor);

				// Insert each lights
				for (uint light=0; light<vectLight.size(); light++)
					view->setLight (light, vectLight[light]);
			}
		}

		// Reset the camera
		view->resetCamera ();

		// Go
		view->go ();

		// Release object viewer
		view->releaseUI ();

		// Delete the pointer
		IObjectViewer::releaseInterface (view);
	}
}

