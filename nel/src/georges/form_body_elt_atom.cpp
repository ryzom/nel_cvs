/** \file form_body_elt_atom.cpp
 * Georges system files
 *
 * $Id: form_body_elt_atom.cpp,v 1.1 2002/02/14 10:40:39 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdgeorges.h"
#include "nel/georges/form_body_elt_atom.h"

namespace NLGEORGES
{

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

}