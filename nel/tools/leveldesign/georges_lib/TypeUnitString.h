// TypeUnitString.h: interface for the CTypeUnitString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TYPEUNITSTRING_H__C32CF14C_50DE_4747_BC9E_4C1D088308FC__INCLUDED_)
#define AFX_TYPEUNITSTRING_H__C32CF14C_50DE_4747_BC9E_4C1D088308FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TypeUnit.h"

class CTypeUnitString : public CTypeUnit  
{
protected:
	unsigned short int	usihighlimit;

public:
	CTypeUnitString( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv );
	virtual ~CTypeUnitString();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
};

#endif // !defined(AFX_TYPEUNITSTRING_H__C32CF14C_50DE_4747_BC9E_4C1D088308FC__INCLUDED_)
