// TypeUnitIntUnsigned.h: interface for the CTypeUnitIntUnsigned class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TYPEUNITINTUNSIGNED_H__FC3B0C14_EEDC_40E6_A45B_3D1E8965E9AA__INCLUDED_)
#define AFX_TYPEUNITINTUNSIGNED_H__FC3B0C14_EEDC_40E6_A45B_3D1E8965E9AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TypeUnit.h"

class CTypeUnitIntUnsigned : public CTypeUnit  
{
protected:
	unsigned __int64	ilowlimit;
	unsigned __int64	ihighlimit;
	unsigned __int64	idefaultvalue;

public:
	CTypeUnitIntUnsigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnitIntUnsigned();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
	virtual	void	SetDefaultValue( const CStringEx _sxdv );
	virtual	void	SetLowLimit( const CStringEx _sxll );
	virtual	void	SetHighLimit( const CStringEx _sxhl );
};

#endif // !defined(AFX_TYPEUNITINTUNSIGNED_H__FC3B0C14_EEDC_40E6_A45B_3D1E8965E9AA__INCLUDED_)
