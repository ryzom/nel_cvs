/** \file mold_loader.cpp
 * Georges system files
 *
 * $Id: mold_loader.cpp,v 1.3 2002/03/06 08:36:18 besson Exp $
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
#include "nel/georges/mold_loader.h"
#include "nel/georges/mold_elt.h"
#include "nel/georges/mold_elt_define.h"
#include "nel/georges/mold_elt_type.h"
#include "nel/georges/mold_elt_define_list.h"
#include "nel/georges/mold_elt_type_list.h"
#include "nel/georges/georges_loader.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldLoader::CMoldLoader()
{
}

CMoldLoader::~CMoldLoader()
{
	Clear();
}

void CMoldLoader::Clear()
{
	for( std::vector< CMoldElt* >::iterator it = vpme.begin(); it != vpme.end(); ++it )
		if( *it )
			delete( *it );
	vpme.clear();
	mmold.clear();
}

void CMoldLoader::SetLoader( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename )
{
	CStringEx sxfn = _sxfilename;	
	sxfn.purge();
	if( sxfn.empty() )
		return( 0 );											

	// liste?
	bool blst = ( sxfn.find( "list<" ) != -1 );
	if( blst )
	{
		unsigned int ipos = sxfn.find( ">" );
		if( ipos < 0 )
			return( 0 );											
		sxfn.mid( 5, ipos-5 ); 
	}

	// find extension
	int ipos = sxfn.reverse_find('.');
	if( ipos < 0 )
		return( 0 );											
	CStringEx sxfileextension = sxfn.get_right( sxfn.length()-ipos-1 );

	// Get only the filename
	ipos = sxfn.reverse_find('\\');
	if (ipos >= 0)
		sxfn = sxfn.get_right (sxfn.length()-ipos-1);

	// find if loaded
	CMoldElt* pme;
	CStringEx sxfullname = pl->WhereIsDfnTyp( sxfn );
	if (sxfullname == "")
		throw NLMISC::Exception ("Unable to find " + sxfn);

	sxfullname.make_lower();
	std::map< CStringEx, CMoldElt* >::iterator it;

	// DEBUG
	/*for (it = mmold.begin(); it != mmold.end(); ++it)
	{
		CMoldElt *pME = it->second;
		CStringEx sTmp = it->first + " Name:" + pME->GetName() + " formula:" + pME->GetFormula();
		nlwarning (sTmp.c_str());
	}*/
	// DEBUG

	it = mmold.find( sxfullname );
	if( it != mmold.end() )
		if( blst )
		{
			if( sxfileextension == "dfn" )
				pme = new CMoldEltDefineList( pl, dynamic_cast< CMoldEltDefine* >( it->second ) );
			else if( sxfileextension == "typ" )
					pme = new CMoldEltTypeList( pl, dynamic_cast< CMoldEltType* >( it->second ) );
				else
					return 0;
			pme->SetName( it->second->GetName() );
			vpme.push_back( pme );			
			return( pme );
		}
		else
			return( it->second );

	// load
	if( sxfileextension == "dfn" )
	{
		pme = new CMoldEltDefine( pl );
		vpme.push_back( pme );			
		if( blst )
		{
			pme = new CMoldEltDefineList( pl, dynamic_cast< CMoldEltDefine* >( pme ) );
			vpme.push_back( pme );			
		}
	}
	else if( sxfileextension == "typ" )
		{
			pme = new CMoldEltType( pl );
			vpme.push_back( pme );			
			if( blst )
			{
				pme = new CMoldEltTypeList( pl, dynamic_cast< CMoldEltType* >( pme ) );
				vpme.push_back( pme );			
			}
		}
	else 
		return( 0 );

	pme->SetName( sxfn );
	pme->Load( sxfullname );
	mmold.insert( std::make_pair( sxfullname, pme->GetMold() ) );
	return( pme );
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate )
{
	return( 0 );
}
/*
CMoldElt* CMoldLoader::Find( const CStringEx _sxfullname )
{
	std::map< CStringEx, CMoldElt* >::iterator it = mmold.find( _sxfullname );
	if( it != mmold.end() )
		return( it->second );
	return( 0 );
}
*/

}