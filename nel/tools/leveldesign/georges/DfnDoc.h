// ---------------------------------------------------------------------------
//
// DfnDoc.h : interface of the CDfnDoc class
//
// ---------------------------------------------------------------------------
// The TDfn defined in this document is just a vector<pair<string,string>>
// The first element of the pair is the name of the field and the second one
// is the type of the field which can be "list< string.typ >" of "uint16.typ"
// for instance.

#ifndef __DFNDOC_H__
#define __DFNDOC_H__

// ---------------------------------------------------------------------------

#include "../georges_lib/stringex.h"
#include <vector>

// ---------------------------------------------------------------------------

class CFormFile;
class CStringList;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
class CDfnDoc : public CDocument
{
public:

	typedef std::pair< CStringEx, CStringEx > TDfnElt;
	typedef std::vector< TDfnElt > TDfn;

protected: // create from serialization only

	CDfnDoc ();
	DECLARE_DYNCREATE (CDfnDoc)

	CStringEx	DocumentName;
	bool		DocumentIsNew;

	CStringEx	DirLevel;
	CStringEx	DirPrototype;
	CStringEx	DirDfnTyp;

	FILE		*FileLock; // To lock the file (nobody else can access it)

	// The stack
	std::vector<TDfn>			UndoRedo;
	std::vector<TDfn>::iterator	itur;

	// The current dfn that is the representation of the grid
	TDfn		CurrentDfn;

// Attributes
public:

// Operations 
public:

	// The stack function
	void Undo ();			// Stack out
	void Redo ();			// Stack out
	void Push ();			// Stack in
	void ResetUndoRedo ();	// Stack RAZ

	void NewDocument (const CStringEx &_sxFilename);
	void UpdateDocument ();

	// Directories management
	CStringEx GetDirLevel		() const;
	CStringEx GetDirPrototype	() const;
	CStringEx GetDirDfnTyp		() const;

	void SetDirLevel		(const CStringEx &_sxDirectory);
	void SetDirPrototype	(const CStringEx &_sxDirectory);
	void SetDirDfnTyp		(const CStringEx &_sxDirectory);

public:
	virtual BOOL OnNewDocument ();
	virtual void DeleteContents ();

	virtual BOOL OnOpenDocument (LPCTSTR lpszPathName);
	void FileSave ();
	void FileSaveAs ();
	virtual BOOL OnSaveDocument (LPCTSTR lpszPathName);
	virtual void OnCloseDocument ();


public:
	virtual ~CDfnDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	DECLARE_MESSAGE_MAP()

};

#endif // __DFNDOC_H__
