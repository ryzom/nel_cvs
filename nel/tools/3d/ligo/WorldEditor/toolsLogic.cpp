#include "stdafx.h"
#include "toolsLogic.h"

#include "typeSelDlg.h"

#include "mainfrm.h"

#include <string>

using namespace std;

// ---------------------------------------------------------------------------

#define ID_MENU_CREATE				0x10010
#define ID_MENU_DELETE				0x10011
#define ID_MENU_PROPERTIES			0x10012
#define ID_MENU_HIDEALL				0x10013
#define ID_MENU_UNHIDEALL			0x10014
#define ID_MENU_HIDE				0x10015
#define ID_MENU_REGIONUNHIDEALL		0x10016
#define ID_MENU_REGIONHIDEALL		0x10017
#define ID_MENU_REGIONHIDETYPE		0x10018
#define ID_MENU_REGIONUNHIDETYPE	0x10019

// ---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CToolsLogic, CTreeView)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_MENU_CREATE, OnMenuCreate)
	ON_COMMAND(ID_MENU_DELETE, OnMenuDelete)
	ON_COMMAND(ID_MENU_PROPERTIES, OnMenuProperties)
	ON_COMMAND(ID_MENU_HIDEALL, OnMenuHideAll)
	ON_COMMAND(ID_MENU_UNHIDEALL, OnMenuUnhideAll)
	ON_COMMAND(ID_MENU_HIDE, OnMenuHide)
	ON_COMMAND(ID_MENU_REGIONHIDEALL, OnMenuRegionHideAll)
	ON_COMMAND(ID_MENU_REGIONUNHIDEALL,OnMenuRegionUnhideAll)
	ON_COMMAND(ID_MENU_REGIONHIDETYPE, OnMenuRegionHideType)
	ON_COMMAND(ID_MENU_REGIONUNHIDETYPE,OnMenuRegionUnhideType)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CToolsLogic, CTreeView)

// ---------------------------------------------------------------------------
CToolsLogic::CToolsLogic ()
{
	_MainFrame = NULL;
}

// ---------------------------------------------------------------------------
void CToolsLogic::init (CMainFrame *pMF)
{
	_MainFrame = pMF;
	pMF->_PRegionBuilder.setToolsLogic (this);
	_RegionsInfo.resize (0);
	pMF->_PRegionBuilder.updateToolsLogic ();
}

// ---------------------------------------------------------------------------
void CToolsLogic::reset ()
{
	GetTreeCtrl().DeleteAllItems();
	_RegionsInfo.clear ();
}

// ---------------------------------------------------------------------------
void CToolsLogic::uninit ()
{
	_MainFrame->_PRegionBuilder.setSelPB (NULL);
	_MainFrame->_PRegionBuilder.setToolsLogic (NULL);
	_MainFrame = NULL;
}

// ---------------------------------------------------------------------------
uint32 CToolsLogic::createNewRegion (const std::string &name)
{
	SRegionInfo ri;

	if (_MainFrame == NULL)
		return 0;
	
	ri.Name = name;
	ri.RegionItem = GetTreeCtrl().InsertItem (ri.Name.c_str());
	ri.PointItem = GetTreeCtrl().InsertItem ("Points", ri.RegionItem);
	ri.PathItem = GetTreeCtrl().InsertItem ("Splines", ri.RegionItem);
	ri.ZoneItem = GetTreeCtrl().InsertItem ("Patatoids", ri.RegionItem);
	_RegionsInfo.push_back (ri);
	
	return _RegionsInfo.size()-1;
}

// ---------------------------------------------------------------------------
HTREEITEM CToolsLogic::addPoint (uint32 nPos, const std::string &name)
{
	return GetTreeCtrl().InsertItem (name.c_str(), _RegionsInfo[nPos].PointItem);
}

// ---------------------------------------------------------------------------
HTREEITEM CToolsLogic::addPath (uint32 nPos, const std::string &name)
{
	return GetTreeCtrl().InsertItem (name.c_str(), _RegionsInfo[nPos].PathItem);
}

// ---------------------------------------------------------------------------
HTREEITEM CToolsLogic::addZone (uint32 nPos, const std::string &name)
{
	return GetTreeCtrl().InsertItem (name.c_str(), _RegionsInfo[nPos].ZoneItem);
}

// ---------------------------------------------------------------------------
void CToolsLogic::expandAll (uint32 nPos)
{
	GetTreeCtrl().Expand (_RegionsInfo[nPos].RegionItem, TVE_EXPAND);
	GetTreeCtrl().Expand (_RegionsInfo[nPos].PointItem, TVE_EXPAND);
	GetTreeCtrl().Expand (_RegionsInfo[nPos].PathItem, TVE_EXPAND);
	GetTreeCtrl().Expand (_RegionsInfo[nPos].ZoneItem, TVE_EXPAND);
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnRButtonDown (UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = GetTreeCtrl().HitTest(point, &uFlags);
	HTREEITEM hParent = GetTreeCtrl().GetParentItem (hItem);
	
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		// Select the item
		GetTreeCtrl().Select(hItem, TVGN_CARET);

		// Menu creation
		CRect r;
		this->GetWindowRect (r);
		CMenu *pMenu = new CMenu;
		pMenu->CreatePopupMenu ();

		if (hParent == NULL)
		{
			// Region contextual menu
			pMenu->AppendMenu (MF_STRING, ID_MENU_REGIONHIDEALL, "Hide All");
			pMenu->AppendMenu (MF_STRING, ID_MENU_REGIONUNHIDEALL, "Unhide All");
			pMenu->AppendMenu (MF_STRING, ID_MENU_REGIONHIDETYPE, "Hide Type");
			pMenu->AppendMenu (MF_STRING, ID_MENU_REGIONUNHIDETYPE, "Unhide Type");
		}
		else if (GetTreeCtrl().GetParentItem(hParent) == NULL)
		{
			// Display the contextual menu for point, spline, etc...
			pMenu->AppendMenu (MF_STRING, ID_MENU_CREATE, "Create");
			pMenu->AppendMenu (MF_STRING, ID_MENU_HIDEALL, "Hide All");
			pMenu->AppendMenu (MF_STRING, ID_MENU_UNHIDEALL, "Unhide All");
		}
		else
		{
			// Display the contextual menu for an element : properties
			pMenu->AppendMenu (MF_STRING, ID_MENU_DELETE, "Delete");
			pMenu->AppendMenu (MF_STRING, ID_MENU_PROPERTIES, "Properties");
			pMenu->AppendMenu (MF_STRING, ID_MENU_HIDE, "Hide");
			if (_MainFrame->_PRegionBuilder.isHidden(hItem))
				pMenu->CheckMenuItem (ID_MENU_HIDE, MF_CHECKED|MF_BYCOMMAND);
		}
		pMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, 
								r.left+point.x, r.top+point.y, this);
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnSelChanged (LPNMHDR pnmhdr, LRESULT *pLResult)
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
	HTREEITEM hParent = GetTreeCtrl().GetParentItem (hItem);

	if (hItem != NULL)
	{
		// Select the item
		GetTreeCtrl().Select(hItem, TVGN_CARET);
		if ((hParent == NULL) || (GetTreeCtrl().GetParentItem(hParent) == NULL))
			_MainFrame->_PRegionBuilder.setSelPB (NULL);
		else
			_MainFrame->_PRegionBuilder.setSelPB (hItem);
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuCreate()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	HTREEITEM parent = GetTreeCtrl().GetParentItem (item);
	uint32 i;
	CCreateDialog dialog (this);
	dialog.TypesForInit = &_MainFrame->_Types;

	for (i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == parent)
	{
		dialog.setRegionName (_RegionsInfo[i].Name);
		break;
	}

	dialog.MainFrame = _MainFrame;

	if (dialog.DoModal () == IDOK)
	if (strlen(dialog.Name) > 0)
	{
		HTREEITEM newItem = GetTreeCtrl().InsertItem (dialog.Name, item);
		GetTreeCtrl().Expand (item, TVE_EXPAND);
		// Create the newItem
		for (i = 0; i < _RegionsInfo.size(); ++i)
		if (_RegionsInfo[i].RegionItem == parent)
		{
			if (item == _RegionsInfo[i].PointItem)
			{
				_MainFrame->_PRegionBuilder.insertPoint (i, newItem, dialog.Name, dialog.LayerName);
			}
			else if (item == _RegionsInfo[i].PathItem)
			{
				_MainFrame->_PRegionBuilder.insertPath (i, newItem, dialog.Name, dialog.LayerName);
			}
			else if (item == _RegionsInfo[i].ZoneItem)
			{
				_MainFrame->_PRegionBuilder.insertZone (i, newItem, dialog.Name, dialog.LayerName);
			}
			break;
		}
		// Callback handling
		_MainFrame->primZoneModified();
	}

}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuDelete()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	GetTreeCtrl().DeleteItem (item);
	_MainFrame->_PRegionBuilder.del (item);

	// Callback handling
	_MainFrame->primZoneModified();
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuProperties()
{
	CCreateDialog dialog (this);

	HTREEITEM item = GetTreeCtrl().GetSelectedItem();
	HTREEITEM parent = GetTreeCtrl().GetParentItem(item);
	HTREEITEM parentparent = GetTreeCtrl().GetParentItem(parent);
	strcpy (dialog.Name, _MainFrame->_PRegionBuilder.getName(item));
	strcpy (dialog.LayerName, _MainFrame->_PRegionBuilder.getLayerName(item));
	dialog.TypesForInit = &_MainFrame->_Types;
	dialog.MainFrame = _MainFrame;
	dialog.PropName = _MainFrame->_PRegionBuilder.getName(item);
	dialog.PropType = _MainFrame->_PRegionBuilder.getLayerName(item);
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == parentparent)
	{
		dialog.setRegionName (_RegionsInfo[i].Name);
		break;
	}

	if (dialog.DoModal () == IDOK)
	if (strlen(dialog.Name) > 0)
	{
		GetTreeCtrl().SetItemText (item, dialog.Name);
		_MainFrame->_PRegionBuilder.setName (item, dialog.Name);
		_MainFrame->_PRegionBuilder.setLayerName (item, dialog.LayerName);
	}	
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuHideAll()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	HTREEITEM parent = GetTreeCtrl().GetParentItem (item);

	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == parent)
	{
		if (item == _RegionsInfo[i].PointItem)
			_MainFrame->_PRegionBuilder.hideAll (i, 0, true);
		if (item == _RegionsInfo[i].PathItem)
			_MainFrame->_PRegionBuilder.hideAll (i, 1, true);
		if (item == _RegionsInfo[i].ZoneItem)
			_MainFrame->_PRegionBuilder.hideAll (i, 2, true);
		break;
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuUnhideAll()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	HTREEITEM parent = GetTreeCtrl().GetParentItem (item);

	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == parent)
	{
		if (item == _RegionsInfo[i].PointItem)
			_MainFrame->_PRegionBuilder.hideAll (i, 0, false);
		if (item == _RegionsInfo[i].PathItem)
			_MainFrame->_PRegionBuilder.hideAll (i, 1, false);
		if (item == _RegionsInfo[i].ZoneItem)
			_MainFrame->_PRegionBuilder.hideAll (i, 2, false);
		break;
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuHide()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();	
	_MainFrame->_PRegionBuilder.hide (item);

}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuRegionHideAll ()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		_MainFrame->_PRegionBuilder.regionHideAll (i, true);
		break;
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuRegionUnhideAll ()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		_MainFrame->_PRegionBuilder.regionHideAll (i, false);
		break;
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuRegionHideType ()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		CTypeSelDlg dial(this);
		dial._TypesInit = &(_MainFrame->_Types);
		if (dial.DoModal() == IDOK)
		{
			_MainFrame->_PRegionBuilder.regionHideType (i, dial._TypeSelected, true);
		}
		break;
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuRegionUnhideType ()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	for (uint32 i = 0; i < _RegionsInfo.size(); ++i)
	if (_RegionsInfo[i].RegionItem == item)
	{
		CTypeSelDlg dial(this);
		dial._TypesInit = &(_MainFrame->_Types);
		if (dial.DoModal() == IDOK)
		{
			_MainFrame->_PRegionBuilder.regionHideType (i, dial._TypeSelected, false);
		}
		break;
	}
}

// ---------------------------------------------------------------------------
// CCreateDialog
// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCreateDialog, CDialog)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
		ON_CBN_SELCHANGE(IDC_COMBOTYPE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ---------------------------------------------------------------------------
CCreateDialog::CCreateDialog (CWnd*pParent) : CDialog(IDD_CREATE_ELEMENT, pParent) 
{
	Name[0] = 0;
	LayerName[0] = 0;
}

// ---------------------------------------------------------------------------
BOOL CCreateDialog::OnInitDialog ()
{
	CDialog::OnInitDialog();

	for (uint32 i = 0; i < TypesForInit->size(); ++i)
	{
		ComboType.InsertString (-1, TypesForInit->operator[](i).Name.c_str());
	}

	if (TypesForInit->size()>0)
	{
		if (ComboType.SelectString (0, LayerName) == CB_ERR)
			ComboType.SetCurSel (0);
		UpdateData();
		OnSelChange();
	}
	
	return true;
}

// ---------------------------------------------------------------------------
void CCreateDialog::DoDataExchange (CDataExchange* pDX )
{
	DDX_Control(pDX, IDC_COMBOTYPE, ComboType);

	DDX_Text(pDX, IDC_EDIT_NAME, (LPTSTR)Name, 128);
	DDV_MaxChars(pDX, Name, 128);

	DDX_Text(pDX, IDC_COMBOTYPE, (LPTSTR)LayerName, 128);
	DDV_MaxChars(pDX, LayerName, 128);
}

// ---------------------------------------------------------------------------
void CCreateDialog::OnOK()
{
	UpdateData ();

	// If the "region_" do not exist add it
	if (strncmp(RegionPost.c_str(), Name, strlen(RegionPost.c_str())) != 0)
	{
		char sTmp[128];
		strcpy (sTmp, RegionPost.c_str());
		strcat (sTmp, Name);
		strcpy (Name, sTmp);
		UpdateData (false);
	}

	if (strcmp(PropName.c_str(), Name) == 0)
		CDialog::OnOK();
	
	if (MainFrame->_PRegionBuilder.isAlreadyExisting (Name))
	{
		MessageBox("This Element already exist. Please type another name", "Eror", MB_ICONERROR|MB_OK);
		return;
	}
	CDialog::OnOK();
}

// ---------------------------------------------------------------------------
void CCreateDialog::setRegionName (const string &rn)
{
	for (uint32 i = 0; i < rn.size(); ++i)
	{
		if (rn[i] == '.')
		{
			RegionPost += '-';
			return;
		}
		RegionPost += rn[i];
	}
	RegionPost += '-';
}

// ---------------------------------------------------------------------------
void CCreateDialog::OnSelChange ()
{
	int cs = ComboType.GetCurSel();
	CString sTmp;
	ComboType.GetLBText (cs, sTmp);

	if (PropType == (LPCSTR)sTmp)
	{
		strcpy (Name, PropName.c_str());
	}
	else
	{
		strcpy (Name, RegionPost.c_str());
		strcat (Name, (LPCSTR)sTmp);
		strcat (Name, "-");

		int nPreNum = MainFrame->_PRegionBuilder.getMaxPostfix (Name);
		++nPreNum;
		char sNumTmp[2];
		sNumTmp[1] = 0;
		sNumTmp[0] = '0'+((nPreNum/100)%10);
		strcat (Name, sNumTmp);
		sNumTmp[0] = '0'+((nPreNum/10 )%10);
		strcat (Name, sNumTmp);
		sNumTmp[0] = '0'+((nPreNum/1  )%10);
		strcat (Name, sNumTmp);
	}

	UpdateData (false);
	Invalidate ();
}