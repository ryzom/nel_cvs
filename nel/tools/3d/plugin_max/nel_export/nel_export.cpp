/** \file nel_export.cpp
 * <File description>
 *
 * $Id: nel_export.cpp,v 1.22 2002/02/26 17:30:23 corvazier Exp $
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
#include "3d/register_3d.h"
#include "3d/skeleton_shape.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_lod.h"
#include "../nel_patch_lib/rpo.h"
#include "nel_export_scene.h"
 
using namespace NL3D;
using namespace NLMISC;

CNelExport theCNelExport;
//CNelExportSceneStruct theExportSceneStruct;
CExportNelOptions theExportSceneStruct;


CNelExportClassDesc CNelExportDesc;
ClassDesc2* GetCNelExportDesc() {return &CNelExportDesc;}

static const char *zoneFilter="NeL zone file (*.zone)\0*.zone\0All files (*.*)\0*.*\0";
static const char *meshFilter="NeL shape file (*.shape)\0*.shape\0All files (*.*)\0*.*\0";
static const char *collisionFilter="NeL collision file (*.cmb)\0*.cmb\0All files (*.*)\0*.*\0";
static const char *animModelFilter="NeL model animation file (*.anim)\0*.anim\0All files (*.*)\0*.*\0";
static const char *SWTFilter="NeL Skeleton Weight Template file (*.swt)\0*.swt\0All files (*.*)\0*.*\0";
static const char *InstanceGroupFilter="NeL Instance Group file (*.ig)\0*.ig\0All files (*.*)\0*.*\0";
static const char *skeletonFilter="NeL Skeleton file (*.skel)\0*.skel\0All files (*.*)\0*.*\0";
static const char *vegetableFilter="NeL Vegetable file (*.veget)\0*.veget\0All files (*.*)\0*.*\0";


void *CNelExportClassDesc::Create(BOOL loading)
{
	return &theCNelExport;
}


int CALLBACK OptionsDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			char tmp[1024];
			CenterWindow( hwndDlg, theCNelExport.ip->GetMAXHWnd() );
			ShowWindow( hwndDlg, TRUE );
			// Initialize from theExportSceneStruct
			if( theExportSceneStruct.bExcludeNonSelected )
				SendMessage( GetDlgItem(hwndDlg,IDC_EXCLUDE), BM_SETCHECK, BST_CHECKED, 0 );
			else
				SendMessage( GetDlgItem(hwndDlg,IDC_EXCLUDE), BM_SETCHECK, BST_UNCHECKED, 0 );
			if( theExportSceneStruct.bExportLighting )
				SendMessage( GetDlgItem(hwndDlg,IDC_CHECKEXPORTLIGHTING), BM_SETCHECK, BST_CHECKED, 0 );
			else
				SendMessage( GetDlgItem(hwndDlg,IDC_CHECKEXPORTLIGHTING), BM_SETCHECK, BST_UNCHECKED, 0 );
			if( theExportSceneStruct.bShadow)
				SendMessage( GetDlgItem(hwndDlg,IDC_SHADOW), BM_SETCHECK, BST_CHECKED, 0 );
			else
				SendMessage( GetDlgItem(hwndDlg,IDC_SHADOW), BM_SETCHECK, BST_UNCHECKED, 0 );
			SendMessage( GetDlgItem(hwndDlg,IDC_EDITEXPORTLIGHTING), WM_SETTEXT, 0, (long)theExportSceneStruct.sExportLighting.c_str() );
			if( theExportSceneStruct.nExportLighting == 0 )
				SendMessage( GetDlgItem(hwndDlg,IDC_RADIONORMALEXPORTLIGHTING), BM_SETCHECK, BST_CHECKED, 0 );
			if( theExportSceneStruct.nExportLighting == 1 )
				SendMessage( GetDlgItem(hwndDlg,IDC_RADIORADIOSITYEXPORTLIGHTING), BM_SETCHECK, BST_CHECKED, 0 );
			sprintf( tmp, "%f", theExportSceneStruct.rLumelSize );
			SendMessage( GetDlgItem(hwndDlg,IDC_EDITLUMELSIZE), WM_SETTEXT, 0, (long)tmp );
			if( theExportSceneStruct.nOverSampling == 1 )
				SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS1), BM_SETCHECK, BST_CHECKED, 0 );
			if( theExportSceneStruct.nOverSampling == 2 )
				SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS2), BM_SETCHECK, BST_CHECKED, 0 );
			if( theExportSceneStruct.nOverSampling == 4 )
				SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS3), BM_SETCHECK, BST_CHECKED, 0 );
			if( theExportSceneStruct.nOverSampling == 8 )
				SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS4), BM_SETCHECK, BST_CHECKED, 0 );
			if( theExportSceneStruct.bShowLumel )
				SendMessage( GetDlgItem(hwndDlg,IDC_SHOWLUMEL), BM_SETCHECK, BST_CHECKED, 0 );
			else
				SendMessage( GetDlgItem(hwndDlg,IDC_SHOWLUMEL), BM_SETCHECK, BST_UNCHECKED, 0 );
			if( theExportSceneStruct.bExportBgColor )
				SendMessage( GetDlgItem(hwndDlg,IDC_EXPORT_BG_COLOR), BM_SETCHECK, BST_CHECKED, 0 );
			else
				SendMessage( GetDlgItem(hwndDlg,IDC_EXPORT_BG_COLOR), BM_SETCHECK, BST_UNCHECKED, 0 );
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			switch (LOWORD(wParam)) 
			{
				case IDC_BUTTONEXPORTLIGHTING:
				{
					char sTemp[1024];
					strcpy(sTemp,theExportSceneStruct.sExportLighting.c_str());
					if( theCNelExport.SelectDir(hwndDlg, "LightMaps Directory", sTemp ) )
					{
						theExportSceneStruct.sExportLighting = sTemp;
						SendMessage( GetDlgItem(hwndDlg,IDC_EDITEXPORTLIGHTING), WM_SETTEXT, 0, (long)theExportSceneStruct.sExportLighting.c_str() );
					}
				}
				break;
				case IDCANCEL:
					EndDialog(hwndDlg, FALSE);
				break;
				case IDOK:
				{
					char tmp[1024];
					// The result goes in theExportSceneStruct
					if( SendMessage( GetDlgItem(hwndDlg,IDC_EXCLUDE), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.bExcludeNonSelected = true;
					else
						theExportSceneStruct.bExcludeNonSelected = false;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_SHADOW), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.bShadow = true;
					else
						theExportSceneStruct.bShadow = false;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_CHECKEXPORTLIGHTING), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.bExportLighting = true;
					else
						theExportSceneStruct.bExportLighting = false;
					SendMessage( GetDlgItem(hwndDlg,IDC_EDITEXPORTLIGHTING), WM_GETTEXT, 1024, (long)tmp );
					theExportSceneStruct.sExportLighting = tmp;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_RADIONORMALEXPORTLIGHTING), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.nExportLighting = 0;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_RADIORADIOSITYEXPORTLIGHTING), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.nExportLighting = 1;
					SendMessage( GetDlgItem(hwndDlg,IDC_EDITLUMELSIZE), WM_GETTEXT, 1024, (long)tmp );
					theExportSceneStruct.rLumelSize = (float)atof( tmp );

					if( SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS1), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.nOverSampling = 1;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS2), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.nOverSampling = 2;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS3), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.nOverSampling = 4;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_RADIOSS4), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.nOverSampling = 8;
					if( SendMessage( GetDlgItem(hwndDlg,IDC_SHOWLUMEL), BM_GETCHECK, 0, 0 ) == BST_CHECKED )
						theExportSceneStruct.bShowLumel = true;
					else
						theExportSceneStruct.bShowLumel = false;
					theExportSceneStruct.bExportBgColor = ( SendMessage( GetDlgItem(hwndDlg,IDC_EXPORT_BG_COLOR), BM_GETCHECK, 0, 0 ) == BST_CHECKED );

					// End the dialog
					EndDialog(hwndDlg, TRUE);
				}
				break;
			}
		break;

		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:						
		break;
	
		default:
		return FALSE;
	}
	return TRUE;
}

static BOOL CALLBACK CNelExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Set locale to english
	setlocale (LC_NUMERIC, "English");

	BOOL ret = TRUE;
	switch (msg) 
	{
		case WM_INITDIALOG:
			{
				theCNelExport.Init(hWnd);

				// Get the module path
				HMODULE hModule = GetModuleHandle("nelexport.dlu");
				if (hModule)
				{
					// Get module file name
					char moduldeFileName[512];
					if (GetModuleFileName (hModule, moduldeFileName, 512))
					{
						// Get version info size
						DWORD doomy;
						uint versionInfoSize=GetFileVersionInfoSize (moduldeFileName, &doomy);
						if (versionInfoSize)
						{
							// Alloc the buffer
							char *buffer=new char[versionInfoSize];

							// Find the verion resource
							if (GetFileVersionInfo(moduldeFileName, 0, versionInfoSize, buffer))
							{
								uint *versionTab;
								uint versionSize;
								if (VerQueryValue (buffer, "\\", (void**)&versionTab,  &versionSize))
								{
									// Get the pointer on the structure
									VS_FIXEDFILEINFO *info=(VS_FIXEDFILEINFO*)versionTab;
									if (info)
									{
 										// Setup version number
										char version[512];
										sprintf (version, "Version %d.%d.%d.%d", 
											info->dwFileVersionMS>>16, 
											info->dwFileVersionMS&0xffff, 
											info->dwFileVersionLS>>16,  
											info->dwFileVersionLS&0xffff);
										SetWindowText (GetDlgItem (hWnd, IDC_VERSION), version);
									}
									else
										SetWindowText (GetDlgItem (hWnd, IDC_VERSION), "VS_FIXEDFILEINFO * is NULL");
								}
								else
									SetWindowText (GetDlgItem (hWnd, IDC_VERSION), "VerQueryValue failed");
							}
							else
								SetWindowText (GetDlgItem (hWnd, IDC_VERSION), "GetFileVersionInfo failed");

							// Free the buffer
							delete [] buffer;
						}
						else
							SetWindowText (GetDlgItem (hWnd, IDC_VERSION), "GetFileVersionInfoSize failed");
					}
					else
						SetWindowText (GetDlgItem (hWnd, IDC_VERSION), "GetModuleFileName failed");
				}
				else
					SetWindowText (GetDlgItem (hWnd, IDC_VERSION), "GetModuleHandle failed");
			}
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
					// Load the options
					theCNelExport.initOptions();

					// Register 3d models
					// done in dllentry registerSerial3d();

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
						else if (CExportNel::isVegetable (*pNode, time))
						{
							// Save path
							char sSavePath[256];
							strcpy (sSavePath, pNode->GetName());

							// Choose a file to export
							if (!CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_DONTEXPORT, 0))
							if (theCNelExport.SelectFileForSave(hWnd, sNodeMsg, vegetableFilter, sSavePath))
							{
								// Export the mesh
								if (!theCNelExport.exportVegetable (sSavePath, *pNode, *theCNelExport.ip, time, true))
								{
									// Error message
									char sErrorMsg[512];
									sprintf (sErrorMsg, "Error exporting the vegetable %s in the file\n%s", pNode->GetName(), sSavePath);
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
							if (!CExportNel::getScriptAppData (pNode, NEL3D_APPDATA_DONTEXPORT, 0))
							if (theCNelExport.SelectFileForSave(hWnd, sNodeMsg, meshFilter, sSavePath))
							{
								// Skin objects
								CSkeletonShape *pSkinShape=NULL;

								CExportNel::deleteLM( *pNode, theExportSceneStruct );
								// Export the mesh
								if (!theCNelExport.exportMesh (sSavePath, *pNode, *theCNelExport.ip, time, theExportSceneStruct, true))
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
					// done in dllentry registerSerial3d();

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
						if (theCNelExport.SelectFileForSave (hWnd, sNodeMsg, (LOWORD(wParam)==ID_SAVE_MODEL_ANIM)?animModelFilter:animModelFilter, 
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
			case ID_SAVECOLLISION:
				{
					// Get time
					TimeValue time=theCNelExport.ip->GetTime();
					
					// Get node count
					int nNumSelNode=theCNelExport.ip->GetSelNodeCount();

					std::vector<INode *>	nodes;

					// Save all selected objects
					for (int nNode=0; nNode<nNumSelNode; nNode++)
					{
						// Get the node
						INode* pNode=theCNelExport.ip->GetSelNode (nNode);

						// It is a zone ?
						if (RPO::isZone (*pNode, time))
						{
						}
						// Try to export a mesh
						else if (CExportNel::isMesh (*pNode, time))
						{
							nodes.push_back(pNode);
						}
					}

					// Name of the node
					char sNodeMsg[256];
					sprintf (sNodeMsg, "Save collision mesh build...");

					// Save path
					char sSavePath[256];
					strcpy (sSavePath, "");

					// Choose a file to export
					//	ULONG SelectDir(HWND Parent, char* Title, char* Path);
//					if (theCNelExport.SelectFileForSave(hWnd, sNodeMsg, collisionFilter, sSavePath))
					if (theCNelExport.SelectDir(hWnd, sNodeMsg, sSavePath))
					{
						// Export the mesh
						if (!theCNelExport.exportCollision (sSavePath, nodes, *theCNelExport.ip, time, theExportSceneStruct))
						{
							// Error message
							MessageBox (hWnd, "Error during export collision", "NeL export", MB_OK|MB_ICONEXCLAMATION);
						}
					}

				}
				break;
			// ---
			case ID_VIEW:
				{
				// Get time
				TimeValue time=theCNelExport.ip->GetTime();
				theCNelExport.initOptions();
				// View mesh
				theCNelExport.viewMesh (*theCNelExport.ip, time, theExportSceneStruct);
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
							// Export the swt
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
				case ID_OPTIONS:
				{
					char sConfigFileName[512];
					strcpy( sConfigFileName, theCNelExport.ip->GetDir(APP_PLUGCFG_DIR) );
					strcat( sConfigFileName, "\\NelExportScene.cfg" );

					theCNelExport.initOptions();

					// Do a modal dialog box to choose the scene export options
					if( DialogBox(	hInstance,
									MAKEINTRESOURCE(IDD_EXPORTSCENE),
									theCNelExport.ip->GetMAXHWnd(),
									OptionsDialogCallback		) )
					{
						// Write configuration file
						{
							COFile outputFile;
							if( outputFile.open(sConfigFileName) )
								theExportSceneStruct.serial( outputFile );
						}							
						// EXPORT THE SCENE
						// theCNelExport.exportScene( vectNode );
					}
				}
				break;
				// ---
				case ID_NODE_PROPERTIES:
				{
					// Build a seleted set
					std::set<INode*> listNode;

					// Get the sel node count
					uint selNodeCount=theCNelExport.ip->GetSelNodeCount();
					for (uint i=0; i<selNodeCount; i++)
					{
						// insert the node
						listNode.insert (theCNelExport.ip->GetSelNode(i));
					}

					// Call the dialog
					theCNelExport.OnNodeProperties (listNode);
					break;
				}
				break;
				// ---
				case ID_EXPORTINSTANCEGROUP:
				{
					uint nNumSelNode = theCNelExport.ip->GetSelNodeCount();

					// done in dllentry registerSerial3d();
					// All the selected nodes are considered as a scene					
					if( nNumSelNode > 0 )
					{		
						std::vector<INode*> vectNode;
						theCNelExport.getSelectedNode (vectNode);
						nlassert (vectNode.size()!=0);

						char sSavePath[256];
						strcpy (sSavePath, (*vectNode.begin())->GetName());

						if (theCNelExport.SelectFileForSave (hWnd, "Save Instance group", InstanceGroupFilter, sSavePath))
						{
							// Export the instance group
							if (!theCNelExport.exportInstanceGroup( sSavePath, vectNode, *theCNelExport.ip))
							{
								// Error message
								char sErrorMsg[512];
								sprintf (sErrorMsg, "Error exporting instance group %s", sSavePath);
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
			ret = FALSE;
	}

	// Set locale to current
	setlocale (LC_NUMERIC, "");

	return ret;
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

void CNelExport::initOptions()
{
	// Initialization of theExportSceneStruct
	char sConfigFileName[512];
	strcpy( sConfigFileName, theCNelExport.ip->GetDir(APP_PLUGCFG_DIR) );
	strcat( sConfigFileName, "\\NelExportScene.cfg" );
	// MessageBox (hWnd, sConfigFileName, "sConfigFileName", MB_OK|MB_ICONEXCLAMATION);
	if( theCNelExport.FileExists(sConfigFileName) )
	{
		// Serial the configuration
		try {
			CIFile inputFile;
			if( inputFile.open(sConfigFileName) )
			{
				theExportSceneStruct.serial( inputFile );
				return;
			}
		}
		catch(...)
		{
			MessageBox( theCNelExport.ip->GetMAXHWnd(), "NelExportScene.cfg corrupted or old version", 
						"Error", MB_OK|MB_ICONEXCLAMATION );
		}
	}
}