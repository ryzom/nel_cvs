#if !defined(AFX_COLOR_EDIT_H__50C45CFE_2188_4161_B565_C773FE029BF3__INCLUDED_)
#define AFX_COLOR_EDIT_H__50C45CFE_2188_4161_B565_C773FE029BF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// color_edit.h : header file
//

#include "nel/misc/rgba.h"

using NLMISC::CRGBA ;

#include "edit_attrib_dlg.h"

#include "color_button.h"

/////////////////////////////////////////////////////////////////////////////
// CColorEdit dialog


class CColorButton ;

class CColorEdit : public CEditAttribDlg
{
// Construction
public:
	CColorEdit(CWnd* pParent = NULL);   // standard constructor


	// inherited from CEditAttribDlg
	virtual void init(uint32 x, uint32 y, CWnd *pParent) ;

// Dialog Data
	//{{AFX_DATA(CColorEdit)
	enum { IDD = IDD_COLOR_EDIT };
	CScrollBar	m_GreenCtrl;
	CScrollBar	m_BlueCtrl;
	CScrollBar	m_RedCtrl;
	CColorButton	m_Color;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation


public:
// set a function to get back the datas (this may be a wrapper to an existing class)
	void setReader(CRGBA (* reader) (void *lParam), void *lParam) { _Reader = reader ; _ReaderParam =  lParam ; }
// set a function to write new datas
	void setWriter(void (* writer) (CRGBA value, void *lParam), void *lParam) { _Writer = writer ; _WriterParam = lParam ; }

protected:

	CColorButton &getColorCtrl(void) { return  * (CColorButton *) GetDlgItem(IDC_PARTICLE_COLOR) ; }
	CRGBA(* _Reader)(void *lParam)  ;
	void(* _Writer) (CRGBA value, void *lParam)  ;
	void *_ReaderParam, *_WriterParam ;


	void CColorEdit::updateColorFromReader(void) ;

	// Generated message map functions
	//{{AFX_MSG(CColorEdit)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBrowseColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOR_EDIT_H__50C45CFE_2188_4161_B565_C773FE029BF3__INCLUDED_)
