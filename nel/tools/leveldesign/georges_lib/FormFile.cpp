// FormFile.cpp: implementation of the CFormFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "FormFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormFile::CFormFile()
{
}

CFormFile::~CFormFile()
{
}

void CFormFile::serial( NLMISC::IStream& s )
{
	s.xmlPush( "File_of_forms" );
		s.serialCheck( (uint32)'FORM' );
		s.serialVersion( 0 );
		s.serialCont( lform );
	s.xmlPop();
}

void CFormFile::Load( const CStringEx _sxfullname )
{
	NLMISC::CIFile fileIn;
	fileIn.open( CStringEx( _sxfullname ) );
	NLMISC::CIXml input;											
	input.init( fileIn );
	serial( input );
}																	// Exception if fileIn.close();

void CFormFile::Save( const CStringEx _sxfullname )
{
	NLMISC::COFile fileOut;
	fileOut.open( CStringEx( _sxfullname ) );
	NLMISC::COXml output;											
	output.init( &fileOut );
	serial( output );
	output.flush();
	fileOut.close();
}																	// Exception if fileOut.close();

void CFormFile::GetForm( CForm& _f ) const 
{
	_f = lform.front();												// The first form is copying
}

void CFormFile::GetForm( CForm& _f, const CStringEx& _sxdate ) const 
{
	std::list< CForm >::const_iterator cit = lform.begin();
	_f = *cit;														// The first form is copying
	while( (cit != lform.end())&&( _sxdate <= cit->GetDate() ) )
		_f += *(++cit);												// While history's date is after the date, add history's form.
}

void CFormFile::SetForm( CForm& _f ) 
{
	if( lform.empty() )					
	{
		lform.push_front( _f );
		return;
	}

	if( ( !_f.GetModified() )||( _f == lform.front() ) )
		return;														// It's the same! Don't save...

	CForm f = lform.front();										// Copy the old first form
	f -= _f;														// overwrite with differences
	lform.pop_front();												// Delete the old fist form
	lform.push_front( f );											// Replace it by differences
	lform.push_front( _f );											// Place the new form in first position
}

