// data_mirrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "data_mirror.h"
#include "data_mirrorDlg.h"

using namespace std;
using namespace NLMISC;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorDlg dialog

CData_mirrorDlg::CData_mirrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CData_mirrorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CData_mirrorDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ButtonYFromBottom = 0;
}

void CData_mirrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CData_mirrorDlg)
	DDX_Control(pDX, IDC_REMOVED_LABEL, RemovedLabelCtrl);
	DDX_Control(pDX, IDC_MODIFIED_LABEL, ModifiedLabelCtrl);
	DDX_Control(pDX, IDC_ADDED_LABEL, AddedLabelCtrl);
	DDX_Control(pDX, IDCANCEL, CancelCtrl);
	DDX_Control(pDX, IDIGNORE, IgnoreCtrl);
	DDX_Control(pDX, IDOK, OkCtrl);
	DDX_Control(pDX, IDC_REMOVED, RemovedList);
	DDX_Control(pDX, IDC_ADDED, AddedList);
	DDX_Control(pDX, IDC_MODIFIED, ModifiedList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CData_mirrorDlg, CDialog)
	//{{AFX_MSG_MAP(CData_mirrorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDIGNORE, OnIgnore)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_MODIFIED, OnClickModified)
	ON_NOTIFY(NM_CLICK, IDC_REMOVED, OnClickRemoved)
	ON_NOTIFY(NM_CLICK, IDC_ADDED, OnClickAdded)
	ON_NOTIFY(NM_DBLCLK, IDC_MODIFIED, OnClickModified)
	ON_NOTIFY(NM_DBLCLK, IDC_REMOVED, OnClickRemoved)
	ON_NOTIFY(NM_DBLCLK, IDC_ADDED, OnClickAdded)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorDlg message handlers

BOOL CData_mirrorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Init position remainder
	RECT client;
	RECT childClient;
	RECT childClient2;
	GetClientRect (&client);
	OkCtrl.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	ButtonYFromBottom = client.bottom - childClient.top;
	OkButtonXFromRight = client.right - childClient.left;
	CancelCtrl.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	CancelButtonXFromRight = client.right - childClient.left;
	IgnoreCtrl.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	IgnorButtonXFromRight = client.right - childClient.left;
	ModifiedList.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	ListBottomFromBottom = client.bottom - childClient.bottom;
	
	ModifiedList.GetWindowRect (&childClient);
	AddedList.GetWindowRect (&childClient2);
	SpaceBetweenList = childClient2.left - childClient.right;
		
	// Init list
	ListView_SetExtendedListViewStyle (ModifiedList, LVS_EX_CHECKBOXES);
	ListView_SetExtendedListViewStyle (AddedList, LVS_EX_CHECKBOXES);
	ListView_SetExtendedListViewStyle (RemovedList, LVS_EX_CHECKBOXES);
	ModifiedList.InsertColumn (0, "");
	AddedList.InsertColumn (0, "");
	RemovedList.InsertColumn (0, "");

	resize ();
	refresh ();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CData_mirrorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CData_mirrorDlg::OnPaint() 
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
HCURSOR CData_mirrorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CData_mirrorDlg::OnIgnore() 
{
	ignore (ModifiedList);
	ignore (AddedList);
	ignore (RemovedList);
}

void CData_mirrorDlg::ignore (CListCtrl &_list)
{
	// Update files
	uint count = _list.GetItemCount ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Checked ?
		if (ListView_GetCheckState (_list, i))
		{
			// Get the file
			CString itemText = _list.GetItemText (i, 0);

			// Add to ignore list
			IgnoreFiles.insert ((const char*)itemText);

			// Remove the item
			_list.DeleteItem (i);
			i--;
			count--;
		}
	}
}

void createDirectory (const string &dir)
{
	string temp = dir;
	temp = temp.substr (0, temp.size ()-1);
	temp = NLMISC::CFile::getPath (temp);
	if (!temp.empty ())
		createDirectory (temp);
	NLMISC::CFile::createDirectory (dir);
}

void CData_mirrorDlg::OnOK() 
{
	bool success = true;
	// Update files
	uint count = ModifiedList.GetItemCount ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Checked ?
		if (ListView_GetCheckState (ModifiedList, i))
		{
			// Copy it
			CString itemText = ModifiedList.GetItemText (i, 0);
			string source = MainDirectory+(const char*)itemText;
			string dest = MirrorDirectory+(const char*)itemText;
			string directory = NLMISC::CFile::getPath (dest);
			createDirectory (directory.c_str ());
			if (!CopyFile (source.c_str (), dest.c_str (), FALSE))
			{
				MessageBox (("Can't copy file "+source+" in file "+dest).c_str (), "NeL Data Mirror", 
					MB_OK|MB_ICONEXCLAMATION);
				success = false;
			}
		}
	}

	count = AddedList.GetItemCount ();
	for (i=0; i<count; i++)
	{
		// Checked ?
		if (ListView_GetCheckState (AddedList, i))
		{
			// Copy it
			CString itemText = AddedList.GetItemText (i, 0);
			string source = MainDirectory+(const char*)itemText;
			string dest = MirrorDirectory+(const char*)itemText;
			string directory = NLMISC::CFile::getPath (dest);
			createDirectory (directory.c_str ());
			if (!CopyFile (source.c_str (), dest.c_str (), FALSE))
			{
				MessageBox (("Can't copy file "+source+" in file "+dest).c_str (), "NeL Data Mirror", 
					MB_OK|MB_ICONEXCLAMATION);
				success = false;
			}
		}
	}

	count = RemovedList.GetItemCount ();
	for (i=0; i<count; i++)
	{
		// Checked ?
		if (ListView_GetCheckState (RemovedList, i))
		{
			// Copy it
			CString itemText = RemovedList.GetItemText (i, 0);
			string dest = MirrorDirectory+(const char*)itemText;
			if (!DeleteFile (dest.c_str ()))
			{
				MessageBox (("Can't delete the file "+dest).c_str (), "NeL Data Mirror", 
					MB_OK|MB_ICONEXCLAMATION);
				success = false;
			}
		}
	}

	if (success)
	{
		FILE *file = fopen ((MainDirectory+"ignore_list.txt").c_str (), "w");
		if (file)
		{
			// Save ignore list
			std::set<string>::iterator ite = IgnoreFiles.begin ();
			while (ite != IgnoreFiles.end ())
			{
				fputs ((*ite + "\n").c_str (), file);		

				ite++;
			}
		}

		CDialog::OnOK();
	}
}

void CData_mirrorDlg::resize () 
{
	// Initialised ?
	if (ButtonYFromBottom)
	{
		RECT client;
		RECT child;
		GetClientRect (&client);
		OkCtrl.SetWindowPos (NULL, client.right - OkButtonXFromRight, client.bottom - ButtonYFromBottom, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		CancelCtrl.SetWindowPos (NULL, client.right - CancelButtonXFromRight, client.bottom - ButtonYFromBottom, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		IgnoreCtrl.SetWindowPos (NULL, client.right - IgnorButtonXFromRight, client.bottom - ButtonYFromBottom, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		int labelHeight;
		ModifiedLabelCtrl.GetWindowRect (&child);
		ScreenToClient (&child);
		labelHeight = child.bottom - child.top;
		int posYLabel = child.top;

		int listHeight = client.bottom - ListBottomFromBottom;
		ModifiedList.GetWindowRect (&child);
		ScreenToClient (&child);
		int posY = child.top;
		listHeight -= posY;
		
		sint border = (child.left - client.left);
		sint listWidth = ((client.right - client.left) - border*2 - SpaceBetweenList * 2)/3;
		ModifiedList.SetWindowPos (NULL, 0, 0, listWidth, listHeight, SWP_NOZORDER|SWP_NOMOVE);
		ModifiedList.SetColumnWidth (0, listWidth);
		ModifiedLabelCtrl.SetWindowPos (NULL, 0, 0, listWidth, labelHeight, SWP_NOZORDER|SWP_NOMOVE);
		AddedList.GetWindowRect (&child);
		ScreenToClient (&child);
		AddedList.SetWindowPos (NULL, border+listWidth+SpaceBetweenList, posY, listWidth, listHeight,
			SWP_NOZORDER);
		AddedList.SetColumnWidth (0, listWidth);
		AddedLabelCtrl.SetWindowPos (NULL, border+listWidth+SpaceBetweenList, posYLabel, listWidth, labelHeight,
			SWP_NOZORDER);
		RemovedList.SetWindowPos (NULL, border+2*listWidth+2*SpaceBetweenList, posY, listWidth, listHeight,
			SWP_NOZORDER);
		RemovedList.SetColumnWidth (0, listWidth);
		RemovedLabelCtrl.SetWindowPos (NULL, border+2*listWidth+2*SpaceBetweenList, posYLabel, listWidth, labelHeight,
			SWP_NOZORDER);

		Invalidate ();
	}
}

void CData_mirrorDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	resize ();
}

void CData_mirrorDlg::refresh ()
{
	UpdateData (FALSE);
	
	ModifiedList.DeleteAllItems ();
	AddedList.DeleteAllItems ();
	RemovedList.DeleteAllItems ();

	// List all files in database
	vector<string> fileSource;
	CPath::getPathContent (MainDirectory+CurrentDir, true, false, true, fileSource);
	uint i;
	const uint count = fileSource.size ();
	for (i=0; i<count; i++)
	{
		// Get the filename
		string &str = fileSource[i];
		str = strlwr (str);
		str = str.substr (MainDirectory.size (), str.size ());

		// In the ignore list ?
		if (IgnoreFiles.find (str) == IgnoreFiles.end () && (str != "ignore_list.txt"))
		{
			// Does the destination exist ?
			string mirrorFile = MirrorDirectory+str;
			string mainFile = MainDirectory+str;
			if (NLMISC::CFile::fileExists (mirrorFile))
			{
				uint time0 = NLMISC::CFile::getFileModificationDate (mirrorFile);
				uint time1 = NLMISC::CFile::getFileModificationDate (mainFile);
				if ( abs ((sint)time0 - (sint)time1) > 1 )
				{
					ModifiedList.InsertItem (0, str.c_str ());
				}
				else
				{
				}
			}
			else
			{
				AddedList.InsertItem (0, str.c_str ());
			}
		}
	}

	// List all files in mirror
	fileSource.clear ();
	CPath::getPathContent (MirrorDirectory+CurrentDir, true, false, true, fileSource);
	const uint count2 = fileSource.size ();
	for (i=0; i<count2; i++)
	{
		// Get the filename
		string &str = fileSource[i];
		str = strlwr (str);
		str = str.substr (MirrorDirectory.size (), str.size ());

		// In the ignore list ?
		if (IgnoreFiles.find (str) == IgnoreFiles.end () && (str != "ignore_list.txt"))
		{
			// Does the destination exist ?
			string mirrorFile = MirrorDirectory+str;
			string mainFile = MainDirectory+str;
			if (!NLMISC::CFile::fileExists (mainFile))
			{
				RemovedList.InsertItem (0, str.c_str ());
			}
		}
	}

	// Sort
	/*ModifiedList.SortItems ();
	AddedList.SortItems ();
	RemovedList.SortItems ();*/

	UpdateData (TRUE);
}

void CData_mirrorDlg::OnClickModified(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pNMHDR;
	onClickList (lpnmlv, ModifiedList);
	*pResult = 0;
}

void CData_mirrorDlg::OnClickRemoved(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pNMHDR;
	onClickList (lpnmlv, RemovedList);
	*pResult = 0;
}

void CData_mirrorDlg::OnClickAdded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pNMHDR;
	onClickList (lpnmlv, AddedList);
	*pResult = 0;
}

void CData_mirrorDlg::onClickList (LPNMLISTVIEW lpnmlv, CListCtrl &_list) 
{
	UpdateData (FALSE);
	UINT flags;
	_list.HitTest (lpnmlv->ptAction, &flags);

	// Get check button state
	BOOL state = ListView_GetCheckState (_list, lpnmlv->iItem);
	if (_list.GetItemState (lpnmlv->iItem, LVIS_SELECTED) == LVIS_SELECTED)
	if (flags & LVHT_ONITEMSTATEICON)
	{
		POSITION pos = _list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = _list.GetNextSelectedItem(pos);
			if (nItem != lpnmlv->iItem)
				ListView_SetCheckState (_list, nItem, !state);
		}
	}
	UpdateData (TRUE);
}

