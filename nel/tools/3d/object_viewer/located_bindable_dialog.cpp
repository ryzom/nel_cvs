// located_bindable_dialog.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "located_bindable_dialog.h"


#include "nel/3d/ps_located.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_zone.h"

#include "attrib_dlg.h"


using NL3D::CPSLocatedBindable ; 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



////////////////////////////////////////////////////
// WRAPPERS to set/get scheme and constant values //
////////////////////////////////////////////////////


//////////
// size //
//////////
static float SizeReader(void *param) { return ((NL3D::CPSSizedParticle *) param)->getSize() ; }
static void  SizeWriter(float value, void *param) { ((NL3D::CPSSizedParticle *) param)->setSize(value) ; }
static NL3D::CPSAttribMaker<float> *SizeSchemeReader(void *param) { return ((NL3D::CPSSizedParticle *) param)->getSizeScheme() ; }
static void  SizeSchemeWriter(NL3D::CPSAttribMaker<float> *scheme, void *param) { ((NL3D::CPSSizedParticle *) param)->setSizeScheme(scheme) ; }

//////////////
// angle 2D //
//////////////

static float Angle2DReader(void *param) { return ((NL3D::CPSRotated2DParticle *) param)->getAngle2D() ; }
static void  Angle2DWriter(float value, void *param) { ((NL3D::CPSRotated2DParticle *) param)->setAngle2D(value) ; }
static NL3D::CPSAttribMaker<float> *Angle2DSchemeReader(void *param) { return ((NL3D::CPSRotated2DParticle *) param)->getAngle2DScheme() ; }
static void  Angle2DSchemeWriter(NL3D::CPSAttribMaker<float> *scheme, void *param) { ((NL3D::CPSRotated2DParticle *) param)->setAngle2DScheme(scheme) ; }

//////////////
// color    //
//////////////

static CRGBA ColorReader(void *param) { return ((NL3D::CPSColoredParticle *) param)->getColor() ; }
static void  ColorWriter(CRGBA value, void *param) { ((NL3D::CPSColoredParticle *) param)->setColor(value) ; }
static NL3D::CPSAttribMaker<CRGBA> *ColorSchemeReader(void *param) { return ((NL3D::CPSColoredParticle *) param)->getColorScheme() ; }
static void  ColorSchemeWriter(NL3D::CPSAttribMaker<CRGBA> *scheme, void *param) { ((NL3D::CPSColoredParticle *) param)->setColorScheme(scheme) ; }






/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog


CLocatedBindableDialog::CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable)
	: _Bindable(bindable)
{
	//{{AFX_DATA_INIT(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CLocatedBindableDialog::~CLocatedBindableDialog()
{
	for (std::vector<CDialog *>::iterator it = _SubDialogs.begin() ; it != _SubDialogs.end() ; ++it)
	{
		delete *it ;
	}
}

void CLocatedBindableDialog::init(CWnd* pParent)
{
	Create(IDD_LOCATED_BINDABLE, pParent) ;
	ShowWindow(SW_SHOW) ;

	uint yPos = 35 ;
	const uint xPos = 5 ;
	RECT rect ;


	if (dynamic_cast<NL3D::CPSParticle *>(_Bindable))
	{
		NL3D::CPSParticle *p = (NL3D::CPSParticle *) _Bindable ;

		// check support for color
		if (dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable))
		{
			
			CAttribDlgRGBA *ad = new CAttribDlgRGBA("PARTICLE_COLOR") ;
			_SubDialogs.push_back(ad) ;

			NL3D::CPSColoredParticle  *sp = dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable) ;

			ad->setReader(ColorReader, sp) ;
			ad->setWriter(ColorWriter, sp) ;
			ad->setSchemeReader(ColorSchemeReader, sp) ;
			ad->setSchemeWriter(ColorSchemeWriter, sp) ;
			

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}

		// check support for size
		if (dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_SIZE", 0.f, 10.f) ;
			_SubDialogs.push_back(ad) ;

			NL3D::CPSSizedParticle  *sp = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable) ;

			ad->setReader(SizeReader, sp) ;
			ad->setWriter(SizeWriter, sp) ;
			ad->setSchemeReader(SizeSchemeReader, sp) ;
			ad->setSchemeWriter(SizeSchemeWriter, sp) ;
			

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_SIZE)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}


		// check support for angle 2D
		if (dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_ANGLE2D", 0.f, 256.f) ;
			_SubDialogs.push_back(ad) ;

			NL3D::CPSSizedParticle  *sp = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable) ;

			ad->setReader(Angle2DReader, sp) ;
			ad->setWriter(Angle2DWriter, sp) ;
			ad->setSchemeReader(Angle2DSchemeReader, sp) ;
			ad->setSchemeWriter(Angle2DSchemeWriter, sp) ;
			

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_ANGLE)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}
	}


	
}


void CLocatedBindableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedBindableDialog, CDialog)
	//{{AFX_MSG_MAP(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog message handlers
