// FormHead.cpp: implementation of the CFormHead class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormHead.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormHead::CFormHead()
{
	sxuser = "Anonymous";
	sxversion = "0.0";
//	sxdate = "unknow";
}

CFormHead::CFormHead( const CFormHead& _fh )
{
	sxuser = _fh.sxuser;
	sxversion = _fh.sxversion;
	sxdate = _fh.sxdate;
}

CFormHead::~CFormHead()
{
}

CStringEx CFormHead::GetUser() const
{
	return( sxuser );
}

CStringEx CFormHead::GetVersion() const
{
	return( sxversion );
}

CStringEx CFormHead::GetDate() const
{
	return( sxdate );
}

void CFormHead::serial( NLMISC::IStream& s )
{

	s.xmlPushBegin( "Head" );
		s.xmlSetAttrib( "User_name" );
			s.serial( sxuser );
		s.xmlSetAttrib( "Version_number" );
			s.serial( sxversion );
		s.xmlSetAttrib( "Date" );
			s.serial( sxdate );
	s.xmlPushEnd();
	s.xmlPop();
/*
	s.xmlPush( "Head" );
		s.serial( sxuser );
		s.serial( sxversion );
		s.serial( sxdate );
	s.xmlPop();
*/
}

CFormHead&	CFormHead::operator =( const CFormHead& _fh )
{
	sxuser = _fh.sxuser;
	sxversion = _fh.sxversion;
	sxdate = _fh.sxdate;
	return( *this );
}
