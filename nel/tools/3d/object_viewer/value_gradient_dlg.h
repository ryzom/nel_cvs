/** \file value_gradient_dlg.h
 * a dialog that allows to edit a gradient of value, used in a particle system
 *
 * $Id: value_gradient_dlg.h,v 1.3 2001/06/25 12:53:28 vizerie Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02
*/



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
template <class T> class CEditableRangeT ;
typedef CEditableRangeT<uint32> CEditableRangeUInt ;

#include "ps_wrapper.h"
#include "attrib_list_box.h"



/** This struct serves as an interface to manage the gradient.
 *  Deriver should provide the following methods.
 */

struct IValueGradientDlgClient
{
	/** must provide a dialog for the edition of one value
	 *  ONLY ONE DIALOG WILL BE QUERRIED AT A TIME. the previous dialog is detroyed before this is called again
	 * \param index the index of the value in the dialog
	 * \grad the dlg that called this method (deriver can ask a redraw then)
	 */
	virtual CEditAttribDlg *createDialog(uint index, CValueGradientDlg *grad) = 0 ;

	/// this enumerate the action that we can apply on a gradient
	enum TAction { Add, Insert, Delete } ;

	/// a function that can add, remove, or insert a new element in the gradient
	virtual void modifyGradient(TAction, uint index) = 0 ;

	/// a function that can display a value in a gradient, with the given offset
	virtual void displayValue(CDC *dc, uint index, sint x, sint y)  = 0;

	// return the number of values in a scheme
	virtual uint32 getSchemeSize(void) const  = 0 ;

	// get the number of interpolation step
	virtual uint32 getNbSteps(void) const = 0 ;

	// set the number of interpolation steps
	virtual void setNbSteps(uint32 value) = 0 ;
	
} ;


class CValueGradientDlg : public CDialog
{

public:


	/** construct the dialog. The user must provides an interface of type IValueGradientDlgClient	
	 * and a pointer to the parent window
	 */

	CValueGradientDlg(IValueGradientDlgClient *clientInterface						
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


	IValueGradientDlgClient *_ClientInterface ;
	
	// the dialog for edition of the current value
	CEditAttribDlg *_EditValueDlg ;

	// the dialog to edit the current number of step for gradient interpolation
	CEditableRangeUInt *_NbStepDlg ;

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

	// a wrapper to tune the number of step
	struct tagNbStepWrapper :public IPSWrapperUInt
	{
		// the interface that was passed to the dialog this struct is part of
		IValueGradientDlgClient *I ;	
		uint32 get(void) const { return I->getNbSteps() ; }
		void set(const uint32 &nbSteps) { I->setNbSteps(nbSteps) ; }

	} _NbStepWrapper ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUE_GRADIENT_DLG_H__45A21D97_D65B_494E_B171_D53F71F4AFC7__INCLUDED_)
