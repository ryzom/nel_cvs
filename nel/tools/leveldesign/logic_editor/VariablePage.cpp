// VariablePage.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "VariablePage.h"

#include "ChildFrm.h"
#include "MainFrm.h"
#include "logic_editorDoc.h"

#include "EditorFormView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVariablePage property page

IMPLEMENT_DYNCREATE(CVariablePage, CPropertyPage)

CVariablePage::CVariablePage() : CPropertyPage(CVariablePage::IDD)
{
	//{{AFX_DATA_INIT(CVariablePage)
	m_sVarName = _T("");
	//}}AFX_DATA_INIT
}

CVariablePage::~CVariablePage()
{
}

void CVariablePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVariablePage)
	DDX_Control(pDX, IDC_LIST_VARIABLES, m_listVariables);
	DDX_Text(pDX, IDC_EDIT_VARIABLE_NAME, m_sVarName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVariablePage, CPropertyPage)
	//{{AFX_MSG_MAP(CVariablePage)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_LBN_SELCHANGE(IDC_LIST_VARIABLES, OnSelchangeListVariables)
	ON_BN_CLICKED(IDC_BUTTON_VAR_DELETE, OnButtonVarDelete)
	ON_BN_CLICKED(IDC_BUTTON_VAR_APPLY, OnButtonVarApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVariablePage message handlers

BOOL CVariablePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CVariablePage::OnButtonAdd() 
{
	UpdateData();	

	if ( m_sVarName.IsEmpty() )
		return;

	
	CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	// Get the active MDI child window.
	CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

	CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
	ASSERT_VALID(pDoc);	

	// check if a var or a counter with the same name already exist in the doc
	void *pointer;

	if ( ( pDoc->m_variables.Find(m_sVarName) != NULL) || ( pDoc->m_counters.Lookup(m_sVarName, pointer) != FALSE) )
		return;

	// add this var in the list and in the document
	m_listVariables.AddString( m_sVarName );
	pDoc->m_variables.AddTail( m_sVarName );

	CEditorFormView *formView = static_cast<CEditorFormView *> (pChild->m_wndSplitter.GetPane(0,1));
//	formView->m_pPropertySheet->Update();

	UpdateData(FALSE);
}



void CVariablePage::OnSelchangeListVariables() 
{
	const int sel = m_listVariables.GetCurSel();

	// set the text in the edit box
	m_listVariables.GetText( sel, m_sVarName );
	UpdateData(FALSE);
}

void CVariablePage::OnButtonVarDelete() 
{
	// get the selected variable if any
	const int sel = m_listVariables.GetCurSel();

	if (sel != LB_ERR)
	{
		CString txt;
		m_listVariables.GetText( sel, txt );

		m_listVariables.DeleteString( sel );

		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);	

		// delete var in doc
		pDoc->deleteVar( txt );
	}
	else
	{
		AfxMessageBox( "No variable selected ! Choose a variable first" );
	}
	
}

void CVariablePage::OnButtonVarApply() 
{
	UpdateData();

	// get the selected variable if any
	const int sel = m_listVariables.GetCurSel();

	if (sel != LB_ERR)
	{
		CString txt;
		m_listVariables.GetText( sel, txt );

		CMainFrame *pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		// Get the active MDI child window.
		CChildFrame *pChild = (CChildFrame *) pFrame->GetActiveFrame();

		CLogic_editorDoc *pDoc = static_cast<CLogic_editorDoc *> (pChild->GetActiveDocument());
		ASSERT_VALID(pDoc);	

		// delete var in doc
		if ( pDoc->changeVarName( txt, m_sVarName ) == TRUE)
		{
			m_listVariables.DeleteString( sel );
			m_listVariables.AddString( m_sVarName );
		}
	}
	else
	{
		AfxMessageBox( "No variable selected ! Choose a variable first" );
	}
	
}
