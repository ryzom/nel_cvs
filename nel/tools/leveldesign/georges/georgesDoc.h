// georgesDoc.h : interface of the CGeorgesDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEORGESDOC_H__15C67AB4_5DBC_4EB2_91D7_716FB8CAE880__INCLUDED_)
#define AFX_GEORGESDOC_H__15C67AB4_5DBC_4EB2_91D7_716FB8CAE880__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CGeorgesDoc : public CDocument
{
protected: // create from serialization only
	CGeorgesDoc();
	DECLARE_DYNCREATE(CGeorgesDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeorgesDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGESDOC_H__15C67AB4_5DBC_4EB2_91D7_716FB8CAE880__INCLUDED_)
