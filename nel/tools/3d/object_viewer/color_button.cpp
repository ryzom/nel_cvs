// color_button.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "color_button.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


WNDPROC CColorButton::_BasicButtonWndProc = NULL  ;


/////////////////////////////////////////////////////////////////////////////
// CColorButton

CColorButton::CColorButton() : _Color(CRGBA::White)
{		
}

CColorButton::~CColorButton()
{
}


BEGIN_MESSAGE_MAP(CColorButton, CButton)
	//{{AFX_MSG_MAP(CColorButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorButton message handlers

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	HDC dc = lpDrawItemStruct->hDC ;
	
	RECT r ;

	GetClientRect(&r) ;

	r.left += 4 ;
	r.top +=  4 ;
	r.bottom -= 4 ;
	r.right -= 4 ;
	GetClientRect(&r) ;
	CBrush b ;
	b.CreateSolidBrush(RGB(_Color.R, _Color.G, _Color.B)) ;
	::FillRect(dc, &r, (HBRUSH) b) ;	


}
