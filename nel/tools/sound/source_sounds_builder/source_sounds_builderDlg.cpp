// source_sounds_builderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "source_sounds_builder.h"
#include "source_sounds_builderDlg.h"

#include "nel/misc/file.h"
using namespace NLMISC;

#include <string>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderDlg dialog

CSource_sounds_builderDlg::CSource_sounds_builderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSource_sounds_builderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSource_sounds_builderDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSource_sounds_builderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSource_sounds_builderDlg)
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSource_sounds_builderDlg, CDialog)
	//{{AFX_MSG_MAP(CSource_sounds_builderDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_AddSound, OnAddSound)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_Save, OnSave)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE1, OnDeleteitemTree1)
	ON_BN_CLICKED(IDC_Load, OnLoad)
	ON_BN_CLICKED(IDC_MoveUp, OnMoveUp)
	ON_BN_CLICKED(IDC_MoveDown, OnMoveDown)
	ON_BN_CLICKED(IDC_Import, OnImport)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE1, OnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, OnEndlabeleditTree1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderDlg message handlers

BOOL CSource_sounds_builderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	/*
	 * Init
	 */

	_Modified = false;

	ResetTree();

	_SoundPage = new CSoundPage( this );
	_SoundPage->setTree( &m_Tree );
	_SoundPage->Create( IDD_SoundPage );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSource_sounds_builderDlg::OnPaint() 
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

HCURSOR CSource_sounds_builderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/*
 *
 */
void CSource_sounds_builderDlg::ResetTree()
{
	m_Tree.DeleteAllItems();

	// Root
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("Sounds");
	m_Tree.InsertItem( &tvInsert );
}


/*
 *
 */
void CSource_sounds_builderDlg::OnAddSound() 
{
	AddSound( "<New Sound>" );
}


/*
 *
 */
void CSource_sounds_builderDlg::AddSound( const char *name )
{
	_Sounds.push_back( new CSound() );
	HTREEITEM item = m_Tree.InsertItem( name, m_Tree.GetRootItem(), TVI_LAST );
	m_Tree.SetItemData( item, _Sounds.size()-1 );
	m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
	m_Tree.EditLabel( item );
}


/*
 *
 */
void CSource_sounds_builderDlg::OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	CString s = m_Tree.GetItemText( pTVDispInfo->item.hItem );
	if ( (s != "") && (s[0] == '<' ) )
	{
		// Sound added by the user
		m_Tree.SelectItem( pTVDispInfo->item.hItem );
		GetDlgItem( IDC_AddSound )->EnableWindow( false );
		*pResult = 0;
	}
	else
	{
		*pResult = 1;
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if ( (pTVDispInfo->item.pszText != NULL) && (pTVDispInfo->item.pszText[0] != '\0') )
	{
		// Changed
		CString s;
		s.Format( "%s*", pTVDispInfo->item.pszText );
		m_Tree.SetItemText( pTVDispInfo->item.hItem, s );
	}
	else
	{
		// Cancelled
		m_Tree.SetItemText( pTVDispInfo->item.hItem, "<New Sound>*" );
	}

	GetDlgItem( IDC_AddSound )->EnableWindow( true );
	*pResult = 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if ( (pNMTreeView->itemNew.hItem != NULL) && (pNMTreeView->itemNew.hItem != m_Tree.GetRootItem()) )
	{
		uint32 index = m_Tree.GetItemData( pNMTreeView->itemNew.hItem );
		nlassert( index < _Sounds.size() );
		_SoundPage->setCurrentSound( _Sounds[index], pNMTreeView->itemNew.hItem );
		_SoundPage->ShowWindow( SW_SHOW );
		((CButton*)GetDlgItem( IDC_Save ))->EnableWindow( false );
		_SoundPage->SetFocus();
		_SoundPage->getPropertiesFromSound();
	}
	else
	{
		_SoundPage->ShowWindow( SW_HIDE );
		_SoundPage->setCurrentSound( NULL, NULL );
		((CButton*)GetDlgItem( IDC_Save ))->EnableWindow( true );
	}

	*pResult = 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnDeleteitemTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	nlassert( pNMTreeView );
	if ( pNMTreeView->itemOld.hItem != m_Tree.GetRootItem() )
	{
		uint32 index = m_Tree.GetItemData( pNMTreeView->itemOld.hItem );
		if ( index < _Sounds.size() )
		{
			_Sounds.erase( _Sounds.begin() + index );
		}

		// Renumber
		HTREEITEM hitem = m_Tree.GetChildItem( m_Tree.GetRootItem() );
		for ( index=0; index!=_Sounds.size(); index++ )
		{
			if ( hitem == pNMTreeView->itemOld.hItem )
			{
				hitem = m_Tree.GetNextItem( hitem, TVGN_NEXT );
			}
			m_Tree.SetItemData( hitem, index );
			hitem = m_Tree.GetNextItem( hitem, TVGN_NEXT );
		}

		_Modified = true;
	}

	*pResult = 0;
}



/*
 *
 */
void CSource_sounds_builderDlg::OnMoveUp() 
{
	HTREEITEM hitem = m_Tree.GetSelectedItem();
	if ( (hitem != NULL) && (hitem != m_Tree.GetRootItem()) )
	{
		uint32 oldindex = m_Tree.GetItemData( hitem );
		uint32 newindex = oldindex - 1;
		if ( oldindex > 0 )
		{
			CSound *snd = _Sounds[oldindex];
			_Sounds[oldindex] = _Sounds[newindex];
			_Sounds[newindex] = snd;
			m_Tree.SetItemText( hitem, _Sounds[oldindex]->getFilename().c_str() );
			hitem = m_Tree.GetPrevSiblingItem( hitem );
			m_Tree.SetItemText( hitem, _Sounds[newindex]->getFilename().c_str() );
			m_Tree.SelectItem( hitem );
			_Modified = true;
		}
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnMoveDown() 
{
	HTREEITEM hitem = m_Tree.GetSelectedItem();
	if ( (hitem != NULL) && (hitem != m_Tree.GetRootItem()) )
	{
		uint32 oldindex = m_Tree.GetItemData( hitem );
		uint32 newindex = oldindex + 1;
		if ( oldindex < _Sounds.size()-1 )
		{
			CSound *snd = _Sounds[oldindex];
			_Sounds[oldindex] = _Sounds[newindex];
			_Sounds[newindex] = snd;
			m_Tree.SetItemText( hitem, _Sounds[oldindex]->getFilename().c_str() );
			hitem = m_Tree.GetNextSiblingItem( hitem );
			m_Tree.SetItemText( hitem, _Sounds[newindex]->getFilename().c_str() );
			m_Tree.SelectItem( hitem );
			_Modified = true;
		}
	}
}


/*
 *
 */
CString CSource_sounds_builderDlg::SoundName( HTREEITEM hitem )
{
	CString s = m_Tree.GetItemText( hitem );
	uint last;
	if ( s[s.GetLength()-1] == '*' )
	{
		last = s.GetLength()-2;
	}
	else
	{
		last = s.ReverseFind( '(' )-2;
	}
	return s.Left( last+1 );
}


/*
 *
 */
void CSource_sounds_builderDlg::OnSave() 
{
	// Prompt filename
	CFileDialog savedlg( false, "nss", "sounds.nss", OFN_OVERWRITEPROMPT, "NeL Source Sounds (*.nss)|*.nss||", this );
	if ( savedlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		// Save
		COFile file;
		file.open( string( savedlg.GetPathName() ), false );
		CSound::save( _Sounds, file );
		file.close();

		_Modified = false;

		waitcursor.Restore();
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnLoad() 
{
	// Prompt filename
	CFileDialog opendlg( true, "nss", "", OFN_HIDEREADONLY, "NeL Source Sounds (*.nss)|*.nss||", this );
	if ( opendlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		// Clear tree and sound vector
		ResetTree();
		_SoundPage->ShowWindow( SW_HIDE );
		((CButton*)GetDlgItem( IDC_Save ))->EnableWindow( true );
		_Sounds.clear();

		// Load
		CIFile file;
		file.open( string( opendlg.GetPathName() ), false );
		TSoundMap soundmap;
		CSound::load( soundmap, file );
		TSoundMap::iterator ipsnds;
		for ( ipsnds=soundmap.begin(); ipsnds!=soundmap.end(); ++ipsnds )
		{
			_Sounds.push_back( (*ipsnds).second );
		}

		file.close();

		// Update tree
		uint32 i;
		for ( i=0; i!=_Sounds.size(); i++ )
		{
			CString s;
			s.Format( "%s (%s)", _Sounds[i]->getName().c_str(), _Sounds[i]->getFilename().c_str() );
			HTREEITEM item = m_Tree.InsertItem( s, m_Tree.GetRootItem(), TVI_LAST );
			m_Tree.SetItemData( item, i );
		}
		m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );

		_Modified = false;

		waitcursor.Restore();
	}
}


/*
 *
 */
HTREEITEM CSource_sounds_builderDlg::FindInTree( char *name )
{
	HTREEITEM hitem = m_Tree.GetChildItem( m_Tree.GetRootItem() );
	while ( hitem != NULL )
	{
		if ( SoundName( hitem ) == CString(name) )
		{
			return hitem;
		}
		hitem = m_Tree.GetNextItem( hitem, TVGN_NEXT );
	}
	return NULL;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnImport() 
{
	// Prompt filename
	CFileDialog opendlg( true, "nsn", "", OFN_HIDEREADONLY, "NeL Sounds Names (*.nsn; *.txt)|*.nsn; *.txt||", this );
	if ( opendlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		char name [80];
		ifstream fs;
		fs.open( opendlg.GetPathName() );
		while ( ! fs.eof() )
		{
			fs.getline( name, 40 );

			// Add new name if not already existing (useful for new versions of the names file)
			HTREEITEM hitem = FindInTree( name );
			if ( hitem == NULL )
			{
				AddSound( (string(name)+string("*")).c_str() );
			}

			// Note1: does not check if some names have been removed
			// Note2: does not check if there is twice the same name
		}
		fs.close();
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnOK()
{
	// Nothing: disable closure by Enter
}


/*
 *
 */
void CSource_sounds_builderDlg::OnCancel()
{
	// Called when exiting (Esc, Alt+F4, etc.)
	
	if ( ! _Modified )
	{
		CDialog::OnCancel();
	}
	else
	{
		switch ( AfxMessageBox( "Save before exiting ?", MB_YESNOCANCEL | MB_ICONQUESTION ) )
		{
		// no break;
		case IDYES:
			OnSave();
		case IDNO:
			CDialog::OnCancel();
		}
	}
}
