// ---------------------------------------------------------------------------

#include "stdafx.h"
#include "dfnGrid.h"
#include "resource.h"
#include "dfnview.h"

#include "ComboInListView.h"

// ---------------------------------------------------------------------------

using namespace std;

// ---------------------------------------------------------------------------
// CDfnGrid
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
#define ID_MENU_ADD	0x0101
#define ID_MENU_DEL	0x0102

// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CDfnGrid, CSuperGridCtrl)
	ON_COMMAND (ID_MENU_ADD, OnAdd)
	ON_COMMAND (ID_MENU_DEL, OnDel)
	ON_WM_RBUTTONDOWN ()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
CDfnGrid::CDfnGrid ()
{
	_NbItems = 0;
}

// ---------------------------------------------------------------------------
int CDfnGrid::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CSuperGridCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Because superGrid do not work if there is no image list ... pfffff ...

	if (!m_image.Create(IDB_FOLDERS, 16, 1, RGB(0, 255, 255)))
		return -1;

	SetImageList (&m_image, LVSIL_SMALL);
	CImageList *pImageList = GetImageList (LVSIL_SMALL);
	if (pImageList)
		ImageList_GetIconSize (pImageList->m_hImageList, &m_cxImage, &m_cyImage);
	else
		return -1;
	
	return 0;
}

// ---------------------------------------------------------------------------
void CDfnGrid::OnRButtonDown (UINT nFlags, CPoint pt)
{
	_ItemSelected = -1;
	CSuperGridCtrl::OnRButtonDown (nFlags, pt);

	CMenu *pMenu = new CMenu;
	pMenu->CreatePopupMenu ();
	pMenu->AppendMenu (MF_STRING, ID_MENU_ADD, "Add");
	if (_ItemSelected != -1)
	{
		pMenu->AppendMenu (MF_STRING, ID_MENU_DEL, "Delete");
	}
	CRect r;
	this->GetWindowRect (r);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, r.left+pt.x, r.top+pt.y, this);
}

// ---------------------------------------------------------------------------
void CDfnGrid::OnControlLButtonDown (UINT nFlags, CPoint point, LVHITTESTINFO& ht)
{
	//now I am sure I added a combobox some where, so check for this control
	CTreeItem*pSelItem = GetTreeItem(ht.iItem);
	if(pSelItem!=NULL)
	{	
		CItemInfo* pInfo = GetData(pSelItem);
		CItemInfo::CONTROLTYPE ctrlType;
		if (pInfo->GetControlType(ht.iSubItem-1, ctrlType))
		{	
			if (ctrlType == CItemInfo::CONTROLTYPE::combobox) 
			{
				if (ht.iSubItem == 2)
				{
					// Get All .typ and .dfn
					GetAllTypAndDfn ();

					// Convert to supergrid struct
					CStringList lst;
					for (uint32 i = 0; i < _DfnTypList.size(); ++i)
						lst.AddTail (_DfnTypList[i].c_str());
					pInfo->SetListData (ht.iSubItem-1, &lst);
					CStringList* list=NULL;
					pInfo->GetListData(ht.iSubItem-1, list);
					CComboBox * pList = ShowList (ht.iItem, ht.iSubItem, list, true);
				}
				else
				{
					CStringList lst;
					lst.AddTail ("----");
					lst.AddTail ("list");
					pInfo->SetListData (ht.iSubItem-1, &lst);
					CStringList* list=NULL;
					pInfo->GetListData(ht.iSubItem-1, list);
					CComboBox * pList = ShowList(ht.iItem, ht.iSubItem, list, false);
				}
			}
			else // activate default edit control
			{
				EditLabelEx (ht.iItem, ht.iSubItem);
			}
		}
	}
}

// ---------------------------------------------------------------------------
void CDfnGrid::OnControlRButtonDown(UINT nFlags, CPoint pt, LVHITTESTINFO& ht)
{
	CMenu *pMenu = new CMenu;
	pMenu->CreatePopupMenu ();

	if (GetTreeItem(ht.iItem) == NULL)
		return;
	_ItemSelected = ht.iItem;
	/*
	pMenu->AppendMenu (MF_STRING, ID_MENU_DEL, "Delete");
	_ItemSelected = ht.iItem;
	pMenu->AppendMenu (MF_STRING, ID_MENU_ADD, "Add");
	CRect r;
	this->GetWindowRect (r);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, r.left+pt.x, r.top+pt.y, this);
	*/
}

// ---------------------------------------------------------------------------
void CDfnGrid::OnAdd ()
{
	SetRedraw (FALSE);

	CItemInfo *pII = new CItemInfo(_NbItems);
	++_NbItems;

	string tmp = NLMISC::toString("Unknown") + NLMISC::toString(_NbItems);
	pII->SetItemText (tmp.c_str());
	pII->SetControlType (CItemInfo::edit, -1);
	pII->AddSubItemText ("---");
	pII->SetControlType (CItemInfo::combobox, 0);
	pII->AddSubItemText ("uint8.typ");
	pII->SetControlType (CItemInfo::combobox, 1);
	pII->SetImage (2);

	InsertRootItem (pII);
	
	SetRedraw (TRUE);
	InvalidateRect (NULL);
	UpdateWindow ();
	((CDfnView*)GetParent ())->GetDocument()->Push();
}

// ---------------------------------------------------------------------------
void CDfnGrid::OnDel ()
{
	if (_ItemSelected != -1)
	{
		SetRedraw (FALSE);

		CTreeItem* pSelItem = GetTreeItem(_ItemSelected);
		if(pSelItem != NULL)
		{
			if(OnDeleteItem(pSelItem, _ItemSelected))
				DeleteItemEx(pSelItem, _ItemSelected);
		}	

///		DeleteRootItem (GetRootItem(_ItemSelected));
///		SelectNode (GetRootItem(_ItemSelected-1));
		
		SetRedraw (TRUE);
		InvalidateRect (NULL);
		UpdateWindow ();

		((CDfnView*)GetParent ())->GetDocument()->Push();
	}
}

// ---------------------------------------------------------------------------
void CDfnGrid::OnUpdateListViewItem (CTreeItem* lpItem, LV_ITEM *plvItem)
{
	CSuperGridCtrl::OnUpdateListViewItem(lpItem, plvItem);
	((CDfnView*)GetParent ())->GetDocument()->Push();
}

// ---------------------------------------------------------------------------
void CDfnGrid::GetAllTypAndDfn ()
{
	_DfnTypList.clear ();
	CDfnView *pDfnView = (CDfnView*)GetParent();
	CDfnDoc *pDfnDoc = (CDfnDoc*)pDfnView->GetDocument();
	parseAdd (pDfnDoc->DirDfnTyp, _DfnTypList);
}

// ---------------------------------------------------------------------------
void CDfnGrid::parseAdd (const string &path, vector<string> &dfntyplist)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;

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

		if (GetFileAttributes(findData.cFileName)&FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((stricmp(findData.cFileName, ".") != 0) &&
				(stricmp(findData.cFileName, "..") != 0))
			{
				string newPath = path;
				newPath += "\\";
				newPath += findData.cFileName;
				parseAdd (newPath, dfntyplist);
			}
		}
		else
		{
			if (strlen(findData.cFileName)>4)
			if ((stricmp(findData.cFileName+strlen(findData.cFileName)-4,".typ") == 0) ||
				(stricmp(findData.cFileName+strlen(findData.cFileName)-4,".dfn") == 0))
			{
				dfntyplist.push_back(string(findData.cFileName));
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
#define IDC_COMBOBOXINLISTVIEW 0x1235
CComboBox* CDfnGrid::ShowList (int nItem, int nCol, CStringList *lstItems, bool bScrollBar)
{
	CString strFind = GetItemText (nItem, nCol);

	//basic code start
	CRect rect;
	int offset = 0;
	// Make sure that the item is visible
	if (!EnsureVisible(nItem, TRUE)) 
		return NULL;
	GetSubItemRect (nItem, nCol, LVIR_BOUNDS, rect);
	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect (rcClient);
	if (offset + rect.left < 0 || offset + rect.left > rcClient.right)
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}
	
	rect.left += offset;	
	rect.right = rect.left + GetColumnWidth(nCol);
	if (rect.right > rcClient.right)
	   rect.right = rcClient.right;
	//basic code end

	rect.bottom += 10 * rect.Height();//dropdown area
	DWORD dwStyle;
	if (bScrollBar)
		dwStyle	=  WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL;
	else
		dwStyle	=  WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL;
	CComboBox *pList = new CComboInListView (nItem, nCol, lstItems);
	pList->Create (dwStyle, rect, this, IDC_COMBOBOXINLISTVIEW);
	pList->ModifyStyleEx (0, WS_EX_CLIENTEDGE);
	pList->SetHorizontalExtent (CalcHorzExtent(pList, lstItems));
	pList->ShowDropDown ();
	pList->SelectString (-1, strFind.GetBuffer(1));
	// The returned pointer should not be saved
	return pList;
}

// ---------------------------------------------------------------------------
int CDfnGrid::CalcHorzExtent(CWnd* pWnd, CStringList *pList)
{
	int nExtent=0;
	if(pWnd!=NULL)
	{
		CDC* pDC = pWnd->GetDC();
		HFONT hFont = (HFONT)pWnd->SendMessage(WM_GETFONT); //why not pWnd->GetFont();..I like the send thing alot and
		CFont *pFont = CFont::FromHandle(hFont);			//this way I get to use this function..cool :)
		if(pFont!=NULL)										//ya what ever makes me happy,.right :}
		{
			CFont* pOldFont = pDC->SelectObject(pFont);
			CSize newExtent;
			POSITION pos = pList->GetHeadPosition();
			while(pos != NULL)
			{
				CString str(pList->GetNext(pos));
				newExtent = pDC->GetTextExtent (str);
				newExtent.cx += 6;
				if (newExtent.cx > nExtent)
				{
					nExtent = newExtent.cx;
				}
			}
			pDC->SelectObject(pOldFont);
		}
		pWnd->ReleaseDC(pDC);
	}
	return nExtent;
}

// ---------------------------------------------------------------------------
BOOL CDfnGrid::OnVkReturn ()
{
	BOOL bResult = FALSE;
	int iItem = GetNextItem( -1, LVNI_ALL | LVNI_SELECTED);
	if( GetCurSubItem() != -1 && iItem != -1)
	{
		CTreeItem*pSelItem = GetTreeItem(iItem);
		if(pSelItem!=NULL)
		{	
			CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
			int iSubItem = Header_OrderToIndex(pHeader->m_hWnd, GetCurSubItem());
			CItemInfo* pInfo = GetData(pSelItem);
			CItemInfo::CONTROLTYPE ctrlType;
			if(pInfo->GetControlType(iSubItem-1, ctrlType))
			{	
				switch(ctrlType)
				{
					case CItemInfo::CONTROLTYPE::datecontrol:
					break;

					case CItemInfo::CONTROLTYPE::spinbutton:
					break;

					case CItemInfo::CONTROLTYPE::dropdownlistviewwhatevercontrol:
					break;

					case CItemInfo::CONTROLTYPE::combobox: 
						if (iSubItem == 2)
						{
							// Get All .typ and .dfn
							GetAllTypAndDfn ();

							// Convert to supergrid struct
							CStringList lst;
							for (uint32 i = 0; i < _DfnTypList.size(); ++i)
								lst.AddTail (_DfnTypList[i].c_str());
							pInfo->SetListData (iSubItem-1, &lst);
							CStringList* list=NULL;
							pInfo->GetListData(iSubItem-1, list);
							CComboBox * pList = ShowList (iItem, iSubItem, list, true);
						}
						else
						{
							CStringList lst;
							lst.AddTail ("----");
							lst.AddTail ("list");
							pInfo->SetListData (iSubItem-1, &lst);
							CStringList* list=NULL;
							pInfo->GetListData (iSubItem-1, list);
							CComboBox * pList = ShowList (iItem, iSubItem, list, false);
						}
						bResult = TRUE;
					break;

					default:
					break;
				}
			}
		}
	}
	return bResult;
}

// ---------------------------------------------------------------------------
void CDfnGrid::convertToDfn (CDfnDoc::TDfn &out)
{
	// From the grid to the dfn
	POSITION pos = GetRootHeadPosition ();

	while (pos != NULL)
	{
		CSuperGridCtrl::CTreeItem *pTI = GetNextRoot (pos);
		CItemInfo *pII = GetData (pTI);
		string name = (LPCSTR)pII->GetItemText();
		string isList = (LPCSTR)pII->GetSubItem(0);
		string type = (LPCSTR)pII->GetSubItem(1);
		if (isList == "list")
			type = string("list< ") + type + string(" >");
		out.push_back (make_pair<CStringEx,CStringEx>(name,type));
	}
}

// ---------------------------------------------------------------------------
void CDfnGrid::makeFromDfn (CDfnDoc::TDfn &in)
{
	SetRedraw (FALSE);
	DeleteAll ();
	for (uint32 i = 0; i < in.size(); ++i)
	{
		CItemInfo *pII = new CItemInfo (_NbItems++);

		// Name
		pII->SetItemText (in[i].first.c_str());
		pII->SetControlType (CItemInfo::edit, -1);

		// List
		CStringEx slist = in[i].second.get_left (4);
		if (slist == "list")
			pII->AddSubItemText ("list");
		else
			pII->AddSubItemText ("---");
		pII->SetControlType (CItemInfo::combobox, 0);

		// Type
		if (slist == "list")
		{
			CStringEx tmp = in[i].second.get_mid(6);
			tmp = tmp.get_left(tmp.size()-2);
			pII->AddSubItemText (tmp.c_str());
		}
		else
			pII->AddSubItemText (in[i].second.c_str());
		pII->SetControlType (CItemInfo::combobox, 1);

		// Insert
		pII->SetImage (2);
		InsertRootItem (pII);
	}

	SetRedraw (TRUE);
	InvalidateRect (NULL);
	UpdateWindow ();
}
