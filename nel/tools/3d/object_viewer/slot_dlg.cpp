// slot_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "select_string.h"
#include "slot_dlg.h"
#include <nel/misc/common.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DELTA_BLEND 0.1f
#define DELTA_TIME (getTimeIncrement ())
#define DELTA_MUL 0.1f

using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CSlotDlg dialog


CSlotDlg::CSlotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSlotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSlotDlg)
	EndBlend = 0.0f;
	EndTime = 0.0f;
	Smoothness = 0.0f;
	SpeedFactor = 1.0f;
	StartBlend = 0.0f;
	StartTime = 0.0f;
	Offset = 0.0f;
	ClampMode = 0;
	StartAnimTime=0.f;
	EndAnimTime=1.f;
	SkeletonWeightInverted = FALSE;
	//}}AFX_DATA_INIT
	AnimationSet=NULL;
	Animation=NULL;
	SkeletonWeightTemplate=NULL;
	MainDlg=NULL;
}

void CSlotDlg::init (uint id, NL3D::CAnimationSet* animationSet, CObjectViewer* mainDlg)
{
	AnimationSet=animationSet;
	Id=id;
	MainDlg=mainDlg;
}

void CSlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSlotDlg)
	DDX_Control(pDX, IDC_INVERT_SKELETON_WEIGHT, InvertSkeletonWeightCtrl);
	DDX_Control(pDX, IDC_ALIGN_BLEND, AlignBlendCtrl);
	DDX_Control(pDX, IDC_CLAMP, ClampCtrl);
	DDX_Control(pDX, IDC_SCROLLBAR, ScrollBarCtrl);
	DDX_Control(pDX, IDC_OFFSET_SPIN, OffsetSpinCtrl);
	DDX_Control(pDX, IDC_OFFSET, OffsetCtrl);
	DDX_Control(pDX, IDC_START_TIME_SPIN, StartTimeSpinCtrl);
	DDX_Control(pDX, IDC_START_BLEND_SPIN, StartBlendSpinCtrl);
	DDX_Control(pDX, IDC_SPEED_FACTOR_SPIN, SpeedFactorSpinCtrl);
	DDX_Control(pDX, IDC_SMOOTHNESS_SPIN, SmoothnessSpinCtrl);
	DDX_Control(pDX, IDC_END_TIME_SPIN, EndTimeSpinCtrl);
	DDX_Control(pDX, IDC_END_BLEND_SPIN, EndBlendSpinCtrl);
	DDX_Control(pDX, IDC_START_TIME, StartTimeCtrl);
	DDX_Control(pDX, IDC_START_BLEND, StartBlendCtrl);
	DDX_Control(pDX, IDC_SPEED_FACTOR, SpeddFactorCtrl);
	DDX_Control(pDX, IDC_SMOOTHNESS, SmoothnessCtrl);
	DDX_Control(pDX, IDC_END_TIME, EndTimeCtrl);
	DDX_Control(pDX, IDC_END_BLEND, EndBlendCtrl);
	DDX_Text(pDX, IDC_END_BLEND, EndBlend);
	DDX_Text(pDX, IDC_END_TIME, EndTime);
	DDX_Text(pDX, IDC_SMOOTHNESS, Smoothness);
	DDV_MinMaxFloat(pDX, Smoothness, 0.f, 1.f);
	DDX_Text(pDX, IDC_SPEED_FACTOR, SpeedFactor);
	DDV_MinMaxFloat(pDX, SpeedFactor, 1.e-002f, 100.f);
	DDX_Text(pDX, IDC_START_BLEND, StartBlend);
	DDV_MinMaxFloat(pDX, StartBlend, 0.f, 1.f);
	DDX_Text(pDX, IDC_START_TIME, StartTime);
	DDX_Text(pDX, IDC_OFFSET, Offset);
	DDX_Radio(pDX, IDC_CLAMP, ClampMode);
	DDX_Check(pDX, IDC_INVERT_SKELETON_WEIGHT, SkeletonWeightInverted);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSlotDlg, CDialog)
	//{{AFX_MSG_MAP(CSlotDlg)
	ON_WM_PAINT()
	ON_NOTIFY(UDN_DELTAPOS, IDC_END_BLEND_SPIN, OnDeltaposEndBlendSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_END_TIME_SPIN, OnDeltaposEndTimeSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SMOOTHNESS_SPIN, OnDeltaposSmoothnessSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPEED_FACTOR_SPIN, OnDeltaposSpeedFactorSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_START_BLEND_SPIN, OnDeltaposStartBlendSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_START_TIME_SPIN, OnDeltaposStartTimeSpin)
	ON_EN_CHANGE(IDC_END_BLEND, OnChangeEndBlend)
	ON_EN_CHANGE(IDC_END_TIME, OnChangeEndTime)
	ON_EN_CHANGE(IDC_SMOOTHNESS, OnChangeSmoothness)
	ON_EN_CHANGE(IDC_SPEED_FACTOR, OnChangeSpeedFactor)
	ON_EN_CHANGE(IDC_START_BLEND, OnChangeStartBlend)
	ON_EN_CHANGE(IDC_START_TIME, OnChangeStartTime)
	ON_BN_CLICKED(IDC_SET_ANIMATION, OnSetAnimation)
	ON_BN_CLICKED(IDC_SET_SKELETON, OnSetSkeleton)
	ON_EN_CHANGE(IDC_OFFSET, OnChangeOffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_OFFSET_SPIN, OnDeltaposOffsetSpin)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CLAMP, OnClamp)
	ON_BN_CLICKED(IDC_REPEAT, OnRepeat)
	ON_BN_CLICKED(IDC_DISABLE, OnDisable)
	ON_BN_CLICKED(IDC_ALIGN_BLEND, OnAlignBlend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlotDlg message handlers

BOOL CSlotDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Init the blend window
	setWindowName ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CSlotDlg::OnPaint() 
{
	//CPaintDC dc(this); // device context for painting
	PAINTSTRUCT paint;
	CDC* pDc=BeginPaint(&paint);
	
	// TODO: Add your message handler code here
	// Draw the blend
	RECT rect;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&rect);
	ScreenToClient (&rect);
	Blend.OnPaint (rect, pDc, StartBlend, EndBlend, StartTime, EndTime, 
		Smoothness, StartAnimTime, EndAnimTime, !isEmpty());
	
	// Do not call CDialog::OnPaint() for painting messages
	EndPaint(&paint);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposEndBlendSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		EndBlend+=DELTA_BLEND;
	if (pNMUpDown->iDelta>0)
		EndBlend-=DELTA_BLEND;
	clamp (EndBlend, 0.f, 1.f);

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposEndTimeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		EndTime+=DELTA_TIME;
	if (pNMUpDown->iDelta>0)
		EndTime-=DELTA_TIME;
	clamp (EndTime, StartAnimTime, EndAnimTime);
	if (EndTime<StartTime)
		StartTime=EndTime;

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposSmoothnessSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		Smoothness+=DELTA_BLEND;
	if (pNMUpDown->iDelta>0)
		Smoothness-=DELTA_BLEND;
	clamp (Smoothness, 0.f, 1.f);

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposSpeedFactorSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		SpeedFactor+=DELTA_MUL;
	if (pNMUpDown->iDelta>0)
		SpeedFactor-=DELTA_MUL;
	clamp (SpeedFactor, 0.01f, 100.f);

	UpdateData (FALSE);

	validateTime ();
	updateScrollBar ();

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposStartBlendSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		StartBlend+=DELTA_BLEND;
	if (pNMUpDown->iDelta>0)
		StartBlend-=DELTA_BLEND;
	clamp (StartBlend, 0.f, 1.f);

	UpdateData (FALSE);
	
	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposStartTimeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		StartTime+=DELTA_TIME;
	if (pNMUpDown->iDelta>0)
		StartTime-=DELTA_TIME;
	clamp (StartTime, StartAnimTime, EndAnimTime);
	if (EndTime<StartTime)
		EndTime=StartTime;

	UpdateData (FALSE);
	
	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnChangeEndBlend() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (EndBlend, 0.f, 1.f);

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnChangeEndTime() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (EndTime, StartAnimTime, EndAnimTime);
	if (EndTime<StartTime)
		StartTime=EndTime;

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnChangeSmoothness() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (Smoothness, 0.f, 1.f);

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnChangeSpeedFactor() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (SpeedFactor, 0.01f, 100.f);

	UpdateData (FALSE);
	validateTime ();
	updateScrollBar ();
}

// ***************************************************************************

void CSlotDlg::OnChangeStartBlend() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (StartBlend, 0.f, 1.f);

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnChangeStartTime() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (StartTime, StartAnimTime, EndAnimTime);
	if (EndTime<StartTime)
		EndTime=StartTime;

	UpdateData (FALSE);

	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
	
}

// ***************************************************************************

void CSlotDlg::OnSetAnimation() 
{
	// List of string
	std::vector<std::string> vectString;

	// Build a list of string
	for (uint a=0; a<AnimationSet->getNumAnimation (); a++)
		vectString.push_back (AnimationSet->getAnimationName (a));

	// Select a string
	CSelectString select (vectString, "Select your animation", this);
	if (select.DoModal ()==IDOK)
	{
		// Set the animation
		if (select.Selection!=-1)
		{
			setAnimation (select.Selection, AnimationSet->getAnimation (select.Selection), vectString[select.Selection].c_str());
			validateTime ();
			updateScrollBar ();
		}
		else
			setAnimation (0, NULL, NULL);

		setWindowName ();
		Invalidate ();
	}
}

// ***************************************************************************

void CSlotDlg::OnSetSkeleton() 
{
	// List of string
	std::vector<std::string> vectString;

	// Build a list of string
	for (uint s=0; s<AnimationSet->getNumSkeletonWeight (); s++)
		vectString.push_back (AnimationSet->getSkeletonWeightName (s));

	// Select a string
	CSelectString select (vectString, "Select your skeleton weight template", this);
	if (select.DoModal ()==IDOK)
	{
		// Set the animation
		if (select.Selection!=-1)
			setSkeletonTemplateWeight (select.Selection, AnimationSet->getSkeletonWeight (select.Selection));
		else
			setSkeletonTemplateWeight (0, NULL);

		setWindowName ();
		Invalidate ();
	}
}

// ***************************************************************************

void CSlotDlg::setWindowName ()
{
	char tmp[512];
	_snprintf (tmp, 512, "Slot %d : ", Id);
	if (isEmpty())
		strcat (tmp, "empty");
	else
		strcat (tmp, AnimationName.c_str());
	GetDlgItem (IDC_SLOT_NAME)->SetWindowText (tmp);
}

// ***************************************************************************

void CSlotDlg::OnChangeOffset() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	validateTime ();
	updateScrollBar ();
}

// ***************************************************************************

void CSlotDlg::OnDeltaposOffsetSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here

	UpdateData ();
	if (pNMUpDown->iDelta<0)
		Offset+=getTimeIncrement ();
	if (pNMUpDown->iDelta>0)
		Offset-=getTimeIncrement ();
	UpdateData (FALSE);
	validateTime ();
	updateScrollBar ();

	*pResult = 0;
}

// ***************************************************************************

void CSlotDlg::validateTime ()
{
	bool reinit=false;
	if (Offset<StartAnimTime)
	{
		StartAnimTime=Offset;
		reinit=true;
	}
	if (Offset+AnimationLength/SpeedFactor>EndAnimTime)
	{
		EndAnimTime=Offset+AnimationLength/SpeedFactor;
		reinit=true;
	}
	if (reinit)
		MainDlg->setAnimTime (StartAnimTime, EndAnimTime);
}

// ***************************************************************************

void CSlotDlg::updateScrollBar ()
{
	// Scroll info
	SCROLLINFO info;
	memset (&info, 0, sizeof (info));

	// Fill the infos
	info.fMask=SIF_ALL;
	info.nMin=0;
	info.nMax=10000;
	info.nPage=(int)(10000.f*(AnimationLength/SpeedFactor)/(EndAnimTime-StartAnimTime));
	info.nPos=(int)(10000.f*(Offset-StartAnimTime)/(EndAnimTime-StartAnimTime));

	// Set scrollbar infos
	ScrollBarCtrl.SetScrollInfo (&info, TRUE);

	// Invalidate blend bar
	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	if (pScrollBar==GetDlgItem (IDC_SCROLLBAR))
	{
		// Only drag and drop
		if (nSBCode==SB_THUMBTRACK)
		{
			UpdateData ();
			Offset=(EndAnimTime-StartAnimTime)*(float)nPos/10000.f+StartAnimTime;
			UpdateData (FALSE);
			updateScrollBar ();
		}
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ***************************************************************************

void CSlotDlg::setAnimTime (float animStart, float animEnd)
{
	StartAnimTime=animStart;
	EndAnimTime=animEnd;
	updateScrollBar ();
}

// ***************************************************************************

float CSlotDlg::getTimeIncrement ()
{
	return (EndAnimTime-StartAnimTime)/100.f;
}

// ***************************************************************************

void CSlotDlg::OnClamp() 
{
	// TODO: Add your control notification handler code here
	
}

// ***************************************************************************

void CSlotDlg::OnRepeat() 
{
	// TODO: Add your control notification handler code here
	
}

// ***************************************************************************

void CSlotDlg::OnDisable() 
{
	// TODO: Add your control notification handler code here
	
}

// ***************************************************************************

void CSlotDlg::OnAlignBlend() 
{
	// Refresh data
	UpdateData ();

	// Change some of them
	StartTime=Offset;
	EndTime=Offset+AnimationLength/SpeedFactor;

	// Invalidate UI
	UpdateData (FALSE);

	// Invalidate blend bar
	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

