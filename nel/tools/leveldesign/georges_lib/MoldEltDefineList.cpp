// MoldEltDefine.cpp: implementation of the CMoldEltDefine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader.h"
#include "MoldEltDefineList.h"
#include "FormBodyEltStruct.h"

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
