// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__48166BCF_67E9_4421_A2E2_BCA806982F32__INCLUDED_)
#define AFX_MAINFRM_H__48166BCF_67E9_4421_A2E2_BCA806982F32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "3d/driver.h"
#include "nel/misc/vector.h"
#include "../lib/ligo_config.h"

#include "BuilderLogic.h"
#include "BuilderZone.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame ();

protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	bool				_Exit;
	bool				_SplitterCreated;
	NLLIGO::CLigoConfig _Config;
	CBuilderLogic		_PRegionBuilder;
	CBuilderZone		_ZoneBuilder;
	sint32				_Mode;	// 0-Mode Zone, 1-Mode Logic
// Operations
public:
	void initDisplay();
	void uninitTools();
	void initTools();
	bool loadConfig();
	void displayCoordinates(NLMISC::CVector &v);
	void adjustSplitter();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CSplitterWnd	m_wndSplitter; // GetPane(0,0) == CDisplay, GetPane(0,1) == CTools


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMenuFileExit ();
	afx_msg void OnMenuFileOpen ();
	afx_msg void OnMenuFileSave ();
	afx_msg void OnMenuModeZone ();
	afx_msg void OnMenuModeLogic ();
	afx_msg void OnMenuViewGrid ();
	afx_msg void OnClose ();
	afx_msg void OnSize (UINT nType, int cx, int cy);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__48166BCF_67E9_4421_A2E2_BCA806982F32__INCLUDED_)
