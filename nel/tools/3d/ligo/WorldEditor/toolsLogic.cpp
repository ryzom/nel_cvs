#include "stdafx.h"
#include "toolsLogic.h"

#include "mainfrm.h"

#include <string>

using namespace std;

// ---------------------------------------------------------------------------

#define ID_MENU_CREATE		0x10010
#define ID_MENU_DELETE		0x10011
#define ID_MENU_PROPERTIES	0x10012

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
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CToolsLogic, CTreeView)

// ---------------------------------------------------------------------------
CToolsLogic::CToolsLogic ()
{
}

// ---------------------------------------------------------------------------
void CToolsLogic::init(CMainFrame *pMF)
{
	_MainFrame = pMF;
	pMF->_PRegionBuilder.setToolsLogic (this);

	_PointItem = GetTreeCtrl().InsertItem ("Points");
	_PathItem = GetTreeCtrl().InsertItem ("Splines");
	_ZoneItem = GetTreeCtrl().InsertItem ("Patatoids");

	pMF->_PRegionBuilder.updateToolsLogic ();
}

// ---------------------------------------------------------------------------
void CToolsLogic::uninit()
{
	_MainFrame->_PRegionBuilder.setSelPB (NULL);
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnRButtonDown (UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = GetTreeCtrl().HitTest(point, &uFlags);

	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		// Select the item
		GetTreeCtrl().Select(hItem, TVGN_CARET);

		// Menu creation
		CRect r;
		this->GetWindowRect (r);
		CMenu *pMenu = new CMenu;
		pMenu->CreatePopupMenu ();

		if (GetTreeCtrl().GetParentItem(hItem) == NULL)
		{
			// Display the contextual menu : create
			pMenu->AppendMenu (MF_STRING, ID_MENU_CREATE, "Create");
		}
		else
		{
			// Display the contextual menu : properties
			pMenu->AppendMenu (MF_STRING, ID_MENU_DELETE, "Delete");
			pMenu->AppendMenu (MF_STRING, ID_MENU_PROPERTIES, "Properties");
		}
		pMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, 
								r.left+point.x, r.top+point.y, this);
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnSelChanged (LPNMHDR pnmhdr, LRESULT *pLResult)
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();

	if (hItem != NULL)
	{
		// Select the item
		GetTreeCtrl().Select(hItem, TVGN_CARET);
		if ((hItem != _PointItem) && (hItem != _PathItem) && (hItem != _ZoneItem))
			_MainFrame->_PRegionBuilder.setSelPB (hItem);
		else
			_MainFrame->_PRegionBuilder.setSelPB (NULL);
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuCreate()
{
	CCreateDialog dialog (this);
	if (dialog.DoModal () == IDOK)
	{
		HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
		HTREEITEM newItem = GetTreeCtrl().InsertItem (dialog.Name, item);
		GetTreeCtrl().Expand (item, TVE_EXPAND);
		// Create the newItem
		if (item == _PointItem)
		{
			_MainFrame->_PRegionBuilder.insertPoint (newItem, dialog.Name, dialog.LayerName);
		}
		else if (item == _PathItem)
		{
			_MainFrame->_PRegionBuilder.insertPath (newItem, dialog.Name, dialog.LayerName);
		}
		else if (item == _ZoneItem)
		{
			_MainFrame->_PRegionBuilder.insertZone (newItem, dialog.Name, dialog.LayerName);
		}
	}
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuDelete()
{
	HTREEITEM item = GetTreeCtrl().GetSelectedItem ();
	GetTreeCtrl().DeleteItem (item);
	_MainFrame->_PRegionBuilder.del (item);
}

// ---------------------------------------------------------------------------
void CToolsLogic::OnMenuProperties()
{
	CCreateDialog dialog (this);
	HTREEITEM item = GetTreeCtrl().GetSelectedItem();
	strcpy (dialog.Name, _MainFrame->_PRegionBuilder.getName(item));
	strcpy (dialog.LayerName, _MainFrame->_PRegionBuilder.getLayerName(item));
	if (dialog.DoModal () == IDOK)
	{
		GetTreeCtrl().SetItemText (item, dialog.Name);
		_MainFrame->_PRegionBuilder.setName (item, dialog.Name);
		_MainFrame->_PRegionBuilder.setLayerName (item, dialog.LayerName);
	}	
}
