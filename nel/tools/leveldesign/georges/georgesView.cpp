// GeorgesView.cpp : implementation of the CGeorgesView class
//

#include "stdafx.h"
#include "Georges.h"

#include "GeorgesDoc.h"
#include "GeorgesView.h"
#include "../georges_lib/Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView

IMPLEMENT_DYNCREATE(CGeorgesView, CListViewEx)

BEGIN_MESSAGE_MAP(CGeorgesView, CListViewEx)
	//{{AFX_MSG_MAP(CGeorgesView)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView construction/destruction

CGeorgesView::CGeorgesView()
{
}

CGeorgesView::~CGeorgesView()
{
}

BOOL CGeorgesView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL | LVS_EDITLABELS | LVS_NOSORTHEADER;
	return CListViewEx::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesView drawing

void CGeorgesView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	m_clrTextBk = ::GetSysColor(COLOR_WINDOW);
	m_clrText = ::GetSysColor(COLOR_WINDOWTEXT);
//		m_clrText = RGB( 128, 128, 128 );
	CListViewEx::DrawItem(lpDrawItemStruct);
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

void CGeorgesView::OnInitialUpdate() 
{
	CListViewEx::OnInitialUpdate();

	LV_COLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

	lvc.iSubItem = 0;
	lvc.pszText = _T("Name");
	lvc.cx = 350;
	GetListCtrl().InsertColumn(0,&lvc);

	lvc.iSubItem = 1;
	lvc.pszText = _T("Result");
	lvc.cx = 180;
	GetListCtrl().InsertColumn(1,&lvc);
	
	lvc.iSubItem = 2;
	lvc.pszText = _T("Value");
	lvc.cx = 180;
	GetListCtrl().InsertColumn(2,&lvc);
	
	lvc.iSubItem = 3;
	lvc.pszText = _T("Type");
	lvc.cx = 180;
	GetListCtrl().InsertColumn(3,&lvc);

	GetListCtrl().SetExtendedStyle( GetListCtrl().GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE  );
}

BOOL CGeorgesView::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	if(message == WM_NOTIFY)
	{
		NMHDR* phdr = (NMHDR*)lParam;

		switch(phdr->code)
		{
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO* pLvdi = (NMLVDISPINFO*)lParam;
				GetDispInfo(&pLvdi->item);
			}
			if(pLResult != NULL)
				*pLResult = 0;
			break;
		case LVN_ODCACHEHINT:
			if(pLResult != NULL)
				*pLResult = 0;
			break;
		case LVN_ODFINDITEM:
			if(pLResult != NULL)
				*pLResult = -1;
			break;
		case LVN_ENDLABELEDIT:
			{
				NMLVDISPINFO* pdi = (NMLVDISPINFO*) lParam;
				LV_ITEM	*pItem = &pdi->item;
				if (pItem->pszText != NULL)
				{
					int r = pItem->iSubItem;
					GetDocument()->SetItemValue( pItem->iItem, pItem->pszText );  
				}
				GetListCtrl().RedrawItems( ySubItem, ySubItem );
				*pLResult = FALSE;
				break;
			}
		default:
			return CListViewEx::OnChildNotify(message, wParam, lParam, pLResult);
		}
	}
	else
		return CListViewEx::OnChildNotify(message, wParam, lParam, pLResult);

	return TRUE;
}


void CGeorgesView::GetDispInfo(LVITEM* const pItem )
{
	unsigned int inblines = GetDocument()->GetItemNbElt();				
	if( ( pItem->mask & LVIF_TEXT )&&( pItem->iItem < ( int )( inblines ) ) )
		switch(pItem->iSubItem)
		{
		case 0:
			lstrcpy( pItem->pszText, GetDocument()->GetItemName( pItem->iItem ) );
			break;
		case 1:
			lstrcpy( pItem->pszText, GetDocument()->GetItemCurrentResult( pItem->iItem ) );
			break;
		case 2:
			lstrcpy( pItem->pszText, GetDocument()->GetItemCurrentValue( pItem->iItem ) );
			break;
		case 3:
			lstrcpy( pItem->pszText, GetDocument()->GetItemFormula( pItem->iItem ) );
			break;
		}
}

void CGeorgesView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	GetListCtrl().SetItemCountEx( GetDocument()->GetItemNbElt(), 0);				
	CListViewEx::OnUpdate( pSender, lHint, pHint);
}

void CGeorgesView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	LVHITTESTINFO lvhti;
	lvhti.pt = point;
	if( GetListCtrl().SubItemHitTest(&lvhti) == -1 )
		return;
}

void CGeorgesView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	LVHITTESTINFO lvhti;
	lvhti.pt = point;
	if( GetListCtrl().SubItemHitTest(&lvhti) == -1 )
		return;
	if( ( ( lvhti.iSubItem == 2 )&&( GetDocument()->GetItemInfos( lvhti.iItem ) & ITEM_ISATOM ) ) )
	{
		unsigned int oldy = ySubItem;
		ySubItem = lvhti.iItem;
		xSubItem = lvhti.iSubItem;

		GetListCtrl().SetItemState( ySubItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED ); 
		if( oldy != ySubItem )
			GetListCtrl().RedrawItems( oldy, oldy ); 
		GetListCtrl().RedrawItems( ySubItem, ySubItem ); 
		CListViewEx::OnLButtonDown(nFlags, point);
	}
}
