// attrib_list_box.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "attrib_list_box.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAttribListBox

CAttribListBox::CAttribListBox() : _DisplayValueFunc(NULL), _DisplayValueFuncParam(NULL)
{
}

CAttribListBox::~CAttribListBox()
{
}


BEGIN_MESSAGE_MAP(CAttribListBox, CListBox)
	//{{AFX_MSG_MAP(CAttribListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttribListBox message handlers

void CAttribListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	

	// TODO: Add your code to draw the specified item
	nlassert(_DisplayValueFunc) ; // setDrawer not called
	CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC) ;

	sint x = lpDrawItemStruct->rcItem.left, y = lpDrawItemStruct->rcItem.top ; 
	_DisplayValueFunc(dc, lpDrawItemStruct->itemID, x, y, _DisplayValueFuncParam) ;

	
					
		CBrush b ;
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			b.CreateSolidBrush(RGB(0,0,0)) ;
		}
		else
		{
			b.CreateSolidBrush(RGB(255,255,255)) ;
		}
		CGdiObject *oldObj = dc->SelectObject(&b) ;				
		RECT r ;
		r.top = y + 3 ; r.bottom = y + 36 ; r.left = x + 3 ; r.right = x + 60 ;
		dc->FrameRect(&r, &b) ;		
		dc->SelectObject(oldObj) ;
		b.DeleteObject() ;
	
}

void CAttribListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->CtlType = ODT_LISTBOX  ;
	lpMeasureItemStruct->CtlID = _Id ;
	lpMeasureItemStruct->itemWidth = 64 ;
	lpMeasureItemStruct->itemHeight = 40 ;		
}

int CAttribListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct) 
{
	// TODO: Add your code to determine the sorting order of the specified items
	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2
	
	return 0;
}
