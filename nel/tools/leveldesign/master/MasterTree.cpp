// MasterTree.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "MasterTree.h"
#include "Mainfrm.h"
#include "nel/misc/types_nl.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ---------------------------------------------------------------------------

#define IDC_TREE					0x1000

// Top level menus
#define ID_MENU_TRASH_EMPTY			0x0040
#define ID_MENU_BACKUP_EMPTY		0x0041
#define ID_MENU_BACKUP_CLEAN		0x0042
#define ID_MENU_BACKUP_RESTORE		0x0043
#define ID_MENU_REGION_NEW			0x0044
#define ID_MENU_REGION_TRASH		0x0045
#define ID_MENU_REGION_BACKUP		0x0046

#define ID_MENU_SORT_NAME_INC		0x0047
#define ID_MENU_SORT_NAME_DEC		0x0048
#define ID_MENU_SORT_DATE_INC		0x0049
#define ID_MENU_SORT_DATE_DEC		0x0050

// Region menus (a level under the top)

#define ID_MENU_TRASH_DELETE		0x0060
#define ID_MENU_TRASH_RESTORE_ONE	0x0061
#define ID_MENU_BACKUP_DELETE		0x0062
#define ID_MENU_BACKUP_RESTORE_ONE	0x0063
#define ID_MENU_REGION_DELETE		0x0064
#define ID_MENU_REGION_BACKUP_ONE	0x0065
#define ID_MENU_REGION_SET_ACTIVE	0x0066
#define ID_MENU_REGION_NEW_PRIM		0x0067
#define ID_MENU_REGION_NEW_GEORGES	0x0068

// ---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CMasterTreeDlg dialog
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP (CMasterTree, CTreeCtrl)

 	ON_WM_TIMER()

	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnLBeginDrag)

	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()

	ON_COMMAND(ID_MENU_TRASH_EMPTY,			OnMenuTrashEmpty)
	ON_COMMAND(ID_MENU_BACKUP_EMPTY,		OnMenuBackupEmpty)
	ON_COMMAND(ID_MENU_BACKUP_CLEAN,		OnMenuBackupClean)
	ON_COMMAND(ID_MENU_BACKUP_RESTORE,		OnMenuBackupRestore)
	ON_COMMAND(ID_MENU_REGION_NEW,			OnMenuRegionNew)
	ON_COMMAND(ID_MENU_REGION_TRASH,		OnMenuRegionTrash)
	ON_COMMAND(ID_MENU_REGION_BACKUP,		OnMenuRegionBackup)

	ON_COMMAND(ID_MENU_SORT_NAME_INC,		OnMenuSortNameInc)
	ON_COMMAND(ID_MENU_SORT_NAME_DEC,		OnMenuSortNameDec)
	ON_COMMAND(ID_MENU_SORT_DATE_INC,		OnMenuSortDateInc)
	ON_COMMAND(ID_MENU_SORT_DATE_DEC,		OnMenuSortDateDec)

	ON_COMMAND(ID_MENU_TRASH_DELETE,		OnMenuTrashDelete)
	ON_COMMAND(ID_MENU_TRASH_RESTORE_ONE,	OnMenuTrashRestoreOne)
	ON_COMMAND(ID_MENU_BACKUP_DELETE,		OnMenuBackupDelete)
	ON_COMMAND(ID_MENU_BACKUP_RESTORE_ONE,	OnMenuBackupRestoreOne)
	ON_COMMAND(ID_MENU_REGION_DELETE,		OnMenuRegionDelete)
	ON_COMMAND(ID_MENU_REGION_BACKUP_ONE,	OnMenuRegionBackupOne)
	ON_COMMAND(ID_MENU_REGION_SET_ACTIVE,	OnMenuRegionSetActive)
	ON_COMMAND(ID_MENU_REGION_NEW_PRIM,		OnMenuRegionNewPrim)
	ON_COMMAND(ID_MENU_REGION_NEW_GEORGES,	OnMenuRegionNewGeorges)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
CMasterTree::CMasterTree ()
{
	_LDrag = false;
	_LastItemSelected = NULL;
	_LastActiveRegion = NULL;
}

// ---------------------------------------------------------------------------
void CMasterTree::OnTimer (UINT nIDEvent)
{
	if( nIDEvent != m_nTimerID )
	{
		CTreeCtrl::OnTimer(nIDEvent);
		return;
	}

	// Doesn't matter that we didn't initialize m_timerticks
	m_timerticks++;

	POINT pt;
	GetCursorPos (&pt);
	RECT rect;
	GetClientRect (&rect);
	ClientToScreen (&rect);

	if( pt.y < rect.top + 10 )
	{
		CImageList::DragShowNolock (FALSE);
		SendMessage (WM_VSCROLL, SB_LINEUP);
		CImageList::DragShowNolock (TRUE);
	}
	else if( pt.y > rect.bottom - 10 )
	{
		CImageList::DragShowNolock (FALSE);
		SendMessage (WM_VSCROLL, SB_LINEDOWN);
		CImageList::DragShowNolock (TRUE);
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLBeginDrag (NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTREEVIEW *pNMTV = (NMTREEVIEW*)pNMHDR;
	_DragItem = pNMTV->itemNew.hItem;
	HTREEITEM hParent = GetParentItem (_DragItem);
	if ((hParent == NULL)&&(GetItemText(_DragItem) == "Trash"))
		return;
	if ((hParent != NULL)&&(GetParentItem(hParent) != NULL))
		return;
	_LDrag = true;
	m_nTimerID = SetTimer (1, 50, NULL);
	_DragImg = CreateDragImage (_DragItem);
	_DragImg->BeginDrag (0, CPoint (8, 8));
	_DragImg->DragEnter (this, ((NM_TREEVIEW *)pNMHDR)->ptDrag);
	SetCapture ();
	*pResult = false;
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLButtonDown (UINT nFlags, CPoint point)
{
	HTREEITEM hItem = HitTest (point);
	HTREEITEM hParent = GetParentItem (hItem) ;
	if ((hParent != NULL)&&(GetParentItem(hParent) == NULL))
	{
		if ((GetParentItem(_LastItemSelected) != NULL)&&(GetParentItem(GetParentItem(_LastItemSelected)) == NULL))
			Expand (_LastItemSelected, TVE_COLLAPSE);
		_LastItemSelected = hItem;
		Expand (hItem, TVE_EXPAND);
	}
	Select (hItem, TVGN_CARET);

	CTreeCtrl::OnLButtonDown (nFlags, point);
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLButtonDblClk (UINT nFlags, CPoint point)
{
	HTREEITEM hItem = HitTest (point);
	HTREEITEM hParent = GetParentItem (hItem);
	
	if (hItem != NULL)
	{
		CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();

		if (hParent == NULL)
			return;

		if ((hParent != NULL) && (GetParentItem (hParent) == NULL))
		{
			// Double click on a region open all editors
			// Parse all files and open them in the good editor
			HTREEITEM hChildItem = GetChildItem (hItem);
			while (hChildItem != NULL)
			{
				pDlg->openAnyFileFromItem (hChildItem);
				hChildItem = GetNextItem (hChildItem, TVGN_NEXT);
			}
			OnMenuRegionSetActive ();
			return;
		}
		else
		{
			// Found the region item in which the file has been clicked
			HTREEITEM hRegion = hParent;
			hParent = GetParentItem (hRegion);

			while (GetParentItem(hParent) != NULL)
			{
				hRegion = hParent;
				hParent = GetParentItem (hRegion);
			}

			CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent ();
			if (_LastActiveRegion != NULL)
				SetItem (_LastActiveRegion, TVIF_STATE, NULL, 0, 0, 0, TVIS_BOLD, 0);
			SetItem (hRegion, TVIF_STATE, NULL, 0, 0, TVIS_BOLD, TVIS_BOLD, 0);
			_LastActiveRegion = hRegion;
			pDlg->selectRegion (string((LPCSTR)GetItemText(hRegion)), string((LPCSTR)GetItemText(hParent)));
			// Open the file
			pDlg->openAnyFileFromItem (hItem);
		}
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnLButtonUp (UINT nFlags, CPoint point)
{
	if (_LDrag)
	{
		SelectDropTarget (NULL);
		_DragImg->DragLeave (this);
		_DragImg->EndDrag ();
		ReleaseCapture ();
		KillTimer (m_nTimerID);
		_LDrag = false;
		delete _DragImg;
		Invalidate ();

		HTREEITEM dragEndItem = HitTest (point);
		if (GetParentItem(_DragItem) == NULL)
		{
			if (GetItemText(_DragItem) == "Regions")
			{
				if (GetParentItem(dragEndItem) == NULL)
				{
					if (GetItemText(dragEndItem) == "Trash")
						OnMenuRegionTrash ();
					if (GetItemText(dragEndItem) == "Backup")
						OnMenuRegionBackup ();
				}
			}
			if (GetItemText(_DragItem) == "Backup")
			{
				if (GetParentItem(dragEndItem) == NULL)
				{
					if (GetItemText(dragEndItem) == "Trash")
						OnMenuBackupClean ();
					if (GetItemText(dragEndItem) == "Regions")
						OnMenuBackupRestore ();
				}
			}
		}
		else if (GetParentItem(GetParentItem(_DragItem)) == NULL)
		{
			if (GetItemText(GetParentItem(_DragItem)) == "Regions")
			{
				if (GetParentItem(dragEndItem) == NULL)
				{
					if (GetItemText(dragEndItem) == "Trash")
					{
						CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
						pDlg->regionDelete (GetItemText(_DragItem));
					}
					if (GetItemText(dragEndItem) == "Backup")
					{
						CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
						pDlg->regionBackupOne (GetItemText(_DragItem));
					}
				}
			}

			if (GetItemText(GetParentItem(_DragItem)) == "Backup")
			{
				if (GetParentItem(dragEndItem) == NULL)
				{
					if (GetItemText(dragEndItem) == "Regions")
					{
						CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
						pDlg->backupRestoreOne (GetItemText(_DragItem));
					}
					if (GetItemText(dragEndItem) == "Trash")
					{
						CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
						pDlg->backupDelete (GetItemText(_DragItem));
					}
				}
			}

			if (GetItemText(GetParentItem(_DragItem)) == "Trash")
			{
				if (GetParentItem(dragEndItem) == NULL)
				{
					if (GetItemText(dragEndItem) == "Regions")
					{
						CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
						pDlg->trashRestoreOne (GetItemText(_DragItem));
					}
				}
			}

		}
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMouseMove (UINT nFlags, CPoint point)
{
	if (_LDrag)
	{
		_DragImg->DragMove (point);
		_DragImg->DragShowNolock (FALSE);

		UINT Flags;
		HTREEITEM hItem = HitTest (point, &Flags);
		HTREEITEM hParent = GetParentItem (hItem) ;

		if ((_DragItem == NULL) || (hItem == NULL))
			return;

		if (GetParentItem(_DragItem) == NULL)
		{
			if ((GetParentItem(hItem) == NULL)&&(hItem != _DragItem))
			{
				if (GetDropHilightItem() != hItem)
					SelectDropTarget (hItem);
			}
			else
				SelectDropTarget (_DragItem);
		}
		else if (GetParentItem(GetParentItem(_DragItem)) == NULL)
		{
			if ((GetParentItem(hItem) == NULL) && 				
				((GetItemText(GetParentItem(_DragItem)) != "Trash") ||
				(GetItemText(hItem) != "Backup")))
			{
				if (GetDropHilightItem() != hItem)
					SelectDropTarget (hItem);
			}
			else
				SelectDropTarget (_DragItem);
		}
		else
			SelectDropTarget (_DragItem);
		
		_DragImg->DragShowNolock (TRUE);			
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnRButtonDown (UINT nFlags, CPoint point)
{
	UINT uFlags;
	HTREEITEM hItem = HitTest (point, &uFlags);
	HTREEITEM hParent = GetParentItem (hItem);
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		Select (hItem, TVGN_CARET);
		CRect r;
		this->GetWindowRect (r);
		CMenu *pMenu = new CMenu;
		pMenu->CreatePopupMenu ();

		map<HTREEITEM,string>::iterator it = pDlg->_Files.find (hItem);

		// System roots
		if (hParent == NULL)
		{
			if (GetItemText(hItem) == "Trash")
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_TRASH_EMPTY, "&Empty");
			}
			if (GetItemText(hItem) == "Backup")
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_BACKUP_EMPTY, "&Empty");
				pMenu->AppendMenu (MF_STRING, ID_MENU_BACKUP_CLEAN, "&Clean");
				pMenu->AppendMenu (MF_STRING, ID_MENU_BACKUP_RESTORE, "&Restore");
			}
			if (GetItemText(hItem) == "Regions")
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_NEW, "&New");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_BACKUP, "&Backup");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_TRASH, "&Trash");
			}
			pMenu->AppendMenu (MF_SEPARATOR);
			pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_NAME_DEC, "Sort By Name (A-Z)");
			pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_NAME_INC, "Sort By Name (Z-A)");
			pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_DATE_INC, "Sort By Date (New-Old)");
			pMenu->AppendMenu (MF_STRING, ID_MENU_SORT_DATE_DEC, "Sort By Date (Old-New)");
		}
		// Just under the roots
		else if ((hParent != NULL) && (GetParentItem (hParent) == NULL))
		{
			if (GetItemText(hParent) == "Trash")
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_SET_ACTIVE, "&Set Active");
				pMenu->AppendMenu (MF_STRING, ID_MENU_TRASH_DELETE, "&Delete");
				pMenu->AppendMenu (MF_STRING, ID_MENU_TRASH_RESTORE_ONE, "&Restore");
			}
			if (GetItemText(hParent) == "Backup")
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_SET_ACTIVE, "&Set Active");
				pMenu->AppendMenu (MF_STRING, ID_MENU_BACKUP_DELETE, "&Delete");
				pMenu->AppendMenu (MF_STRING, ID_MENU_BACKUP_RESTORE_ONE, "&Restore");
			}
			if (GetItemText(hParent) == "Regions")
			{
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_SET_ACTIVE, "&Set Active");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_NEW_PRIM, "New &Patat");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_NEW_GEORGES, "New &Form");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_DELETE, "&Delete");
				pMenu->AppendMenu (MF_STRING, ID_MENU_REGION_BACKUP_ONE, "&Backup");
			}
		}

		pMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON, 
								r.left+point.x, r.top+point.y, this);
	}
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuTrashEmpty ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->emptyTrash ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuBackupEmpty ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->emptyBackup ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuBackupClean ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->cleanBackup ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuBackupRestore ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->backupRestoreAll ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionNew ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionNew ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionTrash ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionTrashAll ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionBackup ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionBackupAll ();
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuSortNameInc ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	CString str = GetItemText (GetSelectedItem());
	if (str == "Regions")
		pDlg->sortRegionBy (MT_SORT_BY_NAME_INC);
	if (str == "Trash")
		pDlg->sortTrashBy (MT_SORT_BY_NAME_INC);
	if (str == "Backup")
		pDlg->sortBackupBy (MT_SORT_BY_NAME_INC);
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuSortNameDec ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	CString str = GetItemText (GetSelectedItem());
	if (str == "Regions")
		pDlg->sortRegionBy (MT_SORT_BY_NAME_DEC);
	if (str == "Trash")
		pDlg->sortTrashBy (MT_SORT_BY_NAME_DEC);
	if (str == "Backup")
		pDlg->sortBackupBy (MT_SORT_BY_NAME_DEC);
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuSortDateInc ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	CString str = GetItemText (GetSelectedItem());
	if (str == "Regions")
		pDlg->sortRegionBy (MT_SORT_BY_DATE_INC);
	if (str == "Trash")
		pDlg->sortTrashBy (MT_SORT_BY_DATE_INC);
	if (str == "Backup")
		pDlg->sortBackupBy (MT_SORT_BY_DATE_INC);
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuSortDateDec ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	CString str = GetItemText (GetSelectedItem());
	if (str == "Regions")
		pDlg->sortRegionBy (MT_SORT_BY_DATE_DEC);
	if (str == "Trash")
		pDlg->sortTrashBy (MT_SORT_BY_DATE_DEC);
	if (str == "Backup")
		pDlg->sortBackupBy (MT_SORT_BY_DATE_DEC);
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuTrashDelete ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->trashDelete (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuTrashRestoreOne ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->trashRestoreOne (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuBackupDelete ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->backupDelete (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuBackupRestoreOne ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->backupRestoreOne (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionDelete ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionDelete (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionBackupOne ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionBackupOne (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionSetActive ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent ();
	HTREEITEM hItem = GetSelectedItem ();
	HTREEITEM hParent = GetParentItem (hItem);

	if (_LastActiveRegion != NULL)
		SetItem (_LastActiveRegion, TVIF_STATE, NULL, 0, 0, 0, TVIS_BOLD, 0);
	SetItem (hItem, TVIF_STATE, NULL, 0, 0, TVIS_BOLD, TVIS_BOLD, 0);
	_LastActiveRegion = hItem;
	pDlg->selectRegion (string((LPCSTR)GetItemText(hItem)), string((LPCSTR)GetItemText(hParent)));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionNewPrim ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionNewPrim (GetItemText(GetSelectedItem()));
}

// ---------------------------------------------------------------------------
void CMasterTree::OnMenuRegionNewGeorges ()
{
	CMasterTreeDlg *pDlg = (CMasterTreeDlg*)GetParent();
	pDlg->regionNewGeorges (GetItemText(GetSelectedItem()));
}

/////////////////////////////////////////////////////////////////////////////
// CMasterTreeDlg dialog
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMasterTreeDlg, CDialog)
	//{{AFX_MSG_MAP(CMasterTreeDlg)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE, OnExpanding)
	//ON_NOTIFY(NM_DBLCLK, IDC_TREE, OnLDblClkTree)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// ---------------------------------------------------------------------------
CMasterTreeDlg::CMasterTreeDlg ()
{
	_Tree = NULL;
	RegionSortBy = 1;	// 0/1 - by name
	TrashSortBy = 1;	// 2/3 - by date
	BackupSortBy = 2;
}

// ---------------------------------------------------------------------------
#define MAX_SYS_DIR 5
char *gSysDir[MAX_SYS_DIR] = 
{
	".",
	"..",
	"ZoneBitmaps",
	"ZoneLigos",
	"DFN"
};

/*#define MAX_VALID_EXT	2
char *gValidExt[MAX_VALID_EXT] =
{
	".prim",
	".land",
};*/

FILETIME getFileTime (const string &fname)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	FILETIME ret = { 0xFFFFFFFF, 0xFFFFFFFF };

	hFind = FindFirstFile (fname.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) 
	{
		ret = FindFileData.ftLastWriteTime;
		FindClose (hFind);
	}
	return ret;
}

void CMasterTreeDlg::parseAdd(HTREEITEM itRoot, const string &path, char SortType)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_TREE);
	vector<string> SortTable;
	sint32 i, j;

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	if (!SetCurrentDirectory (path.c_str()))
	{
		SetCurrentDirectory (sCurDir);
		return;
	}
	
	hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		// Look if the name is a system directory
		bool bFound = false;
		for (uint32 i = 0; i < MAX_SYS_DIR; ++i)
			if (stricmp (findData.cFileName, gSysDir[i]) == 0)
			{
				bFound = true;
				break;
			}
		if (!bFound) // No ok lets add it
		{
			SortTable.push_back(findData.cFileName);
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	if (SortTable.size() == 0)
	{
		SetCurrentDirectory (sCurDir);
		return;
	}
		
	if (SortType == 0) // Sort By Name increasing (z-a)
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		for (j = i+1; j < (sint32)SortTable.size(); ++j)
			if (strcmp(SortTable[i].c_str(), SortTable[j].c_str()) < 0)
			{
				string tmp = SortTable[i];
				SortTable[i] = SortTable[j];
				SortTable[j] = tmp;
			}
	}
	if (SortType == 1) // Sort By Name decreasing (a-z)
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		for (j = i+1; j < (sint32)SortTable.size(); ++j)
			if (strcmp(SortTable[i].c_str(), SortTable[j].c_str()) > 0)
			{
				string tmp = SortTable[i];
				SortTable[i] = SortTable[j];
				SortTable[j] = tmp;
			}
	}
	if (SortType == 2) // Sort By Date increasing
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		{
			FILETIME timeI = getFileTime (SortTable[i]);
			for (j = i+1; j < (sint32)SortTable.size(); ++j)
			{
				FILETIME timeJ = getFileTime (SortTable[j]);
				if ((timeI.dwHighDateTime < timeJ.dwHighDateTime) ||
					(
						(timeI.dwHighDateTime == timeJ.dwHighDateTime) &&
						(timeI.dwLowDateTime < timeJ.dwLowDateTime)
					))
				{
					string tmp = SortTable[i];
					SortTable[i] = SortTable[j];
					SortTable[j] = tmp;
					timeI = timeJ;
				}
			}
		}
	}

	if (SortType == 3) // Sort By Date decreasing
	{
		for (i = 0; i < (sint32)SortTable.size()-1; ++i)
		{
			FILETIME timeI = getFileTime (SortTable[i]);
			for (j = i+1; j < (sint32)SortTable.size(); ++j)
			{
				FILETIME timeJ = getFileTime (SortTable[j]);
				if ((timeI.dwHighDateTime > timeJ.dwHighDateTime) ||
					(
						(timeI.dwHighDateTime == timeJ.dwHighDateTime) &&
						(timeI.dwLowDateTime > timeJ.dwLowDateTime)
					))
				{
					string tmp = SortTable[i];
					SortTable[i] = SortTable[j];
					SortTable[j] = tmp;
					timeI = timeJ;
				}
			}
		}
	}
	for (i = 0; i < (sint32)SortTable.size(); ++i)
	{
		if (GetFileAttributes(SortTable[i].c_str())&FILE_ATTRIBUTE_DIRECTORY)
		{
			HTREEITEM item = pTree->InsertItem (SortTable[i].c_str(), 0, 0, itRoot);
			string newPath = path;
			newPath += "\\";
			newPath += SortTable[i].c_str();
			parseAdd (item, newPath, SortType);
			//pTree->Expand (item, TVE_EXPAND);
		}
		else
		{
			// Look if the name is a valid filename (look at the extension)
			/*
			bool bFound = false;
			for (uint32 j = 0; j < MAX_VALID_EXT; ++j)
				if (strlen(SortTable[i].c_str()) > strlen(gValidExt[j]))
				if (stricmp(&SortTable[i].c_str()[strlen(SortTable[i].c_str())-strlen(gValidExt[j])], gValidExt[j]) == 0)
				{
					bFound = true;
					break;
				}
			if (bFound)
			{
				*/
				HTREEITEM item = pTree->InsertItem (SortTable[i].c_str(), 1, 1, itRoot);
				string sTemp = path;
				sTemp += "\\";
				sTemp += SortTable[i].c_str();
				_Files.insert(map<HTREEITEM,string>::value_type(item, sTemp));
			//}

		}
	}
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::update (const string &path)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem (IDC_TREE);

	_Files.clear();
	pTree->DeleteAllItems ();
	// Parse all the path
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	HTREEITEM hItemReg = pTree->InsertItem ("Regions", 2, 2, TVI_ROOT);
	parseAdd (hItemReg, path + "Regions", RegionSortBy);
	pTree->Expand (hItemReg, TVE_EXPAND);
	HTREEITEM hItemTrash = pTree->InsertItem ("Trash", 4, 4, TVI_ROOT);
	parseAdd (hItemTrash, path + "Trash", TrashSortBy);
	pTree->Expand (hItemTrash, TVE_EXPAND);
	HTREEITEM hItemBack = pTree->InsertItem ("Backup", 3, 3, TVI_ROOT);
	parseAdd (hItemBack, path + "Backup", BackupSortBy);
	pTree->Expand (hItemBack, TVE_EXPAND);
	pTree->EnsureVisible(hItemReg);
	SetCurrentDirectory (sCurDir);
	// Hilight active region
	CMainFrame *pMF = (CMainFrame *)GetParent ();
	HTREEITEM hItem;
	if (pMF->getActiveRegionPath() == "Regions")
		hItem = hItemReg;
	else if (pMF->getActiveRegionPath() == "Trash")
		hItem = hItemTrash;
	else if (pMF->getActiveRegionPath() == "Backup")
		hItem = hItemBack;
	else return;
	HTREEITEM hChildItem = pTree->GetChildItem (hItem);
	bool bFound = false;
	while (hChildItem != NULL)
	{
		if (pMF->getActiveRegion() == (LPCSTR)pTree->GetItemText(hChildItem))
		{
			pTree->SetItem (hChildItem, TVIF_STATE, NULL, 0, 0, TVIS_BOLD, TVIS_BOLD, 0);
			bFound = true;
			break;
		}

		hChildItem = pTree->GetNextItem (hChildItem, TVGN_NEXT);
	}
	if (!bFound)
	{
		pMF->setActiveRegion ("","");
	}
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange (pDX);
	//{{AFX_DATA_MAP(CMasterTreeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ---------------------------------------------------------------------------
BOOL CMasterTreeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	_Tree = new CMasterTree;
	RECT r;
	r.left = r.top = 10;
	r.right = r.bottom = 40;
	_Tree->Create (WS_VISIBLE|WS_BORDER, r, this, IDC_TREE);
	// Load image list
	CImageList *pImgList = new CImageList;
	pImgList->Create (16, 16, ILC_MASK, 0, 5);
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_FOLDER));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_FILE));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_REGIONS));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_BACKUP));
	pImgList->Add (AfxGetApp()->LoadIcon(IDI_TRASH));
	//CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_TREE);
	_Tree->SetImageList (pImgList, TVSIL_NORMAL);
	return true;
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnSize (UINT nType, int cx, int cy)
{
	if (_Tree)
		_Tree->SetWindowPos (&wndTop, 0, 0, cx-20, cy-20, SWP_NOMOVE);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnExpanding (LPNMHDR pnmhdr, LRESULT *pLResult)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem (IDC_TREE);
	NMTREEVIEW *pnmtv = (LPNMTREEVIEW) pnmhdr;

	if ((pnmtv->itemNew.state & TVIS_EXPANDED) != 0)
	{
		// We want to collapse this item -> forbid it
		*pLResult = true;
	}
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::openAnyFile (const char *fname)
{
	int size = strlen (fname);
	CMainFrame *pMF = (CMainFrame *)GetParent ();
	if ((stricmp(&fname[size-5],".prim") == 0) || (stricmp(&fname[size-5],".land") == 0))
	{
		pMF->openWorldEditor ();
		pMF->openWorldEditorFile (fname);
		return;
	}
	if (stricmp(&fname[size-6],".logic") == 0)
	{
		pMF->openLogicEditor ();
		pMF->openLogicEditorFile (fname);
		return;
	}
	pMF->openGeorges ();
	pMF->openGeorgesFile (fname);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::openAnyFileFromItem (HTREEITEM hItem)
{
	map<HTREEITEM,string>::iterator it = _Files.find (hItem);
	if (it != _Files.end())
	{
		openAnyFile (it->second.c_str());
	}
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::emptyTrash ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->emptyTrash ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::emptyBackup ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->emptyBackup ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::cleanBackup ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->cleanBackup ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionBackupAll ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionBackupAll ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionNew ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->onRegionNew ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionTrashAll ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionTrashAll ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::backupRestoreAll ()
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->backupRestoreAll ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::sortRegionBy (int mt_sort_type)
{
	RegionSortBy = mt_sort_type;
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->updateTree ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::sortTrashBy (int mt_sort_type)
{
	TrashSortBy = mt_sort_type;
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->updateTree ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::sortBackupBy (int mt_sort_type)
{
	BackupSortBy = mt_sort_type;
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->updateTree ();
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::trashDelete (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->trashDelete (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::trashRestoreOne (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->trashRestoreOne (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::backupDelete (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->backupDelete (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::backupRestoreOne (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->backupRestoreOne (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionDelete (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionDelete (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionBackupOne (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionBackupOne (str);
}
// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionNewPrim (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionNewPrim (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::regionNewGeorges (const char *str)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->regionNewGeorges (str);
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::selectRegion (const string &Region, const string &Directory)
{
	CMainFrame *pMF = (CMainFrame *)GetParent();
	pMF->setActiveRegion (Region, Directory);
}

// ---------------------------------------------------------------------------
/*
void CMasterTreeDlg::OnLDblClkTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem (IDC_TREE);
	NMTREEVIEW *pnmtv = (LPNMTREEVIEW) pNMHDR;
	HTREEITEM hItem = pTree->GetSelectedItem();
	HTREEITEM hParent = pTree->GetParentItem (hItem);
	
	*pResult = 0;
	if (hItem != NULL)
	{
		map<HTREEITEM,string>::iterator it = _Files.find (hItem);

		if (it != _Files.end())
		{
			// Double click on a file open the corresponding editor
			openAnyFile (it->second.c_str());
			return;
		}

		if ((hParent != NULL) && (pTree->GetParentItem (hParent) == NULL))
		{
			// Double click on a region open all editors
			// Parse all files and open them in the good editor
			HTREEITEM hChildItem = pTree->GetChildItem (hItem);

			while (hChildItem != NULL)
			{
				it = _Files.find (hChildItem);
				if (it != _Files.end())
				{
					openAnyFile (it->second.c_str());
				}
				hChildItem = pTree->GetNextItem (hChildItem, TVGN_NEXT);
			}
			return;
		}
	}

}
*/
// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnCancel ()
{
}

// ---------------------------------------------------------------------------
void CMasterTreeDlg::OnOK ()
{
}

