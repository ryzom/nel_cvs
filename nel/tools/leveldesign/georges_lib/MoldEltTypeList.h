// MoldEltType.h: interface for the CMoldEltType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOLDELTTYPELIST_H__3A065B3D_1B70_4F14_94DD_554CFC8C433A__INCLUDED_)
#define AFX_MOLDELTTYPELIST_H__3A065B3D_1B70_4F14_94DD_554CFC8C433A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MoldEltType.h"
#include <vector>

class CLoader;
class CTypeUnit;

class CMoldEltTypeList : public CMoldEltType  
{
protected:
	CMoldEltType* pmet;

public:
	CMoldEltTypeList( CLoader* const _pl, CMoldEltType* const _pmet );
	virtual ~CMoldEltTypeList();

	virtual void Load( const CStringEx _sxfullname );
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate );

	virtual CMoldElt* GetMold(); 
	virtual CStringEx	GetDefaultValue() const;															// Give the default value of the type
	virtual CStringEx	CalculateResult( const CStringEx _sxvalue, const CStringEx _sxbasevalue ) const;	// Calculate the final result
	virtual CStringEx	Format( const CStringEx _sxvalue ) const;											// Transform any value in the good format					
	virtual CStringEx	GetPredefSubstitute( const CStringEx _sxdesignation ) const;
	virtual CStringEx	GetPredefDesignation( const CStringEx _sxdesignation ) const;						// Give the value corresponding to the string
	virtual CStringEx	GetPredefDesignation( const unsigned int _index ) const;
	virtual	CStringEx	GetFormula();																		// Give the formula of the type : int(0,100)
	virtual unsigned int GetType() const;																	// Give the type ( typ or dfn or nothing )	
};

#endif // !defined(AFX_MOLDELTTYPELIST_H__3A065B3D_1B70_4F14_94DD_554CFC8C433A__INCLUDED_)
