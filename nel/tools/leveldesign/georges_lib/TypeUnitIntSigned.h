// TypeUnitIntSigned.h: interface for the CTypeUnitIntSigned class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TYPEUNITINTSIGNED_H__8F241537_4B04_4C4D_9E31_EBA829908C4B__INCLUDED_)
#define AFX_TYPEUNITINTSIGNED_H__8F241537_4B04_4C4D_9E31_EBA829908C4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TypeUnit.h"

class CTypeUnitIntSigned : public CTypeUnit  
{
protected:
	signed __int64	ilowlimit;
	signed __int64	ihighlimit;
	signed __int64	idefaultvalue;

public:
	CTypeUnitIntSigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnitIntSigned();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
};

#endif // !defined(AFX_TYPEUNITINTSIGNED_H__8F241537_4B04_4C4D_9E31_EBA829908C4B__INCLUDED_)
