// FormBodyEltStruct.cpp: implementation of the CFormBodyEltStruct class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormBodyEltStruct.h"
#include "FormBodyEltList.h"
#include "FormBodyEltAtom.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyEltStruct::CFormBodyEltStruct()
{
}

CFormBodyEltStruct::CFormBodyEltStruct( const CFormBodyEltStruct& _fbes )
{
	sxname = _fbes.sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = _fbes.vpbodyelt.begin(); it != _fbes.vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
}

CFormBodyEltStruct::~CFormBodyEltStruct()
{
	Clear();
}

void CFormBodyEltStruct::Clear()
{
	for( std::vector< CFormBodyElt* >::iterator it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( *it )
			delete *it;
	vpbodyelt.clear();
}

void CFormBodyEltStruct::serial( NLMISC::IStream& s)
{
	s.xmlPushBegin( "Body" );
		s.xmlSetAttrib( "Name" );
			s.serial( sxname );
	s.xmlPushEnd();
		s.serialContPolyPtr( vpbodyelt );
	s.xmlPop();
}

std::vector< CFormBodyElt* >::iterator CFormBodyEltStruct::Find( const CStringEx _sxname ) 
{
	for( std::vector< CFormBodyElt* >::iterator it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return it;
	return it;
}

std::vector< CFormBodyElt* >::const_iterator CFormBodyEltStruct::Find( const CStringEx _sxname ) const 
{
	for( std::vector< CFormBodyElt* >::const_iterator it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return it;
	return it;
}

CFormBodyElt& CFormBodyEltStruct::operator =( const CFormBodyElt& _fbe )
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	Clear();
	sxname = pfbes->sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
	return( *this );
}

CFormBodyElt& CFormBodyEltStruct::operator =( const CFormBodyEltStruct& _fbes )
{
	Clear();
	sxname = _fbes.sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = _fbes.vpbodyelt.begin(); it != _fbes.vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
	return( *this );
}

CFormBodyElt& CFormBodyEltStruct::operator +=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
	{
		std::vector< CFormBodyElt* >::iterator iu = Find( (*it)->GetName() );
		if( iu == vpbodyelt.end() )
		{
			vpbodyelt.push_back( (*it)->Clone() );
			iu = vpbodyelt.end();									
			--iu;
		}
		**iu += **it;	
	}
	return( *this );
}

CFormBodyElt& CFormBodyEltStruct::operator -=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
	{
		std::vector< CFormBodyElt* >::iterator iu = Find( (*it)->GetName() );
		if( iu == vpbodyelt.end() )
			continue;
		**iu -= **it;
		if( (*iu)->Empty() )
		{
			delete( *iu );
			vpbodyelt.erase( iu );
		}
	}
	return( *this );
}

bool CFormBodyEltStruct::operator ==( const CFormBodyElt& _fbe ) const
{
	const CFormBodyEltStruct* pfbes = dynamic_cast< const CFormBodyEltStruct* >( &_fbe );      
	nlassert( pfbes );
	if( vpbodyelt.size() != pfbes->vpbodyelt.size() )
		return( false );
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbes->vpbodyelt.begin(); it != pfbes->vpbodyelt.end(); ++it )
	{
		std::vector< CFormBodyElt* >::const_iterator iu =  Find( (*it)->GetName() );
		if( ( iu == vpbodyelt.end() )||( !(**iu == **it) ) )
			return false;
	}
	return( true );
}

CFormBodyElt* CFormBodyEltStruct::Clone() const 
{
	return( new CFormBodyEltStruct( *this ) );
}

bool CFormBodyEltStruct::Empty() const
{
	return( vpbodyelt.empty() );
}

CFormBodyElt* CFormBodyEltStruct::GetElt( const unsigned int _index ) const
{
	if( _index >= vpbodyelt.size() )
		return( 0 );
	return( vpbodyelt[_index] ); 
}

CFormBodyElt* CFormBodyEltStruct::GetElt( const CStringEx _sxname ) const
{
	std::vector< CFormBodyElt* >::const_iterator it = Find( _sxname );
	if( it == vpbodyelt.end() ) 
		return( 0 );
	return( *it );
}

CStringEx CFormBodyEltStruct::GetComment() const
{
	std::vector< CFormBodyElt* >::const_iterator it =  Find( SXCOMMENT );
	if( it == vpbodyelt.end() )
		return( CStringEx() );									
	const CFormBodyEltAtom* pfbea = dynamic_cast< CFormBodyEltAtom* >( *it );      
	nlassert( pfbea );
	return pfbea->GetValue();
}

CStringEx CFormBodyEltStruct::GetParent( unsigned int _index ) const
{
	std::vector< CFormBodyElt* >::const_iterator it =  Find( SXPARENTS );
	if( it == vpbodyelt.end() )
		return( CStringEx() );									
	const CFormBodyEltList* pfbel = dynamic_cast< CFormBodyEltList* >( *it );      
	if( !pfbel )
		return( CStringEx() );
	const CFormBodyEltStruct* pfbes = dynamic_cast< CFormBodyEltStruct* >( pfbel->GetElt( _index ) );      
	if( !pfbes )
		return( CStringEx() );
	const CFormBodyEltAtom* pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Activity" ) );      
	if( !pfbea )
		return( CStringEx() );
	CStringEx sxactivity = pfbea->GetValue();
	if( sxactivity != "true" )
		return( CStringEx(" ") );
	pfbea = dynamic_cast< CFormBodyEltAtom* >( pfbes->GetElt( "Filename" ) );
	if( !pfbea )
		return( CStringEx() );
	return pfbea->GetValue();
}

void CFormBodyEltStruct::AddElt( CFormBodyElt* const pfbe )
{
	if( !pfbe )
		return;
	vpbodyelt.push_back( pfbe );
}


