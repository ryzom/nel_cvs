#if !defined(AFX_SOUNDPAGE_H__930378AF_E242_40AD_BF1F_882AAED43945__INCLUDED_)
#define AFX_SOUNDPAGE_H__930378AF_E242_40AD_BF1F_882AAED43945__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../src/sound/sound.h"
#include "nel/sound/u_audio_mixer.h"
using namespace NLSOUND;


// SoundPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSoundPage dialog

class CSoundPage : public CDialog
{
// Construction
public:
	CSoundPage(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSoundPage)
	enum { IDD = IDD_SoundPage };
	CString	m_Filename;
	float	m_Gain;
	BOOL	m_Pos3D;
	float	m_MinDist;
	float	m_MaxDist;
	UINT	m_InnerAngleDeg;
	UINT	m_OuterAngleDeg;
	float	m_OuterGain;
	BOOL	m_Looped;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	void		setTree( CTreeCtrl *tree )						  { _Tree = tree; }
	void		setCurrentSound( CSound *sound, HTREEITEM hitem );
	void		getPropertiesFromSound();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSoundPage)
	afx_msg void OnPos3D();
	afx_msg void OnApply();
	afx_msg void OnChooseFile();
	afx_msg void OnCancel();
	afx_msg void OnRemove();
	afx_msg void OnPlaySound();
	afx_msg void OnClose();
	afx_msg void OnLooped();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CSound						*_CurrentSound;
	CTreeCtrl					*_Tree;
	HTREEITEM					_HItem;
	UAudioMixer					*_AudioMixer;
	USource						*_Source;

	void						UpdateCurrentSound();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDPAGE_H__930378AF_E242_40AD_BF1F_882AAED43945__INCLUDED_)
