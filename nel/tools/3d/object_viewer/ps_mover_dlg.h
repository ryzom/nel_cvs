#if !defined(AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_)
#define AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ps_mover_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg dialog

class CPSMoverDlg : public CDialog
{
// Construction
public:
	// construct the object with a pointer to the tree ctrl, and to the item that is being edited
	CPSMoverDlg(class CParticleTreeCtrl *parent, HTREEITEM editedNode);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPSMoverDlg)
	enum { IDD = IDD_PS_MOVER };
	CListBox	m_SubComponentCtrl;
	float	m_X;
	float	m_Y;
	float	m_Z;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPSMoverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	// position has to be updated (for mouse edition)
	void updatePosition(void) ;


	void init(void) ;


// Implementation
protected:

	CParticleTreeCtrl *_TreeCtrl ;
	CParticleTreeCtrl::CNodeType *_EditedNode ;


	// Generated message map functions
	//{{AFX_MSG(CPSMoverDlg)
	afx_msg void OnUpdateXpos();
	afx_msg void OnUpdateYpos();
	afx_msg void OnUpdateZpos();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSubComponent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_)
