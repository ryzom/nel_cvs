#if !defined(AFX_VALUE_FROM_EMITTER_DLG_H__D3416DBF_1735_4FBB_A1FD_7E8DCC133825__INCLUDED_)
#define AFX_VALUE_FROM_EMITTER_DLG_H__D3416DBF_1735_4FBB_A1FD_7E8DCC133825__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	template <typename T> class CPSAttribMakerMemory ;
}

#include "attrib_dlg.h"
#include "ps_wrapper.h"
#include "3d/ps_attrib_maker.h"

/////////////////////////////////////////////////////////////////////////////
// CValueFromEmitterDlg dialog

class CValueFromEmitterDlg : public CDialog
{
// Construction
public:
	CValueFromEmitterDlg(CWnd* pParent = NULL);   // standard constructor

	virtual void  init(void) = 0 ;
// Dialog Data
	//{{AFX_DATA(CValueFromEmitterDlg)
	enum { IDD = IDD_MEMORY_VALUE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValueFromEmitterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CValueFromEmitterDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/** construct a dialog that allow to edit a scheme used for initial attribute generation in a particle
  *  D is the type of the dialog to be crated for edition : CAttribDlgFloat / Int etc ..
  *  v1 is the default value for range min
  *  v2 is the default value for range max
  */
template <class T, class D> class CValueFromEmitterDlgT : public CValueFromEmitterDlg
{
public:
	/// ctruct the given dialog from the given scheme that owns memory
	CValueFromEmitterDlgT(NL3D::CPSAttribMakerMemory<T> *editedScheme, std::string &id, HBITMAP bitmapToDisplay)
		: _Id(id), _BitmapToDisplay(bitmapToDisplay), _AttrbDlg(NULL)
	{
		_SchemeWrapper.S = editedScheme ;
	}

	/// call this before performing DoModal and the like...
	void init()
	{
	//	Create(IDD_MEMORY_VALUE_DLG, pParent) ;
		_AttrbDlg = new D(_Id) ;
		_AttrbDlg->disableConstantValue() ;
		_AttrbDlg->setWrapper(&_DummyWrapper) ;
		_AttrbDlg->setSchemeWrapper(&_SchemeWrapper) ;
		_AttrbDlg->init(_BitmapToDisplay, 10, 10, this) ;
	}

	~CValueFromEmitterDlgT()
	{
		if (_AttrbDlg) _AttrbDlg->DestroyWindow() ;
		delete _AttrbDlg ;
	}	

protected:
	
	// id
	std::string _Id ;


	// the bitmap displayed onthe left
	HBITMAP _BitmapToDisplay ;

	/// the dialog that allow us to edit the scheme
	CAttribDlgT<T> *_AttrbDlg ;

	/// a wrapper to edit the scheme (which himself owns a scheme !!)
	struct CSchemeWrapper : public IPSSchemeWrapper<T>
	{
		NL3D::CPSAttribMakerMemory<T> *S ;
		virtual scheme_type *getScheme(void) const { return S->getScheme() ; }
		virtual void setScheme(scheme_type *s) { S->setScheme(s) ; } ;
	} _SchemeWrapper ;

	/// a dummy wrapper for constant value. This shouldn't be called , however
	struct CDummyWrapper : public IPSWrapper<T>
	{
		T get(void) const { nlassert(false) ; return T() ; }
		void set(const T &) { nlassert(false) ; }
	} _DummyWrapper ;
		
	
} ;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.




#endif // !defined(AFX_VALUE_FROM_EMITTER_DLG_H__D3416DBF_1735_4FBB_A1FD_7E8DCC133825__INCLUDED_)
