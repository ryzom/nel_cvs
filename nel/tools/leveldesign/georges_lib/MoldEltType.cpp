// MoldEltType.cpp: implementation of the CMoldEltType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoldEltType.h"
#include "Loader.h"
#include "FormBodyElt.h"
#include "TypeUnitIntUnsigned.h"
#include "TypeUnitIntSigned.h"
#include "TypeUnitDouble.h"
#include "TypeUnitString.h"
#include "TypeUnitFileName.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltType::CMoldEltType( CLoader* const _pl ) : CMoldElt( _pl )
{
}

CMoldEltType::~CMoldEltType()
{
}

void CMoldEltType::Load( const CStringEx _sxfullname )					
{
	sxfullname = _sxfullname;
	CForm f;
	pl->LoadForm( f, _sxfullname );									

	CStringEx sxll, sxhl, sxdv;
	CFormBodyElt* pfbe;
	pfbe	= f.GetElt("Type");									
	if( pfbe )
		sxtype = pfbe->GetValue();
	pfbe	= f.GetElt("Lowlimit");									
	if( pfbe )
		sxll = pfbe->GetValue();
	pfbe	= f.GetElt("Highlimit");									
	if( pfbe )
		sxhl = pfbe->GetValue();
	pfbe	= f.GetElt("DefaultValue");									
	if( pfbe )
		sxdv = pfbe->GetValue();
	pfbe	= f.GetElt("Formula");									
	if( pfbe )
		sxformula = pfbe->GetValue();
	pfbe	= f.GetElt("Enum");									
	if( pfbe )
		sxenum = pfbe->GetValue();

	if( sxtype == "uint" )
		ptu = new CTypeUnitIntUnsigned( sxll, sxhl, sxdv, sxformula );
	else if( sxtype == "sint" )
		ptu = new CTypeUnitIntSigned( sxll, sxhl, sxdv, sxformula );
	else if( sxtype == "double" )
		ptu = new CTypeUnitDouble( sxll, sxhl, sxdv, sxformula );
	else if( sxtype == "filename" )
		ptu = new CTypeUnitFileName( sxll, sxhl, sxdv, sxformula );
	else if( sxtype == "string" )
		ptu = new CTypeUnitString( sxll, sxhl, sxdv, sxformula );

	benum = ( sxenum == "true" );

	CFormBodyElt* pfbepredef = f.GetElt("Predef");
	if( pfbepredef )
	{
		unsigned int i = 0;
		CFormBodyElt* pfbe = pfbepredef->GetElt( i++ );
		while( pfbe )
		{
			CFormBodyElt* pfbedesignation = pfbe->GetElt( "Designation" );
			CFormBodyElt* pfbesubstitute = pfbe->GetElt( "Substitute" );
			nlassert( pfbedesignation );
			nlassert( pfbesubstitute );
			vpredef.push_back( std::make_pair( pfbedesignation->GetValue(), pfbesubstitute->GetValue() ) );
			pfbe = pfbepredef->GetElt( i++ );
		}
	}
}

void CMoldEltType::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
}

CStringEx CMoldEltType::GetFormula()													 
{
	return( ptu->GetFormula() );
}

CStringEx CMoldEltType::Format( const CStringEx _sxvalue ) const													
{
	if( _sxvalue.empty() )
		return( _sxvalue );
	CStringEx sx = GetPredefSubstitute( _sxvalue );
	if( sx.empty() )
		if( benum )
			return( CStringEx() );
		else
			return( ptu->Format( _sxvalue ) );
	return( GetPredefDesignation( sx ) ); 
}

CStringEx CMoldEltType::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	CStringEx sx = GetPredefSubstitute( _sxvalue );
	if( benum )
	{
		CStringEx sx2 = ptu->CalculateResult( _sxbasevalue, sx );
		return( GetPredefDesignation( sx2 ) );
	}
	if( sx.empty() )
		return( ptu->CalculateResult( _sxbasevalue, _sxvalue ) );
	return( ptu->CalculateResult( _sxbasevalue, sx ) );
}

CStringEx CMoldEltType::GetDefaultValue() const	
{
	return( ptu->GetDefaultValue() );
}

unsigned int CMoldEltType::GetType() const
{
	return( 0 );
}

CStringEx CMoldEltType::GetPredefSubstitute( const CStringEx _sxdesignation ) const
{
	if( !_sxdesignation.empty() )
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = vpredef.begin(); it != vpredef.end(); ++it )
			if( it->first == _sxdesignation )
				return( it->second );
	return( CStringEx() );
}

CStringEx CMoldEltType::GetPredefDesignation( const CStringEx _sxsubstitute ) const
{
	for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = vpredef.begin(); it != vpredef.end(); ++it )
		if( it->second == _sxsubstitute )
			return( it->first );
	return( CStringEx() );
}

CStringEx CMoldEltType::GetPredefDesignation( const unsigned int _index ) const
{
	if( _index < vpredef.size() )
		return( vpredef[_index].first );
	return( CStringEx() );
}

CMoldElt* CMoldEltType::GetMold()
{
	return( this );
}

void CMoldEltType::SetTypPredef( const std::vector< CStringEx >& _pvsx )
{
	
	CStringEx sxdv = GetPredefSubstitute( ptu->GetDefaultValue() );

	vpredef.clear();
	if( sxtype == "string" || sxtype == "filename" )
		for( std::vector< CStringEx >::const_iterator it = _pvsx.begin(); it != _pvsx.end(); ++it )
			vpredef.push_back( std::make_pair( *it, *it ) );
	else if( sxtype == "uint" || sxtype == "sint" )
		{
			CStringEx sx;
			int i = 0;
			for( std::vector< CStringEx >::const_iterator it = _pvsx.begin(); it != _pvsx.end(); ++it )
			{
				sx.format( "%d", i++ );
				vpredef.push_back( std::make_pair( *it, sx ) );
			}		
			if( ( benum )&&( !vpredef.empty() ) )
			{
				ptu->SetLowLimit( vpredef[0].first );
				ptu->SetHighLimit( vpredef[_pvsx.size()-1].first );
				ptu->SetDefaultValue( vpredef[0].first );
			}
		}

	if( ! sxdv.empty() )
		ptu->SetDefaultValue( vpredef[0].first );
}

void CMoldEltType::Save()
{
	if( sxfullname.empty() )
		return;

	std::vector< std::pair< CStringEx, CStringEx > > lpsxparent;
	pl->MakeTyp( sxfullname, sxtype, sxformula, sxenum, ptu->GetLowLimit(), ptu->GetHighLimit(), ptu->GetDefaultValue(), &vpredef, &lpsxparent );

}

/*	
	CStringEx sxfullname = WhereIsDfnTyp( _sxfilename );

	CForm f;
	fl.LoadForm( f, sxfullname );
	if( sxfullname.empty() )
		return;
	CFormBodyElt* pfbetype = f.GetElt("Type");
	nlassert( pfbetype );
	CFormBodyEltList* pfbepredef = dynamic_cast< CFormBodyEltList* >( f.GetElt("Predef") );      
	if( !pfbepredef )
	{
		pfbepredef = new CFormBodyEltList;
		f.GetBody()->AddElt( pfbepredef );
	}	
	pfbepredef->Clear();

	int i = 0;
	CFormBodyEltStruct* pfbes;
	CFormBodyEltAtom* pfbea;
	CStringEx sx;
	CStringEx sxtype = pfbetype->GetValue();
	if( sxtype == "string" )
	{
		for( std::vector< CStringEx >::const_iterator it = _pvsx.begin(); it != _pvsx.end(); ++it )
		{
			pfbes = new CFormBodyEltStruct;
			sx.format( "#%d", i++ );
			pfbes->SetName( sx );
			pfbepredef->AddElt( pfbes );
			
			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Designation" );
			pfbea->SetValue( *it );
			pfbes->AddElt( pfbea );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Substitute" );
			pfbea->SetValue( *it );
			pfbes->AddElt( pfbea );
		}
		pfbea = dynamic_cast< CFormBodyEltAtom* >( f.GetElt("DefaultValue") );
		nlassert( pfbea );
		pfbea->SetValue( _pvsx[0] );
	}
	else
	{
		if( ( sxtype == "uint" )||( sxtype == "sint" ) )
		{
			CStringEx sx2;			
			for( std::vector< CStringEx >::const_iterator it = _pvsx.begin(); it != _pvsx.end(); ++it )
			{
				pfbes = new CFormBodyEltStruct;
				sx.format( "#%d", i );
				pfbes->SetName( sx );
				pfbepredef->AddElt( pfbes );
				
				pfbea = new CFormBodyEltAtom;
				pfbea->SetName( "Designation" );
				pfbea->SetValue( *it );
				pfbes->AddElt( pfbea );

				pfbea = new CFormBodyEltAtom;
				pfbea->SetName( "Substitute" );
				sx.format( "%d", i++ );
				pfbea->SetValue( sx2 );
				pfbes->AddElt( pfbea );
			}
			CFormBodyElt* pfbeenum = f.GetElt("Enum");
			nlassert( pfbeenum );
			pfbea = dynamic_cast< CFormBodyEltAtom* >( f.GetElt("DefaultValue") );
			nlassert( pfbea );
			if( pfbeenum->GetValue() == "true" )
				pfbea->SetValue( _pvsx[0] );
			pfbea = dynamic_cast< CFormBodyEltAtom* >( f.GetElt("Lowlimit") );
			nlassert( pfbea );
			pfbea->SetValue( _pvsx[0] );
			pfbea = dynamic_cast< CFormBodyEltAtom* >( f.GetElt("Highlimit") );
			nlassert( pfbea );
			pfbea->SetValue( _pvsx[_pvsx.size()-1] );
		}
	}
}

void CMoldEltType::Save()
{

	fl.SaveForm( f, sxfullname );
}
*/

