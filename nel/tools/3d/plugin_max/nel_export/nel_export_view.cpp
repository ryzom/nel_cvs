/** \file nel_export_view.cpp
 * <File description>
 *
 * $Id: nel_export_view.cpp,v 1.16 2001/09/18 14:41:24 corvazier Exp $
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
#include "r:/code/nel/tools/3d/object_viewer/object_viewer_interface.h"

#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"

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

void CNelExport::viewMesh (Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Register classes
	// done in dllentry registerSerial3d ();
	CScene::registerBasics ();
	
	// Create an object viewer
	IObjectViewer* view = IObjectViewer::getInterface();
	
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
						pShape=CExportNel::buildShape (*pNode, ip, time, &iteSkelShape->second.MapId, true, opt, true);

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
					IShape *pShape;
					pShape=CExportNel::buildShape (*pNode, ip, time, NULL, true, opt, true);

					// Export successful ?
					if (pShape)
					{
						// Add to the view
						view->addMesh (pShape, pNode->GetName(), (CExportNel::getName (*pNode)+".").c_str(), NULL);
					}
				}

				// Add tracks
				CExportNel::addAnimation (*anim, *pNode, (CExportNel::getName (*pNode)+".").c_str(), &ip, true, true);
				
			}
			ProgBar.updateProgressBar (nNbMesh);
			if( ProgBar.isCanceledProgressBar() )
				break;
		}

		ProgBar.uninitProgressBar();
		opt.FeedBack = NULL;

		// Set the single animation
		view->setSingleAnimation (anim, "3dsmax current animation");

		// Setup ambient light
		view->setAmbientColor (CExportNel::getAmbientColor (ip, time));

		// Setup background color
		view->setBackGroundColor(CExportNel::getBackGroundColor(ip, time));

		// Build light vector
		std::vector<CLight> vectLight;
		CExportNel::getLights (vectLight, time, ip);

		// Insert each lights
		for (uint light=0; light<vectLight.size(); light++)
			view->setLight (light, vectLight[light]);

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

