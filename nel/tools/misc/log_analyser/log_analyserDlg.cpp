/** \file log_analyserDlg.cpp
 * implementation file
 *
 * $Id: log_analyserDlg.cpp,v 1.2 2002/10/21 09:01:02 cado Exp $
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
// log_analyserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "log_analyser.h"
#include "log_analyserDlg.h"
//#include <nel/misc/config_file.h>
#include <fstream>

using namespace std;
//using namespace NLMISC;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CString						LogDateString;


/////////////////////////////////////////////////////////////////////////////
// CLog_analyserDlg dialog

CLog_analyserDlg::CLog_analyserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLog_analyserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLog_analyserDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLog_analyserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLog_analyserDlg)
	DDX_Control(pDX, IDC_SCROLLBAR1, m_ScrollBar);
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLog_analyserDlg, CDialog)
	//{{AFX_MSG_MAP(CLog_analyserDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_AddView, OnAddView)
	ON_BN_CLICKED(IDC_ADDTRACEVIEW, OnAddtraceview)
	ON_BN_CLICKED(IDC_ComputeTraces, OnComputeTraces)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_Reset, OnReset)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_HelpBtn, OnHelpBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLog_analyserDlg message handlers

BOOL CLog_analyserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	Trace = false;
	((CButton*)GetDlgItem( IDC_CheckSessions ))->SetCheck( 1 );

	/*try
	{
		CConfigFile cf;
		cf.load( "log_analyser.cfg" );
		LogDateString = cf.getVar( "LogDateString" ).asString().c_str();
	}
	catch ( EConfigFile& )
	{*/
	LogDateString = "Log Starting [";
	//}
	

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLog_analyserDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLog_analyserDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/*
 *
 */
void CLog_analyserDlg::OnAddView() 
{
	CFileDialog openDialog( true, NULL, "log.log", OFN_HIDEREADONLY, "Log files (*.log)|*.log|All files|*.*||", this );
	if ( openDialog.DoModal() == IDOK )
	{
		CWaitCursor wc;

		CViewDialog *view = onAddCommon( openDialog.GetPathName() );

		// Set filters
		FilterDialog.Trace = false;
		if ( FilterDialog.DoModal() )
		{
			view->PosFilter = FilterDialog.getPosFilter();
			view->NegFilter = FilterDialog.getNegFilter();
		}

		// Load file
		view->reload();
	}
}


/*
 *
 */
void CLog_analyserDlg::OnAddtraceview() 
{
	CFileDialog openDialog( true, NULL, "log.log", OFN_HIDEREADONLY, "Log files (*.log)|*.log|All files|*.*||", this );
	if ( openDialog.DoModal() == IDOK )
	{
		CViewDialog *view = onAddCommon( openDialog.GetPathName() );

		// Set filters
		FilterDialog.Trace = true;
		if ( FilterDialog.DoModal() == IDOK )
		{
			view->PosFilter = FilterDialog.getPosFilter();
		}

		// Load file
		view->reloadTrace();
	}
}


/*
 *
 */
CViewDialog *CLog_analyserDlg::onAddCommon( const CString& filename )
{
	CWaitCursor wc;
	
	// Create view
	CViewDialog *view = new CViewDialog();
	view->Create( IDD_View, this );
	RECT editRect;
	m_Edit.GetWindowRect( &editRect );
	ScreenToClient( &editRect );
	Views.push_back( view );
	int i;
	for ( i=0; i!=(int)Views.size(); ++i )
	{
		Views[i]->resizeView( i, Views.size(), editRect.bottom+10 );
	}
	view->ShowWindow( SW_SHOW );

	// Set params
	view->Index = Views.size()-1;
	view->Filename = filename;
	view->LogSessionStartDate = "";
	view->SessionDatePassed = false;

	if ( ((CButton*)GetDlgItem( IDC_CheckSessions ))->GetCheck() == 1 )
	{
		// Scan file for log sessions dates
		int nbsessions = 0;
		ifstream ifs( filename );
		if ( ! ifs.fail() )
		{
			LogSessionsDialog.addLogSession( "Beginning" );

			char line [1024];
			while ( ! ifs.eof() )
			{
				ifs.getline( line, 1024 );
				if ( strstr( line, LogDateString ) != NULL )
				{
					LogSessionsDialog.addLogSession( line );
					++nbsessions;
				}
			}
			if ( (nbsessions>1) && (LogSessionsDialog.DoModal() == IDOK) )
			{
				view->LogSessionStartDate = LogSessionsDialog.getStartDate();
			}
		}
	}

	return view;
}


/*
 *
 */
void CLog_analyserDlg::displayCurrentLine( const CString& line )
{
	m_Edit.SetSel( 0, -1 );
	m_Edit.Clear();
	m_Edit.ReplaceSel( line, true );
}


/*
 *
 */
void CLog_analyserDlg::insertTraceLine( int index, char *traceLine )
{
	/*CString s0;
	s0.Format( "%s", traceLine );
	MessageBox( s0 );*/

	char *line = strchr( traceLine, ':' );
	char scycle [10];
	strncpy( scycle, traceLine, line-traceLine );
	int cycle = atoi(scycle);
	TStampedLine stampedLine;
	stampedLine.Index = index;
	stampedLine.Line = CString(traceLine);
	TraceMap.insert( make_pair( cycle, stampedLine ) );

	/*CString s;
	s.Format( "%d - %s", cycle, line );
	MessageBox( s );*/
}


/*
 *
 */
void CLog_analyserDlg::OnComputeTraces() 
{
	CWaitCursor wc;

	if ( Views.empty() )
		return;

	Trace = true;
	
	int j;
	for ( j=0; j!=(int)Views.size(); ++j )
	{
		Views[j]->clear();
		Views[j]->setRedraw( false );
	}

	multimap<int, TStampedLine>::iterator itm = TraceMap.begin(), itmU, itmC;
	while ( itm != TraceMap.end() )
	{
		// Fill all the views for one cycle
		itmU = TraceMap.upper_bound( (*itm).first );
		for ( itmC=itm; itmC!=itmU; ++itmC )
		{
			TStampedLine& stampedLine = (*itmC).second;
			Views[stampedLine.Index]->addLine( stampedLine.Line );
		}

		// Get the number of lines of the most filled view
		int i, maxNbLines=0;
		for ( i=0; i!=(int)Views.size(); ++i )
		{
			int vnb = Views[i]->getNbLines();
			if ( vnb > maxNbLines )
			{
				maxNbLines = vnb;
			}
		}

		// Fill the gaps with blank lines
		for ( i=0; i!=(int)Views.size(); ++i )
		{
			Views[i]->fillGaps( maxNbLines );
		}

		itm = itmU;
	}

	for ( j=0; j!=(int)Views.size(); ++j )
	{
		Views[j]->commitAddedLines();
		Views[j]->setRedraw( true );
	}

	m_ScrollBar.SetScrollRange( 0, Views[0]->getNbLines()-Views[0]->getNbVisibleLines() );
	m_ScrollBar.ShowWindow( SW_SHOW );
}

void CLog_analyserDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( Trace )
	{
		int index;
		switch ( nSBCode )
		{
		case SB_TOP : index = 0; break;
		case SB_BOTTOM : index = Views[0]->getNbLines()-1; break;
		case SB_ENDSCROLL : index = -1; break;
		case SB_LINEDOWN : index = Views[0]->getScrollIndex()+1; break;
		case SB_LINEUP : index = Views[0]->getScrollIndex()-1; break;
		case SB_PAGEDOWN : index = Views[0]->getScrollIndex()+Views[0]->getNbVisibleLines(); break;
		case SB_PAGEUP : index = Views[0]->getScrollIndex()-Views[0]->getNbVisibleLines(); break;
		case SB_THUMBPOSITION :
		case SB_THUMBTRACK :
			index = nPos;
			break;
		}

		if ( index != -1 )
		{
			// Scroll the views
			for ( int i=0; i!=(int)Views.size(); ++i )
			{
				Views[i]->scrollTo( index );
			}

			pScrollBar->SetScrollPos( index );
		}
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}


/*
 *
 */
void CLog_analyserDlg::OnReset() 
{
	m_Edit.SetSel( 0, -1 );
	m_Edit.Clear();

	vector<CViewDialog*>::iterator iv;
	for ( iv=Views.begin(); iv!=Views.end(); ++iv )
	{
		(*iv)->DestroyWindow();
		delete (*iv);
	}
	Views.clear();

	Trace = false;
	TraceMap.clear();
	m_ScrollBar.ShowWindow( SW_HIDE );
}


/*
 *
 */
void CLog_analyserDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if ( ::IsWindow(m_Edit) )
	{
		RECT cltRect, editRect, sbRect;
		GetClientRect( &cltRect ),
		m_Edit.GetWindowRect( &editRect );
		m_ScrollBar.GetWindowRect( &sbRect );
		ScreenToClient( &editRect );
		ScreenToClient( &sbRect );
		editRect.right = cltRect.right-16;
		sbRect.right += cltRect.right-28-sbRect.left;
		sbRect.left = cltRect.right-28;
		sbRect.bottom = cltRect.bottom-12;
		m_Edit.MoveWindow( &editRect );
		m_ScrollBar.MoveWindow( &sbRect );

		int i;
		for ( i=0; i!=(int)Views.size(); ++i )
		{
			Views[i]->resizeView( i, Views.size(), editRect.bottom+10 );
		}
	}
}


/*
 *
 */
void CLog_analyserDlg::OnDestroy() 
{
	OnReset();

	CDialog::OnDestroy();
}


/*
 *
 */
void CLog_analyserDlg::OnHelpBtn() 
{
	CString s = "NeL Log Analyser\n(c) 2002 Nevrax\n\n";
	s += "Simple Mode: open one or more log files using the button 'Add View...'.\n";
	s += "If the file being opened contains several log sessions, you can choose one or choose\n";
	s += "to display all sessions, if the checkbox 'Browse Log Sessions' is enabled. You can\n";
	s += "also add some filters. Then click a log line to display it in its entirety in the\n";
	s += "top field.\n\n";
	s += "Trace Mode: open several log files in Trace Format (see below) using the button\n";
	s += "'Add Trace View...', you can limit the lines loaded to the ones containing a\n";
	s += "specific service shortname (see below). Then click the button 'Compute Traces'\n";
	s += "to display the matching lines. The lines are sorted using their gamecycle and\n";
	s += "blank lines are filled so that different parallel views have the same timeline.\n";
	s += "Use the right scrollbar to scroll all the views at the same time.\n\n";
	s += "The logs in Trace Format should contains some lines that have a substring sTRACE:n:\n";
	s += "where s is an optional service name (e.g. FS) and n is the gamecycle of the action\n";
	s += "(an integer).";
	MessageBox( s );	
}