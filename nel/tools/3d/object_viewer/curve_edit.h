#if !defined(AFX_CURVE_EDIT_H__7E8F583B_8E0D_4623_9128_B0FD23E1DC4A__INCLUDED_)
#define AFX_CURVE_EDIT_H__7E8F583B_8E0D_4623_9128_B0FD23E1DC4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// curve_edit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CurveEdit dialog

class CurveEdit : public CDialog
{
// Construction
public:
	CurveEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CurveEdit)
	enum { IDD = IDD_CURVE_EDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CurveEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CurveEdit)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	float _X, _Y, _Width, _Height; // position and size of the drawing window
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CURVE_EDIT_H__7E8F583B_8E0D_4623_9128_B0FD23E1DC4A__INCLUDED_)
