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
	pl = _pl;
	benum = false;
	blist = false;
}

CMoldElt::~CMoldElt()
{
}

CStringEx CMoldElt::GetFormula()
{
	return sxname;
}

void CMoldElt::Load( const CStringEx _sxfullname )
{
}

void CMoldElt::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
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

CStringEx CMoldElt::GetName() const
{
	return( sxname );
}

void CMoldElt::SetName( const CStringEx& _sxname )
{
	sxname = _sxname;
}

bool CMoldElt::IsEnum() const
{
	return( benum );
}

bool CMoldElt::IsList() const
{
	return( blist );
}

CMoldElt* CMoldElt::GetMold()
{
	return( this );
}
