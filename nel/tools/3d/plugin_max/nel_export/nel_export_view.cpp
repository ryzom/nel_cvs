/** \file nel_export_view.cpp
 * <File description>
 *
 * $Id: nel_export_view.cpp,v 1.7 2001/06/26 14:58:35 corvazier Exp $
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

using namespace NLMISC;
using namespace NL3D;

#define VIEW_WIDTH 800
#define VIEW_HEIGHT 600

typedef std::map<INode*, CExportNel::mapBoneBindPos > mapRootMapBoneBindPos;



// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

volatile bool gbCancelCalculation;
float gRatioCalculated;
double gTimeBegin;
uint32 gNbTotalMeshes;
HWND gHWndProgress;

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

int CALLBACK CalculatingDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	double TimeCurrent = CTime::ticksToSecond( CTime::getPerformanceTime() );
	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			CenterWindow( hwndDlg, theCNelExport.ip->GetMAXHWnd() );
			ShowWindow( hwndDlg, SW_SHOWNORMAL );
			gRatioCalculated = 0.0;
			gbCancelCalculation = false;
		}
		break;

		case WM_PAINT:
		{
			char temp[256];
			SendMessage( GetDlgItem( hwndDlg, IDC_PROGRESS1 ), PBM_SETPOS, gRatioCalculated*100, 0 );

			if( gRatioCalculated > 0.0 )
			{
				double TimeLeft = (TimeCurrent - gTimeBegin) * (1.0-gRatioCalculated);
				sprintf( temp, "Time remaining : %02d h %02d m %02d s", ((uint32)TimeLeft)/3600,
																		(((uint32)TimeLeft)/60)%60,
																		(((uint32)TimeLeft))%60 );

				SendMessage( GetDlgItem( hwndDlg, IDC_STATICTIMELEFT ), WM_SETTEXT, 0, (long)temp );
				SendMessage( GetDlgItem( hwndDlg, IDC_BUTTONCANCEL ), WM_PAINT, 0, 0 );
			}
		}
		break;

		case WM_DESTROY:
			gbCancelCalculation = true;
		break;
		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
				// ---
				case IDC_BUTTONCANCEL:
					if( HIWORD(wParam) == BN_CLICKED )
						gbCancelCalculation = true;
				break;
				default:
				break;
			}
		}		
		break;
		default:
			return FALSE;
		break;
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------------
// Initialize the dialog with the total number of meshes to treat
void initProgressBar( sint32 nNbMesh, Interface &ip)
{
	gTimeBegin = CTime::ticksToSecond( CTime::getPerformanceTime() );
	gNbTotalMeshes = nNbMesh;
	gHWndProgress = CreateDialog(	CNelExportDesc.HInstance(), 
									MAKEINTRESOURCE(IDD_CALCULATING),
									NULL,//ip.GetMAXHWnd(), 
									CalculatingDialogCallback			);								
}

// -----------------------------------------------------------------------------------------------
void uninitProgressBar()
{
	DestroyWindow( gHWndProgress );
}

// -----------------------------------------------------------------------------------------------
// Update with the current mesh treated
void updateProgressBar( sint32 NMeshNb )
{
	gRatioCalculated = ((float)NMeshNb) / ((float)gNbTotalMeshes);
	for( uint i = 0; i < 32; ++i )
	{
		MSG msg;
		PeekMessage(&msg,(HWND)gHWndProgress ,0,0,PM_REMOVE);
		{
			if( IsDialogMessage(gHWndProgress ,&msg) )
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}
	}

}

// -----------------------------------------------------------------------------------------------
bool isCanceledProgressBar()
{
	return gbCancelCalculation;
}


void CNelExport::viewMesh (Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Register classes
	registerSerial3d ();
	CScene::registerBasics ();

	// Create an object viewer
	IObjectViewer* view=IObjectViewer::getInterface();

	// Build a skeleton map
	mapRootMapBoneBindPos				skeletonMap;
	std::map<INode*, CSkeletonModel*>	mapSkeletonShape;

	if (view)
	{
		// Init it
		view->initUI ();

		// Get node count
		int nNumSelNode=ip.GetSelNodeCount();

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
							CExportNel::addBoneTracks (*anim, *skeletonRoot, (CExportNel::getName (*skeletonRoot)+".").c_str(), &ip);
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
				
				// Skinning ?
				if (CExportNel::isSkin (*pNode))
				{
					// Get root of the skeleton
					INode *skeletonRoot=CExportNel::getSkeletonRootBone (*pNode);
					
					// Root exist ?
					if (skeletonRoot)
					{
						// Ok, look for the set in the map of desc
						mapRootMapBoneBindPos::iterator iteSkeleton = skeletonMap.find (skeletonRoot);
						std::map<INode*, CSkeletonModel*>::iterator skelBindPod = mapSkeletonShape.find (skeletonRoot);

						// Not found ?
						if (skelBindPod==mapSkeletonShape.end())
						{
							// Insert it
							CSkeletonShape *skelShape=new CSkeletonShape();

							// Build the skeleton based on the bind pos information
							CExportNel::buildSkeletonShape (*skelShape, *skeletonRoot, &(iteSkeleton->second), time);

							// Add the shape in the view
							CSkeletonModel *skelInstance=view->addSkel (skelShape, skeletonRoot->GetName(), 
								(CExportNel::getName (*skeletonRoot)+".").c_str());

							// Insert in the map
							mapSkeletonShape.insert (std::map<INode*, CSkeletonModel*>::value_type ( skeletonRoot, skelInstance));
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
				CExportNel::addAnimation( *autoAnim, *pNode, "", &ip );
		}
		view->setAutoAnimation (autoAnim);

		initProgressBar( nNumSelNode, ip );
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
						// Look for bind pos info for this skeleton
						mapRootMapBoneBindPos::iterator iteSkel=skeletonMap.find (skeletonRoot);
						std::map<INode*, CSkeletonModel*>::iterator iteSkelShape=mapSkeletonShape.find (skeletonRoot);
						nlassert (iteSkel!=skeletonMap.end());
						nlassert (iteSkelShape!=mapSkeletonShape.end());

						// Export the shape
						IShape *pShape;
						CSkeletonShape *skeletonShape=dynamic_cast<CSkeletonShape*> ((IShape*)iteSkelShape->second->Shape);
						pShape=CExportNel::buildShape (*pNode, ip, time, skeletonShape, true, opt);

						// Build succesful ?
						if (pShape)
						{
							// Add the shape in the view
							CMeshInstance* meshInstance=(CMeshInstance*)view->addMesh (pShape, pNode->GetName(), 
								(CExportNel::getName (*pNode)+".").c_str(), iteSkelShape->second);

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
					pShape=CExportNel::buildShape (*pNode, ip, time, NULL, true, opt);

					// Export successful ?
					if (pShape)
					{
						// Add to the view
						view->addMesh (pShape, pNode->GetName(), (CExportNel::getName (*pNode)+".").c_str(), NULL);
					}
				}

				// Add tracks
				CExportNel::addAnimation (*anim, *pNode, (CExportNel::getName (*pNode)+".").c_str(), &ip);
				
			}
			updateProgressBar( nNode );
			if( isCanceledProgressBar() )
				break;
		}
		uninitProgressBar();

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

