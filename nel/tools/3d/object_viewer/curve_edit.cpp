// curve_edit.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "curve_edit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CurveEdit dialog


CurveEdit::CurveEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CurveEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CurveEdit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CurveEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CurveEdit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CurveEdit, CDialog)
	//{{AFX_MSG_MAP(CurveEdit)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CurveEdit message handlers

void CurveEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnMouseMove(nFlags, point);
}

void CurveEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CurveEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CurveEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}


/*
void CurveEdit::drawBackGround(CDC &dc)
{

}


void CurveEdit::drawCurve(CDC &dc)
{

}
*/
