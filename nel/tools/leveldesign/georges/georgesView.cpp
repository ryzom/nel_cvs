// georgesView.cpp : implementation of the CGeorgesView class
//

#include "stdafx.h"
#include "georges.h"

#include "georgesDoc.h"
#include "georgesView.h"

#include "propertylist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView

IMPLEMENT_DYNCREATE(CGeorgesView, CFormView)

BEGIN_MESSAGE_MAP(CGeorgesView, CFormView)
	//{{AFX_MSG_MAP(CGeorgesView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_WM_CREATE()
	ON_WM_SIZE ()
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView construction/destruction

CGeorgesView::CGeorgesView() : CFormView(IDD_TOOLS)
{
	// TODO: add construction code here
	_List = NULL;
	pPipo1 = pPipo2 = pPipo3 = pPipo4 = pPipo5 = NULL;
}

CGeorgesView::~CGeorgesView()
{
	delete pPipo1;
	delete pPipo2;
	delete pPipo3;
	delete pPipo4;
	delete pPipo5;
}

BOOL CGeorgesView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView drawing

void CGeorgesView::OnDraw(CDC* pDC)
{
	CGeorgesDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

#define LIST_TOP 50
#define IDC_LIST 0x0010

/////////////////////////////////////////////////////////////////////////////

int CGeorgesView::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate (lpCreateStruct) == -1)
		return -1;
	CRect iniRect;
	GetClientRect(&iniRect);
	iniRect.top = LIST_TOP; iniRect.left = 10;
	iniRect.right -= 20; iniRect.bottom -= 20;
	_List = new CPropertyList;
	//_List = new CListBox;
	_List->Create (WS_CHILD|WS_VISIBLE|WS_BORDER|WS_HSCROLL|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_NOTIFY,
				iniRect, this, IDC_LIST);
	pPipo1 = new CPropertyItem("Entity Name","",PIT_EDIT,"");
	_List->AddPropItem(pPipo1);

	pPipo2 = new CPropertyItem("Visible","true",PIT_COMBO,"true|false|");
	_List->AddPropItem(pPipo2);

	pPipo3 = new CPropertyItem("Color","",PIT_COLOR,"");
	_List->AddPropItem(pPipo3);

	pPipo4 = new CPropertyItem("Refernce","",PIT_FILE,"");
	_List->AddPropItem(pPipo4);

	pPipo5 = new CPropertyItem("Child","one",PIT_COMBO,"one|two|three|four|five|six|");
	_List->AddPropItem(pPipo5);

	return 0;

}

/////////////////////////////////////////////////////////////////////////////

void CGeorgesView::OnSize (UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if (_List != NULL)
	{
		_List->MoveWindow (10, LIST_TOP, cx-20, cy-(LIST_TOP+10));
		_List->Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView diagnostics

#ifdef _DEBUG
void CGeorgesView::AssertValid() const
{
	CView::AssertValid();
}

void CGeorgesView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGeorgesDoc* CGeorgesView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesDoc)));
	return (CGeorgesDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView message handlers
