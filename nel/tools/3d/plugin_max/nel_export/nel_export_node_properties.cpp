/** \file nel_export_node_properties.cpp
 * Node properties dialog
 *
 * $Id: nel_export_node_properties.cpp,v 1.1 2001/07/04 16:38:39 corvazier Exp $
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

using namespace NLMISC;

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
		return !node->Selected();
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
	int				BlendIn;
	int				BlendOut;
	int				CoarseMesh;
	int				DynamicMesh;
	std::string		DistMax;
	std::string		BlendLength;
	bool			ListActived;
	std::list<std::string>	ListLodName;
};

// ***************************************************************************

CLodDialogBoxParam *currentParam;

// ***************************************************************************

int CALLBACK LodDialogCallback (
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
			// Param pointers
			currentParam=(CLodDialogBoxParam *)lParam;

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

			// Iterate list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
			std::list<std::string>::iterator ite=currentParam->ListLodName.begin();
			while (ite!=currentParam->ListLodName.end())
			{
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) ite->c_str());
				ite++;
			}

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

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
					case IDC_ADD:
						{
							// Callback for the select node dialog
							addSubLodNodeHitCallBack callBack;
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
					case IDC_BLEND_IN:
					case IDC_BLEND_OUT:
					case IDC_COARSE_MESH:
					case IDC_DYNAMIC_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
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
}

// ***************************************************************************

void CNelExport::OnNodeProperties ()
{
	// Get 
	uint nNumSelNode=ip->GetSelNodeCount();

	// Something selected ?
	if (nNumSelNode)
	{
		// Get the selected node
		INode* node=ip->GetSelNode (0);

		// Dialog box param
		CLodDialogBoxParam param;

		// Value of the properties
		param.BlendIn=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED);
		param.BlendOut=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED);
		param.CoarseMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED);
		param.DynamicMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED);
		float distMax=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
		float blendLength=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
		param.DistMax=toString (distMax);
		param.BlendLength=toString (blendLength);
		param.ListActived=true;

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

		// Next node
		for (uint sel=1; sel<nNumSelNode; sel++)
		{
			// Get the selected node
			node=ip->GetSelNode (sel);

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
		}

		if (DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NODE_PROPERTIES), ip->GetMAXHWnd(), LodDialogCallback, (long)&param)==IDOK)
		{
			// Next node
			for (uint sel=0; sel<nNumSelNode; sel++)
			{
				// Get the selected node
				node=ip->GetSelNode (sel);

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
			}
		}
	}
}

// ***************************************************************************
