// color_edit.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "color_edit.h"
#include "color_button.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorEdit dialog


CColorEdit::CColorEdit(CWnd* pParent /*=NULL*/)	
{
	//{{AFX_DATA_INIT(CColorEdit)
	//}}AFX_DATA_INIT
}


void CColorEdit::init(uint32 x, uint32 y, CWnd *pParent)
{
	Create(IDD_COLOR_EDIT, pParent) ;
	RECT r  ;
	GetClientRect(&r) ;

	m_RedCtrl.SetScrollRange(0, 255) ;
	m_GreenCtrl.SetScrollRange(0, 255);
	m_BlueCtrl.SetScrollRange(0, 255);

	MoveWindow(x, y, r.right, r.bottom) ;	
	updateColorFromReader() ;
	ShowWindow(SW_SHOW) ;	
	UpdateData(FALSE) ;
}


void CColorEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorEdit)
	DDX_Control(pDX, IDC_GREEN_AMOUNT, m_GreenCtrl);
	DDX_Control(pDX, IDC_BLUE_AMOUNT, m_BlueCtrl);
	DDX_Control(pDX, IDC_RED_AMOUNT, m_RedCtrl);
	DDX_Control(pDX, IDC_PARTICLE_COLOR, m_Color);
	//}}AFX_DATA_MAP
}


void CColorEdit::updateColorFromReader(void)
{
	if (_Reader)
	{
		CRGBA col = _Reader(_ReaderParam) ;
		m_RedCtrl.SetScrollPos(col.R) ;
		m_GreenCtrl.SetScrollPos(col.G) ;
		m_BlueCtrl.SetScrollPos(col.B) ;
		m_Color.setColor(col) ;
		UpdateData(FALSE) ;
	}
}



BEGIN_MESSAGE_MAP(CColorEdit, CDialog)
	//{{AFX_MSG_MAP(CColorEdit)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BROWSE_COLOR, OnBrowseColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorEdit message handlers

void CColorEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		UpdateData(TRUE) ;
		nlassert(_Writer&&_Reader) ;

		CRGBA col = _Reader(_ReaderParam) ;

		if (pScrollBar == &m_RedCtrl)
		{
			col.R = nPos ;
			m_RedCtrl.SetScrollPos(nPos) ;
		}
		else
		if (pScrollBar == &m_GreenCtrl)
		{
			col.G = nPos ;
			m_GreenCtrl.SetScrollPos(nPos) ;
		}
		else
		if (pScrollBar == &m_BlueCtrl)
		{
			col.B = nPos ;
			m_BlueCtrl.SetScrollPos(nPos) ;
		}	
		
		m_Color.setColor(col) ;
		_Writer(col, _WriterParam) ;
	
		UpdateData(FALSE) ;
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}

	
}

void CColorEdit::OnBrowseColor() 
{
	static COLORREF colTab[16] = { 0, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff
								   , 0x7f7f7f, 0xff7f7f, 0x7fff7f, 0xffff7f, 0x7f7fff, 0xff7fff, 0x7fffff, 0xff7f00 } ;
	nlassert(_Reader) ;
	CRGBA col = _Reader(_ReaderParam) ;
	CHOOSECOLOR cc ;
	cc.lStructSize = sizeof(CHOOSECOLOR) ;
	cc.hwndOwner = this->m_hWnd ;
	cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN  ;	
	cc.rgbResult = RGB(col.R, col.G, col.B) ;
	cc.lpCustColors = colTab ;

	if (::ChooseColor(&cc) == IDOK)
	{		
		col.R = (uint8) (cc.rgbResult & 0xff) ;
		col.G = (uint8) ((cc.rgbResult & 0xff00) >> 8) ;
		col.B = (uint8) ((cc.rgbResult & 0xff0000) >> 16) ;
		_Writer(col , _WriterParam) ;
		updateColorFromReader() ;
	}
}
