// FormBodyElt.cpp: implementation of the CFormBodyElt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormBodyElt.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyElt::CFormBodyElt()
{
}

CFormBodyElt::CFormBodyElt( const CFormBodyElt& _fbe )
{
}

CFormBodyElt::~CFormBodyElt()
{
}

void CFormBodyElt::serial( NLMISC::IStream& s)
{
}

CFormBodyElt& CFormBodyElt::operator =( const CFormBodyElt& _fbe )
{
	return( *this );
}

CFormBodyElt& CFormBodyElt::operator +=( const CFormBodyElt& _fbe )
{
	return( *this );
}

CFormBodyElt& CFormBodyElt::operator -=( const CFormBodyElt& _fbe )
{
	return( *this );
}

bool CFormBodyElt::operator ==( const CFormBodyElt& _fbe ) const
{
	return( false );
}

CFormBodyElt* CFormBodyElt::Clone() const 
{
	return( new CFormBodyElt( *this ) );
}

CStringEx CFormBodyElt::GetName() const
{
	return( sxname );
}

void CFormBodyElt::SetName( const CStringEx _sxname )
{
	sxname = _sxname;
}

bool CFormBodyElt::Empty() const
{
	return( false );
}

CStringEx CFormBodyElt::GetValue() const
{
	return( CStringEx() );
}

CFormBodyElt* CFormBodyElt::GetElt( const unsigned int _index ) const
{
	return( 0 );
}

CFormBodyElt* CFormBodyElt::GetElt( const CStringEx _sxname ) const
{
	return( 0 );
}

