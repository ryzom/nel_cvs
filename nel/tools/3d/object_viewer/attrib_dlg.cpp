// attrib_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "attrib_dlg.h"
#include "editable_range.h"
#include "color_edit.h"
#include "value_blender_dlg.h"
#include "value_gradient_dlg.h"



#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_float.h"
#include "nel/3d/ps_int.h"
#include "nel/3d/ps_color.h"
#include "nel/3d/ps_plane_basis.h"


/////////////////////////////////////////////////////////////////////
// WRAPPERS to set / retrieve the NbCycles parameter of a scheme   //
/////////////////////////////////////////////////////////////////////
static float NbCyclesReader(void *lParam) { return ((CAttribDlg *) lParam)->getSchemeNbCycles() ; }
static void NbCyclesWriter(float value, void *lParam) { ((CAttribDlg *) lParam)->setSchemeNbCycles(value) ; }













#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAttribDlg dialog


CAttribDlg::CAttribDlg(const std::string &valueID) : _CstValueDlg(NULL)	
{
	//{{AFX_DATA_INIT(CAttribDlg)
	m_AttribName = _T("");
	m_Clamp = FALSE;
	//}}AFX_DATA_INIT
	_NbCyclesDlg = new CEditableRangeFloat(valueID + "%%NB_CYCLE_INFO", 0.1f, 10.1f) ;
}


CAttribDlg::~CAttribDlg()	
{
	delete _NbCyclesDlg ;
	if (_CstValueDlg)
	{
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
	
	m_ClampCtrl.EnableWindow(FALSE) ;
	_NbCyclesDlg->EnableWindow(FALSE) ;
	_NbCyclesDlg->emptyDialog() ;
	m_EditScheme.EnableWindow(FALSE) ;
	m_EditScheme.ShowWindow(SW_HIDE) ;
	m_Scheme.EnableWindow(FALSE) ;
	m_Scheme.ShowWindow(SW_HIDE) ;
	m_SchemeInput.EnableWindow(FALSE) ;
	m_SchemeInput.ShowWindow(SW_HIDE) ;


	m_UseScheme.SetCurSel(0) ;
	_CstValueDlg = createConstantValueDlg() ;
	CRect r, ro ;
	m_CstValuePos.GetWindowRect(&r) ;
	GetWindowRect(&ro) ;
	_CstValueDlg->init(r.left - ro.left, r.top - ro.top, this) ;
	UpdateData(FALSE) ;
}

void CAttribDlg::schemeValueUpdate()
{
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


	_NbCyclesDlg->setReader(NbCyclesReader, this) ;
	_NbCyclesDlg->setWriter(NbCyclesWriter, this) ;
	_NbCyclesDlg->updateRange() ;
	_NbCyclesDlg->updateValueFromReader() ;


	m_ClampCtrl.EnableWindow(isClampingSupported()) ;
	if (isClampingSupported())
	{
		m_Clamp = isSchemeClamped() ;
	}


	UpdateData(FALSE) ;
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
// WRAPPERS FOR THE FLOAT BLENDING EDITION DIALOG       ///
//////////////////////////////////////////////////////////


struct BlendFloatCallBackInfo
{
	// Float 0 or 1 being edited
	uint FloatIndex ;
	// the scheme being edited
	NL3D::CPSFloatBlender *Scheme ;
} ;

/// allow the blend dialog to read a Float from the scheme
float BlendFloatDlgReader(void *lParam)
{
	float c1, c2 ;
	BlendFloatCallBackInfo *bccbi = (BlendFloatCallBackInfo *) lParam ;
	((BlendFloatCallBackInfo *) lParam)->Scheme->_F.getValues(c1, c2) ;
	return (bccbi->FloatIndex == 0) ? c1 : c2 ;
}

/// allow the blend dialog to write a Float to the scheme
void BlendFloatDlgWriter(float col, void *lParam)
{
	float c1, c2 ;
	BlendFloatCallBackInfo *bccbi = (BlendFloatCallBackInfo *) lParam ;
	bccbi->Scheme->_F.getValues(c1, c2) ;
	if (bccbi->FloatIndex == 0)
	{
		c1 = col ;
	}
	else
	{
		c2 = col ;
	}
	bccbi->Scheme->_F.setValues(c1, c2) ;
}

/** create the sub dialog (one per Float) for the blending edition dialog 
 */
  
CEditAttribDlg *  BlendFloatDlgCreator(uint index, void *lParam, void **toDelete) 
{ 
	nlassert(lParam) ;	
	BlendFloatCallBackInfo *bccbi = new BlendFloatCallBackInfo ;
	bccbi->FloatIndex = index ;
	bccbi->Scheme = (NL3D::CPSFloatBlender *) lParam ;

	CEditableRangeFloat *dlg = new CEditableRangeFloat(std::string("float blender"), 0.1f, 10.1f) ;
	dlg->setWriter(BlendFloatDlgWriter, bccbi ) ;
	dlg->setReader(BlendFloatDlgReader, bccbi ) ;
	*toDelete = bccbi ;
	return dlg ;
}


CAttribDlgFloat::CAttribDlgFloat(const std::string &valueID, float minRange, float maxRange)
			:  CAttribDlgT<float>(valueID), _MinRange(minRange), _MaxRange(maxRange)			  
{
		
}

CEditAttribDlg *CAttribDlgFloat::createConstantValueDlg()
{
	CEditableRangeFloat *erf = new CEditableRangeFloat(_CstValueId, _MinRange, _MaxRange) ;
	erf->setReader(_Reader, _ReaderParam) ;
	erf->setWriter(_Writer, _WriterParam) ;
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
	const NL3D::CPSAttribMaker<float> *scheme = _SchemeReader(_SchemeReaderParam) ;	

	if (dynamic_cast<const NL3D::CPSFloatBlender *>(scheme)) 
	{				
		CValueBlenderDlg *bd = new CValueBlenderDlg(BlendFloatDlgCreator, _SchemeReader(_SchemeReaderParam), this) ;
		bd->DoModal() ;
	
	}
	if (dynamic_cast<const NL3D::CPSFloatGradient *>(scheme)) 
	{
		
	}
	
}

sint CAttribDlgFloat::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<float> *scheme = _SchemeReader(_SchemeReaderParam) ;	

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
		_SchemeWriter(scheme, _SchemeWriterParam) ;
	}
}



///////////////////////
// CRGBA attributes  //
///////////////////////

//////////////////////////////////////////////////////////
// WRAPPERS FOR THE COLOR BLENDING EDITION DIALOG       ///
//////////////////////////////////////////////////////////


struct BlendColorCallBackInfo
{
	// Color 0 or 1 being edited
	uint ColorIndex ;
	// the scheme being edited
	NL3D::CPSColorBlender *Scheme ;
} ;

/// allow the blend dialog to read a Color from the scheme
static CRGBA BlendColorDlgReader(void *lParam)
{
	CRGBA c1, c2 ;
	BlendColorCallBackInfo *bccbi = (BlendColorCallBackInfo *) lParam ;
	((BlendColorCallBackInfo *) lParam)->Scheme->_F.getValues(c1, c2) ;
	return (bccbi->ColorIndex == 0) ? c1 : c2 ;
}

/// allow the blend dialog to write a Color to the scheme
static void BlendColorDlgWriter(CRGBA col, void *lParam)
{
	CRGBA c1, c2 ;
	BlendColorCallBackInfo *bccbi = (BlendColorCallBackInfo *) lParam ;
	bccbi->Scheme->_F.getValues(c1, c2) ;
	if (bccbi->ColorIndex == 0)
	{
		c1 = col ;
	}
	else
	{
		c2 = col ;
	}
	bccbi->Scheme->_F.setValues(c1, c2) ;	
}

/** create the sub dialog (one per Color) for the blending Blendion dialog 
 */
  
CEditAttribDlg *  BlendColorDlgCreator(uint index, void *lParam, void **infoToDelete) 
{ 
	nlassert(lParam) ;	
	BlendColorCallBackInfo *bccbi = new BlendColorCallBackInfo ;
	bccbi->ColorIndex = index ;
	bccbi->Scheme = (NL3D::CPSColorBlender *) lParam ;

	CColorEdit *dlg = new CColorEdit ;
	dlg->setWriter(BlendColorDlgWriter, bccbi ) ;
	dlg->setReader(BlendColorDlgReader, bccbi ) ;

	*infoToDelete = bccbi ;

	return dlg ;
}

//////////////////////////////////////////////////////////
// WRAPPERS FOR THE GRADIENT EDITION DIALOG            ///
//////////////////////////////////////////////////////////

struct GradientColorCallBackInfo
{
	// Color 0 or 1 being edited
	uint ColorIndex ;
	// the scheme being edited
	NL3D::CPSColorGradient *Scheme ;

	// the gradient dialog
	CValueGradientDlg *GradDlg ;
} ;


/// allow the blend dialog to read a Color from the scheme
static CRGBA GradientColorDlgReader(void *lParam)
{	
	GradientColorCallBackInfo *bccbi = (GradientColorCallBackInfo *) lParam ;
	return bccbi->Scheme->_F.getValue(bccbi->ColorIndex) ;
}
static void GradientColorDlgWriter(CRGBA col, void *lParam)
{
	GradientColorCallBackInfo *bccbi = (GradientColorCallBackInfo *) lParam ;
	CRGBA *tab = new CRGBA[bccbi->Scheme->_F.getNumValues()] ;
	bccbi->Scheme->_F.getValues(tab) ;
	tab[bccbi->ColorIndex] = col ;
	bccbi->Scheme->_F.setValues(tab, bccbi->Scheme->_F.getNumValues(), bccbi->Scheme->_F.getNumstages()) ;
	delete[] tab ;
	bccbi->GradDlg->invalidateGrad() ;
}
static CEditAttribDlg *GradientColorDlgCreator(uint index, void *lParam, void **infoToDelete, CValueGradientDlg *gradDialog)
{
	nlassert(lParam) ;	
	GradientColorCallBackInfo *bccbi = new GradientColorCallBackInfo ;
	bccbi->ColorIndex = index ;
	bccbi->Scheme = (NL3D::CPSColorGradient *) lParam ;
	CColorEdit *dlg = new CColorEdit ;
	dlg->setWriter(GradientColorDlgWriter, bccbi ) ;
	dlg->setReader(GradientColorDlgReader, bccbi ) ;
	bccbi->GradDlg = gradDialog ;
	*infoToDelete = bccbi ;
	return dlg ;	
}
void GradientColorModify(CValueGradientDlg::TAction action, uint index, void *lParam)
{
	NL3D::CPSValueGradientFunc<CRGBA> &scheme = ((NL3D::CPSValueGradient<CRGBA> *) lParam)->_F ;
	CRGBA *tab = new CRGBA[scheme.getNumValues() + 1] ; // +1 is for the add / insert case
	scheme.getValues(tab) ;

	switch(action)
	{
		case CValueGradientDlg::Add:
			tab[scheme.getNumValues()] = CRGBA::White ;
			scheme.setValues(tab, scheme.getNumValues() + 1, scheme.getNumstages()) ;
		break ;
		case CValueGradientDlg::Insert:
			memcpy(tab + (index + 1), tab + index, sizeof(CRGBA) * (scheme.getNumValues() - index)) ;
			tab[index] = CRGBA::White ;
			scheme.setValues(tab, scheme.getNumValues() + 1, scheme.getNumstages()) ;
		break ;
		case CValueGradientDlg::Delete:
			memcpy(tab + index, tab + index + 1, sizeof(CRGBA) * (scheme.getNumValues() - index - 1)) ;
			scheme.setValues(tab, scheme.getNumValues() - 1, scheme.getNumstages()) ;
		break ;
	}
	delete[] tab ;
}
void GradientColorDisplay(CDC *dc, uint index, sint x, sint y, void *lParam)
{

	NL3D::CPSValueGradientFunc<CRGBA> &scheme = ((NL3D::CPSValueGradient<CRGBA> *) lParam)->_F ;

	CRGBA col = scheme.getValue(index) ;

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
	const NL3D::CPSAttribMaker<CRGBA> *scheme = _SchemeReader(_SchemeReaderParam) ;	

	if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme)) 
	{				
		CValueBlenderDlg *bd = new CValueBlenderDlg(BlendColorDlgCreator, _SchemeReader(_SchemeReaderParam), this) ;
		bd->DoModal() ;
	
	}
	if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme)) 
	{
		NL3D::CPSColorGradient *scheme = (NL3D::CPSColorGradient *) _SchemeReader(_SchemeReaderParam) ;
		CValueGradientDlg *gd = new CValueGradientDlg(GradientColorDlgCreator, GradientColorModify, GradientColorDisplay
													  , scheme, scheme, scheme
													  , scheme->_F.getNumValues()
													  , this) ;		
		gd->DoModal() ;
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
		_SchemeWriter(scheme, _SchemeWriterParam) ;
	}
}

sint CAttribDlgRGBA::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<CRGBA> *scheme = _SchemeReader(_SchemeReaderParam) ;	

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
	CColorEdit *ce = new CColorEdit ;

	ce->setReader(_Reader, _ReaderParam) ;
	ce->setWriter(_Writer, _WriterParam) ;
	return ce ;
}


