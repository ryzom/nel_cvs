// nel_launcher.h : main header file for the NEL_LAUNCHER application
//

#if !defined(AFX_NEL_LAUNCHER_H__D2D3087E_82E8_4D6D_8C53_A4721FA122AB__INCLUDED_)
#define AFX_NEL_LAUNCHER_H__D2D3087E_82E8_4D6D_8C53_A4721FA122AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'std_afx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNeLLauncherApp:
// See nel_launcher.cpp for the implementation of this class
//

class CNeLLauncherApp : public CWinApp
{
public:
	CNeLLauncherApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeLLauncherApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNeLLauncherApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEL_LAUNCHER_H__D2D3087E_82E8_4D6D_8C53_A4721FA122AB__INCLUDED_)
