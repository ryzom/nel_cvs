#if !defined(AFX_SLOT_DLG_H__9B22CB8B_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SLOT_DLG_H__9B22CB8B_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// slot_dlg.h : header file
//

#include "blend_wnd.h"
#include <nel/3d/animation_set.h>
#include <nel/misc/debug.h>

/////////////////////////////////////////////////////////////////////////////
// CSlotDlg dialog

class CSlotDlg : public CDialog
{
// Construction
public:
	CSlotDlg(CWnd* pParent = NULL);   // standard constructor

	void init (uint id, NL3D::CAnimationSet* animationSet, class CObjectViewer* mainDlg);

	void setAnimation (uint animationId, NL3D::CAnimation *animation, const char* name)
	{
		Animation=animation;
		AnimationId=animationId;
		if (animation)
		{
			nlassert (name);
			AnimationName=name;
			AnimationLength=animation->getEndTime()-animation->getBeginTime();
		}
		OffsetCtrl.EnableWindow (Animation!=NULL);
		StartTimeCtrl.EnableWindow (Animation!=NULL);
		StartBlendCtrl.EnableWindow (Animation!=NULL);
		SpeddFactorCtrl.EnableWindow (Animation!=NULL);
		SmoothnessCtrl.EnableWindow (Animation!=NULL);
		EndTimeCtrl.EnableWindow (Animation!=NULL);
		EndBlendCtrl.EnableWindow (Animation!=NULL);
		OffsetSpinCtrl.EnableWindow (Animation!=NULL);
		StartTimeSpinCtrl.EnableWindow (Animation!=NULL);
		StartBlendSpinCtrl.EnableWindow (Animation!=NULL);
		SpeedFactorSpinCtrl.EnableWindow (Animation!=NULL);
		SmoothnessSpinCtrl.EnableWindow (Animation!=NULL);
		EndTimeSpinCtrl.EnableWindow (Animation!=NULL);
		EndBlendSpinCtrl.EnableWindow (Animation!=NULL);
		ScrollBarCtrl.EnableWindow (Animation!=NULL);
		AlignBlendCtrl.EnableWindow (Animation!=NULL);
		InvertSkeletonWeightCtrl.EnableWindow (Animation!=NULL);
		GetDlgItem (IDC_CLAMP)->EnableWindow (Animation!=NULL);
		GetDlgItem (IDC_REPEAT)->EnableWindow (Animation!=NULL);
		GetDlgItem (IDC_DISABLE)->EnableWindow (Animation!=NULL);
		setWindowName ();
	}

	void setSkeletonTemplateWeight (uint skeletonWeightId, NL3D::CSkeletonWeight *skeleton)
	{
		SkeletonWeightTemplate=skeleton;
		SkeletonWeightId=skeletonWeightId;
	}

	bool isEmpty ()
	{
		return Animation==NULL;
	}

	void setWindowName ();
	void validateTime ();
	void updateScrollBar ();
	void setAnimTime (float animStart, float animEnd);
	float getTimeIncrement ();

	// A CBlendWnd
	uint					Id;
	uint					AnimationId;
	uint					SkeletonWeightId;
	CBlendWnd				Blend;
	NL3D::CAnimationSet*	AnimationSet;
	NL3D::CAnimation*		Animation;
	std::string				AnimationName;
	NL3D::CSkeletonWeight*	SkeletonWeightTemplate;
	CObjectViewer*			MainDlg;

// Dialog Data
	//{{AFX_DATA(CSlotDlg)
	enum { IDD = IDD_SLOT };
	CButton	InvertSkeletonWeightCtrl;
	CButton	AlignBlendCtrl;
	CButton	ClampCtrl;
	CScrollBar	ScrollBarCtrl;
	CSpinButtonCtrl	OffsetSpinCtrl;
	CEdit	OffsetCtrl;
	CSpinButtonCtrl	StartTimeSpinCtrl;
	CSpinButtonCtrl	StartBlendSpinCtrl;
	CSpinButtonCtrl	SpeedFactorSpinCtrl;
	CSpinButtonCtrl	SmoothnessSpinCtrl;
	CSpinButtonCtrl	EndTimeSpinCtrl;
	CSpinButtonCtrl	EndBlendSpinCtrl;
	CEdit	StartTimeCtrl;
	CEdit	StartBlendCtrl;
	CEdit	SpeddFactorCtrl;
	CEdit	SmoothnessCtrl;
	CEdit	EndTimeCtrl;
	CEdit	EndBlendCtrl;
	float	EndBlend;
	float	EndTime;
	float	Smoothness;
	float	SpeedFactor;
	float	StartBlend;
	float	StartTime;
	float	Offset;
	int		ClampMode;
	BOOL	SkeletonWeightInverted;
	//}}AFX_DATA

	float	StartAnimTime;
	float	EndAnimTime;
	float	AnimationLength;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSlotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool	_Empty;

	// Generated message map functions
	//{{AFX_MSG(CSlotDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDeltaposEndBlendSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposEndTimeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSmoothnessSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpeedFactorSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposStartBlendSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposStartTimeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEndBlend();
	afx_msg void OnChangeEndTime();
	afx_msg void OnChangeSmoothness();
	afx_msg void OnChangeSpeedFactor();
	afx_msg void OnChangeStartBlend();
	afx_msg void OnChangeStartTime();
	afx_msg void OnSetAnimation();
	afx_msg void OnSetSkeleton();
	afx_msg void OnChangeOffset();
	afx_msg void OnDeltaposOffsetSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClamp();
	afx_msg void OnRepeat();
	afx_msg void OnDisable();
	afx_msg void OnAlignBlend();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLOT_DLG_H__9B22CB8B_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
