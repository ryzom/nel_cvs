#if !defined(AFX_MAIN_FRAME_H__90D61263_7782_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_MAIN_FRAME_H__90D61263_7782_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// main_frame.h : header file
//

class CSceneDlgMouseListener : public NLMISC::IEventListener
{
public:
	class CObjectViewer	*ObjViewerDlg ;	
	class CMainFrame *SceneDlg ;
	/** 
	  * Register the listener to the server.
	  */
	void addToServer (NLMISC::CEventServer& server);
	void releaseFromServer (NLMISC::CEventServer& server);

protected:
	virtual void operator ()(const class NLMISC::CEvent& event) ;

} ;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame frame

typedef void (*winProc)(NL3D::IDriver *drv, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class CMainFrame : public CFrameWnd
{
	//DECLARE_DYNCREATE(CMainFrame)
public:
	CMainFrame( CObjectViewer *objView, winProc );
	virtual ~CMainFrame();

// Attributes
public:

	CStatusBar		StatusBar;
	CToolBar		ToolBar;

	winProc			DriverWindowProc;
	CObjectViewer	*ObjView;

	bool			ShowInfo;
	bool			AnimationWindow;
	bool			AnimationSetWindow;
	bool			MixerSlotsWindow;
	bool			ParticlesWindow;
	bool		    DayNightWindow;
	bool		    WaterPoolWindow;
	bool		    VegetableWindow;
	bool		    GlobalWindWindow;
	bool			MoveElement;
	bool			MoveObjectLightTest;
	bool			X;
	bool			Y;
	bool			Z;
	uint			MoveMode;
	float			MoveSpeed;
	NLMISC::CRGBA	BgColor;
	bool			Euler;
	float			GlobalWindPower;
	
	void update ();
	void registerValue (bool update=true);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
public:
	// Generated message map functions
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnClear();
	afx_msg void OnEditMoveelement();
	afx_msg void OnEditX();
	afx_msg void OnEditY();
	afx_msg void OnEditZ();
	afx_msg void OnEnableElementXrotate();
	afx_msg void OnEnableElementYrotate();
	afx_msg void OnEnableElementZrotate();
	afx_msg void OnFileExit();
	afx_msg void OnFileLoadconfig();
	afx_msg void OnFileOpen();	
	afx_msg void OnFileSaveconfig();
	afx_msg void OnViewFirstpersonmode();
	afx_msg void OnViewObjectmode();
	afx_msg void OnResetCamera();
	afx_msg void OnViewSetbackground();
	afx_msg void OnViewSetmovespeed();
	afx_msg void OnWindowAnimation();
	afx_msg void OnWindowAnimationset();
	afx_msg void OnWindowMixersslots();
	afx_msg void OnWindowParticles();
	afx_msg void OnWindowDayNight();
	afx_msg void OnWindowWaterPool();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnUpdateWindowAnimation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowAnimationset(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowMixersslots(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowParticles(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowDayNight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWindowWaterPool(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectmode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFirstpersonmode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditY(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditZ(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMoveelement(CCmdUI* pCmdUI);
	afx_msg void OnHelpAboutobjectviewer();
	afx_msg void OnSetLag();
	afx_msg void OnRemoveAllInstancesFromScene();
	afx_msg void OnActivateTextureSet(UINT nID);
	afx_msg void OnWindowVegetable();
	afx_msg void OnUpdateWindowVegetable(CCmdUI* pCmdUI);
	afx_msg void OnWindowGlobalwind();
	afx_msg void OnUpdateWindowGlobalwind(CCmdUI* pCmdUI);
	afx_msg void OnEditMoveObjectLightTest();
	afx_msg void OnUpdateEditMoveObjectLightTest(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CSceneDlgMouseListener _RightButtonMouseListener ;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAIN_FRAME_H__90D61263_7782_11D5_9CD4_0050DAC3A412__INCLUDED_)
