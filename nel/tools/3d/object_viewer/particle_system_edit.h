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


/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit dialog

class CParticleSystemEdit : public CDialog, public CDialogStack
{
// Construction
public:
	CParticleSystemEdit(NL3D::CParticleSystem *ps);   // standard constructor

	~CParticleSystemEdit() ;

	init(CWnd *pParent) ;
// Dialog Data
	//{{AFX_DATA(CParticleSystemEdit)
	enum { IDD = IDD_EDIT_PARTICLE_SYSTEM };
	CButton	m_EnableSlowDownCtrl;
	BOOL	m_AccurateIntegration;
	BOOL	m_EnableSlowDown;
	BOOL	m_PrecomputeBBox;
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	void updatePrecomputedBBoxParams(void) ;
	void updateIntegrationParams(void) ;

    /// wrapper to tune user parameters
	struct CUserParamWrapper : public IPSWrapperFloat
	{
		NL3D::CParticleSystem *PS ;
		uint32 Index ;
		float get(void) const ;
		void set(const float &v) ;
	} _UserParamWrapper[4] ;


	struct CTimeThresholdWrapper : public IPSWrapperFloat
	{
		NL3D::CParticleSystem *PS ;
		float get(void) const ;		
		void set(const float &) ;
	} _TimeThresholdWrapper ;

	struct CMaxNbIntegrationWrapper : public IPSWrapperUInt
	{
		NL3D::CParticleSystem *PS ;
		uint32 get(void) const ;		
		void set(const uint32 &) ;
	} _MaxNbIntegrationWrapper  ;

	struct CMaxViewDistWrapper : public IPSWrapperFloat
	{
		NL3D::CParticleSystem *PS ;
		float get(void) const ;		
		void set(const float &) ;
	} _MaxViewDistWrapper ;

	struct CLODRatioWrapper : public IPSWrapperFloat
	{
		NL3D::CParticleSystem *PS ;
		float get(void) const ;		
		void set(const float &) ;
	} _LODRatioWrapper ;

	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_)
