// ListVwEx.cpp : implementation of the CListViewEx class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "ListViewEx.h"
#include "InPlaceEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_IPEDIT 1
#define min(a,b)            (((a) < (b)) ? (a) : (b))

/////////////////////////////////////////////////////////////////////////////
// CListViewEx

IMPLEMENT_DYNCREATE(CListViewEx, CListView)

BEGIN_MESSAGE_MAP(CListViewEx, CListView)
	//{{AFX_MSG_MAP(CListViewEx)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(LVM_SETIMAGELIST, OnSetImageList)
	ON_MESSAGE(LVM_SETTEXTCOLOR, OnSetTextColor)
	ON_MESSAGE(LVM_SETTEXTBKCOLOR, OnSetTextBkColor)
	ON_MESSAGE(LVM_SETBKCOLOR, OnSetBkColor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListViewEx construction/destruction

CListViewEx::CListViewEx()
{
	m_bFullRowSel = FALSE;
	m_bClientWidthSel = TRUE;

	m_cxClient = 0;
	m_cxStateImageOffset = 0;

	m_clrText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_clrTextBk = ::GetSysColor(COLOR_WINDOW);
	m_clrBkgnd = ::GetSysColor(COLOR_WINDOW);

	xSubItem = 1;
	ySubItem = 1;
}

CListViewEx::~CListViewEx()
{
}

BOOL CListViewEx::PreCreateWindow(CREATESTRUCT& cs)
{
	// default is report view and full row selection
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED;
	m_bFullRowSel = TRUE;

	return(CListView::PreCreateWindow(cs));
}

BOOL CListViewEx::SetFullRowSel(BOOL bFullRowSel)
{
	// no painting during change
	LockWindowUpdate();

	m_bFullRowSel = bFullRowSel;

	BOOL bRet;

	if (m_bFullRowSel)
		bRet = ModifyStyle(0L, LVS_OWNERDRAWFIXED);
	else
		bRet = ModifyStyle(LVS_OWNERDRAWFIXED, 0L);

	// repaint window if we are not changing view type
	if (bRet && (GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
		Invalidate();

	// repaint changes
	UnlockWindowUpdate();

	return(bRet);
}

BOOL CListViewEx::GetFullRowSel()
{
	return(m_bFullRowSel);
}

void CListViewEx::SetItem( int y, int x )	
{ 
	int old = ySubItem;
	ySubItem = y; 
	xSubItem = x; 
	if( y != old )
	{
		GetListCtrl().SetItemState( y, LVIS_SELECTED + LVIS_FOCUSED, LVIS_SELECTED + LVIS_FOCUSED  );
		GetListCtrl().RedrawItems( old, old );
	}
	GetListCtrl().RedrawItems( y, y );
}

/////////////////////////////////////////////////////////////////////////////
// CListViewEx drawing

// offsets for first and other columns
#define OFFSET_FIRST    2
#define OFFSET_OTHER    6

void CListViewEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CListCtrl& ListCtrl=GetListCtrl();
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem(lpDrawItemStruct->rcItem);
	UINT uiFlags = ILD_TRANSPARENT;
	int nItem = lpDrawItemStruct->itemID;
	COLORREF clrTextSave, clrBkSave, clrImage = m_clrBkgnd;
	static _TCHAR szBuff[MAX_PATH];
	LPCTSTR pszText;

// get item data
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.pszText = szBuff;
	lvi.cchTextMax = sizeof(szBuff);
	lvi.stateMask = 0xFFFF;     // get all state flags
	ListCtrl.GetItem(&lvi);
	BOOL bSelected = ( (GetFocus() == this)||(GetStyle() & LVS_SHOWSELALWAYS) )&&( lvi.state & (LVIS_SELECTED | LVIS_DROPHILITED | LVIS_FOCUSED) );

// set colors if item is selected
	CRect rcAllLabels;
	ListCtrl.GetItemRect(nItem, rcAllLabels, LVIR_BOUNDS);
	CRect rcLabel;
	ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);
	rcAllLabels.left = rcLabel.left;
	
	if (m_bClientWidthSel && rcAllLabels.right<m_cxClient)
		rcAllLabels.right = m_cxClient;

	ListCtrl.GetSubItemRect( nItem, xSubItem, LVIR_BOUNDS, rcAllLabels ); //Permet d'obtenir la sélection sous forme de sub-items!

		clrTextSave = pDC->SetTextColor(m_clrText);
		clrBkSave = pDC->SetBkColor(m_clrBkgnd);
	if (bSelected)
	{
		ySubItem = nItem;
//		clrTextSave = pDC->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
//		clrBkSave = pDC->SetBkColor(::GetSysColor(COLOR_3DFACE));
		pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_3DFACE)));
	}
	else
	{
//		clrTextSave = pDC->SetTextColor(m_clrText);
//		clrBkSave = pDC->SetBkColor(m_clrBkgnd);
		pDC->FillRect(rcAllLabels, &CBrush(m_clrTextBk));
	}

// draw item label
	ListCtrl.GetItemRect(nItem, rcItem, LVIR_LABEL);
	rcItem.right -= m_cxStateImageOffset;

	pszText = MakeShortString(pDC, szBuff,
				rcItem.right-rcItem.left, 2*OFFSET_FIRST);

	rcLabel = rcItem;
	rcLabel.left += OFFSET_FIRST;
	rcLabel.right -= OFFSET_FIRST;

	pDC->DrawText(pszText,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

// draw labels for extra columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	for(int nColumn = 1; ListCtrl.GetColumn(nColumn, &lvc); nColumn++)
	{
		rcItem.left = rcItem.right;
		rcItem.right += lvc.cx;

		int nRetLen = ListCtrl.GetItemText(nItem, nColumn,
						szBuff, sizeof(szBuff));
		if (nRetLen == 0)
			continue;

		pszText = MakeShortString(pDC, szBuff,
			rcItem.right - rcItem.left, 2*OFFSET_OTHER);

		UINT nJustify = DT_LEFT;

		if(pszText == szBuff)
		{
			switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
			{
			case LVCFMT_RIGHT:
				nJustify = DT_RIGHT;
				break;
			case LVCFMT_CENTER:
				nJustify = DT_CENTER;
				break;
			default:
				break;
			}
		}

		rcLabel = rcItem;
		rcLabel.left += OFFSET_OTHER;
		rcLabel.right -= OFFSET_OTHER;

		pDC->DrawText(pszText, -1, rcLabel,
			nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);
	}

// draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
		pDC->DrawFocusRect(rcAllLabels);
/*
// set original colors if item was selected
	if (bSelected)
	{
		pDC->SetTextColor(clrTextSave);
		pDC->SetBkColor(clrBkSave);
	}
*/
	pDC->SetTextColor(clrTextSave);
	pDC->SetBkColor(clrBkSave);

}

LPCTSTR CListViewEx::MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
	static const _TCHAR szThreeDots[] = _T("...");

	int nStringLen = lstrlen(lpszLong);
	if(nStringLen == 0 || (pDC->GetTextExtent(lpszLong, nStringLen).cx + nOffset) <= nColumnLen)
		return(lpszLong);

	static _TCHAR szShort[MAX_PATH];
	lstrcpy(szShort,lpszLong);
	int nAddLen = pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;
	for(int i = nStringLen-1; i > 0; i--)
	{
		szShort[i] = 0;
		if((pDC->GetTextExtent(szShort, i).cx + nOffset + nAddLen) <= nColumnLen)
		{
			break;
		}
	}
	lstrcat(szShort, szThreeDots);
	return(szShort);
}

void CListViewEx::RepaintSelectedItems()
{
	CListCtrl& ListCtrl = GetListCtrl();
	CRect rcItem, rcLabel;

// invalidate focused item so it can repaint properly
	int nItem = ListCtrl.GetNextItem(-1, LVNI_FOCUSED);

	if(nItem != -1)
	{
		ListCtrl.GetItemRect(nItem, rcItem, LVIR_BOUNDS);
		ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);
		rcItem.left = rcLabel.left;
		InvalidateRect(rcItem, FALSE);
	}

// if selected items should not be preserved, invalidate them
	if(!(GetStyle() & LVS_SHOWSELALWAYS))
	{
		for(nItem = ListCtrl.GetNextItem(-1, LVNI_SELECTED);
			nItem != -1; nItem = ListCtrl.GetNextItem(nItem, LVNI_SELECTED))
		{
			ListCtrl.GetItemRect(nItem, rcItem, LVIR_BOUNDS);
			ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);
			rcItem.left = rcLabel.left;
			InvalidateRect(rcItem, FALSE);
		}
	}

// update changes
	UpdateWindow();
}

// EditSubLabel	- Start edit of a sub item label
// Returns		- Temporary pointer to the new edit control
// nItem		- The row index of the item to edit
// nCol			- The column of the sub item.
CEdit* CListViewEx::EditSubLabel( int nItem, int nCol )
{
	// The returned pointer should not be saved

	// Make sure that the item is visible
	if( !GetListCtrl().EnsureVisible( nItem, TRUE ) ) return NULL;

	// Make sure that nCol is valid
	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	int nColumnCount = pHeader->GetItemCount();
	if( nCol >= nColumnCount || GetListCtrl().GetColumnWidth(nCol) < 5 )
		return NULL;

	// Get the column offset
	int offset = 0;
	for( int i = 0; i < nCol; i++ )
		offset += GetListCtrl().GetColumnWidth( i );

	CRect rect;
	GetListCtrl().GetItemRect( nItem, &rect, LVIR_BOUNDS );

	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect( &rcClient );
	if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		GetListCtrl().Scroll( size );
		rect.left -= size.cx;
	}

	// Get Column alignment
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetListCtrl().GetColumn( nCol, &lvcol );
	DWORD dwStyle ;
	if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dwStyle = ES_LEFT;
	else if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
	else dwStyle = ES_CENTER;

	rect.left += offset;
	rect.right = rect.left + GetListCtrl().GetColumnWidth( nCol );
	if( rect.right > rcClient.right) rect.right = rcClient.right;

	dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL;
	CEdit *pEdit = new CInPlaceEdit(nItem, nCol, GetListCtrl().GetItemText( nItem, nCol ));
	pEdit->Create( dwStyle, rect, this, IDC_IPEDIT );

	return pEdit;
}

/////////////////////////////////////////////////////////////////////////////
// CListViewEx diagnostics

#ifdef _DEBUG

void CListViewEx::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);

	dc << "m_bFullRowSel = " << (UINT)m_bFullRowSel;
	dc << "\n";
	dc << "m_cxStateImageOffset = " << m_cxStateImageOffset;
	dc << "\n";
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListViewEx message handlers

LRESULT CListViewEx::OnSetImageList(WPARAM wParam, LPARAM lParam)
{
	// if we're running Windows 4, there's no need to offset the
	// item text location

	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(info);
	VERIFY(::GetVersionEx(&info));

	if( (int) wParam == LVSIL_STATE && info.dwMajorVersion < 4)
	{
		int cx, cy;

		if(::ImageList_GetIconSize((HIMAGELIST)lParam, &cx, &cy))
			m_cxStateImageOffset = cx;
		else
			m_cxStateImageOffset = 0;
	}

	return(Default());
}

LRESULT CListViewEx::OnSetTextColor(WPARAM wParam, LPARAM lParam)
{
	m_clrText = (COLORREF)lParam;
	return(Default());
}

LRESULT CListViewEx::OnSetTextBkColor(WPARAM wParam, LPARAM lParam)
{
	m_clrTextBk = (COLORREF)lParam;
	return(Default());
}

LRESULT CListViewEx::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	m_clrBkgnd = (COLORREF)lParam;
	return(Default());
}

void CListViewEx::OnSize(UINT nType, int cx, int cy)
{
	m_cxClient = cx;
	CListView::OnSize(nType, cx, cy);
}

void CListViewEx::OnPaint()	////////////////////////// BigBugIci!!!!
{
	// in full row select mode, we need to extend the clipping region
	// so we can paint a selection all the way to the right
	if (m_bClientWidthSel &&
		(GetStyle() & LVS_TYPEMASK) == LVS_REPORT &&
		GetFullRowSel())
	{
		CRect rcAllLabels;
		GetListCtrl().GetItemRect(0, rcAllLabels, LVIR_BOUNDS);

		if(rcAllLabels.right < m_cxClient)
		{
			// need to call BeginPaint (in CPaintDC c-tor)
			// to get correct clipping rect
			CPaintDC dc(this);

			CRect rcClip;
			dc.GetClipBox(rcClip);
			rcClip.left = min(rcAllLabels.right-1, rcClip.left);
			rcClip.right = m_cxClient;
			InvalidateRect(rcClip, FALSE);
			// EndPaint will be called in CPaintDC d-tor
		}
	}
	CListView::OnPaint();
}

void CListViewEx::OnSetFocus(CWnd* pOldWnd)
{
	CListView::OnSetFocus(pOldWnd);
/*
	// check if we are getting focus from label edit box
	if(pOldWnd!=NULL && pOldWnd->GetParent()==this)
		return;
	// repaint items that should change appearance
	if(m_bFullRowSel && (GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
		RepaintSelectedItems();
*/
}

void CListViewEx::OnKillFocus(CWnd* pNewWnd)
{
	CListView::OnKillFocus(pNewWnd);
/*
	// check if we are losing focus to label edit box
	if(pNewWnd != NULL && pNewWnd->GetParent() == this)
		return;
	// repaint items that should change appearance
	if(m_bFullRowSel && (GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
		RepaintSelectedItems();
*/
}

void CListViewEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	LVHITTESTINFO lvhti;
	lvhti.pt = point;
	if( GetListCtrl().SubItemHitTest(&lvhti) == -1 )
		return;
	if( !IsValidX(lvhti.iSubItem) )
		return;
	int oldy = ySubItem;
	ySubItem = lvhti.iItem;
	xSubItem = lvhti.iSubItem;

//	GetListCtrl().RedrawItems( oldy, oldy ); // à utiliser uniquement avec l'ancienne item et la nouvelle!!
	CListView::OnLButtonDown(nFlags, point);
	if(lvhti.flags & LVHT_ONITEM)
	{
		UINT flag = LVIS_FOCUSED;
		if( ( (GetListCtrl().GetItemState( ySubItem, flag ) & flag) == flag )&&( xSubItem >= 0) )
		{
			if( GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS )
				EditSubLabel( ySubItem, xSubItem );
		}
//		else
//			GetListCtrl().SetItemState( ySubItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	}
	GetListCtrl().SetItemState( ySubItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	if( oldy != ySubItem )
		GetListCtrl().RedrawItems( oldy, oldy ); 
	GetListCtrl().RedrawItems( ySubItem, ySubItem ); 
}


void CListViewEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CListView::OnKeyDown(nChar, nRepCnt, nFlags);
	switch( nChar )
	{
	case VK_LEFT:
		{
			if( IsValidX(xSubItem-1) )
				--xSubItem;
		}
		break;
	case VK_RIGHT:
		{
			if( IsValidX(xSubItem+1) )
				++xSubItem;
		}
		break;
/*
	case VK_RETURN:
		{
			CRect r;		
			if( !GetListCtrl().GetSubItemRect( ySubItem, xSubItem, LVIR_BOUNDS, r ) )
				return;
			LVHITTESTINFO lvhti;
			lvhti.pt = r.TopLeft( ) +CPoint(1,1);
			if( GetListCtrl().SubItemHitTest(&lvhti) == -1 )
				return;

			if(lvhti.flags & LVHT_ONITEM)
			{
				UINT flag = LVIS_FOCUSED;
				if( ( (GetListCtrl().GetItemState( ySubItem, flag ) & flag) == flag )&&( xSubItem >= 0) )
				{
					if( GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS )
						EditSubLabel( ySubItem, xSubItem );
				}
				else
					GetListCtrl().SetItemState( ySubItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
			}
		}
		break;
*/
	}
	int oldy = ySubItem;
	CRect r;		
	if( !GetListCtrl().GetSubItemRect( ySubItem, xSubItem, LVIR_BOUNDS, r ) )
		return;
	LVHITTESTINFO lvhti;
	lvhti.pt = r.TopLeft( ) +CPoint(1,1);
	if( GetListCtrl().SubItemHitTest(&lvhti) == -1 )
		return;

	if(lvhti.flags & LVHT_ONITEM)
	{
		UINT flag = LVIS_FOCUSED;
		if( ( (GetListCtrl().GetItemState( ySubItem, flag ) & flag) == flag )&&( xSubItem >= 0) )
		{
			if( GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS )
				EditSubLabel( ySubItem, xSubItem );
		}
		else
			GetListCtrl().SetItemState( ySubItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
	}
	if( oldy != ySubItem )
		GetListCtrl().RedrawItems( oldy, oldy ); 
	GetListCtrl().RedrawItems( ySubItem, ySubItem ); // à utiliser uniquement avec l'ancienne item et la nouvelle!!
}
