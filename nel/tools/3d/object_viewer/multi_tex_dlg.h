#if !defined(AFX_MULTI_TEX_DLG_H__5D160C02_0FF4_4DF3_A804_90CE96C4F448__INCLUDED_)
#define AFX_MULTI_TEX_DLG_H__5D160C02_0FF4_4DF3_A804_90CE96C4F448__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "ps_wrapper.h"

namespace NL3D
{
	class CPSMultiTexturedParticle;
}

class CTextureChooser;
struct IPopupNotify;


/////////////////////////////////////////////////////////////////////////////
// CMultiTexDlg dialog

class CMultiTexDlg : public CDialog
{
// Construction
public:
	CMultiTexDlg(NL3D::CPSMultiTexturedParticle *mtp, IPopupNotify* pn, CWnd *pParent);   // standard constructor

	~CMultiTexDlg();

	/// create and show this dialog
	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CMultiTexDlg)
	enum { IDD = IDD_MULTITEX };
	CButton	m_UseParticleDateAltCtrl;
	CButton	m_UseParticleDateCtrl;
	CButton	m_ForceBasicCapsCtrl;
	CButton	m_AltTexCtrl;
	CComboBox	m_AlternateOpCtrl;
	CComboBox	m_MainOpCtrl;
	BOOL	m_ForceBasicCaps;
	BOOL	m_UseParticleDateAlt;
	BOOL	m_UseParticleDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiTexDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IPopupNotify	*_PN;
	// the multitextured particle being edited
	NL3D::CPSMultiTexturedParticle *_MTP;

	// Generated message map functions
	//{{AFX_MSG(CMultiTexDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnEnableAlternate();
	afx_msg void OnUpdateSpeed();
	afx_msg void OnUpdateSpeedAlternate();
	afx_msg void OnSelchangeAlternateOp();
	afx_msg void OnSelchangeMainOp();
	afx_msg void OnForceBasicCaps();
	afx_msg void OnUseParticleDate();
	afx_msg void OnUseParticleDateAlt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	

	// texture wrappers
	struct CMainTexWrapper : IPSWrapperTexture
	{
		NL3D::CPSMultiTexturedParticle *MTP;
		virtual NL3D::ITexture *get(void);
		virtual void set(NL3D::ITexture *);
	} _TexWrapper;


	struct CAlternateTexWrapper : IPSWrapperTexture
	{
		NL3D::CPSMultiTexturedParticle *MTP;
		virtual NL3D::ITexture *get(void);
		virtual void set(NL3D::ITexture *);
	} _AlternateTexWrapper;


	CTextureChooser *_MainTexDlg, *_AltTexDlg;

	void readValues(bool alternate); // get the values from the particle
	void writeValues(bool alternate);
	
	
	void updateAlternate();
	void updateTexOp();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTI_TEX_DLG_H__5D160C02_0FF4_4DF3_A804_90CE96C4F448__INCLUDED_)
