/** \file nel_export_view.cpp
 * <File description>
 *
 * $Id: nel_export_view.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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

#include "nel/3d/scene.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/skeleton_model.h"
#include "nel/3d/mesh_instance.h"
#include "nel/3d/light.h"
#include "r:/code/nel/tools/3d/object_viewer/object_viewer_interface.h"

#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"

#include "nel_export.h"

using namespace NLMISC;
using namespace NL3D;

#define VIEW_WIDTH 800
#define VIEW_HEIGHT 600

typedef std::map<INode*, CExportNel::mapBoneBindPos > mapRootMapBoneBindPos;

void CNelExport::viewMesh (Interface& ip, TimeValue time)
{
	// Register classes
	registerSerial3d ();
	CScene::registerBasics ();

	// Create an object viewer
	IObjectViewer* view=IObjectViewer::getInterface();

	// Build a skeleton map
	mapRootMapBoneBindPos	skeletonMap;

	// Skeleton shape pointers
	typedef std::map<INode*, CSkeletonModel*>	mapNodeSkel;
	mapNodeSkel									nodeSkelMap;

	if (view)
	{
		// Init it
		view->initUI ();

		// Get node count
		int nNumSelNode=ip.GetSelNodeCount();

		// Create an animation for the models
		CAnimation *anim=new CAnimation;

		// *******************
		// * First build skeleton bind pos information
		// *******************

		// View all selected objects
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
						}
						
						// Add the bind pos for the skin
						CExportNel::addSkeletonBindPos (*pNode, iteSkeleton->second);
					}
				}
			}
		}

		// View all selected objects
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
					// Create a skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);

					// Skeleton exist ?
					if (skeletonRoot)
					{
						// Look for a preexisting skeleton
						mapNodeSkel::iterator ite=nodeSkelMap.find (skeletonRoot);

						// Not found ?
						if (ite==nodeSkelMap.end())
						{
							// Build a skeleton
							CSkeletonShape *skeletonShape=new CSkeletonShape();

							// Look for bind pos info for this skeleton
							mapRootMapBoneBindPos::iterator iteSkel=skeletonMap.find (skeletonRoot);
							nlassert (iteSkel!=skeletonMap.end());

							// Build the skeleton based on the bind pos information
							CExportNel::buildSkeletonShape (*skeletonShape, *skeletonRoot, &(iteSkel->second), time);

							// Add the skeleton in the scene
							//CSkeletonModel *skel=(CSkeletonModel*)view->addMesh (skeletonShape, skeletonRoot->GetName(), "");

							// Add a skeleton entry
							/*nodeSkelMap.insert (mapNodeSkel::value_type (skeletonRoot, skel));
							ite=nodeSkelMap.find (skeletonRoot);

							// Add skeleton animation in the anim
							CExportNel::addBoneTracks (*anim, *skeletonRoot, "", &ip);*/
						}

						// Export the shape
						IShape *pShape;
						pShape=CExportNel::buildShape (*pNode, ip, time, (CSkeletonShape*)(IShape*)(ite->second->Shape), true);

						// Build succesful ?
						if (pShape)
						{
							// Add the shape in the view
							//CMeshInstance* meshInstance=(CMeshInstance*)view->addShape (pShape, pNode->GetName(), (CExportNel::getName (*pNode)+".").c_str());
							CMeshInstance* meshInstance=(CMeshInstance*)view->addMesh (pShape, NULL, pNode->GetName(), (CExportNel::getName (*pNode)+".").c_str());

							// Bind to the skeleton
							//ite->second->bindSkin (meshInstance);

							// ok
							skined=true;
						}
					}
				}
				
				// Build skined ?
				if (!skined)
				{
					// Export the shape
					IShape *pShape;
					pShape=CExportNel::buildShape (*pNode, ip, time, NULL, true);

					// Export successful ?
					if (pShape)
					{
						// Add to the view
						view->addMesh (pShape, NULL, pNode->GetName(), (CExportNel::getName (*pNode)+".").c_str());
					}
				}

				// Add tracks
				CExportNel::addAnimation (*anim, *pNode, (CExportNel::getName (*pNode)+".").c_str(), &ip);
			}
		}

		// Set the single animation
		view->setSingleAnimation (anim, "3dsmax current animation");

		// Setup ambient light
		view->setAmbientColor (CExportNel::getAmbientColor (ip, time));

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

