// TypeUnitDouble.h: interface for the CTypeUnitDouble class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TYPEUNITDOUBLE_H__06DB5A4A_9F2C_496D_9261_4B94CC20AEB1__INCLUDED_)
#define AFX_TYPEUNITDOUBLE_H__06DB5A4A_9F2C_496D_9261_4B94CC20AEB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TypeUnit.h"

class CTypeUnitDouble : public CTypeUnit  
{
protected:
	double dlowlimit;
	double dhighlimit;
	double ddefaultvalue;
	CStringEx FormatDouble( const double dvalue ) const;

public:
	CTypeUnitDouble( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv );
	virtual ~CTypeUnitDouble();

	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	

};

#endif // !defined(AFX_TYPEUNITDOUBLE_H__06DB5A4A_9F2C_496D_9261_4B94CC20AEB1__INCLUDED_)
