/** \file located_bindable_dialog.h
 * <File description>
 *
 * $Id: located_bindable_dialog.h,v 1.2 2001/06/12 17:12:36 vizerie Exp $
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

#if !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
#define AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSLocatedBindable ;
}



#include "ps_wrapper.h"
#include "nel/misc/rgba.h"


using NLMISC::CRGBA ;

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


	//////////////////////////////////////////////
	// wrappers to various element of bindables //
	//////////////////////////////////////////////

		//////////
		// size //
		//////////
			struct tagSizeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSSizedParticle *S ;
			   float get(void) const { return S->getSize() ; }
			   void set(const float &v) { S->setSize(v) ; }
			   scheme_type *getScheme(void) const { return S->getSizeScheme() ; }
			   void setScheme(scheme_type *s) { S->setSizeScheme(s) ; }
			} _SizeWrapper ;
			
		///////////
		// color //
		///////////
			struct tagColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
			{
			   NL3D::CPSColoredParticle *S ;
			   CRGBA get(void) const { return S->getColor() ; }
			   void set(const CRGBA &v) { S->setColor(v) ; }
			   scheme_type *getScheme(void) const { return S->getColorScheme() ; }
			   void setScheme(scheme_type *s) { S->setColorScheme(s) ; }
			} _ColorWrapper ;
			
		//////////////
		// angle 2D //
		//////////////
			struct tagAngle2DWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
			{
			   NL3D::CPSRotated2DParticle *S ;
			   float get(void) const { return S->getAngle2D() ; }
			   void set(const float &v) { S->setAngle2D(v) ; }
			   scheme_type *getScheme(void) const { return S->getAngle2DScheme() ; }
			   void setScheme(scheme_type *s) { S->setAngle2DScheme(s) ; }
			} _Angle2DWrapper ;
			



};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
