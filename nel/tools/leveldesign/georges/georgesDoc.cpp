// georgesDoc.cpp : implementation of the CGeorgesDoc class
//

#include "stdafx.h"
#include "georges.h"

#include "georgesDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeorgesDoc

IMPLEMENT_DYNCREATE(CGeorgesDoc, CDocument)

BEGIN_MESSAGE_MAP(CGeorgesDoc, CDocument)
	//{{AFX_MSG_MAP(CGeorgesDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesDoc construction/destruction

CGeorgesDoc::CGeorgesDoc()
{
	// TODO: add one-time construction code here

}

CGeorgesDoc::~CGeorgesDoc()
{
}

BOOL CGeorgesDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CGeorgesDoc serialization

void CGeorgesDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesDoc diagnostics

#ifdef _DEBUG
void CGeorgesDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGeorgesDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeorgesDoc commands
