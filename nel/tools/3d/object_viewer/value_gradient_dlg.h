#if !defined(AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_)
#define AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// value_gradient_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CValueGradientDlg dialog

class CEditAttribDlg ;

#include "attrib_list_box.h"

class CValueGradientDlg : public CDialog
{

public:

	/** a pointer over a function that create a dialog of the right type for the edition of one value, which must have CEditAttribDlg as a base class
	 *  \param index must be the number of the element to be edited, it says which value is being edited
	 *  \infoToDelete: delete is call on this when the dialog is no more needed. The call back must will the pointer with what must be deleted, or NULL
	 */
	typedef CEditAttribDlg *(* TCreateDialog)(uint index, void *lParam, void **infoToDelete, CValueGradientDlg *grad) ;

	/// this enumerate the action that we can apply on a gradient
	enum TAction { Add, Insert, Delete } ;
	/// a function that can add, remove, or insert a new element in the gradient
	typedef void (* TModifyGradient)(TAction, uint index, void *lParam) ;

	/// a function that can display a value in a gradient, with the given offset
	typedef void (* TDisplayValue)(CDC *dc, uint index, sint x, sint y, void *lParam) ;


	/** construct the dialog. The user must provides some callback, which enable this dialog to be more generic
	 *  - a callback that can create an edition dialog on a given value in the gradient
	 *  - a callback that can modify a gradient (add, delete ...)
	 *  - a callback that can draw a velue of the gradient
	 * For each callback, the user can provide a user param. And he must tell wether delete must be called
	 * For each of the user param (struct with no dtor only, and that contain no object with dtor, too  !!)	.
	 * The user must also provide the initial size (there's no callback for it ...)
	 */
	CValueGradientDlg(TCreateDialog createDialogFunc, TModifyGradient modifyGradientFunc, TDisplayValue displayValueFunc
						, void *createLParam, void *modifyLParam, void *displayLParam						
						, uint initialSize
						, CWnd* pParent = NULL);   // standard constructor

	/// invalidate the gradient list box

	void invalidateGrad(void) ;

	/// dtor
	~CValueGradientDlg() ;

// Dialog Data
	//{{AFX_DATA(CValueGradientDlg)
	enum { IDD = IDD_GRADIENT_DLG };
	CAttribListBox	m_GradientList;
	CStatic	m_Value;
	CButton	m_RemoveCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValueGradientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	TCreateDialog _CreateDialogFunc ;
	TModifyGradient _ModifyGradientFunc ;
	TDisplayValue _DisplayValueFunc ;
    void *_CreateLParam ;
	void *_ModifyLParam ;
	void *_DisplayLParam ;
	void *_InfoToDelete ;

	// the dialog for edition of the current value
	CEditAttribDlg *_EditValueDlg ;

	// the current size of the gradient
	uint _Size ;

	// Generated message map functions
	//{{AFX_MSG(CValueGradientDlg)
	afx_msg void OnAddValue();
	afx_msg void OnInsertValue();
	afx_msg void OnRemoveValue();
	afx_msg void OnValueDown();
	afx_msg void OnValueUp();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGradientList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_)
