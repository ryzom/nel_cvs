/** \file log_analyserDlg.cpp
 * implementation file
 *
 * $Id: log_analyserDlg.cpp,v 1.5 2003/05/14 17:26:11 cado Exp $
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
#include <algorithm>

using namespace std;
//using namespace NLMISC;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CLog_analyserApp		theApp;
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
	ON_WM_LBUTTONUP()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_DispLineHeaders, OnDispLineHeaders)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLog_analyserDlg message handlers

BOOL CLog_analyserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	Trace = false;
	ResizeViewInProgress = -1;
	((CButton*)GetDlgItem( IDC_CheckSessions ))->SetCheck( 1 );
	((CButton*)GetDlgItem( IDC_DispLineHeaders ))->SetCheck( 1 );

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

	// Add files given in command-line
	string cmdLine = string(theApp.m_lpCmdLine);
	vector<CString> v;
	/*int pos = cmdLine.find_first_of(' '); // TODO: handle "" with blank characters
	while ( pos != string::npos )
	{
		v.push_back( cmdLine.substr( 0, pos ).c_str() );
		cmdLine = cmdLine.substr( pos );
		pos = cmdLine.find_first_of(' ');
	}*/
	if ( ! cmdLine.empty() )
	{
		v.push_back( cmdLine.c_str() );
		addView( v );
	}

	DragAcceptFiles( true );
	
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
 * Open in the same view
 */
void CLog_analyserDlg::OnDropFiles( HDROP hDropInfo )
{
	UINT nbFiles = DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );
	vector<CString> v;
	for ( UINT i=0; i!=nbFiles; ++i )
	{
		CString filename;
		DragQueryFile( hDropInfo, i, filename.GetBufferSetLength( 200 ), 200 );
		v.push_back( filename );
	}
	addView( v );
}


bool	isNumberChar( char c )
{
	return (c >= '0') && (c <= '9');
}

/*
 *
 */
void CLog_analyserDlg::OnAddView()
{
	vector<CString> v;
	addView( v );
}


/*
 *	 
 */
void CLog_analyserDlg::addView( std::vector<CString>& pathNames ) 
{
	if ( pathNames.empty() )
	{
		CFileDialog openDialog( true, NULL, "log.log", OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT, "Log files (*.log)|*.log|All files|*.*||", this );
		CString filenameList;
		openDialog.m_ofn.lpstrFile = filenameList.GetBufferSetLength( 8192 );
		openDialog.m_ofn.nMaxFile = 8192;
		if ( openDialog.DoModal() == IDOK )
		{
			CWaitCursor wc;

			// Get the selected filenames
			CString pathName;
			POSITION it = openDialog.GetStartPosition();
			while ( it != NULL )
			{
				pathNames.push_back( openDialog.GetNextPathName( it ) );
			}
			if ( pathNames.empty() )
				return;
		}
		else
			return;
	}

	unsigned int i;
	if ( pathNames.size() > 1 )
	{
		// Sort the filenames
		for ( i=0; i!=pathNames.size(); ++i )
		{
			// Ensure that a log file without number comes *after* the ones with a number
			string name = string(pathNames[i]);
			unsigned int dotpos = name.find_last_of('.');
			if ( (dotpos!=string::npos) && (dotpos > 2) )
			{
				if ( ! (isNumberChar(name[dotpos-1]) && isNumberChar(name[dotpos-2]) && isNumberChar(name[dotpos-3])) )
				{
					name = name.substr( 0, dotpos ) + "ZZZ" + name.substr( dotpos );
					pathNames[i] = name.c_str();
				}
			}
		}
		sort( pathNames.begin(), pathNames.end() );
		for ( i=0; i!=pathNames.size(); ++i )
		{
			// Set the original names back
			string name = pathNames[i];
			unsigned int tokenpos = name.find( "ZZZ." );
			if ( tokenpos != string::npos )
			{
				name = name.substr( 0, tokenpos ) + name.substr( tokenpos + 3 );
				pathNames[i] = name.c_str();
			}
		}
	}

	// Display the filenames
	string names;
	if ( isLogSeriesEnabled() )
		names += "Loading series corresponding to :\r\n";
	else
		names += "Loading files:\r\n";
	for ( i=0; i!=pathNames.size(); ++i )
		names += string(pathNames[i]) + "\r\n";
	displayCurrentLine( names.c_str() );
	
	// Add view and browse sessions if needed
	CViewDialog *view = onAddCommon( pathNames );

	// Set filters
	FilterDialog.Trace = false;
	if ( FilterDialog.DoModal() == IDOK )
	{
		view->PosFilter = FilterDialog.getPosFilter();
		view->NegFilter = FilterDialog.getNegFilter();

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
		vector<CString> pathNames;
		pathNames.push_back( openDialog.GetPathName() );
		CViewDialog *view = onAddCommon( pathNames );

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
CViewDialog *CLog_analyserDlg::onAddCommon( const vector<CString>& filenames )
{
	CWaitCursor wc;
	
	// Create view
	CViewDialog *view = new CViewDialog();
	view->Create( IDD_View, this );
	view->Index = Views.size();
	RECT editRect;
	m_Edit.GetWindowRect( &editRect );
	ScreenToClient( &editRect );
	RECT parentRect;
	GetClientRect( &parentRect );
	Views.push_back( view );
	int i, w = 0;
	for ( i=0; i!=(int)Views.size(); ++i )
	{
		Views[i]->WidthR = 1.0f/(float)Views.size();
		Views[i]->resizeView( Views.size(), editRect.bottom+10, w );
		w += (int)(Views[i]->WidthR*(parentRect.right-32));
	}
	view->ShowWindow( SW_SHOW );

	// Set params
	if ( filenames.size() == 1 )
	{
		// One file or a whole log series
		view->Seriesname = filenames.front();
		getLogSeries( filenames.front(), view->Filenames );
	}
	else
	{
		// Multiple files
		view->Seriesname = filenames.front() + "...";
		view->Filenames = filenames;
	}

	view->LogSessionStartDate = "";
	LogSessionsDialog.clear();

	if ( ((CButton*)GetDlgItem( IDC_CheckSessions ))->GetCheck() == 1 )
	{
		LogSessionsDialog.addLogSession( "Beginning" );
		int nbsessions = 0;
		for ( i=0; i!=(int)(view->Filenames.size()); ++i )
		{
			// Scan file for log sessions dates
			ifstream ifs( view->Filenames[i] );
			if ( ! ifs.fail() )
			{
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
			}
		}
		if ( (nbsessions>1) && (LogSessionsDialog.DoModal() == IDOK) )
		{
			view->LogSessionStartDate = LogSessionsDialog.getStartDate();
		}
	}

	return view;
}


int smprintf( char *buffer, size_t count, const char *format, ... )
{
	int ret;

	va_list args;
	va_start( args, format );
	ret = vsnprintf( buffer, count, format, args );
	if ( ret == -1 )
	{
		buffer[count-1] = '\0';
	}
	va_end( args );

	return( ret );
}


/*
 *
 */
void CLog_analyserDlg::getLogSeries( const CString& filenameStr, std::vector<CString>& filenameList )
{
	if ( isLogSeriesEnabled() )
	{
		string filename = filenameStr;
		unsigned int dotpos = filename.find_last_of ('.');
		if ( dotpos != string::npos )
		{
			string start = filename.substr( 0, dotpos );
			string end = filename.substr( dotpos );
			char numchar [4];
			unsigned int i = 0;
			bool anymore = true;
			while ( anymore )
			{
				// If filename is my_service.log, try my_service001.log..my_service999.log
				string npath = start;
				smprintf( numchar, 4, "%03d", i );
				npath += numchar + end;
				if ( ! ! fstream( npath.c_str(), ios::in ) )
				{
					// File exists => add it
					filenameList.push_back( npath.c_str() );
					if ( i == 999 )
					{
						filenameList.push_back( "<Too many log files in the series>" );
						anymore = false;
					}
					++i;
				}
				else
				{
					// No more files
					anymore = false;
				}
			}
		}
	}

	// At last, add the filename
	filenameList.push_back( filenameStr );
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
void CLog_analyserDlg::OnDispLineHeaders() 
{
	vector<CViewDialog*>::iterator iv;
	for ( iv=Views.begin(); iv!=Views.end(); ++iv )
	{
		(*iv)->Invalidate();
	}		
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

		resizeViews();
	}
}


/*
 *
 */
void CLog_analyserDlg::resizeViews()
{
	RECT editRect;
	m_Edit.GetWindowRect( &editRect );
	ScreenToClient( &editRect );
	RECT parentRect;
	GetClientRect( &parentRect );
	int i, w = 0;
	for ( i=0; i!=(int)Views.size(); ++i )
	{
		Views[i]->resizeView( Views.size(), editRect.bottom+10, w );
		w += (int)(Views[i]->WidthR*(parentRect.right-32));
	}
}


/*
 *
 */
void CLog_analyserDlg::beginResizeView( int index )
{
	ResizeViewInProgress = index;
	SetCursor( theApp.LoadStandardCursor( IDC_SIZEWE ) );
	SetCapture();
}


/*
 * 
 */
void CLog_analyserDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( ResizeViewInProgress != -1 )
	{
		if ( ResizeViewInProgress > 0 )
		{
			RECT viewRect, appClientRect;
			Views[ResizeViewInProgress]->GetWindowRect( &viewRect );
			ScreenToClient( &viewRect );
			GetClientRect( &appClientRect );
			if ( point.x < 0 )
				point.x = 10;
			int deltaPosX = point.x - viewRect.left;
			float deltaR = (float)deltaPosX / (float)(appClientRect.right-32);
			if ( -deltaR > Views[ResizeViewInProgress-1]->WidthR )
				deltaR = -Views[ResizeViewInProgress-1]->WidthR + 0.01f;
			if ( deltaR > Views[ResizeViewInProgress]->WidthR )
				deltaR = Views[ResizeViewInProgress]->WidthR - 0.01f;
			Views[ResizeViewInProgress-1]->WidthR += deltaR;
			Views[ResizeViewInProgress]->WidthR -= deltaR;
		}
		ResizeViewInProgress = -1;
		ReleaseCapture();
		SetCursor( theApp.LoadStandardCursor( IDC_ARROW ) );
		resizeViews();
	}

	CDialog::OnLButtonUp(nFlags, point);
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
	CString s = "NeL Log Analyser v1.3.0-alpha\n(c) 2002-2003 Nevrax\n\n";
	s += "Simple Mode: open one or more log files using the button 'Add View...'.\n";
	s += "You can make a multiple selection, then the files will be sorted by log order.\n";
	s += "If the file(s) being opened contain(s) several log sessions, you can choose one or\n";
	s += "choose to display all sessions, if the checkbox 'Browse Log Sessions' is enabled. If the\n";
	s += "checkbox 'Browse All File Series' is checked and you choose my_service.log, all log\n";
	s += "files of the series beginning with my_service000.log up to the biggest number found,\n";
	s += "and ending with my_service.log, will be opened in the same view.\n";
	s += "You can add some filters. Finally, you may click a log line to display it in its\n";
	s += "entirety in the top field.\n";
	s += "Another way to open a file is to pass its filename as an argument. An alternative way to\n";
	s += "open one or more files is to drag & drop them onto the main window!.\n";
	s += "To actualize a file (which may have changed if a program is still writing into it), just\n";
	s += "click 'Filter...' and OK.\n";
	s += "Resizing a view is done by dragging its left border.\n\n";
	s += "Trace Mode: open several log files in Trace Format (see below) using the button\n";
	s += "'Add Trace View...', you can limit the lines loaded to the ones containing a\n";
	s += "specific service shortname (see below). Then click the button 'Compute Traces'\n";
	s += "to display the matching lines. The lines are sorted using their gamecycle and\n";
	s += "blank lines are filled so that different parallel views have the same timeline.\n";
	s += "Use the right scrollbar to scroll all the views at the same time.\n";
	s += "The logs in Trace Format should contains some lines that have a substring sTRACE:n:\n";
	s += "where s is an optional service name (e.g. FS) and n is the gamecycle of the action\n";
	s += "(an integer).";
	MessageBox( s );	
}

