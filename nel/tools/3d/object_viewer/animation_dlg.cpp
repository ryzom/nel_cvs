// animation_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "animation_dlg.h"
#include <math.h>
#include <nel/misc/time_nl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NLMISC;
using namespace NL3D;

#define SPEED_FOREWARD 3

/////////////////////////////////////////////////////////////////////////////
// CAnimationDlg dialog


CAnimationDlg::CAnimationDlg(class CObjectViewer* main, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationDlg)
	End = 0.0f;
	Speed = 30.0f;
	Start = 0.0f;
	Loop = TRUE;
	UICurrentFrame = 0;
	CurrentFrame = 0.0f;
	//}}AFX_DATA_INIT
	Playing=false;
	Main=main;
}


void CAnimationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationDlg)
	DDX_Control(pDX, IDC_FRW, FRWCtrl);
	DDX_Control(pDX, IDC_FFW, FFWCtrl);
	DDX_Control(pDX, IDC_TIME_LINE, TimeLineCtrl);
	DDX_Control(pDX, IDC_PLAY, PlayCtrl);
	DDX_Control(pDX, IDC_STOP, StopCtrl);
	DDX_Text(pDX, IDC_END_EDIT, End);
	DDX_Text(pDX, IDC_SPEED, Speed);
	DDX_Text(pDX, IDC_START_EDIT, Start);
	DDX_Check(pDX, IDC_LOOP, Loop);
	DDX_Text(pDX, IDC_CURRENT_FRAME, UICurrentFrame);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimationDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimationDlg)
	ON_BN_CLICKED(IDC_END, OnEnd)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_EN_CHANGE(IDC_CURRENT_FRAME, OnChangeCurrentFrame)
	ON_EN_CHANGE(IDC_END_EDIT, OnChangeEndEdit)
	ON_EN_CHANGE(IDC_SPEED, OnChangeSpeed)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_EN_CHANGE(IDC_START_EDIT, OnChangeStartEdit)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationDlg message handlers

void CAnimationDlg::OnEnd() 
{
	UpdateData ();
	CurrentFrame=End;
	UICurrentFrame=(int)CurrentFrame;
	UpdateData (FALSE);
	updateBar ();
}

void CAnimationDlg::OnPlay() 
{
	// play
	UpdateData ();
	StopCtrl.SetCheck (0);
	PlayCtrl.SetCheck (1);
	PlayCtrl.EnableWindow (FALSE);
	StopCtrl.EnableWindow (TRUE);

	if (!Playing)
	{
		Playing=true;
	}
}

void CAnimationDlg::OnStop() 
{
	// Is checked ?
	UpdateData ();
	Playing=false;
	StopCtrl.SetCheck (1);
	PlayCtrl.SetCheck (0);
	PlayCtrl.EnableWindow (TRUE);
	StopCtrl.EnableWindow (FALSE);
	UpdateData (FALSE);
}

void CAnimationDlg::OnChangeCurrentFrame() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// Update values
	UpdateData ();
	
	// Clamp current frame
	clamp (UICurrentFrame, (int)Start, (int)End);
	CurrentFrame=(float)UICurrentFrame;

	// Update
	updateBar ();
	UpdateData (FALSE);	
}

void CAnimationDlg::OnChangeEndEdit() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	// Update values
	UpdateData ();
	
	// Clamp current frame
	if (End<Start)
		Start=End;
	if (End<CurrentFrame)
		CurrentFrame=End;
	UICurrentFrame=(int)CurrentFrame;

	// Update
	UpdateData (FALSE);	
	Main->setAnimTime (Start, End);
}

void CAnimationDlg::OnChangeSpeed() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	UpdateData();
	if (Speed<=0.001f)
		Speed=0.001f;
	Start=Speed*Start;
	End=Speed*End;
	UpdateData(FALSE);
	Main->setAnimTime (Start, End);
}

void CAnimationDlg::OnStart() 
{
	UpdateData ();
	CurrentFrame=Start;
	UICurrentFrame=(int)CurrentFrame;
	UpdateData (FALSE);
	updateBar ();
}

void CAnimationDlg::OnChangeStartEdit() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// Update values
	UpdateData ();
	
	// Clamp current frame
	if (End<Start)
		End=Start;
	if (CurrentFrame<Start)
		CurrentFrame=Start;
	UICurrentFrame=(int)CurrentFrame;

	// Update
	UpdateData (FALSE);	
	Main->setAnimTime (Start, End);
}

void CAnimationDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default

	// Drag the time line ?
	if (pScrollBar==GetDlgItem (IDC_TIME_LINE))
	{
		// Update values
		UpdateData ();

		// Setup current pos
		CurrentFrame=(float)TimeLineCtrl.GetPos()*(End-Start)/65535.f+Start;
		CurrentFrame=(float)floor(CurrentFrame+0.5f);
		UICurrentFrame=(int)CurrentFrame;

		// Update values
		UpdateData (FALSE);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAnimationDlg::handle ()
{
	// New time
	uint64 newTime=NLMISC::CTime::getLocalTime ();

	// Forward or rewind ?
	bool forward=(((CButton*)GetDlgItem (IDC_FFW))->GetState()&BST_PUSHED)!=0;
	bool rewind=(((CButton*)GetDlgItem (IDC_FRW))->GetState()&BST_PUSHED)!=0;

	if (forward||rewind||Playing)
	{
		UpdateData ();

		// Delta time
		uint deltaTime=(uint)(newTime-LastTime);

		if (forward)
			// Fast forward
			CurrentFrame+=(float)((float)deltaTime*SPEED_FOREWARD*Speed/1000.0);
		else if (rewind)
			// Fast rewind
			CurrentFrame-=(float)((float)deltaTime*SPEED_FOREWARD*Speed/1000.0);
		else
		{
			// Compute new time
			if (Playing)
				CurrentFrame+=(float)((float)deltaTime*Speed/1000.0);
		}

		// Loop
		if (Loop&&(!forward)&&(!rewind)&&Playing)
			CurrentFrame=(float)fmod ((CurrentFrame-Start), End-Start)+Start;

		// Clamp time
		if (CurrentFrame>End)
		{
			CurrentFrame=End;
			UpdateData (FALSE);

			// Stop animation
			OnStop ();
		}
		if (CurrentFrame<Start)
		{
			CurrentFrame=Start;
			UpdateData (FALSE);

			// Stop animation
			OnStop ();
		}

		UICurrentFrame=(int)CurrentFrame;
		UpdateData (FALSE);
		updateBar ();
	}

	// Setup new time
	LastTime=newTime;
}

BOOL CAnimationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Stop
	LastTime=NLMISC::CTime::getLocalTime ();
	StopCtrl.SetCheck (1);

	// Set time line range
	TimeLineCtrl.SetRangeMin (0);
	TimeLineCtrl.SetRangeMax (65535);

	// Update the time line
	updateBar ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimationDlg::setAnimTime (float animStart, float animEnd)
{
	UpdateData();
	Start=animStart;
	End=animEnd;
	UpdateData(FALSE);
	updateBar ();
}

void CAnimationDlg::updateBar ()
{
	// Update value
	UpdateData();
	
	// Set cursor position
	int position;
	if (fabs (End-Start)<0.00001f)
		position=0;
	else
		position=(int)((CurrentFrame-Start)*65535.f/(End-Start));
	clamp (position, 0, 65535);
	TimeLineCtrl.SetPos(position);

	// Update value
	UpdateData (FALSE);
}

NL3D::CAnimationTime CAnimationDlg::getTime ()
{
	// Return current time in second
	return CurrentFrame/Speed;
}
