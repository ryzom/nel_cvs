// StringEx.h: interface for the stringEx class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable:4786)

#if !defined(AFX_STRINGEX_H__79C5601E_99DA_4C4D_A4AC_3421F33D4033__INCLUDED_)
#define AFX_STRINGEX_H__79C5601E_99DA_4C4D_A4AC_3421F33D4033__INCLUDED_
#pragma once

#include <string>
#include "nel/misc/stream.h"

class CStringEx : public std::basic_string<char, std::char_traits<char>, std::allocator<char> >  
{

public:
	CStringEx()						: std::basic_string<char, std::char_traits<char>, std::allocator<char> >() {;} 
	CStringEx( char* _pc ) 			: std::basic_string<char, std::char_traits<char>, std::allocator<char> >( (char *)_pc ) {};
	CStringEx( std::string _s ) 	: std::basic_string<char, std::char_traits<char>, std::allocator<char> >( (std::string)_s) {}; 
	CStringEx( const char _c, int _i ) 	{ append( _i, _c ); }
	
	virtual ~CStringEx();
	CStringEx	get_remove() const;
	CStringEx	get_remove( const char ) const;
	CStringEx	get_make_lower() const;
	CStringEx	get_make_upper() const;
	CStringEx	get_trim_left() const;
	CStringEx	get_trim_left( const char _c ) const;
	CStringEx	get_trim_right() const; 
	CStringEx	get_trim_right( const char _c ) const;
	CStringEx	get_trim() const;
	CStringEx	get_trim( const char _c ) const;
	CStringEx	get_mid( const int nFirst, const int nCount ) const; 
	CStringEx	get_mid( const int nFirst ) const; 
	CStringEx	get_left( const int nCount ) const; 
	CStringEx	get_right( const int nCount ) const;
	CStringEx	get_purge() const;

	void		remove();
	void		remove( const char _c );
	void		make_lower();
	void		make_upper();
	void		trim_left();
	void		trim_left( const char _c );
	void		trim_right();
	void		trim_right( const char _c );
	void		trim();
	void		trim( const char _c );
	void		mid( const int nFirst, const int nCount ); 
	void		mid( const int nFirst ); 
	void		left( const int nCount ); 
	void		right( const int nCount );
	void		purge();

	void		format( const char*, ... ); 
	int			reverse_find( const char _c ) const; 
	
	void	serial( NLMISC::IStream& s ) { s.serial( (std::string&)(*this) );}

	bool	operator <= ( const CStringEx& s ) const;
	bool	operator < ( const CStringEx& s ) const;
};

#endif // !defined(AFX_STRINGEX_H__79C5601E_99DA_4C4D_A4AC_3421F33D4033__INCLUDED_)
