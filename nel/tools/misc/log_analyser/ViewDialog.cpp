/** \file ViewDialog.cpp
 * implementation file
 *
 * $Id: ViewDialog.cpp,v 1.1 2002/10/18 12:04:56 cado Exp $
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
	if ( LogSessionStartDate.IsEmpty() )
	{
		SessionDatePassed = true;
	}

	((CButton*)GetDlgItem( IDC_BUTTON1 ))->ShowWindow( SW_SHOW );
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
				if ( (! LogSessionStartDate.IsEmpty()) && (strstr( line, "Log Starting [" )) )
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
				if ( (! LogSessionStartDate.IsEmpty()) && (strstr( line, "Log Starting [" )) )
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
	
	return TRUE;
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



