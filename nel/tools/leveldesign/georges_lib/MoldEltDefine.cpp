// MoldEltDefine.cpp: implementation of the CMoldEltDefine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader.h"
#include "MoldEltDefine.h"
#include "FormBodyEltStruct.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

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
// Download the form of _sxfilename and find the "data" field of the form
// For each element of "data"
//		Get the name and the filename
//		Download the mold of the filename
//		push_back the pair ( name, moldelt* )
{
	CForm f;
	sxname = CStringEx( _sxfilename );
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
