/** \file attrib_dlg.h
 * <File description>
 *
 * $Id: attrib_dlg.h,v 1.2 2001/06/12 17:12:36 vizerie Exp $
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

#if !defined(AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_)
#define AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include "nel/misc/rgba.h"
#include "editable_range.h"



using NLMISC::CRGBA ;



class CEditAttribDlg ;


/////////////////////////////////////////////////////////////////////////////
// CAttribDlg dialog

class CAttribDlg : public CDialog
{
// Construction
public:
	CAttribDlg(const std::string &valueID);   // standard constructor


	CAttribDlg::~CAttribDlg() ;

// Dialog Data
	//{{AFX_DATA(CAttribDlg)
	enum { IDD = IDD_ATTRIB_DLG };
	CComboBox	m_SchemeInput;
	CStatic	m_CstValuePos;
	CStatic	m_NbCyclePos;
	CButton	m_AttrBitmap;
	CButton	m_ClampCtrl;
	CButton	m_EditScheme;
	CComboBox	m_UseScheme;
	CComboBox	m_Scheme;
	CString	m_AttribName;
	BOOL	m_Clamp;
	//}}AFX_DATA


	/// init the dialog with the given bitmap
	void init(HBITMAP bitmap, sint x, sint y, CWnd *pParent = NULL) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttribDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	// private usage (not private because accessed by a static function) : return the nbCycles parameter of the scheme (e.g the input multiplier).
	virtual float getSchemeNbCycles(void) const = 0 ;

	// private usage (not private because accessed by a static function) : set the nbCycles parameter of the scheme (e.g the input multiplier)
	virtual void setSchemeNbCycles(float nbCycles) = 0 ;

// Implementation
protected:	

	// change the dialog for constant values
	void cstValueUpdate() ;

	// change the dialog for scheme usage
	void schemeValueUpdate() ;

	// return true if a scheme is used
	virtual bool useScheme(void) const = 0 ;


	// get the number of attributes makers  (schemes) that are available
	virtual uint getNumScheme(void) const = 0 ;

	// get the text associated with an attribute maker
	virtual std::string getSchemeName(uint index) const = 0 ;

	// edit the current scheme
	virtual void editScheme(void) = 0 ;

	// set the current scheme
	virtual void setCurrentScheme(uint index) = 0 ;

	// get the current scheme, -1 if the scheme is unknow (created outside the editor ?)
	virtual sint getCurrentScheme(void) const = 0 ;


	// this must return the right dialog for a constant value (created with nex)
	virtual CEditAttribDlg *createConstantValueDlg() = 0 ;


	// tells wether the scheme supports custom input
	virtual bool hasSchemeCustomInput(void) const = 0 ;
	// retrieve the scheme input id
	virtual uint getSchemeInput(void) const = 0 ;
	// set the scheme input id
	virtual void setSchemeInput(uint index) = 0 ;

	// tells wether the scheme input value is clamped or not
	virtual bool isSchemeClamped(void) const = 0 ;
	// clamp / unclamp the scheme
	virtual void clampScheme(bool clamped = true) = 0 ;
	// return true if clamping is supported
	virtual bool isClampingSupported(void) const = 0 ;


	// the dialog used to tune the nb cycles param (when available)
	CEditableRangeFloat *_NbCyclesDlg ;	

	// wrapper to tune the number of cycles
	struct tagNbCyclesWrapper : public IPSWrapperFloat
	{
			CAttribDlg *Dlg ;
			float get(void) const { return Dlg->getSchemeNbCycles() ; }
			void set(const float &v) { Dlg->setSchemeNbCycles(v) ; }
	} _NbCyclesWrapper ;



	// the dialog used to tune a constant value
	CEditAttribDlg *_CstValueDlg ;


	// Generated message map functions
	//{{AFX_MSG(CAttribDlg)
	afx_msg void OnSelchangeUseScheme();
	afx_msg void OnSelchangeScheme();
	afx_msg void OnEditScheme();
	afx_msg void OnSelchangeSchemeInput();
	afx_msg void OnClampAttrib();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/**
 * a template class that helps to specialize the attrib maker edition dialog with various types 
 */


template <typename T> class CAttribDlgT : public CAttribDlg
{	
public:

	CAttribDlgT(const std::string &valueID) : CAttribDlg(valueID), _Wrapper(NULL)
											, _SchemeWrapper(NULL)
	{
	}
		

	void setWrapper(IPSWrapper<T> *wrapper) { nlassert(wrapper) ; _Wrapper = wrapper ; }
	void setSchemeWrapper(IPSSchemeWrapper<T> *schemeWrapper) { nlassert(schemeWrapper) ; _SchemeWrapper = schemeWrapper ; }
	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const = 0 ;	
	virtual std::string getSchemeName(uint index) const = 0 ;	
	virtual void editScheme(void) = 0 ;	
	virtual void setCurrentScheme(uint index) = 0 ;
	virtual sint getCurrentScheme(void) const  = 0 ;

	virtual bool hasSchemeCustomInput(void) const { return _SchemeWrapper->getScheme()->hasCustomInput() ; }
	virtual uint getSchemeInput(void) const { return (uint)  _SchemeWrapper->getScheme()->getInput() ; }	
	virtual void setSchemeInput(uint index) { _SchemeWrapper->getScheme()->setInput((NL3D::CPSLocated::AttributeType) index) ; }


	virtual float getSchemeNbCycles(void) const { return _SchemeWrapper->getScheme()->getNbCycles() ; }
	virtual void setSchemeNbCycles(float nbCycles) { _SchemeWrapper->getScheme()->setNbCycles(nbCycles) ; }


	
	virtual bool isSchemeClamped(void) const { return _SchemeWrapper->getScheme()->getClamping() ; }
	virtual void clampScheme(bool clamped = true) { _SchemeWrapper->getScheme()->setClamping(true) ; }	
	virtual bool isClampingSupported(void) const { return _SchemeWrapper->getScheme()->isClampingSupported() ; } ;



protected:


	virtual bool useScheme(void) const
	{
		nlassert(_SchemeWrapper) ;
		return(_SchemeWrapper->getScheme() != NULL) ;
	}


public:
	// wrapper to set/get a constant float
	IPSWrapper<T> *_Wrapper ;

	// wrapper to set/get a scheme
	IPSSchemeWrapper<T> *_SchemeWrapper	;

} ;






/** an attribute editor specialized for float values
 */

class CAttribDlgFloat : public CAttribDlgT<float>
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 *  \param minValue : the min value for the editable range dlg(for constant value)
	 *  \param maxValue : the min value for the editable range dlg (for constant value)
	 */
	CAttribDlgFloat(const std::string &valueID, float minValue, float maxValue)  ;

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const ;	
	virtual std::string getSchemeName(uint index) const ;	
	virtual void editScheme(void) ;	
	virtual void setCurrentScheme(uint index) ;
	virtual sint getCurrentScheme(void) const  ;



protected:

	virtual CEditAttribDlg *createConstantValueDlg() ;
	// ID for the cst float value  edition dialog
	std::string _CstValueId ;
	float _MinRange, _MaxRange ;
} ;


/** an attribute editor specialized for RGB values
 */

class CAttribDlgRGBA : public CAttribDlgT<CRGBA>
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 */
	CAttribDlgRGBA(const std::string &valueID)  ;

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const ;	
	virtual std::string getSchemeName(uint index) const ;	
	virtual void editScheme(void) ;	
	virtual void setCurrentScheme(uint index) ;
	virtual sint getCurrentScheme(void) const  ;



protected:

	virtual CEditAttribDlg *createConstantValueDlg() ;
	// ID for the cst float value  edition dialog
	std::string _CstValueId ;	
} ;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_)
