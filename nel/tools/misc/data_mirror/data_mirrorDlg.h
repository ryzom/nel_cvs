// data_mirrorDlg.h : header file
//

#if !defined(AFX_DATA_MIRRORDLG_H__D812F8AA_58F2_4ED2_B0F3_0CAB1B0DCB77__INCLUDED_)
#define AFX_DATA_MIRRORDLG_H__D812F8AA_58F2_4ED2_B0F3_0CAB1B0DCB77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorDlg dialog

class CData_mirrorDlg : public CDialog
{
// Construction
public:
	CData_mirrorDlg(CWnd* pParent = NULL);	// standard constructor

	uint	ButtonYFromBottom;
	uint	ListBottomFromBottom;
	uint	OkButtonXFromRight;
	uint	IgnorButtonXFromRight;
	uint	CancelButtonXFromRight;
	uint	SpaceBetweenList;

	void	resize ();
	void	refresh ();
	void	onClickList (LPNMLISTVIEW lpnmlv, CListCtrl &list);
	void	ignore (CListCtrl &_list);

// Dialog Data
	//{{AFX_DATA(CData_mirrorDlg)
	enum { IDD = IDD_DATA_MIRROR_DIALOG };
	CStatic	RemovedLabelCtrl;
	CStatic	ModifiedLabelCtrl;
	CStatic	AddedLabelCtrl;
	CButton	CancelCtrl;
	CButton	IgnoreCtrl;
	CButton	OkCtrl;
	CListCtrl	RemovedList;
	CListCtrl	AddedList;
	CListCtrl	ModifiedList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CData_mirrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CData_mirrorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnIgnore();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickModified(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickRemoved(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClickAdded(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATA_MIRRORDLG_H__D812F8AA_58F2_4ED2_B0F3_0CAB1B0DCB77__INCLUDED_)
