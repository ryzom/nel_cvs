// WorldEditor_Exe.h : main header file for the WORLDEDITOR_EXE application
//

#if !defined(AFX_WORLDEDITOR_EXE_H__E6A00B38_184F_4D1E_9D2D_686959EC879B__INCLUDED_)
#define AFX_WORLDEDITOR_EXE_H__E6A00B38_184F_4D1E_9D2D_686959EC879B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp:
// See WorldEditor_Exe.cpp for the implementation of this class
//

class CWorldEditorApp : public CWinApp
{
public:
	CWorldEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CWorldEditorApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDITOR_EXE_H__E6A00B38_184F_4D1E_9D2D_686959EC879B__INCLUDED_)
