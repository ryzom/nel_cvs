// MySuperGrid.cpp : implementation file
//

#include "stdafx.h"
#include "Georges.h"
#include "GeorgesDoc.h"
#include "MySuperGrid.h"
#include "ComboInListView.h"
#include "../georges_lib/Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySuperGrid



CMySuperGrid::CMySuperGrid()
{
	m_bDrag = TRUE;
	pitemroot = 0;
	pdoc = 0;
	currenttreeitem = 0;
}

CMySuperGrid::~CMySuperGrid()
{
}


BEGIN_MESSAGE_MAP(CMySuperGrid, CSuperGridCtrl)
	//{{AFX_MSG_MAP(CMySuperGrid)
	ON_WM_CREATE()
	ON_COMMAND(ID_LIST_NEWITEM, OnListNewitem)
	ON_COMMAND(ID_LISTCHILD_DELITEM, OnListchildDelitem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySuperGrid message handlers

int CMySuperGrid::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSuperGridCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	/////////////////////
	//remember this	
	///////////////////

	//associate imagelist with listviewctrl, 
	//this imagelist is only for the first column re: the tree column	
	if(!m_image.Create(IDB_FOLDERS,16,1,RGB(0, 255, 255)))
		return -1;

	SetImageList(&m_image, LVSIL_SMALL);
	CImageList *pImageList = GetImageList(LVSIL_SMALL);
	if(pImageList)
		ImageList_GetIconSize(pImageList->m_hImageList, &m_cxImage, &m_cyImage);
	else
		return -1;

	//If you want to assiciate images to subitems, create an imagelist
	//and call SetSubItemImageList
	//now the next time you want to map an image from this imagelist to a
	//particular subitem call SetSubItemText(col,text,image-index);


//UP juste enlever si pas besoin d'items, mais corriger la largeur d'édition
	CImageList imageSubItem;
	if(!imageSubItem.Create(IDB_SUBITEMS,16,1,RGB(0, 255, 255)))
		return -1;
    if(!SetSubItemImageList(&imageSubItem))
		return -1;
//endUP juste enlever si pas besoin d'items, mais corriger la largeur d'édition

	return 0;
}

void CMySuperGrid::InitializeGrid( CGeorgesDoc* const _pdoc )
{
	SetExtendedStyle(LVS_EX_GRIDLINES);
	if( !pdoc )
		pdoc = _pdoc;
	
	//initialize the columns
	LPTSTR lpszCols[] = {_T("Name"),_T("Result"),_T("Value"),_T("Type"),0};
	LV_COLUMN   lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 350;
	for(int x = 0; lpszCols[x]!=NULL; x++)
    {
		if(x)
		  lvColumn.cx = 200;
		lvColumn.pszText = lpszCols[x];
		InsertColumn(x,&lvColumn);
    }
}

 
void CMySuperGrid::InitializeSubItemNumber( CTreeItem* _itemparent, unsigned int& _index )
{
/*
	CItemInfo* lpiteminfo = GetData( _itemparent ); 
	if( !lpiteminfo )
		return;
	lpiteminfo->SetItemIndex( _index+1 ); 


	CTreeItem* pParent = GetItem( _itemparent, nbre? ); ou GetNextItem

	int n = pdoc->GetItemNbElt( _index );
	_index++;
	int nn = n;
	while( nn > 1 )
		nn -= InitializeSubItemNumber( pParent, _index );
	return( n );
*/
}

void CMySuperGrid::InitializeItemNumber()
{
/*
	int nb_elt = pdoc->GetItemNbElt();
	SetItemCount( nb_elt );
	unsigned int index = 0;
	CItemInfo* lpiteminfo = GetData( pitemroot );
	if( !lpiteminfo )
		return;
	lpiteminfo->SetItemIndex( 0 ); 
	while( nb_elt > 0 )
		nb_elt -= LoadSubItem( pitemroot, index );
*/
}
/*
void CSuperGridCtrl::ExpandAll(CTreeItem *pItem, int& nScroll)
{
	const int nChildren = pItem->m_listChild.GetCount();
	if (nChildren > 0)
	{
		int nIndex = NodeToIndex(pItem);
		nScroll = Expand(pItem, nIndex);
	}

	POSITION pos = pItem->m_listChild.GetHeadPosition();
	while (pos)
	{
		CTreeItem *pChild = (CTreeItem*)pItem->m_listChild.GetNext(pos);
		ExpandAll(pChild, nScroll);
	}
	
}



int CSuperGridCtrl::Expand(CTreeItem* pSelItem, int nIndex)
{
	if(ItemHasChildren(pSelItem) && IsCollapsed(pSelItem))
	{

		LV_ITEM lvItem;
		lvItem.mask = LVIF_INDENT;
		lvItem.iItem = nIndex;
		lvItem.iSubItem = 0;
		lvItem.lParam=(LPARAM)pSelItem;
		lvItem.iIndent = GetIndent(pSelItem);
		SetItem(&lvItem);
		
		Hide(pSelItem, FALSE);
		//expand children
		POSITION pos = pSelItem->m_listChild.GetHeadPosition();
		while(pos != NULL)
		{
			CTreeItem* pNextNode = (CTreeItem*)pSelItem->m_listChild.GetNext(pos);
			CString str = GetData(pNextNode)->GetItemText();
			LV_ITEM lvItem;
			lvItem.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM;
			lvItem.pszText =str.GetBuffer(1); 
			lvItem.iItem = nIndex + 1;
			lvItem.iSubItem = 0;
			lvItem.lParam=(LPARAM)pNextNode;
			lvItem.iIndent = GetIndent(pSelItem)+1;
			CListCtrl::InsertItem(&lvItem);
			if(GetData(pNextNode)->GetCheck())
				SetCheck(nIndex + 1);
			//get subitems
			int nSize = GetData(pNextNode)->GetItemCount();
			for(int i=0; i< nSize;i++)
			{
			   CString str=GetData(pNextNode)->GetSubItem(i);
			   lvItem.mask = LVIF_TEXT;
			   lvItem.iSubItem = i+1;
			   lvItem.pszText=str.GetBuffer(1);
			   SetItem(&lvItem);
			}
			nIndex++;
		}
	}
	InternaleUpdateTree();
	return nIndex;
}

  */

void CMySuperGrid::LoadItem()
{
	DeleteAll();
	CItemInfo* lp = new CItemInfo( 0 );
	lp->SetItemText( CString( "ROOT" ) );
	lp->AddSubItemText( CString( "" ) );
	lp->AddSubItemText( CString( "" ) );
	lp->AddSubItemText( CString( "" ) );
	lp->SetImage(4);
	pitemroot = InsertRootItem(lp);

	int nb_elt = pdoc->GetItemNbElt();
	SetItemCount( nb_elt );

	unsigned int index = 0;
	while( nb_elt > 0 )
		nb_elt -= LoadSubItem( pitemroot, index );

	CWaitCursor wait;
	int nScrollIndex = 0;
	if(OnVKAdd(pitemroot, 0))
		 nScrollIndex = Expand(pitemroot, 0);
	InvalidateItemRect(0);
	EnsureVisible(nScrollIndex, 1);

//	ExpandAllItems();

	SetRedraw(1);
	InvalidateRect(NULL);
}


//helper function to copy CItemInfo used when drag/drop you must override this this function to suit your own CItemInfo class
CItemInfo* CMySuperGrid::CopyData(CItemInfo* lpSrc)
{
	ASSERT(lpSrc!=NULL);
	CItemInfo* lpDest = new CItemInfo;
	//well okay I put all the copy thing in one function, located in CItemInfo class, 
	//so you should check out this function, remember to modify this function each time you add new data to CItemInfo class.
	lpDest->CopyObjects(lpSrc);
	return lpDest;
}


//override, like red!
COLORREF CMySuperGrid::GetCellRGB()
{
	return RGB(128,128,200);
}


//this is my override of GetIcon, override this to set what ever icon suits you
int CMySuperGrid::GetIcon(const CTreeItem* pItem)
{
	if(pItem!=NULL)
	{
		int n = GetData(pItem)->GetImage();
		if(n!=-1)
			return n;
		
		int iImage = 0;
		if(ItemHasChildren(pItem))
		{
			IsCollapsed(pItem) ? iImage = 1/*close icon*/:iImage = 0;/*open icon*/
		}
		else
			iImage = 2;//doc icon
		return iImage;
	}
	return 0;
}


//override
void CMySuperGrid::OnUpdateListViewItem(CTreeItem* lpItem, LV_ITEM *plvItem)
{
	CString str = (CString)plvItem->pszText;
	CItemInfo *lp = GetData(lpItem);
	if(lp!=NULL)
	{
		unsigned int itemindex = lp->GetItemIndex();
		if( plvItem->iSubItem == 2 )
		{
			lp->SetSubItemText(plvItem->iSubItem-1, str);
			pdoc->SetItemValue( itemindex-1, str );
			lp->SetSubItemText( plvItem->iSubItem-2, pdoc->GetItemCurrentResult( itemindex-1 ) );
		}
		UpdateData(lpItem, lp); 
		SetItemText(plvItem->iItem, plvItem->iSubItem, str.GetBuffer(1));
	}
}



			
BOOL CMySuperGrid::OnItemLButtonDown(LVHITTESTINFO& ht)
{
	if(ht.iItem!=-1)
	{
/*
#ifdef _WHAT_EVER_
		CTreeItem*pSelItem = GetTreeItem(ht.iItem);
		if(pSelItem!=NULL)
		{	
			CItemInfo* pInfo = GetData(pSelItem);
			if((ht.iSubItem==0) && (pInfo->GetItemText()==CString(_T("Hello World"))))
			{
				AfxMessageBox("no can do buddy");
				return 0;
			}

			for(int i=0;i<pInfo->GetItemCount(); i++)
			{
				if(i==ht.iSubItem-1)
				{
					CString str=pInfo->GetSubItem(i);
					if(str == CString(_T("Ethan Hunt"))) 
					{
						AfxMessageBox("Tom Cruise\nliked him in a few good men");
						return 0;
					}
				}
			}
		}
#endif
*/
	}
	return 1;
}

//override called when OnLButtondown
void CMySuperGrid::OnControlLButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht)
{
	//now I am sure I added a combobox some where, so check for this control
	CTreeItem*pSelItem = GetTreeItem(ht.iItem);
	if(pSelItem!=NULL)
	{	
		CItemInfo* pInfo = GetData(pSelItem);
		CItemInfo::CONTROLTYPE ctrlType;
		if(pInfo->GetControlType(ht.iSubItem-1, ctrlType))
		{	
			if(ctrlType==pInfo->CONTROLTYPE::combobox) 
			{
//					CStringList* list=NULL;
//					pInfo->GetListData(ht.iSubItem-1, list);
//					CComboBox * pList = ShowList(ht.iItem, ht.iSubItem, list);

					CStringList lst;
					int ind = pInfo->GetItemIndex();
					pdoc->GetItemListPredef( pInfo->GetItemIndex()-1, &lst );
					pInfo->SetListData(1, &lst);
					CStringList* list=NULL;
					pInfo->GetListData(ht.iSubItem-1, list);
					CComboBox * pList = ShowList(ht.iItem, ht.iSubItem, list);
					
			}
		}								
		else //activate default edit control
			CSuperGridCtrl::OnControlLButtonDown(nFlags, point, ht);
	}
}

BOOL CMySuperGrid::OnItemRButtonDown(LVHITTESTINFO& ht)
{
//	if( ht.iItem == -1 )
//		return 0;
	return 1; 
}

void CMySuperGrid::OnControlRButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht)
{
	currenttreeitem = GetTreeItem(ht.iItem);
	if( !currenttreeitem )
		return;	
	CItemInfo *lp = GetData( currenttreeitem );
	if( !lp )
		return;
	unsigned int infos = pdoc->GetItemInfos( lp->GetItemIndex()-1 );
	if( infos & ITEM_ISLISTCHILD )
	{
		CMenu menu;
		menu.LoadMenu( IDR_M_LISTCHILD );
		CMenu* pPopup = menu.GetSubMenu( 0 );
		CPoint pt = point; 
		ClientToScreen( &pt ) ; 
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	if( infos & ITEM_ISLIST )
	{
		CMenu menu;
		menu.LoadMenu( IDR_M_LIST );
		CMenu* pPopup = menu.GetSubMenu( 0 );
		CPoint pt = point ; 
		ClientToScreen( &pt ) ; 
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
}


void CMySuperGrid::ExpandAllItems() 
{
	CWaitCursor wait; 
	SetRedraw(0);
	CTreeItem *pParent = GetTreeItem(0);
	int nScroll=0;
	if(OnVKMultiply(pParent, 0))
	{	
		ExpandAll(pParent, nScroll);
	}		
	SetRedraw(1);
	RedrawItems(0, nScroll);
	EnsureVisible(nScroll, TRUE);
}



void CMySuperGrid::OnListNewitem() 
{
	if( !currenttreeitem  )
		return;	
	CItemInfo *lp = GetData( currenttreeitem );
	if( lp == NULL )
		return;

	unsigned int currentlistindex = lp->GetItemIndex();
	if( currentlistindex < 0 )
		return;
	
	unsigned int curlistoldnbelt = pdoc->GetItemNbElt( currentlistindex-1 );
	pdoc->AddListParent( currentlistindex );
	unsigned int curlistnewnbelt = pdoc->GetItemNbElt( currentlistindex-1 );

	SetRedraw(0);
	BOOL bUpdate = FALSE;
	if( !IsCollapsed( currenttreeitem ) )
		bUpdate = TRUE;

	unsigned int _index = currentlistindex+curlistoldnbelt-1;

	CItemInfo* lpiteminfo = new CItemInfo( _index+1 );
	lpiteminfo->SetItemText( pdoc->GetItemName( _index ) );
	lpiteminfo->AddSubItemText( pdoc->GetItemCurrentResult( _index ) );
	lpiteminfo->AddSubItemText( pdoc->GetItemCurrentValue( _index ) );
	lpiteminfo->AddSubItemText( pdoc->GetItemFormula( _index ) );

//	if( pdoc->IsItemEnum( _index ) )
	if( pdoc->IsItemPredef( _index ) )
	{
		lpiteminfo->SetControlType(lpiteminfo->CONTROLTYPE::combobox, 1);
		CStringList lst;
		pdoc->GetItemListPredef( _index, &lst );
		lpiteminfo->SetListData(1, &lst);
	}

	CTreeItem* pParent = InsertItem( currenttreeitem, lpiteminfo, bUpdate );
	
	int n = pdoc->GetItemNbElt( _index );
	_index++;
	int nn = n;
	while( nn > 1 )
		nn -= LoadSubItem( pParent, _index );
	
	UpdateItemIndex();

	SetRedraw(1);
	InvalidateRect(NULL);
	UpdateWindow();
}


void CMySuperGrid::UpdateItemIndex()
{
	POSITION pos = GetRootHeadPosition();
	unsigned int k = 0;
	while(pos != NULL)
	{
		CTreeItem *pParent =(CTreeItem*)GetNextRoot(pos); 
		CTreeItem *pItem = pParent;
		CItemInfo* lp = GetData(pParent);
		lp->SetItemIndex( k+1 );
		for(;;k++)
		{
			CTreeItem *pCur = GetNext(pParent,pItem, TRUE, FALSE);	  
			
			if(!IsChildOf(pParent, pCur))
				break;
			else
			if(pCur==pItem)
				break;
			
			CItemInfo* lp = GetData(pCur);
			lp->SetItemIndex( k+1 );
			lp->SetItemText( pdoc->GetItemName( k ) );
			pItem=pCur;
		}
	}
}

unsigned int CMySuperGrid::LoadSubItem( CTreeItem* _itemparent, unsigned int& _index )
{
	CItemInfo* lpiteminfo = new CItemInfo( _index+1 );
	lpiteminfo->SetItemText( pdoc->GetItemName( _index ) );
	lpiteminfo->AddSubItemText( pdoc->GetItemCurrentResult( _index ) );
	lpiteminfo->AddSubItemText( pdoc->GetItemCurrentValue( _index ) );
	lpiteminfo->AddSubItemText( pdoc->GetItemFormula( _index ) );

//	if( pdoc->IsItemEnum( _index ) )
	if( pdoc->IsItemPredef( _index ) )
	{
		lpiteminfo->SetControlType(lpiteminfo->CONTROLTYPE::combobox, 1);
		CStringList lst;
		pdoc->GetItemListPredef( _index, &lst );
		lpiteminfo->SetListData(1, &lst);
	}
	CTreeItem* pParent = InsertItem( _itemparent, lpiteminfo );
	
	int n = pdoc->GetItemNbElt( _index );
	_index++;
	int nn = n;
	while( nn > 1 )
		nn -= LoadSubItem( pParent, _index );
	return( n );
}

void CMySuperGrid::HowToInsertItemsAfterTheGridHasBeenInitialized(int nIndex, int indexxx )
{
}


void CMySuperGrid::OnListchildDelitem() 
{
	if( !currenttreeitem  )
		return;	
	CItemInfo *lp = GetData( currenttreeitem );
	if( lp == NULL )
		return;
	unsigned int currentitem = lp->GetItemIndex();
	if( currentitem < 0 )
		return;
	pdoc->DelListChild( currentitem );

//	LoadItem();
//	ExpandAllItems();

	unsigned int ui = GetCurIndex( currenttreeitem );
	DeleteItemEx(currenttreeitem, ui);
	UpdateItemIndex();

	InvalidateRect(NULL);
	UpdateWindow();
}

BOOL CMySuperGrid::CanEdit( CTreeItem* const _pItem )
{
	CItemInfo *lp = GetData( _pItem );
	if( lp == NULL )
		return( false );
	unsigned int index = lp->GetItemIndex();
	return( pdoc->CanEditItem( index-1 ) );
}

BOOL CMySuperGrid::OnVkReturn()
{
	BOOL bResult=FALSE;
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
					case pInfo->CONTROLTYPE::datecontrol:break;
					case pInfo->CONTROLTYPE::spinbutton:break;
					case pInfo->CONTROLTYPE::dropdownlistviewwhatevercontrol:break;
					case pInfo->CONTROLTYPE::combobox: 
						{
//							CStringList* list=NULL;
//							pInfo->GetListData(iSubItem-1, list);
//							CComboBox * pList = ShowList(iItem, iSubItem, list);
							CStringList lst;
							pdoc->GetItemListPredef( iItem-1, &lst );
							pInfo->SetListData(1, &lst);
							CStringList* list=NULL;
							pInfo->GetListData(iSubItem-1, list);
							CComboBox * pList = ShowList(iItem, iSubItem, list);

							bResult=TRUE; //I'll handle it from here
						}break;
					default:break;
				}
			}
		}
	}
	return( bResult );
}





#define IDC_COMBOBOXINLISTVIEW 0x1235
CComboBox* CMySuperGrid::ShowList(int nItem, int nCol, CStringList *lstItems)
{
	CString strFind = GetItemText(nItem, nCol);

	//basic code start
	CRect rect;
	int offset = 0;
	// Make sure that the item is visible
	if( !EnsureVisible(nItem, TRUE)) return NULL;
	GetSubItemRect(nItem, nCol, LVIR_BOUNDS, rect);
	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(rcClient);
	if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}
	
	rect.left += offset;	
	rect.right = rect.left + GetColumnWidth(nCol);
	if(rect.right > rcClient.right) 
	   rect.right = rcClient.right;
	//basic code end

	rect.bottom += 10 * rect.Height();//dropdown area
	
	DWORD dwStyle =  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | CBS_SORT | CBS_DISABLENOSCROLL;
	 
	CComboBox *pList = new CComboInListView(nItem, nCol, lstItems);
	pList->Create(dwStyle, rect, this, IDC_COMBOBOXINLISTVIEW);
	pList->ModifyStyleEx(0,WS_EX_CLIENTEDGE);//can we tell at all
	pList->SetHorizontalExtent(CalcHorzExtent(pList, lstItems));
	pList->ShowDropDown();
	pList->SelectString(-1, strFind.GetBuffer(1));
	// The returned pointer should not be saved
	return pList;
}



int CMySuperGrid::CalcHorzExtent(CWnd* pWnd, CStringList *pList)
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
				newExtent = pDC->GetTextExtent(str);
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


void CMySuperGrid::HowToLoopThroughAllItems_if_we_wanted_to_print_them_or_what_ever(CDC *pDC)
{
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int cy = tm.tmHeight + tm.tmExternalLeading;
	int nLineY=0;
	nLineY+=cy;

	pDC->TextOut(10, nLineY, _T("GIVE ME A BREAK YOU KNOW PRINT PREVIEW IS NOT THE ISSUE HERE"));
	nLineY+=cy;
	nLineY+=cy;
	int nIndex = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED); //print from current node
	if(nIndex!=-1)
	{
		//GetHeadPosition
		POSITION pos = GetRootHeadPosition();
		while(pos != NULL)
		{
			CTreeItem *pParent =(CTreeItem*)GetNextRoot(pos); 
			CTreeItem *pItem = pParent;
			CItemInfo* lp = GetData(pParent);

			CString strData = lp->GetItemText();
			strData+='\t';
			nLineY+=cy;
			for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
			{
				CString str = lp->GetSubItem(nCol);
				strData+=str;
				strData+='\t';
			}	
			pDC->TabbedTextOut(10,nLineY,strData,strData.GetLength(), 0, NULL, 0);
			nLineY+=cy;
			//GetNext ....loop through all children 
			for(;;)
			{
				CTreeItem *pCur = GetNext(pParent,pItem, TRUE, FALSE/*regardless of the item are hidden or not*/);	  

				if(!IsChildOf(pParent, pCur))
					break;
				else
				if(pCur==pItem)
					break;
				
				CItemInfo* lp = GetData(pCur);
				CString strData = lp->GetItemText();
				strData+='\t';
				for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
				{
					CString str = lp->GetSubItem(nCol);
					strData+=str;
					strData+='\t';
				}	
				pDC->TabbedTextOut(10,nLineY,strData,strData.GetLength(), 0, NULL, 0);
				nLineY+=cy;
				pItem=pCur;
			}
		}
	 }
}



void CMySuperGrid::HowToLoopThroughAllItems_that_has_a_checkmark_and_print_them_or_what_ever(CDC *pDC)
{
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int cy = tm.tmHeight + tm.tmExternalLeading;
	int nLineY=0;
	nLineY+=cy;

	pDC->TextOut(10,nLineY,_T("GIVE ME A BREAK YOU KNOW PRINT PREVIEW IS NOT THE ISSUE HERE"));
	nLineY+=cy;
	nLineY+=cy;
	if(!GetItemCount())
		return;

	int nIndex=0;//has to be the root 
	if(nIndex!=-1)
	{
		//do a GetHeadPosition
		POSITION pos = GetRootHeadPosition();
		while(pos != NULL)
		{
			CTreeItem *pParent = (CTreeItem*)GetNextRoot(pos); 
			CTreeItem *pItem = pParent;
			CItemInfo* lp = GetData(pParent);
			if(lp->GetCheck())//very hard :)=
			{
				CString strData=lp->GetItemText();
				strData+='\t';
				nLineY+=cy;
				for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
	
				{
					CString str = lp->GetSubItem(nCol);
					strData+=str;
					strData+='\t';
				}	
				pDC->TabbedTextOut(10,nLineY,strData,strData.GetLength(), 0, NULL, 0);
				nLineY+=cy;
			}
			//GetNext ....loop through all children 
			for(;;)
			{
				CTreeItem *pCur = GetNext(pParent, pItem, TRUE, FALSE/*regardless of the item are hidden or not*/);	  
				if(!IsChildOf(pParent, pCur))
					break;
				else
				if(pCur==pItem)
					break;
				

				CItemInfo* lp = GetData(pCur);
				if(lp->GetCheck())
				{
					CString strData = lp->GetItemText();
					strData+='\t';
					for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
					{
						CString str = lp->GetSubItem(nCol);
						strData+=str;
						strData+='\t';
					}	
					pDC->TabbedTextOut(10,nLineY,strData,strData.GetLength(), 0, NULL, 0);
					nLineY+=cy;
				}
				pItem=pCur;
			}
		}//while
	 }//if
}


//HOWTO: Search nodeptr that have a specific item and subitems also shows you how to select the node and delete it
void CMySuperGrid::HowToSearch_I_am_using_hardcoded_values_here_cause_I_am_tired_now(void)
{

	//one Item and two Subitems
	CTreeItem *pNode =	Search(__T("Hello World"),_T("Happy"),_T("Programming"),NULL);
	
	if(pNode!=NULL)
	{
		CItemInfo *pInfo = GetData(pNode);
		AfxMessageBox(_T("Found Item ") + pInfo->GetItemText());
	}
	else AfxMessageBox(_T("not found"));
	//one Item and one Subitem
	CTreeItem *pNode1 = Search(_T("Mission: Impossible"),_T("Allan Nielsen"),NULL);
	if(pNode1!=NULL)
	{
		CItemInfo *pInfo = GetData(pNode1);
		AfxMessageBox(_T("Found Item ") + pInfo->GetItemText());

	}
	else AfxMessageBox(_T("not found"));
	
	//one Item and one Subitem
	CTreeItem *pNode2 = Search(_T("Training Agent"),_T("Mr. Bean"),NULL);
	if(pNode2!=NULL)
	{
		CItemInfo *pInfo = GetData(pNode2);
		AfxMessageBox(_T("Found Item") + pInfo->GetItemText());
	}
	else AfxMessageBox(_T("not found"));

	CTreeItem *pNode3 = Search(_T("BC"),NULL);
	if(pNode3!=NULL)
	{
		CItemInfo *pInfo = GetData(pNode3);
		AfxMessageBox(_T("Found Item ") + pInfo->GetItemText());
		int nIndex = SelectNode(pNode3);
		if(nIndex!=-1)
		if(AfxMessageBox(_T("Do you want to delete it"),MB_OKCANCEL)==IDOK)
			DeleteItemEx(pNode3, nIndex);
	}
	else AfxMessageBox(_T("not found"));

	CTreeItem *pItem = NULL;

	POSITION pos = GetRootHeadPosition();
	while(pos != NULL)
	{
		CTreeItem * pRoot = (CTreeItem*)GetNextRoot(pos); 
		pItem = SearchEx(pRoot, "AB");
		if(pItem!=NULL)
		{
			CItemInfo *pInfo = GetData(pItem);
			AfxMessageBox(_T("Found Item ") + pInfo->GetItemText());
			int nIndex = SelectNode(pItem);
			if(nIndex!=-1)
			if(AfxMessageBox(_T("Do you want to delete it"),MB_OKCANCEL)==IDOK)
				DeleteItemEx(pItem, nIndex);
		}
	}
		
	if( pItem == NULL )
		AfxMessageBox(_T("not found"));


}



//this is just one way to search items...strItem must match and then all subitems must be
//a match before returning the node
//the search function here search all nodes regardless if collapsed or expanded
CMySuperGrid::CTreeItem* CMySuperGrid::Search(CString strItem,...)
{
	if(!GetItemCount())
		return NULL;

	va_list args;
	va_start(args, strItem);
	int nCount=0;
	
	for(;;)
	{
		LPCTSTR lpsz = va_arg(args, LPCTSTR);
		if(lpsz==NULL)
			break;
	   nCount++;
	}


	POSITION pos = GetRootHeadPosition();
	while(pos != NULL)
	{
		CTreeItem *pParent = (CTreeItem*)GetNextRoot(pos); 
		CTreeItem *pItem = pParent;
		CItemInfo* lp = GetData(pParent);
		CString strData = lp->GetItemText();
		if(strData==strItem)//must be a match before going further...suit you self
		{
			va_start(args, strItem);
			int nResult=0;
			for(int i=0; i<nCount;i++)
			{
				LPCTSTR lpsz = va_arg(args, LPCTSTR);   
				for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
				{
					CString str = lp->GetSubItem(nCol);
					if(!str.CompareNoCase(lpsz))
					{
						nResult++;
						break;
					}
				}	
			}
			va_end(args);
			if(nCount==nResult)//all args was found return node
				return pParent;
		}

		//GetNext ....loop through all children 
		for(;;)
		{
			CTreeItem *pCur = GetNext(pParent, pItem, TRUE, FALSE/*regardless of the item are hidden or not, set to TRUE if only visible items must be included in the search*/);	  
			if(!IsChildOf(pParent, pCur))
				break;
			else
			if(pCur==pItem)
				break;
			CItemInfo* lp = GetData(pCur);
			CString strData = lp->GetItemText();
			if(strData==strItem)//must be a match before going further
			{
				va_start(args, strItem);
				int nResult=0;
				for(int i=0; i<nCount;i++)
				{
					LPCTSTR lpsz = va_arg(args, LPCTSTR);   
					for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
					{
						CString str = lp->GetSubItem(nCol);
						if(!str.CompareNoCase(lpsz))
						{
							nResult++;
							break;
						}
					}	
				}
				va_end(args);
				if(nCount==nResult)//all args was found return node
					return pCur;
			}
			pItem=pCur;//next;
		}
	}	
	return NULL;
}



void CMySuperGrid::SortData()
{
	int nIndex = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED); 
	if(nIndex==-1)
		return;

	if(AfxMessageBox("Sort roots and all children(Yes)\nor just sort the rootitems(No)",MB_YESNO)==IDYES)
		SortEx(1);			
	else
		SortEx(0);
}



//another search thing
CMySuperGrid::CTreeItem* CMySuperGrid::SearchEx(CTreeItem *pStartPosition, CString strItem)
{
	CItemInfo* lp = GetData(pStartPosition);
	//if(lp->GetCheck()) another condition here maybe
	CString strData = lp->GetItemText();
	if(strData==strItem)
	{
		return pStartPosition;
	}

	const int nChildren = NumChildren(pStartPosition);
	if (nChildren > 0)
	{
		POSITION pos = GetHeadPosition(pStartPosition);
		while (pos)
		{
			CTreeItem *pChild = GetNextChild(pStartPosition, pos);
			CItemInfo* lp = GetData(pChild);
			CString strData = lp->GetItemText();
			if(strData==strItem)
			{
				return pChild;
			}
		}
	}

	POSITION pos = GetHeadPosition(pStartPosition);
	while (pos)
	{
		CTreeItem *pChild = GetNextChild(pStartPosition, pos);
		CItemInfo* lp = GetData(pChild);
		CString strData = lp->GetItemText();
		if(strData==strItem)
		{
			return pChild;
		}

		pChild = SearchEx(pChild, strItem);
		if(pChild!=NULL)
			return pChild;
	}
	return NULL;
}





BOOL CMySuperGrid::OnDeleteItem(CTreeItem* pItem, int nIndex)
{
	return 1;
}



BOOL CMySuperGrid::OnItemExpanding(CTreeItem *pItem, int iItem)
{
	return 1;
}


BOOL CMySuperGrid::OnItemExpanded(CTreeItem* pItem, int iItem)
{
	return 1;
}


BOOL CMySuperGrid::OnCollapsing(CTreeItem *pItem)
{
	return 1;
}



BOOL CMySuperGrid::OnItemCollapsed(CTreeItem *pItem)
{
	return 1;
}


CImageList *CMySuperGrid::CreateDragImageEx(int nItem)
{
		if(m_bDrag)
			return CSuperGridCtrl::CreateDragImageEx(GetDragItem());
		else
			return NULL;
}



void CMySuperGrid::_DeleteAll()
{
	DeleteAll();//call CSuperGridCtrl::DeleteAll();
	//add some new data
	CItemInfo* lp = new CItemInfo();
	lp->SetImage(4);
	//add item text
	lp->SetItemText(_T("New data"));
	//Create root item
	CTreeItem * pRoot = InsertRootItem(lp);//previous we call CreateTreeCtrl(lp)
	if( pRoot == NULL )
		return;
	//insert items	
	int nCol = GetNumCol();
	for(int i=0; i < nCol; i++)
	{
		CItemInfo* lpItemInfo = new CItemInfo();
		CString strItem;
		strItem.Format(_T("Item %d"),i);
		//add items text
		lpItemInfo->SetItemText(strItem);
		//add subitem text
		for(int y=0;y < nCol-1; y++) 
		{
			CString str;
			str.Format(_T("subItem %d of %s"),y,lpItemInfo->GetItemText());
			lpItemInfo->AddSubItemText(str);
			lpItemInfo->AddSubItemText(str);
		}
		//insert the iteminfo with ParentPtr
		CTreeItem* pParent = InsertItem(pRoot, lpItemInfo);
		//other nodes
		if(i%nCol)
		{
			CTreeItem* pParent1=NULL;
			CTreeItem* pParent2=NULL;
			for(int x=0; x < nCol; x++)
			{
				CItemInfo* lpItemInfo = new CItemInfo();
				CString strItem;
				strItem.Format(_T("Item %d"),x);
				lpItemInfo->SetItemText(strItem);
				for(int z=0; z < nCol-1; z++) 
				{
					CString str;
					str.Format(_T("subItem %d of %s"),z, lpItemInfo->GetItemText());
					lpItemInfo->AddSubItemText(str);
				}
				pParent1 = InsertItem(pParent, lpItemInfo);
				
			}
		}
	}
	//expand one level
	Expand(pRoot, 0 /*listview index 0*/); 
	UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
	SetItemState(0, uflag, uflag);


}


void CMySuperGrid::DynamicUpdateSomeItems(int nItem)
{
	CTreeItem*pItem = GetTreeItem(nItem);
	CItemInfo *lp = GetData(pItem);
	if(lp!=NULL)
	{
		lp->SetItemText("Dynamic");
		//subitems
		lp->SetSubItemText(0,"updates");
		lp->SetSubItemText(1, "are");
		lp->SetSubItemText(2, "not always",2/*image index 2*/);
		lp->SetSubItemText(3, "fun");
		//update internal nodes
		UpdateData(pItem, lp, TRUE); 
		//set bUpdateRow to TRUE here, this would update the listview row e.g write the content of CItemInfo
	}
}

void CMySuperGrid::SetNewImage(int nItem)
{
	CTreeItem*pItem = GetTreeItem(nItem);
	CItemInfo *lp = GetData(pItem);
	if(lp!=NULL)
	{
		lp->SetSubItemImage(3/*subitem*/,4/*image index*/);
		lp->SetSubItemImage(2/*subitem*/,4/*image index*/);
		//update internal node
		UpdateData(pItem, lp, TRUE); 
		InvalidateItemRect(nItem);
	}
}









/*


	CItemInfo* lp = new CItemInfo();
	lp->SetImage(4);
	//add item text
	lp->SetItemText(_T("Hello World"));
	//add subitem text
	lp->AddSubItemText(_T("Happy"));      // 0 zero based subitems...
	lp->AddSubItemText(_T("Programming"));// 1
	lp->AddSubItemText(_T("with this"));  // 2
	lp->AddSubItemText(_T("SuperGrid"));  // 3 combo goes here
	//setup a combobox for the last column
	lp->SetControlType(lp->CONTROLTYPE::combobox, 3
	//col # 3 from subitems...really col #4 in view, live with it
	);
	CStringList list;
	//initalized list for the combobox
	list.AddTail("Super control :}");
	list.AddTail("bad written control :{");
	list.AddTail("said piece of code :{");
	lp->SetListData(3, &list);


	//Create root item
	CTreeItem * pRoot = InsertRootItem(lp);//previous on N.Y.P.D we call it CreateTreeCtrl(lp)
	if( pRoot == NULL )
		return;
	//insert items	
	int nCol = GetNumCol();
	for(int i=0; i < nCol; i++)
	{
		CItemInfo* lpItemInfo = new CItemInfo();
		CString strItem;
		strItem.Format(_T("Item %d"),i);
		//add items text
		lpItemInfo->SetItemText(strItem);
		
		//add subitem text
		for(int y=0;y < nCol-1; y++) 
		{
			CString str;
			str.Format(_T("subItem %d of %s"),y,lpItemInfo->GetItemText());
			//lpItemInfo->AddSubItemText(str);
			lpItemInfo->SetSubItemText(y, str,y);
		}
		//insert the iteminfo with ParentPtr
		CTreeItem* pParent = InsertItem(pRoot, lpItemInfo);

	
	  
		//other nodes
		if(i%nCol)
		{
			CTreeItem* pParent1=NULL;
			CTreeItem* pParent2=NULL;
			for(int x=0; x < nCol; x++)
			{
				CItemInfo* lpItemInfo = new CItemInfo();
				CString strItem;
				strItem.Format(_T("Item %d"),x);
				lpItemInfo->SetItemText(strItem);
				for(int z=0; z < nCol-1; z++) 
				{
					CString str;
					str.Format(_T("subItem %d of %s"),z, lpItemInfo->GetItemText());
					lpItemInfo->AddSubItemText(str);
				}
				pParent1 = InsertItem(pParent, lpItemInfo);
				
			}
			
			//this was pretty hard to get information !
			CItemInfo* lpAgent = new CItemInfo();
			lpAgent->SetItemText(_T("Mission: Impossible"));
			lpAgent->AddSubItemText(_T("Ethan Hunt"));
			lpAgent->AddSubItemText(_T("Jack Harmen"));
			lpAgent->AddSubItemText(_T("Jim Phelps"));
			lpAgent->AddSubItemText(_T("Allan Nielsen"));
			pParent2 = InsertItem(pParent1, lpAgent);
		 
			CItemInfo* lpRelative = new CItemInfo();
			lpRelative->SetItemText("Nearst relative");
			lpRelative->AddSubItemText(_T("Margeret Ethan Hunt"));
			lpRelative->AddSubItemText(_T("Alexandra Soria"));
			lpRelative->AddSubItemText(_T("Claire Phelps"));
			lpRelative->AddSubItemText(_T("N/A"));
			InsertItem(pParent2, lpRelative);

			CItemInfo* lpTraningAgent = new CItemInfo();
			lpTraningAgent->SetItemText(_T("Training Agent"));
			lpTraningAgent->AddSubItemText(_T("Peter Menich"));
			lpTraningAgent->AddSubItemText(_T("Chris Smith"));
			lpTraningAgent->AddSubItemText(_T("Eva Wojdat"));
			lpTraningAgent->AddSubItemText(_T("Mr. Bean"));
			InsertItem(pParent2, lpTraningAgent);
	
		}
	}

	//Epilog
	CItemInfo* lpAgent = new CItemInfo();
	lpAgent->SetCheck(1);//I want this to be checked if LVS_EX_CHECKBOXES
	lpAgent->SetItemText(_T("Bugs: Impossible"));
	lpAgent->SetImage(3);
	CTreeItem *pParent2 = InsertItem(pRoot,lpAgent);

	CItemInfo* lpRelative = new CItemInfo();
	lpRelative->SetItemText("But if.....no When");
	lpRelative->AddSubItemText(_T("You"));//0
	lpRelative->AddSubItemText(_T("find one"));//1
	lpRelative->AddSubItemText(_T("please notify"));//2
	lpRelative->AddSubItemText(_T("me"));//3 combo goes here
	lpRelative->SetCheck(1); //I want this to be checked if LVS_EX_CHECKBOXES
	//add combobox in col 2(3)
	lpRelative->SetControlType(lpRelative->CONTROLTYPE::combobox, 2);
	CStringList lst;
	lst.AddTail("please call");
	lst.AddTail("shout at");
	lst.AddTail("phone");
	lpRelative->SetListData(2
	//col
	, &lst);

	//add another combobox in this row in col 3(4)
	lpRelative->SetControlType(lpRelative->CONTROLTYPE::combobox, 3);
	CStringList list1;
	list1.AddTail("your mama");
	list1.AddTail("your pet dog");
	list1.AddTail("911");
	//set initalized list for combobox
	lpRelative->SetListData(3, &list1);
	InsertItem(pParent2, lpRelative);

	//create some node so we can sort them later on, well you can sort all of them but..
	CItemInfo* lpSort = new CItemInfo();
	lpSort->SetItemText("you can sort me and my children");
	CTreeItem *p=InsertItem(pRoot, lpSort);
	for(int a=10;a>=0;a--)
	{
		CItemInfo* lp = new CItemInfo();
		CString str;
		str.Format("%c%c",a+65,a+66);
		lp->SetItemText(str);
		CTreeItem *p1=InsertItem(p, lp);
		if(a%2)
		{
			for(int k=10;k>=0;k--)
			{
				CItemInfo* lp = new CItemInfo();
				CString str;
				str.Format("%c",k+65);
				lp->SetItemText(str);
				InsertItem(p1, lp);
			}
		}
	}

	//could now expand one level
	Expand(pRoot, 0 
	//listview index 0
	); 
	UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
	SetItemState(0, uflag, uflag);

	//Create another rootitem	
	//block 
	{
		CItemInfo* lp = new CItemInfo();
		lp->SetImage(4);
		//add item text
		lp->SetItemText(_T("Another Root Item"));
		CTreeItem * pRoot = InsertRootItem(lp);

		CItemInfo* lpAgent = new CItemInfo();
		lpAgent->SetItemText(_T("Bugs: well it's possible"));
		lpAgent->SetImage(3);
		CTreeItem *pParent2 = InsertItem(pRoot,lpAgent);

		CItemInfo* lpRelative = new CItemInfo();
		lpRelative->SetItemText("if");
		lpRelative->AddSubItemText(_T("You"));
		lpRelative->AddSubItemText(_T("find them"));
		lpRelative->AddSubItemText(_T("you are"));
		lpRelative->AddSubItemText(_T("on your own"));//3 combo goes here
		lpRelative->SetCheck(1); //I want this to be checked if LVS_EX_CHECKBOXES
		//add another combobox in this row in col 3(4)
		lpRelative->SetControlType(lpRelative->CONTROLTYPE::combobox, 3);
		CStringList list1;
		list1.AddTail("screwed");
		list1.AddTail("fucked");
		list1.AddTail("lucky");
		//set initalized list for combobox
		lpRelative->SetListData(3, &list1);
		InsertItem(pParent2, lpRelative);

		Expand(pRoot, NodeToIndex(pRoot) 
		//listview index 0
		); 
		UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
		SetItemState(0, uflag, uflag);
	}
*/



