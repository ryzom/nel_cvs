#if !defined(AFX_LOCATED_TARGET_DLG_H__FA197835_AE71_4057_88A4_48F28A01E367__INCLUDED_)
#define AFX_LOCATED_TARGET_DLG_H__FA197835_AE71_4057_88A4_48F28A01E367__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// located_target_dlg.h : header file
//

#include "3d/ps_located.h"

/////////////////////////////////////////////////////////////////////////////
// CLocatedTargetDlg dialog

class CLocatedTargetDlg : public CDialog
{
// Construction
public:
	CLocatedTargetDlg(NL3D::CPSTargetLocatedBindable *blTarget);   // standard constructor

	// init the dialog with the given parent
	void init(CWnd* pParent) ;

// Dialog Data
	//{{AFX_DATA(CLocatedTargetDlg)
	enum { IDD = IDD_LOCATED_TARGET_DLG };
	CListBox	m_AvailableTargets;
	CListBox	m_Targets;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedTargetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the target we're focusing on
	NL3D::CPSTargetLocatedBindable *_LBTarget ;

	// Generated message map functions
	//{{AFX_MSG(CLocatedTargetDlg)
	afx_msg void OnAddTarget();
	afx_msg void OnRemoveTarget();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_TARGET_DLG_H__FA197835_AE71_4057_88A4_48F28A01E367__INCLUDED_)
