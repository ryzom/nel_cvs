#if !defined(AFX_EDIT_PS_SOUND_H__E11AAB2A_04BB_453A_B722_AA47DB840D5A__INCLUDED_)
#define AFX_EDIT_PS_SOUND_H__E11AAB2A_04BB_453A_B722_AA47DB840D5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "ps_wrapper.h"
#include "3d/ps_sound.h"




class CAttribDlgFloat;

/////////////////////////////////////////////////////////////////////////////
// CEditPSSound dialog

class CEditPSSound : public CDialog
{
// Construction
public:
	CEditPSSound(NL3D::CPSSound *sound);   // standard constructor

	~CEditPSSound();


	void init(CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CEditPSSound)
	enum { IDD = IDD_SOUND };
	CString	m_SoundName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditPSSound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSSound *		_Sound;				// the sound being edited	
	CAttribDlgFloat *	_VolumeDlg;			// dlg to tune sounds volume
	CAttribDlgFloat *	_FrequencyDlg;	// dlg to tune sounds frequency

	// Generated message map functions
	//{{AFX_MSG(CEditPSSound)
	afx_msg void OnBrowseSound();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSoundName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	/////////////////////////////////////////
	// wrapper to set the volume of sounds //
	/////////////////////////////////////////
	struct CVolumeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const { return S->getVolume(); }
		void set(const float &v) { S->setVolume(v); }
		scheme_type *getScheme(void) const { return S->getVolumeScheme(); }
		void setScheme(scheme_type *s) { S->setVolumeScheme(s); }
	} _VolumeWrapper;
	////////////////////////////////////////////
	// wrapper to set the frequency of sounds //
	////////////////////////////////////////////
	struct CFrequencyWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSSound *S;
		float get(void) const { return S->getFrequency(); }
		void set(const float &v) { S->setFrequency(v); }
		scheme_type *getScheme(void) const { return S->getFrequencyScheme(); }
		void setScheme(scheme_type *s) { S->setFrequencyScheme(s); }
	} _FrequencyWrapper;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_PS_SOUND_H__E11AAB2A_04BB_453A_B722_AA47DB840D5A__INCLUDED_)
