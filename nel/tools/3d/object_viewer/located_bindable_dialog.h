#if !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
#define AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSLocatedBindable ;
}





// located_bindable_dialog.h : header file
//




/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog

class CLocatedBindableDialog : public CDialog
{
// Construction
public:
	// create this dialog to edit the given bindable
	CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable);   // standard constructor


	// dtor
	~CLocatedBindableDialog() ;

	// init the dialog as a child of the given wnd
	void init(CWnd* pParent = NULL) ;
// Dialog Data
	//{{AFX_DATA(CLocatedBindableDialog)
	enum { IDD = IDD_LOCATED_BINDABLE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedBindableDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL




// Implementation
protected:

	NL3D::CPSLocatedBindable *_Bindable ;

	// list of the dialogs that we instanciated
	std::vector<CDialog *> _SubDialogs ;

	// Generated message map functions
	//{{AFX_MSG(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
