// ---------------------------------------------------------------------------
// MainFrm.h : interface of the CMainFrame class
// ---------------------------------------------------------------------------

#if !defined(AFX_MAINFRM_H__D9ABC57D_9514_49B1_A65F_1CC64C6D6BB6__INCLUDED_)
#define AFX_MAINFRM_H__D9ABC57D_9514_49B1_A65F_1CC64C6D6BB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ---------------------------------------------------------------------------

#include "nel/misc/stream.h"
#include "nel/misc/file.h"
#include "MasterTree.h"
#include <string>

// ---------------------------------------------------------------------------
// Interface to the tools
// ---------------------------------------------------------------------------

class IWorldEditor;
class IGeorges;
class ILogicEditor;

// ---------------------------------------------------------------------------

struct CEnvironnement
{
	// Master params
	sint32 MasterX, MasterY;
	sint32 MasterTreeX, MasterTreeY;
	sint32 MasterTreeCX, MasterTreeCY;
	bool MasterTreeLocked;
	std::string RootDir;

	// WorldEditor params
	bool WorldEdOpened;
	sint32 WorldEdX, WorldEdY, WorldEdCX, WorldEdCY;

	// Georges params
	bool GeorgesOpened;
	sint32 GeorgesX, GeorgesY, GeorgesCX, GeorgesCY;

	// LogicEditor params
	bool LogicEditorOpened;
	sint32 LogicEditorX, LogicEditorY, LogicEditorCX, LogicEditorCY;

	CEnvironnement();
	void serial (NLMISC::IStream& s);
};


// ---------------------------------------------------------------------------

class CMainFrame : public CFrameWnd
{
	CEnvironnement	_Environnement;

	IWorldEditor	*_WorldEditor;
	HMODULE			_WorldEditorModule;

	IGeorges		*_Georges;
	HMODULE			_GeorgesModule;

	ILogicEditor	*_LogicEditor;
	HMODULE			_LogicEditorModule;

	CMasterTreeDlg	*_Tree;

//protected: // create from serialization only
//	DECLARE_DYNCREATE(CMainFrame)

public:

	CMainFrame ();
	virtual ~CMainFrame ();

	void getAllInterfaces (); // Load all dlls and get tools interfaces
	void releaseAllInterfaces ();

	void openWorldEditor ();
	void openWorldEditorFile (const char *fileName);
	void closeWorldEditor ();

	void openGeorges ();
	void openGeorgesFile (const char *fileName);
	void closeGeorges ();

	void openLogicEditor ();
	void openLogicEditorFile (const char *fileName);
	void closeLogicEditor ();

	// Tools
	
	void updateTree ();
	void emptyTrash ();
	void emptyBackup ();
	void cleanBackup ();
	void regionBackupAll ();
	void regionTrashAll ();
	void backupRestoreAll ();

	void trashDelete		(const char *str);
	void trashRestoreOne	(const char *str);
	void backupDelete		(const char *str);
	void backupRestoreOne	(const char *str);
	void regionDelete		(const char *str);
	void regionBackupOne	(const char *str);


	void deltree (const std::string &dirName);


#ifdef _DEBUG
	virtual void AssertValid () const;
	virtual void Dump (CDumpContext& dc) const;
#endif

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnMove (int x, int y);
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd (CDC* pDC);

public:
	afx_msg void onRegionNew (); // Called from MasterTree

protected:
	afx_msg void onRegionSave ();
	afx_msg void onRegionEmptyTrash ();
	afx_msg void onRegionEmptyBackup ();
	afx_msg void onRegionBackupAll ();
	afx_msg void onRegionTrashRegions ();
	afx_msg void onRegionRestoreTag ();

	afx_msg void OnRegionDeleteInTrash ();
	afx_msg void OnRegionDeleteInBackup ();
	afx_msg void OnRegionDelete ();
	afx_msg void OnRegionRestoreFromTrash ();
	afx_msg void OnRegionRestoreFromBackup ();
	afx_msg void OnRegionBackup ();

	afx_msg void onOptionsTreeLock ();
	afx_msg void onOptionsSetRoot ();

	afx_msg void onWindowsWorldEditor ();
	afx_msg void onWindowsGeorges ();
	afx_msg void onWindowsLogicEditor ();

	afx_msg void OnClose ();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void createDirIfNotExist (const std::string& dirName, const std::string& errorMsg);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__D9ABC57D_9514_49B1_A65F_1CC64C6D6BB6__INCLUDED_)
