// ---------------------------------------------------------------------------
//
// GeorgesDoc.h : interface of the CGeorgesDoc class
//
// ---------------------------------------------------------------------------

#ifndef __GEORGESDOC_H__
#define __GEORGESDOC_H__

// ---------------------------------------------------------------------------

#include "../georges_lib/Item.h"

// ---------------------------------------------------------------------------

class CFormFile;
class CStringList;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
class CGeorgesDoc : public CDocument
{

protected:

	CGeorgesDoc ();
	DECLARE_DYNCREATE (CGeorgesDoc)
	DECLARE_MESSAGE_MAP ()

protected:

	CItem		CurItem;

	FILE		*FileLock;

	bool		DocumentIsNew;
	CStringEx	DocumentName;

	CStringEx	DirDfnTyp;
	CStringEx	DirPrototype;
	CStringEx	DirLevel;

	std::vector< CForm > UndoRedo;
	std::vector< CForm >::iterator itur;

public:

	virtual ~CGeorgesDoc();

	// ----------------
	// Stack operations
	// ----------------

	// Undo get previous version of the form, convert it to an item and update the view
	void Undo();

	// Redo is like undo but serve to navigate through the stack
	void Redo();

	// Push stack in the current item (called item)
	void Push();

	// Reset the stack (done when saving, loading and the like)
	void ResetUndoRedo();

	// ---------------
	// Item operations
	// ---------------

	// * Add *
	// *******

	// Add a new item to a list
	void AddList (uint32 nIndex);

	// Add a new parent with activity=false (update the view)
	void AddParent (uint32 nIndex);

	// * Del *
	// *******

	void DelListChild (uint32 nIndex);
	void DelParent (uint32 nIndex);

	// * Set *
	// *******

	void SetItemValue (uint32 nIndex, const CString sValue);

	// (update the view)
	void SetItemParent (uint32 nIndex, const CString sString);

	// Set the parent acitvity (sBool == "true" or "false") (update the view)
	void SetItemActivity (uint32 nIndex, const CString sBool);

	// * Get *
	// *******
	
	uint32 GetItemNbElt () const;
	uint32 GetItemNbElt (uint32 nIndex) const;
	uint32 GetItemNbParent () const;
	uint32 GetItemInfos (uint32 nIndex) const;
	CString GetItemName (uint32 nIndex ) const;
	CString GetItemCurrentResult (uint32 nIndex) const;
	CString GetItemCurrentValue (uint32 nIndex) const;
	CString GetItemFormula (uint32 nIndex) const;
	CString GetItemActivity (uint32 nIndex) const;
	CString GetItemParent (uint32 nIndex) const;
	bool IsItemEnum (uint32 nIndex) const;
	bool IsItemPredef (uint32 nIndex) const;
	bool CanEditItem (uint32 nIndex) const;
	void GetItemListPredef (uint32 nIndex, CStringList* _slist) const;

	// ----------------------
	// Directories management
	// ----------------------

	// * Get *
	// *******
	CStringEx GetDirLevel		() const;
	CStringEx GetDirPrototype	() const;
	CStringEx GetDirDfnTyp		() const;

	// * Set *
	// *******
	void SetDirLevel		(const CStringEx &_sxDirectory);
	void SetDirPrototype	(const CStringEx &_sxDirectory);
	void SetDirDfnTyp		(const CStringEx &_sxDirectory);


	// -----------------
	// New / Load / Save
	// -----------------
	
	virtual BOOL OnNewDocument ();
	void NewDocument (const CStringEx _sxfilename);
	virtual BOOL OnOpenDocument (LPCTSTR lpszPathName);
	void FileSave ();
	void FileSaveAs ();
	virtual BOOL OnSaveDocument (LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void DeleteContents();
	void UpdateDocument ();



	// -----
	// Debug
	// -----
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

#endif // __GEORGESDOC_H__
