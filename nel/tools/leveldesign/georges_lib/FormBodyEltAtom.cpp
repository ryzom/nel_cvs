// FormBodyEltAtom.cpp: implementation of the CFormBodyEltAtom class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormBodyEltAtom.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyEltAtom::CFormBodyEltAtom()
{
}

CFormBodyEltAtom::CFormBodyEltAtom( const CFormBodyEltAtom& _fbea ) : CFormBodyElt( _fbea )
{
	sxname = _fbea.sxname;
	sxvalue = _fbea.sxvalue;
}

CFormBodyEltAtom::~CFormBodyEltAtom()
{
}

void CFormBodyEltAtom::serial( NLMISC::IStream& s)
{

	s.xmlPushBegin( "Atom" );
		s.xmlSetAttrib( "Name" );
			s.serial( sxname );
		s.xmlSetAttrib( "Value" );
			s.serial( sxvalue );
	s.xmlPushEnd();
	s.xmlPop();
}

CFormBodyElt& CFormBodyEltAtom::operator =( const CFormBodyElt& _fbe )
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	sxname = pfbea->sxname;
	sxvalue = pfbea->sxvalue;
	return( *this );
}

CFormBodyElt& CFormBodyEltAtom::operator +=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	sxvalue = pfbea->sxvalue;
	return( *this );
}

CFormBodyElt& CFormBodyEltAtom::operator -=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	if( sxvalue == pfbea->sxvalue )
		sxvalue.erase();
	return( *this );
}

bool CFormBodyEltAtom::operator ==( const CFormBodyElt& _fbe ) const
{
	const CFormBodyEltAtom* pfbea = dynamic_cast< const CFormBodyEltAtom* >( &_fbe );      
	nlassert( pfbea );
	return( sxvalue == pfbea->sxvalue );
}

CFormBodyElt* CFormBodyEltAtom::Clone() const 
{
	return( new CFormBodyEltAtom( *this ) );
}

bool CFormBodyEltAtom::Empty() const
{
	return( sxvalue.empty() );
}

CStringEx CFormBodyEltAtom::GetValue() const
{
	return( sxvalue );
}

void CFormBodyEltAtom::SetValue( const CStringEx _sxvalue )
{
	sxvalue = _sxvalue;
}
