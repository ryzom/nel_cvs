// GeorgesDoc.cpp : implementation of the CGeorgesDoc class
//

#include "stdafx.h"
#include "Georges.h"

#include "GeorgesDoc.h"

#include "../georges_lib/Form.h"
#include "../georges_lib/FormFile.h"
#include "../georges_lib/FormBodyEltAtom.h"
#include "../georges_lib/FormBodyEltList.h"
#include "../georges_lib/FormBodyEltStruct.h"
#include "../georges_lib/Item.h"

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
	//NLMISC_REGISTER_CLASS( CFormBodyElt );
	//NLMISC_REGISTER_CLASS( CFormBodyEltAtom );
	//NLMISC_REGISTER_CLASS( CFormBodyEltList );
	//NLMISC_REGISTER_CLASS( CFormBodyEltStruct );

	item.SetLoader( &loader );
/*
	pff = new CFormFile();
	CForm f;

	CFormBodyEltStruct* pbody = f.GetBody();
	CFormBodyEltStruct* pfbes;
	CFormBodyEltList* pfbel;
	CFormBodyEltAtom* pfbea;
*/
/* 
//Ecriture de object.test
	pfbes = new CFormBodyEltStruct;
	pfbes->SetName( "Vector3D" );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "X" );
	pfbea->SetValue( "0.0" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Y" );
	pfbea->SetValue( "0.0" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Z" );
	pfbea->SetValue( "0.0" );
	pfbes->AddElt( pfbea );
	pbody->AddElt( pfbes );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Vitesse" );
	pfbea->SetValue( "100" );
	pbody->AddElt( pfbea );
*/
/*
//Ecriture de test.dfn
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Vector3D" );
	pfbea->SetValue( "V3D.dfn" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Vitesse" );
	pfbea->SetValue( "MyInt.typ" );
	pbody->AddElt( pfbea );
*/
/*
//Ecriture de V3D.dfn
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "X" );
	pfbea->SetValue( "MyFloat.typ" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Y" );
	pfbea->SetValue( "MyFloat.typ" );
	pbody->AddElt( pfbea );
	
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Z" );
	pfbea->SetValue( "MyFloat.typ" );
	pbody->AddElt( pfbea );
*/
/*
//Ecriture de MyInt.typ
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "type" );
	pfbea->SetValue( "uint" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "enum" );
	pfbea->SetValue( "false" );
	pbody->AddElt( pfbea );
	
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "lowlimit" );
	pfbea->SetValue( "0" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "highlimit" );
	pfbea->SetValue( "1000" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "defaultvalue" );
	pfbea->SetValue( "0" );
	pbody->AddElt( pfbea );
*/
/*
// suite en predef...
	pfbel = new CFormBodyEltList;
	pfbel->SetName( "predef" );
	
	pfbes = new CFormBodyEltStruct;
	pfbes->SetName( "#0" );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "designation" );
	pfbea->SetValue( "PREDEF_A" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "substitute" );
	pfbea->SetValue( "1" );
	pfbes->AddElt( pfbea );
	pfbel->AddElt( pfbes );
	
	pfbes = new CFormBodyEltStruct;
	pfbes->SetName( "#1" );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "designation" );
	pfbea->SetValue( "PREDEF_B" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "substitute" );
	pfbea->SetValue( "2" );
	pfbes->AddElt( pfbea );
	pfbel->AddElt( pfbes );

	pbody->AddElt( pfbel );
*/
/*
//Ecriture de MyFloat.typ
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "type" );
	pfbea->SetValue( "double" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "enum" );
	pfbea->SetValue( "false" );
	pbody->AddElt( pfbea );
	
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "lowlimit" );
	pfbea->SetValue( "0.0" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "highlimit" );
	pfbea->SetValue( "10000.0" );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "defaultvalue" );
	pfbea->SetValue( "0.0" );
	pbody->AddElt( pfbea );
*/
//---------------------------------------------
/* 
//Ecriture de object.testlst
	pfbel = new CFormBodyEltList;
	pfbel->SetName( "Position" );

	pfbes = new CFormBodyEltStruct;
	pfbes->SetName( "#0" );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "X" );
	pfbea->SetValue( "1.0" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Y" );
	pfbea->SetValue( "2.0" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Z" );
	pfbea->SetValue( "3.0" );
	pfbes->AddElt( pfbea );
	pfbel->AddElt( pfbes );

	pfbes = new CFormBodyEltStruct;
	pfbes->SetName( "#1" );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "X" );
	pfbea->SetValue( "4.0" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Y" );
	pfbea->SetValue( "5.0" );
	pfbes->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Z" );
	pfbea->SetValue( "6.0" );
	pfbes->AddElt( pfbea );
	pfbel->AddElt( pfbes );
	
	pbody->AddElt( pfbel );

	pfbel = new CFormBodyEltList;
	pfbel->SetName( "Vitesse" );
	
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "prem" );
	pfbea->SetValue( "10" );
	pfbel->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "deuz" );
	pfbea->SetValue( "20" );
	pfbel->AddElt( pfbea );

	pbody->AddElt( pfbel );
*/
/*
//Ecriture de testlst.dfn
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Position" );
	pfbea->SetValue( "list<V3D.dfn>" );
	pbody->AddElt( pfbea );
	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Vitesse" );
	pfbea->SetValue( "list<MyInt.dfn>" );
	pbody->AddElt( pfbea );
*/
//	pff->SetForm( f );
}

CGeorgesDoc::~CGeorgesDoc()
{
//	delete pff;
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesDoc serialization

void CGeorgesDoc::Serialize(CArchive& ar)
{

	BOOL Bstoring = ar.IsStoring();
	CFile* file = ar.GetFile();
	ASSERT( file );
	CStringEx sxfilename = LPCTSTR( file->GetFileName() );
	CStringEx sxfilepath = LPCTSTR( file->GetFilePath() );
	ar.Abort();
	file->Close();
	delete file;

	if( Bstoring )
		item.Save( sxfilepath );		
	else
		item.Load( sxfilepath );		

/*
	if (ar.IsStoring())
	{
//		pff->Save( "Object.test" );		
//		pff->Save( "test.dfn" );		
//		pff->Save( "V3D.dfn" );		
//		pff->Save( "MyInt.typ" );		
//		pff->Save( "MyFloat.typ" );		
//		pff->Save( "Object.testlst" );		
		pff->Save( "testlst.dfn" );		
	}
	else
	{
//		CItem item( &loader );
//		item.Load( "Object.test" );		
//		item.Save( "Result.test" );
	}
*/
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

BOOL CGeorgesDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	SetModifiedFlag( FALSE );
	return TRUE;
}

void CGeorgesDoc::OnCloseDocument() 
{
	CDocument::OnCloseDocument();
}

BOOL CGeorgesDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	DeleteContents();
	item.Load( CStringEx( lpszPathName ) );		
	SetModifiedFlag( FALSE );
	UpdateAllViews( NULL );
	return( true );
}

BOOL CGeorgesDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	item.Save( CStringEx( lpszPathName ) );		
	SetModifiedFlag( FALSE );
	UpdateAllViews( NULL );
	return( true );
}

void CGeorgesDoc::DeleteContents() 
{
	item.Clear();
}

/////////////////////////////////////////////////////////////////////:
unsigned int CGeorgesDoc::GetItemNbElt() const
{
	return( item.GetNbElt()-1 );
}

unsigned int CGeorgesDoc::GetItemInfos( const unsigned int _index ) const
{
	return( item.GetInfos( _index+1 ) );
}

void CGeorgesDoc::SetItemValue( const unsigned int _index, const CString s )
{
	item.SetCurrentValue( _index+1, CStringEx( LPCTSTR( s ) ) );
}

CString CGeorgesDoc::GetItemName( const unsigned int _index ) const
{
	return( CString( item.GetName( _index+1 ).c_str() ) );
}

CString CGeorgesDoc::GetItemCurrentResult( const unsigned int _index ) const
{
	return( CString( item.GetCurrentResult( _index+1 ).c_str() ) );
}

CString CGeorgesDoc::GetItemCurrentValue( const unsigned int _index ) const
{
	return( CString( item.GetCurrentValue( _index+1 ).c_str() ) );
}

CString CGeorgesDoc::GetItemFormula( const unsigned int _index ) const
{
	return( CString( item.GetFormula( _index+1 ).c_str() ) );
}


