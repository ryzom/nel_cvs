#if !defined(AFX_SCENE_DLG_H__A9ECE124_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SCENE_DLG_H__A9ECE124_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// scene_dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSceneDlg dialog

class CSceneDlg : public CDialog
{
// Construction
	friend class CObjectViewer;
public:
	CSceneDlg(class CObjectViewer *objView, CWnd* pParent = NULL);   // standard constructor
	~CSceneDlg();

// Dialog Data
	//{{AFX_DATA(CSceneDlg)
	enum { IDD = IDD_SCENE };
	CEdit	MoveSpeedCtrl;
	BOOL	ViewAnimation;
	BOOL	ViewAnimationSet;
	BOOL	ViewSlots;
	BOOL	Euler;
	BOOL	ObjectMode;
	float	MoveSpeed;
	//}}AFX_DATA

	class CObjectViewer	*ObjView;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSceneDlg)
	afx_msg void OnClearScene();
	afx_msg void OnLoadMesh();
	afx_msg void OnLoadPlaylist();
	afx_msg void OnLoadScene();
	afx_msg void OnSavePlaylist();
	afx_msg void OnViewAnimation();
	afx_msg void OnViewAnimationset();
	afx_msg void OnViewSlots();
	virtual BOOL OnInitDialog();
	afx_msg void OnResetCamera();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENE_DLG_H__A9ECE124_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
