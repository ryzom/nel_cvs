/** \file attrib_dlg.cpp
 * class for a dialog box that help to edit an attrib value : it helps setting a constant value or not
 *
 * $Id: attrib_dlg.cpp,v 1.7 2001/06/25 13:15:32 vizerie Exp $
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

// attrib_dlg.cpp : implementation file
//



#include "std_afx.h"
#include "object_viewer.h"
#include "attrib_dlg.h"
#include "editable_range.h"
#include "color_edit.h"
#include "basis_edit.h"
#include "value_blender_dlg.h"
#include "value_gradient_dlg.h"




#include "3d/ps_attrib_maker.h"
#include "3d/ps_float.h"
#include "3d/ps_int.h"
#include "3d/ps_color.h"
#include "3d/ps_plane_basis.h"
#include "3d/ps_plane_basis_maker.h"





#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////
// WRAPPERS to set / retrieve the NbCycles parameter of a scheme   //
/////////////////////////////////////////////////////////////////////
static float NbCyclesReader(void *lParam) { return ((CAttribDlg *) lParam)->getSchemeNbCycles() ; }
static void NbCyclesWriter(float value, void *lParam) { ((CAttribDlg *) lParam)->setSchemeNbCycles(value) ; }


///////////////////////////////////////////
// GENERAL INTERFACE FOR BLENDER EDITION //
///////////////////////////////////////////

/**  T is the type to be edited (color, float, etc..)
 *   E is the dialog to create. It must have at least a constructor with a single parameter of type std::string
 *     It must also derive from CEditAttribDlg, and have a set wrapper method that match the T type
 *   , even if it is unused
 */

template <typename T, class E> 
class CValueBlenderDlgClientT : public IValueBlenderDlgClient
{
	public:
		std::string Id ; // the Id of each of the dialog (it will be followed by %1 or %2)
						 // must be filled by the user

		// the scheme being used. Must be set by the user
		NL3D::CPSValueBlender<T> *Scheme ;

	protected:
		virtual CEditAttribDlg *createDialog(uint index)
		{
			std::string id = Id ;
			if (index == 0) id += "%1" ; else id += "%2" ;
			E *dlg = new E(id) ;
			dlg->setWrapper(&_ValueInfos[index]) ;
			_ValueInfos[index].ValueIndex = index ;
			_ValueInfos[index].Scheme = Scheme ;

			return dlg ;
		}

		// inherited from IPSWrapper<T>

		struct COneValueInfo : public IPSWrapper<T>
		{
			// valuet 0 or 1 being edited
			uint ValueIndex ;
			// the scheme being edited
			NL3D::CPSValueBlender<T> *Scheme ;

			virtual T get(void) const 
			{ 
				T t1, t2 ;
				Scheme->_F.getValues(t1, t2) ;
				return ValueIndex == 0 ? t1 : t2 ;
			}			
			virtual void set(const T &value)
			{
				T t1, t2 ;
				Scheme->_F.getValues(t1, t2) ;
				if (ValueIndex == 0 ) t1 = value ; else t2 = value ;
				Scheme->_F.setValues(t1, t2) ;
			}
		} ;

		COneValueInfo _ValueInfos[2] ;


} ;


////////////////////////////////////////////
// GENERAL INTERFACE FOR GRADIENT EDITION //
////////////////////////////////////////////

/** This template generate an interface that is used with the gradient edition dialog
 *  T is the type to be edited (color, floet, etc..)
 *  V is the default value for creation
 *  E is the class for one value edition. It must derives from CEditAttribDlg
 */
 

template <typename T, class E> 
class CValueGradientDlgClientT : public IValueGradientDlgClient, public IPSWrapper<T>
{
public:

	std::string Id ; // the Id of each of the dialog (it will be followed by %1 or %2)
						 // must be filled by the user
	// the gradient being edited, must be filled by the instancier
	NL3D::CPSValueGradientFunc<T> *Scheme ;
	// the gradient dialog, must be filled by the instancier
	CValueGradientDlg *GradDlg ;
	// the difault value for new values creation. Must be filled by the instancier
	T DefaultValue ;

	/// a function that can display a value in a gradient, with the given offset. Deriver must define this
	virtual void displayValue(CDC *dc, uint index, sint x, sint y)  = 0;

	
	/// inherited from IPSWrapper
	virtual T get(void) const { return Scheme->getValue(_CurrentEditedIndex) ; }
	virtual void set(const T &v)
	{
		T *tab = new T[Scheme->getNumValues()] ;
		Scheme->getValues(tab) ;
		tab[_CurrentEditedIndex] = v ;
		Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages()) ;
		delete[] tab ;
		GradDlg->invalidateGrad() ;
	}
	
	/** must provide a dialog for the edition of one value (only ONE exist at a time)
	 * \param index the index of the value in the dialog
	 * \grad the dlg that called this method (deriver can ask a redraw then)
	 */
	virtual CEditAttribDlg *createDialog(uint index, CValueGradientDlg *grad)
	{						
		E *dlg = new E(Id) ;
		dlg->setWrapper(this) ;		
		_CurrentEditedIndex = index ;
		return dlg ;
	}
	/// a function that can add, remove, or insert a new element in the gradient
	virtual void modifyGradient(TAction action, uint index)
	{
		
		T *tab = new T[Scheme->getNumValues() + 1] ; // +1 is for the add / insert case
		Scheme->getValues(tab) ;

		switch(action)
		{
			case IValueGradientDlgClient::Add:
				tab[Scheme->getNumValues()] = DefaultValue ;
				Scheme->setValues(tab, Scheme->getNumValues() + 1, Scheme->getNumStages()) ;
			break ;
			case IValueGradientDlgClient::Insert:
				memcpy(tab + (index + 1), tab + index, sizeof(T) * (Scheme->getNumValues() - index)) ;
				tab[index] = DefaultValue ;
				Scheme->setValues(tab, Scheme->getNumValues() + 1, Scheme->getNumStages()) ;
			break ;
			case IValueGradientDlgClient::Delete:
				memcpy(tab + index, tab + index + 1, sizeof(T) * (Scheme->getNumValues() - index - 1)) ;
				Scheme->setValues(tab, Scheme->getNumValues() - 1, Scheme->getNumStages()) ;
			break ;
		}
		delete[] tab ;
	}		
	virtual uint32 getSchemeSize(void) const { return Scheme->getNumValues() ; }

	// get the number of interpolation step
	uint32 getNbSteps(void) const
	{
		return Scheme->getNumStages() ;
	}

	// set the number of interpolation steps
	void setNbSteps(uint32 value)
	{
		Scheme->setNumStages(value) ;	
	}

protected:
		// index of the value OF the current dialog that exist
		uint32 _CurrentEditedIndex ;	
} ;



/////////////////////////////////////////////////////////////////////////////
// CAttribDlg dialog


CAttribDlg::CAttribDlg(const std::string &valueID) : _CstValueDlg(NULL), _FirstDrawing(true)	
{
	//{{AFX_DATA_INIT(CAttribDlg)
	m_AttribName = _T("");
	m_Clamp = FALSE;
	//}}AFX_DATA_INIT
	_NbCyclesDlg = new CEditableRangeFloat(valueID + "%%NB_CYCLE_INFO", 0.1f, 10.1f) ;
}


BOOL CAttribDlg::EnableWindow( BOOL bEnable)
{
	if (_CstValueDlg)
	{
		_CstValueDlg->EnableWindow(bEnable) ;
	}
	_NbCyclesDlg->EnableWindow(bEnable) ;
	m_UseScheme.EnableWindow(bEnable) ;
	m_AttrBitmap.EnableWindow(bEnable) ;

	if (useScheme())
	{
		m_Scheme.EnableWindow(bEnable) ;
		m_SchemeInput.EnableWindow(bEnable) ;
		m_EditScheme.EnableWindow(bEnable) ;
		m_ClampCtrl.EnableWindow(bEnable) ;
	}
	else
	{		
		m_ClampCtrl.EnableWindow(FALSE) ;
	}

	UpdateData(FALSE) ;

	return CDialog::EnableWindow(bEnable) ;
}

CAttribDlg::~CAttribDlg()	
{
	_NbCyclesDlg->DestroyWindow() ;
	delete _NbCyclesDlg ;
	if (_CstValueDlg)
	{
		_CstValueDlg->DestroyWindow() ;
		_CstValueDlg->DestroyWindow() ;
		delete _CstValueDlg ;	
	}
}

void CAttribDlg::init(HBITMAP bitmap, sint x, sint y, CWnd *pParent)
{
	Create(IDD_ATTRIB_DLG, pParent) ;
	RECT r, ro ;
	GetClientRect(&r) ;
	
	m_AttrBitmap.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bitmap) ;
	MoveWindow(x, y, r.right, r.bottom) ;

	m_NbCyclePos.GetWindowRect(&r) ;
	GetWindowRect(&ro) ;
	_NbCyclesDlg->init(r.left - ro.left, r.top - ro.top, this) ;


	// fill the combo box with the list of available scheme

	m_Scheme.InitStorage(getNumScheme(), 32) ; // 32 char per string pre-allocated

	for (uint k = 0 ; k < getNumScheme() ; ++k)
	{	
		m_Scheme.InsertString(k, getSchemeName(k).c_str()) ;
	}


	if (useScheme())
	{
		schemeValueUpdate() ;
	}
	else
	{
		cstValueUpdate() ;
	}





	ShowWindow(SW_SHOW) ;
}

void CAttribDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttribDlg)
	DDX_Control(pDX, IDC_SCHEME_INPUT, m_SchemeInput);
	DDX_Control(pDX, IDC_CONSTANT_VALUE_POS, m_CstValuePos);
	DDX_Control(pDX, IDC_ATTRIB_NB_CYCLES, m_NbCyclePos);
	DDX_Control(pDX, IDC_ATTR_BITMAP, m_AttrBitmap);
	DDX_Control(pDX, IDC_CLAMP_ATTRIB, m_ClampCtrl);
	DDX_Control(pDX, IDC_EDIT_SCHEME, m_EditScheme);
	DDX_Control(pDX, IDC_USE_SCHEME, m_UseScheme);
	DDX_Control(pDX, IDC_SCHEME, m_Scheme);
	DDX_Check(pDX, IDC_CLAMP_ATTRIB, m_Clamp);
	//}}AFX_DATA_MAP
}


void CAttribDlg::cstValueUpdate()
{
	if (!_FirstDrawing && !useScheme()) return ;	

	m_ClampCtrl.EnableWindow(FALSE) ;
	_NbCyclesDlg->EnableWindow(FALSE) ;
	_NbCyclesDlg->emptyDialog() ;
	m_EditScheme.EnableWindow(FALSE) ;
	m_EditScheme.ShowWindow(SW_HIDE) ;
	m_Scheme.EnableWindow(FALSE) ;
	m_Scheme.ShowWindow(SW_HIDE) ;
	m_SchemeInput.EnableWindow(FALSE) ;
	m_SchemeInput.ShowWindow(SW_HIDE) ;

	resetCstValue() ;			



	m_UseScheme.SetCurSel(0) ;
	_CstValueDlg = createConstantValueDlg() ;
	CRect r, ro ;
	m_CstValuePos.GetWindowRect(&r) ;
	GetWindowRect(&ro) ;
	_CstValueDlg->init(r.left - ro.left, r.top - ro.top, this) ;
	UpdateData(FALSE) ;

	_FirstDrawing = false ;
}

void CAttribDlg::schemeValueUpdate()
{
	if (!_FirstDrawing && useScheme()) return ;		

	if (_CstValueDlg)
	{
		_CstValueDlg->DestroyWindow() ;
		delete _CstValueDlg ;
		_CstValueDlg = NULL ;
	}


	_NbCyclesDlg->EnableWindow(TRUE) ;


	m_EditScheme.EnableWindow(TRUE) ;
	m_EditScheme.ShowWindow(SW_SHOW) ;
	m_Scheme.EnableWindow(TRUE) ;
	m_Scheme.ShowWindow(SW_SHOW) ;
	m_SchemeInput.EnableWindow(TRUE) ;
	m_SchemeInput.ShowWindow(SW_SHOW) ;

	m_UseScheme.SetCurSel(1) ;

	sint k = getCurrentScheme() ;
	if (k == -1) // unknow scheme ...
	{
		setCurrentScheme(0) ;
		k = 0 ;
	}

	m_Scheme.SetCurSel(k) ;

	if (hasSchemeCustomInput())
	{
		m_SchemeInput.EnableWindow() ;
		m_SchemeInput.SetCurSel(getSchemeInput()) ;
	}
	else
	{
		m_SchemeInput.EnableWindow(FALSE) ;
		m_SchemeInput.SetCurSel(0) ;
	}


	
	_NbCyclesDlg->setWrapper(&_NbCyclesWrapper) ;
	_NbCyclesWrapper.Dlg = this ;
	
	_NbCyclesDlg->updateRange() ;
	_NbCyclesDlg->updateValueFromReader() ;


	m_ClampCtrl.EnableWindow(isClampingSupported()) ;
	if (isClampingSupported())
	{
		m_Clamp = isSchemeClamped() ;
	}


	UpdateData(FALSE) ;

	_FirstDrawing = false ;
}


void CAttribDlg::OnSelchangeUseScheme() 
{
	if (m_UseScheme.GetCurSel() == 0)
	{
		cstValueUpdate() ;
	}
	else
	{
		schemeValueUpdate() ;
	}
}



void CAttribDlg::OnSelchangeScheme() 
{
	UpdateData() ;
	setCurrentScheme(m_Scheme.GetCurSel()) ;	
}


void CAttribDlg::OnEditScheme() 
{
	editScheme() ;
}



void CAttribDlg::OnSelchangeSchemeInput() 
{
	UpdateData() ;
	setSchemeInput((NL3D::CPSLocated::AttributeType) m_SchemeInput.GetCurSel()) ;
}


void CAttribDlg::OnClampAttrib() 
{
	UpdateData() ;
	clampScheme(m_Clamp ? true : false /* avoid performance warning */) ;
}





BEGIN_MESSAGE_MAP(CAttribDlg, CDialog)
	//{{AFX_MSG_MAP(CAttribDlg)
	ON_CBN_SELCHANGE(IDC_USE_SCHEME, OnSelchangeUseScheme)
	ON_CBN_SELCHANGE(IDC_SCHEME, OnSelchangeScheme)
	ON_BN_CLICKED(IDC_EDIT_SCHEME, OnEditScheme)
	ON_CBN_SELCHANGE(IDC_SCHEME_INPUT, OnSelchangeSchemeInput)
	ON_BN_CLICKED(IDC_CLAMP_ATTRIB, OnClampAttrib)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttribDlg message handlers



////////////////////////////////////
// CAttribDlgFloat implementation //
////////////////////////////////////



	//////////////////////////////////////////////////////////
	// FLOAT GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////


	class CFloatGradientDlgWrapper : public CValueGradientDlgClientT<float, CEditableRangeFloat>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			
			CString out ;
			out.Format("%g",  Scheme->getValue(index) ) ;
			dc->TextOut(x + 10, y + 4, out) ;
		}
	} ;



	CAttribDlgFloat::CAttribDlgFloat(const std::string &valueID, float minRange, float maxRange)
				:  CAttribDlgT<float>(valueID), _MinRange(minRange), _MaxRange(maxRange)			  
	{
			
	}

	CEditAttribDlg *CAttribDlgFloat::createConstantValueDlg()
	{
		CEditableRangeFloat *erf = new CEditableRangeFloat(_CstValueId, _MinRange, _MaxRange) ;
		erf->setWrapper(_Wrapper) ;		
		return erf ;
	}

	uint CAttribDlgFloat::getNumScheme(void) const
	{
		return 2 ;
	}
	std::string CAttribDlgFloat::getSchemeName(uint index) const
	{
		nlassert(index < 2) ;
		switch (index)
		{
			case 0 :
				return std::string("value blender") ;
			break ;
			case 1 :
				return std::string("values gradient") ;
			break ;
			default:
				return std::string("") ;
			break ;
		}
	}
	void CAttribDlgFloat::editScheme(void)
	{
		const NL3D::CPSAttribMaker<float> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSFloatBlender *>(scheme)) 
		{				
			CValueBlenderDlgClientT<float, CEditableRangeFloat> myInterface ;
			myInterface.Id = std::string("FLOAT_BLENDER") ;
			myInterface.Scheme = (NL3D::CPSValueBlender<float> *) scheme ;
			
			CValueBlenderDlg bd(&myInterface, this) ;
			bd.DoModal() ;
		
		}
		if (dynamic_cast<const NL3D::CPSFloatGradient *>(scheme)) 
		{
			CFloatGradientDlgWrapper wrapper ;
			wrapper.Scheme = &(((NL3D::CPSFloatGradient *) (_SchemeWrapper->getScheme()) )->_F) ;
			CValueGradientDlg gd(&wrapper, this) ;		
			wrapper.GradDlg = &gd ;
			wrapper.DefaultValue = 0.f ;
			wrapper.Id = std::string("FLOAT GRADIENT") ;

			gd.DoModal() ;
			
		}
		
	}

	sint CAttribDlgFloat::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<float> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSFloatBlender *>(scheme)) 
		{
			return 0 ;
		}
		if (dynamic_cast<const NL3D::CPSFloatGradient *>(scheme)) 
		{
			return 1 ;
		}
		return -1 ;
	}


	void CAttribDlgFloat::setCurrentScheme(uint index)
	{
		nlassert(index < 2) ;


		NL3D::CPSAttribMaker<float> *scheme = NULL ;

		switch (index)
		{
			case 0 :
				scheme = new NL3D::CPSFloatBlender ;
			break ;
			case 1 :
				scheme = new NL3D::CPSFloatGradient ;
			break ;
			default:	
			break ;
		}

		if (scheme)
		{
			_SchemeWrapper->setScheme(scheme) ;
		}
	}


////////////////////////////////////
// CAttribDlgUInt implementation //
////////////////////////////////////

	//////////////////////////////////////////////////////////
	// UINT GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////


	class CUIntGradientDlgWrapper : public CValueGradientDlgClientT<uint32, CEditableRangeUInt>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			
			CString out ;
			out.Format("%d",  Scheme->getValue(index) ) ;
			dc->TextOut(x + 10, y + 4, out) ;
		}
	} ;



	CAttribDlgUInt::CAttribDlgUInt(const std::string &valueID, uint32 minRange, uint32 maxRange)
				:  CAttribDlgT<uint32>(valueID), _MinRange(minRange), _MaxRange(maxRange)			  
	{
			
	}

	CEditAttribDlg *CAttribDlgUInt::createConstantValueDlg()
	{
		CEditableRangeUInt *erf = new CEditableRangeUInt(_CstValueId, _MinRange, _MaxRange) ;
		erf->setWrapper(_Wrapper) ;		
		return erf ;
	}

	uint CAttribDlgUInt::getNumScheme(void) const
	{
		return 2 ;
	}
	std::string CAttribDlgUInt::getSchemeName(uint index) const
	{
		nlassert(index < 2) ;
		switch (index)
		{
			case 0 :
				return std::string("value blender") ;
			break ;
			case 1 :
				return std::string("values gradient") ;
			break ;
			default:
				return std::string("") ;
			break ;
		}
	}
	void CAttribDlgUInt::editScheme(void)
	{
		const NL3D::CPSAttribMaker<uint32> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSUIntBlender *>(scheme)) 
		{				
			CValueBlenderDlgClientT<uint32, CEditableRangeUInt> myInterface ;
			myInterface.Id = std::string("FLOAT_BLENDER") ;
			myInterface.Scheme = (NL3D::CPSValueBlender<uint32> *) scheme ;
			
			CValueBlenderDlg bd(&myInterface, this) ;
			bd.DoModal() ;
		
		}
		if (dynamic_cast<const NL3D::CPSUIntGradient *>(scheme)) 
		{
			CUIntGradientDlgWrapper wrapper ;
			wrapper.Scheme = &(((NL3D::CPSUIntGradient *) (_SchemeWrapper->getScheme()) )->_F) ;
			CValueGradientDlg gd(&wrapper, this) ;		
			wrapper.GradDlg = &gd ;
			wrapper.DefaultValue = 0 ;
			wrapper.Id = std::string("FLOAT GRADIENT") ;

			gd.DoModal() ;
			
		}
		
	}

	sint CAttribDlgUInt::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<uint32> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSIntBlender *>(scheme)) 
		{
			return 0 ;
		}
		if (dynamic_cast<const NL3D::CPSIntGradient *>(scheme)) 
		{
			return 1 ;
		}
		return -1 ;
	}


	void CAttribDlgUInt::setCurrentScheme(uint index)
	{
		nlassert(index < 2) ;


		NL3D::CPSAttribMaker<uint32> *scheme = NULL ;

		switch (index)
		{
			case 0 :
				scheme = new NL3D::CPSUIntBlender ;
			break ;
			case 1 :
				scheme = new NL3D::CPSUIntGradient ;
			break ;
			default:	
			break ;
		}

		if (scheme)
		{
			_SchemeWrapper->setScheme(scheme) ;
		}
	}



///////////////////////
// CRGBA attributes  //
///////////////////////
	

	//////////////////////////////////////////////////////////
	// COLOR GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////


	class CColorGradientDlgWrapper : public CValueGradientDlgClientT<CRGBA, CColorEdit>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			CRGBA col = Scheme->getValue(index) ;

			RECT r ;

			r.left = x + 10;
			r.top = y + 10 ;
			r.right = x + 53 ;
			r.bottom = y + 29 ;

			CBrush b ;
			b.CreateSolidBrush(RGB(col.R, col.G, col.B)) ;
			dc->FillRect(&r, &b) ;	
			b.DeleteObject() ;

			b.CreateSolidBrush(RGB(0, 0, 0)) ;
			CGdiObject *old = dc->SelectObject(&b) ;	
			r.top = y + 10 ; r. bottom = y + 29 ;
			r.right = x + 53; r.left = x + 10 ;
			dc->FrameRect(&r, &b) ;
			dc->SelectObject(old) ;
			b.DeleteObject() ;
		}
	} ;



	////////////////////////////

	CAttribDlgRGBA::CAttribDlgRGBA(const std::string &valueID)  : CAttribDlgT<CRGBA>(valueID)
	{
	}

	uint CAttribDlgRGBA::getNumScheme(void) const
	{
		return 3 ;
	}

	std::string CAttribDlgRGBA::getSchemeName(uint index) const
	{
		nlassert(index < 3) ;
		switch (index)
		{
			case 0 :
				return std::string("color sampled blender") ;
			break ;
			case 1 :
				return std::string("color gradient") ;
			break ;
			case 2 :
				return std::string("color exact blender") ;
			break ;
			default:
				return std::string("") ;
			break ;
		}
	}



	void CAttribDlgRGBA::editScheme(void)
	{	
		const NL3D::CPSAttribMaker<CRGBA> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme)) 
		{				
			CValueBlenderDlgClientT<CRGBA, CColorEdit> myInterface ;
			myInterface.Id = std::string("RGBA_BLENDER") ;
			myInterface.Scheme = (NL3D::CPSValueBlender<CRGBA> *) scheme ;
			
			CValueBlenderDlg bd(&myInterface, this) ;
			bd.DoModal() ;
		
		}
		if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme)) 
		{
			CColorGradientDlgWrapper wrapper ;
			wrapper.Scheme = &(((NL3D::CPSColorGradient *) (_SchemeWrapper->getScheme()) )->_F) ;
			CValueGradientDlg gd(&wrapper, this) ;		
			wrapper.GradDlg = &gd ;
			wrapper.DefaultValue = CRGBA::White ;
			wrapper.Id = std::string("RGBA_GRADIENT") ;

			gd.DoModal() ;
		}
		if (dynamic_cast<const NL3D::CPSColorBlenderExact *>(scheme)) 
		{
			
		}
	}

	void CAttribDlgRGBA::setCurrentScheme(uint index)
	{
		nlassert(index < 3) ;

		NL3D::CPSAttribMaker<CRGBA> *scheme = NULL ;

		switch (index)
		{
			case 0 :
				scheme = new NL3D::CPSColorBlender ;
			break ;
			case 1 :
				scheme = new NL3D::CPSColorGradient ;
			break ;
			case 2 :
				scheme = new NL3D::CPSColorBlenderExact ;
			break ;
			default:	
			break ;
		}

		if (scheme)
		{
			_SchemeWrapper->setScheme(scheme) ;
		}
	}

	sint CAttribDlgRGBA::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<CRGBA> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme)) 
		{
			return 0 ;
		}
		if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme)) 
		{
			return 1 ;
		}
		if (dynamic_cast<const NL3D::CPSColorBlenderExact *>(scheme)) 
		{
			return 2 ;
		}
		return -1 ;
	}

	CEditAttribDlg *CAttribDlgRGBA::createConstantValueDlg()
	{
		CColorEdit *ce = new CColorEdit(std::string("COLOR_ATTRIB_EDIT")) ;

		ce->setWrapper(_Wrapper) ;
	
		return ce ;
	}



/////////////////////////////
// plane basis attributes  //
/////////////////////////////
	
	//////////////////////////////////////////////////////////
	// PLANE BASIS GRADIENT EDITION INTERFACE				//
	//////////////////////////////////////////////////////////


	class CPlaneBasisGradientDlgWrapper : public CValueGradientDlgClientT<NL3D::CPlaneBasis, CBasisEdit>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
				
			NLMISC::CRGBA c1[] ={ NLMISC::CRGBA::Black, NLMISC::CRGBA::Black, NLMISC::CRGBA::Black } ;
			NLMISC::CRGBA c2[] ={ NLMISC::CRGBA::Green, NLMISC::CRGBA::Green, NLMISC::CRGBA::Red } ;

		
			// read plane basis
			NL3D::CPlaneBasis pb =  Scheme->getValue(index) ;

			CPoint center(x + 20, y + 25) ;
			
			NLMISC::CMatrix m ;			
			m.setRot(pb.X, pb.Y, pb.X ^ pb.Y) ;
			DrawBasisInDC(center, 12, m, *dc, c2) ;
		
		}
	} ;


	



	CAttribDlgPlaneBasis::CAttribDlgPlaneBasis(const std::string &valueID)  : CAttribDlgT<NL3D::CPlaneBasis>(valueID)
	{
	}

	uint CAttribDlgPlaneBasis::getNumScheme(void) const
	{
		return 2 ;
	}

	std::string CAttribDlgPlaneBasis::getSchemeName(uint index) const
	{
		nlassert(index < 2) ;
		switch (index)
		{			
			case 0:
				return std::string("basis gradient") ;
			break ;
			case 1:
				return std::string("follow path") ;
			break ;
			default:
				return std::string("") ;
			break ;
		}
	}



	void CAttribDlgPlaneBasis::editScheme(void)
	{	
		const NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = _SchemeWrapper->getScheme() ;	
		if (dynamic_cast<const NL3D::CPSPlaneBasisGradient *>(scheme)) 
		{
			CPlaneBasisGradientDlgWrapper wrapper ;
			wrapper.Scheme = &(((NL3D::CPSPlaneBasisGradient *) (_SchemeWrapper->getScheme()) )->_F) ;
			CValueGradientDlg gd(&wrapper, this) ;		
			wrapper.GradDlg = &gd ;
			wrapper.DefaultValue = NL3D::CPlaneBasis(NLMISC::CVector::K) ;
			wrapper.Id = std::string("PLANE_BASIS_GRADIENT") ;
			gd.DoModal() ;
		}

		if (dynamic_cast<const NL3D::CPSPlaneBasisFollowSpeed *>(scheme)) 
		{
			MessageBox("NO properties available", "edition", MB_OK) ;
		}
	
	}

	void CAttribDlgPlaneBasis::setCurrentScheme(uint index)
	{
		nlassert(index < 3) ;

		NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = NULL ;

		switch (index)
		{	
			case 0:	
				scheme = new NL3D::CPSPlaneBasisGradient ;
			break ;
				case 1:	
				scheme = new NL3D::CPSPlaneBasisFollowSpeed ;
			break ;
			default:	
			break ;
		}

		if (scheme)
		{
			_SchemeWrapper->setScheme(scheme) ;
		}
	}

	sint CAttribDlgPlaneBasis::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = _SchemeWrapper->getScheme() ;	

		if (dynamic_cast<const NL3D::CPSPlaneBasisGradient *>(scheme)) 
		{
			return 0 ;
		}

		if (dynamic_cast<const NL3D::CPSPlaneBasisFollowSpeed *>(scheme)) 
		{
			return 1 ;
		}
		
		
		return -1 ;
	}

	CEditAttribDlg *CAttribDlgPlaneBasis::createConstantValueDlg()
	{
		CBasisEdit *ce = new CBasisEdit(std::string("PLANE_BASIS_ATTRIB_EDIT")) ;

		ce->setWrapper(_Wrapper) ;
	
		return ce ;
	}



