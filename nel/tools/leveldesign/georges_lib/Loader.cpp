// Loader.cpp: implementation of the CLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Loader.h"

#include "FormBodyEltAtom.h"
#include "FormBodyEltStruct.h"
#include "FormBodyEltList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoader::CLoader()
{
	ml.SetLoader( this );
	sxworkdirectory = "U:\\";
	sxrootdirectory = "U:\\";
	NLMISC::CPath::clearMap();
    NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
    NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
}

CLoader::~CLoader()
{
}

void CLoader::LoadForm( CForm& _f, const CStringEx& _sxfullname )
{
	fl.LoadForm( _f, _sxfullname );
}

void CLoader::LoadForm( CForm& _f, const CStringEx& _sxfullname, const CStringEx& _sxdate ) 
{
	fl.LoadForm( _f, _sxfullname, _sxdate );
}

void CLoader::LoadSearchForm( CForm& _f, const CStringEx& _sxfilename )
{
	fl.LoadForm( _f, WhereIsForm( _sxfilename ) );
}

void CLoader::LoadSearchForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	fl.LoadForm( _f, WhereIsForm( _sxfilename ), _sxdate );
}

void CLoader::SaveForm( CForm& _f, const CStringEx& _sxfullname )
{
	fl.SaveForm( _f, _sxfullname );
}

CMoldElt* CLoader::LoadMold( const CStringEx _sxfilename )
{
	return( ml.LoadMold( _sxfilename ) );
}

CMoldElt* CLoader::LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ) 
{
	return( ml.LoadMold( _sxfilename, _sxdate ) );
}

void CLoader::SetWorkDirectory( const CStringEx _sxworkdirectory )
{
	sxworkdirectory = _sxworkdirectory;
	NLMISC::CPath::clearMap();
    NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
    NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
}

void CLoader::SetRootDirectory( const CStringEx _sxrootdirectory )
{
	sxrootdirectory = _sxrootdirectory;
	NLMISC::CPath::clearMap();
    NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
    NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
}

CStringEx CLoader::GetWorkDirectory() const
{
	return( sxworkdirectory );
}

CStringEx CLoader::GetRootDirectory() const
{
	return( sxrootdirectory );
}

CStringEx CLoader::WhereIsDfnTyp( const CStringEx _sxfilename )
{
	return( NLMISC::CPath::lookup( _sxfilename, false ) );
}

CStringEx CLoader::WhereIsForm( const CStringEx _sxfilename )
{
	return( NLMISC::CPath::lookup( _sxfilename, false ) );
}

void CLoader::MakeDfn( const CStringEx _sxfullname, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvdefine )
{
	CFormFile pff;
	CForm f;

	CFormBodyEltStruct* pbody = f.GetBody();
	CFormBodyEltAtom* pfbea;

	if( ( _pvdefine )&&( !_pvdefine->empty() ) )
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _pvdefine->begin(); it != _pvdefine->end(); ++it )
		{
			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( it->first );
			pfbea->SetValue( it->second );
			pbody->AddElt( pfbea );
		}

	pff.SetForm( f );
	pff.Save( _sxfullname );
}

void CLoader::MakeTyp( const CStringEx _sxfullname, const CStringEx _sxtype, const CStringEx _sxformula, const CStringEx _sxenum, const CStringEx _sxlow, const CStringEx _sxhigh, const CStringEx _sxdefault, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvpredef , const std::vector< std::pair< CStringEx, CStringEx > >* const _pvparent )
{
	CFormFile pff;
	CForm f;

	CFormBodyEltStruct* pbody = f.GetBody();
	CFormBodyEltAtom* pfbea;
	CFormBodyEltList* pfbel;
	CFormBodyEltStruct* pfbes;

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Type" );
	pfbea->SetValue( _sxtype );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Enum" );
	pfbea->SetValue( _sxenum );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Formula" );
	pfbea->SetValue( _sxformula );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Lowlimit" );
	pfbea->SetValue( _sxlow );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Highlimit" );
	pfbea->SetValue( _sxhigh );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "DefaultValue" );
	pfbea->SetValue( _sxdefault );
	pbody->AddElt( pfbea );

	CStringEx sx;

	if( ( _pvpredef )&&( !_pvpredef->empty() ) )
	{
		pfbel = new CFormBodyEltList;
		pfbel->SetName( "Predef" );
		int i = 0;
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _pvpredef->begin(); it != _pvpredef->end(); ++it )
		{
			pfbes = new CFormBodyEltStruct;
			sx.format( "#%d", i++ );
			pfbes->SetName( sx );
			pfbel->AddElt( pfbes );
			
			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Designation" );
			pfbea->SetValue( it->first );
			pfbes->AddElt( pfbea );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Substitute" );
			pfbea->SetValue( it->second );
			pfbes->AddElt( pfbea );
		}
		pbody->AddElt( pfbel );
	}

	if( ( _pvparent )&&( !_pvparent->empty() ) )
	{
		pfbel = new CFormBodyEltList;
		pfbel->SetName( "Parent" );
		int i = 0;
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _pvparent->begin(); it != _pvparent->end(); ++it )
		{
			pfbes = new CFormBodyEltStruct;
			sx.format( "#%d", i++ );
			pfbes->SetName( sx );
			pfbel->AddElt( pfbes );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Activity" );
			pfbea->SetValue( it->first );
			pfbes->AddElt( pfbea );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Filename" );
			pfbea->SetValue( it->second );
			pfbes->AddElt( pfbea );
		}
		pbody->AddElt( pfbel );
	}

	pff.SetForm( f );
	pff.Save( _sxfullname );
}

void CLoader::SetTypPredef( const CStringEx _sxfilename, const std::vector< CStringEx >& _pvsx )
{
	CStringEx sxpathname = WhereIsDfnTyp( _sxfilename );
	CForm f;
	fl.LoadForm( f, sxpathname );
	if( sxpathname.empty() )
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
		
	fl.SaveForm( f, sxpathname );
}


/*
CStringEx CLoader::WhereIs( const CStringEx _sxdirectory, const CStringEx _sxfilename )
{
	if( _sxfilename.empty() )
		return( CStringEx() );

	_finddata_t info;
	CStringEx searchname = _sxdirectory +_sxfilename;
	long lhandle = _findfirst( searchname.c_str(), &info );
	if( lhandle != -1 )
	{
		_findclose( lhandle );
		return( searchname );
	}

	searchname = CStringEx( _sxdirectory + "*.*" );
	lhandle = _findfirst( searchname.c_str(), &info ); 
	do
	{
		if( !(info.attrib & _A_SUBDIR ) )
			continue;
		if( ( info.name == "." )||( info.name == ".." ) )
			continue;
		CStringEx sxresult = WhereIs( CStringEx( _sxdirectory + info.name ), _sxfilename );
		if( !sxresult.empty() )
			return( sxresult );
	}
	while( _findnext( lhandle, &info ) != -1 );
	
	_findclose( lhandle );
	return( CStringEx() );
}

CStringEx CLoader::WhereIsDfnTyp( const CStringEx _sxfilename )
{
	CStringEx sxfullname = WhereIs( CStringEx( sxworkdirectory +"dfn\\" ), _sxfilename );
	if( sxfullname.empty() && ( sxrootdirectory != sxworkdirectory ) )
		sxfullname = WhereIs( CStringEx( sxrootdirectory +"dfn\\" ), _sxfilename );
	return( sxfullname );
}

CStringEx CLoader::WhereIsForm( const CStringEx _sxfilename )
{
	CStringEx sxfullname = WhereIs( sxworkdirectory, _sxfilename );
	if( sxfullname.empty() && ( sxrootdirectory != sxworkdirectory ) )
		sxfullname = WhereIs( sxrootdirectory, _sxfilename );
	return( sxfullname );
}
*/

