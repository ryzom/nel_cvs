// bug_reportDlg.h : header file
//

#if !defined(AFX_BUG_REPORTDLG_H__B6E74FF5_5259_4636_AE56_0DC247D4BDAF__INCLUDED_)
#define AFX_BUG_REPORTDLG_H__B6E74FF5_5259_4636_AE56_0DC247D4BDAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBug_reportDlg dialog

class CBug_reportDlg : public CDialog
{
// Construction
public:
	CBug_reportDlg(CWnd* pParent = NULL);	// standard constructor

	std::string get (int nID);
	void		set (int nID, const std::string &val, bool enable = true);
	void		setFocus (int nID);
	
	std::string	DumpFilename;
	std::string	SMTPServer, ToEmail;

// Dialog Data
	//{{AFX_DATA(CBug_reportDlg)
	enum { IDD = IDD_BUG_REPORT_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBug_reportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBug_reportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSendreport();
	afx_msg void OnClear();
	afx_msg void OnAttachfile();
	afx_msg void OnNoattach();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUG_REPORTDLG_H__B6E74FF5_5259_4636_AE56_0DC247D4BDAF__INCLUDED_)
