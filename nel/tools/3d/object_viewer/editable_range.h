/** \file editable_range.h
 * <File description>
 *
 * $Id: editable_range.h,v 1.1 2001/06/12 08:39:50 vizerie Exp $
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
 * MA 02111-1307, USA.
 */


#if !defined(AFX_EDITABLE_RANGE_H__0B1EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_)
#define AFX_EDITABLE_RANGE_H__0B1EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// editable_range.h : header file
//

#include <string>
#include "edit_attrib_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CEditableRange dialog

class CEditableRange : public CEditAttribDlg
{
public:
	

	/**
	 * construct the dialog by giving it an Id. It will be used to save the user preference
	 * Each dialog of this type must have its own id in the app
	 */
	CEditableRange(const std::string &id);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditableRange)
	enum { IDD = IDD_EDITABLE_RANGE };
	CSliderCtrl	m_SliderCtrl;
	CEdit	m_ValueCtrl;
	CButton	m_UpdateValue;
	CButton	m_SelectRange;
	CString	m_MinRange;
	CString	m_MaxRange;
	CString	m_Value;
	int		m_SliderPos;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditableRange)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


	// init the dialog at the given position
	public:

		virtual void init(uint32 x, uint32 y, CWnd *pParent) ;

		BOOL EnableWindow( BOOL bEnable = TRUE );


public:
	/// for derivers : this must querries the value to the desired manager and set the dialog values
	virtual void updateRange(void) = 0 ;
	/** for derivers : value update from a linked object (reader). See examples in subclasses
	 */
	virtual void updateValueFromReader(void) = 0 ;

	// empty the values and the slider
	void emptyDialog(void) ;


// Implementation
protected:

	
	/** for derivers : value update : this is call with a float ranging from 0.f to 1.f (from the slider)
	 *  And it must convert it to the desired value, changing the value of this dialog
	 */
	virtual void updateValueFromSlider(float sliderValue) = 0 ;

	

	/// the text has changed, and the user has pressed update
	virtual void updateValueFromText(void) = 0 ;	




	// the range tune button was pressed. It muist show a dialog that allows the user to choose the range he wants
	virtual void selectRange(void) = 0 ;

	

	// the unique id of this dialog
	std::string _Id ;


	// Generated message map functions
	//{{AFX_MSG(CEditableRange)
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectRange();
	afx_msg void OnUpdateValue();
	afx_msg void OnSetfocusValue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// here, we define EditableRanges for the most common types
// edit float with no specific values
class CEditableRangeFloat : public CEditableRange
{
public:
/// ctor, it gives the range for the values
	CEditableRangeFloat(const std::string &id, float defaultMin, float defaultMax) ;
	
// set a function to get back the datas (this may be a wrapper to an existing class)
	void setReader(float (* reader) (void *lParam), void *lParam) { _Reader = reader ; _ReaderParam =  lParam ; }
// set a function to write new datas
	void setWriter(void (* writer) (float value, void *lParam), void *lParam) { _Writer = writer ; _WriterParam = lParam ; }



public:
	virtual void updateRange(void) ;
	virtual void updateValueFromReader(void) ;
protected:	
	virtual void updateValueFromText(void) ;	
	virtual void selectRange(void) ;
	virtual void updateValueFromSlider(float sliderValue) ;	
	/// set a new value that will affect both slider and value display
	void setValue(float value) ;

	std::pair<float, float> _Range ;


	float(* _Reader)(void *lParam)  ;
	void(* _Writer) (float value, void *lParam)  ;

	void *_ReaderParam, *_WriterParam ;

} ;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.







#endif // !defined(AFX_EDITABLE_RANGE_H__0B1EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_)
