// logic_editor.h : main header file for the LOGIC_EDITOR application
//

#if !defined(AFX_LOGIC_EDITOR_H__FC54B10B_B276_4955_AC98_C2935672E146__INCLUDED_)
#define AFX_LOGIC_EDITOR_H__FC54B10B_B276_4955_AC98_C2935672E146__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorApp:
// See logic_editor.cpp for the implementation of this class
//

class CLogic_editorApp : public CWinApp
{
public:
	CLogic_editorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogic_editorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CLogic_editorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGIC_EDITOR_H__FC54B10B_B276_4955_AC98_C2935672E146__INCLUDED_)
