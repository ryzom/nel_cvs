// source_sounds_builderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "source_sounds_builder.h"
#include "source_sounds_builderDlg.h"

#include "nel/misc/file.h"
using namespace NLMISC;

#include <string>

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

	// Root
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("Sounds");
	m_Tree.InsertItem( &tvInsert );

	_SoundPage = new CSoundPage( this );
	_SoundPage->Create( IDD_SoundPage );
	_SoundPage->setTree( &m_Tree );

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
void CSource_sounds_builderDlg::OnAddSound() 
{
	_Sounds.push_back( new CSound() );
	HTREEITEM item = m_Tree.InsertItem( _T("New sound"), m_Tree.GetRootItem(), TVI_LAST );
	m_Tree.SetItemData( item, _Sounds.size()-1 );
	m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
	m_Tree.SelectItem( item );
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
		_SoundPage->getPropertiesFromSound();
		_SoundPage->ShowWindow( SW_SHOW );
		_SoundPage->SetFocus();
	}
	else
	{
		_SoundPage->ShowWindow( SW_HIDE );
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
	if ( pNMTreeView->itemNew.hItem != m_Tree.GetRootItem() )
	{
		uint32 index = m_Tree.GetItemData( pNMTreeView->itemOld.hItem );
		if ( index < _Sounds.size() )
		{
			_Sounds.erase( _Sounds.begin() + index );
		}
	}

	*pResult = 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnSave() 
{
	// Prompt filename
	CFileDialog savedlg( false, "nss", "sounds.nss", OFN_OVERWRITEPROMPT, "NeL Source Sounds (*.nss)|*.nss", this );
	if ( savedlg.DoModal()==IDOK ) // BUG: does not work in debug mode
	{
		// Save
		COFile file;
		file.open( string( savedlg.GetPathName() ), false );
		CSound::save( _Sounds, file );
		file.close();
	}
}


