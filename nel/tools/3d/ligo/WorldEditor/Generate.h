#if !defined(AFX_GENERATE_H__40A5DE91_9FB6_44AC_BE31_839844ED999A__INCLUDED_)
#define AFX_GENERATE_H__40A5DE91_9FB6_44AC_BE31_839844ED999A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Generate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGenerate dialog

class CGenerate : public CDialog
{
// Construction
public:
	CGenerate(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGenerate)
	enum { IDD = IDD_GENERATE };
	CComboBox	ComboMaterial;
	int		MinX;
	int		MinY;
	int		MaxY;
	int		MaxX;
	int		ZoneBaseX;
	int		ZoneBaseY;
	CString	ComboMaterialString;
	//}}AFX_DATA


	std::vector<std::string> AllMaterials;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenerate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGenerate)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATE_H__40A5DE91_9FB6_44AC_BE31_839844ED999A__INCLUDED_)
