#if !defined(AFX_PS_GLOBAL_COLOR_DLG_H__A0259E2D_877E_418E_B8A4_202615C5D141__INCLUDED_)
#define AFX_PS_GLOBAL_COLOR_DLG_H__A0259E2D_877E_418E_B8A4_202615C5D141__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "dialog_stack.h"
#include "ps_wrapper.h"

namespace NL3D
{
	class CParticleSystem;
}

struct IPopupNotify;

class CPSGlobalColorDlg : public CDialog, public CDialogStack
{
// Construction
public:
	CPSGlobalColorDlg(NL3D::CParticleSystem *ps, IPopupNotify *pn, CWnd* pParent = NULL);   // standard constructor

	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CPSGlobalColorDlg)
	enum { IDD = IDD_GLOBAL_COLOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPSGlobalColorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CParticleSystem *_PS;
	IPopupNotify		  *_PN;
	// Generated message map functions
	//{{AFX_MSG(CPSGlobalColorDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	struct CGlobalColorWrapper : public IPSSchemeWrapperRGBA
	{
		NL3D::CParticleSystem *PS;
		virtual scheme_type *getScheme(void) const;
		virtual void setScheme(scheme_type *s);
	}
	_GlobalColorWrapper;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PS_GLOBAL_COLOR_DLG_H__A0259E2D_877E_418E_B8A4_202615C5D141__INCLUDED_)
