/** \file nel_export.cpp
 * <File description>
 *
 * $Id: nel_export.cpp,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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
#include "nel_export.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "checkversion.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_patch_lib/rpo.h"

using namespace NL3D;
using namespace NLMISC;

CNelExport theCNelExport;

class CNelExportClassDesc:public ClassDesc2 
{
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
	{
		// Check only one time the version of the plugin on the server
		static bool bPassed=false;
		if (!bPassed)
		{
			bPassed=true;
			CheckPluginVersion ("plugins max\\plugins\\nelexport.dlu");
		}

		return &theCNelExport;
	}
	const TCHAR *	ClassName() {return _T("NeL Export");}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return CNELEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return _T("NeL Tools");}
	const TCHAR*	InternalName() { return _T("NeL export and view"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static CNelExportClassDesc CNelExportDesc;
ClassDesc2* GetCNelExportDesc() {return &CNelExportDesc;}

static const char *zoneFilter="NeL zone file (*.zone)\0*.zone\0All files (*.*)\0*.*\0";
static const char *meshFilter="NeL shape file (*.shape)\0*.shape\0All files (*.*)\0*.*\0";
static const char *animModelFilter="NeL model animation file (*.anim)\0*.anim\0All files (*.*)\0*.*\0";
static const char *animSceneFilter="NeL scene animation file (*.sceneanim)\0*.sceneanim\0All files (*.*)\0*.*\0";
static const char *SWTFilter="NeL Skeleton Weight Template file (*.swt)\0*.swt\0All files (*.*)\0*.*\0";
static const char *InstanceGroupFilter="NeL Instance Group file (*.ig)\0*.ig\0All files (*.*)\0*.*\0";
static const char *skeletonFilter="NeL Skeleton file (*.skel)\0*.skel\0All files (*.*)\0*.*\0";

static BOOL CALLBACK CNelExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
			theCNelExport.Init(hWnd);
			break;

		case WM_DESTROY:
			theCNelExport.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
			// ---
			case ID_SAVEMODEL:
				{
					// Register 3d models
					registerSerial3d();

					// Get time
					TimeValue time=theCNelExport.ip->GetTime();
					
					// Get node count
					int nNumSelNode=theCNelExport.ip->GetSelNodeCount();

					// Save all selected objects
					for (int nNode=0; nNode<nNumSelNode; nNode++)
					{
						// Get the node
						INode* pNode=theCNelExport.ip->GetSelNode (nNode);

						// Name of the node
						char sNodeMsg[256];
						sprintf (sNodeMsg, "Save %s model...", pNode->GetName());

						// It is a zone ?
						if (RPO::isZone (*pNode, time))
						{
							// Save path
							char sSavePath[256];
							strcpy (sSavePath, pNode->GetName());

							// Choose a file to export
							if (theCNelExport.SelectFileForSave(hWnd, sNodeMsg, zoneFilter, sSavePath))
							{
								// Export the zone
								if (!theCNelExport.exportZone (sSavePath, *pNode, time))
								{
									// Error message
									char sErrorMsg[512];
									sprintf (sErrorMsg, "Error exporting the zone %s in the file\n%s", pNode->GetName(), sSavePath);
									MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
								}
							}
						}
						// Try to export a mesh
						else if (CExportNel::isMesh (*pNode, time))
						{
							// Save path
							char sSavePath[256];
							strcpy (sSavePath, pNode->GetName());

							// Choose a file to export
							if (theCNelExport.SelectFileForSave(hWnd, sNodeMsg, meshFilter, sSavePath))
							{
								// Skin objects
								CSkeletonShape *pSkinShape=NULL;

								// Check if it is a skin
								if (CExportNel::isSkin (*pNode))
								{
									char sSkinMsg[256];
									char sSkeletonPath[512];
									sprintf (sSkinMsg, "Select a skeleton for %s", pNode->GetName());

									// Ask for a skin
									if (theCNelExport.SelectFileForLoad(hWnd, sSkinMsg, skeletonFilter, sSkeletonPath))
									{
										// Load the skin
										CIFile file;
										if (file.open (sSkeletonPath))
										{
											// Serial the skeleton
											try
											{
												// Serial
												CShapeStream serialShape;
												serialShape.serial (file);

												// Get the pointer
												pSkinShape=dynamic_cast<CSkeletonShape*> (serialShape.getShapePointer ());

												// If it is the wrong class, delete this pointer
												if (pSkinShape==NULL)
													delete pSkinShape;
											}
											catch (Exception& e)
											{
												char sErrorMsg[512];
												sprintf (sErrorMsg, "Error during reading the skeleton: %s.", e.what());
												MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
											}
										}
										else
										{
											// Error message
											char sErrorMsg[512];
											sprintf (sErrorMsg, "Can't open file %s for reading.", sSkeletonPath);
											MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
										}
									}
								}

								// Export the zone
								if (!theCNelExport.exportMesh (sSavePath, *pNode, *theCNelExport.ip, time, pSkinShape))
								{
									// Error message
									char sErrorMsg[512];
									sprintf (sErrorMsg, "Error exporting the mesh %s in the file\n%s", pNode->GetName(), sSavePath);
									MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
								}

								// Delete the skeleton pointer
								if (pSkinShape)
									delete pSkinShape;
							}
						}
					}
				}
				break;
			// ---
			case ID_SAVE_MODEL_ANIM:
			case ID_SAVE_SCENE_ANIM:
				{
					// Register 3d models
					registerSerial3d();

					// Get time
					TimeValue time=theCNelExport.ip->GetTime();
					
					// Get node count
					uint nNumSelNode=theCNelExport.ip->GetSelNodeCount();

					// Save all selected objects
					if (nNumSelNode)
					{
						// Make a list of nodes
						std::vector<INode*> vectNode;
						theCNelExport.getSelectedNode (vectNode);
						nlassert (vectNode.size()!=0);

						// Name of the node
						char sNodeMsg[256];
						sprintf (sNodeMsg, "Save animations...");

						// Save path
						char sSavePath[256];
						strcpy (sSavePath, (*vectNode.begin())->GetName());

						// Choose a file to export
						if (theCNelExport.SelectFileForSave (hWnd, sNodeMsg, (LOWORD(wParam)==ID_SAVE_MODEL_ANIM)?animModelFilter:animSceneFilter, 
							sSavePath))
						{
							// Export the zone
							if (!theCNelExport.exportAnim (sSavePath, vectNode, *theCNelExport.ip, time, LOWORD(wParam)==ID_SAVE_SCENE_ANIM))
							{
								// Error message
								char sErrorMsg[512];
								sprintf (sErrorMsg, "Error exporting animation %s in the file\n%s", (*vectNode.begin())->GetName(), sSavePath);
								MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
							}
						}
					}
				}
				break;
			// ---
			case ID_VIEW:
				{
				// Get time
				TimeValue time=theCNelExport.ip->GetTime();

				// View mesh
				theCNelExport.viewMesh (*theCNelExport.ip, time);
				}
				break;
			// ---
			case ID_SAVESWT:
				{
					uint nNumSelNode = theCNelExport.ip->GetSelNodeCount();

					// Save all selected objects
					if (nNumSelNode)
					{		
						std::vector<INode*> vectNode;
						theCNelExport.getSelectedNode (vectNode);
						nlassert (vectNode.size()!=0);

						char sDialogMsg[256];
						sprintf (sDialogMsg, "Save SWT...");

						// Save path
						char sSavePath[256];
						strcpy (sSavePath, (*vectNode.begin())->GetName());

						if (theCNelExport.SelectFileForSave (hWnd, sDialogMsg, SWTFilter, sSavePath))
						{
							// Export the zone
							if (!theCNelExport.exportSWT (sSavePath, vectNode, *theCNelExport.ip))
							{
								// Error message
								char sErrorMsg[512];
								sprintf (sErrorMsg, "Error exporting SWT %s in the file\n%s", (*vectNode.begin())->GetName(), sSavePath);
								MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
							}
						}
					}
				}
				break;
				// ---
				case ID_EXPORTINSTANCEGROUP:
				{
					uint nNumSelNode = theCNelExport.ip->GetSelNodeCount();

					// Save all selected objects
					if (nNumSelNode)
					{		
						std::vector<INode*> vectNode;
						theCNelExport.getSelectedNode (vectNode);
						nlassert (vectNode.size()!=0);

						char sDialogMsg[256];
						sprintf (sDialogMsg, "Save SWT...");

						// Save path
						char sSavePath[256];
						strcpy (sSavePath, (*vectNode.begin())->GetName());

						if (theCNelExport.SelectFileForSave (hWnd, sDialogMsg, InstanceGroupFilter, sSavePath))
						{
							// Export the zone
							if (!theCNelExport.exportScene (sSavePath, vectNode, *theCNelExport.ip))
							{
								// Error message
								char sErrorMsg[512];
								sprintf (sErrorMsg, "Error exporting scene %s in the file\n%s", (*vectNode.begin())->GetName(), sSavePath);
								MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
							}
						}
					}
				}
				break;
				// ---
				case ID_SAVESKELETON:
				{
					uint nNumSelNode = theCNelExport.ip->GetSelNodeCount();
					if (nNumSelNode!=1)
					{
						// Select only the root of the skeleton
						MessageBox (hWnd, "Select the root node of the skeleton.", "NeL export", MB_OK|MB_ICONEXCLAMATION);
					}
					else
					{
						// Get the selected node
						INode* pNode=theCNelExport.ip->GetSelNode (0);

						// Save path
						char sSavePath[256];
						strcpy (sSavePath, "*.skel");

						// Select the file
						if (theCNelExport.SelectFileForSave (hWnd, "Save skeleton...", skeletonFilter, sSavePath))
						{
							// Export the zone
							if (!theCNelExport.exportSkeleton (sSavePath, pNode, *theCNelExport.ip, theCNelExport.ip->GetTime()))
							{
								// Error message
								char sErrorMsg[512];
								sprintf (sErrorMsg, "Error exporting skeleton %s in the file\n%s", pNode->GetName(), sSavePath);
								MessageBox (hWnd, sErrorMsg, "NeL export", MB_OK|MB_ICONEXCLAMATION);
							}
						}
					}
				}
				break;
			}
			
			break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theCNelExport.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}



//--- CNelExport -------------------------------------------------------
CNelExport::CNelExport()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

CNelExport::~CNelExport()
{

}

void CNelExport::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(hInstance,MAKEINTRESOURCE(IDD_PANEL),CNelExportDlgProc,GetString(IDS_PARAMS),0);
	hBar=GetDlgItem(hPanel,ID_BAR);
	SendMessage(hBar,PBM_SETPOS,0,0);
}
	
void CNelExport::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void CNelExport::Init(HWND hWnd)
{

}

void CNelExport::Destroy(HWND hWnd)
{

}

void CNelExport::getSelectedNode (std::vector<INode*>& vectNode)
{
	// Get node count
	uint nNumSelNode=ip->GetSelNodeCount();

	// Save all selected objects
	for (uint nNode=0; nNode<nNumSelNode; nNode++)
	{
		// Get the node
		vectNode.push_back (theCNelExport.ip->GetSelNode (nNode));
	}
}
