#if !defined(AFX_VALUEBLENDERDLG_H__4242C860_C538_45BD_8348_C6DF314D688A__INCLUDED_)
#define AFX_VALUEBLENDERDLG_H__4242C860_C538_45BD_8348_C6DF314D688A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ValueBlenderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CValueBlenderDlg dialog : allow to choose 2 values of a given type


class CEditAttribDlg ;

class CValueBlenderDlg : public CDialog
{
// Construction
public:


	/** a pointer over a function that create a dialog of the right type, which must have CEditAttribDlg as a base class
	 *  \param index must be 0 or 1, it says which value is being edited
	 *  \infoToDelete the callback can set the pointer  to NULL or to something that must be deleted when the dialog is
	 */
	typedef CEditAttribDlg *(* TCreateDialog)(uint index, void *lParam, void **infoToDelete) ;

	/** Create the dialog. Then, it will call createFunc (with the user param lParam) to generate the appropriate dialogs
	 *  If mustDelete is set, then new will be performed on lParam after it has been used
	 */

	CValueBlenderDlg(TCreateDialog createFunc, void *lParam, CWnd* pParent = NULL);   // standard constructor

	// dtor
	~CValueBlenderDlg() ;

// Dialog Data
	//{{AFX_DATA(CValueBlenderDlg)
	enum { IDD = IDD_VALUE_BLENDER };
	CStatic	m_Value2;
	CStatic	m_Value1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValueBlenderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	TCreateDialog _CreateFunc ;
	void *_CreateFuncParam ;

	void *_InfoToDelete1, *_InfoToDelete2 ;

	// this will delete _CreateFuncParam in this obj dtor if set to true
	bool _MustDelete ;

	// the 2 dialog used to choose the blending value
	CEditAttribDlg *_Dlg1, *_Dlg2 ;

	// Generated message map functions
	//{{AFX_MSG(CValueBlenderDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUEBLENDERDLG_H__4242C860_C538_45BD_8348_C6DF314D688A__INCLUDED_)
