// Form.cpp: implementation of the CForm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Form.h"
//#include "FormBodyEltStruct.h"
#include "FormBodyElt.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CForm::CForm()
{
	body.SetName( "BODY" );
	bmodified = false;
}

CForm::CForm( const CForm& _f )
{
	bmodified = _f.bmodified;
	head = _f.head;
	body = _f.body;
}

CForm::~CForm()
{
}

void CForm::serial( NLMISC::IStream& s )
{
	s.serial( head );
	s.serial( body );
}

CStringEx CForm::GetDate() const
{
	return( head.GetDate() );
}

CStringEx CForm::GetVersion() const
{
	return( head.GetVersion() );
}

CStringEx CForm::GetUser() const
{
	return( head.GetUser() );
}

CStringEx CForm::GetComment() const
{
	return( body.GetComment() );
}

CStringEx CForm::GetParent( unsigned int _index ) const
{
	return( body.GetParent( _index ) );
}

CFormBodyElt* CForm::GetElt( const CStringEx _sxname ) const
{
	return( body.GetElt( _sxname ) );
}

CFormBodyEltStruct* CForm::GetBody() const
{
	return( (CFormBodyEltStruct*)(&body) );
}

bool CForm::GetModified() const
{
	return( bmodified );
}

void CForm::SetModified( const bool _b )
{
	bmodified = _b;
}

CForm& CForm::operator =( const CForm& _f )
{
	head = _f.head;
	body = _f.body;
	bmodified = true;
	return( *this );
}

CForm& CForm::operator +=( const CForm& _f )
{
	head = _f.head;
	body += _f.body;
	bmodified = true;
	return( *this );
}

CForm& CForm::operator -=( const CForm& _f )
{
//	head = _f.head;												// TODO: vérify
	body -= _f.body;
	bmodified = true;
	return( *this );
}

bool CForm::operator ==( const CForm& _f ) const
{
	return( (body == _f.body) );
}