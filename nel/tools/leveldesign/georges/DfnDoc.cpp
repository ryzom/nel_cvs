// GeorgesDoc.cpp : implementation of the CDfnDoc class
//

#include "stdafx.h"
#include "Georges.h"

#include "../georges_lib/item.h"
#include "../georges_lib/itemElt.h"

#include "DfnDoc.h"
#include "DfnView.h"

#include "share.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDfnDoc

IMPLEMENT_DYNCREATE (CDfnDoc, CDocument)

BEGIN_MESSAGE_MAP (CDfnDoc, CDocument)
	//{{AFX_MSG_MAP(CDfnDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDfnDoc construction/destruction

// ---------------------------------------------------------------------------
CDfnDoc::CDfnDoc()
{
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	SetDirLevel (pApp->GetDirLevel());
	SetDirPrototype (pApp->GetDirPrototype());
	SetDirDfnTyp (pApp->GetDirDfnTyp());
	FileLock = NULL;
}

// ---------------------------------------------------------------------------
CDfnDoc::~CDfnDoc()
{
}

// ****************
// Stack Management
// ****************

// ---------------------------------------------------------------------------
void CDfnDoc::Undo()
{
	if (itur != UndoRedo.begin())
		--itur;
	CurrentDfn = *itur;

	POSITION pos = GetFirstViewPosition ();
	CDfnView* pDfnView = (CDfnView*)GetNextView (pos);
	pDfnView->makeFromDfn (CurrentDfn);
}

// ---------------------------------------------------------------------------
void CDfnDoc::Redo()
{
	if (itur != UndoRedo.end())
		++itur;
	if (itur != UndoRedo.end())
	{
		CurrentDfn = *itur;
		
		POSITION pos = GetFirstViewPosition ();
		CDfnView* pDfnView = (CDfnView*)GetNextView (pos);
		pDfnView->makeFromDfn (CurrentDfn);
	}
	else
		--itur;
}

// ---------------------------------------------------------------------------
void CDfnDoc::Push()
{
	++itur;
	UndoRedo.erase (itur, UndoRedo.end());

	POSITION pos = GetFirstViewPosition ();
	CDfnView* pDfnView = (CDfnView*)GetNextView (pos);
	CurrentDfn.clear ();
	pDfnView->convertToDfn (CurrentDfn);

	UndoRedo.push_back (CurrentDfn);
	itur = UndoRedo.end ();
	--itur;
	SetModifiedFlag (TRUE);
}

// ---------------------------------------------------------------------------
void CDfnDoc::ResetUndoRedo()
{
	UndoRedo.clear ();
	UndoRedo.push_back (CurrentDfn);
	itur = UndoRedo.end ();
	--itur;
}

/////////////////////////////////////////////////////////////////////////////
// CDfnDoc diagnostics

#ifdef _DEBUG
void CDfnDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDfnDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDfnDoc commands

// ---------------------------------------------------------------------------
void CDfnDoc::OnCloseDocument() 
{
	// Unlock
	if (FileLock != NULL)
		fclose (FileLock);
	DeleteContents();
	CDocument::OnCloseDocument();
}

// *********
// Load/Save
// *********

// ---------------------------------------------------------------------------
BOOL CDfnDoc::OnOpenDocument (LPCTSTR lpszPathName) 
{
	DocumentName = CStringEx(lpszPathName);
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	CLoader *pLoader = pApp->GetLoader();

	pApp->SetDirLevel (DirLevel);
	pApp->SetDirPrototype (DirPrototype);
	pApp->SetDirDfnTyp (DirDfnTyp);
	DeleteContents ();

	// Test if the file is already locked ?
	if ((FileLock = _fsopen(lpszPathName, "r", _SH_DENYRW)) == NULL)
	{
		string msg = string("File ") + lpszPathName + string(" is already opened by someone else");
		pApp->m_pMainWnd->MessageBox (msg.c_str(), "Cannot Open File", MB_ICONWARNING|MB_OK);
		return FALSE;
	}
	else
	{
		fclose (FileLock);
	}

	// Load from an georges form
	try
	{
		CFormLoader FormLoader;
		CForm Form;
		FormLoader.LoadForm (Form, CStringEx(lpszPathName));
		CFormBodyEltStruct *pFBES = Form.GetBody();

		// Convert to a vector of string,string (name,type)
		uint32 nNbElt = pFBES->GetNbElt ();
		CurrentDfn.clear();
		for (uint32 i = 0; i < nNbElt; ++i)
		{
			CFormBodyElt *pElt = pFBES->GetElt (i);
			CStringEx name = pElt->GetName ();
			CStringEx curval = pElt->GetValue ();
			CurrentDfn.push_back (make_pair<CStringEx,CStringEx>(name,curval));
		}

		// Update the view with this vector
		POSITION pos = GetFirstViewPosition ();
		CDfnView* pDfnView = (CDfnView*)GetNextView (pos);
		pDfnView->makeFromDfn (CurrentDfn);

		SetModifiedFlag (FALSE);
	}
	catch (NLMISC::Exception &e)
	{
		pApp->m_pMainWnd->MessageBox (e.what(), "CDfnDoc::OnOpenDocument", MB_ICONERROR|MB_OK);
		return FALSE;
	}

	ResetUndoRedo ();

	// Put a lock onto the file
	FileLock = _fsopen (lpszPathName, "r", _SH_DENYRW);

	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CDfnDoc::OnSaveDocument (LPCTSTR lpszPathName) 
{
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	CLoader *pLoader = pApp->GetLoader();

	// Unlock
	if (FileLock != NULL)
		fclose (FileLock);

	try
	{
		POSITION pos = GetFirstViewPosition ();
		CDfnView* pDfnView = (CDfnView*)GetNextView (pos);
		CurrentDfn.clear ();
		pDfnView->convertToDfn (CurrentDfn);
		pLoader->MakeDfn (CStringEx(lpszPathName), &CurrentDfn);
		SetModifiedFlag (FALSE);
	}
	catch (NLMISC::Exception &e)
	{
		pApp->m_pMainWnd->MessageBox (e.what(), "CDfnDoc::OnSaveDocument", MB_ICONERROR|MB_OK);
		return FALSE;
	}

	ResetUndoRedo ();

	// Relock file
	FileLock = _fsopen (lpszPathName, "r", _SH_DENYRW);

	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CDfnDoc::OnNewDocument()
{
	NewDocument ("");
	return TRUE;
}

// ---------------------------------------------------------------------------
void CDfnDoc::NewDocument (const CStringEx &_sxFilename)
{
	CGeorgesApp* pApp = dynamic_cast<CGeorgesApp*>(AfxGetApp());
	pApp->SetDirLevel		(DirLevel);
	pApp->SetDirPrototype	(DirPrototype);
	pApp->SetDirDfnTyp		(DirDfnTyp);
	DeleteContents ();

	DocumentName = _sxFilename;

	ResetUndoRedo ();
	SetModifiedFlag (FALSE);
}

// ---------------------------------------------------------------------------
void CDfnDoc::DeleteContents () 
{
	CurrentDfn.clear ();
}

// ---------------------------------------------------------------------------
void CDfnDoc::UpdateDocument() 
{
	UpdateAllViews (0);
}

// ********************
// Directory management
// ********************

// ---------------------------------------------------------------------------
CStringEx CDfnDoc::GetDirLevel		() const
{
	return DirLevel;
}

// ---------------------------------------------------------------------------
CStringEx CDfnDoc::GetDirPrototype	() const
{
	return DirPrototype;
}

// ---------------------------------------------------------------------------
CStringEx CDfnDoc::GetDirDfnTyp		() const
{
	return DirDfnTyp;
}

// ---------------------------------------------------------------------------
void CDfnDoc::SetDirLevel		(const CStringEx &_sxDirectory)
{
	DirLevel = _sxDirectory;
}

// ---------------------------------------------------------------------------
void CDfnDoc::SetDirPrototype	(const CStringEx &_sxDirectory)
{
	DirPrototype = _sxDirectory;
}

// ---------------------------------------------------------------------------
void CDfnDoc::SetDirDfnTyp		(const CStringEx &_sxDirectory)
{
	DirDfnTyp = _sxDirectory;
}