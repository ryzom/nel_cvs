/** \file mold_elt_define_list.cpp
 * Georges system files
 *
 * $Id: mold_elt_define_list.cpp,v 1.2 2002/02/21 16:54:42 besson Exp $
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
#include "nel/georges/georges_loader.h"
#include "nel/georges/mold_elt_define_list.h"
#include "nel/georges/form_body_elt_struct.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltDefineList::CMoldEltDefineList( CLoader* const _pl, CMoldEltDefine* const _pmed ) : CMoldEltDefine( _pl )
{
	pmed = _pmed;
	blist = true;
	benum = pmed->IsEnum();
	sxname = pmed->GetName();
}

CMoldEltDefineList::~CMoldEltDefineList()
{
}

void CMoldEltDefineList::Load( const CStringEx _sxfullname )
{
	pmed->Load( _sxfullname );
}

void CMoldEltDefineList::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
	pmed->Load( _sxfullname, _sxdate );
	benum = pmed->IsEnum();
	sxname = pmed->GetName();
}

CStringEx CMoldEltDefineList::GetEltName( const unsigned int _index ) const
{
	return( pmed->GetEltName( _index ) );
}

CMoldElt* CMoldEltDefineList::GetEltPtr( const unsigned int _index ) const
{
	return( pmed->GetEltPtr( _index ) );
}

unsigned int CMoldEltDefineList::GetType() const
{
	return( pmed->GetType() );
}

CMoldElt* CMoldEltDefineList::GetMold()
{
	return( pmed );
}
/*
CStringEx CMoldEltDefineList::GetName() const													
{
	return( pmed->GetName() );
}

void CMoldEltDefineList::SetName( const CStringEx& _sxname )
{
	pmed->SetName( _sxname );
}

bool CMoldEltDefineList::IsEnum() const
{
	return( pmed->IsEnum() );
}

bool CMoldEltDefineList::IsList() const
{
	return( pmed->IsList() );
}
*/

}