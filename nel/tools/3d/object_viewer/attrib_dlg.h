#if !defined(AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_)
#define AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include "nel/misc/rgba.h"

using NLMISC::CRGBA ;


class CEditableRange ;
class CEditableRangeFloat ;
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

	CAttribDlgT(const std::string &valueID) : CAttribDlg(valueID), _Reader(NULL)
											, _Writer(NULL), _SchemeWriter(NULL), _SchemeReader(NULL)
	{
	}
		
	// set a function to get back the constant datas (this may be a wrapper to an existing class)
	void setReader(T (* reader) (void *lParam), void *lParam) { _Reader = reader ; _ReaderParam =  lParam ; }
	// set a function to write new constant datas
	void setWriter(void (* writer) (T value, void *lParam), void *lParam) { _Writer = writer ; _WriterParam = lParam ; }

	// set a function to get back the float scheme
	void setSchemeReader(NL3D::CPSAttribMaker<T> *(* reader) (void *lParam), void *lParam) { _SchemeReader = reader ; _SchemeReaderParam =  lParam ; }
	// set a function to set a new float scheme
	void setSchemeWriter(void (* writer) (NL3D::CPSAttribMaker<T> *scheme, void *lParam), void *lParam) { _SchemeWriter = writer ; _SchemeWriterParam = lParam ; }

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const = 0 ;	
	virtual std::string getSchemeName(uint index) const = 0 ;	
	virtual void editScheme(void) = 0 ;	
	virtual void setCurrentScheme(uint index) = 0 ;
	virtual sint getCurrentScheme(void) const  = 0 ;

	virtual bool hasSchemeCustomInput(void) const { return _SchemeReader(_SchemeReaderParam)->hasCustomInput() ; }
	virtual uint getSchemeInput(void) const { return (uint)  _SchemeReader(_SchemeReaderParam)->getInput() ; }	
	virtual void setSchemeInput(uint index) { _SchemeReader(_SchemeReaderParam)->setInput((NL3D::CPSLocated::AttributeType) index) ; }


	virtual float getSchemeNbCycles(void) const { return _SchemeReader(_SchemeReaderParam)->getNbCycles() ; }
	virtual void setSchemeNbCycles(float nbCycles) { _SchemeReader(_SchemeReaderParam)->setNbCycles(nbCycles) ; }


	
	virtual bool isSchemeClamped(void) const { return _SchemeReader(_SchemeReaderParam)->getClamping() ; }
	virtual void clampScheme(bool clamped = true) { _SchemeReader(_SchemeReaderParam)->setClamping(true) ; }	
	virtual bool isClampingSupported(void) const { return _SchemeReader(_SchemeReaderParam)->isClampingSupported() ; } ;



protected:


	virtual bool useScheme(void) const
	{
		nlassert(_SchemeReader) ;
		return(_SchemeReader(_SchemeReaderParam) != NULL) ;
	}


public:
	// wrappers to set/get a constant float value, or a float scheme
	T(* _Reader)(void *lParam)  ;
	void(* _Writer) (T value, void *lParam)  ;
	NL3D::CPSAttribMaker<T> *(* _SchemeReader) (void *lParam) ; 
	void (* _SchemeWriter) (NL3D::CPSAttribMaker<T> *scheme, void *lParam) ;

	// wrappers user parameters
	void *_ReaderParam, *_WriterParam
		 , *_SchemeReaderParam, *_SchemeWriterParam ;

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
