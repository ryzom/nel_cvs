/** \file ViewDialog.cpp
 * implementation file
 *
 * $Id: ViewDialog.cpp,v 1.3 2002/12/20 16:32:58 cado Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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
// ViewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "log_analyser.h"
#include "ViewDialog.h"
#include "log_analyserDlg.h"

#include <fstream>
using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CString						LogDateString;
static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);


void CListCtrlEx::initIt()
{
	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE); 
	SetWindowLong( m_hWnd, GWL_STYLE, dwStyle | LVS_OWNERDRAWFIXED );
}

// Adapted from http://zeus.eed.usv.ro/misc/doc/prog/c/mfc/listview/sel_row.html
void CListCtrlEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem(lpDrawItemStruct->rcItem);
	int nItem = lpDrawItemStruct->itemID;
	CImageList* pImageList;

	// Save dc state
	int nSavedDC = pDC->SaveDC();

	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;		// get all state flags
	GetItem(&lvi);

	BOOL bHighlight =((lvi.state & LVIS_DROPHILITED)
					|| ( (lvi.state & LVIS_SELECTED)
						&& ((GetFocus() == this)
							|| (GetStyle() & LVS_SHOWSELALWAYS)
							)
						)
					);

	// Get rectangles for drawing
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel, LVIR_LABEL);
	GetItemRect(nItem, rcIcon, LVIR_ICON);
	CRect rcCol( rcBounds ); 


	CString sLabel = GetItemText( nItem, 0 );

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;

	CRect rcHighlight;
	CRect rcWnd;
	GetClientRect(&rcWnd);
	rcHighlight = rcBounds;
	rcHighlight.left = rcLabel.left;
	rcHighlight.right = rcWnd.right;

	// Draw the background color
	if( bHighlight )
	{
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));

		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
	}
	else
		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_WINDOW)));

	// Set clip region
	rcCol.right = rcCol.left + GetColumnWidth(0);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcCol);
	pDC->SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// Draw state icon
	if (lvi.state & LVIS_STATEIMAGEMASK)
	{
		int nImage = ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		pImageList = GetImageList(LVSIL_STATE);
		if (pImageList)
		{
			pImageList->Draw(pDC, nImage,
				CPoint(rcCol.left, rcCol.top), ILD_TRANSPARENT);
		}
	}
	
	// Draw normal and overlay icon
	pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList)
	{
		UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
		pImageList->Draw(pDC, lvi.iImage, 
			CPoint(rcIcon.left, rcIcon.top),
			ILD_TRANSPARENT | nOvlImageMask );
	}

	
	
	// Draw item label - Column 0
	rcLabel.left += offset/2;
	rcLabel.right -= offset;

	pDC->SetTextColor( _ViewDialog->getColorForLine( nItem ) );
	pDC->DrawText(sLabel,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP 
				| DT_VCENTER | DT_END_ELLIPSIS);


	// Draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right :
							rcBounds.right;
	rgn.CreateRectRgnIndirect(&rcBounds);
	pDC->SelectClipRgn(&rgn);
				   
	for(int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)
	{
		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		sLabel = GetItemText(nItem, nColumn);
		if (sLabel.GetLength() == 0)
			continue;

		// Get the text justification
		UINT nJustify = DT_LEFT;
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

		rcLabel = rcCol;
		rcLabel.left += offset;
		rcLabel.right -= offset;

		pDC->DrawText(sLabel, -1, rcLabel, nJustify | DT_SINGLELINE | 
					DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
	}

	// Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
	{
		pDC->DrawFocusRect(rcHighlight);
	}

	
	// Restore dc
	pDC->RestoreDC( nSavedDC );
}


void CListCtrlEx::RepaintSelectedItems()
{
	CRect rcBounds, rcLabel;

	// Invalidate focused item so it can repaint 
	int nItem = GetNextItem(-1, LVNI_FOCUSED);

	if(nItem != -1)
	{
		GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
		GetItemRect(nItem, rcLabel, LVIR_LABEL);
		rcBounds.left = rcLabel.left;

		InvalidateRect(rcBounds, FALSE);
	}

	// Invalidate selected items depending on LVS_SHOWSELALWAYS
	if(!(GetStyle() & LVS_SHOWSELALWAYS))
	{
		for(nItem = GetNextItem(-1, LVNI_SELECTED);
			nItem != -1; nItem = GetNextItem(nItem, LVNI_SELECTED))
		{
			GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
			GetItemRect(nItem, rcLabel, LVIR_LABEL);
			rcBounds.left = rcLabel.left;

			InvalidateRect(rcBounds, FALSE);
		}
	}

	UpdateWindow();
}


/*void CListCtrlEx::OnKillFocus(CWnd* pNewWnd) 
{
	CListCtrl::OnKillFocus(pNewWnd);

	// check if we are losing focus to label edit box
	if(pNewWnd != NULL && pNewWnd->GetParent() == this)
		return;

	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
		RepaintSelectedItems();
}

void CListCtrlEx::OnSetFocus(CWnd* pOldWnd) 
{
	CListCtrl::OnSetFocus(pOldWnd);
	
	// check if we are getting focus from label edit box
	if(pOldWnd!=NULL && pOldWnd->GetParent()==this)
		return;

	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
		RepaintSelectedItems();
}*/


/////////////////////////////////////////////////////////////////////////////
// CViewDialog dialog


CViewDialog::CViewDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CViewDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewDialog)
	m_Caption = _T("");
	//}}AFX_DATA_INIT
}


void CViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewDialog)
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	DDX_Text(pDX, IDC_Service, m_Caption);
	//}}AFX_DATA_MAP
}


/*
 *Load, using the current filters
 */
void		CViewDialog::reload()
{
	CWaitCursor wc;
	if ( LogSessionStartDate.IsEmpty() || (LogSessionStartDate == "Beginning") )
	{
		SessionDatePassed = true;
	}

	((CButton*)GetDlgItem( IDC_BUTTON1 ))->ShowWindow( SW_SHOW );
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->ShowWindow( SW_SHOW );
	m_Caption.Format( "%s %u+ %u- (%s)", Filename, PosFilter.size(), NegFilter.size(), LogSessionStartDate.IsEmpty()?"all":CString("session ")+LogSessionStartDate );
	UpdateData( false );
	clear();
	setRedraw( false );

	ifstream ifs( Filename );
	if ( ! ifs.fail() )
	{
		char line [1024];
		while ( ! ifs.eof() )
		{
			ifs.getline( line, 1024 );
			if ( SessionDatePassed )
			{
				// Stop if the session is finished
				if ( (! LogSessionStartDate.IsEmpty()) && (strstr( line, LogDateString )) )
					break;

				// Apply the filters
				if ( passFilter( line ) )
					addLine( line );
			}
			else
			{
				// Look for the session beginning
				if ( strstr( line, LogSessionStartDate ) != NULL )
				{
					SessionDatePassed = true;
				}
			}
		}
	}
	else
	{
		addLine( "<Cannot open log file>" );
	}

	commitAddedLines();

	setRedraw( true );
}


/*
 * Returns true if the string must be logged, according to the current filters
 */
bool		CViewDialog::passFilter( const char *filter ) const
{
	bool yes = PosFilter.empty();

	bool found;
	vector<CString>::const_iterator ilf;

	// 1. Positive filter
	for ( ilf=PosFilter.begin(); ilf!=PosFilter.end(); ++ilf )
	{
		found = ( strstr( filter, *ilf ) != NULL );
		if ( found )
		{
			yes = true; // positive filter passed (no need to check another one)
			break;
		}
		// else try the next one
	}
	if ( ! yes )
	{
		return false; // positive filter not passed
	}

	// 2. Negative filter
	for ( ilf=NegFilter.begin(); ilf!=NegFilter.end(); ++ilf )
	{
		found = ( strstr( filter, *ilf ) != NULL );
		if ( found )
		{
			return false; // negative filter not passed (no need to check another one)
		}
	}
	return true; // negative filter passed
}


/*
 * Load trace
 */
void		CViewDialog::reloadTrace()
{
	CWaitCursor wc;
	((CButton*)GetDlgItem( IDC_BUTTON1 ))->ShowWindow( SW_HIDE );
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->ShowWindow( SW_HIDE );
	if ( LogSessionStartDate.IsEmpty() )
	{
		SessionDatePassed = true;
		if ( PosFilter.empty() )
			m_Caption = "Trace of " + Filename + " (all)";
		else
			m_Caption = "Trace of " + PosFilter[0] + " (all)";
	}
	else
	{
		if ( LogSessionStartDate == "Beginning" )
		{
			SessionDatePassed = true;
		}
		if ( PosFilter.empty() )
			m_Caption = "Trace of " + Filename + " (session " + LogSessionStartDate + ")" ;
		else
			m_Caption = "Trace of " + PosFilter[0] + " (session " + LogSessionStartDate + ")" ;
	}

	UpdateData( false );
	clear();

	ifstream ifs( Filename );
	if ( ! ifs.fail() )
	{
		char line [1024];
		while ( ! ifs.eof() )
		{
			ifs.getline( line, 1024 );
			if ( SessionDatePassed )
			{
				// Stop if the session is finished
				if ( (! LogSessionStartDate.IsEmpty()) && (strstr( line, LogDateString )) )
					break;

				// Read if it's a TRACE
				char *pc = strstr( line, "TRACE" );
				if ( pc != NULL )
				{
					if ( PosFilter.empty() || (strncmp( pc-PosFilter[0].GetLength(), PosFilter[0], PosFilter[0].GetLength() ) == 0) )
					{
						((CLog_analyserDlg*)GetParent())->insertTraceLine( Index, pc+6 );
					}
				}
			}
			else
			{
				// Look for the session beginning
				if ( strstr( line, LogSessionStartDate ) != NULL )
				{
					SessionDatePassed = true;
				}
			}
		}

		addLine( "<After adding all the views" );
		addLine( "you need, click Compute Traces" );
		addLine( "to generate all the views>" );
	}
	else
	{
		addLine( "<Cannot open log file>" );
	}

	commitAddedLines();
}


BEGIN_MESSAGE_MAP(CViewDialog, CDialog)
	//{{AFX_MSG_MAP(CViewDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnButtonFilter)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST1, OnGetdispinfoList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, OnSetfocusList1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButtonFind)
	ON_REGISTERED_MESSAGE( WM_FINDREPLACE, OnFindReplace )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewDialog message handlers


/*
 *
 */
void CViewDialog::OnSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Force to display the current item when the current view changes
	if ( m_ListCtrl.GetSelectionMark() != -1 )
		displayString();

	m_ListCtrl.RepaintSelectedItems();
	*pResult = 0;
}

/*
 *
 */
void CViewDialog::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Display the current item when it changes
	if ( pNMListView->iItem != -1 )
		displayString();

	m_ListCtrl.RepaintSelectedItems();
	*pResult = 0;
}


/*
 * Resize
 */
void CViewDialog::resizeView( int index, int nbViews, int top )
{
	RECT parentRect;
	GetParent()->GetClientRect( &parentRect );

	int width = (parentRect.right-32)/nbViews;
	RECT viewRect;
	viewRect.left = index*width;
	viewRect.top = top;
	viewRect.right = viewRect.left + width;
	viewRect.bottom = parentRect.bottom-10;
	MoveWindow( &viewRect, TRUE );
	
	m_ListCtrl.MoveWindow( 5, 32, width-5, viewRect.bottom-top-42 );
	LVCOLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = width-24;
	m_ListCtrl.SetColumn( 0, &lvc );
}


/*
 * Return the nb of lines
 */
int CViewDialog::getNbLines() const
{
	return Buffer.size();
}


/*
 * Return the nb of visible lines
 */
int CViewDialog::getNbVisibleLines() const
{
	return m_ListCtrl.GetCountPerPage();
}


/*
 * Fill from getNbLines() to maxNbLines with blank lines
 */
void CViewDialog::fillGaps( int maxNbLines )
{
	int nbLines = getNbLines();
	for ( int i=0; i!=maxNbLines-nbLines; ++i )
	{
		addLine( "" );
	}
}


/*
 * Commit the lines previously added
 */
void CViewDialog::commitAddedLines()
{
	m_ListCtrl.SetItemCount( Buffer.size() );
}


/*
 * Scroll
 */
void CViewDialog::scrollTo( int index )
{
	int deltaIndex = index - m_ListCtrl.GetTopIndex();
	RECT rect;
	if ( m_ListCtrl.GetItemRect( 0, &rect, LVIR_BOUNDS ) )
	{
		int itemH = rect.bottom-rect.top;
		m_ListCtrl.Scroll( CSize( 0, deltaIndex*itemH ) );
	}
	
	//m_ListCtrl.EnsureVisible( index, false );
}


/*
 * Select
 */
void CViewDialog::select( int index )
{
	LVITEM itstate;
	itstate.mask = LVIF_STATE;
	itstate.state = 0;
	int sm = m_ListCtrl.GetSelectionMark();
	if ( sm != -1 )
	{
		m_ListCtrl.SetItemState( sm, &itstate );
	}

	if ( index != -1 )
	{
		itstate.state = LVIS_SELECTED | LVIS_DROPHILITED | LVIS_FOCUSED;
		m_ListCtrl.SetItemState( index, &itstate );
		m_ListCtrl.SetSelectionMark( index );
	}
}


/*
 * Return the index of the top of the listbox
 */
int CViewDialog::getScrollIndex() const
{
	return m_ListCtrl.GetTopIndex();
}


/*
 * Clear
 */
void CViewDialog::clear()
{
	Buffer.clear();
	m_ListCtrl.DeleteAllItems();
}


/*
 *
 */
void CViewDialog::OnButtonFilter() 
{
	if ( ((CLog_analyserDlg*)GetParent())->FilterDialog.DoModal() )
	{
		PosFilter = ((CLog_analyserDlg*)GetParent())->FilterDialog.getPosFilter();
		NegFilter = ((CLog_analyserDlg*)GetParent())->FilterDialog.getNegFilter();
	}

	if ( ! ((CLog_analyserDlg*)GetParent())->Trace )
	{
		reload();
	}
}

BOOL CViewDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ListCtrl.GetHeaderCtrl()->ModifyStyle( 0, HDS_HIDDEN );
	m_ListCtrl.InsertColumn( 0, "" );
	m_ListCtrl.setViewDialog( this );
	m_ListCtrl.initIt();

	BeginFindIndex = -1;
	FindDialog = NULL;
	return TRUE;
}


/*
 * Return the color
 */
COLORREF CViewDialog::getColorForLine( int index )
{
	if ( Buffer[index].Find( "DBG" ) != -1 )
		return RGB(0x80,0x80,0x80);
	else if ( Buffer[index].Find( "WRN" ) != -1 )
		return RGB(0x80,0,0);
	else if ( Buffer[index].Find( "ERR" ) != -1 )
		return RGB(0xFF,0,0);
	else // INF and others
		return RGB(0,0,0);
}


void CViewDialog::OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	int iItemIndx = pItem->iItem;
	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		lstrcpy( pItem->pszText, Buffer[iItemIndx] );
	}
	*pResult = 0;
}


/*
 * Display string
 */
void CViewDialog::displayString()
{
	// Build the string
	CString s;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while ( pos != NULL )
	{
		int index = m_ListCtrl.GetNextSelectedItem( pos );
		s += Buffer[index] + "\r\n";
	}

	// Display it
	((CLog_analyserDlg*)GetParent())->displayCurrentLine( s );
}


/*
 * Search string
 */
void CViewDialog::OnButtonFind() 
{
	m_ListCtrl.ModifyStyle( 0, LVS_SHOWSELALWAYS );
	BeginFindIndex = m_ListCtrl.GetSelectionMark()+1;
	select( -1 );
	FindDialog = new CFindReplaceDialog();
	FindDialog->Create( true, FindStr, NULL, FR_DOWN | FR_HIDEWHOLEWORD, this );
}


bool matchString( const CString& str, const CString& substr, bool matchCase )
{
	if ( matchCase )
	{
		return str.Find( substr ) != -1;
	}
	else
	{
		CString str2 = str, substr2 = substr;
		str2.MakeUpper();
		substr2.MakeUpper();
		return str2.Find( substr2 ) != -1;
	}
}


/*
 *
 */
afx_msg LONG CViewDialog::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
	if ( FindDialog->IsTerminating() )
    {
		m_ListCtrl.ModifyStyle( LVS_SHOWSELALWAYS, 0 );
		select( -1 );
        return 0;
	}

	if ( FindDialog->FindNext() )
	{
		FindStr = FindDialog->GetFindString();
		int index;
		if ( FindDialog->SearchDown() )
		{
			for ( index=BeginFindIndex; index!=(int)Buffer.size(); ++index )
			{
				if ( matchString( Buffer[index], FindStr, FindDialog->MatchCase()!=0 ) )
				{
					scrollTo( index );
					select( index );
					BeginFindIndex = m_ListCtrl.GetSelectionMark()+1;
					displayString();
					return 1;
				}
			}
		}
		else
		{
			for ( index=BeginFindIndex; index>=0; --index )
			{
				if ( matchString( Buffer[index], FindStr, FindDialog->MatchCase()!=0 ) )
				{
					scrollTo( index );
					select( index );
					BeginFindIndex = m_ListCtrl.GetSelectionMark()-1;
					displayString();
					return 1;
				}
			}
		}
		AfxMessageBox( "Not found" );
		BeginFindIndex = 0;
		return 0;
	}

	return 0;
}


