// MoldEltDefine.cpp: implementation of the CMoldEltDefine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader.h"
#include "MoldEltDefine.h"
#include "FormBodyEltStruct.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltDefine::CMoldEltDefine( CLoader* const _pl ) : CMoldElt( _pl )
{
}

CMoldEltDefine::~CMoldEltDefine()
{
}

void CMoldEltDefine::Load( const CStringEx _sxfilename )
{
	CForm f;
	pl->LoadForm( f, _sxfilename );
	CFormBodyElt* pbody = f.GetBody();

	unsigned int i = 0;
	CFormBodyElt* pfbeatom = pbody->GetElt( i++ );
	while( pfbeatom )
	{
		CStringEx sxvalue = pfbeatom->GetValue();
		CStringEx sxname = pfbeatom->GetName();
		pfbeatom = pbody->GetElt( i++ );
		CMoldElt* pme = pl->LoadMold( sxvalue );
		if( pme )
			vpair.push_back( std::make_pair( sxname, pme ) );
	}
}

void CMoldEltDefine::Load( const CStringEx _sxfilename, const CStringEx _sxdate )
{
}

CStringEx CMoldEltDefine::GetEltName( const unsigned int _index ) const
{
	if( _index < vpair.size() )
		return( vpair[_index].first );
	return( 0 );
}

CMoldElt* CMoldEltDefine::GetEltPtr( const unsigned int _index ) const
{
	if( _index < vpair.size() )
		return( vpair[_index].second );
	return( 0 );
}

unsigned int CMoldEltDefine::GetType() const
{
	return( 1 );
}

CMoldElt* CMoldEltDefine::GetMold()
{
	return( this );
}

