#if !defined(AFX_CHOOSE_FRAME_DELAY_H__F07A2A7E_FB96_4856_99B5_46DE3B05CD6D__INCLUDED_)
#define AFX_CHOOSE_FRAME_DELAY_H__F07A2A7E_FB96_4856_99B5_46DE3B05CD6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_frame_delay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseLag dialog

class CChooseFrameDelay : public CDialog
{
// Construction
public:
	CChooseFrameDelay(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseLag)
	enum { IDD = IDD_CHOOSE_FRAME_DELAY };
	UINT	m_FrameDelay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseLag)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseLag)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_FRAME_DELAY_H__F07A2A7E_FB96_4856_99B5_46DE3B05CD6D__INCLUDED_)
