/** \file direction_attr.h
 * a dialog to choose a direction (normalized vector). It gives several choices, or allow 
 * to call a more complete dialog (CDirectionEdit)
 *
 * $Id: direction_attr.h,v 1.3 2001/12/18 18:36:18 vizerie Exp $
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


#if !defined(AFX_DIRECTION_ATTR_H__C906E2A2_917C_4F4A_B771_5656204914F5__INCLUDED_)
#define AFX_DIRECTION_ATTR_H__C906E2A2_917C_4F4A_B771_5656204914F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "edit_attrib_dlg.h"
#include "ps_wrapper.h"
#include "popup_notify.h"

/// This dialog helps to choose from several preset directions, or to choose a custom one
class CDirectionAttr : public CEditAttribDlg, public IPopupNotify
{
// Construction
public:	
	// construct the dialog.
	CDirectionAttr(const std::string &id);   // standard constructor

	// inherited from CEditAttribDlg
	virtual void init(uint32 x, uint32 y, CWnd *pParent);

	void setWrapper(IPSWrapper<NLMISC::CVector> *wrapper) { _Wrapper = wrapper; }


	BOOL EnableWindow( BOOL bEnable = TRUE );


// Dialog Data
	//{{AFX_DATA(CDirectionAttr)
	enum { IDD = IDD_DIRECTION_ATTR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDirectionAttr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	friend class CDirectionEdit; 

	// a dialog that we may create for custom direction edition
	class CDirectionEdit *_DirectionDlg;

	// inherited from IPopupNotify
	void childPopupClosed(CWnd *);
	

	IPSWrapper<NLMISC::CVector> *_Wrapper;

	// Generated message map functions
	//{{AFX_MSG(CDirectionAttr)
	afx_msg void OnVectI();
	afx_msg void OnVectJ();
	afx_msg void OnVectK();
	afx_msg void OnVectMinusI();
	afx_msg void OnVectMinusJ();
	afx_msg void OnVectMinusK();
	afx_msg void OnCustomDirection();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTION_ATTR_H__C906E2A2_917C_4F4A_B771_5656204914F5__INCLUDED_)
