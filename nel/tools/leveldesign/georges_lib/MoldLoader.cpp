// MoldLoader.cpp: implementation of the CMoldLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoldLoader.h"
#include "MoldElt.h"
#include "MoldEltDefine.h"
#include "MoldEltType.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldLoader::CMoldLoader()
{
}

CMoldLoader::~CMoldLoader()
{
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
	
	// find file extension
	int ipos = sxfn.reverse_find('.');
	if( ipos < 0 )
		return( 0 );											
	CStringEx sxfileextension = sxfn.get_right( sxfn.length()-ipos-1 );

	// define?
	if( sxfileextension == "dfn" )
	{
		CMoldEltDefine* pmed = new CMoldEltDefine( pl );
		pmed->SetList( blst );
		pmed->Load( CStringEx( "D:\\" +sxfn ) );
		return( pmed );
	}

	// typ?
	if( sxfileextension == "typ" )
	{
		CMoldEltType* pmet = new CMoldEltType( pl );
		pmet->SetList( blst );
		pmet->Load( CStringEx( "D:\\" +sxfn ) );
		return( pmet );
	}
	
	return( 0 );
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate )
{
	return( new CMoldElt(0) );
}
