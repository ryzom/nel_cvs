/** \file nel_export_node_properties.cpp
 * Node properties dialog
 *
 * $Id: nel_export_node_properties.cpp,v 1.26 2002/03/13 10:59:41 corvazier Exp $
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
#include "../nel_mesh_lib/export_lod.h"
#include "../nel_mesh_lib/calc_lm.h"
#include "../nel_patch_lib/nel_patch_mesh.h"

using namespace NLMISC;

// ***************************************************************************

#define TAB_COUNT	7
#define VP_COUNT	1
// Which dialog tab is the VerytexProgram one?
#define TAB_VP_ID	5


// ***************************************************************************

const std::set<INode*> *listNodeCallBack;

// ***************************************************************************

class addSubLodNodeHitCallBack : public HitByNameDlgCallback 
{
public:
	INodeTab	NodeTab;
private:
	virtual TCHAR *dialogTitle()
	{
		return "Select sub lod objects to add";
	}
	virtual TCHAR *buttonText()
	{
		return "Add";
	}
	virtual BOOL singleSelect()
	{
		return FALSE;
	}
	virtual BOOL useFilter()
	{
		return TRUE;
	}
	virtual int filter(INode *node)
	{
		return (int)(listNodeCallBack->find (node)==listNodeCallBack->end());
	}
	virtual BOOL useProc()
	{
		return TRUE;
	}
	virtual void proc(INodeTab &nodeTab)
	{
		NodeTab=nodeTab;
	}
	virtual BOOL doCustomHilite()
	{
		return FALSE;
	}
	virtual BOOL showHiddenAndFrozen()
	{
		return TRUE;
	}
};

// ***************************************************************************

class CLodDialogBoxParam
{
public:
	CLodDialogBoxParam ()
	{
		uint i;
		for (i=0; i<TAB_COUNT; i++)
			SubDlg[i] = NULL;
		for (i=0; i<VP_COUNT; i++)
			SubVPDlg[i] = NULL;
	}

	bool					ListActived;
	std::list<std::string>	ListLodName;
	int						BlendIn;
	int						BlendOut;
	int						CoarseMesh;
	int						DynamicMesh;
	std::string				DistMax;
	std::string				BlendLength;
	int						MRM;
	int						SkinReduction;
	std::string				NbLod;
	std::string				Divisor;
	std::string				DistanceFinest;
	std::string				DistanceMiddle;
	std::string				DistanceCoarsest;

	const std::set<INode*> *ListNode;

	int						AccelType; // -1->undeterminate   0->Not  1->Portal  2->Cluster
										// 3rd bit -> Father visible
										// 4th bit -> Visible from father
										// 5th bit -> Dynamic Portal
										// 6th bit -> Clusterize
	std::string				InstanceShape;
	std::string				InstanceName;
	std::string				InstanceGroupName;
	int						DontAddToScene;	
	int						DontExport;

	// Lighting
	std::string				LumelSizeMul;
	std::string				SoftShadowRadius;
	std::string				SoftShadowConeLength;


	// VertexProgram.
	int						VertexProgramId;
	// WindTree VertexProgram.
	CVPWindTreeAppData		VertexProgramWindTree;


	// misc
	int						ExportRealTimeLight;
	int						ExportLightMapLight;
	int						ExportAsSunLight;
	int						UseLightingLocalAttenuation;
	int						ExportLightMapAnimated;
	std::string				ExportLightMapName;

	// misc
	int						FloatingObject;
	int						ExportNoteTrack;
	int						ExportAnimatedMaterials;
	int						SWT;
	std::string				SWTWeight;
	int						LigoSymmetry;
	std::string				LigoRotate;


	// Vegetable
	int						Vegetable;
	int						VegetableAlphaBlend;
	int						VegetableAlphaBlendOnLighted;
	int						VegetableAlphaBlendOffLighted;
	int						VegetableAlphaBlendOffDoubleSided;
	int						VegetableBendCenter;
	std::string				VegetableBendFactor;


	// Collision
	int						Collision;
	int						CollisionExterior;


	// Dialog
	HWND					SubDlg[TAB_COUNT];

	// Dialog
	HWND					SubVPDlg[VP_COUNT];
};

int CALLBACK MRMDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK AccelDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK InstanceDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK LightmapDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK VegetableDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK MiscDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK VertexProgramDialogCallBack (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

const char				*SubText[TAB_COUNT]	= {"LOD & MRM", "Accelerator", "Instance", "Lighting", "Vegetable", "VertexProgram", "Misc"};
const int				SubTab[TAB_COUNT]	= {IDD_LOD, IDD_ACCEL, IDD_INSTANCE, IDD_LIGHTMAP, IDD_VEGETABLE, IDD_VERTEX_PROGRAM, IDD_MISC};
DLGPROC					SubProc[TAB_COUNT]	= {MRMDialogCallback, AccelDialogCallback, InstanceDialogCallback, LightmapDialogCallback, VegetableDialogCallback, VertexProgramDialogCallBack, MiscDialogCallback};

// VertexPrograms.
int CALLBACK VPWindTreeCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
const int				SubVPTab[VP_COUNT]	= {IDD_VP_WINDTREE};
DLGPROC					SubVPProc[VP_COUNT]	= {VPWindTreeCallback};


// The last opened TAB.
static int				LastTabActivated= 0;


// ***************************************************************************

void MRMStateChanged (HWND hwndDlg)
{
	bool enable = ( SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0)!=BST_UNCHECKED ) &&
		( SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0)==BST_UNCHECKED );
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_MIN), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_MAX), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_BEST), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_NB_LOD), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIVISOR), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_FINEST), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), enable);
}

// ***************************************************************************

void LightingStateChanged (HWND hwndDlg, CLodDialogBoxParam *currentParam)
{
	bool lightmapLight = (SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_LIGHT), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
	lightmapLight |= (SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
	lightmapLight |= (SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_ANIMATED), BM_GETCHECK, 0, 0)!=BST_UNCHECKED);
	EnableWindow (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), lightmapLight);
}

// ***************************************************************************

void CoarseStateChanged (HWND hwndDlg)
{
	// Like if MRM button was clicked
	MRMStateChanged (hwndDlg);

	// Bouton enabled ?
	bool enable = SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0)==BST_UNCHECKED;
	EnableWindow (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), enable);
}

// ***************************************************************************

void VegetableStateChanged (HWND hwndDlg)
{
	// Vegetable ?
	bool enable = ( SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0)!=BST_UNCHECKED );
	
	// Enable alpha blend button
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CENTER_Z), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CENTER_NULL), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), enable);
	
	// Alpha blend ?
	bool alphaBlend = IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON)!=BST_UNCHECKED;

	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED), enable && alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_ON_UNLIGHTED), enable && alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_UNLIGHTED), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), enable && !alphaBlend);
}


// ***************************************************************************
void InstanceStateChanged (HWND hwndDlg)
{
	bool	colEnable= ( SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION), BM_GETCHECK, 0, 0)!=BST_UNCHECKED );

	EnableWindow (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION_EXTERIOR), colEnable);
}

// ***************************************************************************

int CALLBACK AccelDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			if (currentParam->AccelType != -1)
			{
				CheckRadioButton (hwndDlg, IDC_RADIOACCELNO, IDC_RADIOACCELCLUSTER, IDC_RADIOACCELNO+(currentParam->AccelType&3));
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), true);
				if ((currentParam->AccelType&3) == 2) // Cluster ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), BM_SETCHECK, currentParam->AccelType&4, 0);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), BM_SETCHECK, currentParam->AccelType&8, 0);
				}
				else
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				}
				
				if ((currentParam->AccelType&3) == 1) // Portal ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), BM_SETCHECK, currentParam->AccelType&16, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);

				if ((currentParam->AccelType&3) == 0) // Not an accelerator
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), BM_SETCHECK, currentParam->AccelType&32, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);

			}
			else
			{
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);
			}
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELNO) == BST_CHECKED)
								currentParam->AccelType = 0;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELPORTAL) == BST_CHECKED)
								currentParam->AccelType = 1;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELCLUSTER) == BST_CHECKED)
								currentParam->AccelType = 2;

							if (IsDlgButtonChecked (hwndDlg, IDC_FATHER_VISIBLE) == BST_CHECKED)
								currentParam->AccelType |= 4;
							if (IsDlgButtonChecked (hwndDlg, IDC_VISIBLE_FROM_FATHER) == BST_CHECKED)
								currentParam->AccelType |= 8;
							if (IsDlgButtonChecked (hwndDlg, IDC_DYNAMIC_PORTAL) == BST_CHECKED)
								currentParam->AccelType |= 16;
							if (IsDlgButtonChecked (hwndDlg, IDC_CLUSTERIZE) == BST_CHECKED)
								currentParam->AccelType |= 32;

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
					case IDC_RADIOACCELNO:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), true);
						break;
					case IDC_RADIOACCELPORTAL:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_RADIOACCELCLUSTER:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_FATHER_VISIBLE:
					case IDC_VISIBLE_FROM_FATHER:
					case IDC_DYNAMIC_PORTAL:
					case IDC_CLUSTERIZE:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}


// ***************************************************************************
int CALLBACK MRMDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Window text
			std::string winName=(*(currentParam->ListNode->begin()))->GetName();
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, winName.c_str());

			// Set default state
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_SETCHECK, currentParam->BlendIn, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_SETCHECK, currentParam->BlendOut, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_SETCHECK, currentParam->CoarseMesh, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_SETCHECK, currentParam->DynamicMesh, 0);


			EnableWindow (GetDlgItem (hwndDlg, IDC_LIST1), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_ADD), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_REMOVE), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_UP), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_DOWN), currentParam->ListActived);
			
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), currentParam->DistMax.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), currentParam->BlendLength.c_str());

			SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_SETCHECK, currentParam->MRM, 0);
			CoarseStateChanged (hwndDlg);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			SetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), currentParam->NbLod.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), currentParam->Divisor.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), currentParam->DistanceFinest.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), currentParam->DistanceMiddle.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), currentParam->DistanceCoarsest.c_str());

			// Iterate list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
			std::list<std::string>::iterator ite=currentParam->ListLodName.begin();
			while (ite!=currentParam->ListLodName.end())
			{
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) ite->c_str());
				ite++;
			}
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							currentParam->BlendIn=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_GETCHECK, 0, 0);
							currentParam->BlendOut=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_GETCHECK, 0, 0);
							currentParam->CoarseMesh=SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0);
							currentParam->DynamicMesh=SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_GETCHECK, 0, 0);
							
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), tmp, 512);
							currentParam->DistMax=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), tmp, 512);
							currentParam->BlendLength=tmp;

							currentParam->MRM=SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0);

							currentParam->SkinReduction=-1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MIN)==BST_CHECKED)
								currentParam->SkinReduction=0;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MAX)==BST_CHECKED)
								currentParam->SkinReduction=1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_BEST)==BST_CHECKED)
								currentParam->SkinReduction=2;

							GetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), tmp, 512);
							currentParam->NbLod=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), tmp, 512);
							currentParam->Divisor=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), tmp, 512);
							currentParam->DistanceFinest=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), tmp, 512);
							currentParam->DistanceMiddle=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), tmp, 512);
							currentParam->DistanceCoarsest=tmp;

							// Iterate list
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
							currentParam->ListLodName.clear ();

							// Insert item in the list
							uint itemCount=SendMessage (hwndList, LB_GETCOUNT, 0, 0);
							for (uint item=0; item<itemCount; item++)
							{
								// Get the string
								SendMessage (hwndList, LB_GETTEXT, item, (LPARAM) tmp);

								// Push it back
								currentParam->ListLodName.push_back (tmp);
							}
						}
					break;
					case IDC_ADD:
						{
							// Callback for the select node dialog
							addSubLodNodeHitCallBack callBack;
							listNodeCallBack=currentParam->ListNode;
							if (theCNelExport.ip->DoHitByNameDialog(&callBack))
							{
								// Add the selected object in the list
								HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
								for (uint i=0; i<(uint)callBack.NodeTab.Count(); i++)
									SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) callBack.NodeTab[i]->GetName());
							}
						}
						break;
					case IDC_REMOVE:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if (sel!=LB_ERR)
							{
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								if (sel==SendMessage (hwndList, LB_GETCOUNT, 0, 0))
									sel--;
								if (sel>=0)
									SendMessage (hwndList, LB_SETCURSEL, sel, 0);
							}
						}
					break;
					case IDC_UP:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ((sel!=LB_ERR)&&(sel>0))
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move up the item
								SendMessage (hwndList, LB_INSERTSTRING, sel-1, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel+1, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel-1, 0);
							}
						}
					break;
					case IDC_DOWN:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ( (sel!=LB_ERR) && (sel<SendMessage (hwndList, LB_GETCOUNT, 0, 0)-1 ) )
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move down the item
								SendMessage (hwndList, LB_INSERTSTRING, sel+2, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel+1, 0);
							}
						}
					break;
					// 3 states management
					case IDC_COARSE_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							CoarseStateChanged (hwndDlg);
						}
						break;
					case IDC_ACTIVE_MRM:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							MRMStateChanged (hwndDlg);
						}
						break;
					case IDC_BLEND_IN:
					case IDC_BLEND_OUT:
					case IDC_DYNAMIC_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					char name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport.ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}




// ***************************************************************************
int CALLBACK InstanceDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_SHAPE), currentParam->InstanceShape.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), currentParam->InstanceName.c_str());
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_SETCHECK, currentParam->DontAddToScene, 0);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), currentParam->InstanceGroupName.c_str());

			EnableWindow (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), true);
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_SETCHECK, currentParam->DontExport, 0);

			SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION), BM_SETCHECK, currentParam->Collision, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION_EXTERIOR), BM_SETCHECK, currentParam->CollisionExterior, 0);

			InstanceStateChanged(hwndDlg);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_SHAPE), tmp, 512);
							currentParam->InstanceShape=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), tmp, 512);
							currentParam->InstanceName=tmp;
							currentParam->DontAddToScene=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), tmp, 512);
							currentParam->InstanceGroupName=tmp;
							currentParam->DontExport=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_GETCHECK, 0, 0);

							currentParam->Collision= SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION), BM_GETCHECK, 0, 0);
							currentParam->CollisionExterior= SendMessage (GetDlgItem (hwndDlg, IDC_CHECK_COLLISION_EXTERIOR), BM_GETCHECK, 0, 0);
						}
					break;
					case IDC_DONT_ADD_TO_SCENE:
					case IDC_DONT_EXPORT:
					case IDC_CHECK_COLLISION:
					case IDC_CHECK_COLLISION_EXTERIOR:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						// if change collision state
						if ( LOWORD(wParam) == IDC_CHECK_COLLISION )
							InstanceStateChanged(hwndDlg);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}



// ***************************************************************************
int CALLBACK LightmapDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), currentParam->LumelSizeMul.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), currentParam->SoftShadowRadius.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), currentParam->SoftShadowConeLength.c_str());

			// Lighting
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_SETCHECK, currentParam->ExportRealTimeLight, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_LIGHT), BM_SETCHECK, currentParam->ExportLightMapLight, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_ANIMATED), BM_SETCHECK, currentParam->ExportLightMapAnimated, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_AS_SUN_LIGHT), BM_SETCHECK, currentParam->ExportAsSunLight, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_USE_LIGHT_LOCAL_ATTENUATION), BM_SETCHECK, currentParam->UseLightingLocalAttenuation, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), currentParam->ExportLightMapName.c_str());

			// Set enable disable
			LightingStateChanged (hwndDlg, currentParam);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDC_RESET_NAME:
						SetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), "GlobalLight");
					break;
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), tmp, 512);
							currentParam->LumelSizeMul = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), tmp, 512);
							currentParam->SoftShadowRadius = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), tmp, 512);
							currentParam->SoftShadowConeLength = tmp;

							// RealTime light
							currentParam->ExportRealTimeLight = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_REALTIME_LIGHT), BM_GETCHECK, 0, 0);
							currentParam->ExportLightMapLight = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_LIGHT), BM_GETCHECK, 0, 0);
							currentParam->ExportAsSunLight = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_AS_SUN_LIGHT), BM_GETCHECK, 0, 0);
							currentParam->UseLightingLocalAttenuation = SendMessage (GetDlgItem (hwndDlg, IDC_USE_LIGHT_LOCAL_ATTENUATION), BM_GETCHECK, 0, 0);
							currentParam->ExportLightMapAnimated = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_ANIMATED), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EXPORT_LIGHTMAP_NAME), tmp, 512);
							currentParam->ExportLightMapName = tmp;
						}
					break;
					case IDC_EXPORT_REALTIME_LIGHT:
					case IDC_USE_LIGHT_LOCAL_ATTENUATION:
					case IDC_EXPORT_LIGHTMAP_LIGHT:
					case IDC_EXPORT_LIGHTMAP_ANIMATED:
					case IDC_EXPORT_AS_SUN_LIGHT:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);

						// Set enable disable
						LightingStateChanged (hwndDlg, currentParam);

						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					char name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport.ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
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




// ***************************************************************************
int CALLBACK VegetableDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_SETCHECK, currentParam->Vegetable, 0);
			
			CheckRadioButton(hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON, IDC_VEGETABLE_ALPHA_BLEND_OFF, IDC_VEGETABLE_ALPHA_BLEND_ON+currentParam->VegetableAlphaBlend);
			
			CheckRadioButton(hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED, IDC_VEGETABLE_AB_ON_UNLIGHTED, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED+currentParam->VegetableAlphaBlendOnLighted);
			
			CheckRadioButton(hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED, IDC_VEGETABLE_AB_OFF_UNLIGHTED, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED+currentParam->VegetableAlphaBlendOffLighted);
			
			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), BM_SETCHECK, currentParam->VegetableAlphaBlendOffDoubleSided, 0);
			
			CheckRadioButton(hwndDlg, IDC_CENTER_NULL, IDC_CENTER_Z, IDC_CENTER_NULL+currentParam->VegetableBendCenter);

			SetWindowText (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), currentParam->VegetableBendFactor.c_str());

			VegetableStateChanged (hwndDlg);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->Vegetable=SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0);

							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON) == BST_CHECKED)
								currentParam->VegetableAlphaBlend = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF) == BST_CHECKED)
								currentParam->VegetableAlphaBlend = 1;
							else 
								currentParam->VegetableAlphaBlend = -1;
							
							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOnLighted = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_UNLIGHTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOnLighted = 1;
							else 
								currentParam->VegetableAlphaBlendOnLighted = -1;
							
							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 1;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_UNLIGHTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 2;
							else 
								currentParam->VegetableAlphaBlendOffLighted = -1;
							
							currentParam->VegetableAlphaBlendOffDoubleSided = SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), BM_GETCHECK, 0, 0);

							if (IsDlgButtonChecked (hwndDlg, IDC_CENTER_NULL) == BST_CHECKED)
								currentParam->VegetableBendCenter = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_CENTER_Z) == BST_CHECKED)
								currentParam->VegetableBendCenter = 1;
							else 
								currentParam->VegetableBendCenter = -1;
							
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), tmp, 512);
							currentParam->VegetableBendFactor = tmp;
						}
					break;
					case IDC_VEGETABLE:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
					case IDC_VEGETABLE_ALPHA_BLEND_ON:
					case IDC_VEGETABLE_ALPHA_BLEND_OFF:
						VegetableStateChanged (hwndDlg);
						break;
					case IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}

// ***************************************************************************
int CALLBACK VertexProgramDialogCallBack (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			if(currentParam->VertexProgramId>=0)
			{
				// Init DropList.
				SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_ADDSTRING, 0, (LPARAM)"Disable");
				SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_ADDSTRING, 0, (LPARAM)"Wind Tree");
				SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_SETCURSEL, currentParam->VertexProgramId, 0);
				EnableWindow( GetDlgItem(hwndDlg, IDC_COMBO_VP), TRUE);
			}
			else
			{
				SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_SETCURSEL, -1, 0);
				EnableWindow( GetDlgItem(hwndDlg, IDC_COMBO_VP), FALSE);
			}

			// Get the tab client rect in screen
			RECT tabRect;
			GetClientRect (hwndDlg, &tabRect);
			// Remove VP choose combo box
			tabRect.top += 25;
			tabRect.left += 2;
			tabRect.right -= 2;
			tabRect.bottom -= 2;
			ClientToScreen (hwndDlg, (POINT*)&tabRect.left);
			ClientToScreen (hwndDlg, (POINT*)&tabRect.right);

			// For each VP Dlg to init.
			for (uint vpId=0; vpId<VP_COUNT; vpId++)
			{
				// Create the dialog
				currentParam->SubVPDlg[vpId] = CreateDialogParam (hInstance, MAKEINTRESOURCE(SubVPTab[vpId]), hwndDlg, SubVPProc[vpId], (LONG)lParam);

				// To client coord
				RECT client = tabRect;
				ScreenToClient (currentParam->SubVPDlg[vpId], (POINT*)&client.left);
				ScreenToClient (currentParam->SubVPDlg[vpId], (POINT*)&client.right);

				// Resize and pos it
				SetWindowPos (currentParam->SubVPDlg[vpId], NULL, client.left, client.top, client.right-client.left, client.bottom-client.top, SWP_NOOWNERZORDER|SWP_NOZORDER);
			}

			// Show the prop window
			if(currentParam->VertexProgramId>0)
			{
				int	vpWind= currentParam->VertexProgramId-1;
				ShowWindow(currentParam->SubVPDlg[vpWind], SW_SHOW);
			}
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Validate chosen VertexProgram.
							currentParam->VertexProgramId= SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_GETCURSEL, 0, 0);
							if(currentParam->VertexProgramId==CB_ERR)
								currentParam->VertexProgramId= -1;

							// Advice VP windows
							for (uint vpId=0; vpId<VP_COUNT; vpId++)
							{
								// Send back an ok message
								SendMessage (currentParam->SubVPDlg[vpId], uMsg, wParam, lParam);
							}

						}
					break;
				}
			}
			else if( HIWORD(wParam) == CBN_SELCHANGE )
			{
				if(LOWORD(wParam) == IDC_COMBO_VP)
				{
					// Validate chosen VertexProgram.
					currentParam->VertexProgramId= SendDlgItemMessage(hwndDlg, IDC_COMBO_VP, CB_GETCURSEL, 0, 0);
					if(currentParam->VertexProgramId==CB_ERR)
						currentParam->VertexProgramId= -1;

					// Show the appropriate window (if VP enabled)
					for(int vpId=0; vpId<VP_COUNT;vpId++)
					{
						int		vpWindow= currentParam->VertexProgramId - 1;
						if(vpId==vpWindow)
							ShowWindow (currentParam->SubVPDlg[vpId], SW_SHOW);
						else
							ShowWindow (currentParam->SubVPDlg[vpId], SW_HIDE);
					}

				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}


// ***************************************************************************


// Slider Control Ids.
static	int	VPWTFreqSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_FREQ_L0, IDC_SLIDER_VPWT_FREQ_L1, IDC_SLIDER_VPWT_FREQ_L2};
static	int	VPWTFreqWDSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_FREQWD_L0, IDC_SLIDER_VPWT_FREQWD_L1, IDC_SLIDER_VPWT_FREQWD_L2};
static	int	VPWTDistXYSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_DISTXY_L0, IDC_SLIDER_VPWT_DISTXY_L1, IDC_SLIDER_VPWT_DISTXY_L2};
static	int	VPWTDistZSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_DISTZ_L0, IDC_SLIDER_VPWT_DISTZ_L1, IDC_SLIDER_VPWT_DISTZ_L2};
static	int	VPWTBiasSliderId[CVPWindTreeAppData::HrcDepth]=
	{IDC_SLIDER_VPWT_BIAS_L0, IDC_SLIDER_VPWT_BIAS_L1, IDC_SLIDER_VPWT_BIAS_L2};
// Static Control Ids.
static	int	VPWTFreqStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_FREQ_L0, IDC_STATIC_VPWT_FREQ_L1, IDC_STATIC_VPWT_FREQ_L2};
static	int	VPWTFreqWDStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_FREQWD_L0, IDC_STATIC_VPWT_FREQWD_L1, IDC_STATIC_VPWT_FREQWD_L2};
static	int	VPWTDistXYStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_DISTXY_L0, IDC_STATIC_VPWT_DISTXY_L1, IDC_STATIC_VPWT_DISTXY_L2};
static	int	VPWTDistZStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_DISTZ_L0, IDC_STATIC_VPWT_DISTZ_L1, IDC_STATIC_VPWT_DISTZ_L2};
static	int	VPWTBiasStaticId[CVPWindTreeAppData::HrcDepth]=
	{IDC_STATIC_VPWT_BIAS_L0, IDC_STATIC_VPWT_BIAS_L1, IDC_STATIC_VPWT_BIAS_L2};


void	updateVPWTStatic(HWND hwndDlg, uint type, uint depth, const CVPWindTreeAppData &vpwt)
{
	int		sliderValue;
	char	stmp[256];
	float	nticks= CVPWindTreeAppData::NumTicks;
	float	scale;
	
	// which scale??
	switch(type)
	{
	case 0:
	case 1:
		scale= vpwt.FreqScale;
		break;
	case 2:
	case 3:
		scale= vpwt.DistScale;
		break;
	// case 4: special code from -2 to 2 ...
	}

	// update static according to type.
	switch(type)
	{
	case 0:	
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[depth], TBM_GETPOS, 0, 0);
		sprintf(stmp, "%.2f", scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTFreqStaticId[depth]), stmp );
		break;
	case 1:	
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[depth], TBM_GETPOS, 0, 0);
		sprintf(stmp, "%.2f", scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTFreqWDStaticId[depth]), stmp );
		break;
	case 2:	
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[depth], TBM_GETPOS, 0, 0);
		sprintf(stmp, "%.2f", scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTDistXYStaticId[depth]), stmp );
		break;
	case 3:	
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[depth], TBM_GETPOS, 0, 0);
		sprintf(stmp, "%.2f", scale * float(sliderValue)/nticks);
		SetWindowText( GetDlgItem(hwndDlg, VPWTDistZStaticId[depth]), stmp );
		break;
	case 4:	
		sliderValue= SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[depth], TBM_GETPOS, 0, 0);
		// expand to -2 to 2.
		float	biasVal= 4 * float(sliderValue)/nticks - 2;
		sprintf(stmp, "%.2f", biasVal);
		SetWindowText( GetDlgItem(hwndDlg, VPWTBiasStaticId[depth]), stmp );
		break;
	}
}


static	void concatEdit2Lines(HWND edit)
{
	const	uint lineLen= 1000;
	uint	n;
	// retrieve the 2 lines.
	char	tmp0[2*lineLen];
	char	tmp1[lineLen];
	*(WORD*)tmp0= lineLen;
	*(WORD*)tmp1= lineLen;
	n= SendMessage(edit, EM_GETLINE, 0, (LONG)tmp0); tmp0[n]= 0;
	n= SendMessage(edit, EM_GETLINE, 1, (LONG)tmp1); tmp1[n]= 0;
	// concat and update the CEdit.
	SetWindowText(edit, strcat(tmp0, tmp1));
}


static	void updateVPWTStaticForControl(HWND hwndDlg, HWND ctrlWnd, CVPWindTreeAppData &vpwt, int sliderValue )
{
	// What ctrlWnd is modified??
	int		sliderType=-1;
	int		depth;
	for(depth =0;depth<CVPWindTreeAppData::HrcDepth;depth ++)
	{
		if(ctrlWnd== GetDlgItem(hwndDlg, VPWTFreqSliderId[depth]))
		{sliderType= 0; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTFreqWDSliderId[depth]))
		{sliderType= 1; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTDistXYSliderId[depth]))
		{sliderType= 2; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTDistZSliderId[depth]))
		{sliderType= 3; break;}
		if(ctrlWnd==GetDlgItem(hwndDlg, VPWTBiasSliderId[depth]))
		{sliderType= 4; break;}
	}

	// Set to value, and update static.
	if(sliderType>=0)
	{
		// update value.
		if(sliderType==0)	vpwt.Frequency[depth]= sliderValue;
		if(sliderType==1)	vpwt.FrequencyWindFactor[depth]= sliderValue;
		if(sliderType==2)	vpwt.DistXY[depth]= sliderValue;
		if(sliderType==3)	vpwt.DistZ[depth]= sliderValue;
		if(sliderType==4)	vpwt.Bias[depth]= sliderValue;
		// update text.
		updateVPWTStatic(hwndDlg, sliderType, depth, vpwt);
	}
}


int CALLBACK VPWindTreeCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{ 
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Init controls
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int	nticks= CVPWindTreeAppData::NumTicks;

			// Init Global. editBox
			char		stmp[256];
			sprintf(stmp, "%.2f", vpwt.FreqScale);
			SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp );
			sprintf(stmp, "%.2f", vpwt.DistScale);
			SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp );
			SendDlgItemMessage(hwndDlg, IDC_CHECK_VP_SPECLIGHT, BM_SETCHECK, vpwt.SpecularLighting, 0);

			// Init sliders for each level.
			nlassert(CVPWindTreeAppData::HrcDepth==3);
			for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
			{
				// Init ranges.
				SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));
				SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[i], TBM_SETSEL, TRUE, MAKELONG(0, nticks));

				// Clamp values to range.
				clamp(vpwt.Frequency[i], 0, nticks);
				clamp(vpwt.FrequencyWindFactor[i], 0, nticks);
				clamp(vpwt.DistXY[i], 0, nticks);
				clamp(vpwt.DistZ[i], 0, nticks);
				clamp(vpwt.Bias[i], 0, nticks);

				// Init current selection.
				SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[i], TBM_SETPOS, TRUE, vpwt.Frequency[i]);
				SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[i], TBM_SETPOS, TRUE, vpwt.FrequencyWindFactor[i]);
				SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[i], TBM_SETPOS, TRUE, vpwt.DistXY[i]);
				SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[i], TBM_SETPOS, TRUE, vpwt.DistZ[i]);
				SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[i], TBM_SETPOS, TRUE, vpwt.Bias[i]);

				// Init current Static selection.
				updateVPWTStatic(hwndDlg, 0, i, vpwt);	// FreqStatic
				updateVPWTStatic(hwndDlg, 1, i, vpwt);	// FreqWDStatic
				updateVPWTStatic(hwndDlg, 2, i, vpwt);	// DistXYStatic
				updateVPWTStatic(hwndDlg, 3, i, vpwt);	// DistZStatic
				updateVPWTStatic(hwndDlg, 4, i, vpwt);	// BiasStatic
			}
		}
		break;

		case WM_COMMAND:
		{
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int	nticks= CVPWindTreeAppData::NumTicks;
			char		stmp[256];
			float		val;

			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
					{
						// Read FreqScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp, 256 );
						val= float(atof(stmp));
						if(val>0)
							vpwt.FreqScale= val;
						// Read DistScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp, 256 );
						val= float(atof(stmp));
						if(val>0)
							vpwt.DistScale= val;
						// Read SpecularLighting.
						vpwt.SpecularLighting= SendDlgItemMessage(hwndDlg, IDC_CHECK_VP_SPECLIGHT, BM_GETCHECK, 0, 0);

						// Read slider for each level.
						nlassert(CVPWindTreeAppData::HrcDepth==3);
						for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
						{
							// Read current selection.
							vpwt.Frequency[i]= SendDlgItemMessage(hwndDlg, VPWTFreqSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.FrequencyWindFactor[i]= SendDlgItemMessage(hwndDlg, VPWTFreqWDSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.DistXY[i]= SendDlgItemMessage(hwndDlg, VPWTDistXYSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.DistZ[i]= SendDlgItemMessage(hwndDlg, VPWTDistZSliderId[i], TBM_GETPOS, 0, 0);
							vpwt.Bias[i]= SendDlgItemMessage(hwndDlg, VPWTBiasSliderId[i], TBM_GETPOS, 0, 0);

							// Clamp values to range.
							clamp(vpwt.Frequency[i], 0, nticks);
							clamp(vpwt.FrequencyWindFactor[i], 0, nticks);
							clamp(vpwt.DistXY[i], 0, nticks);
							clamp(vpwt.DistZ[i], 0, nticks);
							clamp(vpwt.Bias[i], 0, nticks);
						}
					}
					break;
				}
			}
			// Aware EditBox: selectAll on setFocus
			if( HIWORD(wParam) == EN_SETFOCUS )
			{
				// Select All.
				PostMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), EM_SETSEL, 0, -1);
				InvalidateRect(GetDlgItem(hwndDlg, LOWORD(wParam)), NULL, TRUE);
			}

			// Aware EditBox: Update and killFocus on enter!!
			bool	EnChangeReturn= false;
			if( HIWORD(wParam) == EN_CHANGE )
			{
				// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
				// user has press enter.
				if( SendMessage(GetDlgItem(hwndDlg, LOWORD(wParam)), EM_GETLINECOUNT, 0, 0) > 1)
				{
					// Concat the 2 lines.
					concatEdit2Lines( GetDlgItem(hwndDlg, LOWORD(wParam)) );
					// Must update value next.
					EnChangeReturn= true;
				}
			}

			// EditBox change: ...
			if( HIWORD(wParam) == EN_KILLFOCUS || EnChangeReturn)
			{
				switch (LOWORD(wParam)) 
				{
					case IDC_EDIT_VPWT_FREQ_SCALE:
					{
						// Read FreqScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp, 256 );
						val= float(atof(stmp));
						if(val>0)
						{
							// update
							vpwt.FreqScale= val;
							// update All Statics
							for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
							{
								updateVPWTStatic(hwndDlg, 0, i, vpwt);	// FreqStatic
								updateVPWTStatic(hwndDlg, 1, i, vpwt);	// FreqWDStatic
							}
						}
						// Update Scale Edit text.
						sprintf(stmp, "%.2f", vpwt.FreqScale);
						SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_FREQ_SCALE), stmp );
					}
					break;
					case IDC_EDIT_VPWT_DIST_SCALE:
					{
						// Read DistScale
						GetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp, 256 );
						val= float(atof(stmp));
						if(val>0)
						{
							// update
							vpwt.DistScale= val;
							// update All Statics
							for(uint i=0;i<CVPWindTreeAppData::HrcDepth;i++)
							{
								updateVPWTStatic(hwndDlg, 2, i, vpwt);	// DistXYStatic
								updateVPWTStatic(hwndDlg, 3, i, vpwt);	// DistZStatic
							}
						}
						// Update Scale Edit text.
						sprintf(stmp, "%.2f", vpwt.DistScale);
						SetWindowText( GetDlgItem(hwndDlg, IDC_EDIT_VPWT_DIST_SCALE), stmp );
					}
					break;
				}
			}
		}
		break;

		// Handle dynamic scroll updating static
		case WM_HSCROLL:
		{
			HWND	ctrlWnd= (HWND)lParam;
			UINT	nSBCode= LOWORD(wParam);
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int		nticks= CVPWindTreeAppData::NumTicks;

			if( nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
			{
				int		sliderValue= HIWORD(wParam);
				clamp(sliderValue, 0, nticks);

				updateVPWTStaticForControl( hwndDlg, ctrlWnd, vpwt, sliderValue );
			}
		}
		break;

		// update static on release
		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			HWND	ctrlWnd= pnmh->hwndFrom;
			CVPWindTreeAppData		&vpwt= currentParam->VertexProgramWindTree;
			int		nticks= CVPWindTreeAppData::NumTicks;

			if( pnmh->code == NM_RELEASEDCAPTURE )
			{
				int sliderValue= SendMessage (ctrlWnd, TBM_GETPOS, 0, 0);
				clamp(sliderValue, 0, nticks);

				updateVPWTStaticForControl( hwndDlg, ctrlWnd, vpwt, sliderValue );
			}
		}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}



// ***************************************************************************
int CALLBACK MiscDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{ 
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_SETCHECK, currentParam->ExportNoteTrack, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_SETCHECK, currentParam->FloatingObject, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATED_MATERIALS), BM_SETCHECK, currentParam->ExportAnimatedMaterials, 0);

			// Ligoscape
			SendMessage (GetDlgItem (hwndDlg, IDC_LIGO_SYMMETRY), BM_SETCHECK, currentParam->LigoSymmetry, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_LIGO_ROTATE), currentParam->LigoRotate.c_str());

			// SWT
			SendMessage (GetDlgItem (hwndDlg, IDC_SWT), BM_SETCHECK, currentParam->SWT, 0);
			SetWindowText (GetDlgItem (hwndDlg, IDC_SWT_WEIGHT), currentParam->SWTWeight.c_str());
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->ExportNoteTrack=SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_GETCHECK, 0, 0);
							currentParam->FloatingObject=SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_GETCHECK, 0, 0);
							currentParam->ExportAnimatedMaterials = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATED_MATERIALS), BM_GETCHECK, 0, 0);

							// Ligoscape
							currentParam->LigoSymmetry = SendMessage (GetDlgItem (hwndDlg, IDC_LIGO_SYMMETRY), BM_GETCHECK, 0, 0);
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_LIGO_ROTATE), tmp, 512);
							currentParam->LigoRotate = tmp;

							// SWT
							currentParam->SWT = SendMessage (GetDlgItem (hwndDlg, IDC_SWT), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_SWT_WEIGHT), tmp, 512);
							currentParam->SWTWeight = tmp;
						}
					break;
					case IDC_EXPORT_NOTE_TRACK:
					case IDC_FLOATING_OBJECT:
					case IDC_EXPORT_ANIMATED_MATERIALS:
					case IDC_SWT:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}






// ***************************************************************************
int CALLBACK LodDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Window text
			std::string winName=(*(currentParam->ListNode->begin()))->GetName();
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, winName.c_str());

			// Move dialog
			RECT windowRect, desktopRect;
			GetWindowRect (hwndDlg, &windowRect);
			HWND desktop=GetDesktopWindow ();
			GetClientRect (desktop, &desktopRect);
			SetWindowPos (hwndDlg, NULL, (desktopRect.right-desktopRect.left-(windowRect.right-windowRect.left))/2,
				(desktopRect.bottom-desktopRect.top-(windowRect.bottom-windowRect.top))/2, 0, 0, SWP_NOOWNERZORDER|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);

			// List of windows to create

			// Get the tab client rect in screen
			RECT tabRect;
			GetClientRect (GetDlgItem (hwndDlg, IDC_TAB), &tabRect);
			tabRect.top += 30;
			tabRect.left += 5;
			tabRect.right -= 5;
			tabRect.bottom -= 5;
			ClientToScreen (GetDlgItem (hwndDlg, IDC_TAB), (POINT*)&tabRect.left);
			ClientToScreen (GetDlgItem (hwndDlg, IDC_TAB), (POINT*)&tabRect.right);

			// For each tab
			for (uint tab=0; tab<TAB_COUNT; tab++)
			{
				// Insert a tab
				TCITEM tabItem;
				tabItem.mask = TCIF_TEXT;
				tabItem.pszText = (char*)SubText[tab];
				SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_INSERTITEM, SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_GETITEMCOUNT, 0, 0), (LPARAM)&tabItem);

				// Create the dialog
				currentParam->SubDlg[tab] = CreateDialogParam (hInstance, MAKEINTRESOURCE(SubTab[tab]), GetDlgItem (hwndDlg, IDC_TAB), SubProc[tab], (LONG)lParam);

				// To client coord
				RECT client = tabRect;
				ScreenToClient (currentParam->SubDlg[tab], (POINT*)&client.left);
				ScreenToClient (currentParam->SubDlg[tab], (POINT*)&client.right);

				// Resize and pos it
				SetWindowPos (currentParam->SubDlg[tab], NULL, client.left, client.top, client.right-client.left, client.bottom-client.top, SWP_NOOWNERZORDER|SWP_NOZORDER);
			}

			// Activate the last activated TAB
			SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_SETCURSEL, LastTabActivated, 0);
			// Show the last activated TAB
			ShowWindow (currentParam->SubDlg[LastTabActivated], SW_SHOW);
		}
		break;

		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch (pnmh->code)
				{
				case TCN_SELCHANGE:
					{
						
						uint curSel=SendMessage (pnmh->hwndFrom, TCM_GETCURSEL, 0, 0);
						for (uint tab=0; tab<TAB_COUNT; tab++)
						{
							ShowWindow (currentParam->SubDlg[tab], (tab == curSel)?SW_SHOW:SW_HIDE);
						}
						LastTabActivated= curSel;
						clamp(LastTabActivated, 0, TAB_COUNT-1);
						break;
					}
				}
				break;
			}
		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Ok in each other window
							for (uint tab=0; tab<TAB_COUNT; tab++)
							{
								// Send back an ok message
								SendMessage (currentParam->SubDlg[tab], uMsg, wParam, lParam);
							}

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
				}
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
/*	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Window text
			std::string winName=(*(currentParam->ListNode->begin()))->GetName();
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, winName.c_str());

			// Set default state
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_SETCHECK, currentParam->BlendIn, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_SETCHECK, currentParam->BlendOut, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_SETCHECK, currentParam->CoarseMesh, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_SETCHECK, currentParam->DynamicMesh, 0);


			EnableWindow (GetDlgItem (hwndDlg, IDC_LIST1), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_ADD), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_REMOVE), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_UP), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_DOWN), currentParam->ListActived);
			
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), currentParam->DistMax.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), currentParam->BlendLength.c_str());

			SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_SETCHECK, currentParam->MRM, 0);
			CoarseStateChanged (hwndDlg);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			SetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), currentParam->NbLod.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), currentParam->Divisor.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), currentParam->DistanceFinest.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), currentParam->DistanceMiddle.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), currentParam->DistanceCoarsest.c_str());

			// Iterate list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
			std::list<std::string>::iterator ite=currentParam->ListLodName.begin();
			while (ite!=currentParam->ListLodName.end())
			{
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) ite->c_str());
				ite++;
			}

			if (currentParam->AccelType != -1)
			{
				CheckRadioButton (hwndDlg, IDC_RADIOACCELNO, IDC_RADIOACCELCLUSTER, IDC_RADIOACCELNO+(currentParam->AccelType&3));
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), true);
				if ((currentParam->AccelType&3) == 2) // Cluster ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), BM_SETCHECK, currentParam->AccelType&4, 0);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), BM_SETCHECK, currentParam->AccelType&8, 0);
				}
				else
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				}
				
				if ((currentParam->AccelType&3) == 1) // Portal ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), BM_SETCHECK, currentParam->AccelType&16, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);

				if ((currentParam->AccelType&3) == 0) // Not an accelerator
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), BM_SETCHECK, currentParam->AccelType&32, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);

			}
			else
			{
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);
			}

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), currentParam->InstanceName.c_str());
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_SETCHECK, currentParam->DontAddToScene, 0);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), currentParam->InstanceGroupName.c_str());

			EnableWindow (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), true);
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_SETCHECK, currentParam->DontExport, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_SETCHECK, currentParam->ExportNoteTrack, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_SETCHECK, currentParam->FloatingObject, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_SETCHECK, currentParam->Vegetable, 0);

			if (currentParam->DontExport == BST_UNCHECKED)
				EnableWindow (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), true);
			else
				EnableWindow (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), false);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), currentParam->LumelSizeMul.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), currentParam->SoftShadowRadius.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), currentParam->SoftShadowConeLength.c_str());
			
			// Move dialog
			RECT windowRect, desktopRect;
			GetWindowRect (hwndDlg, &windowRect);
			HWND desktop=GetDesktopWindow ();
			GetClientRect (desktop, &desktopRect);
			SetWindowPos (hwndDlg, NULL, (desktopRect.right-desktopRect.left-(windowRect.right-windowRect.left))/2,
				(desktopRect.bottom-desktopRect.top-(windowRect.bottom-windowRect.top))/2, 0, 0, SWP_NOOWNERZORDER|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							currentParam->BlendIn=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_GETCHECK, 0, 0);
							currentParam->BlendOut=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_GETCHECK, 0, 0);
							currentParam->CoarseMesh=SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0);
							currentParam->DynamicMesh=SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_GETCHECK, 0, 0);
							
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), tmp, 512);
							currentParam->DistMax=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), tmp, 512);
							currentParam->BlendLength=tmp;

							currentParam->MRM=SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0);

							currentParam->SkinReduction=-1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MIN)==BST_CHECKED)
								currentParam->SkinReduction=0;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MAX)==BST_CHECKED)
								currentParam->SkinReduction=1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_BEST)==BST_CHECKED)
								currentParam->SkinReduction=2;

							GetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), tmp, 512);
							currentParam->NbLod=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), tmp, 512);
							currentParam->Divisor=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), tmp, 512);
							currentParam->DistanceFinest=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), tmp, 512);
							currentParam->DistanceMiddle=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), tmp, 512);
							currentParam->DistanceCoarsest=tmp;

							// Iterate list
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
							currentParam->ListLodName.clear ();

							// Insert item in the list
							uint itemCount=SendMessage (hwndList, LB_GETCOUNT, 0, 0);
							for (uint item=0; item<itemCount; item++)
							{
								// Get the string
								SendMessage (hwndList, LB_GETTEXT, item, (LPARAM) tmp);

								// Push it back
								currentParam->ListLodName.push_back (tmp);
							}

							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELNO) == BST_CHECKED)
								currentParam->AccelType = 0;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELPORTAL) == BST_CHECKED)
								currentParam->AccelType = 1;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELCLUSTER) == BST_CHECKED)
								currentParam->AccelType = 2;

							if (IsDlgButtonChecked (hwndDlg, IDC_FATHER_VISIBLE) == BST_CHECKED)
								currentParam->AccelType |= 4;
							if (IsDlgButtonChecked (hwndDlg, IDC_VISIBLE_FROM_FATHER) == BST_CHECKED)
								currentParam->AccelType |= 8;
							if (IsDlgButtonChecked (hwndDlg, IDC_DYNAMIC_PORTAL) == BST_CHECKED)
								currentParam->AccelType |= 16;
							if (IsDlgButtonChecked (hwndDlg, IDC_CLUSTERIZE) == BST_CHECKED)
								currentParam->AccelType |= 32;

							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), tmp, 512);
							currentParam->InstanceName=tmp;
							currentParam->DontAddToScene=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), tmp, 512);
							currentParam->InstanceGroupName=tmp;
							currentParam->DontExport=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_GETCHECK, 0, 0);
							currentParam->ExportNoteTrack=SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_GETCHECK, 0, 0);
							currentParam->FloatingObject=SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_GETCHECK, 0, 0);
							currentParam->Vegetable=SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0);

							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), tmp, 512);
							currentParam->LumelSizeMul = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), tmp, 512);
							currentParam->SoftShadowRadius = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), tmp, 512);
							currentParam->SoftShadowConeLength = tmp;

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
					case IDC_ADD:
						{
							// Callback for the select node dialog
							addSubLodNodeHitCallBack callBack;
							listNodeCallBack=currentParam->ListNode;
							if (theCNelExport.ip->DoHitByNameDialog(&callBack))
							{
								// Add the selected object in the list
								HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
								for (uint i=0; i<(uint)callBack.NodeTab.Count(); i++)
									SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) callBack.NodeTab[i]->GetName());
							}
						}
						break;
					case IDC_REMOVE:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if (sel!=LB_ERR)
							{
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								if (sel==SendMessage (hwndList, LB_GETCOUNT, 0, 0))
									sel--;
								if (sel>=0)
									SendMessage (hwndList, LB_SETCURSEL, sel, 0);
							}
						}
					break;
					case IDC_UP:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ((sel!=LB_ERR)&&(sel>0))
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move up the item
								SendMessage (hwndList, LB_INSERTSTRING, sel-1, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel+1, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel-1, 0);
							}
						}
					break;
					case IDC_DOWN:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ( (sel!=LB_ERR) && (sel<SendMessage (hwndList, LB_GETCOUNT, 0, 0)-1 ) )
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move down the item
								SendMessage (hwndList, LB_INSERTSTRING, sel+2, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel+1, 0);
							}
						}
					break;
					// 3 states management
					case IDC_COARSE_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							CoarseStateChanged (hwndDlg);
						}
						break;
					case IDC_ACTIVE_MRM:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							MRMStateChanged (hwndDlg);
						}
						break;
					case IDC_BLEND_IN:
					case IDC_BLEND_OUT:
					case IDC_DYNAMIC_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
					case IDC_RADIOACCELNO:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), true);
						break;
					case IDC_RADIOACCELPORTAL:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_RADIOACCELCLUSTER:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_DONT_ADD_TO_SCENE:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
					case IDC_DONT_EXPORT:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);

						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
							EnableWindow (GetDlgItem(hwndDlg, IDC_EXPORT_NOTE_TRACK), true);
						else
							EnableWindow (GetDlgItem(hwndDlg, IDC_EXPORT_NOTE_TRACK), false);
						break;
					case IDC_EXPORT_NOTE_TRACK:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					char name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport.ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
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
	return TRUE;*/
}

// ***************************************************************************

void CNelExport::OnNodeProperties (const std::set<INode*> &listNode)
{
	// Get 
	uint nNumSelNode=listNode.size();

	if (nNumSelNode)
	{
		// Get the selected node
		INode* node=*listNode.begin();

		// Dialog box param
		CLodDialogBoxParam param;

		// Value of the properties
		param.ListNode=&listNode;
		param.ListActived=true;
		param.BlendIn=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED);
		param.BlendOut=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED);
		param.CoarseMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED);
		param.DynamicMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED);
		float floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
		param.DistMax=toString (floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
		param.BlendLength=toString (floatTmp);
		param.MRM=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_MRM, BST_UNCHECKED);
		param.SkinReduction=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT);

		int intTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT);
		param.NbLod=toString (intTmp);
		intTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT);
		param.Divisor=toString(intTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT);
		param.DistanceFinest=toString(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT);
		param.DistanceMiddle=toString(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT);
		param.DistanceCoarsest=toString(floatTmp);

		// Lod names
		int nameCount=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
		int name;
		for (name=0; name<std::min (nameCount, NEL3D_APPDATA_LOD_NAME_COUNT_MAX); name++)
		{
			// Get a string
			std::string nameLod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME+name, "");
			if (nameLod!="")
			{
				param.ListLodName.push_back (nameLod);
			}
		}
		param.AccelType = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, 32);

		param.InstanceShape=CExportNel::getScriptAppData (node, NEL_OBJET_NAME_DATA, "");
		param.InstanceName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, "");
		param.DontAddToScene=CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, BST_UNCHECKED);
		param.InstanceGroupName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_IGNAME, "");
		param.DontExport=CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, BST_UNCHECKED);
		param.ExportNoteTrack=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, BST_UNCHECKED);
		param.ExportAnimatedMaterials=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, BST_UNCHECKED);
		param.FloatingObject=CExportNel::getScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, BST_UNCHECKED);
		param.LumelSizeMul=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, "1.0");
		param.SoftShadowRadius=CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toString(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT));
		param.SoftShadowConeLength=CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toString(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT));

		// Vegetable
		param.Vegetable = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED);
		param.VegetableAlphaBlend = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0);
		param.VegetableAlphaBlendOnLighted = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, BST_UNCHECKED);
		param.VegetableAlphaBlendOffLighted = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, BST_UNCHECKED);
		param.VegetableAlphaBlendOffDoubleSided = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, BST_UNCHECKED);
		param.VegetableBendCenter = CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, 0);
		param.VegetableBendFactor = toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT));
		param.ExportLightMapName = CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_GROUPNAME, NEL3D_LM_GROUPNAME_DEFAULT);

		// Ligoscape
		param.LigoSymmetry = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_SYMMETRY, BST_UNCHECKED);
		param.LigoRotate = toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_ROTATE, 0));

		// Ligoscape
		param.SWT = CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT, BST_UNCHECKED);
		param.SWTWeight = toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, 0.f));

		// RealTimeLigt.
		param.ExportRealTimeLight= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED);

		// LightmapLigt. (true by default)
		param.ExportLightMapLight= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, BST_CHECKED);

		// LightmapLigt. (true by default)
		param.ExportLightMapAnimated= CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED, BST_UNCHECKED);

		// ExportAsSunLight.
		param.ExportAsSunLight= CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED);

		// UseLightingLocalAttenuation
		param.UseLightingLocalAttenuation= CExportNel::getScriptAppData (node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, BST_UNCHECKED);

		// VertexProgram
		param.VertexProgramId= CExportNel::getScriptAppData (node, NEL3D_APPDATA_VERTEXPROGRAM_ID, 0);
		// VertexProgramWindTree
		CExportNel::getScriptAppDataVPWT(node, param.VertexProgramWindTree);


		// Collision
		param.Collision= CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
		param.CollisionExterior= CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION_EXTERIOR, BST_UNCHECKED);


		// Something selected ?
		std::set<INode*>::const_iterator ite=listNode.begin();
		ite++;
		while (ite!=listNode.end())
		{
			// Get the selected node
			node=*ite;

			// Get the properties
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED)!=param.BlendIn)
				param.BlendIn=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED)!=param.BlendOut)
				param.BlendOut=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED)!=param.CoarseMesh)
				param.CoarseMesh=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED)!=param.DynamicMesh)
				param.DynamicMesh=BST_INDETERMINATE;
			if (param.DistMax!=toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT)))
				param.DistMax="";
			if (param.BlendLength!=toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT)))
				param.BlendLength="";

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_MRM, BST_UNCHECKED)!=param.MRM)
				param.MRM=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT)!=param.SkinReduction)
				param.SkinReduction=-1;
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT))!=param.NbLod)
				param.NbLod="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT))!=param.Divisor)
				param.Divisor="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT))!=param.DistanceFinest)
				param.DistanceFinest="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT))!=param.DistanceMiddle)
				param.DistanceMiddle="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT))!=param.DistanceCoarsest)
				param.DistanceCoarsest="";

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, 32)!=param.AccelType)
				param.AccelType = -1;

			if (CExportNel::getScriptAppData (node, NEL_OBJET_NAME_DATA, "")!=param.InstanceShape)
				param.InstanceShape = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, "")!=param.InstanceName)
				param.InstanceName = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, BST_UNCHECKED)!=param.DontAddToScene)
				param.DontAddToScene = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_IGNAME, "")!=param.InstanceGroupName)
				param.InstanceGroupName = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, BST_UNCHECKED)!=param.DontExport)
				param.DontExport= BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, BST_UNCHECKED)!=param.ExportNoteTrack)
				param.ExportNoteTrack = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, BST_UNCHECKED)!=param.ExportAnimatedMaterials)
				param.ExportAnimatedMaterials = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, BST_UNCHECKED)!=param.FloatingObject)
				param.FloatingObject = BST_INDETERMINATE;

			// Vegetable
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED)!=param.Vegetable)
				param.Vegetable = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0)!=param.VegetableAlphaBlend)
				param.VegetableAlphaBlend = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, 0)!=param.VegetableAlphaBlendOnLighted)
				param.VegetableAlphaBlendOnLighted = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0)!=param.VegetableAlphaBlendOffLighted)
				param.VegetableAlphaBlendOffLighted = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, BST_UNCHECKED)!=param.VegetableAlphaBlendOffDoubleSided)
				param.VegetableAlphaBlendOffDoubleSided = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, BST_UNCHECKED)!=param.VegetableBendCenter)
				param.VegetableBendCenter = -1;
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT))!=param.VegetableBendFactor)
				param.VegetableBendFactor = "";

			// Lightmap
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, "1.0")!=param.LumelSizeMul)
				param.LumelSizeMul = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toString(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT))!=param.SoftShadowRadius)
				param.SoftShadowRadius = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toString(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT))!=param.SoftShadowConeLength)
				param.SoftShadowConeLength = "";

			// Get name count for this node
			std::list<std::string> tmplist;
			nameCount=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
			for (name=0; name<std::min (nameCount, NEL3D_APPDATA_LOD_NAME_COUNT_MAX); name++)
			{
				// Get a string
				std::string nameLod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME+name, "");
				if (nameLod!="")
				{
					tmplist.push_back (nameLod);
				}
			}

			// Compare with original list
			if (tmplist!=param.ListLodName)
			{
				// Not the same, so clear
				param.ListLodName.clear();
				param.ListActived=false;
			}

			// Ligoscape
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_SYMMETRY, BST_UNCHECKED) != param.LigoSymmetry)
				param.LigoSymmetry = BST_INDETERMINATE;
			if (toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ZONE_ROTATE, 0)) != param.LigoRotate)
				param.LigoRotate = "";

			// SWT
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT, BST_UNCHECKED) != param.SWT)
				param.SWT = BST_INDETERMINATE;
			if (toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, 0.f)) != param.SWTWeight)
				param.SWTWeight = "";

			// RealTimeLight
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, BST_CHECKED) != param.ExportRealTimeLight)
				param.ExportRealTimeLight= BST_INDETERMINATE;

			// ExportLightMapLight
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, BST_CHECKED) != param.ExportLightMapLight)
				param.ExportLightMapLight= BST_INDETERMINATE;

			// ExportLightMapAnimated
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED, BST_UNCHECKED) != param.ExportLightMapAnimated)
				param.ExportLightMapAnimated= BST_INDETERMINATE;

			// ExportAsSunLight
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, BST_UNCHECKED) != param.ExportAsSunLight)
				param.ExportAsSunLight= BST_INDETERMINATE;

			// Lightmap name
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_GROUPNAME, NEL3D_LM_GROUPNAME_DEFAULT)!=param.ExportLightMapName)
			{
				param.ExportLightMapName = "";
			}

			// UseLightingLocalAttenuation
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, BST_UNCHECKED) != param.UseLightingLocalAttenuation)
				param.UseLightingLocalAttenuation= BST_INDETERMINATE;

			
			// VertexProgram
			// simply disable VertexProgram edition of multiple selection... 
			param.VertexProgramId= -1;

			// Collision
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED) != param.Collision)
				param.Collision= BST_INDETERMINATE;
			if(CExportNel::getScriptAppData (node, NEL3D_APPDATA_COLLISION_EXTERIOR, BST_UNCHECKED) != param.CollisionExterior)
				param.CollisionExterior= BST_INDETERMINATE;

			// Next sel
			ite++;
		}

		if (DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NODE_PROPERTIES), ip->GetMAXHWnd(), LodDialogCallback, (long)&param)==IDOK)
		{
			// Next node
			ite=listNode.begin();
			while (ite!=listNode.end())
			{
				// Get the selected node
				node=*ite;

				// Ok pushed, copy returned params
				if (param.BlendIn!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, param.BlendIn);
				if (param.BlendOut!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, param.BlendOut);
				if (param.CoarseMesh!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, param.CoarseMesh);
				if (param.DynamicMesh!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, param.DynamicMesh);

				if (param.DistMax!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, param.DistMax);
				if (param.BlendLength!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, param.BlendLength);

				if (param.MRM!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_MRM, param.MRM);
				if (param.SkinReduction!=-1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, param.SkinReduction);
				if (param.NbLod!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, param.NbLod);
				if (param.Divisor!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, param.Divisor);
				if (param.DistanceFinest!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, param.DistanceFinest);
				if (param.DistanceMiddle!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, param.DistanceMiddle);
				if (param.DistanceCoarsest!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, param.DistanceCoarsest);

				if (param.AccelType != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ACCEL, param.AccelType);

				if (param.InstanceShape != "")
					CExportNel::setScriptAppData (node, NEL_OBJET_NAME_DATA, param.InstanceShape);
				if (param.InstanceName != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, param.InstanceName);
				if (param.DontAddToScene != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, param.DontAddToScene);
				if (param.InstanceGroupName != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_IGNAME, param.InstanceGroupName);
				if (param.DontExport != BST_INDETERMINATE)
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, param.DontExport);
					if (param.ExportNoteTrack != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, param.ExportNoteTrack);
					if (param.ExportAnimatedMaterials != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, param.ExportAnimatedMaterials);
				}
				if (param.LumelSizeMul != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, param.LumelSizeMul);
				if (param.SoftShadowRadius != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, param.SoftShadowRadius);
				if (param.SoftShadowConeLength != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, param.SoftShadowConeLength);
				if (param.FloatingObject != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, param.FloatingObject);

				// Vegetable
				if (param.Vegetable != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE, param.Vegetable);
				if (param.VegetableAlphaBlend != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, param.VegetableAlphaBlend);
				if (param.VegetableAlphaBlendOnLighted != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, param.VegetableAlphaBlendOnLighted);
				if (param.VegetableAlphaBlendOffLighted != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, param.VegetableAlphaBlendOffLighted);
				if (param.VegetableAlphaBlendOffDoubleSided != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, param.VegetableAlphaBlendOffDoubleSided);
				if (param.VegetableBendCenter != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, param.VegetableBendCenter);
				if (param.VegetableBendFactor != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, param.VegetableBendFactor);

				if (param.ListActived)
				{
					// Write size of the list
					uint sizeList=std::min (param.ListLodName.size(), (uint)NEL3D_APPDATA_LOD_NAME_COUNT_MAX);
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, (int)sizeList);
				
					// Write the strings
					uint stringIndex=0;
					std::list<std::string>::iterator ite=param.ListLodName.begin();
					while (ite!=param.ListLodName.end())
					{
						// Write the string
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NAME+stringIndex, *ite);
						stringIndex++;
						ite++;
					}
				}

				// Ligoscape
				if (param.LigoSymmetry != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ZONE_SYMMETRY, param.LigoSymmetry);
				if (param.LigoRotate != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ZONE_ROTATE, param.LigoRotate);

				// SWT
				if (param.SWT != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT, param.SWT);
				if (param.SWTWeight != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_SWT_WEIGHT, param.SWTWeight);

				// RealTime Light.
				if (param.ExportRealTimeLight != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_REALTIME_LIGHT, param.ExportRealTimeLight);

				// ExportLightMapLight.
				if (param.ExportLightMapLight != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_LIGHTMAP_LIGHT, param.ExportLightMapLight);

				// ExportLightMapAnimated.
				if (param.ExportLightMapAnimated != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED, param.ExportLightMapAnimated);

				// ExportAsSunLight.
				if (param.ExportAsSunLight != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_AS_SUN_LIGHT, param.ExportAsSunLight);

				// ExportLightMapName
				if (param.ExportLightMapName != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LM_GROUPNAME, param.ExportLightMapName);

				// UseLightingLocalAttenuation
				if (param.UseLightingLocalAttenuation != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_USE_LIGHT_LOCAL_ATTENUATION, param.UseLightingLocalAttenuation);

				// VertexProgram
				if (param.VertexProgramId!=-1)
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VERTEXPROGRAM_ID, param.VertexProgramId);
					// according to VertexProgram, change setup 
					if(param.VertexProgramId==1)
					{
						CExportNel::setScriptAppDataVPWT(node, param.VertexProgramWindTree);
					}
				}

				// Collision
				if (param.Collision != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_COLLISION, param.Collision);
				if (param.CollisionExterior != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_COLLISION_EXTERIOR, param.CollisionExterior);

				// Next node
				ite++;
			}
		}
	}
}

// ***************************************************************************
