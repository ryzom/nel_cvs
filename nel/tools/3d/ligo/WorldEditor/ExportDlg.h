#if !defined(AFX_EXPORTDLG_H__F0E3FA13_0D4B_4A26_8CFA_1C315EAA1CBE__INCLUDED_)
#define AFX_EXPORTDLG_H__F0E3FA13_0D4B_4A26_8CFA_1C315EAA1CBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

class CExportDlg : public CDialog
{
// Construction
public:
	CExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
	CString	OutZoneDir;
	CString	RefZoneDir;
	CString	TileBankFile;
	CString	HeightMapFile;
	CString	ZFactor;
	CString	HeightMapFile2;
	CString	ZFactor2;
	uint8	Lighting;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	virtual void OnOK ();
	afx_msg void OnButtonRefzonedir ();
	afx_msg void OnButtonOutzonedir ();
	afx_msg void OnButtonTilebankfile ();
	afx_msg void OnButtonHeightmapfile ();
	afx_msg void OnButtonHeightmapfile2 ();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__F0E3FA13_0D4B_4A26_8CFA_1C315EAA1CBE__INCLUDED_)
