// Georges.h : main header file for the GEORGES application
//

#if !defined(AFX_GEORGES_H__FCC07321_0C96_48BE_AE90_C3E1F0E61134__INCLUDED_)
#define AFX_GEORGES_H__FCC07321_0C96_48BE_AE90_C3E1F0E61134__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

//////////////////////////²///////////////////////////////////////////////////
// CGeorgesApp:
// See Georges.cpp for the implementation of this class
//

class CGeorgesApp : public CWinApp
{
	CMultiDocTemplate *_MultiDocTemplate;
public:
	CGeorgesApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesApp)
	public:
	BOOL initInstance (int x = 0, int y = 0, int cx = 0, int cy = 0);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CGeorgesApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGES_H__FCC07321_0C96_48BE_AE90_C3E1F0E61134__INCLUDED_)
