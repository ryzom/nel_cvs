// FormBodyEltAtom.h: interface for the CFormBodyEltAtom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMBODYELTATOM_H__C3372771_C4F1_4239_89C5_DB43C8FF8445__INCLUDED_)
#define AFX_FORMBODYELTATOM_H__C3372771_C4F1_4239_89C5_DB43C8FF8445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "StringEx.h"
#include "FormBodyElt.h"

// La classe CFormBodyEltAtom représente l'élément terminal de l'arbre. 
// Il est composé de deux string: sxname et sxvalue
class CFormBodyEltAtom : public CFormBodyElt  
{
protected:
	CStringEx	sxvalue;

public:
	NLMISC_DECLARE_CLASS( CFormBodyEltAtom );
	CFormBodyEltAtom();
	CFormBodyEltAtom( const CFormBodyEltAtom& _fbea );
	virtual ~CFormBodyEltAtom();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	virtual CStringEx GetValue() const;

	void SetValue( const CStringEx _sxvalue );
};

#endif // !defined(AFX_FORMBODYELTATOM_H__C3372771_C4F1_4239_89C5_DB43C8FF8445__INCLUDED_)
