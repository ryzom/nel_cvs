#if !defined(AFX_MASTERTREE_H__62FA2687_A7AC_4FA3_837A_12DCA1C2D080__INCLUDED_)
#define AFX_MASTERTREE_H__62FA2687_A7AC_4FA3_837A_12DCA1C2D080__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MasterTree.h : header file
//

#include <string>
#include <map>

/////////////////////////////////////////////////////////////////////////////

#define MT_SORT_BY_NAME_INC	0
#define MT_SORT_BY_NAME_DEC 1
#define MT_SORT_BY_DATE_INC 2
#define MT_SORT_BY_DATE_DEC 3

/////////////////////////////////////////////////////////////////////////////
// CMasterTree Control
/////////////////////////////////////////////////////////////////////////////

class CMasterTree : public CTreeCtrl
{

	bool _LDrag;
	CImageList *_DragImg;
	HTREEITEM _DragItem;
	HTREEITEM _LastItemSelected;
	HTREEITEM _LastActiveRegion;

	UINT    m_nTimerID;
	UINT    m_timerticks;
	
public:

	CMasterTree ();

	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLBeginDrag (NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnMouseMove (UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk (UINT nFlags, CPoint point);

	afx_msg void OnRButtonDown (UINT nFlags, CPoint point);

	// Top level (just under the root) handlers
	afx_msg void OnMenuTrashEmpty ();
	afx_msg void OnMenuBackupEmpty ();
	afx_msg void OnMenuBackupClean ();
	afx_msg void OnMenuBackupRestore ();
	afx_msg void OnMenuRegionNew ();
	afx_msg void OnMenuRegionTrash ();
	afx_msg void OnMenuRegionBackup ();

	afx_msg void OnMenuSortNameInc ();
	afx_msg void OnMenuSortNameDec ();
	afx_msg void OnMenuSortDateInc ();
	afx_msg void OnMenuSortDateDec ();

	// Region menus (a level under the top)

	afx_msg void OnMenuTrashDelete ();
	afx_msg void OnMenuTrashRestoreOne ();
	afx_msg void OnMenuBackupDelete ();
	afx_msg void OnMenuBackupRestoreOne ();
	afx_msg void OnMenuRegionDelete ();
	afx_msg void OnMenuRegionBackupOne ();
	afx_msg void OnMenuRegionSetActive ();
	afx_msg void OnMenuRegionNewPrim ();
	afx_msg void OnMenuRegionNewGeorges ();

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMasterTreeDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CMasterTreeDlg : public CDialog
{
	CMasterTree						*_Tree;
	std::map<HTREEITEM,std::string>	_Files;

	char RegionSortBy;	// 0/1 - Sort by name increasing(z-a)/decreasing(a-z)
	char TrashSortBy;	// 2/3 - Sort by date increasing(new-old)/decreasing(old-new)
	char BackupSortBy;

private:

	void parseAdd (HTREEITEM itRoot, const std::string &path, char nSortType);

public:
	
	CMasterTreeDlg();   // standard constructor
	
	void update (const std::string& path);
	
	void openAnyFile (const char *fname);
	void openAnyFileFromItem (HTREEITEM hItem);
	void emptyTrash ();
	void emptyBackup ();
	void cleanBackup ();
	void regionNew ();
	void regionBackupAll ();
	void regionTrashAll ();
	void backupRestoreAll ();
	void sortRegionBy (int mt_sort_type);
	void sortTrashBy (int mt_sort_type);
	void sortBackupBy (int mt_sort_type);

	void trashDelete (const char *str);
	void trashRestoreOne (const char *str);
	void backupDelete (const char *str);
	void backupRestoreOne (const char *str);
	void regionDelete (const char *str);
	void regionBackupOne (const char *str);
	void regionNewPrim (const char *str);
	void regionNewGeorges (const char *str);

	void selectRegion (const std::string &Region, const std::string &Directory);

// Dialog Data
	//{{AFX_DATA(CMasterTree)
	enum { IDD = IDD_MASTERTREE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMasterTree)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	BOOL OnInitDialog ();
	void OnCancel ();
	void OnOK ();
	
	// Generated message map functions
	//{{AFX_MSG(CMasterTree)
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnExpanding (LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnLDblClkTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Friends
	
	friend class CMasterTree;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MASTERTREE_H__62FA2687_A7AC_4FA3_837A_12DCA1C2D080__INCLUDED_)
