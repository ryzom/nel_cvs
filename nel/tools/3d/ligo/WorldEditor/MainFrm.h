// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__48166BCF_67E9_4421_A2E2_BCA806982F32__INCLUDED_)
#define AFX_MAINFRM_H__48166BCF_67E9_4421_A2E2_BCA806982F32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ---------------------------------------------------------------------------

#include "3d/driver.h"
#include "nel/misc/vector.h"
#include "../lib/ligo_config.h"

#include "BuilderLogic.h"
#include "BuilderZone.h"

// ---------------------------------------------------------------------------

class IMasterCB;

// ---------------------------------------------------------------------------

class CType 
{
public:
	std::string		Name;
	NLMISC::CRGBA	Color;

	void serial (NLMISC::IStream&f);
};

// ---------------------------------------------------------------------------

class CMainFrame : public CFrameWnd
{

public:

	CMainFrame ();

protected: 

	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	
	int					CreateX, CreateY, CreateCX, CreateCY;

	bool				_Exit;
	bool				_SplitterCreated;
	NLLIGO::CLigoConfig _Config;
	CBuilderLogic		_PRegionBuilder;
	CBuilderZone		_ZoneBuilder;
	sint32				_Mode;	// 0-Mode Zone, 1-Mode Logic
	std::string			_RootDir;
	IMasterCB			*_MasterCB;

	std::vector<CType>	_Types;

// Operations
public:

	void setRootDir (const char* str);
	void loadLand (const char* str, const char* path);
	void loadPrim (const char* str, const char* path);
	void saveAll ();
	void initDisplay();
	void uninitTools();
	void initTools();

	void primZoneModified();

	// Initialize the main frame (must be done before init of the tools)
	bool init (bool bMakeAZone = true);
	void uninit ();

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

public:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CSplitterWnd	m_wndSplitter; // GetPane(0,0) == CDisplay, GetPane(0,1) == CTools


// Generated message map functions
protected:

	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMenuFileExit ();
	afx_msg void OnMenuFileNewLogic ();
	afx_msg void OnMenuFileUnloadLogic ();
	afx_msg void OnMenuFileOpenLogic ();
	afx_msg void OnMenuFileSaveLogic ();
	afx_msg void OnMenuFileNewLandscape ();
	afx_msg void OnMenuFileUnloadLandscape ();
	afx_msg void OnMenuFileOpenLandscape ();
	afx_msg void OnMenuFileSaveLandscape ();
	afx_msg void OnMenuFileGenerate ();
	afx_msg void OnMenuFileExportToClient ();
	afx_msg void OnMenuModeZone ();
	afx_msg void OnMenuModeLogic ();
	afx_msg void OnMenuModeType ();
	afx_msg void onMenuModeSelectZone ();
	afx_msg void onMenuModeMove ();
	afx_msg void OnMenuViewGrid ();
	afx_msg void OnMenuViewBackground ();
	afx_msg void OnClose ();
	afx_msg void OnSize (UINT nType, int cx, int cy);

public:

	afx_msg void onMenuModeUndo ();
	afx_msg void onMenuModeRedo ();
	//afx_msg void OnKeyDown (UINT nChar, UINT nRepCnt, UINT nFlags);

		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

class CSelectDialog : public CDialog
{
	int				_Sel;

	CBuilderZone	*_ToInit;
	CBuilderLogic	*_ToInitLogic;


public:

	CSelectDialog (CWnd*pParent);

	void set (CBuilderZone *bzForInit);
	void setLogic (CBuilderLogic *blForInit);

	BOOL OnInitDialog ();
	void OnOK ();
	int getSel ();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__48166BCF_67E9_4421_A2E2_BCA806982F32__INCLUDED_)
