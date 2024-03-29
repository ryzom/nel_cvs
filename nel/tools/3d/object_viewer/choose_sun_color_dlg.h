#if !defined(AFX_CHOOSE_SUN_COLOR_DLG_H__CA0B25DD_6DEE_4EF0_B561_2EFAA445321C__INCLUDED_)
#define AFX_CHOOSE_SUN_COLOR_DLG_H__CA0B25DD_6DEE_4EF0_B561_2EFAA445321C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_sun_color_dlg.h : header file
//

#include "nel/../../src/3d/scene.h"
#include "ps_wrapper.h"

class CColorEdit;

/** Dialog to choose the sun color in a 3D scene
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CChooseSunColorDlg : public CDialog
{
// Construction
public:
	CChooseSunColorDlg(NL3D::CScene *scene, CWnd* pParent = NULL);   // standard constructor
	~CChooseSunColorDlg();

// Dialog Data
	//{{AFX_DATA(CChooseSunColorDlg)
	enum { IDD = IDD_CHOOSE_SUN_COLOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseSunColorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CColorEdit *_DiffuseColorEdit;
	CColorEdit *_AmbientColorEdit;
	CColorEdit *_SpecularColorEdit;

	struct CDiffuseColorWrapper : public IPSWrapperRGBA
	{
		NL3D::CScene *S;
		void set(const NLMISC::CRGBA &col) { S->setSunDiffuse(col); }
		NLMISC::CRGBA get() const  { return S->getSunDiffuse(); }
	} _DiffuseColorWrapper;
	//
	struct CAmbientColorWrapper : public IPSWrapperRGBA
	{
		NL3D::CScene *S;
		void set(const NLMISC::CRGBA &col) { S->setSunAmbient(col); }
		NLMISC::CRGBA get() const  { return S->getSunAmbient(); }
	} _AmbientColorWrapper;
	//
	struct CSpecularColorWrapper : public IPSWrapperRGBA
	{
		NL3D::CScene *S;
		void set(const NLMISC::CRGBA &col) { S->setSunSpecular(col); }
		NLMISC::CRGBA get() const  { return S->getSunSpecular(); }
	} _SpecularColorWrapper;

	// Generated message map functions
	//{{AFX_MSG(CChooseSunColorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_SUN_COLOR_DLG_H__CA0B25DD_6DEE_4EF0_B561_2EFAA445321C__INCLUDED_)
