// GeorgesDoc.h : interface of the CGeorgesDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEORGESDOC_H__16A90258_8DA9_4F0C_873E_E0B4C98228A1__INCLUDED_)
#define AFX_GEORGESDOC_H__16A90258_8DA9_4F0C_873E_E0B4C98228A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../georges_lib/Item.h"

class CFormFile;
class CStringList;

class CGeorgesDoc : public CDocument
{
protected: // create from serialization only
	CGeorgesDoc();
	DECLARE_DYNCREATE(CGeorgesDoc)

	CItem		item;
	CStringEx	sxrootdirectory;
	CStringEx	sxworkdirectory;

// Attributes
public:

// Operations 
public:
	void SetItemValue( const unsigned int _index, const CString s );
	unsigned int GetItemNbElt() const;
	unsigned int GetItemNbElt( const unsigned int _index ) const;
	unsigned int GetItemInfos( const unsigned int _index ) const;
	CString GetItemName( const unsigned int _index ) const;
	CString GetItemCurrentResult( const unsigned int _index ) const;
	CString GetItemCurrentValue( const unsigned int _index ) const;
	CString GetItemFormula( const unsigned int _index ) const;
	bool IsItemEnum( const unsigned int _index ) const;
	void GetItemListPredef( const unsigned int _index, CStringList* _slist ) const;
	void NewDocument( const CStringEx& _sxdfnname ) const;
	void NewDocument( const CStringEx _sxfilename );
	void UpdateDocument();

	void AddListParent( const unsigned int _index ) const;
	void AddListChild( const unsigned int _index ) const;
	void DelListChild( const unsigned int _index ) const;

	CStringEx GetWorkDirectory() const;
	CStringEx GetRootDirectory() const;
	void SetWorkDirectory( const CStringEx _sxworkdirectory );
	void SetRootDirectory( const CStringEx _sxrootdirectory );
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();

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

#endif // !defined(AFX_GEORGESDOC_H__16A90258_8DA9_4F0C_873E_E0B4C98228A1__INCLUDED_)
