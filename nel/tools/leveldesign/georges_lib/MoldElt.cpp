// MoldElt.cpp: implementation of the CMoldElt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoldElt.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldElt::CMoldElt( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
	bliste = false;
	benum = false;
}

CMoldElt::~CMoldElt()
{
}

CStringEx CMoldElt::GetFormula()
{
	return sxname;
}

void CMoldElt::Load( const CStringEx _sxfilename )
{
}

void CMoldElt::Load( const CStringEx _sxfilename, const CStringEx _sxdate )
{
}

CStringEx CMoldElt::GetEltName( const unsigned int _index ) const
{
	return( CStringEx() );
}

CMoldElt* CMoldElt::GetEltPtr( const unsigned int _index ) const
{
	return( 0 );
}

unsigned int CMoldElt::GetType() const
{
	return( -1 );
}

void CMoldElt::SetList( const bool _b )
{
	bliste = _b;
}

bool CMoldElt::IsList() const
{
	return( bliste );
}

bool CMoldElt::IsEnum() const
{
	return( benum );
}

