// bug_report.h : main header file for the BUG_REPORT application
//

#if !defined(AFX_BUG_REPORT_H__CBA9B944_1B26_43E9_86F3_A3188053B180__INCLUDED_)
#define AFX_BUG_REPORT_H__CBA9B944_1B26_43E9_86F3_A3188053B180__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBug_reportApp:
// See bug_report.cpp for the implementation of this class
//

class CBug_reportApp : public CWinApp
{
public:
	CBug_reportApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBug_reportApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBug_reportApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUG_REPORT_H__CBA9B944_1B26_43E9_86F3_A3188053B180__INCLUDED_)
