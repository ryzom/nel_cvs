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

void CLoader::MakeDfn( const CStringEx _sxfullname, const std::list< std::pair< CStringEx, CStringEx > >* const _plistdefine )
{
	CFormFile pff;
	CForm f;

	CFormBodyEltStruct* pbody = f.GetBody();
	CFormBodyEltAtom* pfbea;

	for( std::list< std::pair< CStringEx, CStringEx > >::const_iterator it = _plistdefine->begin(); it != _plistdefine->end(); ++it )
	{
		pfbea = new CFormBodyEltAtom;
		pfbea->SetName( it->first );
		pfbea->SetValue( it->second );
		pbody->AddElt( pfbea );
	}

	pff.SetForm( f );
	pff.Save( _sxfullname );
}

void CLoader::MakeTyp( const CStringEx _sxfullname, const CStringEx _sxtype, const CStringEx _sxformula, const CStringEx _sxenum, const CStringEx _sxlow, const CStringEx _sxhigh, const CStringEx _sxdefault, const std::list< std::pair< CStringEx, CStringEx > >* const _plistpredef, const std::list< std::pair< CStringEx, CStringEx > >* const _plistparent )
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
	pfbel = new CFormBodyEltList;
	pfbel->SetName( "Predef" );
	int i = 0;
	std::list< std::pair< CStringEx, CStringEx > >::const_iterator it;
	for( it = _plistpredef->begin(); it != _plistpredef->end(); ++it )
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

	pfbel = new CFormBodyEltList;
	pfbel->SetName( "Parent" );
	i = 0;
	for( it = _plistparent->begin(); it != _plistparent->end(); ++it )
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

	pff.SetForm( f );
	pff.Save( _sxfullname );
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

