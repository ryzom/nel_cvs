#if !defined(AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_)
#define AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "ps_wrapper.h"
#include "dialog_stack.h"

namespace NL3D
{
	class CParticleSystem ;
}



template <class T> class CEditableRangeT ;
typedef CEditableRangeT<float> CEditableRangeFloat ;
typedef CEditableRangeT<uint32> CEditableRangeUInt ;


class CUserParamWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS ;
	uint32 Index ;
	float get(void) const ;
	void set(const float &v) ;
};


class CTimeThresholdWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS ;
	float get(void) const ;		
	void set(const float &) ;
};

class CMaxNbIntegrationWrapper : public IPSWrapperUInt
{
public:
	NL3D::CParticleSystem *PS ;
	uint32 get(void) const ;		
	void set(const uint32 &) ;
};

class CMaxViewDistWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS ;
	float get(void) const ;		
	void set(const float &) ;
};

class CLODRatioWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS ;
	float get(void) const ;		
	void set(const float &) ;
};

/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit dialog

class CParticleSystemEdit : public CDialog, public CDialogStack
{
// Construction
public:
	CParticleSystemEdit(NL3D::CParticleSystem *ps);   // standard constructor

	~CParticleSystemEdit() ;

	void init(CWnd *pParent) ;
// Dialog Data
	//{{AFX_DATA(CParticleSystemEdit)
	enum { IDD = IDD_EDIT_PARTICLE_SYSTEM };
	CButton	m_PerformMotionWhenOutOfFrustumDlg;
	CComboBox	m_DieOnEvent;
	CButton	m_PrecomputeBBoxCtrl;
	CEdit	m_BBoxZCtrl;
	CEdit	m_BBoxYCtrl;
	CEdit	m_BBoxXCtrl;
	CButton	m_EnableSlowDownCtrl;
	BOOL	m_AccurateIntegration;
	BOOL	m_EnableSlowDown;
	BOOL	m_DieWhenOutOfRange;
	BOOL	m_DieWhenOutOfFrustum;
	BOOL	m_PerformMotionWhenOutOfFrustum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleSystemEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NL3D::CParticleSystem *_PS ;


	CEditableRangeFloat *_TimeThresholdDlg, *_MaxViewDistDlg, *_LODRatioDlg ;
	CEditableRangeUInt  *_MaxIntegrationStepDlg ; 

	// Generated message map functions
	//{{AFX_MSG(CParticleSystemEdit)
	afx_msg void OnAccurateIntegration();
	afx_msg void OnEnableSlowDown();
	afx_msg void OnPrecomputeBbox();
	afx_msg void OnUpdateBbox();
	afx_msg void OnIncBbox();
	afx_msg void OnDecBbox();
	afx_msg void OnDieWhenOutOfRange();
	afx_msg void OnSelchangePsDieOnEvent();
	afx_msg void OnChangeApplyAfterDelay();
	afx_msg void OnDieWhenOutOfFrustum();
	afx_msg void OnPerformMotionWhenOutOfFrustum();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	void updatePrecomputedBBoxParams(void) ;
	void updateIntegrationParams(void) ;
	void updateDieOnEventParams(void) ;

	static CTimeThresholdWrapper _TimeThresholdWrapper ;

	static CMaxViewDistWrapper _MaxViewDistWrapper ;

	static CMaxNbIntegrationWrapper _MaxNbIntegrationWrapper  ;

	static CLODRatioWrapper _LODRatioWrapper ;

    /// wrapper to tune user parameters
	static CUserParamWrapper _UserParamWrapper[4] ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_)
