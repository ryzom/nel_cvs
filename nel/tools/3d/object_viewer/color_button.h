#if !defined(AFX_COLOR_BUTTON_H__286CFF58_DD7F_4310_95EB_3477F5A2B923__INCLUDED_)
#define AFX_COLOR_BUTTON_H__286CFF58_DD7F_4310_95EB_3477F5A2B923__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "nel/misc/rgba.h"

using NLMISC::CRGBA ;




/////////////////////////////////////////////////////////////////////////////
// CColorButton window

class CColorButton : public CButton
{

// Construction
public:
	CColorButton();

// Attributes
public:
	// set a nex color for the button
	void setColor(CRGBA col) { _Color = col ; Invalidate() ; }

	// get the color of the button
	CRGBA getColor(void) const { return _Color ; }


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorButton();

	// register this custom control to window
	static void CColorButton::registerClass(void) ;

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()


	// current color of the button
	CRGBA _Color ;


	// the event proc for basic cbutton...
	static WNDPROC _BasicButtonWndProc ;

	// a hook to create the dialog
	static LRESULT CALLBACK EXPORT WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) ;


};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOR_BUTTON_H__286CFF58_DD7F_4310_95EB_3477F5A2B923__INCLUDED_)
