// Form.cpp: implementation of the CForm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Form.h"
#include "FormBodyElt.h"

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

void CForm::SetDate( const CStringEx _sxdate )
{
	head.SetDate( _sxdate );
}

void CForm::SetVersion( const CStringEx _sxversion )
{
	head.SetVersion( _sxversion );
}

void CForm::SetUser( const CStringEx _sxuser )
{
	head.SetUser( _sxuser );
}

void CForm::SetComment( const CStringEx _sxcomment )
{
	body.SetComment( _sxcomment );
}

CStringEx CForm::GetParent( unsigned int _index ) const
{
	return( body.GetParent( _index ) );
}

CStringEx CForm::GetActivity( unsigned int _index ) const
{
	return( body.GetActivity( _index ) );
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
	body -= _f.body;
	bmodified = true;
	return( *this );
}

bool CForm::operator ==( const CForm& _f ) const
{
	return( (body == _f.body) );
}