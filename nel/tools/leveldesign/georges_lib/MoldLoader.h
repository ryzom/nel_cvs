// MoldLoader.h: interface for the CMoldLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDLOADER_H__5E812FFC_E18D_45A4_A955_703EB2F8DFC3__INCLUDED_)
#define AFX_MOLDLOADER_H__5E812FFC_E18D_45A4_A955_703EB2F8DFC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"
#include < map >

class CMoldElt;
class CLoader;

class CMoldLoader  
{
protected:
	std::map< CStringEx, CMoldElt* > mmold;
	std::vector< CMoldElt* > vpme;
	CLoader* pl;	

public:
	CMoldLoader();
	virtual ~CMoldLoader();

	void Clear();
	void SetLoader( CLoader* const _pl );
	CMoldElt* LoadMold( const CStringEx _sxfilename );
	CMoldElt* LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ); 
//	CMoldElt* Find( const CStringEx _sxfullname ); 
};

#endif // !defined(AFX_MOLDLOADER_H__5E812FFC_E18D_45A4_A955_703EB2F8DFC3__INCLUDED_)


/*
struct StringExGreater : public std::binary_function< CStringEx, CStringEx, bool > 
{
	bool operator()( const CStringEx& x, const CStringEx& y ) const
	{
		return( y <= x );
	}
};
//	std::map< CStringEx, CMoldElt*, StringExGreater > moldmap;
*/
