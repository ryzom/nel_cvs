// TypeUnit.h: interface for the CTypeUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TYPEUNIT_H__BAB9EC8B_427A_428D_B149_F733629EDD44__INCLUDED_)
#define AFX_TYPEUNIT_H__BAB9EC8B_427A_428D_B149_F733629EDD44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"

class CTypeUnit  
{
protected:
	CStringEx	sxformula;
	CStringEx	sxhighlimit;
	CStringEx	sxlowlimit;
	CStringEx	sxdefaultvalue;

public:
	CTypeUnit( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnit();

	CStringEx GetFormula() const;													
	CStringEx GetDefaultValue() const;													
	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
};

#endif // !defined(AFX_TYPEUNIT_H__BAB9EC8B_427A_428D_B149_F733629EDD44__INCLUDED_)
