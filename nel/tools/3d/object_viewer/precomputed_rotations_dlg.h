#if !defined(AFX_PRECOMPUTED_ROTATIONS_DLG_H__D7F383B7_AE48_4BFF_9E3E_AA41A8BE76A5__INCLUDED_)
#define AFX_PRECOMPUTED_ROTATIONS_DLG_H__D7F383B7_AE48_4BFF_9E3E_AA41A8BE76A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

namespace NL3D
{
	struct CPSHintParticleRotateTheSame ;
} ;



/////////////////////////////////////////////////////////////////////////////
// CPrecomputedRotationsDlg dialog

class CPrecomputedRotationsDlg : public CDialog
{

public:
	/** ctor
	 *  \param prts the particle being edited
	 *  \param toDisable a window that is disabled when precomputed rotation are activated (null = none)
	 */
	CPrecomputedRotationsDlg(NL3D::CPSHintParticleRotateTheSame *prts, class CAttribDlg *toDisable = NULL);   // standard constructor
	void init(CWnd *pParent, sint x, sint y) ;

// Dialog Data
	//{{AFX_DATA(CPrecomputedRotationsDlg)
	enum { IDD = IDD_HINT_ROTATE_THE_SAME };
	CEdit	m_RotSpeedMinCtrl;
	CEdit	m_NbModelsCtrl;
	CEdit	m_RotSpeedMaxCtrl;
	CString	m_RotSpeedMax;
	BOOL	m_PrecomputedRotations;
	CString	m_RotSpeedMin;
	CString	m_NbModels;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrecomputedRotationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// anable / disable the controls for the tuning of the number of prerotated models
	void enablePrecompRotationControl(void) ;
	// update the control value from the particle being edited
	void updateFromReader(void)	;


	// the particle being edited
	NL3D::CPSHintParticleRotateTheSame *_RotatedParticle ;

	CAttribDlg *_WndToDisable ;

	// Generated message map functions
	//{{AFX_MSG(CPrecomputedRotationsDlg)
	afx_msg void OnUpdateMinRotSpeed();
	afx_msg void OnUpdateMaxRotSpeed();
	afx_msg void OnUpdateNbModels();
	afx_msg void OnHintPrecomputedRotations();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRECOMPUTED_ROTATIONS_DLG_H__D7F383B7_AE48_4BFF_9E3E_AA41A8BE76A5__INCLUDED_)
