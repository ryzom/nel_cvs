// CStringEx.cpp: implementation of the CStringEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringEx.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;
CStringEx::~CStringEx()
{

}

void CStringEx::remove( const char _c )
{
	if( empty() )
		return;
	iterator it = begin();
	while( it != end() ) 
	{
		if( (*it) == _c )
			it = erase( it );
		else
			++it;
	}
}

void CStringEx::remove()
{
	remove(' ');
	remove('\t');
	remove('\n');
}

void	CStringEx::make_lower( )
{
	for( iterator it = begin(); it != end(); ++it )
		if( (*it >= 'A')&&(*it <= 'Z') )
			*it += 'a'-'A';
}

void	CStringEx::make_upper( )
{
	for( iterator it = begin(); it != end(); ++it )
		if( (*it >= 'a')&&(*it <= 'z') )
			*it += 'A'-'a';
}

void	CStringEx::trim_left( )
{
	if( empty() )
		return;
	for( iterator it = begin(); (it != end())&&( (*it==' ')||(*it=='\t')||(*it=='\n') ); ++it );
	erase( begin(), it );
}

void	CStringEx::trim_left( const char _c )
{
	if( empty() )
		return;
	for( iterator it = begin(); (it != end())&&( *it == _c ); ++it );
	erase( begin(), it );
}

void	CStringEx::trim_right( ) 
{
	if( empty() )
		return;
	iterator it = end();
	--it;
	while( it != begin() )
	{
		iterator i = it--;
		if( (*i==' ')||(*i=='\t')||(*i=='\n') )
			erase( i );
		else
			break;
	}
	if( (*it==' ')||(*it=='\t')||(*it=='\n') )
		erase( it );
}

void	CStringEx::trim_right( char c )
{
	if( empty() )
		return;
	iterator it = end();
	while( it != begin() )
	{
		iterator i = it--;
		if( *i == c )
			erase( i );
		else
			break;
	}
	if( *it == c )
		erase( it );
}

void CStringEx::trim()
{
	trim_left();
	trim_right();
}

void CStringEx::purge()
{
	make_lower();
	remove(' ');
	remove('\t');
	remove('\n');
}

void CStringEx::trim( const char _c )
{
	trim_left( _c );
	trim_right( _c );
}

void	CStringEx::mid( const int nFirst )
{
	CStringEx s( *this );
	erase();
	append( s.get_mid( nFirst ));
}

void	CStringEx::mid( const int nFirst, const int nCount )
{
	CStringEx s( *this );
	erase();
	append( s.get_mid( nFirst, nCount ));
}

void	CStringEx::left( const int nCount )
{
	CStringEx s( *this );
	erase();
	append( s.get_left( nCount ));
}

void	CStringEx::right( const int nCount )
{
	CStringEx s( *this );
	erase();
	append( s.get_right( nCount ));
}


CStringEx CStringEx::get_remove( const char _c ) const
{
	CStringEx s( *this );
	s.remove( _c );
	return( s );
}

CStringEx CStringEx::get_remove() const
{
	CStringEx s( *this );
	s.remove();
	return( s );
}

CStringEx CStringEx::get_make_lower() const
{
	CStringEx s( *this );
	s.make_lower();
	return( s );
}

CStringEx CStringEx::get_make_upper() const
{
	CStringEx s( *this );
	s.make_upper();
	return( s );
}

CStringEx CStringEx::get_trim_left() const
{
	CStringEx s( *this );
	s.trim_left();
	return( s );
}

CStringEx CStringEx::get_trim_left( const char _c ) const
{
	CStringEx s( *this );
	s.trim_left( _c );
	return( s );
}

CStringEx CStringEx::get_trim_right() const
{
	CStringEx s( *this );
	s.trim_right();
	return( s );
}

CStringEx CStringEx::get_trim_right( const char _c ) const
{
	CStringEx s( *this );
	s.trim_right( _c );
	return( s );
}

CStringEx CStringEx::get_trim() const
{
	CStringEx s( *this );
	s.trim();
	return( s );
}

CStringEx CStringEx::get_purge() const
{
	CStringEx s( *this );
	s.purge();
	return( s );
}

CStringEx CStringEx::get_trim( const char _c ) const
{
	CStringEx s( *this );
	s.trim( _c );
	return( s );
}

CStringEx	CStringEx::get_mid( const int nFirst ) const
{
	if( !size() )
		return( CStringEx() );
	return( get_right( size()-nFirst ) );
}

CStringEx	CStringEx::get_mid( const int nFirst, const int nCount ) const
{
	if( !size() )
		return( CStringEx() );
	return( substr( nFirst, nCount ) );
}

CStringEx	CStringEx::get_left( const int nCount ) const
{
	if( !size() )
		return( CStringEx() );
	return( substr( 0, nCount ) );
}

CStringEx	CStringEx::get_right( const int nCount ) const
{
	if( !size() )
		return( CStringEx() );
	return( substr( size()-nCount, nCount ) );
}


bool  CStringEx::operator <= ( const CStringEx& s ) const 
{
	const_iterator it = begin();
	const_iterator is = s.begin();
	while( ( it != end() )&&( is != s.end() ) )
	{
		if( *it != *is )
			return( *it < *is );
		it++;
		is++;
	}
	return( ( it == end() )&&( is == s.end() ) );
}

bool  CStringEx::operator < ( const CStringEx& s ) const 
{
	const_iterator it = begin();
	const_iterator is = s.begin();
	while( ( it != end() )&&( is != s.end() ) )
	{
		if( *it != *is )
			return( *it < *is );
		it++;
		is++;
	}
	return( is != s.end() );
}

unsigned int CStringEx::reverse_find( const char _c ) const
{
	unsigned int i = length();
	const_iterator it = end();
	while( it != begin() )
	{
		--it;
		--i;
		if( *it == _c )
			return( i );
	}
	return( npos );
}

void CStringEx::format( const char* s, ... )
{
	char *p = new char[256];
	va_list ap;
	va_start(ap, s);
	int x = vsprintf( p, s, ap);
	erase();
	append(p);
	delete[] p;
}

/*
   #include <stdio.h>
   #include <stdlib.h>
   #include <stdarg.h>
   char *newfmt(const char *fmt, ...)
   {
	   char *p;
	   va_list ap;
	   if ((p = malloc(128)) == NULL)
			   return (NULL);
	   va_start(ap, fmt);
	   (void) vsnprintf(p, 128, fmt, ap);
	   va_end(ap);
	   return (p);
   }
*/
