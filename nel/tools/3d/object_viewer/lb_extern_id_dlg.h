#if !defined(AFX_LB_EXTERN_ID_DLG_H__A045B811_4473_4F2B_A27E_580B4407837C__INCLUDED_)
#define AFX_LB_EXTERN_ID_DLG_H__A045B811_4473_4F2B_A27E_580B4407837C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSLocatedBindable;
}

/////////////////////////////////////////////////////////////////////////////
// CLBExternIDDlg dialog

class CLBExternIDDlg : public CDialog
{
// Construction
public:
	CLBExternIDDlg(uint32 id, CWnd* pParent = NULL);   // standard constructor

	uint32 getNewID(void) const { return _ID; } // the id after edition by this dialog
// Dialog Data
	//{{AFX_DATA(CLBExternIDDlg)
	enum { IDD = IDD_LB_EXTERN_IB };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLBExternIDDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:	
	uint32					  _ID; // the current ID

	// Generated message map functions
	//{{AFX_MSG(CLBExternIDDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableExternId();
	afx_msg void OnChangeIdValue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LB_EXTERN_ID_DLG_H__A045B811_4473_4F2B_A27E_580B4407837C__INCLUDED_)
